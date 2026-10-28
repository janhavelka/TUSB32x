"""Finite physical-fixture plans using the existing Session and ordinary CLI operations.

Operator confirmations are prerequisites, not measured electrical evidence. Saved
observations retain both the attestation and actual typed firmware records. No
GPIO, power control, register protocol, retry or replacement result store lives here.
The caller owns serial teardown; all waits include the immutable whole-run deadline.
"""

import math
import time

from tusb32x_hil_plans import BASE_VARIANTS, PlanBlocked, PlanFailure, _payload
from tusb32x_hil_fixture_setup import FIXTURE_SUITES, console_transition


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


class FixtureNotRun(PlanBlocked):
    """Missing fixture/operator evidence prevents execution; NOT RUN, CLI exit 2."""


def fixture_prerequisite(plan, setup, expected):
    """Reject missing physical setup and closed variant gates before serial/device traffic."""
    if not setup:
        raise FixtureNotRun("explicit --fixture and --fixture-config required")
    if not expected or not {"git", "framework", "target"} <= expected.keys():
        raise PlanBlocked("fixture plans require expected flashed commit/framework/target")
    if setup["variant"] != plan.variant:
        raise PlanBlocked("fixture setup variant differs from selected exact part")
    if plan.variant == "tusb322i":
        raise PlanBlocked("TUSB322I fixture writes/return-to-DRP remain gated; "
                          "advertisement/09/fault-clear evidence is unresolved")
    for suite in plan.suites:
        required = {suite} if suite != "accessory" else set(ACCESSORIES)
        if suite in FIXTURE_SUITES and not required & set(setup["capabilities"]):
            raise FixtureNotRun("declared fixture capability absent: " + suite)
    if (plan.variant in BASE_VARIANTS and "accessory" in plan.suites and
            set(setup["capabilities"]) & {"debug-local-dfp", "debug-local-ufp"}):
        raise PlanBlocked("selected base variant does not support declared extended accessory")
    if not setup.get("startup_ready") or (plan.variant in BASE_VARIANTS and
                                          not setup.get("base_recovery_complete")):
        raise PlanBlocked("owner supply/startup readiness and base recovery proof required")


