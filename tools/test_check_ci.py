#!/usr/bin/env python3
"""Exercise CI policy failures without parsing YAML, invoking CI or building SDKs.

The existing validate_runner CTest owns this suite through test_validate.load_tests.
Tests use only Python 3.10+; actual workflow parsing is a separate source audit
using the explicitly installed pinned PyYAML maintainer dependency.
"""

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
import copy
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

import check_ci as ci


def fixture():
    """Model the complete declared workflow independently of any checked-in YAML."""
    checkout = "actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1"
    upload = "actions/upload-artifact@cf430e030ddbb5b0abf93d22962f4752f3646cd9"
    download = "actions/download-artifact@9000827ccba6bdab643e8b6fd33ac0654aef8333"
    model = {"on": {"push": None, "pull_request": None, "workflow_dispatch": None},
             "permissions": {"contents": "read"}, "jobs": {}}
    for name in ("check", "host", "python", "docs", "package", "arduino", "idf"):
        model["jobs"][name] = {"runs-on": "ubuntu-24.04", "timeout-minutes": 30,
            "steps": [{"uses": checkout}, {"uses": upload, "if": "always()", "with": {
                "path": "build/evidence/**", "retention-days": 7,
                "if-no-files-found": "error"}}]}
    host = model["jobs"]["host"]
    host["strategy"] = {"fail-fast": False, "matrix": {"include": [
        {"name": "gcc-debug", "compiler": "g++-14", "config": "Debug", "sanitize": False},
        {"name": "gcc-release", "compiler": "g++-14", "config": "Release", "sanitize": False},
        {"name": "clang-debug", "compiler": "clang++-18", "config": "Debug", "sanitize": False},
        {"name": "clang-release", "compiler": "clang++-18", "config": "Release", "sanitize": False},
        {"name": "clang-sanitized", "compiler": "clang++-18", "config": "Debug", "sanitize": True}]}}
    host["steps"].append({"run": "python tools/validate.py --checks native,contracts "
                                  "--configs '${{ matrix.config }}' "
                                  "--compiler '${{ matrix.compiler }}' "
                                  "--out build/host "
                                  "${{ matrix.sanitize && '--sanitize' || '' }}"})
    for name, group in (("python", "python"), ("docs", "doxygen"), ("package", "package")):
        model["jobs"][name]["steps"].append({
            "run": f"python tools/validate.py --checks {group} --configs Release "
                   f"--out build/{name}"})
    model["jobs"]["docs"]["steps"][-1]["run"] = (
        "doxygen --version > build/docs/tool.log\n" + model["jobs"]["docs"]["steps"][-1]["run"])
    model["jobs"]["idf"]["strategy"] = {"fail-fast": False, "matrix": {"include": [
        {"version": "5.5.5", "target": "esp32s2"},
        {"version": "5.5.5", "target": "esp32s3"},
        {"version": "6.1", "target": "esp32s2"},
        {"version": "6.1", "target": "esp32s3"}]}}
    for row in model["jobs"]["idf"]["strategy"]["matrix"]["include"]:
        row["image"] = ci.IDF_IMAGES[row["version"]]
    model["jobs"]["idf"]["container"] = {"image": "${{ matrix.image }}"}
    for name in ("arduino", "idf"):
        model["jobs"][name]["needs"] = "package"
        model["jobs"][name]["steps"].append({"uses": download, "with": {
            "name": "tusb32x-package", "path": "build/package"}})
    model["jobs"]["package"]["steps"].extend([
        {"run": "pio pkg pack . --output build/package/TUSB32x.tar.gz "
                "> build/package/pack.log 2>&1\n"
                "sha256sum build/package/TUSB32x.tar.gz > build/package/archive-sha.log\n"
                "python tools/check_package.py --archive build/package/TUSB32x.tar.gz "
                "--out build/package/distribution > build/package/consumer.log 2>&1"},
        {"uses": upload, "with": {"name": "tusb32x-package", "path":
            "build/package/TUSB32x.tar.gz\nbuild/package/archive-sha.log\n"
            "build/package/source-sha.log",
            "retention-days": 7, "if-no-files-found": "error"}},
    ])
    for name in ("arduino", "idf"):
        run = ("sha256sum --check build/package/archive-sha.log\n"
               "python tools/check_package.py --archive build/package/TUSB32x.tar.gz "
               f"--extract-only --out build/{name}/distribution > build/{name}/extract.log 2>&1\n"
               "package=$(python -c 'import json; print(json.load(open(\"build/" + name +
               "/distribution/package-report.json\"))[\"extracted\"])')\n")
        if name == "arduino":
            run += ('pio pkg install --project-dir "$package" > build/arduino/install.log 2>&1\n'
                    'python tools/check_firmware.py --package "$package" '
                    '--out "$GITHUB_WORKSPACE/build/arduino/firmware" --checks arduino '
                    '> build/arduino/link.log 2>&1')
        else:
            run = '. "$IDF_PATH/export.sh" > build/idf/environment.log 2>&1\n' + run
            run += ('check=idf-s2\nif [ "$TARGET" = esp32s3 ]; then check=idf-s3; fi\n'
                    'python tools/check_firmware.py --package "$package" '
                    '--out "$GITHUB_WORKSPACE/build/idf/firmware" --checks "$check" '
                    '--idf-path "$IDF_PATH" --idf-python "$(command -v python)" '
                    '> build/idf/link.log 2>&1')
        step = {"run": run}
        if name == "idf":
            step["env"] = {"TARGET": "${{ matrix.target }}"}
        model["jobs"][name]["steps"].append(step)
    return model


