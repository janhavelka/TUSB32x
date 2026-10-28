"""Exercise finite fixture plans through a real framed Session without physical hardware.

The peer models physical attachment and a coalescing latch separately from CLI
staging and applied settings. Tests inject faults at wire/operation boundaries;
neither Session guards nor production plan helpers are replaced.
"""

from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest

import tusb32x_hil_runner as runner
from tusb32x_hil_artifacts import Artifacts
from tusb32x_hil_plans import ObservationPlan, PlanBlocked, PlanFailure
from test_tusb32x_hil_runner import FakeClock, GIT, PlanPeer, record


EXPECTED = {"git": GIT, "framework": "host", "target": "native"}
CAPABILITIES = ["orientation", "audio", "debug", "charge-through", "debug-local-dfp",
                "debug-local-ufp", "role-current", "interrupt"]


def setup_for(variant="tusb320lai"):
    """Declare one explicit test fixture; these strings never qualify physical equipment."""
    return dict(id="modeled_fixture", board="host_model", part=variant, adapter="fake_serial",
                wiring="SDA SCL shared; INT input; CC reversible; VBUS_DET connected",
                addr_strap="0x60" if variant in {"tusb320", "tusb320i"} else "0x47",
                enable="asserted", supply_mv=5000, partner="controllable_CC_partner",
                operator="deterministic_test", variant=variant, capabilities=list(CAPABILITIES),
                startup_ready=True, base_recovery_complete=True)


