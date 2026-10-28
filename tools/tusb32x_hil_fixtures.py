"""Finite physical-fixture plans using the existing Session and ordinary CLI operations.

Operator confirmations are prerequisites, not measured electrical evidence. Saved
observations retain both the attestation and actual typed firmware records. No
GPIO, power control, register protocol, retry or replacement result store lives here.
The caller owns serial teardown; all waits include the immutable whole-run deadline.
"""

import math
import time

from tusb32x_cli_contract import SCHEMAS
from tusb32x_hil_plans import BASE_VARIANTS, PlanBlocked, PlanFailure, _payload, _plan_deadline
from tusb32x_hil_fixture_setup import FAULT_CODES, FIXTURE_SUITES, console_transition


PROFILE_COMMANDS = {
    "mode": ("mode", 1), "current": ("current", 2), "preference": ("preference", 4),
    "duty": ("duty", 8), "debounce": ("debounce", 16),
    "ufpaccessory": ("ufpaccessory", 32), "termination": ("termination", 64),
    "rd_rp": ("rd-rp", 128),
}
ACCESSORIES = {
    "audio": "AUDIO", "debug": "DEBUG", "charge-through": "CHARGE_THROUGH",
    "debug-local-dfp": "DEBUG_LOCAL_DFP", "debug-local-ufp": "DEBUG_LOCAL_UFP",
}
CURRENT = {"default": "DEFAULT", "1a5": "MA_1500", "3a0": "MA_3000"}


def _controls_consistent(profile, fields):
    """Require each applicable on/off control to agree with its raw disable flag."""
    return all(not fields & PROFILE_COMMANDS[key][1] or (
        profile[key] in {"on", "off"} and profile[raw] == ("0" if profile[key] == "on" else "1"))
        for key, raw in (("ufpaccessory", "disable_ufp_accessory"),
                         ("termination", "disable_term"), ("rd_rp", "disable_rd_rp")))


class FixtureNotRun(PlanBlocked):
    """Missing fixture/operator evidence prevents execution; NOT RUN, CLI exit 2."""


def fixture_prerequisite(plan, setup, expected, *, base_recovery_required=None):
    """Reject missing setup/gates before traffic, using current-run base startup proof.

    None uses the initial operator declaration. A bool supersedes that declaration
    after this run's power cycle or qualified typed recovery; it is not USB policy.
    """
    if not setup:
        raise FixtureNotRun("explicit --fixture and --fixture-config required")
    if base_recovery_required is not None and type(base_recovery_required) is not bool:
        raise PlanBlocked("current-run base recovery evidence must be a boolean")
    if not expected or not {"git", "framework", "target"} <= expected.keys():
        raise PlanBlocked("fixture plans require expected flashed commit/framework/target")
    if setup["variant"] != plan.variant:
        raise PlanBlocked("fixture setup variant differs from selected exact part")
    if plan.variant == "tusb322i" and any(suite in FIXTURE_SUITES and suite != "console"
                                        for suite in plan.suites):
        raise PlanBlocked("TUSB322I fixture writes/return-to-DRP remain gated; "
                          "advertisement/09/fault-clear evidence is unresolved")
    for suite in plan.suites:
        required = {suite} if suite != "accessory" else set(ACCESSORIES)
        if suite in FIXTURE_SUITES and not required & set(setup["capabilities"]):
            raise FixtureNotRun("declared fixture capability absent: " + suite)
    if (plan.variant in BASE_VARIANTS and "accessory" in plan.suites and
            set(setup["capabilities"]) & {"debug-local-dfp", "debug-local-ufp"}):
        raise PlanBlocked("selected base variant does not support declared extended accessory")
    required_actions = set()
    if "reset-termination" in plan.suites:
        required_actions.add("reset")
        if plan.variant in BASE_VARIANTS:
            required_actions.add("recover-dead-battery")
            if not setup.get("stable_vdd") or not setup.get("source_startup"):
                raise PlanBlocked("base recovery requires stable VDD and source startup readiness")
    if "fault" in plan.suites:
        required_actions.update(("nack", "disconnect", "power-cycle"))
        if any(setup.get(name) not in FAULT_CODES for name in ("nack_code", "disconnect_code")):
            raise PlanBlocked("fault setup requires exact NACK and disconnect backend codes")
    if "console" in plan.suites:
        required_actions.update(("console-backpressure", "console-reconnect", "console-reboot"))
        if plan.variant in BASE_VARIANTS and not setup.get("target_vdd_continuous"):
            raise PlanBlocked("original-part console requires target VDD continuity "
                              "across MCU reboot")
    if required_actions - set(setup.get("authorized_actions", [])):
        raise PlanBlocked("fixture actions require separate explicit setup authorization")
    base_ready = (setup.get("base_recovery_complete") if base_recovery_required is None
                  else not base_recovery_required)
    if not setup.get("startup_ready") or (plan.variant in BASE_VARIANTS and
            "reset-termination" not in plan.suites and not base_ready):
        raise PlanBlocked("owner supply/startup readiness and base recovery proof required")


