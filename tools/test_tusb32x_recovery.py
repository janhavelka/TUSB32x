"""Verify recovery expectations and finite plans through framed modeled serial I/O.

The existing fixture peer and virtual clock supply all transport behavior. Tests
exercise public Session and plan entrypoints without serial ports, real sleeps,
hardware switching or replacements for production ownership/cleanup helpers.
"""

from __future__ import annotations

from pathlib import Path
from contextlib import ExitStack, redirect_stderr, redirect_stdout
import io
import json
import tempfile
import unittest
from unittest import mock

import tusb32x_hil_runner as runner
from tusb32x_cli_contract import ROWS, SCHEMAS
from tusb32x_hil_artifacts import Artifacts, CaptureLimits, EvidenceError
from tusb32x_hil_plans import ObservationPlan, PlanBlocked, PlanFailure
from test_tusb32x_fixtures import EXPECTED, FixturePeer, setup_for
from test_tusb32x_hil_runner import FakeClock, encode_record, record


TRANSPORT_CODES = (
    "I2C_ERROR", "I2C_NACK_ADDR", "I2C_NACK_DATA", "I2C_TIMEOUT", "I2C_BUS", "I2C_BUSY",
)
RECOVERY_SUITES = ("reset-termination", "fault", "console")
AUTHORIZED_ACTIONS = (
    "reset", "recover-dead-battery", "nack", "disconnect", "power-cycle",
    "console-backpressure", "console-reconnect", "console-reboot",
)


def recovery_setup(variant="tusb320lai"):
    """Declare modeled recovery actions explicitly; declarations never qualify hardware."""
    setup = setup_for(variant)
    setup.update(capabilities=list(RECOVERY_SUITES), authorized_actions=list(AUTHORIZED_ACTIONS),
                 stable_vdd=True, source_startup=True, nack_code="I2C_NACK_ADDR",
                 disconnect_code="I2C_ERROR", target_vdd_continuous=True)
    return setup


class RecoveryPeer(FixturePeer):
    """Extend the shared wire peer with deadline controls, recovery and reconnect diagnostics."""

    def __init__(self, clock):
        """Keep physical fault state and configuration trust independent of local staging."""
        super().__init__(clock)
        self.config_unknown = False
        self.recovery_unresolved = False
        self.bad_elapsed = False
        self.console_drops = 0
        self.absent_desired_mode = self.core_profile["mode"]

    def _profile(self, request):
        """Publish independent aggregate termination evidence from both actual controls."""
        rows = super()._profile(request)
        for row in rows:
            if row["type"] == "CONFIG":
                row["configuration"] = "UNKNOWN" if self.config_unknown else "KNOWN"
            elif row["type"] == "CONFIG_DETAIL":
                row["termination"] = ("DISABLED" if any(self.core_profile[key] == "off"
                                                       for key in ("termination", "rd_rp"))
                                      else "ENABLED")
            elif (row["type"] == "PROFILE" and row["source"] == "desired" and
                  row["available"] == "0"):
                # Reset changes actual mode; absent desired storage stays untouched.
                row["mode"] = self.absent_desired_mode
        return rows

    def _console(self, request):
        """Report separately injected native console loss without pretending USB delivery."""
        row = super()._console(request)
        row.update(dropped_records=str(self.console_drops), dropped_bytes=str(self.console_drops))
        return row

    def queue(self, rows):
        """Attach fixture completion-time/trust evidence before bytes enter the wire buffer."""
        op = self.operation
        for row in rows:
            if row["type"] == "RESULT_DETAIL" and op:
                completed_ms = op.setdefault("completed_ms", int(self.clock() * 1000))
                row.update(started_ms=str(op.get("started_ms", int(self.clock() * 1000))),
                           completed_ms=str(completed_ms),
                           completed_time_valid="1")
                if self.bad_elapsed:
                    row["completed_ms"] = row["started_ms"]
            if row["type"] == "DIAG":
                row.update(configuration="UNKNOWN" if self.config_unknown else "KNOWN",
                           initialized="0" if self.config_unknown else "1",
                           recovery_unresolved=str(int(self.recovery_unresolved)))
        super().queue(rows)

    def terminal(self, code, effect="NONE"):
        """Emit one owned terminal closure without adding a callback or replacing its token."""
        op = self.operation
        state = {"OK": "SUCCEEDED", "CANCELLED": "CANCELLED", "TIMEOUT": "TIMED_OUT"}.get(
            code, "FAILED")
        payload = [record("RESULT", op["request"], op["nonce"], token=op["token"],
                          kind=runner.KINDS[op["command"]], state=state, code=code,
                          effect=effect, callbacks=op["callbacks"],
                          configuration="UNKNOWN" if self.config_unknown else "KNOWN"),
                   record("RESULT_DETAIL", op["request"], op["nonce"], token=op["token"])]
        op.update(payload=payload, done=True)
        self.queue(payload + [record("END", op["request"], op["nonce"], command=op["command"],
                                     token=op["token"], outcome=state, code=code,
                                     callbacks=op["callbacks"])])

    def _advance(self, budget):
        """Apply finite firmware deadlines/waits, leaving ordinary plans in the shared peer."""
        op = self.operation
        if not op or op["done"]:
            return 0
        if op.get("short_deadline") is not None and self.clock() >= op["short_deadline"]:
            self.terminal("TIMEOUT")
            return 0
        if self.fail_code == "CANCELLED" and self.fail_operation == op["command"]:
            self.terminal("CANCELLED")
            self.fail_operation = None
            return 0
        if op["command"] in {"reset", "recover-dead-battery"} and op["callbacks"] == 2:
            ready = op.setdefault("ready", self.clock() + (
                0.031 if op["command"] == "recover-dead-battery" else 0.096))
            if self.clock() < ready:
                return 0
        if op["command"] == "recover-dead-battery":
            if not budget:
                return 0
            self.attempts += 1
            op["callbacks"] += 1
            op["remaining"] -= 1
            if op["callbacks"] in {2, 3}:
                self.adapter_writes += 1
                self.mutation_history.append((op["command"], dict(self.core_profile)))
                self.recovery_unresolved = True
            if (self.fail_operation == op["command"] and
                    op["callbacks"] >= self.fail_at_callback):
                self.failures += 1
                self.config_unknown = self.recovery_unresolved
                self.terminal(self.fail_code,
                              "INDETERMINATE" if self.recovery_unresolved else "NONE")
                return 1
            if not op["remaining"]:
                self.config_unknown = True
                self.recovery_unresolved = False
                self.terminal("OK", "CONFIRMED")
            return 1
        used = super()._advance(budget)
        if op["done"] and op["payload"][0]["code"] == "OK":
            if op["command"] in {"init", "reconcile"}:
                self.config_unknown = False
            elif op["command"] == "reset":
                self.core_profile["mode"] = "drp"
        return used

    def write(self, data, timeout):
        """Add only routes missing from the existing peer, preserving shared framing helpers."""
        text = data.decode("ascii").rstrip("\r\n")
        command = runner.canonical_command(text)
        if command not in {"recover-dead-battery", "invalidate", "help"}:
            previous = self.operation
            answer = super().write(data, timeout)
            if self.operation is not None and self.operation is not previous:
                self.operation["started_ms"] = int(self.clock() * 1000)
                if command == "reset" and text.split()[-1] == "1":
                    self.operation["short_deadline"] = self.clock() + 0.001
            return answer
        self.writes.append(text)
        self.timeouts.append(("write", timeout))
        self.clock.now += 0.001
        self.request += 1
        ack = record("ACK", self.request, self.nonce, command=command)
        end = record("END", self.request, self.nonce, command=command)
        payload = []
        if command == "recover-dead-battery":
            if self.variant not in {"tusb320", "tusb320i"}:
                ack.update(action="UNCHANGED", code="UNSUPPORTED_FEATURE")
                end.update(outcome="FAILED", code="UNSUPPORTED_FEATURE")
            else:
                self.token += 1
                self.operation = dict(request=self.request, nonce=self.nonce, command=command,
                                      token=self.token, done=False, remaining=4, callbacks=0,
                                      started_ms=int(self.clock() * 1000))
                self.operation_history.append(command)
                ack.update(action="ADMITTED", code="IN_PROGRESS", token=str(self.token))
                return self._respond(text, [ack, self._job(self.request, "IN_PROGRESS")], data)
        elif command == "invalidate":
            self.config_unknown = True
            ack["action"] = "LOCAL"
            payload = [record("INVALIDATE", self.request, self.nonce, reason=text.split()[1])]
        else:
            payload = [record("HELP", self.request, self.nonce, canonical=row["command"],
                              **{key: row[key] for key in SCHEMAS["HELP"][3:]
                                 if key != "canonical"}) for row in ROWS]
        return self._respond(text, [ack, *payload, end], data)