class FixturePeer(PlanPeer):
    """Reactive wire target with independently controlled connector, current and IRQ state."""

    def __init__(self, clock):
        """Separate saved local staging from actual profile and physical observations."""
        super().__init__(clock)
        self.core_profile["duty"] = "50"
        self.applied_profile = None
        self.desired_profile = None
        self.desired_generation = 0
        self.applied_generation = 0
        self.attachment = "DETACHED"
        self.orientation = "NOT_APPLICABLE"
        self.remote_current = "NOT_APPLICABLE"
        self.pending_remote_current = None
        self.accessory = "NOT_APPLICABLE"
        self.irq_pending = False
        self.irq_available = True
        self.detail_fault = {}
        self.profile_fault = {}
        self.fail_operation = None
        self.hang_operation = None
        self.fail_at_callback = 1
        self.fail_code = "I2C_ERROR"
        self.reassert_service = False
        self.operation_history = []
        self.candidate_history = []
        self.mutation_history = []
        self.reset_count = 0
        self.success_effect = {}
        self.reset_evidence = "COMPLETED"
        self.profile_fields = 255

    def physical(self, attachment, *, orientation="CC1", current="DEFAULT", accessory="NONE",
                 irq=None):
        """Change modeled hardware as an external fixture action, never as a CLI side effect."""
        previous_attachment = self.attachment
        previous_current = self.remote_current
        self.attachment = attachment
        self.orientation = orientation if attachment not in {"DETACHED", "ACCESSORY"} else (
            "NOT_APPLICABLE" if attachment == "DETACHED" else "UNKNOWN")
        self.remote_current = current if attachment == "LOCAL_SINK" else "NOT_APPLICABLE"
        if (self.variant in {"tusb320", "tusb320i"} and attachment == "LOCAL_SINK" and
                previous_attachment == "LOCAL_SINK" and current != previous_current):
            self.pending_remote_current = current
            self.remote_current = previous_current
        self.accessory = "NOT_APPLICABLE" if attachment == "DETACHED" else accessory
        if attachment == "ACCESSORY":
            self.remote_current = {"CHARGE_THROUGH": "CHARGE_THROUGH_500_MA",
                                   "DEBUG_LOCAL_UFP": "UNKNOWN"}.get(accessory, "NOT_APPLICABLE")
        if irq is not None:
            self.irq_pending = bool(irq)

    def _profile(self, request):
        """Publish actual, desired and applied profiles without adopting unrelated staging."""
        raw09 = {"30": 0, "40": 2, "50": 4, "60": 6}[self.core_profile["duty"]]
        rows = [record("CONFIG", request, self.nonce, raw_valid_mask=15,
                       observed_valid_fields=self.profile_fields,
                       has_desired=int(self.desired_profile is not None),
                       has_applied=int(self.applied_profile is not None),
                       desired_generation=self.desired_generation,
                       applied_generation=self.applied_generation,
                       raw09=raw09 | (16 if self.irq_pending else 0)),
                record("CONFIG_DETAIL", request, self.nonce)]
        for source, values in (("staged", self.staged), ("desired", self.desired_profile),
                               ("observed", self.core_profile), ("applied", self.applied_profile)):
            available = values is not None
            values = values or self.core_profile
            fields = dict(values, source=source, variant=self.variant,
                          available=int(available), valid_fields=self.profile_fields,
                          generation=self.staged_generation if source == "staged" else
                          self.desired_generation if source == "desired" else
                          self.applied_generation if source == "applied" else 0,
                          disable_ufp_accessory=int(values["ufpaccessory"] == "off"),
                          disable_term=int(values["termination"] == "off"),
                          disable_rd_rp=int(values["rd_rp"] == "off"))
            fields.update(self.profile_fault.get(source, {}))
            rows.append(record("PROFILE", request, self.nonce, **fields))
        return rows

    def _job(self, request, code="OK"):
        """Keep retained progress terminal state consistent with its original result payload."""
        row = super()._job(request, code)
        if self.operation and self.operation["done"]:
            row["state"] = self.operation["payload"][0]["state"]
        return row

    def _port(self, request):
        """Report current physical evidence with independently injectable invalidity."""
        roles = {"DETACHED": ("NOT_APPLICABLE", "NOT_APPLICABLE"),
                 "ACCESSORY": ("NOT_APPLICABLE", "NOT_APPLICABLE"),
                 "LOCAL_SOURCE": ("SOURCE", "SINK"), "LOCAL_SINK": ("SINK", "SOURCE")}
        raw09 = {"DETACHED": 0, "LOCAL_SOURCE": 64, "LOCAL_SINK": 128,
                 "ACCESSORY": 192}[self.attachment]
        raw09 |= 32 if self.orientation == "CC2" else 0
        raw09 |= 16 if self.irq_pending else 0
        raw09 |= {"30": 0, "40": 2, "50": 4, "60": 6}[self.core_profile["duty"]]
        raw08 = {"default": 0, "1a5": 64, "3a0": 128}[self.core_profile["current"]]
        raw08 |= {"NOT_APPLICABLE": 0, "DEFAULT": 0, "MA_1500": 16,
                  "MA_3000": 48, "CHARGE_THROUGH_500_MA": 32}.get(self.remote_current, 0)
        raw08 |= {"NONE": 0, "NOT_APPLICABLE": 0, "AUDIO": 8, "CHARGE_THROUGH": 10,
                  "DEBUG": 12, "DEBUG_LOCAL_DFP": 12, "DEBUG_LOCAL_UFP": 14,
                  "UNKNOWN": 2}[self.accessory]
        valid = 511 if self.attachment == "LOCAL_SOURCE" else 383
        if self.attachment == "ACCESSORY":
            valid &= ~8
        if self.remote_current == "UNKNOWN":
            valid &= ~16
        fields = dict(raw08=raw08, raw09=raw09, raw_valid_mask=3, valid_fields=valid,
                      invalidated=0, attachment=self.attachment,
                      local_role=roles[self.attachment][0], peer_role=roles[self.attachment][1],
                      orientation=self.orientation, detected_current=self.remote_current,
                      sequence=self.sequence, observed_ms=self.observed_ms,
                      observed_time_valid=1, age_ms=0)
        fields.update(self.port_fault)
        detail = dict(advertised_current=self.core_profile["current"], accessory=self.accessory,
                      active_cable=0, interrupt_pending=int(self.irq_pending),
                      configuration="KNOWN", coherence="BOOKENDS_MATCH",
                      config_generation=self.applied_generation)
        detail.update(self.detail_fault)
        return [record("PORT", request, self.nonce, **fields),
                record("PORT_DETAIL", request, self.nonce, **detail)]

    def _advance(self, budget):
        """Advance at most the requested callbacks; no failed operation is automatically retried."""
        op = self.operation
        if not op or op["done"] or not budget or op["command"] == self.hang_operation:
            return 0
        used = min(budget, op["remaining"])
        self.attempts += used
        op["callbacks"] += used
        op["remaining"] -= used
        command = op["command"]
        mutate = command in {"apply", "reset", "irq.ack", "irq.service"}
        failed = command == self.fail_operation and op["callbacks"] >= self.fail_at_callback
        if (mutate and (command != "apply" or self.core_profile != op["candidate"]) and
                not op.get("write_attempted") and op["callbacks"] >= 2):
            op["write_attempted"] = True
            self.adapter_writes += 1
            self.mutation_history.append((command, dict(self.core_profile)))
        if not op["remaining"] or failed:
            code = self.fail_code if failed else "OK"
            self.failures += int(failed)
            effect = "INDETERMINATE" if failed and op.get("write_attempted") else "NONE"
            if not failed:
                if command == "apply":
                    changed = self.core_profile != op["candidate"]
                    self.core_profile = dict(op["candidate"])
                    self.applied_profile = dict(self.core_profile)
                    self.applied_generation = self.desired_generation
                    effect = "CONFIRMED" if changed else "NONE"
                elif command == "reset":
                    self.reset_count += 1
                    if self.pending_remote_current is not None:
                        self.remote_current = self.pending_remote_current
                        self.pending_remote_current = None
                    effect = "CONFIRMED"
                elif command in {"irq.ack", "irq.service"}:
                    self.irq_pending = command == "irq.service" and self.reassert_service
                    effect = "CONFIRMED"
                if command in {"read", "irq.read", "irq.service"}:
                    self.sequence += 1
                    self.observed_ms = int(self.clock.now * 1000)
            outcome = {"OK": "SUCCEEDED", "CANCELLED": "CANCELLED", "TIMEOUT": "TIMED_OUT"}.get(
                code, "FAILED")
            if not failed:
                effect = self.success_effect.get(command, effect)
            payload = [record("RESULT", op["request"], op["nonce"], token=op["token"],
                              kind=runner.KINDS[command], state=outcome, code=code,
                              effect=effect, callbacks=op["callbacks"],
                              desired_generation=self.desired_generation,
                              applied_generation=self.applied_generation),
                       record("RESULT_DETAIL", op["request"], op["nonce"], token=op["token"],
                              irq_observed=int(command.startswith("irq.")),
                              irq_before=int(op.get("irq_before", False)),
                              ack_attempted=int(command in {"irq.ack", "irq.service"}),
                              ack_confirmed=int(not failed and command in {
                                  "irq.ack", "irq.service"}),
                              irq_after=int(self.irq_pending),
                              irq_after_valid=int(command == "irq.service"),
                              reset=self.reset_evidence if command == "reset" else "NONE")]
            op.update(payload=payload, done=True)
            self.queue(payload + [record("END", op["request"], op["nonce"], command=command,
                                         token=op["token"], outcome=outcome, code=code,
                                         callbacks=op["callbacks"])])
        return used

    def write(self, data, timeout):
        """Add mutation admissions and cached IRQ/pin queries to the observation peer."""
        text = data.decode("ascii").rstrip("\r\n")
        canonical = runner.canonical_command(text)
        if canonical not in {"apply", "reset", "irq.read", "irq.ack", "irq.service", "reconcile",
                             "intpin", "irq", "cancel", "capabilities"}:
            previous = self.operation
            result = super().write(data, timeout)
            if self.operation is not None and self.operation is not previous:
                self.operation["irq_before"] = self.irq_pending
                self.operation_history.append(canonical)
            return result
        self.timeouts.append(("write", timeout))
        self.clock.now += self.write_delay + 0.001
        if self.write_error is not None:
            raise self.write_error
        self.writes.append(text)
        self.request += 1
        request = self.request
        ack = record("ACK", request, self.nonce, command=canonical)
        end = record("END", request, self.nonce, command=canonical)
        if canonical in runner.KINDS:
            self.token += 1
            self.operation = dict(request=request, nonce=self.nonce, command=canonical,
                                  token=self.token, done=False, remaining=3, callbacks=0,
                                  irq_before=self.irq_pending, candidate=dict(self.staged))
            self.operation_history.append(canonical)
            if canonical == "apply":
                self.candidate_history.append(dict(self.staged))
                self.desired_profile = dict(self.staged)
                self.desired_generation += 1
                self.has_desired = True
            ack.update(action="ADMITTED", code="IN_PROGRESS", token=str(self.token))
            return self._respond(text, [ack, self._job(request, "IN_PROGRESS")], data)
        if canonical == "intpin":
            payload = [record("PIN", request, self.nonce, source="application",
                              available=int(self.irq_available),
                              value=int(not self.irq_pending) if self.irq_available else "unknown")]
        elif canonical == "irq":
            payload = [record("IRQ", request, self.nonce, source="managed_operation", observed=1,
                              pending_before=int(self.irq_pending), ack_attempted=0,
                              ack_confirmed=0, pending_after=int(self.irq_pending),
                              pending_after_valid=1)]
        elif canonical == "capabilities":
            payload = [record("CAPABILITIES", request, self.nonce,
                              configured_variant=self.variant, observed_variant=self.variant,
                              profile_fields=self.profile_fields, identity_valid=1,
                              identity_matches=1,
                              advertisement_write=int(self.variant != "tusb322i"))]
        else:
            ack.update(action="LOCAL", token=str(self.token))
            end["token"] = str(self.token)
            self.fail_operation = self.operation["command"]
            self.fail_code = "CANCELLED"
            self.hang_operation = None
            self._advance(1)
            payload = []
        self._advance(int(self.auto == "1"))
        return self._respond(text, [ack] + payload + [end], data)