def source(model):
    """Supply action-use source lines with one audited release comment per instance."""
    return "\n".join("      - uses: " + step["uses"] + " # " +
        ci.ACTION_PINS.get(step["uses"].split("@")[0], ("", "v0"))[1]
        for job in model["jobs"].values() for step in job["steps"] if "uses" in step)


def inventory():
    """Build all expected registrations, including every adapter/application fault row."""
    names = {group: set(values) for group, values in ci.validate.REQUIRED_TESTS.items()}
    names["native"].update("tusb32x_arduino_app_" + value for value in (
        "startup", "startup-failure", "begin-failure", "preparation-failure", "cleanup-failure",
        "end-failure", "existing-bus", "shutdown", "usb_sda", "usb_irq"))
    names["native"].update("tusb32x_idf_app_" + value for value in (
        "startup", "startup-failure", "shutdown-active", "shutdown-startup", "backpressure",
        "bus-failure", "device-failure", "uart-config-failure", "uart-pin-failure",
        "uart-install-failure", "console-already-owned", "device-remove-failure",
        "bus-delete-failure", "uart-delete-failure", "rx-overflow", "raw-error",
        "rx-overflow-after-read", "rx-event-flood", "device-cleanup-failure",
        "teardown-fifo-pending", "teardown-fifo-stalled", "existing-bus", "irq-configured",
        "irq-config-failure", "irq-cleanup-failure", "diagnostic", "pin_conflict", "runtime"))
    return {"tests": [{"name": name, "properties": [
        {"name": "LABELS", "value": ["validation_" + group]},
        {"name": "TIMEOUT", "value": 30}]} for group, values in names.items()
        for name in sorted(values)]}