def run_fixture_plan(session, plan, setup, expected=None, *, transition=console_transition,
                     clock=time.monotonic, sleep=time.sleep, run_deadline=None, reconnect=None):
    """Execute selected fixture suites, consuming each exact result once and saving outcomes.

    ``setup`` is validated by load_setup before opening. ``transition(step, deadline)``
    returns True only after explicit physical setup/observation, False for unavailable
    evidence; injected tests model these observations and never qualify hardware.
    No mutation is retried. Device restoration requires a successful primary plan,
    a fresh detached fixture attestation, unchanged identity/target/supply, coherent
    port data and known actual settings. A failed/uncertain plan performs no device
    restoration; explicit later owner work must resolve its residual effects.
    Optional run_deadline is an absolute monotonic host bound in seconds; session
    construction and earlier suites cannot renew it.
    For console, reconnect(old_session, absolute_deadline) must return a fresh,
    unstarted Session over the explicitly reopened transport and the same artifacts.
    The caller owns opening/closing; no old token or local baseline is transferred.
    """
    artifacts = session.artifacts
    deadline = _plan_deadline(clock(), plan, run_deadline)
    previous_time = clock()
    dirty = False
    hardware_cleanup = "NOT RUN"
    baseline = None
    fields = 0
    sequence = -1
    observation_time = None
    current_suite = "fixture_setup"
    group_name = "+".join(plan.suites)
    summary = artifacts.metadata.setdefault(
        "fixture_progress", {"transitions": 0, "operations": 0, "mutation_requests": 0})
    artifacts.metadata["plan"] = plan.describe()
    artifacts.metadata["fixture"] = setup
    artifacts.metadata["fixture_progress"] = summary
    evidence = artifacts.metadata.setdefault("fixture_observations", {}).setdefault(group_name, {})
    cleanups = artifacts.metadata.setdefault("fixture_cleanups", {})
    cleanups[group_name] = "NOT RUN"
    artifacts.metadata["hardware_cleanup"] = hardware_cleanup
    for suite in plan.suites:
        artifacts.check(suite, "NOT RUN", "fixture evidence not completed")
    artifacts.check("fixture_cleanup", "NOT RUN", "device cleanup not established")
    artifacts.checkpoint()

    def end_by(bound=deadline):
        """Check monotonic time and clip exchanges/confirmations to one whole-run bound."""
        nonlocal previous_time
        now = clock()
        if not math.isfinite(now) or now < previous_time or now >= min(deadline, bound):
            raise PlanFailure("fixture deadline expired or clock regressed")
        previous_time = now
        return min(deadline, bound, now + plan.command_timeout)

    def command(text, code="OK", limit=None):
        """Require exact delivery status; Session validates schemas and attribution."""
        response = session.command(text, min(end_by(), limit or deadline))
        if response.end is None or response.end["code"] != code:
            status = response.end or response.ack
            raise PlanFailure(f"{text}: {status['code']} detail={status['detail']}")
        return response

    def target():
        """Require the selected physical target and the same flashed firmware identity."""
        command("version")
        bus = _payload(command("bus"), "BUS")
        if bus["variant"] != plan.variant or int(bus["address"]) != plan.address:
            raise PlanBlocked("fixture target changed; no mutation or restoration allowed")
        return bus

    def operation(text, mutation=False, *, expected_code="OK", control=None, label=None):
        """Admit once, step finite one-callback turns, copy evidence and take once.

        Operation payload failure is checked after exact-token delivery. Neither
        failed delivery nor uncertain transport causes automatic cancel/take/retry.
        Wait-only polls sleep at most 5 ms; SDK blocking remains the adapter's bound.
        """
        nonlocal dirty
        limit = end_by()
        if mutation:
            dirty = True
            summary["mutation_requests"] += 1
            artifacts.metadata["device_cleanup_required"] = True
            artifacts.checkpoint()
        wire = text
        if text != "read":
            wire += " " + str(1 if control == "timeout" else max(
                1, math.ceil((limit - clock()) * 1000)))
        admitted = session.command(wire, limit, expected_code=expected_code)
        if not admitted.admitted:
            raise PlanFailure(f"{text} admission: {admitted.ack['code']}")
        if control == "cancel":
            command("cancel " + str(session.operation.token), limit=limit)
        elif control == "timeout":
            command("job step 0", limit=limit)
            if not session.operation.complete:
                sleep(min(0.010, max(0, limit - clock())))
        for _ in range(512):
            if session.operation.complete:
                break
            job = _payload(command("job step 1", limit=limit), "JOB")
            if int(job["last_poll_callbacks"]) > 1:
                raise PlanFailure("fixture poll exceeded one callback")
            if not session.operation.complete:
                remaining = min(limit, end_by()) - clock()
                if remaining <= 0:
                    raise PlanFailure("fixture operation deadline expired")
                sleep(min(0.005, remaining))
        if not session.operation.complete:
            raise PlanFailure("fixture operation exceeded 512 finite turns")
        result = session.wait_operation(min(limit, end_by()))
        detail = dict(session.operation.records["RESULT_DETAIL"])
        taken = session.take_result(min(limit, end_by()))
        if taken.end["code"] != "OK":
            raise PlanFailure("result take rejected; owned evidence retained without retry")
        summary["operations"] += 1
        artifacts.metadata["last_fixture_result"] = {"result": result, "detail": detail}
        if label:
            artifacts.metadata.setdefault("recovery_results", {})[label] = {
                "expected_code": expected_code, "result": result, "detail": detail}
        artifacts.checkpoint()
        if result["code"] != expected_code:
            raise PlanFailure(f"{text}: {result['code']} detail={result['detail']} "
                              f"effect={result['effect']}")
        if control and (result["effect"] != "NONE" or result["callbacks"] != "0"):
            raise PlanFailure("pre-callback cancellation/timeout has unexpected possible effects")
        if not mutation and result["effect"] != "NONE":
            raise PlanFailure("observation reported an unexpected device effect")
        if mutation and expected_code == "OK" and (result["effect"] not in {"NONE", "CONFIRMED"} or
                         text != "apply" and result["effect"] != "CONFIRMED"):
            raise PlanFailure("successful mutation lacks confirmed effect evidence")
        if text == "reset" and expected_code == "OK" and detail["reset"] != "COMPLETED":
            raise PlanFailure("successful reset lacks proved completion")
        return result, detail

    def profiles():
        """Require complete known actual fields; raw/reserved evidence is retained on failure."""
        response = command("profile show")
        config = _payload(response, "CONFIG")
        actual = _payload(response, "PROFILE", "observed")
        artifacts.metadata["fixture_profile"] = {"config": config, "observed": actual}
        # Public CONFIG validity bits back the same observed profile: 08/09/0A,
        # and 45 when DISABLE_RD_RP is applicable. Unknown raw bytes cannot
        # support a supposedly complete actual image without those observations.
        required_raw = 0x07 | (0x08 if fields & 128 else 0)
        if (config["configuration"] != "KNOWN" or config["has_observed"] != "1" or
                int(config["observed_valid_fields"]) != int(actual["valid_fields"]) or
                int(config["raw_valid_mask"]) & required_raw != required_raw or
                any(config[key] == "unknown" for key, bit in (
                    ("raw08", 1), ("raw09", 2), ("raw0a", 4), ("raw45", 8))
                    if required_raw & bit) or
                actual["available"] != "1" or actual["variant"] != plan.variant or
                int(actual["valid_fields"]) & fields != fields or
                any(actual[key] == "unknown" for key, (_, bit) in PROFILE_COMMANDS.items()
                    if fields & bit)):
            raise PlanBlocked("complete known actual profile required; no defaults substituted")
        if not _controls_consistent(actual, fields):
            raise PlanBlocked("contradictory observed termination/accessory controls")
        return actual

    def sample(expect=None, extra_mask=0):
        """Refresh once; require coherent advancing fields before interpreting attachment."""
        nonlocal sequence, observation_time
        before = _payload(command("sample"), "PORT")
        operation("read")
        response = command("sample")
        port = _payload(response, "PORT")
        detail = _payload(response, "PORT_DETAIL")
        artifacts.metadata["fixture_port"] = {"port": port, "detail": detail}
        mask = 7 | extra_mask
        if (port["invalidated"] != "0" or int(port["raw_valid_mask"]) != 3 or
                port["raw08"] == "unknown" or port["raw09"] == "unknown" or
                port["observed_time_valid"] != "1" or
                int(port["valid_fields"]) & mask != mask or
                detail["coherence"] != "BOOKENDS_MATCH" or
                int(port["sequence"]) <= max(sequence, int(before["sequence"])) or
                int(port["sequence"]) == (1 << 64) - 1 or
                observation_time is not None and
                (int(port["observed_ms"]) - observation_time) % (1 << 64) >= 1 << 63):
            raise PlanFailure("fixture port lacks current coherent valid observation")
        roles = {"DETACHED": ("NOT_APPLICABLE", "NOT_APPLICABLE"),
                 "ACCESSORY": ("NOT_APPLICABLE", "NOT_APPLICABLE"),
                 "LOCAL_SOURCE": ("SOURCE", "SINK"), "LOCAL_SINK": ("SINK", "SOURCE")}
        if roles.get(port["attachment"]) != (port["local_role"], port["peer_role"]):
            raise PlanFailure("fixture local/peer role evidence inconsistent")
        sequence, observation_time = int(port["sequence"]), int(port["observed_ms"])
        for key, value in (expect or {}).items():
            if key == "irq":
                actual = detail["interrupt_pending"]
            elif key in detail:
                actual = detail[key]
            else:
                actual = port[key]
            if actual != value:
                raise PlanFailure(f"fixture {key}: expected {value}, observed {actual}")
        artifacts.checkpoint()
        return port, detail

    def physical(name, instruction, expect=None, mask=0, partner=None, *, observe=True,
                 external=False):
        """Checkpoint a bounded operator transition, then compare fresh device evidence.

        Ready attests the exact declared target, wiring, unchanged measured supply,
        named partner and requested physical observation. Firmware readings alone
        cannot establish those external facts. No operation is parked during input.
        """
        nonlocal dirty
        if session.operation is not None:
            raise PlanFailure("fixture transition cannot abandon an owned result")
        if session.valid:
            target()
        step = {"id": name, "instruction": instruction, "fixture": setup["id"],
                "partner": partner or setup["partner"], "supply_mv": setup["supply_mv"],
                "expect": expect or {}}
        if external:
            dirty = True
            artifacts.metadata["device_cleanup_required"] = True
            artifacts.metadata["external_intent"] = name
            if name == "fault-power-cycle" and plan.variant in BASE_VARIANTS:
                # B SLLSEN9F section 7.4.3 p14: VDD return revokes the old Rp
                # recovery proof, including across later sessions in this run.
                artifacts.metadata["base_recovery_required"] = True
        artifacts.metadata["fixture_step"] = dict(step, acknowledged=False)
        artifacts.checkpoint()
        limit = min(deadline, clock() + min(300, plan.run_timeout))
        try:
            ready = transition(step, limit)
        except (TimeoutError, ValueError) as error:
            raise PlanFailure("invalid/expired fixture attestation: " + str(error)) from error
        if type(ready) is not bool:
            raise PlanFailure("fixture transition must return an explicit boolean attestation")
        end_by(limit)  # Late input never establishes proof or becomes a timely missing fixture.
        if not ready:
            raise FixtureNotRun("fixture transition unavailable: " + name)
        if session.valid:
            target()
        summary["transitions"] += 1
        artifacts.metadata["fixture_step"] = dict(step, acknowledged=True)
        artifacts.checkpoint()
        if len(evidence) >= 64:
            raise PlanFailure("fixture transition evidence exceeds finite 64-step limit")
        if not observe:
            evidence[name] = {"attestation": step}
            artifacts.checkpoint()
            return None
        port, detail = sample(expect, mask)
        evidence[name] = {"attestation": step, "port": port, "detail": detail}
        artifacts.checkpoint()
        return port, detail

    def stage(values):
        """Stage a complete proved candidate and check the reported values before applying it."""
        for key, (route, bit) in PROFILE_COMMANDS.items():
            if fields & bit and key in values:
                command(route + " " + values[key])
        staged = _payload(command("profile validate"), "PROFILE", "staged")
        if (staged["available"] != "1" or staged["variant"] != plan.variant or
                int(staged["valid_fields"]) & fields != fields or
                not _controls_consistent(staged, fields) or
                any(staged[key] != values[key] for key, (_, bit) in PROFILE_COMMANDS.items()
                    if fields & bit)):
            raise PlanFailure("validated CLI staging differs from the intended candidate")

    def apply(values, attachment="DETACHED"):
        """Refresh prerequisites immediately before one explicit apply and verify operation."""
        target()
        sample({"attachment": attachment})
        candidate = dict(profiles(), **values)
        if (plan.variant in BASE_VARIANTS and candidate["mode"] != "ufp" and
                artifacts.metadata.get("base_recovery_required")):
            raise PlanBlocked("new original-part VDD epoch requires explicit qualified recovery "
                              "before source-capable settings; no old readiness replay")
        # SLLSEN9F 6.5 p7 / SLLSEQ8D 6.5 p5, footnote 1: local 3 A
        # needs VDD >=3.5 V. This applies to every source-capable candidate,
        # including baseline restoration; remote sink detection is independent.
        if (candidate["mode"] != "ufp" and candidate["current"] == "3a0" and
                setup["supply_mv"] < 3500):
            raise PlanBlocked("source-capable local 3 A requires measured VDD >=3500 mV")
        stage(candidate)
        operation("apply", True)
        operation("verify")
        actual = profiles()
        if any(actual[key] != candidate[key] for key, (_, bit) in PROFILE_COMMANDS.items()
               if fields & bit):
            raise PlanFailure("verified actual profile differs from requested settings")

    def detached(name):
        """Require an explicitly disconnected connector before sequenced role work."""
        physical(name, "Disconnect the Type-C fixture; keep target and supply unchanged.",
                 {"attachment": "DETACHED"})

    def orientation():
        """Prove both chip CC inputs with the same identified reversible source partner."""
        apply(dict(baseline, mode="ufp", preference="normal"))
        for direction in ("CC1", "CC2"):
            physical("orientation-" + direction.lower(),
                     "Connect the reversible source partner in " + direction +
                     "; confirm connector mapping and externally observed local sink/peer source.",
                     {"attachment": "LOCAL_SINK", "orientation": direction}, 8)
        detached("orientation-detach")

    def accessory():
        """Observe every explicitly declared supported accessory; retain unknown raw evidence."""
        for fixture, value in ACCESSORIES.items():
            if fixture not in setup["capabilities"]:
                continue
            mode = "ufp" if fixture == "debug-local-ufp" else "dfp"
            apply(dict(baseline, mode=mode, preference="normal", ufpaccessory="on"))
            if fixture == "debug" and plan.variant not in BASE_VARIANTS:
                value = "DEBUG_LOCAL_DFP"
            physical("accessory-" + fixture,
                     "Connect the exact declared " + fixture + " accessory fixture; "
                     "confirm its wiring and supported local/peer interpretation.",
                     {"attachment": "ACCESSORY", "accessory": value}, 64)
            detached("accessory-detach-" + fixture)

    def role_current():
        """Exercise sink detection, source advertisement and resolved DRP without power policy."""
        apply(dict(baseline, mode="ufp", preference="normal"))
        for index, (category, detected) in enumerate(CURRENT.items()):
            # B SLLSEN9F section 7.3.1.2 p11 requires explicit software reset to
            # update attached-current detection. No readonly/soak helper does this.
            expect = {"attachment": "LOCAL_SINK"}
            if plan.variant not in BASE_VARIANTS or index == 0:
                expect["detected_current"] = detected
            physical("sink-" + category,
                     "Keep the SAME source partner attached; set external CC advertisement to " +
                     category + "; confirm category with the fixture, not measured load/PD.",
                     expect, 16)
            if plan.variant in BASE_VARIANTS and index:
                reset_result, reset_detail = operation("reset", True)
                target()
                operation("reconcile")
                profiles()
                port, detail = physical("sink-settled-" + category,
                                        "After the explicit reset, keep the SAME source partner "
                                        "connected; wait for physically stable sink/current "
                                        "conditions, then confirm external CC " + category + ".",
                                        {"attachment": "LOCAL_SINK",
                                         "detected_current": detected}, 16)
                evidence["sink-" + category]["after_reset"] = {
                    "result": reset_result, "reset_detail": reset_detail,
                    "port": port, "detail": detail}
                artifacts.checkpoint()
        detached("sink-detach")
        apply(dict(baseline, mode="dfp", preference="normal"))
        physical("source-connect", "Connect the declared sink fixture; confirm local source "
                 "and fixture-owned safe VBUS conditions. No power output is controlled here.",
                 {"attachment": "LOCAL_SOURCE"})
        for category in CURRENT:
            apply({"current": category}, "LOCAL_SOURCE")
            physical("source-" + category,
                     "Observe local CC advertisement " + category +
                     " with the sink fixture. Supply must still match the declared measured VDD.",
                     {"attachment": "LOCAL_SOURCE", "advertised_current": category}, 32)
        detached("source-detach")
        apply(dict(baseline, mode="drp", preference="normal"))
        physical("drp-resolved", "Connect the declared source partner and confirm DRP resolves "
                 "as local sink; DRP is a configuration, not an attachment role.",
                 {"attachment": "LOCAL_SINK", "orientation": "CC1"}, 8)
        detached("drp-detach")

    def pin(value):
        """Read only the application opt-in INT input; low is asserted on these parts."""
        row = _payload(command("intpin"), "PIN")
        if row["available"] != "1":
            raise FixtureNotRun("wired opt-in application INT input unavailable")
        if row["value"] != value:
            raise PlanFailure("physical INT level does not match declared fixture event")

    def interrupt():
        """Prove applied-versus-staged ACK preservation, pending writes and bounded service."""
        apply(dict(baseline, mode="drp", duty="50", preference="normal"))
        physical("irq-assert", "Trigger a connector event and observe wired active-low INT.",
                 {"attachment": "LOCAL_SINK", "irq": "1"}, 256)
        pin("0")
        stage(dict(profiles(), duty="30"))
        _, acknowledged = operation("irq ack", True)
        if acknowledged["ack_attempted"] != "1" or acknowledged["ack_confirmed"] != "1":
            raise PlanFailure("IRQ ACK lacks explicit accepted-write evidence")
        operation("profile read")
        if profiles()["duty"] != "50":
            raise PlanFailure("IRQ ACK replayed staged duty instead of actual 50%")
        pin("1")
        physical("irq-reassert", "Trigger another event and observe INT reassertion; "
                 "keep the same partner, local role and supply.",
                 {"attachment": "LOCAL_SINK", "irq": "1"}, 256)
        pin("0")
        apply({"duty": "30"}, "LOCAL_SINK")
        sample({"attachment": "LOCAL_SINK", "irq": "1"}, 256)
        pin("0")
        physical("irq-coalesced", "Trigger at least two declared connector events while INT "
                 "is already low; confirm the same final attachment and a coalesced pending latch. "
                 "This is not lossless edge-count evidence.",
                 {"attachment": "LOCAL_SINK", "irq": "1"}, 256)
        result, detail = operation("irq service", True)
        if (detail["irq_observed"] != "1" or detail["irq_before"] != "1" or
                detail["ack_attempted"] != "1" or detail["ack_confirmed"] != "1" or
                detail["irq_after_valid"] != "1" or detail["irq_after"] != "0"):
            raise PlanFailure("bounded IRQ service lacks observed/ACK/recheck evidence")
        pin("1")
        artifacts.metadata["irq_service"] = {"result": result, "detail": detail}
        detached("irq-detach")

    def reconcile():
        """Choose an exact read-only comparison outcome from complete observed evidence."""
        response = command("profile show")
        config = _payload(response, "CONFIG")
        actual = _payload(response, "PROFILE", "observed")
        desired = _payload(response, "PROFILE", "desired")
        diag = _payload(command("diag"), "DIAG")
        # Acceptance history is retained independently: a complete reconcile can
        # prove current configuration while reset_acceptance_uncertain stays true.
        unresolved = diag["recovery_unresolved"] == "1"
        expected = "CONFIGURATION_UNKNOWN" if unresolved else "OK"
        if not unresolved and config["has_desired"] == "1" and any(
                actual[key] != desired[key] for key, (_, bit) in PROFILE_COMMANDS.items()
                if fields & bit):
            expected = "CONFIGURATION_MISMATCH"
        return operation("reconcile", expected_code=expected, label="reconcile")

    def unchanged_after_raw(require_unknown=False):
        """A successful unrelated identity-byte read cannot repair trust or cached freshness."""
        before = _payload(command("profile show"), "CONFIG")
        port = _payload(command("sample"), "PORT")
        command("reg 0")
        after = _payload(command("profile show"), "CONFIG")
        copied = _payload(command("sample"), "PORT")
        if (before["configuration"] != after["configuration"] or
                require_unknown and after["configuration"] != "UNKNOWN" or
                any(port[key] != copied[key] for key in (
                    "sequence", "invalidated", "raw_valid_mask", "observed_ms"))):
            raise PlanFailure("unrelated raw success repaired trust or cached freshness")

    def reset_termination():
        """Use explicit reset and independent controls; adverse writes stop without replay."""
        saved = command("profile show")
        desired = _payload(saved, "PROFILE", "desired")
        config = _payload(saved, "CONFIG")
        before = _payload(command("bus"), "BUS")
        result, detail = operation("reset", True, label="reset")
        after_bus = _payload(command("bus"), "BUS")
        artifacts.metadata["reset_adapter"] = {"before": before, "after": after_bus}
        if before["adapter_available"] == "1" and (
                after_bus["adapter_available"] != "1" or
                int(after_bus["adapter_writes"]) >= (1 << 32) - 1 or
                int(after_bus["adapter_writes"]) - int(before["adapter_writes"]) != 1):
            raise PlanFailure("reset adapter evidence shows extra writes or saturated counters")
        elapsed = (int(detail["completed_ms"]) - int(detail["started_ms"])) % (1 << 64)
        if detail["completed_time_valid"] != "1" or not 96 <= elapsed < 1 << 63:
            raise PlanFailure("reset lacks fresh completion and 95ms plus clock-margin evidence")
        after = command("profile show")
        if (_payload(after, "CONFIG")["desired_generation"] != config["desired_generation"] or
                any(_payload(after, "PROFILE", "desired")[key] != desired[key]
                    for key in SCHEMAS["PROFILE"] if key not in {"nonce", "request"})):
            raise PlanFailure("reset changed retained desired settings or replayed ownership")
        unchanged_after_raw()
        reconcile()
        for control, code in (("cancel", "CANCELLED"), ("timeout", "TIMEOUT")):
            operation("reset", True, expected_code=code, control=control,
                      label="reset-" + control)
        # 0A DISABLE_TERM and 45 DISABLE_RD_RP are independent (SLLSEQ8D Tables10/11).
        controls = [key for key in ("termination", "rd_rp")
                    if fields & PROFILE_COMMANDS[key][1]]
        for selected in controls:
            candidate = dict(profiles(), **{key: "off" if key == selected else "on"
                                           for key in controls})
            apply(candidate)
            detail = _payload(command("profile show"), "CONFIG_DETAIL")
            if detail["termination"] != "DISABLED":
                raise PlanFailure("one enabled control concealed a separate disabled termination")
        apply(dict(profiles(), **{key: "on" for key in controls}))
        if _payload(command("profile show"), "CONFIG_DETAIL")["termination"] != "ENABLED":
            raise PlanFailure("enabled controls do not prove overall termination")
        if plan.variant not in BASE_VARIANTS:
            before = _payload(command("xfer_stats"), "XFER")
            denied = session.command("recover-dead-battery", end_by())
            if denied.ack["code"] != "UNSUPPORTED_FEATURE" or denied.admitted:
                raise PlanFailure("later variant admitted original-part recovery")
            after = _payload(command("xfer_stats"), "XFER")
            if before["attempts"] != after["attempts"]:
                raise PlanFailure("unsupported recovery invoked transport")
        detached("reset-detach")

    def fault():
        """Observe explicitly induced failures and power loss without retry or profile replay."""
        for name, setting in (("nack", "nack_code"), ("disconnect", "disconnect_code")):
            physical("fault-" + name, "Perform only the separately authorized " + name +
                     " fixture action; keep the declared target/partner/supply assumptions.",
                     observe=False, external=True)
            before = _payload(command("xfer_stats"), "XFER")
            result, _ = operation("profile read", expected_code=setup[setting], label=name)
            after = _payload(command("xfer_stats"), "XFER")
            if (result["effect"] != "NONE" or int(after["failures"]) <= int(before["failures"]) or
                    after["saturated"] != "0"):
                raise PlanFailure("fault lacks failed callback accounting and no-write evidence")
            command("diag")  # Cached owner remains responsive; no device retry.
            command("invalidate target-change")
            physical("fault-" + name + "-restore", "Remove the fault explicitly. Reconfirm "
                     "the exact target, same partner and measured supply; no retry is automatic.",
                     observe=False)
            unchanged_after_raw(True)
            target()
            operation("init", label=name + "-identity")
            reconcile()
        command("invalidate power-loss")
        physical("fault-power-cycle", "Perform the separately authorized target power cycle. "
                 "Prevent back-powering through pins; retain required VBUS_DET sensing. "
                 "Return only after declared supply/startup readiness and same partner are proved.",
                 observe=False, external=True)
        unchanged_after_raw(True)
        target()
        operation("init", label="power-cycle-identity")
        reconcile()
        if plan.variant in BASE_VARIANTS and profiles()["mode"] != "ufp":
            raise PlanBlocked("new original-part source-capable VDD epoch needs explicit owner "
                              "recovery; prior startup history cannot authorize cleanup")
        # A new power epoch cannot authorize replay of the old source role.
        detached("fault-detach")

    def console():
        """Test bounded output and explicit new correlation domains after reconnect/reboot."""
        nonlocal session, sequence, observation_time
        if reconnect is None:
            raise FixtureNotRun("console suite needs an explicit transport reconnect owner")
        before = _payload(command("console"), "CONSOLE")
        physical("console-backpressure", "Arm the authorized finite USB/serial backpressure "
                 "fixture for the next help response. Release within the command deadline; "
                 "do not reset, power-cycle or change the target.", observe=False)
        command("help")
        after = _payload(command("console"), "CONSOLE")
        artifacts.metadata["console_backpressure"] = {"before": before, "after": after}
        if (int(after["dropped_records"]) < int(before["dropped_records"]) or
                after["loss_pending"] != "0" or
                after["dropped_records"] != before["dropped_records"] or
                after["dropped_bytes"] != before["dropped_bytes"]):
            raise PlanFailure("console output loss invalidated the experiment")
        for action in ("console-reconnect", "console-reboot"):
            old = session
            identity = old.identity
            nonce = old.nonce
            old.finish(end_by())  # Restore local state before intentional transport loss.
            old_nonces = {nonce, old.nonce, old._begin_nonce}
            physical(action, "Perform only the authorized " + action +
                     "; restore the same port/firmware/target and declared supply/partner. "
                     "For original parts, separately verify target VDD stayed continuous; "
                     "Old requests/results are invalid; no hardware profile is restored.",
                     observe=False, external=True)
            candidate = reconnect(old, end_by())
            end_by()
            if (not isinstance(candidate, type(old)) or candidate is old or
                    candidate.artifacts is not artifacts or candidate.operation or
                    candidate.valid or candidate._started or candidate._changed or
                    candidate.last_request or candidate.last_token):
                raise PlanFailure("reconnect must return a fresh unstarted Session")
            session = candidate
            session.begin(expected, end_by())
            if session.nonce in old_nonces or session.identity != identity:
                raise PlanBlocked("reconnect did not establish fresh framing and same identity")
            target()
            artifacts.metadata.setdefault("console_sessions", {})[action] = {
                "old_request": old.last_request, "new_request": session.last_request,
                "old_nonce": nonce, "new_nonce": session.nonce, "identity_verified": True,
                "restored_old_local": old._local}
            command("job auto 0")
            command("color off")
            command("verbose 1")
            if _payload(command("bus"), "BUS")["bound"] == "0":
                command("bind")
            operation("init", label=action + "-identity")
            profiles()
            sequence, observation_time = -1, None  # Observation domain belongs to the new boot.
        detached("console-detach")

    try:
        fixture_prerequisite(plan, setup, expected, base_recovery_required=(
            artifacts.metadata.get("base_recovery_required")))
        session.begin(expected, end_by())
        if session.identity is None or not session.identity.known:
            raise PlanBlocked("known firmware identity required for fixture actions")
        bus = target()
        capabilities = _payload(command("capabilities"), "CAPABILITIES")
        fields = int(capabilities["profile_fields"])
        if not fields or fields & ~255:
            raise PlanBlocked("applicable profile fields unavailable")
        if "interrupt" in plan.suites and capabilities["duty_write"] != "1":
            raise PlanBlocked("duty write capability required for IRQ preservation plan")
        if "role-current" in plan.suites and capabilities["advertisement_write"] != "1":
            raise PlanBlocked("advertisement write capability required")
        if "interrupt" in plan.suites:
            input_response = session.command("intpin", end_by())
            input_level = _payload(input_response, "PIN")
            if input_level["available"] != "1" and input_response.end["code"] in {
                    "OK", "UNSUPPORTED_FEATURE", "INVALID_CONFIG"}:
                raise FixtureNotRun("wired opt-in application INT input unavailable")
            if input_response.end["code"] != "OK":
                raise PlanFailure("INT application reader failed before fixture work")
        command("job auto 0")
        command("color off")
        command("verbose 1")
        if bus["bound"] == "0":
            command("bind")
        operation("init")
        if plan.variant in BASE_VARIANTS and "reset-termination" in plan.suites:
            # Save actual evidence even if it is incomplete. A later known cleanup
            # baseline must not replace the facts that preceded the recovery writes.
            artifacts.metadata["fixture_baseline"] = {
                "target": bus, "supply_mv": setup["supply_mv"], "partner": setup["partner"],
                "profiles": [dict(row) for row in command("profile show").records
                             if row["type"] in {"CONFIG", "CONFIG_DETAIL", "PROFILE"}]}
            artifacts.metadata["pre_recovery_baseline"] = artifacts.metadata["fixture_baseline"]
            artifacts.checkpoint()
            physical("recovery-ready", "Confirm stable VDD after return and the separately "
                     "selected source-capable startup need for original-part recovery. "
                     "Keep this experiment detached. No board power change is implied.",
                     {"attachment": "DETACHED"})
            _, detail = operation("recover-dead-battery", True, label="dead-battery")
            elapsed = (int(detail["completed_ms"]) - int(detail["started_ms"])) % (1 << 64)
            diag = _payload(command("diag"), "DIAG")
            if (detail["completed_time_valid"] != "1" or not 31 <= elapsed < 1 << 63 or
                    detail["termination"] != "ENABLED" or diag["recovery_unresolved"] != "0"):
                raise PlanFailure("recovery lacks qualified sequence/wait/termination evidence")
            artifacts.metadata["base_recovery_required"] = False
            artifacts.checkpoint()
            reconcile()
        baseline = dict(profiles())
        artifacts.metadata["fixture_baseline"] = {
            "profile": baseline, "target": bus, "supply_mv": setup["supply_mv"],
            "partner": setup["partner"],
            "profiles": [dict(row) for row in command("profile show").records
                         if row["type"] in {"CONFIG", "CONFIG_DETAIL", "PROFILE"}]}
        artifacts.metadata.setdefault("fixture_baselines", {})[group_name] = (
            artifacts.metadata["fixture_baseline"])
        if any(baseline[key] != "on" for key in ("termination", "rd_rp")
               if fields & PROFILE_COMMANDS[key][1]):
            raise PlanBlocked("fixture plans require proved enabled terminations")
        detached("initial-detached")
        for suite in plan.suites:
            current_suite = suite
            if suite not in FIXTURE_SUITES:
                continue  # The command-line owner runs observations in separate verified sessions.
            {"orientation": orientation, "accessory": accessory,
             "role-current": role_current, "interrupt": interrupt,
             "reset-termination": reset_termination, "fault": fault, "console": console}[suite]()
            artifacts.check(suite, "PASS", "declared physical transitions and typed evidence")
            artifacts.checkpoint()
        current_suite = "fixture_cleanup"
        detached("cleanup-detached")
        target()
        profiles()
        # No restoration follows a failed/uncertain mutation. Even on success,
        # source-capable settings are restored only at proved detached attachment,
        # with the original target/partner/supply explicitly attested again.
        if dirty and not set(plan.suites) & {"fault", "console"}:
            apply(baseline)
        if _payload(command("profile show"), "CONFIG_DETAIL")["termination"] != "ENABLED":
            raise PlanBlocked("cleanup cannot establish overall enabled terminations")
        hardware_cleanup = "PASS"
        cleanups[group_name] = hardware_cleanup
        artifacts.metadata["hardware_cleanup"] = hardware_cleanup
        artifacts.metadata["device_cleanup_required"] = False
        artifacts.check("fixture_cleanup", "PASS", "known enabled controls at attested detach; "
                        "no old-role replay after power/reboot")
        session.finish(end_by())
        end_by()
        artifacts.cleanup = "PASS"
        artifacts.primary = "PASS"
        artifacts.checkpoint()
        end_by()
        return dict(summary)
    except BaseException as error:
        status = "NOT RUN" if isinstance(error, FixtureNotRun) else (
            "FAIL" if getattr(error, "exit_code", 2) == 1 else "BLOCKED")
        artifacts.primary = "FAIL" if status == "FAIL" else "BLOCKED"
        artifacts.interrupted |= isinstance(error, KeyboardInterrupt)
        artifacts.check(current_suite, status, str(error) or type(error).__name__)
        if dirty and hardware_cleanup != "PASS":
            artifacts.metadata["hardware_cleanup"] = "BLOCKED"
            cleanups[group_name] = "BLOCKED"
            if current_suite != "fixture_cleanup" or status != "FAIL":
                artifacts.check("fixture_cleanup", "BLOCKED", "no device restore after failure")
            artifacts.residual_uncertainty.append(
                "Fixture effects remain; no role/apply/reset/ACK retry or blind restoration.")
            artifacts.metadata["manual_reconciliation"] = (
                "Keep the exact target/firmware/partner and measured supply recorded. Inspect "
                "diag, profile show, sample and the retained owned token before new work. "
                "After physical readiness, choose explicit init/reconcile; observe both 0A "
                "disableTerm and 45 disableRdRp where present. Cleared bits do not prove a "
                "completed historical reset/recovery. Do not replay an old source role.")
        primary = artifacts.primary
        if session.valid and session.operation is None:
            try:
                session.finish(end_by())  # Local staging/auto/output only, never hardware apply.
            except BaseException as cleanup_error:
                artifacts.interrupted |= isinstance(cleanup_error, KeyboardInterrupt)
                artifacts.metadata["fixture_cleanup_error"] = str(cleanup_error)[:1024]
                session.invalidate("fixture local cleanup incomplete")
                artifacts.cleanup = "FAIL"
        else:
            session.invalidate("fixture stopped; owned/uncertain operation is not retried")
        if dirty and hardware_cleanup != "PASS":
            artifacts.cleanup = "BLOCKED"  # Local success cannot certify device cleanup.
        artifacts.primary = primary
        artifacts.checkpoint()
        raise