class RecoveryExpectationTests(unittest.TestCase):
    """Compare each owned terminal result against its predeclared exact outcome."""

    def make_session(self):
        """Establish real framing/identity over the existing fixture model and disk artifacts."""
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        clock = FakeClock()
        artifacts = Artifacts(Path(temporary.name), clock)
        self.addCleanup(artifacts.close)
        peer = FixturePeer(clock)
        nonces = iter(f"recovery_{index}" for index in range(1000))
        session = runner.Session(peer, artifacts, clock=clock,
                                 nonce_factory=lambda: next(nonces))
        session.begin(expected=EXPECTED)
        session.command("job auto 0")
        return session, peer, clock, artifacts

    def complete(self, session):
        """Advance a fixed number of owner turns and return the original retained payload."""
        for _ in range(8):
            if session.operation.complete:
                return session.wait_operation()
            session.command("job step 1")
        self.fail("modeled operation exceeded its finite callback plan")

    def test_malformed_expectations_reject_before_any_io(self):
        """Expectation spelling/types are invocation errors before markers, writes or reads."""
        invalid = (None, True, 0, [], {}, "", "ok", "TIMEOUT ", "I2C_UNKNOWN",
                   "IN_PROGRESS", "BUSY", "CONFIG_MISMATCH", "OK\n")
        for value in invalid:
            with self.subTest(value=value):
                session, peer, _, _ = self.make_session()
                before = (list(peer.writes), list(peer.timeouts))
                with self.assertRaises(runner.InvocationError):
                    session.command("reset", expected_code=value)
                self.assertEqual((peer.writes, peer.timeouts), before)
                self.assertIsNone(session.operation)

    def test_nonmanaged_expected_failure_rejects_without_io(self):
        """An error expectation cannot excuse failed queries, local edits or raw writes."""
        for command in ("version", "bus", "profile show", "job auto 0", "job step 1",
                        "cancel", "result", "reg 0x0a", "wreg 0x0a 0x30"):
            with self.subTest(command=command):
                session, peer, _, _ = self.make_session()
                before = (list(peer.writes), list(peer.timeouts))
                with self.assertRaises(runner.InvocationError):
                    session.command(command, expected_code="I2C_ERROR")
                self.assertEqual((peer.writes, peer.timeouts), before)

    def test_default_failed_payload_remains_failed_after_successful_delivery(self):
        """Delivery OK preserves the failed hardware payload and its report verdict."""
        session, peer, _, artifacts = self.make_session()
        peer.fail_operation, peer.fail_code = "reset", "I2C_TIMEOUT"
        peer.fail_at_callback = 2
        session.command("reset")
        payload = self.complete(session)
        self.assertEqual(payload["code"], "I2C_TIMEOUT")
        self.assertEqual(payload["effect"], "INDETERMINATE")
        response = session.take_result()
        self.assertEqual(response.end["code"], "OK")
        copied = next(row for row in response.records if row["type"] == "RESULT")
        self.assertEqual(copied["code"], "I2C_TIMEOUT")
        self.assertEqual(copied["origin_request"], payload["origin_request"])
        self.assertIsNone(session.operation)
        self.assertEqual(artifacts.snapshot()["checks"]["operation_payloads"]["status"], "FAIL")

    def test_each_supported_expected_failure_retains_exact_terminal_code(self):
        """Typed cancellation, owner timeout and each backend error can be explicit proof."""
        for code in ("CANCELLED", "TIMEOUT", *TRANSPORT_CODES):
            with self.subTest(code=code):
                session, peer, _, artifacts = self.make_session()
                peer.fail_operation, peer.fail_code = "reset", code
                peer.fail_at_callback = 2
                session.command("reset", expected_code=code)
                payload = self.complete(session)
                self.assertEqual(payload["code"], code)
                self.assertEqual(payload["effect"], "INDETERMINATE")
                self.assertEqual(artifacts.snapshot()["checks"]["operation_payloads"]["status"],
                                 "PASS")
                copied = session.take_result()
                self.assertEqual(copied.end["code"], "OK")
                self.assertEqual(next(row for row in copied.records if row["type"] == "RESULT")
                                 ["code"], code)
                self.assertIsNone(peer.operation)

    def test_expected_failure_is_bound_before_admission_and_completion(self):
        """An operation finishing during admission still compares its saved expectation."""
        session, peer, _, artifacts = self.make_session()
        peer.auto = "1"
        peer.fail_operation, peer.fail_code = "reset", "I2C_NACK_DATA"
        peer.fail_at_callback = 1
        def immediate_completion(text, rows):
            """Place an owner completion immediately after its admission on the wire."""
            peer.queue(rows)
            if text == "reset":
                peer._advance(1)
            return []
        peer.response_hook = immediate_completion
        session.command("reset", expected_code="I2C_NACK_DATA")
        payload = session.wait_operation()
        self.assertEqual(payload["code"], "I2C_NACK_DATA")
        self.assertEqual(artifacts.snapshot()["checks"]["operation_payloads"]["status"], "PASS")
        session.take_result()

    def test_expected_error_preserves_full_signed_detail_and_uncertain_effect(self):
        """Expected failure remains raw device evidence in reports and owned result copies."""
        session, peer, _, artifacts = self.make_session()
        peer.fail_operation, peer.fail_code = "reset", "I2C_ERROR"
        peer.fail_at_callback = 2
        original_queue = peer.queue
        def signed_failure(rows):
            """Inject full-width backend detail into matching failed terminal wire records."""
            for row in rows:
                if row["type"] in {"RESULT", "END"} and row.get("code") == "I2C_ERROR":
                    row["detail"] = "-2147483001"
            original_queue(rows)
        peer.queue = signed_failure
        session.command("reset", expected_code="I2C_ERROR")
        payload = self.complete(session)
        self.assertEqual(payload["detail"], "-2147483001")
        self.assertEqual(payload["effect"], "INDETERMINATE")
        metadata = artifacts.snapshot()["metadata"]["operation"]
        self.assertEqual(metadata["result"]["code"], "I2C_ERROR")
        self.assertEqual(metadata["result"]["detail"], "-2147483001")
        self.assertEqual(metadata["end"]["detail"], "-2147483001")
        copied = session.take_result()
        self.assertEqual(next(row for row in copied.records if row["type"] == "RESULT")
                         ["detail"], "-2147483001")
        self.assertEqual(copied.end["code"], "OK")

    def test_explicit_reconcile_keeps_mismatch_or_unknown_as_observation_status(self):
        """Read-only reconciliation can report unresolved proof without rewriting hardware."""
        for code in ("CONFIGURATION_MISMATCH", "CONFIGURATION_UNKNOWN"):
            with self.subTest(code=code):
                session, peer, _, artifacts = self.make_session()
                peer.fail_operation, peer.fail_code = "reconcile", code
                session.command("reconcile", expected_code=code)
                payload = self.complete(session)
                self.assertEqual(payload["code"], code)
                self.assertEqual(payload["effect"], "NONE")
                self.assertEqual(peer.adapter_writes, 0)
                session.take_result()
                self.assertEqual(artifacts.snapshot()["checks"]["operation_payloads"]["status"],
                                 "PASS")

    def test_unexpected_success_is_failed_proof(self):
        """An induced-fault experiment cannot pass when its operation unexpectedly succeeds."""
        session, _, _, artifacts = self.make_session()
        session.command("reset", expected_code="I2C_NACK_ADDR")
        payload = self.complete(session)
        self.assertEqual(payload["code"], "OK")
        self.assertEqual(payload["effect"], "CONFIRMED")
        self.assertEqual(artifacts.snapshot()["checks"]["operation_payloads"]["status"], "FAIL")
        session.take_result()

    def test_different_error_cannot_satisfy_the_declared_fault(self):
        """A generic bus error is no evidence that an expected address NACK occurred."""
        session, peer, _, artifacts = self.make_session()
        peer.fail_operation, peer.fail_code = "reset", "I2C_ERROR"
        session.command("reset", expected_code="I2C_NACK_ADDR")
        payload = self.complete(session)
        self.assertEqual(payload["code"], "I2C_ERROR")
        self.assertEqual(artifacts.snapshot()["checks"]["operation_payloads"]["status"], "FAIL")
        session.take_result()

    def test_admission_rejection_cannot_satisfy_expected_operation_failure(self):
        """A rejected admission has no original RESULT and therefore proves no fault plan."""
        session, peer, _, artifacts = self.make_session()
        def denied_admission(text, rows):
            """Return a valid synchronous rejection without admitting or completing work."""
            if text == "reset":
                peer.operation = None
                request = peer.request
                return [record("ACK", request, peer.nonce, command="reset",
                               action="UNCHANGED", code="I2C_NACK_ADDR"),
                        record("END", request, peer.nonce, command="reset",
                               outcome="FAILED", code="I2C_NACK_ADDR")]
            return rows
        peer.response_hook = denied_admission
        response = session.command("reset", expected_code="I2C_NACK_ADDR")
        self.assertFalse(response.admitted)
        self.assertEqual(response.end["code"], "I2C_NACK_ADDR")
        self.assertIsNone(session.operation)
        check = artifacts.snapshot()["checks"].get("operation_payloads", {})
        self.assertNotEqual(check.get("status"), "PASS")
        self.assertFalse(any(row["type"] == "RESULT" for row in response.records))

    def test_later_successful_operations_and_cleanup_preserve_primary_failure(self):
        """Neither a subsequent observation nor successful local cleanup erases mismatch."""
        session, peer, _, artifacts = self.make_session()
        session.command("reset", expected_code="TIMEOUT")
        self.complete(session)
        session.take_result()
        session.command("read")
        self.assertEqual(self.complete(session)["code"], "OK")
        session.take_result()
        session.finish()
        report = artifacts.snapshot()
        self.assertEqual(report["checks"]["operation_payloads"]["status"], "FAIL")
        self.assertEqual(report["exit_code"], 1)
        self.assertEqual(report["cleanup"], "PASS")
        self.assertIsNone(peer.operation)

    def test_take_cannot_relabel_an_unexpected_failure(self):
        """An expectation added after terminal failure cannot excuse the earlier payload."""
        session, peer, _, artifacts = self.make_session()
        peer.fail_operation, peer.fail_code = "reset", "I2C_BUS"
        session.command("reset")
        self.complete(session)
        before = list(peer.writes)
        with self.assertRaises(runner.InvocationError):
            session.command(f"result take {session.operation.token}", expected_code="I2C_BUS")
        self.assertEqual(peer.writes, before)
        self.assertIsNotNone(session.operation)
        session.take_result()
        self.assertEqual(artifacts.snapshot()["checks"]["operation_payloads"]["status"], "FAIL")

    def test_explicit_ok_preserves_ordinary_query_and_managed_success(self):
        """The ordinary default remains compatible with exact OK on both route types."""
        session, _, _, artifacts = self.make_session()
        response = session.command("version", expected_code="OK")
        self.assertEqual(response.end["code"], "OK")
        session.command("read", expected_code="OK")
        self.assertEqual(self.complete(session)["code"], "OK")
        session.take_result()
        self.assertEqual(artifacts.snapshot()["checks"]["operation_payloads"]["status"], "PASS")

    def test_expected_owner_timeout_cannot_excuse_missing_original_terminal(self):
        """A host deadline is incomplete capture, not the firmware's typed TIMEOUT result."""
        session, peer, clock, artifacts = self.make_session()
        peer.hang_operation = "reset"
        session.command("reset", expected_code="TIMEOUT")
        before = list(peer.writes)
        with self.assertRaises(runner.SessionTimeout):
            session.wait_operation(deadline=clock() + 0.1)
        report = artifacts.snapshot()
        self.assertFalse(session.valid)
        self.assertNotEqual(report["exit_code"], 0)
        self.assertEqual(report["checks"]["session"]["status"], "FAIL")
        self.assertFalse(session.operation.complete)
        self.assertEqual(peer.writes, before)
        self.assertTrue(report["residual_uncertainty"])

    def test_expected_failure_cannot_excuse_late_successful_transport(self):
        """An exchange returning after its deadline invalidates correlation and cleanup."""
        session, peer, clock, artifacts = self.make_session()
        session.command("reset", expected_code="TIMEOUT")
        peer.read_delay = 0.1
        with self.assertRaises(runner.SessionTimeout):
            session.command("job step 1", deadline=clock() + 0.05)
        self.assertFalse(session.valid)
        self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)
        before = list(peer.writes)
        with self.assertRaises(runner.SessionError):
            session.restore_local()
        self.assertEqual(peer.writes, before)

    def test_expected_failure_cannot_excuse_reboot_or_unframed_success(self):
        """A boot invalidates old ownership before an expected code can count as proof."""
        session, peer, _, artifacts = self.make_session()
        session.command("reset", expected_code="I2C_ERROR")
        peer.pending.extend(b"ESP-ROM:esp32s3\nPASS\n")
        with self.assertRaises(runner.ProtocolError):
            session.wait_operation()
        before = list(peer.writes)
        with self.assertRaises(runner.SessionError):
            session.command("reset", expected_code="I2C_ERROR")
        self.assertEqual(peer.writes, before)
        self.assertFalse(session.valid)
        self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)

    def test_interruption_retains_intent_and_blocks_automatic_cleanup(self):
        """Ctrl-C during an expected fault keeps mutation intent and unconsumed ownership."""
        session, peer, _, artifacts = self.make_session()
        session.command("reset", expected_code="CANCELLED")
        peer.read_error = KeyboardInterrupt()
        with self.assertRaises(KeyboardInterrupt):
            session.wait_operation()
        report = artifacts.snapshot()
        self.assertTrue(report["interrupted"])
        self.assertEqual(report["exit_code"], 130)
        self.assertEqual(report["metadata"]["device_intent"], "reset")
        self.assertEqual(report["metadata"]["unconsumed_operation"]["token"],
                         session.operation.token)
        before = list(peer.writes)
        with self.assertRaises(runner.SessionError):
            session.restore_local()
        self.assertEqual(peer.writes, before)