class CiChecks(unittest.TestCase):
    """Reject omitted rows, mutable actions and duplicate or incomplete test ownership."""

    def reject(self, mutate):
        """Apply one independent policy violation to an otherwise valid workflow."""
        model = fixture()
        mutate(model)
        with self.assertRaises(ci.AuditError):
            ci.audit_workflow(model, source(model))

    def test_complete_declared_matrix(self):
        """Seven jobs and every frozen host/SDK row pass structural source review."""
        model = fixture()
        self.assertEqual(ci.audit_workflow(model, source(model)),
                         {"jobs": 7, "host_rows": 5, "idf_rows": 4})

    def test_required_triggers_and_permissions(self):
        """Filtering trigger coverage or expanding token access fails admission."""
        for mutate in (lambda m: m.pop("on"), lambda m: m["on"].pop("push"),
                       lambda m: m["on"].update(push={"branches": ["main"]}),
                       lambda m: m["permissions"].update(contents="write"),
                       lambda m: m["permissions"].update(actions="write")):
            self.reject(mutate)

    def test_required_job_inventory(self):
        """Missing and unexpected jobs both require an explicit reviewed policy change."""
        self.reject(lambda m: m["jobs"].pop("docs"))
        self.reject(lambda m: m["jobs"].update(extra=copy.deepcopy(m["jobs"]["check"])))

    def test_job_timeouts_and_failure_admission(self):
        """Required jobs cannot run unbounded, be skipped or conceal failed execution."""
        for change in ({"timeout-minutes": 0}, {"timeout-minutes": 91},
                       {"timeout-minutes": "nan"}, {"timeout-minutes": True},
                       {"if": "false"}, {"continue-on-error": True},
                       {"runs-on": "ubuntu-latest"}, {"permissions": {"contents": "write"}},
                       {"steps": []}):
            with self.subTest(change=change):
                self.reject(lambda m: m["jobs"]["check"].update(change))

    def test_mutable_wrong_and_unknown_action_pins(self):
        """A formatted SHA alone cannot substitute for an audited official release."""
        for action in ("actions/checkout@v7.0.1", "actions/checkout@" + "a" * 40,
                       "unreviewed/action@" + "a" * 40, "./local-action"):
            self.reject(lambda m: m["jobs"]["check"]["steps"][0].update(uses=action))

    def test_each_action_has_its_own_release_comment(self):
        """Repeated uses cannot borrow one valid comment from another source line."""
        model = fixture()
        original = source(model)
        for text in (original.replace(" # v7.0.1", "", 1),
                     original.replace(" # v7.0.1", " # v7.0.0", 1),
                     original + "\n- uses: actions/checkout@" + "a" * 40 + " # v7.0.1"):
            with self.subTest(text=text[:100]), self.assertRaises(ci.AuditError):
                ci.audit_workflow(model, text)

    def test_step_failure_cannot_be_ignored(self):
        """A failed command remains a failed required job."""
        self.reject(lambda m: m["jobs"]["check"]["steps"][0].update(
            {"continue-on-error": True}))

    def test_required_run_and_action_steps_cannot_be_conditionally_skipped(self):
        """A present but disabled command or setup action cannot count as execution."""
        for condition in (False, "false", "${{ false }}", "success()"):
            self.reject(lambda m: m["jobs"]["host"]["steps"][-1].update({"if": condition}))
            self.reject(lambda m: m["jobs"]["check"]["steps"][0].update({"if": condition}))

    def test_artifacts_are_required_bounded_and_saved_after_failure(self):
        """Empty/missing evidence and expiring failure logs cannot silently pass."""
        for change in ({"path": ""}, {"retention-days": 0}, {"retention-days": 15},
                       {"if-no-files-found": "ignore"}):
            self.reject(lambda m: m["jobs"]["check"]["steps"][1]["with"].update(change))
        self.reject(lambda m: m["jobs"]["check"]["steps"][1].pop("if"))
        self.reject(lambda m: m["jobs"]["check"]["steps"].pop(1))

    def test_success_only_package_payload_can_accompany_failure_evidence(self):
        """Archive upload may require success while independent diagnostic upload always runs."""
        model = fixture()
        archive = model["jobs"]["package"]["steps"][-1]
        archive["if"] = "success()"
        ci.audit_workflow(model, source(model))
        archive["if"] = "false"
        with self.assertRaises(ci.AuditError):
            ci.audit_workflow(model, source(model))

    def test_exact_host_matrix_and_no_early_cancellation(self):
        """Both compiler configurations and the separate sanitized row remain mandatory."""
        for key, value in (("compiler", "g++-13"), ("config", "Release"),
                           ("sanitize", True), ("name", "renamed")):
            self.reject(lambda m: m["jobs"]["host"]["strategy"]["matrix"]["include"][0].update(
                {key: value}))
        self.reject(lambda m: m["jobs"]["host"]["strategy"]["matrix"]["include"].pop())
        self.reject(lambda m: m["jobs"]["host"]["strategy"].update({"fail-fast": True}))

    def test_host_validator_uses_declared_compiler_and_sanitizer(self):
        """Declared matrix fields must drive the actual validator invocation."""
        for removed in ("--compiler '${{ matrix.compiler }}' ",
                        "${{ matrix.sanitize && '--sanitize' || '' }}"):
            self.reject(lambda m: m["jobs"]["host"]["steps"][-1].update(run=
                m["jobs"]["host"]["steps"][-1]["run"].replace(removed, "")))
        for old, new in (("${{ matrix.compiler }}", "g++-14"),
                         ("matrix.sanitize", "false"),
                         ("${{ matrix.sanitize && '--sanitize' || '' }}", "--sanitize")):
            self.reject(lambda m: m["jobs"]["host"]["steps"][-1].update(run=
                m["jobs"]["host"]["steps"][-1]["run"].replace(old, new)))

    def test_exact_idf_matrix_and_package_dependency(self):
        """Oldest/current SDK S2/S3 rows all consume the same built archive."""
        self.reject(lambda m: m["jobs"]["idf"]["strategy"]["matrix"]["include"].pop())
        self.reject(lambda m: m["jobs"]["idf"]["strategy"]["matrix"]["include"][0].update(
            version="5.5.4"))
        for name in ("idf", "arduino"):
            self.reject(lambda m: m["jobs"][name].pop("needs"))
            self.reject(lambda m: m["jobs"][name]["steps"].pop())

    def test_idf_image_pin_and_actual_container_binding_are_required(self):
        """SDK names alone cannot establish that jobs execute in those SDK images."""
        self.reject(lambda m: m["jobs"]["idf"]["strategy"]["matrix"]["include"][0].pop("image"))
        for image in ("espressif/idf:v5.5.5", ci.IDF_IMAGES["6.1"]):
            self.reject(lambda m: m["jobs"]["idf"]["strategy"]["matrix"]["include"][0].update(
                image=image))
        self.reject(lambda m: m["jobs"]["idf"].pop("container"))
        self.reject(lambda m: m["jobs"]["idf"]["container"].update(image=ci.IDF_IMAGES["5.5.5"]))

    def test_each_ctest_group_has_one_configuration_owner(self):
        """Nested broad validation or repeated groups cannot execute registered suites twice."""
        self.reject(lambda m: m["jobs"]["python"]["steps"][-1].update(run=
            "python tools/validate.py --checks all --configs Release"))
        self.reject(lambda m: m["jobs"]["host"]["steps"].append(
            copy.deepcopy(m["jobs"]["host"]["steps"][-1])))
        self.reject(lambda m: m["jobs"]["docs"]["steps"][-1].update(run=
            "python tools/validate.py --checks doxygen --configs Debug"))
        self.reject(lambda m: m["jobs"]["python"].update(strategy={"matrix": {"x": [1, 2]}}))

    def test_validation_must_execute_instead_of_echoing_its_arguments(self):
        """A shell no-op cannot satisfy the validator command ownership check."""
        for prefix in ("echo ", ": ", "# "):
            self.reject(lambda m: m["jobs"]["host"]["steps"][-1].update(run=
                prefix + m["jobs"]["host"]["steps"][-1]["run"]))

    def test_validator_cannot_suppress_failures_or_override_arguments(self):
        """Trailing controls and argparse's last-value wins cannot weaken scope."""
        for suffix in (" || true", "; true", " &", " --checks python",
                       " --configs Debug", " --out build/other"):
            with self.subTest(suffix=suffix):
                self.reject(lambda m: m["jobs"]["host"]["steps"][-1].update(run=
                    m["jobs"]["host"]["steps"][-1]["run"] + suffix))

    def test_required_commands_cannot_be_heredoc_data_or_disabled_blocks(self):
        """A literal command in an inert shell block is not executed validation."""
        for name in ("host", "package", "arduino", "idf"):
            index = -2 if name == "package" else -1
            for opening, closing in (("cat <<'NOT_EXECUTED'", "NOT_EXECUTED"),
                                     ("if false; then", "fi")):
                with self.subTest(name=name, opening=opening):
                    self.reject(lambda m: m["jobs"][name]["steps"][index].update(run=
                        opening + "\n" + m["jobs"][name]["steps"][index]["run"]
                        + "\n" + closing))

    def test_distribution_commands_must_share_one_ordered_step(self):
        """Extraction, package selection and linking execute in the same shell."""
        for name in ("package", "arduino", "idf"):
            index = -2 if name == "package" else -1

            def split_step(model):
                """Move the last command into a fresh shell that loses local variables."""
                step = model["jobs"][name]["steps"][index]
                lines = step["run"].splitlines()
                step["run"] = "\n".join(lines[:-1])
                model["jobs"][name]["steps"].append({"run": lines[-1]})

            with self.subTest(name=name, mutation="split"):
                self.reject(split_step)
            with self.subTest(name=name, mutation="reverse"):
                self.reject(lambda m: m["jobs"][name]["steps"][index].update(run=
                    "\n".join(reversed(m["jobs"][name]["steps"][index]["run"].splitlines()))))

    def test_firmware_commands_cannot_be_omitted_or_replaced_by_noops(self):
        """Both firmware owners must execute the actual extracted-package link helper."""
        for name in ("arduino", "idf"):
            for replacement in ("", "echo ", ": ", "# "):
                def mutate(model):
                    """Remove or disable only the real link command in a complete job."""
                    step = model["jobs"][name]["steps"][-1]
                    lines = []
                    for line in step["run"].splitlines():
                        if "python tools/check_firmware.py " in line:
                            if not replacement:
                                continue
                            line = replacement + line
                        lines.append(line)
                    step["run"] = "\n".join(lines)
                with self.subTest(name=name, replacement=replacement):
                    self.reject(mutate)

    def test_distribution_provenance_and_target_binding_are_required(self):
        """Archive checks, extracted paths and target selection cannot drift silently."""
        for name in ("arduino", "idf"):
            for before, after in (
                    ("sha256sum --check", "echo sha256sum --check"),
                    ("--extract-only", ""),
                    ('--package "$package"', '--package "$GITHUB_WORKSPACE"'),
                    ('["extracted"]', '["archive"]')):
                self.reject(lambda m: m["jobs"][name]["steps"][-1].update(run=
                    m["jobs"][name]["steps"][-1]["run"].replace(before, after)))
            for key, value in (("name", "other-package"), ("path", "build/other")):
                self.reject(lambda m: m["jobs"][name]["steps"][-2]["with"].update({key: value}))
        self.reject(lambda m: m["jobs"]["idf"]["steps"][-1]["env"].update(TARGET="esp32s2"))
        self.reject(lambda m: m["jobs"]["idf"]["steps"][-1].update(run=
            m["jobs"]["idf"]["steps"][-1]["run"].replace("then check=idf-s3", "then check=idf-s2")))
        self.reject(lambda m: m["jobs"]["package"]["steps"][-1]["with"].update(
            path="build/package/source-sha.log"))
        for before in ("pio pkg pack", "sha256sum", "python tools/check_package.py"):
            self.reject(lambda m: m["jobs"]["package"]["steps"][-2].update(run=
                m["jobs"]["package"]["steps"][-2]["run"].replace(before, "echo " + before)))

    def test_exact_nonzero_ctest_counts_reuse_existing_ownership(self):
        """Actual JSON registration counts close omissions while validate owns label semantics."""
        data = inventory()
        groups = ci.audit_registration(data)
        self.assertEqual(sum(groups.values()), 72 if sys.platform == "win32" else 71)
        for mutation in ("missing", "extra", "unowned", "disabled", "zero"):
            current = copy.deepcopy(data)
            if mutation == "missing":
                current["tests"].pop()
            elif mutation == "extra":
                extra = copy.deepcopy(current["tests"][0])
                extra["name"] = "undeclared"
                current["tests"].append(extra)
            elif mutation == "unowned":
                current["tests"][0]["properties"][0]["value"] = []
            elif mutation == "disabled":
                current["tests"][0]["properties"].append({"name": "DISABLED", "value": True})
            else:
                current["tests"] = []
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                ci.audit_registration(current)

    def test_missing_yaml_dependency_is_explicitly_incomplete(self):
        """Native negative tests need no YAML dependency; actual parser absence is exit two."""
        real_import = __import__

        def unavailable(name, *args, **kwargs):
            """Model the optional maintainer parser's absence at its import boundary."""
            if name == "yaml":
                raise ImportError("modeled absent parser")
            return real_import(name, *args, **kwargs)

        with patch("builtins.__import__", side_effect=unavailable), \
                self.assertRaisesRegex(RuntimeError, "PyYAML 6.0.3"):
            ci.parse_workflow("on: [push]")

    def test_cli_reports_real_input_failures_without_execution(self):
        """CLI preserves invocation failures and can audit a saved actual CTest JSON file."""
        with tempfile.TemporaryDirectory(prefix="tusb32x-ci-test-") as temporary:
            workflow = Path(temporary) / "ci.yml"
            saved_inventory = Path(temporary) / "ctest.json"
            model = fixture()
            workflow.write_text(source(model), encoding="utf-8")
            saved_inventory.write_text(json.dumps(inventory()), encoding="utf-8")
            with patch.object(ci, "parse_workflow", return_value=model), redirect_stdout(io.StringIO()):
                self.assertEqual(ci.main(["--workflow", str(workflow),
                                          "--ctest-json", str(saved_inventory)]), 0)
            with redirect_stderr(io.StringIO()):
                self.assertEqual(ci.main(["--workflow", str(workflow.with_name("absent"))]), 1)
            with patch.object(ci, "parse_workflow", side_effect=RuntimeError("missing parser")), \
                    redirect_stderr(io.StringIO()):
                self.assertEqual(ci.main(["--workflow", str(workflow)]), 2)


if __name__ == "__main__":
    unittest.main()
