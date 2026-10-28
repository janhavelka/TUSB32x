"""Check finite observation planning and real CLI admission without opening hardware.

These tests call the production plan and main entrypoint. Invalid invocations and
dry runs must never call open_serial; no optional serial package, waits or board
are needed. Session/protocol behavior is covered by the shared runner suite.
"""

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
from dataclasses import FrozenInstanceError
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import tusb32x_hil_runner as runner


def make_plan(**values):
    """Construct the public plan lazily, keeping test discovery independent of execution."""
    from tusb32x_hil_plans import ObservationPlan
    return ObservationPlan(**values)


class ObservationPlanTests(unittest.TestCase):
    """Validate bounds and selected silicon before any session can be opened."""

    def test_default_plan_is_frozen_and_readonly(self):
        """The default selects the primary part and one finite readonly suite."""
        plan = make_plan()
        self.assertEqual(plan.variant, "tusb320lai")
        self.assertEqual(plan.address, 0x47)
        self.assertEqual(plan.suites, ("readonly",))
        self.assertEqual(plan.count, 10)
        self.assertEqual(plan.interval_ms, 1000)
        self.assertIsNone(plan.duration_ms)
        self.assertEqual(plan.run_timeout, 60.0)
        self.assertEqual(plan.command_timeout, 5.0)
        with self.assertRaises(FrozenInstanceError):
            plan.count = 20

    def test_each_verified_variant_accepts_only_its_strap_addresses(self):
        """Base and later devices retain their different seven-bit strap addresses."""
        for variant in ("tusb320", "tusb320i", "tusb320li", "tusb320hi",
                        "tusb320lai", "tusb320hai", "tusb322i"):
            allowed = (0x60, 0x61) if variant in ("tusb320", "tusb320i") else (0x47, 0x67)
            for address in allowed:
                with self.subTest(variant=variant, address=address):
                    self.assertEqual(make_plan(variant=variant, address=address).address,
                                     address)
            for address in (0, 0x46, 0x60 if allowed[0] == 0x47 else 0x47, 0x80, 0xFF):
                with self.subTest(variant=variant, address=address):
                    with self.assertRaises(ValueError):
                        make_plan(variant=variant, address=address)

    def test_variant_and_address_require_exact_types_and_known_values(self):
        """Unknown silicon, aliases, booleans and string addresses cannot become bindings."""
        for values in ({"variant": "unknown"}, {"variant": "TUSB320LAI"},
                       {"variant": "tusb321"}, {"variant": None}, {"variant": True},
                       {"variant": []}, {"variant": {}},
                       {"address": True}, {"address": "0x47"}, {"address": 71.0}):
            with self.subTest(values=values), self.assertRaises(ValueError):
                make_plan(**values)

    def test_suite_selection_is_explicit_and_repeated_selection_is_deduplicated(self):
        """Repeated options preserve their first selected order without repeated work."""
        self.assertEqual(make_plan(suites=("soak",)).suites, ("soak",))
        self.assertEqual(make_plan(suites=("readonly", "soak", "readonly")).suites,
                         ("readonly", "soak"))
        self.assertEqual(make_plan(suites=("soak", "readonly", "soak")).suites,
                         ("soak", "readonly"))

    def test_empty_unknown_and_untyped_suites_are_rejected(self):
        """An empty or malformed selection cannot produce an empty successful run."""
        for suites in ((), ("",), ("all",), ("READONLY",), (None,), ([],), ({},),
                       "readonly", None, True):
            with self.subTest(suites=suites), self.assertRaises(ValueError):
                make_plan(suites=suites)

    def test_count_interval_and_duration_enforce_integer_bounds(self):
        """Every bounded integer rejects boolean coercion, fractions and huge arithmetic."""
        for name, maximum in (("count", 100000), ("interval_ms", 3600000),
                              ("duration_ms", 86400000)):
            for value in (False, True, 0, -1, 1.5, "1", maximum + 1, 10 ** 100):
                with self.subTest(name=name, value=value), self.assertRaises(ValueError):
                    make_plan(**{name: value})
        self.assertEqual(make_plan(count=1, interval_ms=1, duration_ms=1).count, 1)
        self.assertEqual(make_plan(count=1, interval_ms=3600000).interval_ms, 3600000)

    def test_timeouts_reject_nonfinite_nonpositive_and_untyped_values(self):
        """A finite command and whole-run deadline are mandatory even for dry plans."""
        for name in ("command_timeout", "run_timeout"):
            for value in (False, True, 0, -1, "5", None, float("nan"),
                          float("inf"), -float("inf"), 10 ** 100):
                with self.subTest(name=name, value=value), self.assertRaises(ValueError):
                    make_plan(**{name: value})
        for values in ({"run_timeout": 86400.001}, {"command_timeout": 3600.001}):
            with self.subTest(values=values), self.assertRaises(ValueError):
                make_plan(**values)

    def test_count_interval_product_must_fit_duration_and_run_deadline(self):
        """Explicit finite ceilings reject an impossible requested observation span."""
        for values in (
            {"count": 3, "interval_ms": 1000, "duration_ms": 1999},
            {"count": 3, "interval_ms": 1000, "duration_ms": 2000},
            {"count": 3, "interval_ms": 1000, "run_timeout": 2,
             "command_timeout": 1},
            {"count": 100000, "interval_ms": 3600000, "run_timeout": 86400},
            {"count": 100, "interval_ms": 1000, "run_timeout": 60},
        ):
            with self.subTest(values=values), self.assertRaises(ValueError):
                make_plan(suites=("soak",), **values)

    def test_whole_deadline_covers_command_and_requested_duration(self):
        """A command or requested observation window cannot outlive the whole plan."""
        for values in ({"run_timeout": 4, "command_timeout": 5},
                       {"duration_ms": 60001, "run_timeout": 60}):
            with self.subTest(values=values), self.assertRaises(ValueError):
                make_plan(**values)
        plan = make_plan(suites=("soak",), count=3, interval_ms=1000,
                         duration_ms=2001, run_timeout=3, command_timeout=1)
        self.assertEqual(plan.duration_ms, 2001)

    def test_unused_soak_span_does_not_reject_readonly_selection(self):
        """Readonly validates integer ranges without scheduling unselected soak work."""
        plan = make_plan(count=100000, interval_ms=3600000)
        self.assertEqual(plan.suites, ("readonly",))
        self.assertEqual(plan.count, 100000)

    def test_documented_maximum_bounds_remain_usable(self):
        """The allowed maxima are usable when their actual selected span fits."""
        plan = make_plan(suites=("soak",), count=100000, interval_ms=1,
                         duration_ms=86400000, run_timeout=86400, command_timeout=3600)
        self.assertEqual(plan.count, 100000)
        self.assertEqual(plan.duration_ms, 86400000)
        self.assertEqual(plan.run_timeout, 86400)

    def test_describe_returns_detached_finite_json(self):
        """The saved description contains ordinary serializable values and no shared state."""
        plan = make_plan(suites=("readonly", "soak"), count=2, interval_ms=1)
        description = plan.describe()
        encoded = json.dumps(description, allow_nan=False)
        self.assertIn("tusb320lai", encoded)
        self.assertIn("readonly", encoded)
        self.assertIn("soak", encoded)
        self.assertEqual(description["variant"], "tusb320lai")
        self.assertEqual(description["address"], 71)
        self.assertEqual(description["suites"], ["readonly", "soak"])
        self.assertEqual(description["hardware"], "NOT RUN")
        self.assertTrue(description["registers"])
        self.assertTrue(all(type(register) is int and 0 <= register <= 255
                            for register in description["registers"]))
        self.assertTrue(description["actions"])
        self.assertNotIn("wreg ", encoded)
        description.clear()
        self.assertTrue(plan.describe())

    def test_register_observation_never_scans_an_unmapped_variant_offset(self):
        """The documented base A0 and 322I45 holes stay outside concrete read plans."""
        for variant in ("tusb320", "tusb320i", "tusb320li", "tusb320hi",
                        "tusb320lai", "tusb320hai", "tusb322i"):
            address = 0x60 if variant in ("tusb320", "tusb320i") else 0x47
            allowed = set(range(0x0B))
            if variant != "tusb322i":
                allowed.add(0x45)
            if variant not in ("tusb320", "tusb320i"):
                allowed.add(0xA0)
            with self.subTest(variant=variant):
                description = make_plan(variant=variant, address=address).describe()
                registers = description["registers"]
                self.assertTrue(registers)
                self.assertEqual(len(registers), len(set(registers)))
                self.assertTrue(set(registers).issubset(allowed))