class RecoveryPlanGuardTests(unittest.TestCase):
    """Require explicit finite suite selection and reject absent setup before serial I/O."""

    def test_each_recovery_suite_has_a_concrete_portless_finite_plan(self):
        """A selected physical plan describes its work while claiming hardware NOT RUN."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                plan = ObservationPlan(suites=(suite,), run_timeout=60)
                described = plan.describe()
                self.assertEqual(described["hardware"], "NOT RUN")
                self.assertEqual(described["suites"], [suite])
                self.assertTrue(any(action.startswith(suite + ":")
                                    for action in described["actions"]))
                self.assertGreater(plan.command_timeout, 0)
                self.assertLessEqual(plan.command_timeout, plan.run_timeout)

    def test_all_nine_families_can_be_explicitly_selected_without_generic_mutation(self):
        """An explicit nine-suite request retains exact names and deduplicates repeats."""
        suites = ("readonly", "soak", "orientation", "accessory", "role-current", "interrupt",
                  *RECOVERY_SUITES)
        plan = ObservationPlan(suites=(*suites, "fault"), count=2, interval_ms=1)
        self.assertEqual(plan.suites, suites)
        for invalid in ("all", "all-mutating", "recovery", "reset", "console "):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                ObservationPlan(suites=(invalid,))

    def test_missing_fixture_rejects_all_new_suites_without_any_serial_bytes(self):
        """A requested recovery suite cannot claim success from instructions alone."""
        from tusb32x_hil_fixtures import run_fixture_plan

        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite), tempfile.TemporaryDirectory() as directory:
                clock = FakeClock()
                artifacts = Artifacts(Path(directory), clock)
                try:
                    peer = FixturePeer(clock)
                    session = runner.Session(peer, artifacts, clock=clock)
                    plan = ObservationPlan(suites=(suite,))
                    with self.assertRaises(PlanBlocked):
                        run_fixture_plan(session, plan, None, EXPECTED, clock=clock,
                                         sleep=lambda _: self.fail("missing fixture cannot wait"))
                    self.assertFalse(peer.writes)
                    self.assertEqual(peer.attempts, 0)
                    self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)
                    self.assertEqual(artifacts.snapshot()["checks"][suite]["status"], "NOT RUN")
                finally:
                    artifacts.close()

    def rejected_setup(self, suite, setup, *, expected=EXPECTED, variant="tusb320lai"):
        """Run the real prerequisite path and require saved nonpassing evidence without I/O."""
        from tusb32x_hil_fixtures import run_fixture_plan

        with tempfile.TemporaryDirectory() as directory:
            clock = FakeClock()
            artifacts = Artifacts(Path(directory), clock)
            try:
                peer = FixturePeer(clock)
                session = runner.Session(peer, artifacts, clock=clock)
                plan = ObservationPlan(variant=variant,
                                       address=0x60 if variant in {"tusb320", "tusb320i"}
                                       else 0x47, suites=(suite,))
                with self.assertRaises((PlanBlocked, ValueError)):
                    run_fixture_plan(session, plan, setup, expected, clock=clock,
                                     sleep=lambda _: self.fail("rejected setup cannot wait"))
                self.assertFalse(peer.writes)
                self.assertEqual(peer.attempts, 0)
                self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)
            finally:
                artifacts.close()

    def test_missing_firmware_identity_rejects_each_new_suite_before_io(self):
        """A fixture does not substitute for expected flashed commit, framework and target."""
        for suite in RECOVERY_SUITES:
            for expected in (None, {}, {"git": EXPECTED["git"]},
                             {"git": EXPECTED["git"], "framework": "host"}):
                with self.subTest(suite=suite, expected=expected):
                    self.rejected_setup(suite, recovery_setup(), expected=expected)

    def test_runtime_recovery_readiness_requires_exact_boolean_evidence(self):
        """Falsy integers or strings cannot substitute for this run's typed recovery proof."""
        from tusb32x_hil_fixtures import fixture_prerequisite

        plan = ObservationPlan(variant="tusb320", address=0x60, suites=("fault",))
        for value in (0, 1, "false", [], {}):
            with self.subTest(value=value), self.assertRaises(PlanBlocked):
                fixture_prerequisite(plan, recovery_setup("tusb320"), EXPECTED,
                                     base_recovery_required=value)

    def test_missing_declared_capability_rejects_each_new_suite_before_io(self):
        """An action authorization cannot supply an absent fixture capability."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                setup = recovery_setup()
                setup["capabilities"] = []
                self.rejected_setup(suite, setup)

    def test_each_physical_action_requires_separate_authorization(self):
        """Every selected external action must be named before any serial or device activity."""
        for suite, actions in (("reset-termination", ("reset",)),
                               ("fault", ("nack", "disconnect", "power-cycle")),
                               ("console", ("console-backpressure", "console-reconnect",
                                            "console-reboot"))):
            for action in actions:
                with self.subTest(suite=suite, action=action):
                    setup = recovery_setup()
                    setup["authorized_actions"].remove(action)
                    self.rejected_setup(suite, setup)

    def test_original_recovery_requires_stable_supply_source_startup_and_authorization(self):
        """Original-part recovery has three independent preconditions before traffic."""
        for variant in ("tusb320", "tusb320i"):
            for missing in ("stable_vdd", "source_startup", "recover-dead-battery"):
                with self.subTest(variant=variant, missing=missing):
                    setup = recovery_setup(variant)
                    if missing == "recover-dead-battery":
                        setup["authorized_actions"].remove(missing)
                    else:
                        setup[missing] = False
                    self.rejected_setup("reset-termination", setup, variant=variant)

    def test_changed_setup_target_rejects_before_any_serial_bytes(self):
        """A fixture declared for another exact variant cannot authorize this target."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                self.rejected_setup(suite, recovery_setup("tusb320li"))

    def test_322_mutation_gates_remain_closed_while_console_prerequisites_are_legal(self):
        """Console reconnection may observe 322 without opening its reset or fault gates."""
        from tusb32x_hil_fixtures import fixture_prerequisite

        for suite in ("reset-termination", "fault"):
            with self.subTest(suite=suite):
                self.rejected_setup(suite, recovery_setup("tusb322i"), variant="tusb322i")
        fixture_prerequisite(ObservationPlan(variant="tusb322i", suites=("console",)),
                             recovery_setup("tusb322i"), EXPECTED)

    def test_original_console_requires_continuous_target_vdd_before_io(self):
        """A reconnect/reboot authorization cannot imply original-part supply continuity."""
        for variant in ("tusb320", "tusb320i"):
            with self.subTest(variant=variant):
                setup = recovery_setup(variant)
                setup["target_vdd_continuous"] = False
                self.rejected_setup("console", setup, variant=variant)


