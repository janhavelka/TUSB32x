"""Exercise fixture command-line guards and complete entrypoint evidence without hardware.

The real main, Session, setup loader and production plans run over the existing
reactive peer. Only transport opening, host identity, clock/sleep and explicitly
modeled physical transitions are injected. No serial device or real wait is used.
"""

from __future__ import annotations

from contextlib import ExitStack, redirect_stderr, redirect_stdout
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import tusb32x_hil_fixtures as fixtures
import tusb32x_hil_plans as plans
import tusb32x_hil_runner as runner
import test_tusb32x_fixtures as fixture_tests
from test_tusb32x_fixtures import FixturePeer, setup_for
from test_tusb32x_hil_runner import FakeClock, GIT


class FixtureInvocationTests(unittest.TestCase):
    """Require truthful saved outcomes from the actual repeated-suite command-line owner."""

    def invoke(self, *, suites=("orientation",), setup="default", extra=(), identity=True,
               fault=None, raw_setup=None):
        """Return exit, peer, saved report and invocation evidence from one isolated run."""
        temporary = tempfile.TemporaryDirectory(prefix="tusb32x-fixture-invocation-")
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        directory = root / "evidence"
        clock = FakeClock()
        peer = FixturePeer(clock)
        peer.settings = {"port": "COM_TEST", "dtr": "unchanged", "rts": "unchanged"}
        if fault == "mutation":
            peer.fail_operation = "apply"
            peer.fail_at_callback = 2
        if fault == "unknown-identity":
            peer.identity = dict(git="unknown", tree="unknown", build="unknown")
        arguments = ["--port", "COM_TEST", "--out", str(directory)]
        for suite in suites:
            arguments.extend(("--suite", suite))
        if setup is not None:
            values = dict(setup_for() if setup == "default" else setup)
            values.pop("id", None)
            values["wiring"] += "; VBUS_DET explicitly modeled, no physical connection"
            setup_path = root / "setup.json"
            setup_path.write_text(raw_setup if raw_setup is not None else json.dumps(values),
                                  encoding="utf-8")
            arguments.extend(("--fixture", "modeled_fixture", "--fixture-config",
                              str(setup_path)))
        if identity:
            arguments.extend(("--expect-git", GIT, "--expect-framework", "host",
                              "--expect-target", "native"))
        arguments.extend(extra)
        original_session = runner.Session
        original_fixture = fixtures.run_fixture_plan
        original_observation = plans.run_plan
        physical = fixture_tests.FixtureExecutionTests().transitions(peer)
        evidence = {"sessions": [], "groups": [], "steps": [], "clock": clock}

        def sleep(seconds):
            """Advance only the same virtual clock used by plans, Session and the peer."""
            clock.now += seconds

        def transition(step, deadline):
            """Apply explicit modeled observations or interrupt after a confirmed mutation."""
            evidence["steps"].append(step["id"])
            if fault == "interrupt" and step["id"] == "orientation-cc1":
                raise KeyboardInterrupt()
            return physical(step, deadline)

        def session(transport, artifacts, **kwargs):
            """Retain actual Session behavior while sharing the injected monotonic clock."""
            result = original_session(transport, artifacts, clock=clock, **kwargs)
            evidence["sessions"].append(result)
            return result

        def fixture(session, plan, *args, **kwargs):
            """Run the complete production fixture plan with finite external observations."""
            evidence["groups"].append((plan.suites, clock(), kwargs.get(
                "run_deadline", clock() + plan.run_timeout)))
            kwargs.update(clock=clock, sleep=sleep, transition=transition)
            return original_fixture(session, plan, *args, **kwargs)

        def observation(session, plan, *args, **kwargs):
            """Run complete observation groups under the same immutable whole-run bound."""
            evidence["groups"].append((plan.suites, clock(), kwargs.get(
                "run_deadline", clock() + plan.run_timeout)))
            kwargs.update(clock=clock, sleep=sleep)
            result = original_observation(session, plan, *args, **kwargs)
            if fault == "between-groups":
                clock.now += 100
            return result

        with ExitStack() as stack:
            stack.enter_context(redirect_stdout(io.StringIO()))
            stack.enter_context(redirect_stderr(io.StringIO()))
            stack.enter_context(mock.patch.object(runner, "host_identity", return_value={
                "git": "f" * 40, "tree": "dirty"}))
            stack.enter_context(mock.patch.object(runner.time, "monotonic", side_effect=clock))
            stack.enter_context(mock.patch.object(runner, "Session", side_effect=session))
            stack.enter_context(mock.patch.object(fixtures, "run_fixture_plan", new=fixture))
            stack.enter_context(mock.patch.object(plans, "run_plan", new=observation))
            opened = stack.enter_context(mock.patch.object(
                runner, "open_serial", return_value=peer))
            code = runner.main(arguments)
        path = directory / "report.json"
        report = json.loads(path.read_text("utf-8")) if path.exists() else None
        return code, peer, report, directory, opened, evidence

    def test_all_four_dry_run_is_concrete_and_never_opens_transport(self):
        """Repeated explicit mutation suites produce a finite no-I/O plan with exit two."""
        suites = ("orientation", "accessory", "role-current", "interrupt")
        code, peer, report, directory, opened, evidence = self.invoke(
            suites=suites, extra=("--dry-run",))
        self.assertEqual(code, 2)
        opened.assert_not_called()
        self.assertFalse(peer.writes)
        self.assertFalse(evidence["groups"])
        self.assertEqual(report["metadata"]["plan"]["suites"], list(suites))
        self.assertEqual(report["metadata"]["plan"]["hardware"], "NOT RUN")
        self.assertEqual(report["checks"]["hardware"]["status"], "NOT RUN")
        self.assertGreater(report["metadata"]["plan"]["run_timeout"], 0)
        actions = "\n".join(report["metadata"]["plan"]["actions"])
        for suite in suites:
            self.assertIn(suite + ":", actions)
        self.assertTrue((directory / "summary.md").is_file())

    def test_missing_fixture_saves_not_run_without_opening(self):
        """Absent external equipment leaves selected checks NOT RUN in a saved report."""
        code, peer, report, directory, opened, _ = self.invoke(setup=None)
        self.assertEqual(code, 2)
        opened.assert_not_called()
        self.assertFalse(peer.writes)
        self.assertEqual(report["checks"]["orientation"]["status"], "NOT RUN")
        self.assertNotEqual(report["primary"], "PASS")
        self.assertTrue((directory / "summary.md").is_file())

    def test_unpaired_setup_options_are_rejected_before_open(self):
        """Either fixture option without its paired declaration is an invocation error."""
        for extra in (("--fixture", "unpaired"),
                      ("--fixture-config", "missing-setup.json")):
            with self.subTest(extra=extra):
                code, peer, report, _, opened, _ = self.invoke(setup=None, extra=extra)
                self.assertEqual(code, 2)
                opened.assert_not_called()
                self.assertFalse(peer.writes)
                self.assertIsNone(report)

    def test_invalid_setup_and_supply_are_rejected_before_open(self):
        """Strict setup parsing and variant-specific VDD requirements precede serial work."""
        cases = [({"supply_mv": 3499}, ("role-current",), ()),
                 ({"supply_mv": 5001}, ("orientation",), ()),
                 ({"supply_mv": True}, ("orientation",), ()),
                 ({"variant": "tusb322i", "supply_mv": 4400}, ("orientation",),
                  ("--variant", "tusb322i")),
                 ({"variant": "tusb320hi"}, ("orientation",), ())]
        for changes, suites, extra in cases:
            with self.subTest(changes=changes):
                setup = dict(setup_for(), **changes)
                code, peer, report, _, opened, _ = self.invoke(
                    setup=setup, suites=suites, extra=extra)
                self.assertEqual(code, 2)
                opened.assert_not_called()
                self.assertFalse(peer.writes)
                self.assertIsNone(report)
        for raw in ('{"board": "a", "board": "b"}', "[]", "{invalid"):
            with self.subTest(raw=raw):
                code, _, report, _, opened, _ = self.invoke(raw_setup=raw)
                self.assertEqual(code, 2)
                self.assertIsNone(report)
                opened.assert_not_called()

    def test_missing_expected_identity_and_capability_stop_before_open(self):
        """Missing mutation identity and explicit fixture capabilities are pre-open guards."""
        for options in ({"identity": False},
                        {"identity": False, "extra": ("--expect-git", GIT)},
                        {"identity": False, "extra": ("--expect-framework", "host",
                                                       "--expect-target", "native")},
                        {"setup": dict(setup_for(), capabilities=[])},
                        {"setup": dict(setup_for(), startup_ready=False)}):
            with self.subTest(options=options):
                code, peer, report, _, opened, _ = self.invoke(**options)
                self.assertEqual(code, 2)
                opened.assert_not_called()
                self.assertFalse(peer.writes)
                self.assertNotEqual(report["primary"], "PASS")

    def test_base_variant_extended_accessories_are_blocked_before_open(self):
        """An explicit base-part declaration cannot authorize later-family debug fixtures."""
        code, peer, report, _, opened, _ = self.invoke(
            suites=("accessory",), setup=setup_for("tusb320"),
            extra=("--variant", "tusb320", "--address", "0x60"))
        self.assertEqual(code, 2)
        opened.assert_not_called()
        self.assertFalse(peer.writes)
        self.assertIn("extended accessory", report["checks"]["session"]["detail"])

    def test_unresolved_322_writes_are_blocked_before_open(self):
        """An otherwise valid 322 setup cannot authorize unresolved profile/IRQ actions."""
        code, peer, report, _, opened, _ = self.invoke(
            setup=setup_for("tusb322i"), extra=("--variant", "tusb322i"))
        self.assertEqual(code, 2)
        opened.assert_not_called()
        self.assertFalse(peer.writes)
        self.assertIn("gated", report["checks"]["session"]["detail"])

    def test_actual_main_runs_all_four_suites_and_saves_owned_cleanup(self):
        """The actual command-line owner completes all four plans over one borrowed transport."""
        suites = ("orientation", "accessory", "role-current", "interrupt")
        code, peer, report, directory, opened, evidence = self.invoke(suites=suites)
        self.assertEqual(code, 0)
        opened.assert_called_once()
        self.assertTrue(peer.closed)
        self.assertEqual(report["primary"], "PASS")
        self.assertEqual(report["cleanup"], "PASS")
        self.assertFalse(report["residual_uncertainty"])
        for suite in suites:
            self.assertEqual(report["checks"][suite]["status"], "PASS")
        self.assertEqual(len(evidence["sessions"]), 1)
        self.assertIsNone(peer.operation)
        self.assertTrue(all(session.operation is None and not session.valid
                            for session in evidence["sessions"]))
        self.assertEqual(sum(text.startswith("result take ") for text in peer.writes),
                         len(peer.operation_history))
        self.assertEqual(report["metadata"]["host"]["git"], "f" * 40)
        self.assertEqual(report["metadata"]["firmware"]["git"], GIT)
        self.assertTrue((directory / "transcript.log").is_file())
        self.assertTrue((directory / "summary.md").is_file())

    def test_mixed_repeated_suites_share_port_and_deadline_with_fresh_sessions(self):
        """Normalization preserves requested group order and closes each prior Session."""
        code, peer, report, _, opened, evidence = self.invoke(
            suites=("readonly", "orientation", "orientation", "soak"),
            extra=("--count", "2", "--interval-ms", "1"))
        self.assertEqual(code, 0)
        opened.assert_called_once()
        self.assertTrue(peer.closed)
        self.assertEqual([group[0] for group in evidence["groups"]],
                         [("readonly",), ("orientation",), ("soak",)])
        self.assertEqual(len({id(session) for session in evidence["sessions"]}), 3)
        deadlines = [group[2] for group in evidence["groups"]]
        self.assertTrue(all(abs(deadline - deadlines[0]) < 1e-9 for deadline in deadlines))
        self.assertTrue(all(not session.valid and session.operation is None
                            for session in evidence["sessions"]))
        self.assertEqual(report["metadata"]["selected_plan"]["suites"],
                         ["readonly", "orientation", "soak"])
        for suite in ("readonly", "orientation", "soak"):
            self.assertEqual(report["checks"][suite]["status"], "PASS")

    def test_expired_whole_deadline_cannot_start_another_group(self):
        """Time consumed between groups cannot be replaced by a fresh fixture timeout."""
        code, peer, report, _, opened, evidence = self.invoke(
            suites=("readonly", "orientation"), fault="between-groups")
        self.assertEqual(code, 1)
        opened.assert_called_once()
        self.assertTrue(peer.closed)
        self.assertEqual(len(evidence["groups"]), 1)
        self.assertFalse(peer.mutation_history)
        self.assertEqual(report["checks"]["orientation"]["status"], "NOT RUN")

    def test_requested_soak_duration_is_clipped_after_a_completed_fixture_group(self):
        """A duration ceiling equal to the run limit remains valid after earlier work."""
        code, peer, report, _, opened, evidence = self.invoke(
            suites=("orientation", "soak"),
            extra=("--count", "1", "--duration-ms", "60000"))
        self.assertEqual(code, 0)
        opened.assert_called_once()
        self.assertTrue(peer.closed)
        self.assertEqual(report["primary"], "PASS")
        self.assertEqual(report["metadata"]["selected_plan"]["duration_ms"], 60000)
        self.assertEqual([group[0] for group in evidence["groups"]],
                         [("orientation",), ("soak",)])
        first, second = evidence["groups"]
        self.assertGreater(second[1], first[1])
        self.assertEqual(first[2], second[2])
        self.assertLess(evidence["clock"](), first[2])
        for suite in ("orientation", "soak"):
            self.assertEqual(report["checks"][suite]["status"], "PASS")

    def test_separated_fixture_groups_retain_prior_physical_proof_and_cleanup(self):
        """A later fixture session cannot erase earlier passing orientation/cleanup evidence."""
        code, _, report, _, _, evidence = self.invoke(
            suites=("orientation", "readonly", "interrupt"))
        self.assertEqual(code, 0)
        self.assertEqual(len(evidence["sessions"]), 3)
        observations = report["metadata"]["fixture_observations"]
        for group, key in (("orientation", "orientation-cc2"), ("interrupt", "irq-coalesced")):
            self.assertIn(key, observations[group])
            self.assertTrue(observations[group]["cleanup-detached"]["attestation"])
            self.assertIn(group, report["metadata"]["fixture_baselines"])
            self.assertEqual(report["metadata"]["fixture_cleanups"][group], "PASS")

    def test_unknown_firmware_identity_opens_for_reading_but_cannot_mutate(self):
        """Host checkout provenance cannot satisfy an unknown live firmware identity."""
        code, peer, report, _, opened, _ = self.invoke(fault="unknown-identity")
        self.assertEqual(code, 2)
        opened.assert_called_once()
        self.assertTrue(peer.closed)
        self.assertFalse(peer.mutation_history)
        self.assertNotEqual(report["primary"], "PASS")

    def test_mutation_failure_and_interrupt_save_truthful_exit_and_residuals(self):
        """Failed/interrupting plans preserve primary outcome and never blindly restore roles."""
        for fault, expected in (("mutation", 1), ("interrupt", 130)):
            with self.subTest(fault=fault):
                code, peer, report, directory, opened, _ = self.invoke(fault=fault)
                self.assertEqual(code, expected)
                self.assertEqual(report["exit_code"], expected)
                opened.assert_called_once()
                self.assertTrue(peer.closed)
                self.assertNotEqual(report["primary"], "PASS")
                self.assertTrue(report["residual_uncertainty"])
                self.assertEqual(report["cleanup"], "BLOCKED")
                self.assertEqual(len(peer.mutation_history), 1)
                self.assertEqual(report["interrupted"], fault == "interrupt")
                self.assertTrue((directory / "summary.md").is_file())


if __name__ == "__main__":
    unittest.main()