class ObservationInvocationTests(unittest.TestCase):
    """Exercise pre-open validation and concrete dry-run artifacts through main()."""

    def invoke(self, extra=(), *, dry=True, output_option="--out"):
        """Run main with serial opening forbidden and return its saved detached evidence."""
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        directory = Path(temporary.name) / "evidence"
        arguments = [output_option, str(directory), *extra]
        if dry:
            arguments.append("--dry-run")
        with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()), \
                mock.patch.object(runner, "host_identity", return_value={
                    "git": "f" * 40, "tree": "dirty"}), \
                mock.patch.object(runner, "open_serial", side_effect=AssertionError(
                    "planning must not open a serial port")) as opened:
            code = runner.main(arguments)
        opened.assert_not_called()
        report_path = directory / "report.json"
        report = (json.loads(report_path.read_text(encoding="utf-8"))
                  if report_path.exists() else None)
        return code, report, directory

    def test_portless_dry_run_saves_plan_without_hardware_success(self):
        """A valid plan is saved while every physical observation remains NOT RUN."""
        code, report, directory = self.invoke()
        self.assertEqual(code, 2)
        self.assertEqual(report["exit_code"], 2)
        self.assertEqual(report["primary"], "NOT RUN")
        self.assertEqual(report["checks"]["plan"]["status"], "PASS")
        self.assertEqual(report["checks"]["hardware"]["status"], "NOT RUN")
        self.assertEqual(report["metadata"]["host"]["git"], "f" * 40)
        self.assertIn("plan", report["metadata"])
        self.assertNotIn("firmware", report["metadata"])
        self.assertTrue((directory / "summary.md").is_file())
        self.assertTrue((directory / "transcript.log").is_file())
        self.assertEqual(report["capture"]["tx_bytes"], 0)
        self.assertEqual(report["capture"]["rx_bytes"], 0)

    def test_repeated_suite_and_numeric_address_are_saved_in_concrete_plan(self):
        """The real parser accepts repeated selection and decimal or hexadecimal addresses."""
        for address in ("71", "0x47"):
            with self.subTest(address=address):
                code, report, _ = self.invoke((
                    "--variant", "tusb320lai", "--address", address,
                    "--suite", "soak", "--suite", "readonly", "--suite", "soak",
                    "--count", "2", "--interval-ms", "1", "--duration-ms", "10",
                    "--run-timeout", "60", "--command-timeout", "2"))
                self.assertEqual(code, 2)
                plan = report["metadata"]["plan"]
                self.assertEqual(plan["variant"], "tusb320lai")
                self.assertEqual(plan["address"], 71)
                self.assertEqual(plan["suites"], ["soak", "readonly"])

    def test_legacy_output_timeout_and_git_spellings_still_accept_valid_dry_run(self):
        """Previously published options share the same validated production path."""
        code, report, _ = self.invoke(("--timeout", "2", "--expect-git", "a" * 40),
                                      output_option="--output")
        self.assertEqual(code, 2)
        self.assertEqual(report["checks"]["plan"]["status"], "PASS")

    def test_expected_identity_remains_unproved_in_a_valid_dry_run(self):
        """Operator expectations can plan a run but cannot manufacture firmware records."""
        code, report, _ = self.invoke((
            "--expect-commit", "a" * 40, "--expect-framework", "esp-idf",
            "--expect-target", "esp32s3", "--expect-library", "0.1.0",
            "--expect-tree", "clean", "--expect-build", "qualified_build",
            "--expect-framework-version", "5.5.5"))
        self.assertEqual(code, 2)
        self.assertEqual(report["primary"], "NOT RUN")
        self.assertEqual(report["checks"]["plan"]["status"], "PASS")
        self.assertNotIn("firmware", report["metadata"])

    def test_dry_run_close_interruption_returns_the_saved_signal_verdict(self):
        """A late Ctrl-C cannot leave main's returned status inconsistent with report.json."""
        close = runner.Artifacts.close

        def interrupted_close(artifacts):
            """Close the real capture before delivering the injected final signal."""
            close(artifacts)
            raise KeyboardInterrupt()

        with mock.patch.object(runner.Artifacts, "close", new=interrupted_close):
            code, report, _ = self.invoke()
        self.assertEqual(code, 130)
        self.assertEqual(report["exit_code"], 130)
        self.assertTrue(report["interrupted"])

    def test_unwritable_output_prevents_hardware_open(self):
        """A path occupied by a regular file cannot admit serial work without evidence."""
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "occupied"
            path.write_text("existing evidence", encoding="utf-8")
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()), \
                    mock.patch.object(runner, "open_serial") as opened:
                code = runner.main(["--port", "COM_TEST", "--out", str(path)])
            self.assertEqual(code, 2)
            opened.assert_not_called()
            self.assertEqual(path.read_text(encoding="utf-8"), "existing evidence")

    def test_real_run_requires_explicit_physical_port_before_open(self):
        """A missing, empty or URL port cannot reach the serial opener."""
        for extra in ((), ("--port", ""), ("--port", "   "),
                      ("--port", "loop://"), ("--port", "socket://host:42")):
            with self.subTest(extra=extra):
                code, report, _ = self.invoke(extra, dry=False)
                self.assertEqual(code, 2)
                self.assertIsNone(report)

    def test_malformed_unsigned_arguments_are_rejected_before_open(self):
        """Unsigned fields reject signs, whitespace, junk suffixes and alternate bases."""
        for name in ("address", "count", "interval-ms", "duration-ms", "baud"):
            for value in ("-1", "+1", " 1", "1 ", "1junk", "1.0", "1e2", "0b1000111"):
                with self.subTest(name=name, value=value):
                    code, report, _ = self.invoke(("--port", "COM_TEST", "--" + name, value))
                    self.assertEqual(code, 2)
                    self.assertIsNone(report)

    def test_out_of_range_arguments_are_rejected_before_open(self):
        """Numeric boundaries and exact target pairing are checked before opening."""
        for extra in (
            ("--address", "0"), ("--address", "0x60"), ("--address", "128"),
            ("--count", "0"), ("--count", "100001"),
            ("--interval-ms", "0"), ("--interval-ms", "3600001"),
            ("--duration-ms", "0"), ("--duration-ms", "86400001"),
            ("--run-timeout", "0"), ("--run-timeout", "86400.001"),
            ("--command-timeout", "0"), ("--command-timeout", "3600.001"),
            ("--write-timeout", "0"), ("--baud", "0"), ("--baud", "4000001"),
            ("--variant", "tusb321"), ("--suite", "all"), ("--dtr", "toggle"),
            ("--rts", "toggle"), ("--response-bytes", "0"),
            ("--response-bytes", "4194305"), ("--transcript-bytes", "134217729"),
            ("--tail-bytes", "1048577"), ("--transcript-bytes", "10", "--tail-bytes", "11"),
            ("--suite", "soak", "--count", "100000", "--interval-ms", "3600000"),
        ):
            with self.subTest(extra=extra):
                code, report, _ = self.invoke(("--port", "COM_TEST", *extra))
                self.assertEqual(code, 2)
                self.assertIsNone(report)

    def test_nonfinite_deadlines_are_rejected_before_open(self):
        """NaN and infinity cannot disable a command, write or whole-run ceiling."""
        for name in ("run-timeout", "command-timeout", "write-timeout"):
            for value in ("nan", "NaN", "inf", "-inf", "1e309"):
                with self.subTest(name=name, value=value):
                    code, report, _ = self.invoke(("--" + name + "=" + value,))
                    self.assertEqual(code, 2)
                    self.assertIsNone(report)

    def test_unknown_and_malformed_expected_identity_is_rejected_before_open(self):
        """Host-supplied unknown labels and malformed commits cannot satisfy proof gates."""
        for name in ("commit", "git", "framework", "target", "library", "tree", "build",
                     "framework-version"):
            for value in ("", "unknown", "none", "two words", "line\nbreak"):
                with self.subTest(name=name, value=value):
                    code, report, _ = self.invoke(("--expect-" + name, value))
                    self.assertEqual(code, 2)
                    self.assertIsNone(report)
        for value in ("a" * 39, "a" * 41, "g" * 40, "123abc"):
            with self.subTest(commit=value):
                code, report, _ = self.invoke(("--expect-commit", value))
                self.assertEqual(code, 2)
                self.assertIsNone(report)
        for value in ("broken", "CLEAN", "changed"):
            with self.subTest(tree=value):
                code, report, _ = self.invoke(("--expect-tree", value))
                self.assertEqual(code, 2)
                self.assertIsNone(report)

    def test_expected_identity_cannot_exceed_its_firmware_record_field(self):
        """An expectation that no valid wire record can contain is rejected before opening."""
        for name, maximum in (("tree", 8), ("framework", 32), ("target", 32),
                              ("framework-version", 32), ("build", 64)):
            with self.subTest(name=name):
                code, report, _ = self.invoke(("--expect-" + name, "x" * (maximum + 1)))
                self.assertEqual(code, 2)
                self.assertIsNone(report)

    def test_help_requires_no_port_and_creates_no_session(self):
        """The public help lists the implemented plan controls and exits successfully."""
        output = io.StringIO()
        with redirect_stdout(output), redirect_stderr(io.StringIO()), \
                mock.patch.object(runner, "open_serial") as opened:
            self.assertEqual(runner.main(["--help"]), 0)
        opened.assert_not_called()
        for option in ("--suite", "--dry-run", "--run-timeout", "--expect-commit", "--out"):
            self.assertIn(option, output.getvalue())

    def test_session_construction_cannot_renew_the_whole_run_deadline(self):
        """Late Session setup fails before traffic under the original absolute run bound."""
        from test_tusb32x_hil_runner import FakeClock, PlanPeer

        clock = FakeClock()
        peer = PlanPeer(clock)
        peer.settings = {"port": "COM_TEST"}
        original_session = runner.Session

        def delayed_session(*args, **kwargs):
            """Consume the complete run budget before entering the real Session/plan."""
            clock.now += 2
            return original_session(*args, clock=clock, **kwargs)

        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary) / "evidence"
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()), \
                    mock.patch.object(runner, "host_identity", return_value={}), \
                    mock.patch.object(runner.time, "monotonic", side_effect=clock), \
                    mock.patch.object(runner, "Session", side_effect=delayed_session), \
                    mock.patch.object(runner, "open_serial", return_value=peer) as opened:
                code = runner.main([
                    "--port", "COM_TEST", "--out", str(directory),
                    "--run-timeout", "1", "--command-timeout", "1"])
            report = json.loads((directory / "report.json").read_text("utf-8"))
        self.assertEqual(code, 1)
        self.assertEqual(report["exit_code"], 1)
        self.assertEqual(report["primary"], "FAIL")
        self.assertEqual(report["checks"]["readonly"]["status"], "NOT RUN")
        self.assertFalse(peer.writes)
        self.assertEqual(peer.attempts, 0)
        self.assertTrue(peer.closed)
        opened.assert_called_once()


if __name__ == "__main__":
    unittest.main()
