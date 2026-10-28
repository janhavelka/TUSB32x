#!/usr/bin/env python3
"""Exercise firmware evidence rejection and bounded children without building SDKs.

Temporary files are deliberately modeled inspection inputs, never linked firmware.
The actual Python children only test logging, failure and timeout handling. Run
with Python3.10+; no PlatformIO, IDF, serial port or compiler is required.
"""

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
import io
import json
import os
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest import mock

import check_firmware as firmware


class FirmwareChecks(unittest.TestCase):
    """Test narrow check scope, missing prerequisites and evidence boundaries."""

    def setUp(self):
        """Create disjoint package and evidence directories without SDK content."""
        self.temporary = tempfile.TemporaryDirectory(prefix="tusb32x-firmware-tests-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()
        self.package = self.root / "renamed-package"
        self.output = self.root / "evidence"
        self.package.mkdir()
        self.output.mkdir()
        for name in ("library.json", "platformio.ini", "scripts/pio.cmd",
                     "examples/esp_idf/basic/CMakeLists.txt"):
            path = self.package / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("fixture only\n", encoding="utf-8")

    def invoke(self, *arguments):
        """Capture normal CLI output while keeping actual summary artifacts."""
        with redirect_stdout(io.StringIO()):
            return firmware.main(["--package", str(self.package), "--out", str(self.output),
                                  *arguments])

    def test_selection_is_explicit_and_deduplicated(self):
        """Narrow selection stays narrow and duplicate labels cannot repeat builds."""
        self.assertEqual(firmware.selections("idf-s3,idf-s3"), ("idf-s3",))
        self.assertEqual(firmware.selections("arduino,idf-s2"), ("arduino", "idf-s2"))
        for value in ("", "all", "native", "idf-s2,", "arduino,missing"):
            with self.subTest(value=value), self.assertRaises(firmware.argparse.ArgumentTypeError):
                firmware.selections(value)

    def test_invalid_cli_selection_returns_two_before_execution(self):
        """Unknown row names cannot silently select a different firmware matrix."""
        with mock.patch.object(firmware, "execute") as execute:
            with self.assertRaises(SystemExit) as caught, redirect_stderr(io.StringIO()):
                self.invoke("--checks", "missing")
            self.assertEqual(caught.exception.code, 2)
            execute.assert_not_called()

    def test_paths_require_absolute_disjoint_extraction(self):
        """Checkout, relative, nested output and package Git roots are rejected."""
        firmware.validate_paths(self.package, self.output)
        invalid = ((Path("relative"), self.output), (self.package, Path("relative")),
                   (self.package, self.package / "build"), (self.package, self.root),
                   (firmware.ROOT, self.output))
        for package, output in invalid:
            with self.subTest(package=package, output=output), \
                    self.assertRaises(firmware.Incomplete):
                firmware.validate_paths(package, output)
        (self.package / ".git").mkdir()
        with self.assertRaises(firmware.Incomplete):
            firmware.validate_paths(self.package, self.output)

    def test_missing_tool_retains_incomplete_summary(self):
        """An absent required executable is exit two with no passing firmware rows."""
        with mock.patch.object(firmware.shutil, "which", return_value=None):
            code = self.invoke("--checks", "idf-s2", "--core-dir", str(self.root / "absent"))
        self.assertEqual(code, 2)
        result = json.loads((self.output / "firmware-result.json").read_text(encoding="utf-8"))
        self.assertEqual(result["checks"], ["idf-s2"])
        self.assertEqual(result["result"], "INCOMPLETE")
        self.assertEqual(result["rows"], [])
        self.assertEqual(result["hardware"], "NOT RUN")
        self.assertIn("xtensa-esp-elf-nm", result["error"])

    def test_executable_paths_preserve_driver_and_virtual_environment_names(self):
        """Tool lookup must not resolve symlink names that select C++ or a Python venv."""
        python = self.root / "python-env/bin/python"
        compiler = self.root / "packages/compiler/bin/clang++"
        suffix = ".exe" if os.name == "nt" else ""
        installed = compiler.with_name(compiler.name + suffix)
        for path in (python, installed):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("modeled executable", encoding="utf-8")
        # Resolving any executable path is forbidden even on Windows, where
        # creating an actual symlink may require privileges unavailable to CI.
        with mock.patch.object(Path, "resolve", side_effect=AssertionError("resolved executable")):
            self.assertEqual(firmware.require_file(python), python)
            self.assertEqual(firmware.tool(self.root, "compiler", "clang++"), installed)
            with mock.patch.object(firmware.shutil, "which", return_value=str(python)):
                self.assertEqual(firmware.tool(self.root, "missing", "python"), python)

    def test_native_compiler_uses_unique_generated_metadata(self):
        """A real CMake compiler record works without a cache entry; uncertain records fail."""
        compiler = self.root / "compiler with spaces/xtensa-g++"
        compiler.parent.mkdir()
        compiler.write_text("modeled compiler", encoding="utf-8")
        metadata = self.output / "CMakeFiles/4.0.3/CMakeCXXCompiler.cmake"
        metadata.parent.mkdir(parents=True)
        with self.assertRaisesRegex(firmware.FirmwareError, "exactly one"):
            firmware.native_compiler(self.output)
        good = f'set(CMAKE_CXX_COMPILER "{compiler.as_posix()}")\n'
        metadata.write_text(good, encoding="utf-8")
        self.assertEqual(firmware.native_compiler(self.output), compiler)
        second = self.output / "CMakeFiles/3.30.0/CMakeCXXCompiler.cmake"
        second.parent.mkdir()
        second.write_text(good, encoding="utf-8")
        with self.assertRaisesRegex(firmware.FirmwareError, "exactly one"):
            firmware.native_compiler(self.output)
        second.unlink()
        for text in ("", good + good, 'set(CMAKE_CXX_COMPILER compiler)\n',
                     'set(CMAKE_CXX_COMPILER "relative/compiler")\n',
                     'set(CMAKE_CXX_COMPILER "${PREFIX}/compiler")\n',
                     good.replace('g++"', 'g++;second"'),
                     good + 'set(CMAKE_CXX_COMPILER other)\n'):
            with self.subTest(text=text), self.assertRaises(firmware.FirmwareError):
                metadata.write_text(text, encoding="utf-8")
                firmware.native_compiler(self.output)
        metadata.write_text(good, encoding="utf-8")
        compiler.unlink()
        with self.assertRaises(firmware.Incomplete):
            firmware.native_compiler(self.output)

    def test_zero_or_omitted_images_cannot_pass(self):
        """A successful wrapper with no inspected images is incomplete evidence, not PASS."""
        with mock.patch.object(firmware, "execute"):
            self.assertEqual(self.invoke("--checks", "arduino,idf-s2"), 1)
        result = json.loads((self.output / "firmware-result.json").read_text(encoding="utf-8"))
        self.assertIn("did not all complete", result["error"])

    def test_native_abi_response_matches_every_compiled_source(self):
        """The current SDK probe uses precisely the response file in actual commands."""
        response = self.output / "toolchain/cxxflags"
        response.parent.mkdir()
        response.write_text('"-specs=/sdk/picolibc.specs"\n-fno-rtti\n', encoding="utf-8")
        selected = [{"command": f'compiler @"{response.as_posix()}" -c source.cpp'},
                    {"command": f'compiler @"{response.as_posix()}" -c other.cpp'}]
        self.assertEqual(firmware.native_compile_flags(self.output, selected, "6.1.0"),
                         ["@" + str(response)])
        self.assertEqual(firmware.native_compile_flags(self.output, [], "5.5.5"), [])
        for bad in ([], [{"command": "compiler -c source.cpp"}],
                    [{"command": "compiler @toolchain/cxxflags -c source.cpp"}],
                    [{"command": 'compiler @"${BUILD}/toolchain/cxxflags"'}],
                    [{"command": f'compiler @"{self.root.as_posix()}/other"'}],
                    selected + [{"command": selected[0]["command"] + ' @"second"'}]):
            with self.subTest(bad=bad), self.assertRaises(firmware.FirmwareError):
                firmware.native_compile_flags(self.output, bad, "6.1.0")
        response.write_text("\n", encoding="utf-8")
        with self.assertRaisesRegex(firmware.FirmwareError, "nonempty"):
            firmware.native_compile_flags(self.output, selected, "6.1.0")
        response.unlink()
        with self.assertRaisesRegex(firmware.FirmwareError, "nonempty"):
            firmware.native_compile_flags(self.output, selected, "6.1.0")

    def test_include_probe_preserves_abi_response_and_build_directory(self):
        """Compiler-selected multilib roots are approved without their broader parents."""
        root = self.root / "picolibc/include/target/no-rtti"
        root.mkdir(parents=True)
        response = "@" + str(self.output / "toolchain/cxxflags")
        output = f"#include <...> search starts here:\n {root}\nEnd of search list.\n"
        with mock.patch.object(firmware, "run_logged", return_value=output) as child:
            roots = firmware.compiler_roots(Path("compiler"), {}, self.root, "idf-s3",
                                            [response], self.output)
        self.assertEqual(roots, [root])
        self.assertEqual(child.call_args.args[0],
                         [Path("compiler"), response, "-E", "-x", "c++", "-", "-v"])
        self.assertEqual(child.call_args.args[1], self.output)
        self.assertEqual(child.call_args.kwargs["input_text"], "")
        for bad in ("", "relative/include", "${SDK}/include", str(self.package / "library.json")):
            output = f"#include <...> search starts here:\n {bad}\nEnd of search list.\n"
            with self.subTest(bad=bad), \
                    mock.patch.object(firmware, "run_logged", return_value=output), \
                    self.assertRaises(firmware.FirmwareError):
                firmware.compiler_roots(Path("compiler"), {}, self.root, "idf-s3")

    def test_arduino_build_materializes_tools_before_inspection(self):
        """Metadata-only installed packages become executable during the build child."""
        manifest = self.root / "platforms/espressif32/platform.json"
        manifest.parent.mkdir(parents=True)
        manifest.write_text(json.dumps({"name": "espressif32", "version": "55.03.311"}),
                            encoding="utf-8")
        framework = self.root / "packages/framework-arduinoespressif32/package.json"
        framework.parent.mkdir(parents=True)
        framework.write_text('{"version":"3.3.11"}', encoding="utf-8")
        (self.root / "packages/framework-arduinoespressif32-libs").mkdir()
        pio = self.root / ".platformio/penv/Scripts/pio.exe"
        pio.parent.mkdir(parents=True)
        pio.touch()
        events = []
        suffix = ".exe" if os.name == "nt" else ""
        tool_names = ("xtensa-esp-elf-nm", "xtensa-esp32s2-elf-g++", "xtensa-esp32s3-elf-g++")
        binary = self.root / "packages/toolchain-xtensa-esp-elf/bin"

        def build(command, *_args, **kwargs):
            """Model package materialization, without creating or certifying firmware."""
            self.assertIn("run", command)
            self.assertEqual(kwargs["timeout"], 1800)
            events.append("build")
            binary.mkdir(parents=True)
            for name in tool_names:
                (binary / (name + suffix)).touch()

        def inspect(*_args):
            """Stop at inspection; no simulated image can yield a passing row."""
            self.assertEqual(events, ["build"])
            self.assertTrue(all((binary / (name + suffix)).is_file() for name in tool_names))
            events.append("inspect")
            raise firmware.FirmwareError("inspection boundary reached")

        args = SimpleNamespace(core_dir=self.root, package=self.package, checks=("arduino",))
        rows = []
        with mock.patch.object(firmware, "run_logged", side_effect=build), \
                mock.patch.object(firmware, "inspect_row", side_effect=inspect), \
                mock.patch.object(Path, "home", return_value=self.root), \
                mock.patch.object(firmware.shutil, "which",
                                  side_effect=lambda name: str(pio) if name == "pio" else None):
            with self.assertRaisesRegex(firmware.FirmwareError, "inspection boundary"):
                firmware.execute(args, self.output, rows)
        self.assertEqual(events, ["build", "inspect"])
        self.assertEqual(rows, [])

    def test_unusable_output_directory_is_invocation_incomplete(self):
        """An ordinary output file is preserved and returns two before any SDK work."""
        output = self.root / "ordinary-file"
        output.write_text("preserve", encoding="utf-8")
        with mock.patch.object(firmware, "execute") as execute, redirect_stdout(io.StringIO()):
            code = firmware.main(["--package", str(self.package), "--out", str(output)])
        self.assertEqual(code, 2)
        self.assertEqual(output.read_text(encoding="utf-8"), "preserve")
        execute.assert_not_called()

    def test_partial_failure_and_interruption_preserve_evidence(self):
        """Failure and interruption retain partial evidence with nonzero verdicts."""
        for error, expected in ((firmware.FirmwareError("missing image"), 1),
                                (KeyboardInterrupt(), 130)):
            with self.subTest(expected=expected):
                with mock.patch.object(firmware, "execute", side_effect=error):
                    self.assertEqual(self.invoke("--checks", "arduino"), expected)
                result = json.loads(
                    (self.output / "firmware-result.json").read_text(encoding="utf-8"))
                self.assertNotEqual(result["result"], "PASS")
                self.assertEqual(result["rows"], [])
                self.assertTrue(Path(result["work"]).is_dir())

    def test_build_cannot_modify_extracted_inputs(self):
        """Even an otherwise successful child cannot certify a mutated extraction."""
        def mutate(args, work, rows):
            """Model one forbidden source mutation rather than a firmware success."""
            (args.package / "library.json").write_text("changed", encoding="utf-8")
        with mock.patch.object(firmware, "execute", side_effect=mutate):
            self.assertEqual(self.invoke("--checks", "idf-s2"), 1)
        result = json.loads((self.output / "firmware-result.json").read_text(encoding="utf-8"))
        self.assertIn("modified the extracted", result["error"])

    def test_child_failure_and_timeout_keep_actual_output(self):
        """Real bounded Python children fail with their logs available for diagnosis."""
        log = self.output / "child.log"
        command = [sys.executable, "-c", "print('failed fixture', flush=True); raise SystemExit(7)"]
        with self.assertRaisesRegex(firmware.FirmwareError, "exited 7"):
            firmware.run_logged(command, self.root, dict(os.environ), log)
        self.assertIn("failed fixture", log.read_text(encoding="utf-8"))
        evidence = json.loads(log.with_suffix(".command.json").read_text(encoding="utf-8"))
        self.assertEqual(evidence["command"], command)
        self.assertEqual(evidence["timeout_seconds"], 60)
        self.assertEqual(evidence["cwd"], str(self.root))
        command = [sys.executable, "-c",
                   "import time; print('waiting', flush=True); time.sleep(60)"]
        with self.assertRaisesRegex(firmware.FirmwareError, "timed out"):
            firmware.run_logged(command, self.root, dict(os.environ), log, timeout=0.1)
        self.assertTrue(log.is_file())

    def test_missing_child_executable_is_incomplete(self):
        """Failure to spawn a required child differs from a failed compiler run."""
        with self.assertRaises(firmware.Incomplete):
            firmware.run_logged([self.root / "absent"], self.root, dict(os.environ),
                                self.output / "missing.log")

    def test_idf_constraints_override_requires_explicit_flag(self):
        """Inherited opt-outs are removed; only an explicit switch preserves the waiver."""
        sdk = self.root / "sdk"
        (sdk / "tools").mkdir(parents=True)
        (sdk / "tools/idf.py").write_text("fixture", encoding="utf-8")
        metadata = sdk / "tools/cmake/version.cmake"
        metadata.parent.mkdir()
        metadata.write_text("set(IDF_VERSION_MAJOR 5)\nset(IDF_VERSION_MINOR 5)\n"
                            "set(IDF_VERSION_PATCH 5)\n", encoding="utf-8")
        python = self.root / "python-env/Scripts/python.exe"
        python.parent.mkdir(parents=True)
        python.write_text("fixture", encoding="utf-8")
        for allowed in (False, True):
            args = SimpleNamespace(idf_path=sdk, idf_python=python, core_dir=self.root,
                                   accept_missing_constraints=allowed)
            environment = {"IDF_PYTHON_CHECK_CONSTRAINTS": "no"}
            firmware.idf_settings(args, environment)
            self.assertEqual(environment.get("IDF_PYTHON_CHECK_CONSTRAINTS"),
                             "no" if allowed else None)
        metadata.write_text("set(IDF_VERSION_MAJOR 6)\nset(IDF_VERSION_MINOR 0)\n"
                            "set(IDF_VERSION_PATCH 1)\n", encoding="utf-8")
        with self.assertRaises(firmware.Incomplete):
            firmware.idf_settings(args, {})

    def test_official_sdk_version_metadata_and_current_release(self):
        """Official SDKs need no optional archive version.txt; malformed fields fail."""
        metadata = self.root / "sdk/tools/cmake/version.cmake"
        metadata.parent.mkdir(parents=True)
        good = "set(IDF_VERSION_MAJOR 6)\nset(IDF_VERSION_MINOR 1)\nset(IDF_VERSION_PATCH 0)\n"
        metadata.write_text(good, encoding="utf-8")
        self.assertEqual(firmware.idf_version(self.root / "sdk"), "6.1.0")
        self.assertIn("6.1.0", firmware.IDF_VERSIONS)
        for bad in (good.replace("set(IDF_VERSION_PATCH 0)", ""), good + good,
                    good.replace("PATCH 0", "PATCH unknown")):
            metadata.write_text(bad, encoding="utf-8")
            with self.assertRaises(firmware.Incomplete):
                firmware.idf_version(self.root / "sdk")

    def test_ninja_requires_valid_nonempty_used_dependency_record(self):
        """A stale or absent Ninja database cannot prove compiler header provenance."""
        good = "object: #deps 2, deps mtime 12 (VALID)\n    source.cpp\n    header.h\n\n"
        self.assertEqual(firmware.ninja_dependencies(good), ["source.cpp", "header.h"])
        for text in ("", "object: #deps 0, deps mtime 12 (VALID)\n",
                     good.replace("VALID", "STALE"), "source.cpp\n"):
            with self.subTest(text=text), self.assertRaises(firmware.FirmwareError):
                firmware.ninja_dependencies(text)

    def test_offline_idf_prerequisite_failure_is_incomplete(self):
        """Missing SDK Python modules/constraints cannot become a successful build scope."""
        sdk = self.root / "sdk"
        (sdk / "tools").mkdir(parents=True)
        (sdk / "tools/idf_tools.py").write_text("fixture", encoding="utf-8")
        with mock.patch.object(firmware, "run_logged", side_effect=firmware.FirmwareError("cache")):
            with self.assertRaisesRegex(firmware.Incomplete, "prerequisites are incomplete"):
                firmware.idf_prerequisites(sdk, Path(sys.executable), {}, self.output)

    def test_missing_or_wrong_installed_platform_is_incomplete(self):
        """Arduino rows require the actual audited platform before PlatformIO can run."""
        manifest = self.root / "platforms/espressif32/platform.json"
        manifest.parent.mkdir(parents=True)
        for version in (None, "54.03.20", "55.03.311"):
            manifest.write_text(json.dumps({"name": "espressif32", "version": version}),
                                encoding="utf-8")
            if version == "55.03.311":
                self.assertEqual(firmware.arduino_platform(self.root), manifest.parent)
            else:
                with self.assertRaises(firmware.Incomplete):
                    firmware.arduino_platform(self.root)

    def test_compiler_dependency_escape_and_omitted_sources_fail(self):
        """Actual used paths must remain in approved source/build/SDK roots."""
        source = self.package / "main.cpp"
        source.write_text("fixture", encoding="utf-8")
        header = self.package / "include/public.h"
        header.parent.mkdir()
        header.write_text("fixture", encoding="utf-8")
        outside = self.root / "outside.h"
        outside.write_text("fixture", encoding="utf-8")
        approved = [self.package, self.output]
        args = (self.package, self.output, approved, {"main.cpp"}, self.package)
        evidence = firmware.verify_dependencies([[str(source), str(header)]], *args)
        self.assertEqual(evidence["compiled_sources"], ["main.cpp"])
        for records in ([], [[]], [[str(source), str(outside)]], [[str(header)]],
                        [[str(source), str(header)], [str(source)]]):
            with self.subTest(records=records), self.assertRaises(firmware.FirmwareError):
                firmware.verify_dependencies(records, *args)

    def test_identity_cannot_borrow_parent_git_or_wrong_target(self):
        """Archive firmware must report unknown Git and its actual target."""
        path = self.output / "FirmwareBuild.h"
        good = ('#define TUSB32X_BUILD_GIT "unknown"\n'
                '#define TUSB32X_BUILD_TREE "unknown"\n'
                '#define TUSB32X_BUILD_TARGET "esp32s2"\n')
        path.write_text(good, encoding="utf-8")
        firmware.verify_identity(path, "esp32s2")
        for text in (good.replace('GIT "unknown"', 'GIT "1234567"'),
                     good.replace('TREE "unknown"', 'TREE "clean"'),
                     good.replace("esp32s2", "esp32s3")):
            with self.subTest(text=text), self.assertRaises(firmware.FirmwareError):
                path.write_text(text, encoding="utf-8")
                firmware.verify_identity(path, "esp32s2")

    def test_artifacts_must_exist_and_native_symbols_reject_arduino(self):
        """Modeled files test inspection rejection, never claim a firmware link PASS."""
        path = self.output / "firmware.elf"
        with self.assertRaises(firmware.FirmwareError):
            firmware.artifact(path)
        path.touch()
        with self.assertRaises(firmware.FirmwareError):
            firmware.artifact(path)
        with self.assertRaisesRegex(firmware.FirmwareError, "symbols missing"):
            firmware.verify_symbols("app_main", True)
        with self.assertRaisesRegex(firmware.FirmwareError, "Arduino linkage"):
            firmware.verify_symbols("app_main\nTwoWire::begin()", True)
        native = "\n".join("00000000 T " + name for name in (
            "app_main", "tusb32x_idf::loopExample()", "tusb32x_idf::writeRead(void*)",
            "tusb32x_cli::Cli::tick()", "TUSB32x::TUSB32x::poll(int)",
            "uart_read_bytes", "uart_tx_chars", "i2c_master_transmit", "i2c_master_receive"))
        firmware.verify_symbols(native, True)
        with self.assertRaisesRegex(firmware.FirmwareError, "symbols missing"):
            firmware.verify_symbols(native.replace(" T app_main", " T other_app_main"), True)
        for row in (*firmware.ARDUINO_ROWS, "idf-s2", "idf-s3"):
            self.assertEqual(len(firmware.expected_sources(row)), 11)


if __name__ == "__main__":
    unittest.main()