class RecoveryReconnectTests(unittest.TestCase):
    """Re-establish framing with fresh ownership after an explicitly finished old session."""

    def fresh_session(self, peer, artifacts, clock):
        """Construct a distinct unstarted session with fresh nonces over the same evidence log."""
        nonces = iter(f"reconnected_{index}" for index in range(1000))
        return runner.Session(peer, artifacts, clock=clock,
                              nonce_factory=lambda: next(nonces))

    def test_finished_old_session_cannot_control_new_session_even_when_tokens_restart(self):
        """New framing and identity do not adopt an earlier session's retained token."""
        session, peer, clock, artifacts = RecoveryExpectationTests.make_session(self)
        session.command("read")
        result = RecoveryExpectationTests.complete(self, session)
        old_token = result["token"]
        session.take_result()
        session.finish()
        peer = FixturePeer(clock)
        fresh = self.fresh_session(peer, artifacts, clock)
        fresh.begin(expected=EXPECTED)
        before = list(peer.writes)
        with self.assertRaises(runner.SessionBlocked):
            fresh.command("result take " + old_token)
        self.assertEqual(peer.writes, before)
        self.assertIsNone(fresh.operation)
        self.assertFalse(session.valid)
        fresh.command("job auto 0")
        fresh.command("read")
        newer = RecoveryExpectationTests.complete(self, fresh)
        self.assertEqual(newer["token"], old_token)
        self.assertNotEqual(newer["nonce"], result["nonce"])
        fresh.take_result()
        fresh.finish()

    def test_stale_old_success_cannot_establish_the_reconnected_session(self):
        """A genuine old terminal record is rejected when injected into a fresh handshake."""
        session, _, clock, artifacts = RecoveryExpectationTests.make_session(self)
        session.command("read")
        old_result = RecoveryExpectationTests.complete(self, session)
        session.take_result()
        session.finish()
        peer = FixturePeer(clock)
        def stale_after_marker(text, rows):
            """Inject the old success inside the newly claimed marker frame."""
            return rows[:1] + [old_result] + rows[1:] if text.startswith("hilmark ") else rows
        peer.response_hook = stale_after_marker
        fresh = self.fresh_session(peer, artifacts, clock)
        with self.assertRaises(runner.ProtocolError):
            fresh.begin(expected=EXPECTED)
        self.assertFalse(fresh.valid)
        self.assertFalse(any(text.startswith("result take") for text in peer.writes))
        self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)

    def test_changed_firmware_during_reconnect_blocks_following_mutation(self):
        """A fresh framing marker cannot authorize writes to changed firmware."""
        session, _, clock, artifacts = RecoveryExpectationTests.make_session(self)
        session.finish()
        peer = FixturePeer(clock)
        peer.identity["git"] = "f" * 40
        fresh = self.fresh_session(peer, artifacts, clock)
        with self.assertRaises(runner.SessionError):
            fresh.begin(expected=EXPECTED)
        before = list(peer.writes)
        with self.assertRaises(runner.SessionError):
            fresh.command("reset")
        self.assertEqual(peer.writes, before)
        self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)

    def test_owned_work_prevents_local_finish_before_any_reconnect(self):
        """An active or retained result cannot be abandoned to make reconnection appear clean."""
        for completed in (False, True):
            with self.subTest(completed=completed):
                session, peer, _, artifacts = RecoveryExpectationTests.make_session(self)
                session.command("reset")
                if completed:
                    RecoveryExpectationTests.complete(self, session)
                before = list(peer.writes)
                with self.assertRaises(runner.SessionBlocked):
                    session.finish()
                self.assertEqual(peer.writes, before)
                self.assertIsNotNone(session.operation)
                self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)

    def test_benign_human_reboot_prose_does_not_invalidate_machine_framing(self):
        """Ordinary help prose mentioning reboot/reset is not a firmware startup banner."""
        session, peer, _, _ = RecoveryExpectationTests.make_session(self)
        prose = b"New part resets staging; reboot requires explicit owner action.\n"
        peer.response_hook = lambda _, rows: prose + b"".join(map(encode_record, rows))
        self.assertEqual(session.command("version").end["code"], "OK")
        self.assertTrue(session.valid)

    def test_real_boot_banners_invalidate_old_session_and_prevent_cleanup_bytes(self):
        """Anchored device startup/reboot records invalidate correlation despite trailing OK."""
        for banner in (b"ESP-ROM:esp32s3-20210327\n", b"rst:0x1 (POWERON),boot:0x8\n",
                       b"TUSB_BOOT startup=1\n", b"Rebooting...\n",
                       b"\x1b[32mESP-ROM:esp32s3\x1b[0m\n",
                       b"  \x1b[31mRebooting...\x1b[0m\n"):
            with self.subTest(banner=banner):
                session, peer, _, artifacts = RecoveryExpectationTests.make_session(self)
                peer.response_hook = lambda _, rows: banner + b"".join(map(encode_record, rows))
                with self.assertRaises(runner.ProtocolError):
                    session.command("version")
                before = list(peer.writes)
                with self.assertRaises(runner.SessionError):
                    session.restore_local()
                self.assertEqual(peer.writes, before)
                self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)