def run_fixture_plan(session, plan, setup, expected=None, *, transition=console_transition,
                     clock=time.monotonic, sleep=time.sleep):
    """Execute selected fixture suites, consuming each exact result once and saving outcomes.

    ``setup`` is validated by load_setup before opening. ``transition(step, deadline)``
    returns True only after explicit physical setup/observation, False for unavailable
    evidence; injected tests model these observations and never qualify hardware.
    No mutation is retried. Device restoration requires a successful primary plan,
    a fresh detached fixture attestation, unchanged identity/target/supply, coherent
    port data and known actual settings. A failed/uncertain plan performs no device
    restoration; explicit later owner work must resolve its residual effects.
    """
    artifacts = session.artifacts
    deadline = clock() + plan.run_timeout
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

    def end_by():
        """Check monotonic time and clip exchanges/confirmations to one whole-run bound."""
        nonlocal previous_time
        now = clock()
        if not math.isfinite(now) or now < previous_time or now >= deadline:
            raise PlanFailure("fixture whole-run deadline expired or clock regressed")
        previous_time = now
        return min(deadline, now + plan.command_timeout)

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

    def operation(text, mutation=False):
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
            wire += " " + str(max(1, math.ceil((limit - clock()) * 1000)))
        admitted = session.command(wire, limit)
        if not admitted.admitted:
            raise PlanFailure(f"{text} admission: {admitted.ack['code']}")
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
        artifacts.checkpoint()
        if result["code"] != "OK":
            raise PlanFailure(f"{text}: {result['code']} detail={result['detail']} "
                              f"effect={result['effect']}")
        if not mutation and result["effect"] != "NONE":
            raise PlanFailure("observation reported an unexpected device effect")
        if mutation and (result["effect"] not in {"NONE", "CONFIRMED"} or
                         text != "apply" and result["effect"] != "CONFIRMED"):
            raise PlanFailure("successful mutation lacks confirmed effect evidence")
        if text == "reset" and detail["reset"] != "COMPLETED":
            raise PlanFailure("successful reset lacks proved completion")
        return result, detail

    def profiles():
        """Require complete known actual fields; raw/reserved evidence is retained on failure."""
        response = command("profile show")
        config = _payload(response, "CONFIG")
        actual = _payload(response, "PROFILE", "observed")
        artifacts.metadata["fixture_profile"] = {"config": config, "observed": actual}
        if (config["configuration"] != "KNOWN" or config["has_observed"] != "1" or
                actual["available"] != "1" or actual["variant"] != plan.variant or
                int(actual["valid_fields"]) & fields != fields or
                any(actual[key] == "unknown" for key, (_, bit) in PROFILE_COMMANDS.items()
                    if fields & bit)):
            raise PlanBlocked("complete known actual profile required; no defaults substituted")
        for key, raw in (("ufpaccessory", "disable_ufp_accessory"),
                         ("termination", "disable_term"), ("rd_rp", "disable_rd_rp")):
            if fields & PROFILE_COMMANDS[key][1] and actual[raw] != (
                    "0" if actual[key] == "on" else "1"):
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

    def physical(name, instruction, expect, mask=0, partner=None):
        """Checkpoint a bounded operator transition, then compare fresh device evidence.

        Ready attests the exact declared target, wiring, unchanged measured supply,
        named partner and requested physical observation. Firmware readings alone
        cannot establish those external facts. No operation is parked during input.
        """
        if session.operation is not None:
            raise PlanFailure("fixture transition cannot abandon an owned result")
        target()
        step = {"id": name, "instruction": instruction, "fixture": setup["id"],
                "partner": partner or setup["partner"], "supply_mv": setup["supply_mv"],
                "expect": expect}
        artifacts.metadata["fixture_step"] = dict(step, acknowledged=False)
        artifacts.checkpoint()
        limit = min(deadline, clock() + min(300, plan.run_timeout))
        try:
            ready = transition(step, limit)
        except (TimeoutError, ValueError) as error:
            raise PlanFailure("invalid/expired fixture attestation: " + str(error)) from error
        if type(ready) is not bool:
            raise PlanFailure("fixture transition must return an explicit boolean attestation")
        if not ready:
            raise FixtureNotRun("fixture transition unavailable: " + name)
        end_by()  # Late input never establishes proof.
        target()
        summary["transitions"] += 1
        artifacts.metadata["fixture_step"] = dict(step, acknowledged=True)
        artifacts.checkpoint()
        port, detail = sample(expect, mask)
        if len(evidence) >= 64:
            raise PlanFailure("fixture transition evidence exceeds finite 64-step limit")
        evidence[name] = {"attestation": step, "port": port, "detail": detail}
        artifacts.checkpoint()
        return port, detail

    def stage(values):
        """Stage only applicable fields from a proved profile, never retained desired defaults."""
        for key, (route, bit) in PROFILE_COMMANDS.items():
            if fields & bit and key in values:
                command(route + " " + values[key])
        command("profile validate")

    def apply(values, attachment="DETACHED"):
        """Refresh prerequisites immediately before one explicit apply and verify operation."""
        target()
        sample({"attachment": attachment})
        candidate = dict(profiles(), **values)
        # SLLSEN9F 6.5 p7 / SLLSEQ8D 6.5 p5, footnote 1: local 3 A
        # needs VDD >=3.5 V. This applies to every source-capable candidate,
        # including baseline restoration; remote sink detection is independent.
        if (candidate["mode"] != "ufp" and candidate["current"] == "3a0" and
                setup["supply_mv"] < 3500):
            raise PlanBlocked("source-capable local 3 A requires measured VDD >=3500 mV")
        stage(values)
        operation("apply", True)
        operation("verify")
        actual = profiles()
        if any(actual[key] != value for key, value in values.items()
               if key in PROFILE_COMMANDS and fields & PROFILE_COMMANDS[key][1]):
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
        command("duty 30")
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

    try:
        fixture_prerequisite(plan, setup, expected)
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
             "role-current": role_current, "interrupt": interrupt}[suite]()
            artifacts.check(suite, "PASS", "declared physical transitions and typed evidence")
            artifacts.checkpoint()
        current_suite = "fixture_cleanup"
        detached("cleanup-detached")
        target()
        profiles()
        # No restoration follows a failed/uncertain mutation. Even on success,
        # source-capable settings are restored only at proved detached attachment,
        # with the original target/partner/supply explicitly attested again.
        if dirty:
            apply(baseline)
        hardware_cleanup = "PASS"
        cleanups[group_name] = hardware_cleanup
        artifacts.metadata["hardware_cleanup"] = hardware_cleanup
        artifacts.metadata["device_cleanup_required"] = False
        artifacts.check("fixture_cleanup", "PASS", "original profile verified at attested detach")
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