class FixtureExecutionTests(unittest.TestCase):
    """Run production fixture plans with finite fake time and explicit physical transitions."""

    def make_session(self, suite="orientation", variant="tusb320lai", **options):
        """Create isolated artifact files and a public Session over the modeled serial target."""
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        clock = FakeClock()
        artifacts = Artifacts(Path(temporary.name), clock)
        self.addCleanup(artifacts.close)
        peer = FixturePeer(clock)
        peer.variant = variant
        peer.address = 0x60 if variant in {"tusb320", "tusb320i"} else 0x47
        peer.profile_fields = 155 if variant in {"tusb320", "tusb320i"} else (
            127 if variant == "tusb322i" else 255)
        nonces = iter(f"fixture_{index}" for index in range(10000))
        session = runner.Session(peer, artifacts, clock=clock,
                                 nonce_factory=lambda: next(nonces))
        plan = ObservationPlan(variant=variant, address=peer.address, suites=(suite,), **options)
        return session, peer, clock, artifacts, plan

    def execute(self, fixture, *, setup="default", expected=EXPECTED, transition=None):
        """Call the real plan; all operator waiting uses an injected monotonic clock."""
        from tusb32x_hil_fixtures import run_fixture_plan
        session, peer, clock, _, plan = fixture
        setup = setup_for(plan.variant) if setup == "default" else setup
        return run_fixture_plan(session, plan, setup, expected,
                                transition=transition or self.transitions(peer), clock=clock,
                                sleep=lambda seconds: setattr(clock, "now", clock() + seconds))

    def transitions(self, peer):
        """Return an explicit modeled fixture driver, filled with frozen named transitions."""
        def transition(step, deadline):
            """Apply the independently named physical state before the declared deadline."""
            if peer.clock() >= deadline:
                return False
            name = step["id"]
            if "detach" in name:
                peer.physical("DETACHED")
            elif name in {"orientation-cc1", "orientation-cc2"}:
                peer.physical("LOCAL_SINK", orientation=name[-3:].upper())
            elif name.startswith("accessory-"):
                accessory = {"audio": "AUDIO", "debug": "DEBUG" if peer.variant in {
                    "tusb320", "tusb320i"} else "DEBUG_LOCAL_DFP",
                    "charge-through": "CHARGE_THROUGH", "debug-local-dfp": "DEBUG_LOCAL_DFP",
                    "debug-local-ufp": "DEBUG_LOCAL_UFP"}[name[len("accessory-"):]]
                peer.physical("ACCESSORY", accessory=accessory)
            elif name.startswith("sink-"):
                current = {"sink-default": "DEFAULT", "sink-1a5": "MA_1500",
                           "sink-3a0": "MA_3000", "sink-settled-1a5": "MA_1500",
                           "sink-settled-3a0": "MA_3000"}[name]
                peer.physical("LOCAL_SINK", current=current)
            elif name.startswith("source-"):
                peer.physical("LOCAL_SOURCE")
            elif name == "drp-resolved":
                peer.physical("LOCAL_SINK")
            elif name in {"irq-assert", "irq-reassert", "irq-coalesced"}:
                peer.physical("LOCAL_SINK", irq=True)
            else:
                raise AssertionError(f"unknown fixture step {name!r}")
            return True
        return transition

    def assert_nonpassing(self, fixture, **options):
        """Require saved failure/incomplete evidence from the actual execution exception."""
        failures = (PlanFailure, PlanBlocked, runner.SessionError, ValueError)
        with self.assertRaises(failures) as error:
            self.execute(fixture, **options)
        report = fixture[3].snapshot()
        self.assertNotEqual(report["exit_code"], 0)
        saved = json.loads((fixture[3].directory / "report.json").read_text(encoding="utf-8"))
        self.assertEqual(saved["exit_code"], report["exit_code"])
        return error.exception

    def test_each_complete_suite_restores_actual_and_local_profiles(self):
        """All four suites prove observations, exact owned takes and detached cleanup."""
        for suite in ("orientation", "accessory", "role-current", "interrupt"):
            with self.subTest(suite=suite):
                fixture = self.make_session(suite)
                session, peer, _, artifacts, _ = fixture
                peer.auto, peer.color, peer.verbose = "1", "1", "0"
                actual_before, staged_before = dict(peer.core_profile), dict(peer.staged)
                summary = self.execute(fixture)
                self.assertGreater(summary["transitions"], 2)
                self.assertGreater(summary["operations"], 2)
                self.assertEqual(artifacts.snapshot()["exit_code"], 0)
                self.assertEqual(artifacts.snapshot()["checks"][suite]["status"], "PASS")
                self.assertEqual(artifacts.metadata["hardware_cleanup"], "PASS")
                self.assertEqual(artifacts.metadata["fixture"], setup_for())
                self.assertEqual(artifacts.metadata["fixture_baseline"]["supply_mv"], 5000)
                self.assertEqual(peer.core_profile, actual_before)
                self.assertEqual(peer.staged, staged_before)
                self.assertEqual((peer.auto, peer.color, peer.verbose), ("1", "1", "0"))
                self.assertEqual(peer.attachment, "DETACHED")
                self.assertIsNone(session.operation)
                self.assertIsNone(peer.operation)
                admitted = len(peer.operation_history)
                self.assertEqual(sum(text.startswith("result take ") for text in peer.writes),
                                 admitted)
                self.assertTrue(all(used <= 1 for _, _, used in peer.turn_history))
                self.assertFalse(any(text.startswith(("wreg ", "recover-dead-battery"))
                                     for text in peer.writes))
                self.assertTrue(peer.writes[-1].startswith("hilmark "))

    def test_absent_fixture_is_not_run_before_any_device_callback(self):
        """No selected suite can turn missing external equipment into passing defaults."""
        for suite in ("orientation", "accessory", "role-current", "interrupt"):
            with self.subTest(suite=suite):
                fixture = self.make_session(suite)
                self.assert_nonpassing(fixture, setup=None)
                self.assertEqual(fixture[1].attempts, 0)
                self.assertFalse(fixture[1].writes)
                statuses = [row["status"] for row in fixture[3].snapshot()["checks"].values()]
                self.assertIn("NOT RUN", statuses)
                self.assertEqual(fixture[3].snapshot()["exit_code"], 2)

    def test_absent_declared_capability_stops_each_suite_before_i2c(self):
        """A fixture declaration must explicitly include the selected observation capability."""
        for suite in ("orientation", "accessory", "role-current", "interrupt"):
            with self.subTest(suite=suite):
                fixture = self.make_session(suite)
                setup = setup_for()
                setup["capabilities"] = []
                self.assert_nonpassing(fixture, setup=setup)
                self.assertEqual(fixture[1].attempts, 0)
                self.assertFalse(fixture[1].writes)

    def test_missing_expected_identity_is_blocked_before_device_traffic(self):
        """Every mutation requires commit/framework/target expectations, not host checkout hints."""
        for expected in (None, {}, {"git": GIT}, {"framework": "host", "target": "native"}):
            with self.subTest(expected=expected):
                fixture = self.make_session()
                self.assert_nonpassing(fixture, expected=expected)
                self.assertFalse(fixture[1].writes)
                self.assertEqual(fixture[3].snapshot()["exit_code"], 2)

    def test_unknown_and_mismatched_flashed_identity_never_write_device(self):
        """The live firmware provenance is independently required before any mutation."""
        for identity in (dict(git="unknown", tree="unknown", build="unknown"),
                         dict(git="f" * 40, tree="clean", build="test_build")):
            with self.subTest(identity=identity):
                fixture = self.make_session()
                fixture[1].identity = identity
                self.assert_nonpassing(fixture)
                self.assertEqual(fixture[1].adapter_writes, 0)
                self.assertFalse(any(text.startswith(("apply", "reset", "irq ack", "irq service"))
                                     for text in fixture[1].writes))

    def test_322_unresolved_mutation_suites_reject_before_i2c(self):
        """Unresolved advertisement and IRQ semantics are explicit blocked capability gates."""
        for suite in ("role-current", "interrupt"):
            with self.subTest(suite=suite):
                fixture = self.make_session(suite, "tusb322i")
                self.assert_nonpassing(fixture)
                self.assertFalse(fixture[1].writes)
                self.assertEqual(fixture[1].attempts, 0)
                self.assertEqual(fixture[3].snapshot()["exit_code"], 2)

    def test_base_extended_accessory_denial_precedes_device_callbacks(self):
        """Base parts cannot use an extended local-role debug fixture by assumption."""
        for capability in ("debug-local-dfp", "debug-local-ufp"):
            with self.subTest(capability=capability):
                fixture = self.make_session("accessory", "tusb320")
                setup = setup_for("tusb320")
                setup["capabilities"] = [capability]
                self.assert_nonpassing(fixture, setup=setup)
                self.assertEqual(fixture[1].attempts, 0)
                self.assertFalse(fixture[1].writes)

    def test_base_charge_through_fixture_uses_documented_code5(self):
        """The base code 5 charge-through accessory remains supported by its own evidence."""
        fixture = self.make_session("accessory", "tusb320")
        setup = setup_for("tusb320")
        setup["capabilities"] = ["charge-through"]
        self.execute(fixture, setup=setup)
        self.assertEqual(fixture[3].snapshot()["exit_code"], 0)
        self.assertEqual(fixture[3].snapshot()["checks"]["accessory"]["status"], "PASS")

    def test_debug_local_ufp_fixture_requires_actual_ufp_mode(self):
        """Connecting a local-UFP debug fixture requires an explicitly applied matching mode."""
        fixture = self.make_session("accessory")
        peer = fixture[1]
        setup = setup_for()
        setup["capabilities"] = ["debug-local-ufp"]
        normal = self.transitions(peer)
        connected = []
        def check_mode(step, deadline):
            """Observe actual profile before accepting the special local-UFP debug connection."""
            if step["id"] == "accessory-debug-local-ufp":
                connected.append(peer.core_profile["mode"])
                self.assertEqual(peer.core_profile["mode"], "ufp")
            return normal(step, deadline)
        self.execute(fixture, setup=setup, transition=check_mode)
        self.assertEqual(connected, ["ufp"])
        self.assertEqual(fixture[3].snapshot()["exit_code"], 0)

    def test_low_vdd_3a_actual_sink_profile_cannot_be_reused_for_source_capable_mode(self):
        """An observed sink setting at 3.3 V does not qualify source/DRP advertisement at 3 A."""
        for suite in ("accessory", "interrupt"):
            with self.subTest(suite=suite):
                fixture = self.make_session(suite)
                peer = fixture[1]
                peer.core_profile.update(mode="ufp", current="3a0")
                setup = setup_for()
                setup["supply_mv"] = 3300
                self.assert_nonpassing(fixture, setup=setup)
                self.assertNotIn("apply", peer.operation_history)
                self.assertEqual(peer.adapter_writes, 0)
                self.assertEqual(fixture[3].snapshot()["exit_code"], 2)

    def test_startup_or_base_recovery_rejection_precedes_mutation(self):
        """Declared supply readiness and base recovery are owner prerequisites, never inferred."""
        for variant, field in (("tusb320lai", "startup_ready"),
                               ("tusb320", "base_recovery_complete")):
            with self.subTest(variant=variant):
                fixture = self.make_session("orientation", variant)
                setup = setup_for(variant)
                setup[field] = False
                self.assert_nonpassing(fixture, setup=setup)
                self.assertFalse(fixture[1].writes)

    def test_invalid_port_evidence_stops_all_suites(self):
        """Current invalidation, raw validity and field validity cannot become defaults."""
        for suite in ("orientation", "accessory", "role-current", "interrupt"):
            for fault in ({"invalidated": 1}, {"valid_fields": 0}, {"raw_valid_mask": 1},
                          {"observed_time_valid": 0}, {"raw08": "unknown"},
                          {"raw09": "unknown"}):
                with self.subTest(suite=suite, fault=fault):
                    fixture = self.make_session(suite)
                    fixture[1].port_fault = fault
                    self.assert_nonpassing(fixture)
                    self.assertEqual(fixture[1].adapter_writes, 0)
                    self.assertEqual(fixture[3].snapshot()["exit_code"], 1)

    def test_unexpected_attachment_never_starts_role_apply(self):
        """A declared disconnected fixture must agree with a fresh observed detached port."""
        fixture = self.make_session("role-current")
        def attached(step, deadline):
            """Model a partner that remains attached despite the explicit disconnect request."""
            fixture[1].physical("LOCAL_SOURCE")
            return True
        self.assert_nonpassing(fixture, transition=attached)
        self.assertEqual(fixture[1].adapter_writes, 0)
        self.assertNotIn("apply", fixture[1].operation_history)

    def test_suite_specific_validity_bits_are_required_for_connected_evidence(self):
        """Orientation, accessory, remote-current and IRQ values each require their own bit."""
        for suite, bit in (("orientation", 8), ("accessory", 64), ("role-current", 16),
                           ("interrupt", 256)):
            with self.subTest(suite=suite):
                fixture = self.make_session(suite)
                fixture[1].port_fault["valid_fields"] = 511 & ~bit
                self.assert_nonpassing(fixture)
                self.assertEqual(fixture[3].snapshot()["exit_code"], 1)
                self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")

    def test_unavailable_wired_int_input_is_not_run_before_device_callbacks(self):
        """Modeled latch evidence cannot substitute for the required opt-in physical INT input."""
        fixture = self.make_session("interrupt")
        fixture[1].irq_available = False
        self.assert_nonpassing(fixture)
        self.assertEqual(fixture[3].snapshot()["exit_code"], 2)
        self.assertIn("NOT RUN", [row["status"]
                                  for row in fixture[3].snapshot()["checks"].values()])
        self.assertNotIn("irq.ack", fixture[1].operation_history)
        self.assertNotIn("apply", fixture[1].operation_history)
        self.assertEqual(fixture[1].attempts, 0)
        self.assertFalse(fixture[3].residual_uncertainty)

    def test_missing_operator_transition_saves_incomplete_checkpoint(self):
        """Absent input is terminal NOT RUN and cannot authorize device mutation."""
        fixture = self.make_session()
        self.assert_nonpassing(fixture, transition=lambda step, deadline: False)
        self.assertFalse(fixture[3].metadata["fixture_step"]["acknowledged"])
        self.assertEqual(fixture[1].adapter_writes, 0)
        self.assertEqual(fixture[3].snapshot()["exit_code"], 2)

    def test_late_transition_is_failed_under_original_deadline(self):
        """A late ready response never renews the immutable whole-run limit or starts apply."""
        fixture = self.make_session(run_timeout=1, command_timeout=1)
        def late(step, deadline):
            """Advance fake time beyond the exact operator deadline without wall-clock sleep."""
            fixture[2].now = deadline + 0.001
            fixture[1].physical("DETACHED")
            return True
        self.assert_nonpassing(fixture, transition=late)
        self.assertEqual(fixture[1].adapter_writes, 0)
        self.assertEqual(fixture[3].snapshot()["exit_code"], 1)

    def test_callback_failure_or_cancelled_mutation_is_never_replayed(self):
        """Failures around writes retain payload status and prohibit blind profile restoration."""
        for suite in ("orientation", "accessory", "role-current", "interrupt"):
            for callback, code in ((1, "I2C_ERROR"), (2, "I2C_TIMEOUT"), (2, "CANCELLED"),
                                   (2, "TIMEOUT")):
                with self.subTest(suite=suite, callback=callback, code=code):
                    fixture = self.make_session(suite)
                    peer = fixture[1]
                    peer.fail_operation = "apply"
                    peer.fail_at_callback, peer.fail_code = callback, code
                    self.assert_nonpassing(fixture)
                    self.assertEqual(peer.operation_history.count("apply"), 1)
                    self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")
                    self.assertTrue(fixture[3].residual_uncertainty)
                    self.assertEqual(fixture[3].snapshot()["exit_code"], 1)
                    self.assertIsNone(fixture[0].operation)

    def test_incomplete_mutation_retains_owned_token_and_never_retries_take(self):
        """Finite timeout retains unfinished ownership and saves explicit cleanup uncertainty."""
        fixture = self.make_session(command_timeout=0.05)
        fixture[1].hang_operation = "apply"
        self.assert_nonpassing(fixture)
        self.assertEqual(fixture[1].operation_history.count("apply"), 1)
        self.assertIsNotNone(fixture[0].operation)
        self.assertFalse(fixture[0].operation.complete)
        self.assertTrue(fixture[3].residual_uncertainty)
        self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")

    def test_irq_and_reset_write_failures_take_once_and_never_replay(self):
        """ACK, service and reset failures preserve operation and cleanup evidence."""
        for command in ("irq.ack", "irq.service", "reset"):
            for boundary in (1, 2):
                with self.subTest(command=command, boundary=boundary):
                    base = command == "reset"
                    fixture = self.make_session("role-current" if base else "interrupt",
                                                "tusb320" if base else "tusb320lai")
                    peer = fixture[1]
                    peer.fail_operation = command
                    peer.fail_at_callback = boundary
                    setup = setup_for(peer.variant)
                    setup["capabilities"] = ["role-current"] if base else ["interrupt"]
                    self.assert_nonpassing(fixture, setup=setup)
                    self.assertEqual(peer.operation_history.count(command), 1)
                    self.assertEqual(sum(text.startswith("result take ") for text in peer.writes),
                                     len(peer.operation_history))
                    result = fixture[3].metadata["last_fixture_result"]["result"]
                    self.assertEqual(result["kind"], runner.KINDS[command])
                    self.assertEqual(result["code"], "I2C_ERROR")
                    self.assertEqual(result["effect"],
                                     "NONE" if boundary == 1 else "INDETERMINATE")
                    self.assertEqual(fixture[3].snapshot()["exit_code"], 1)
                    self.assertTrue(fixture[3].residual_uncertainty)

    def test_refused_result_take_is_retained_without_cleanup_or_take_retry(self):
        """A refused owned-result take cannot be followed by replacement work or restoration."""
        fixture = self.make_session()
        peer = fixture[1]
        normal = self.transitions(peer)
        def refuse_take(step, deadline):
            """Fail result delivery only after the initial detached observation succeeded."""
            ready = normal(step, deadline)
            if step["id"] == "initial-detached":
                peer.take_code = "BUSY"
            return ready
        self.assert_nonpassing(fixture, transition=refuse_take)
        takes = [text for text in peer.writes if text.startswith("result take ")]
        self.assertEqual(len(takes), len(set(takes)))
        self.assertIsNotNone(fixture[0].operation)
        self.assertNotIn("apply", peer.operation_history)
        self.assertNotEqual(fixture[3].cleanup, "PASS")

    def test_failed_cleanup_attestation_does_not_restore_old_source_role(self):
        """A changed partner at cleanup blocks restoration after a successful primary suite."""
        fixture = self.make_session()
        peer = fixture[1]
        peer.core_profile["mode"] = "dfp"
        normal = self.transitions(peer)
        def changed(step, deadline):
            """Report an attached sink instead of the required final disconnected target."""
            if step["id"] == "cleanup-detached":
                peer.physical("LOCAL_SINK")
                return True
            return normal(step, deadline)
        self.assert_nonpassing(fixture, transition=changed)
        self.assertEqual(peer.core_profile["mode"], "ufp")
        self.assertEqual(peer.operation_history.count("apply"), 1)
        self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")
        self.assertEqual(fixture[3].snapshot()["checks"]["orientation"]["status"], "PASS")
        self.assertTrue(fixture[3].residual_uncertainty)

    def test_irq_ack_retains_actual_50_and_unrelated_write_leaves_pending(self):
        """ACK never adopts staged 30%; changing duty explicitly does not clear the latch."""
        fixture = self.make_session("interrupt")
        peer = fixture[1]
        self.execute(fixture)
        ack = [profile for command, profile in peer.mutation_history if command == "irq.ack"]
        self.assertEqual(len(ack), 1)
        self.assertEqual(ack[0]["duty"], "50")
        self.assertEqual(peer.operation_history.count("irq.ack"), 1)
        self.assertEqual(peer.operation_history.count("irq.service"), 1)
        self.assertEqual(fixture[3].metadata["irq_service"]["detail"]["irq_after"], "0")

    def test_irq_recheck_still_pending_is_failed_and_not_serviced_again(self):
        """A bounded service with reassertion cannot count as deassertion or lossless edge proof."""
        fixture = self.make_session("interrupt")
        fixture[1].reassert_service = True
        self.assert_nonpassing(fixture)
        self.assertEqual(fixture[1].operation_history.count("irq.service"), 1)
        self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")

    def test_successful_payload_cannot_conceal_uncertain_mutation_effect(self):
        """A codeOK delivery with uncertain effect still stops without hardware restoration."""
        for effect in ("PARTIAL", "INDETERMINATE"):
            with self.subTest(effect=effect):
                fixture = self.make_session()
                peer = fixture[1]
                peer.success_effect["apply"] = effect
                self.assert_nonpassing(fixture)
                self.assertEqual(peer.operation_history.count("apply"), 1)
                self.assertIsNone(fixture[0].operation)
                self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")

    def test_successful_reset_requires_explicit_completed_evidence(self):
        """An accepted reset result cannot hide its unresolved completion state."""
        fixture = self.make_session("role-current", "tusb320")
        setup = setup_for("tusb320")
        setup["capabilities"] = ["role-current"]
        peer = fixture[1]

        peer.reset_evidence = "MAY_BE_PENDING"
        self.assert_nonpassing(fixture, setup=setup)
        self.assertEqual(peer.operation_history.count("reset"), 1)
        self.assertIsNone(fixture[0].operation)
        self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")

    def test_unknown_accessory_encoding_retains_raw_and_cannot_pass(self):
        """A raw reserved accessory code remains unknown instead of a supported default."""
        fixture = self.make_session("accessory")
        peer = fixture[1]
        normal = self.transitions(peer)
        def unknown(step, deadline):
            """Present the independently reserved code1 after the first accessory connect."""
            ready = normal(step, deadline)
            if step["id"] == "accessory-audio":
                peer.accessory = "UNKNOWN"
            return ready
        self.assert_nonpassing(fixture, transition=unknown)
        saved = fixture[3].metadata["fixture_port"]
        self.assertEqual(int(saved["port"]["raw08"]) & 14, 2)
        self.assertEqual(saved["detail"]["accessory"], "UNKNOWN")

    def test_base_current_update_uses_explicit_resets_only_in_mutating_suite(self):
        """The modeled base part keeps old attached current until each explicit reset completes."""
        fixture = self.make_session("role-current", "tusb320")
        setup = setup_for("tusb320")
        setup["capabilities"] = ["role-current"]
        self.execute(fixture, setup=setup)
        self.assertEqual(fixture[1].reset_count, 2)
        self.assertEqual(fixture[1].operation_history.count("reset"), 2)
        self.assertEqual(fixture[3].snapshot()["exit_code"], 0)
        observations = fixture[3].metadata["fixture_observations"]["role-current"]
        for category, current in (("1a5", "MA_1500"), ("3a0", "MA_3000")):
            self.assertEqual(observations["sink-" + category]["after_reset"]["port"][
                "detected_current"], current)
            self.assertTrue(observations["sink-settled-" + category]["attestation"])

    def test_apply_candidate_preserves_observed_fields_over_cli_defaults(self):
        """The first planned mode change inherits actual fields instead of stale staging."""
        fixture = self.make_session()
        peer = fixture[1]
        peer.core_profile.update(current="1a5", duty="60", debounce="151")
        self.execute(fixture)
        first = peer.candidate_history[0]
        self.assertEqual(first["mode"], "ufp")
        self.assertEqual((first["current"], first["duty"], first["debounce"]),
                         ("1a5", "60", "151"))
        self.assertEqual(peer.staged["current"], "default")
        self.assertEqual(peer.staged["duty"], "30")

    def test_unknown_applicable_observed_field_blocks_before_mutation(self):
        """A validmask alone cannot turn unknown semantic profile evidence into settings."""
        fixture = self.make_session()
        fixture[1].profile_fault["observed"] = {"duty": "unknown"}
        self.assert_nonpassing(fixture)
        self.assertEqual(fixture[1].adapter_writes, 0)
        self.assertNotIn("apply", fixture[1].operation_history)

    def test_contradictory_observed_controls_block_before_mutation(self):
        """Semantic on/off values must agree with their independently reported raw disable bits."""
        for raw in ("disable_ufp_accessory", "disable_term", "disable_rd_rp"):
            with self.subTest(raw=raw):
                fixture = self.make_session()
                fixture[1].profile_fault["observed"] = {raw: "1"}
                self.assert_nonpassing(fixture)
                self.assertEqual(fixture[1].adapter_writes, 0)
                self.assertNotIn("apply", fixture[1].operation_history)

    def test_target_change_at_cleanup_keeps_residual_and_never_restores(self):
        """A target change after successful observations cannot authorize restoration writes."""
        fixture = self.make_session()
        peer = fixture[1]
        normal = self.transitions(peer)
        def changed(step, deadline):
            """Change the modeled target address at the final physical checkpoint."""
            ready = normal(step, deadline)
            if step["id"] == "cleanup-detached":
                peer.address = 0x67
            return ready
        self.assert_nonpassing(fixture, transition=changed)
        self.assertEqual(peer.operation_history.count("apply"), 1)
        self.assertTrue(fixture[3].residual_uncertainty)
        self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")

    def test_interruption_after_write_intent_is_saved_without_restore(self):
        """An interruption after mutation records exit130 and preserves required cleanup."""
        fixture = self.make_session()
        peer = fixture[1]
        normal = self.transitions(peer)
        def interrupted(step, deadline):
            """Interrupt at the first connected orientation, after its explicit role mutation."""
            if step["id"] == "orientation-cc1":
                raise KeyboardInterrupt()
            return normal(step, deadline)
        with self.assertRaises(KeyboardInterrupt):
            self.execute(fixture, transition=interrupted)
        saved = json.loads((fixture[3].directory / "report.json").read_text(encoding="utf-8"))
        self.assertEqual(saved["exit_code"], 130)
        self.assertTrue(saved["interrupted"])
        self.assertEqual(saved["metadata"]["hardware_cleanup"], "BLOCKED")
        self.assertEqual(peer.operation_history.count("apply"), 1)
        self.assertTrue(saved["residual_uncertainty"])


if __name__ == "__main__":
    unittest.main()