class RecoveryExecutionBoundaryTests(unittest.TestCase):
    """Stop real recovery plans at failed identity, capture and external-action boundaries."""

    def fixture(self, suite, *, limits=None, peer_type=FixturePeer):
        """Construct an unstarted Session and a selected finite plan with durable evidence."""
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        clock = FakeClock()
        artifacts = Artifacts(Path(temporary.name), clock, limits=limits)
        self.addCleanup(artifacts.close)
        peer = peer_type(clock)
        nonces = iter(f"boundary_{index}" for index in range(1000))
        session = runner.Session(peer, artifacts, clock=clock,
                                 nonce_factory=lambda: next(nonces))
        plan = ObservationPlan(suites=(suite,))
        return session, peer, clock, artifacts, plan

    def execute(self, fixture, transition):
        """Invoke production execution; operator waits advance only the fake clock."""
        from tusb32x_hil_fixtures import run_fixture_plan

        session, _, clock, _, plan = fixture
        return run_fixture_plan(session, plan, recovery_setup(), EXPECTED,
                                transition=transition, clock=clock,
                                sleep=lambda seconds: setattr(clock, "now", clock() + seconds))

    def assert_saved_nonpassing(self, fixture):
        """Compare authoritative disk evidence to memory after an interrupted execution."""
        artifacts = fixture[3]
        report = artifacts.snapshot()
        saved = json.loads((artifacts.directory / "report.json").read_text(encoding="utf-8"))
        self.assertNotEqual(saved["exit_code"], 0)
        self.assertEqual(saved["exit_code"], report["exit_code"])
        self.assertFalse(fixture[1].mutation_history)
        self.assertFalse(any(text.startswith(("apply", "reset", "recover-dead-battery", "wreg"))
                             for text in fixture[1].writes))
        return saved

    def test_declined_initial_detach_preserves_baseline_and_never_mutates(self):
        """A real baseline cannot substitute for required explicit detached-fixture evidence."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                fixture = self.fixture(suite)
                seen = []
                def declined(step, deadline):
                    """Inspect the durable pre-action baseline before declining physical setup."""
                    seen.append(step["id"])
                    saved = json.loads((fixture[3].directory / "report.json").read_text())
                    self.assertIn("fixture_baseline", saved["metadata"])
                    self.assertLess(fixture[2](), deadline)
                    return False
                with self.assertRaises(PlanBlocked):
                    self.execute(fixture, declined)
                self.assertEqual(seen, ["initial-detached"])
                self.assert_saved_nonpassing(fixture)

    def test_changed_flashed_identity_blocks_each_plan_before_callbacks(self):
        """The firmware actually answering serial must match the requested commit."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                fixture = self.fixture(suite)
                fixture[1].identity["git"] = "f" * 40
                with self.assertRaises(runner.SessionError):
                    self.execute(fixture, lambda *_: self.fail("identity must precede actions"))
                self.assertEqual(fixture[1].attempts, 0)
                self.assert_saved_nonpassing(fixture)

    def test_changed_live_target_blocks_each_plan_before_callbacks(self):
        """A selected address is checked against the firmware's live target before work."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                fixture = self.fixture(suite)
                fixture[1].address = 0x67
                with self.assertRaises(PlanBlocked):
                    self.execute(fixture, lambda *_: self.fail("target must precede actions"))
                self.assertEqual(fixture[1].attempts, 0)
                self.assert_saved_nonpassing(fixture)

    def test_late_operator_confirmation_cannot_extend_any_recovery_plan(self):
        """A True response at the immutable transition deadline remains failed evidence."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                fixture = self.fixture(suite)
                def late(step, deadline):
                    """Advance virtual time exactly to the finite operator deadline."""
                    self.assertEqual(step["id"], "initial-detached")
                    fixture[2].now = deadline
                    return True
                with self.assertRaises(PlanFailure):
                    self.execute(fixture, late)
                self.assert_saved_nonpassing(fixture)

    def test_ctrl_c_at_external_boundary_keeps_saved_interruption_and_no_cleanup_write(self):
        """Each suite preserves interruption before its first physical action."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                fixture = self.fixture(suite)
                def interrupted(step, deadline):
                    """Interrupt the explicit action without any background or hardware work."""
                    self.assertEqual(step["id"], "initial-detached")
                    raise KeyboardInterrupt()
                with self.assertRaises(KeyboardInterrupt):
                    self.execute(fixture, interrupted)
                report = self.assert_saved_nonpassing(fixture)
                self.assertTrue(report["interrupted"])
                self.assertEqual(report["exit_code"], 130)

    def test_capture_exhaustion_stops_all_new_plans_before_device_mutation(self):
        """The bounded transcript cannot silently lose initial identity/baseline proof."""
        for suite in RECOVERY_SUITES:
            with self.subTest(suite=suite):
                fixture = self.fixture(suite, limits=CaptureLimits(1024, 1024))
                with self.assertRaises((runner.SessionError, EvidenceError)):
                    self.execute(fixture, lambda *_: self.fail("capture must precede actions"))
                report = self.assert_saved_nonpassing(fixture)
                self.assertTrue(report["capture"]["lost"])


class RecoveryPlanExecutionTests(unittest.TestCase):
    """Execute complete production recovery plans with independently named modeled actions."""

    def fixture(self, suite, variant="tusb320lai"):
        """Reuse artifact/session construction and vary only the missing recovery wire routes."""
        fixture = list(RecoveryExecutionBoundaryTests.fixture(self, suite, peer_type=RecoveryPeer))
        peer = fixture[1]
        peer.variant = variant
        peer.address = 0x60 if variant in {"tusb320", "tusb320i"} else 0x47
        peer.profile_fields = 155 if variant in {"tusb320", "tusb320i"} else (
            127 if variant == "tusb322i" else 255)
        fixture[4] = ObservationPlan(variant=variant, address=peer.address, suites=(suite,))
        return fixture

    def execute(self, fixture, *, stop=None, transition_fault=None, reconnect_fault=None):
        """Drive frozen physical steps and fresh sessions without replacing production helpers."""
        from tusb32x_hil_fixtures import run_fixture_plan

        session, peer, clock, artifacts, plan = fixture
        self.peers = [peer]
        self.steps = []
        current = {"peer": peer}
        def transition(step, deadline):
            """Change only the external state identified by this exact declared fixture action."""
            name = step["id"]
            self.steps.append(name)
            if name == stop:
                return False
            if transition_fault:
                answer = transition_fault(step, deadline, current["peer"])
                if answer is not None:
                    return answer
            target = current["peer"]
            if name.endswith("detached") or name.endswith("detach"):
                target.physical("DETACHED")
            elif name in {"fault-nack", "fault-disconnect"}:
                target.fail_operation = "profile.read"
                target.fail_code = "I2C_NACK_ADDR" if name == "fault-nack" else "I2C_ERROR"
            elif name in {"fault-nack-restore", "fault-disconnect-restore"}:
                target.fail_operation = None
            elif name in {"recovery-ready", "fault-power-cycle", "console-reconnect",
                          "console-reboot", "console-backpressure"}:
                pass  # Explicit modeled attestation; the runner never controls real hardware.
            else:
                raise AssertionError(f"undeclared recovery transition {name}")
            self.assertLess(clock(), deadline)
            return True
        def reconnect(old, deadline):
            """Create a fresh framing/ownership domain, retaining the same evidence and target."""
            self.assertFalse(old.valid)
            self.assertIsNone(old.operation)
            if reconnect_fault:
                answer = reconnect_fault(old, deadline)
                if answer is not None:
                    return answer
            target = RecoveryPeer(clock)
            target.variant, target.address = peer.variant, peer.address
            target.profile_fields = peer.profile_fields
            current["peer"] = target
            self.peers.append(target)
            serial_epoch = len(self.peers)
            nonces = iter(f"epoch_{serial_epoch}_{index}" for index in range(1000))
            return runner.Session(target, artifacts, clock=clock,
                                  nonce_factory=lambda: next(nonces))
        return run_fixture_plan(session, plan, recovery_setup(plan.variant), EXPECTED,
                                transition=transition, reconnect=reconnect, clock=clock,
                                sleep=lambda seconds: setattr(clock, "now", clock() + seconds))

    def test_complete_new_suites_and_original_recovery_use_owned_results_and_finite_actions(self):
        """Each plan reaches real software success while retaining hardware-model provenance."""
        for suite, variant in (("reset-termination", "tusb320lai"),
                               ("reset-termination", "tusb320"),
                               ("reset-termination", "tusb320i"),
                               ("fault", "tusb320lai"), ("console", "tusb320lai"),
                               ("console", "tusb322i"), ("console", "tusb320")):
            with self.subTest(suite=suite, variant=variant):
                fixture = self.fixture(suite, variant)
                summary = self.execute(fixture)
                report = fixture[3].snapshot()
                self.assertEqual(report["exit_code"], 0)
                self.assertEqual(report["checks"][suite]["status"], "PASS")
                self.assertEqual(report["metadata"]["hardware_cleanup"], "PASS")
                self.assertLessEqual(summary["transitions"], 64)
                self.assertEqual(self.steps[-1], "cleanup-detached")
                for target in self.peers:
                    self.assertIsNone(target.operation)
                    self.assertFalse(any(text.startswith("wreg") for text in target.writes))
                    self.assertTrue(all(used <= 1 for _, _, used in target.turn_history))
                commands = [text for target in self.peers for text in target.writes]
                if suite in {"fault", "console"}:
                    self.assertFalse(any(text.startswith("apply") for text in commands))
                if suite == "reset-termination" and variant in {"tusb320", "tusb320i"}:
                    self.assertEqual(sum(text.startswith("recover-dead-battery")
                                         for text in commands), 1)
                if suite == "console":
                    self.assertEqual(len(self.peers), 3)
                    self.assertIn("console_sessions", report["metadata"])

    def test_every_declined_transition_stays_incomplete_without_replay(self):
        """Each frozen operator boundary has a tested refusal path and durable stopped plan."""
        cases = (
            ("reset-termination", "tusb320", ("recovery-ready", "initial-detached",
                                              "reset-detach", "cleanup-detached")),
            ("fault", "tusb320lai", ("initial-detached", "fault-nack", "fault-nack-restore",
                                      "fault-disconnect", "fault-disconnect-restore",
                                      "fault-power-cycle", "fault-detach", "cleanup-detached")),
            ("console", "tusb320lai", ("initial-detached", "console-backpressure",
                                        "console-reconnect", "console-reboot", "console-detach",
                                        "cleanup-detached")),
        )
        for suite, variant, steps in cases:
            for stop in steps:
                with self.subTest(suite=suite, stop=stop):
                    fixture = self.fixture(suite, variant)
                    with self.assertRaises(PlanBlocked):
                        self.execute(fixture, stop=stop)
                    report = fixture[3].snapshot()
                    saved = json.loads((fixture[3].directory / "report.json").read_text())
                    self.assertNotEqual(report["exit_code"], 0)
                    self.assertEqual(saved["exit_code"], report["exit_code"])
                    self.assertEqual(self.steps[-1], stop)
                    self.assertNotEqual(report["metadata"]["hardware_cleanup"], "PASS")
                    if report["metadata"].get("device_cleanup_required"):
                        self.assertTrue(report["residual_uncertainty"])
                        self.assertIn("manual_reconciliation", report["metadata"])

    def test_uncertain_reset_write_is_taken_once_and_never_replayed_or_restored(self):
        """Accepted/error reset boundary remains primary failure after successful local cleanup."""
        fixture = self.fixture("reset-termination")
        peer = fixture[1]
        peer.fail_operation, peer.fail_code, peer.fail_at_callback = "reset", "I2C_TIMEOUT", 2
        with self.assertRaises(PlanFailure):
            self.execute(fixture)
        report = fixture[3].snapshot()
        self.assertEqual(report["primary"], "FAIL")
        self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
        self.assertTrue(report["residual_uncertainty"])
        result = report["metadata"]["last_fixture_result"]["result"]
        self.assertEqual((result["code"], result["effect"]), ("I2C_TIMEOUT", "INDETERMINATE"))
        self.assertEqual(sum(text.startswith("reset ") for text in peer.writes), 1)
        self.assertFalse(any(text.startswith("apply") for text in peer.writes))
        self.assertIsNone(peer.operation)
        self.assertEqual(report["checks"]["operation_payloads"]["status"], "FAIL")

    def test_historical_reset_acceptance_uncertainty_does_not_invent_unknown_reconcile(self):
        """Current read-only proof can succeed while historical reset acceptance stays unknown."""
        fixture = self.fixture("reset-termination")
        def historical_flag(text, rows):
            """Retain an independent historic flag while actual current profile is known."""
            return [dict(row, reset_acceptance_uncertain="1") if row["type"] == "DIAG"
                    else row for row in rows]
        fixture[1].response_hook = historical_flag
        self.execute(fixture)
        result = fixture[3].metadata["recovery_results"]["reconcile"]
        self.assertEqual((result["expected_code"], result["result"]["code"]), ("OK", "OK"))
        self.assertEqual(fixture[3].snapshot()["exit_code"], 0)

    def test_new_original_source_epoch_blocks_without_recovery_replay(self):
        """A power-cycle can invalidate earlier source-startup proof even after fresh identity."""
        for variant in ("tusb320", "tusb320i"):
            with self.subTest(variant=variant):
                fixture = self.fixture("fault", variant)
                with self.assertRaises(PlanBlocked):
                    self.execute(fixture)
                report = fixture[3].snapshot()
                self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
                self.assertTrue(report["residual_uncertainty"])
                self.assertEqual(self.steps[-1], "fault-power-cycle")
                self.assertFalse(any(text.startswith(("recover-dead-battery", "reset", "apply"))
                                     for text in fixture[1].writes))

    def test_new_original_ufp_epoch_needs_no_automatic_recovery_or_role_change(self):
        """An observed sink-only original-part configuration can finish the bounded fault plan."""
        fixture = self.fixture("fault", "tusb320")
        fixture[1].core_profile["mode"] = "ufp"
        self.execute(fixture)
        self.assertEqual(fixture[3].snapshot()["exit_code"], 0)
        self.assertFalse(any(text.startswith(("recover-dead-battery", "reset", "apply"))
                             for text in fixture[1].writes))

    def follow_group(self, fixture, suite, setup):
        """Start another real plan session on the same target and run evidence."""
        from test_tusb32x_fixtures import FixtureExecutionTests
        from tusb32x_hil_fixtures import run_fixture_plan

        _, peer, clock, artifacts, _ = fixture
        epoch = len(peer.writes)
        nonces = iter(f"group_{epoch}_{index}" for index in range(1000))
        session = runner.Session(peer, artifacts, clock=clock,
                                 nonce_factory=lambda: next(nonces))
        plan = ObservationPlan(variant=peer.variant, address=peer.address, suites=(suite,))
        ordinary = FixtureExecutionTests().transitions(peer)

        def transition(step, deadline):
            """Model the explicitly selected recovery readiness or ordinary fixture step."""
            if step["id"] == "recovery-ready":
                return clock() < deadline
            return ordinary(step, deadline)

        return run_fixture_plan(session, plan, setup, EXPECTED, transition=transition,
                                clock=clock, sleep=lambda seconds: setattr(
                                    clock, "now", clock() + seconds))

    def test_new_base_power_epoch_blocks_later_groups_with_old_readiness(self):
        """A sink-only fault PASS cannot reuse pre-cycle readiness for another group."""
        for variant in ("tusb320", "tusb320i"):
            for suite in ("accessory", "role-current"):
                with self.subTest(variant=variant, suite=suite):
                    fixture = self.fixture("fault", variant)
                    peer, artifacts = fixture[1], fixture[3]
                    peer.core_profile["mode"] = "ufp"
                    self.execute(fixture)
                    self.assertEqual(artifacts.snapshot()["exit_code"], 0)
                    setup = recovery_setup(variant)
                    setup["capabilities"].extend(("audio", "role-current"))
                    boundary = len(peer.writes)
                    with self.assertRaises(PlanBlocked):
                        self.follow_group(fixture, suite, setup)
                    self.assertFalse(any(text.startswith(("apply", "recover-dead-battery"))
                                         for text in peer.writes[boundary:]))
                    self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)

    def test_new_base_power_epoch_accepts_only_new_explicit_recovery(self):
        """Qualified recovery after power loss permits a later source accessory plan."""
        for variant in ("tusb320", "tusb320i"):
            with self.subTest(variant=variant):
                fixture = self.fixture("fault", variant)
                peer, artifacts = fixture[1], fixture[3]
                peer.core_profile["mode"] = "ufp"
                self.execute(fixture)
                setup = recovery_setup(variant)
                setup["capabilities"].append("audio")
                boundary = len(peer.writes)
                self.follow_group(fixture, "reset-termination", setup)
                recovery = next(index for index, text in enumerate(peer.writes[boundary:])
                                if text.startswith("recover-dead-battery"))
                self.follow_group(fixture, "accessory", setup)
                mutations = peer.writes[boundary:]
                self.assertLess(recovery, mutations.index("mode dfp"))
                self.assertEqual(sum(text.startswith("recover-dead-battery")
                                     for text in mutations), 1)
                self.assertEqual(artifacts.snapshot()["exit_code"], 0)

    def test_new_base_power_epoch_blocks_source_apply_within_same_group(self):
        """A combined fixture call cannot bypass the new supply-epoch guard."""
        from test_tusb32x_fixtures import FixtureExecutionTests
        from tusb32x_hil_fixtures import run_fixture_plan

        for variant in ("tusb320", "tusb320i"):
            with self.subTest(variant=variant):
                session, peer, clock, artifacts, _ = self.fixture("fault", variant)
                peer.core_profile["mode"] = "ufp"
                setup = recovery_setup(variant)
                setup["capabilities"].append("audio")
                plan = ObservationPlan(variant=variant, address=peer.address,
                                       suites=("fault", "accessory"))
                ordinary = FixtureExecutionTests().transitions(peer)

                def transition(step, deadline):
                    """Induce declared transport failures and attest the new power epoch."""
                    name = step["id"]
                    if name in {"fault-nack", "fault-disconnect"}:
                        peer.fail_operation = "profile.read"
                        peer.fail_code = ("I2C_NACK_ADDR" if name == "fault-nack"
                                          else "I2C_ERROR")
                    elif name in {"fault-nack-restore", "fault-disconnect-restore"}:
                        peer.fail_operation = None
                    elif name != "fault-power-cycle":
                        return ordinary(step, deadline)
                    return clock() < deadline

                with self.assertRaises(PlanBlocked):
                    run_fixture_plan(session, plan, setup, EXPECTED, transition=transition,
                                     clock=clock, sleep=lambda seconds: setattr(
                                         clock, "now", clock() + seconds))
                self.assertFalse(any(text.startswith(("apply", "recover-dead-battery"))
                                     for text in peer.writes))
                self.assertNotEqual(artifacts.snapshot()["exit_code"], 0)

    def test_new_explicit_base_recovery_supersedes_initial_missing_readiness(self):
        """A completed typed recovery supplies evidence absent from the initial setup."""
        for variant in ("tusb320", "tusb320i"):
            with self.subTest(variant=variant):
                fixture = self.fixture("reset-termination", variant)
                setup = recovery_setup(variant)
                setup["base_recovery_complete"] = False
                setup["capabilities"].append("audio")
                self.follow_group(fixture, "reset-termination", setup)
                self.follow_group(fixture, "accessory", setup)
                self.assertEqual(fixture[3].snapshot()["exit_code"], 0)

    def test_late_completion_evidence_cannot_certify_reset_or_original_recovery(self):
        """A successful code without the documented elapsed-time proof fails independently."""
        for variant in ("tusb320lai", "tusb320"):
            with self.subTest(variant=variant):
                fixture = self.fixture("reset-termination", variant)
                fixture[1].bad_elapsed = True
                with self.assertRaises(PlanFailure):
                    self.execute(fixture)
                report = fixture[3].snapshot()
                self.assertEqual(report["primary"], "FAIL")
                self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
                self.assertTrue(report["residual_uncertainty"])
                self.assertFalse(any(text.startswith("apply") for text in fixture[1].writes))

    def test_reset_adapter_evidence_rejects_extra_saturated_regressed_or_missing_counts(self):
        """A scalar reset success cannot conceal extra writes or unavailable callback evidence."""
        for damage in ("extra", "saturated", "regressed", "unavailable"):
            with self.subTest(damage=damage):
                fixture = self.fixture("reset-termination")
                peer = fixture[1]
                peer.adapter_writes = 3
                def damaged_bus(text, rows):
                    """Alter only post-reset backend evidence, preserving the actual result."""
                    if text == "bus" and peer.reset_count:
                        rows = [dict(row) for row in rows]
                        for row in rows:
                            if row["type"] == "BUS":
                                if damage == "unavailable":
                                    row["adapter_available"] = "0"
                                else:
                                    row["adapter_writes"] = {
                                        "extra": str(peer.adapter_writes + 1),
                                        "saturated": str((1 << 32) - 1),
                                        "regressed": "0",
                                    }[damage]
                    return rows
                peer.response_hook = damaged_bus
                with self.assertRaises(PlanFailure):
                    self.execute(fixture)
                report = fixture[3].snapshot()
                self.assertEqual(report["primary"], "FAIL")
                self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
                self.assertTrue(report["residual_uncertainty"])
                self.assertEqual(sum(text.startswith("reset ") for text in peer.writes), 1)
                self.assertFalse(any(text.startswith(("apply", "recover-dead-battery"))
                                     for text in peer.writes))

    def test_one_disabled_control_cannot_be_reported_as_overall_termination_enabled(self):
        """Contradictory aggregate evidence fails while preserving the distinct control bits."""
        fixture = self.fixture("reset-termination")
        peer = fixture[1]
        def contradictory_termination(text, rows):
            """Corrupt only aggregate termination once an independent control is disabled."""
            if text == "profile show" and any(peer.core_profile[key] == "off"
                                              for key in ("termination", "rd_rp")):
                return [dict(row, termination="ENABLED") if row["type"] == "CONFIG_DETAIL"
                        else row for row in rows]
            return rows
        peer.response_hook = contradictory_termination
        with self.assertRaises(PlanFailure):
            self.execute(fixture)
        self.assertEqual(fixture[3].snapshot()["primary"], "FAIL")
        self.assertTrue(any(peer.core_profile[key] == "off" for key in ("termination", "rd_rp")))
        self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")

    def test_fault_wrong_error_or_unexpected_success_never_satisfies_the_induced_nack(self):
        """Expected address NACK requires that exact callback failure with no write effects."""
        for actual in (None, "I2C_BUSY"):
            with self.subTest(actual=actual):
                fixture = self.fixture("fault")
                def wrong_fault(step, deadline, peer):
                    """Attest the physical action while independently altering its wire outcome."""
                    if step["id"] == "fault-nack":
                        peer.fail_operation = "profile.read" if actual else None
                        peer.fail_code = actual or "OK"
                        return True
                    return None
                with self.assertRaises(PlanFailure):
                    self.execute(fixture, transition_fault=wrong_fault)
                report = fixture[3].snapshot()
                self.assertEqual(report["primary"], "FAIL")
                self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
                self.assertNotIn("fault-nack-restore", self.steps)
                self.assertFalse(any(text.startswith("apply") for text in fixture[1].writes))

    def test_console_loss_prevents_any_reconnect_and_preserves_failure(self):
        """A lost response cannot be repaired by a later fresh serial session."""
        fixture = self.fixture("console")
        def dropped(step, deadline, peer):
            """Expose modeled whole-record output loss immediately after the armed stall."""
            if step["id"] == "console-backpressure":
                peer.console_drops = 1
            return None
        with self.assertRaises(PlanFailure):
            self.execute(fixture, transition_fault=dropped)
        self.assertEqual(fixture[3].snapshot()["primary"], "FAIL")
        self.assertNotIn("console-reconnect", self.steps)
        self.assertEqual(len(self.peers), 1)

    def test_reconnect_reusing_old_session_is_rejected_without_further_writes(self):
        """An owner callback must return a new unstarted framing domain."""
        fixture = self.fixture("console")
        before = []
        def old_session(old, deadline):
            """Return the finished object to model an invalid reconnect implementation."""
            before.extend(old.io.writes)
            return old
        with self.assertRaises(PlanFailure):
            self.execute(fixture, reconnect_fault=old_session)
        self.assertEqual(fixture[1].writes, before)
        self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")
        self.assertTrue(fixture[3].residual_uncertainty)

    def test_invalid_reconnect_owner_return_retains_concrete_failure_and_cleanup_evidence(self):
        """A malformed reconnect callback cannot replace the session or mask its saved failure."""
        fixture = self.fixture("console")
        with self.assertRaises(PlanFailure):
            self.execute(fixture, reconnect_fault=lambda *_: object())
        report = fixture[3].snapshot()
        saved = json.loads((fixture[3].directory / "report.json").read_text())
        self.assertEqual(report["primary"], "FAIL")
        self.assertEqual(saved["primary"], "FAIL")
        self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
        self.assertTrue(report["residual_uncertainty"])

    def test_reconnect_identity_or_target_change_blocks_before_new_device_callbacks(self):
        """Fresh serial framing cannot authorize work against a changed build or address."""
        for changed in ("build", "address"):
            with self.subTest(changed=changed):
                fixture = self.fixture("console")
                new_peers = []
                def wrong_target(old, deadline):
                    """Return fresh framing over an independently changed reported target."""
                    peer = RecoveryPeer(fixture[2])
                    if changed == "build":
                        peer.identity["build"] = "different_build"
                    else:
                        peer.address = 0x67
                    new_peers.append(peer)
                    nonces = iter(f"changed_{index}" for index in range(100))
                    return runner.Session(peer, fixture[3], clock=fixture[2],
                                          nonce_factory=lambda: next(nonces))
                with self.assertRaises((PlanBlocked, runner.SessionError)):
                    self.execute(fixture, reconnect_fault=wrong_target)
                self.assertEqual(len(new_peers), 1)
                self.assertEqual(new_peers[0].attempts, 0)
                self.assertFalse(any(text.startswith(("init", "apply", "reset"))
                                     for text in new_peers[0].writes))
                self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")
                self.assertTrue(fixture[3].residual_uncertainty)

    def test_reconnect_callback_interruption_cannot_certify_device_cleanup(self):
        """Ctrl-C after authorized disconnect preserves the old finished session's evidence."""
        fixture = self.fixture("console")
        def interrupted(old, deadline):
            """Interrupt reopening before a new Session or cleanup command can be sent."""
            raise KeyboardInterrupt()
        with self.assertRaises(KeyboardInterrupt):
            self.execute(fixture, reconnect_fault=interrupted)
        report = fixture[3].snapshot()
        self.assertEqual(report["exit_code"], 130)
        self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
        self.assertEqual(self.steps[-1], "console-reconnect")
        self.assertTrue(report["residual_uncertainty"])

    def test_unrelated_raw_success_cannot_repair_fault_invalidated_trust(self):
        """A backend returning bytes must not make configuration usable after physical loss."""
        fixture = self.fixture("fault")
        peer = fixture[1]
        def false_repair(text, rows):
            """Model an incorrect firmware trust promotion after an unrelated raw read."""
            if text == "reg 0" and peer.config_unknown:
                peer.config_unknown = False
            return rows
        peer.response_hook = false_repair
        with self.assertRaises(PlanFailure):
            self.execute(fixture)
        self.assertEqual(fixture[3].snapshot()["primary"], "FAIL")
        self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")
        self.assertNotIn("fault-disconnect", self.steps)
        self.assertFalse(any(text.startswith("apply") for text in peer.writes))

    def test_changed_target_after_fault_or_before_cleanup_never_restores_profile(self):
        """External attestations cannot hide a changed live address from target revalidation."""
        for suite, step_id in (("fault", "fault-nack-restore"),
                               ("reset-termination", "cleanup-detached")):
            with self.subTest(suite=suite):
                fixture = self.fixture(suite)
                def changed(step, deadline, peer):
                    """Change the live target exactly at the named physical boundary."""
                    if step["id"] == step_id:
                        peer.address = 0x67
                    return None
                with self.assertRaises(PlanBlocked):
                    self.execute(fixture, transition_fault=changed)
                self.assertEqual(self.steps[-1], step_id)
                self.assertEqual(fixture[3].metadata["hardware_cleanup"], "BLOCKED")
                self.assertTrue(fixture[3].residual_uncertainty)

    def test_ctrl_c_after_reset_write_keeps_primary_interruption_and_manual_cleanup(self):
        """Interruption after modeled effects cannot trigger role restoration or reset retry."""
        fixture = self.fixture("reset-termination")
        def interrupted(step, deadline, peer):
            """Interrupt after the reset and control experiments at their final detach boundary."""
            if step["id"] == "reset-detach":
                raise KeyboardInterrupt()
            return None
        with self.assertRaises(KeyboardInterrupt):
            self.execute(fixture, transition_fault=interrupted)
        report = fixture[3].snapshot()
        self.assertEqual(report["exit_code"], 130)
        self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
        self.assertTrue(report["residual_uncertainty"])
        self.assertEqual(self.steps[-1], "reset-detach")

    def test_original_recovery_has_durable_actual_baseline_before_mutation(self):
        """Stable-supply attestation cannot precede the saved actual recovery baseline."""
        fixture = self.fixture("reset-termination", "tusb320")
        checked = []
        def before_recovery(step, deadline, peer):
            """Inspect disk before the typed recovery operation can send its first write."""
            if step["id"] == "recovery-ready":
                report = json.loads((fixture[3].directory / "report.json").read_text())
                baseline = report["metadata"]["fixture_baseline"]
                profile = next(row for row in baseline["profiles"]
                               if row["type"] == "PROFILE" and row["source"] == "observed")
                self.assertEqual(profile["variant"], "tusb320")
                self.assertEqual(peer.adapter_writes, 0)
                checked.append(True)
            return None
        self.execute(fixture, transition_fault=before_recovery)
        self.assertEqual(checked, [True])
        observed = fixture[3].metadata["fixture_observations"]["reset-termination"][
            "recovery-ready"]["port"]
        self.assertEqual(observed["attachment"], "DETACHED")
        self.assertEqual(observed["invalidated"], "0")

    def test_original_recovery_requires_current_port_proof_before_writes(self):
        """Supply readiness cannot substitute for a fresh detached fixture observation."""
        for damaged in ({"invalidated": "1"}, {"valid_fields": "0"}):
            with self.subTest(damaged=damaged):
                fixture = self.fixture("reset-termination", "tusb320")
                def invalidate_port(step, deadline, peer):
                    """Invalidate only the observation following the recovery readiness step."""
                    if step["id"] == "recovery-ready":
                        peer.port_fault.update(damaged)
                    return None
                with self.assertRaises(PlanFailure):
                    self.execute(fixture, transition_fault=invalidate_port)
                self.assertEqual(fixture[1].adapter_writes, 0)
                self.assertNotIn("recover-dead-battery", fixture[1].operation_history)

    def test_original_recovery_error_retains_history_and_never_retries_or_resets(self):
        """An error after the first qualified recovery write needs manual reconciliation."""
        fixture = self.fixture("reset-termination", "tusb320")
        peer = fixture[1]
        peer.fail_operation, peer.fail_code = "recover-dead-battery", "I2C_TIMEOUT"
        peer.fail_at_callback = 2
        with self.assertRaises(PlanFailure):
            self.execute(fixture)
        report = fixture[3].snapshot()
        self.assertTrue(peer.recovery_unresolved)
        self.assertEqual(report["primary"], "FAIL")
        self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
        self.assertTrue(report["residual_uncertainty"])
        self.assertEqual(sum(text.startswith("recover-dead-battery") for text in peer.writes), 1)
        self.assertFalse(any(text.startswith(("reset ", "apply", "reconcile"))
                             for text in peer.writes))
        result = report["metadata"]["last_fixture_result"]["result"]
        self.assertEqual((result["code"], result["effect"]), ("I2C_TIMEOUT", "INDETERMINATE"))


