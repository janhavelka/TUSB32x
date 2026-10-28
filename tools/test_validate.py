"""Exercise validator selection, real child failures, deadlines and saved partial results.

Only small local child processes are run. Firmware, HIL and nested validation
are replaced at their invocation boundary; these tests do not run release checks.
"""

from __future__ import annotations

import argparse
from contextlib import redirect_stdout, redirect_stderr
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import Mock, patch

import validate


def inventory():
    """Construct a finite disjoint CTest inventory with every mandatory registration."""
    return {"tests": [{"name": name, "properties": [
        {"name": "LABELS", "value": ["validation_" + group]},
        {"name": "TIMEOUT", "value": 30.0}]}
        for group, names in validate.REQUIRED_TESTS.items() for name in sorted(names)]}


class ValidatorTests(unittest.TestCase):
    """Check requested-scope verdicts without invoking a second full test framework."""

    def setUp(self):
        """Keep child logs, reports and mutations inside one temporary directory."""
        self.temporary = tempfile.TemporaryDirectory(prefix="tusb32x-validator-test-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def invoke(self, arguments, ctests=None, firmware=None):
        """Execute real argument/report handling with injected check boundaries."""
        with patch.object(validate, "run_ctests", side_effect=ctests), \
                patch.object(validate, "run_firmware", side_effect=firmware), \
                redirect_stdout(io.StringIO()):
            code = validate.main(["--out", str(self.root), *arguments])
        reports = list(self.root.glob("run-*/report.json"))
        return code, json.loads(reports[-1].read_text(encoding="utf-8"))

    def test_selection_is_explicit_and_deduplicated(self):
        """All expands once; repeated names cannot duplicate checks or admit HIL."""
        self.assertEqual(validate.selections("all"), validate.CHECKS)
        self.assertEqual(validate.selections("native,python,native"), ("native", "python"))
        for value in ("", "all,native", "native,", "hil", "flash", "ci", "native python"):
            with self.subTest(value=value), self.assertRaises(argparse.ArgumentTypeError):
                validate.selections(value)

    def test_invalid_cli_never_starts_checks(self):
        """Bad selections/timeouts/configurations fail with invocation exit two."""
        for arguments in (("--checks", "hil"), ("--timeout", "0"), ("--timeout", "nan"),
                          ("--timeout", "7201"), ("--configs", "Unknown")):
            with self.subTest(arguments=arguments), patch.object(validate, "run_ctests") as run, \
                    redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as raised:
                validate.main(list(arguments))
            self.assertEqual(raised.exception.code, 2)
            run.assert_not_called()

    def test_unusable_output_is_incomplete_without_traceback(self):
        """An existing regular output file is invocation failure two, never a test failure."""
        output = self.root / "ordinary-file"
        output.write_text("preserve", encoding="utf-8")
        with patch.object(validate, "run_ctests") as run, redirect_stderr(io.StringIO()):
            self.assertEqual(validate.main(["--checks", "native", "--out", str(output)]), 2)
        self.assertEqual(output.read_text(encoding="utf-8"), "preserve")
        run.assert_not_called()

    def test_registration_rejects_zero_missing_duplicate_unowned_and_unbounded(self):
        """Inventory assertions require real nonzero suites with one owner and timeout."""
        self.assertEqual(set(validate.registration(inventory())), set(validate.CTEST_CHECKS))
        for mutation in ("empty", "missing", "duplicate", "unowned", "two-owners", "timeout",
                         "disabled"):
            data = inventory()
            if mutation == "empty":
                data["tests"] = []
            elif mutation == "missing":
                data["tests"].pop()
            elif mutation == "duplicate":
                data["tests"].append(data["tests"][0])
            else:
                properties = data["tests"][0]["properties"]
                if mutation == "unowned":
                    properties[0]["value"] = []
                elif mutation == "two-owners":
                    properties[0]["value"].append("validation_python")
                elif mutation == "timeout":
                    properties[1]["value"] = 0
                else:
                    properties.append({"name": "DISABLED", "value": True})
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                validate.registration(data)

    def test_real_child_pass_failure_and_missing_executable(self):
        """Argument-list children preserve logs, exact exit and missing-tool evidence."""
        for code, expected in ((0, "PASS"), (1, "FAIL"), (2, "INCOMPLETE")):
            result = validate.run_command("child" + str(code), [sys.executable, "-c",
                f"print('kept output'); raise SystemExit({code})"], self.root, 10,
                incomplete_codes=(2,))
            self.assertEqual(result.status, expected)
            self.assertEqual(result.returncode, code)
            self.assertIn("kept output", Path(result.log).read_text())
        result = validate.run_command("missing", [str(self.root / "absent-tool")], self.root, 1)
        self.assertEqual(result.status, "INCOMPLETE")

    def test_timeout_stops_child_and_fails(self):
        """A timed-out command cannot become a late PASS or leave its child running."""
        child = Mock()
        child.wait.side_effect = subprocess.TimeoutExpired(["tool"], 1)
        with patch.object(validate.subprocess, "Popen", return_value=child), \
                patch.object(validate, "stop_process") as stop:
            result = validate.run_command("timeout", ["tool"], self.root, 1)
        stop.assert_called_once_with(child)
        self.assertEqual(result.status, "FAIL")
        self.assertIn("timeout", result.detail)

    def test_child_interrupt_stops_and_propagates(self):
        """The command boundary terminates its child before propagating Ctrl-C."""
        child = Mock()
        child.wait.side_effect = KeyboardInterrupt
        with patch.object(validate.subprocess, "Popen", return_value=child), \
                patch.object(validate, "stop_process") as stop, \
                self.assertRaises(KeyboardInterrupt):
            validate.run_command("interrupt", ["tool"], self.root, 1)
        stop.assert_called_once_with(child)

    def test_partial_completion_is_incomplete(self):
        """Successful preparation or one selected group cannot certify unfinished scope."""
        def partial(args, directory, record):
            """Complete native only, leaving selected Python explicitly pending."""
            record(validate.Result("debug-native", "PASS", "fixture"))
        code, report = self.invoke(["--checks", "native,python", "--configs", "Debug"], partial)
        self.assertEqual(code, 2)
        self.assertEqual(report["pending"], ["debug-python"])
        self.assertEqual(report["scope"], "focused checks")

    def test_requested_focused_pass_and_configs_deduplicate(self):
        """Focused PASS reports its scope; duplicate configurations execute once."""
        def finish(args, directory, record):
            """Complete exactly the one requested group/configuration."""
            self.assertEqual(args.configs, ("Debug",))
            record(validate.Result("debug-native", "PASS", "fixture"))
        code, report = self.invoke(["--checks", "native", "--configs", "Debug", "Debug"], finish)
        self.assertEqual(code, 0)
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["scope"], "focused checks")
        self.assertEqual(report["hardware"], "NOT RUN")

    def test_failure_is_not_erased_by_later_incomplete_or_pass(self):
        """Primary failed checks retain exit one and FAIL even with missing later tools."""
        def fail(args, directory, record):
            """Record distinct failing and unavailable children."""
            record(validate.Result("debug-native", "FAIL", "failed child"))
            record(validate.Result("debug-python", "INCOMPLETE", "missing tool"))
        code, report = self.invoke(["--checks", "native,python", "--configs", "Debug"], fail)
        self.assertEqual(code, 1)
        self.assertEqual(report["status"], "FAIL")

    def test_interrupt_saves_prior_outcome_and_exit_130(self):
        """An interrupted run leaves an atomic readable report and precise pending work."""
        def interrupt(args, directory, record):
            """Interrupt after useful failure evidence has already been checkpointed."""
            record(validate.Result("debug-native", "FAIL", "retained failure"))
            raise KeyboardInterrupt
        code, report = self.invoke(["--checks", "native,python", "--configs", "Debug"], interrupt)
        self.assertEqual(code, 130)
        self.assertTrue(report["interrupted"])
        self.assertEqual(report["results"][0]["detail"], "retained failure")
        self.assertFalse(list(self.root.rglob("*.tmp")))

    def test_missing_required_build_tool_records_each_requested_group(self):
        """No false green when CMake/compiler/build tools cannot be executed."""
        args = validate.parser().parse_args(["--checks", "native,python"])
        records = []
        with patch.object(validate.shutil, "which", return_value=None):
            validate.run_ctests(args, self.root, records.append)
        self.assertEqual([result.name for result in records], ["native", "python"])
        self.assertTrue(all(result.status == "INCOMPLETE" for result in records))

    def test_checkpoint_failure_is_incomplete_without_recursive_save_error(self):
        """A later storage error cannot become green or recurse through record()."""
        actual_save = validate.save_report
        attempts = []

        def save(*args, **kwargs):
            """Allow initial setup, then model an unavailable evidence filesystem."""
            attempts.append(1)
            if len(attempts) > 1:
                raise OSError("fixture storage unavailable")
            return actual_save(*args, **kwargs)

        def finish(args, directory, record):
            """Model one successful child followed by failure saving its checkpoint."""
            record(validate.Result("debug-native", "PASS", "fixture"))

        with patch.object(validate, "save_report", side_effect=save), \
                patch.object(validate, "run_ctests", side_effect=finish), \
                redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            code = validate.main(["--checks", "native", "--configs", "Debug",
                                  "--out", str(self.root)])
        self.assertEqual(code, 2)
        report = json.loads(next(self.root.glob("run-*/report.json")).read_text())
        self.assertEqual(report["status"], "INCOMPLETE")

    def test_junit_requires_exact_nonzero_execution(self):
        """A green child that omitted tests or skipped a required case is rejected."""
        path = self.root / "tests.xml"
        for xml, good in (('<testsuite><testcase name="one"/></testsuite>', True),
                          ('<testsuite/>', False),
                          ('<testsuite><testcase name="other"/></testsuite>', False),
                          ('<testsuite><testcase name="one"><skipped/>'
                           '</testcase></testsuite>', False),
                          ('<testsuite><testcase name="one" status="notrun"/></testsuite>', False),
                          ('<testsuite><testcase name="one"><error/>'
                           '</testcase></testsuite>', False),
                          ('<testsuite><testcase name="one"/>'
                           '<testcase name="one"/></testsuite>', False)):
            path.write_text(xml)
            if good:
                validate.verify_execution(path, ["one"])
            else:
                with self.assertRaises(ValueError):
                    validate.verify_execution(path, ["one"])

    def test_combined_selections_execute_each_suite_once(self):
        """Actual selection code constructs one disjoint CTest call per group/config."""
        args = validate.parser().parse_args(
            ["--checks", "native,python,native", "--configs", "Debug"])
        calls, records = [], []
        groups = validate.registration(inventory())

        def child(name, command, directory, timeout, **kwargs):
            """Supply standard CTest inventory/results at the child boundary."""
            calls.append((name, command))
            log = self.root / (name + ".log")
            log.write_text(json.dumps(inventory()) if name.endswith("inventory") else "")
            if "--output-junit" in command:
                group = name.removeprefix("debug-")
                path = Path(command[command.index("--output-junit") + 1])
                path.write_text("<testsuite>" + "".join(
                    f'<testcase name="{item}"/>' for item in groups[group]) + "</testsuite>")
            return validate.Result(name, "PASS", "fixture", log=str(log))

        with patch.object(validate, "run_command", side_effect=child), \
                patch.object(validate.shutil, "which", return_value="tool"):
            validate.run_ctests(args, self.root, lambda result: records.append(result) or result)
        names = [name for name, command in calls]
        self.assertEqual(names, ["debug-configure", "debug-build", "debug-inventory",
                                 "debug-native", "debug-python"])
        self.assertIn(f"-DPython3_EXECUTABLE={sys.executable}", calls[0][1])
        self.assertTrue(Path(args.make_program).is_absolute())
        self.assertIn(f"-DCMAKE_MAKE_PROGRAM={args.make_program}", calls[0][1])
        for name, command in calls[-2:]:
            selected = command[-1]
            if name.endswith("native"):
                self.assertNotIn("hil_runner", selected)
            self.assertNotIn("cli_contract", selected)


if __name__ == "__main__":
    unittest.main()