class RecoverySetupTests(unittest.TestCase):
    """Validate exact external-action declarations before any serial backend is opened."""

    def load(self, setup, *, suites=RECOVERY_SUITES):
        """Write a bounded test declaration and invoke the public setup loader."""
        from tusb32x_hil_fixture_setup import load_setup

        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixture.json"
            setup = dict(setup)
            setup.pop("id", None)
            path.write_text(json.dumps(setup), encoding="utf-8")
            return load_setup(path, "recovery_model", setup["variant"], suites)

    def test_complete_recovery_declaration_and_nine_suite_selection_load(self):
        """All supported suite names and individually authorized actions survive parsing."""
        suites = ("readonly", "soak", "orientation", "accessory", "role-current", "interrupt",
                  *RECOVERY_SUITES)
        setup = recovery_setup()
        loaded = self.load(setup, suites=suites)
        self.assertEqual(loaded["authorized_actions"], list(AUTHORIZED_ACTIONS))
        self.assertTrue(loaded["stable_vdd"])
        self.assertTrue(loaded["source_startup"])

    def test_action_list_requires_exact_unique_bounded_strings(self):
        """Unknown, duplicate, malformed and wrongly typed authorizations cannot be guessed."""
        for actions in (None, True, "reset", {}, ["all"], ["RESET"], ["reset "],
                        ["reset", "reset"], [True], ["power-cycle\n"],
                        list(AUTHORIZED_ACTIONS) * 2):
            with self.subTest(actions=actions), self.assertRaises(ValueError):
                setup = recovery_setup()
                setup["authorized_actions"] = actions
                self.load(setup)

    def test_supply_and_source_flags_require_actual_booleans(self):
        """Truthy strings/numbers cannot authorize original-part power prerequisites."""
        for field in ("stable_vdd", "source_startup", "target_vdd_continuous"):
            for value in (None, 0, 1, "true", [], {}):
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    setup = recovery_setup("tusb320")
                    setup[field] = value
                    self.load(setup, suites=("reset-termination",))

    def test_expected_fault_codes_require_exact_terminal_transport_errors(self):
        """A backend failure phase is declared exactly and cannot use generic success/status."""
        for field in ("nack_code", "disconnect_code"):
            for value in (None, True, "OK", "TIMEOUT", "CANCELLED", "I2C_UNKNOWN",
                          "I2C_ERROR ", ["I2C_ERROR"]):
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    setup = recovery_setup()
                    setup[field] = value
                    self.load(setup, suites=("fault",))


class RecoveryInvocationTests(unittest.TestCase):
    """Exercise the real command-line transport/reconnect owner with existing wire fixtures."""

    def invoke(self, suites=RECOVERY_SUITES, *, extra=(), port=False, setup="default",
               identity=True, reopen_fault=None):
        """Run main with finite injected time and preserve actual setup/session/plan behavior."""
        import tusb32x_hil_fixtures as fixture_plans
        import tusb32x_hil_plans as observation_plans
        import test_tusb32x_fixtures as shared_fixtures

        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        directory = Path(temporary.name)
        output = directory / "evidence"
        clock = FakeClock()
        arguments = ["--out", str(output), "--count", "1", "--interval-ms", "1"]
        if port:
            arguments.extend(("--port", "COM_RECOVERY", "--baud", "230400",
                              "--dtr", "off", "--rts", "on", "--write-timeout", "0.25",
                              "--response-bytes", "123456"))
        for suite in suites:
            arguments.extend(("--suite", suite))
        if setup is not None:
            values = recovery_setup() if setup == "default" else dict(setup)
            values.pop("id", None)
            if setup == "default":
                values["capabilities"] = list(dict.fromkeys(
                    [*setup_for()["capabilities"], *RECOVERY_SUITES]))
            path = directory / "fixture.json"
            path.write_text(json.dumps(values), encoding="utf-8")
            arguments.extend(("--fixture", "recovery_main", "--fixture-config", str(path)))
        if identity:
            arguments.extend(("--expect-git", EXPECTED["git"], "--expect-framework", "host",
                              "--expect-target", "native"))
        arguments.extend(extra)
        original_session = runner.Session
        original_fixture = fixture_plans.run_fixture_plan
        original_observation = observation_plans.run_plan
        evidence = {"peers": [], "sessions": [], "opens": [], "groups": [], "closes": []}
        current = {}

        def opened(selected_port, **options):
            """Return a distinct simulated port for every real main-owned opening attempt."""
            evidence["opens"].append((selected_port, options))
            if len(evidence["opens"]) == 2 and reopen_fault == "exception":
                raise OSError("modeled serial reopen failed")
            peer = RecoveryPeer(clock)
            peer.settings = {"port": selected_port, "baud": options["baud"],
                             "dtr": options["dtr"], "rts": options["rts"]}
            original_close = peer.close
            def closed():
                """Count every main-owned serial closure while preserving backend behavior."""
                evidence["closes"].append(peer)
                return original_close()
            peer.close = closed
            evidence["peers"].append(peer)
            current["peer"] = peer
            if len(evidence["opens"]) == 2 and reopen_fault == "late":
                clock.now += 1000
            return peer

        def session(transport, artifacts, **options):
            """Keep actual Session guards and preserve the supplied timeout/capture config."""
            value = original_session(transport, artifacts, clock=clock, **options)
            evidence["sessions"].append(value)
            return value

        def transition(step, deadline):
            """Apply exactly named external states using the shared existing fixture helpers."""
            name, peer = step["id"], current["peer"]
            if name in {"fault-nack", "fault-disconnect"}:
                peer.fail_operation = "profile.read"
                peer.fail_code = "I2C_NACK_ADDR" if name == "fault-nack" else "I2C_ERROR"
            elif name in {"fault-nack-restore", "fault-disconnect-restore"}:
                peer.fail_operation = None
            elif name in {"fault-power-cycle", "console-backpressure", "console-reconnect",
                          "console-reboot", "recovery-ready"}:
                pass
            else:
                return shared_fixtures.FixtureExecutionTests().transitions(peer)(step, deadline)
            return clock() < deadline

        def sleep(seconds):
            """Advance virtual time under the unchanged whole-run deadline."""
            clock.now += seconds

        def fixture(session, plan, *args, **options):
            """Run the production plan, preserving main's actual reconnect factory."""
            evidence["groups"].append((plan.suites, options["run_deadline"]))
            options.update(clock=clock, sleep=sleep, transition=transition)
            return original_fixture(session, plan, *args, **options)

        def observation(session, plan, *args, **options):
            """Run ordinary observation groups using the same finite model and deadline."""
            evidence["groups"].append((plan.suites, options["run_deadline"]))
            options.update(clock=clock, sleep=sleep)
            return original_observation(session, plan, *args, **options)

        with ExitStack() as stack:
            stack.enter_context(redirect_stdout(io.StringIO()))
            stack.enter_context(redirect_stderr(io.StringIO()))
            stack.enter_context(mock.patch.object(runner, "host_identity", return_value={
                "git": "f" * 40, "tree": "dirty"}))
            stack.enter_context(mock.patch.object(runner.time, "monotonic", side_effect=clock))
            stack.enter_context(mock.patch.object(runner, "Session", side_effect=session))
            stack.enter_context(mock.patch.object(fixture_plans, "run_fixture_plan", new=fixture))
            stack.enter_context(mock.patch.object(observation_plans, "run_plan", new=observation))
            opener = stack.enter_context(mock.patch.object(
                runner, "open_serial", side_effect=opened))
            code = runner.main(arguments)
        report_path = output / "report.json"
        report = json.loads(report_path.read_text()) if report_path.exists() else None
        return code, report, evidence, opener

    def test_dry_run_all_new_suites_needs_no_port_and_saves_concrete_not_run(self):
        """A portless explicit recovery plan preserves every action without claiming hardware."""
        code, report, evidence, opened = self.invoke(extra=("--dry-run",))
        self.assertEqual(code, 2)
        opened.assert_not_called()
        self.assertFalse(evidence["sessions"])
        self.assertEqual(report["metadata"]["serial_requested"]["port"], None)
        self.assertEqual(report["metadata"]["plan"]["suites"], list(RECOVERY_SUITES))
        self.assertEqual(report["checks"]["hardware"]["status"], "NOT RUN")
        for suite in RECOVERY_SUITES:
            self.assertTrue(any(action.startswith(suite + ":")
                                for action in report["metadata"]["plan"]["actions"]))

    def test_missing_or_malformed_authorization_and_identity_reject_before_open(self):
        """The main entrypoint validates physical authorization before any backend constructor."""
        missing = recovery_setup()
        missing.pop("authorized_actions")
        malformed = recovery_setup()
        malformed["authorized_actions"] = ["all"]
        for options in ({"setup": missing}, {"setup": malformed}, {"identity": False}):
            with self.subTest(options=options):
                code, report, evidence, opened = self.invoke(port=True, **options)
                self.assertEqual(code, 2)
                opened.assert_not_called()
                self.assertFalse(evidence["peers"])
                if report is not None:
                    self.assertNotEqual(report["primary"], "PASS")

    def test_main_console_reopens_explicit_port_with_identical_settings_and_closes_all(self):
        """Both reconnects use fresh objects and retain serial and Session configuration."""
        code, report, evidence, opened = self.invoke(("console",), port=True)
        self.assertEqual(code, 0)
        self.assertEqual(opened.call_count, 3)
        self.assertEqual(len({id(peer) for peer in evidence["peers"]}), 3)
        self.assertEqual(len({id(session) for session in evidence["sessions"]}), 3)
        for selected_port, options in evidence["opens"]:
            self.assertEqual(selected_port, "COM_RECOVERY")
            self.assertEqual((options["dtr"], options["rts"], options["baud"]),
                             ("off", "on", 230400))
            self.assertEqual(options["timeouts"].write, 0.25)
        for session in evidence["sessions"]:
            self.assertEqual(session.timeouts.write, 0.25)
            self.assertEqual(session.limits.response_bytes, 123456)
            self.assertFalse(session.valid)
            self.assertIsNone(session.operation)
        self.assertEqual(evidence["closes"], evidence["peers"])
        self.assertTrue(all(peer.closed for peer in evidence["peers"]))
        self.assertEqual(report["cleanup"], "PASS")
        self.assertEqual(len(report["metadata"]["console_sessions"]), 2)

    def test_main_reopen_exception_or_late_return_is_saved_without_retry(self):
        """Opening failures retain evidence and close every handle actually returned by backend."""
        for fault in ("exception", "late"):
            with self.subTest(fault=fault):
                code, report, evidence, opened = self.invoke(("console",), port=True,
                                                            reopen_fault=fault)
                self.assertNotEqual(code, 0)
                self.assertNotEqual(report["primary"], "PASS")
                self.assertEqual(opened.call_count, 2)
                self.assertEqual(evidence["closes"], evidence["peers"])
                self.assertTrue(all(peer.closed for peer in evidence["peers"]))
                self.assertEqual(report["metadata"]["hardware_cleanup"], "BLOCKED")
                self.assertTrue(report["residual_uncertainty"])

    def test_all_nine_mixed_suites_retain_group_evidence_under_one_bounded_deadline(self):
        """Repeated selection preserves all suite proofs while each group gets fresh ownership."""
        suites = ("readonly", "soak", "orientation", "accessory", "role-current", "interrupt",
                  *RECOVERY_SUITES)
        code, report, evidence, _ = self.invoke(suites, port=True)
        self.assertEqual(code, 0)
        self.assertEqual(report["metadata"]["selected_plan"]["suites"], list(suites))
        self.assertEqual([group[0] for group in evidence["groups"]],
                         [("readonly", "soak"), ("orientation", "accessory", "role-current",
                                                  "interrupt"), ("reset-termination",),
                          ("fault",), ("console",)])
        self.assertEqual(len({deadline for _, deadline in evidence["groups"]}), 1)
        for suite in suites:
            self.assertEqual(report["checks"][suite]["status"], "PASS")
        self.assertLess(len(json.dumps(report).encode("utf-8")), 512 * 1024)
        self.assertTrue(all(peer.closed for peer in evidence["peers"]))


if __name__ == "__main__":
    unittest.main()
