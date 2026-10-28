"""Exercise native example CMake isolation and real incremental identity generation.

The CMake fixture supplies only IDF registration, not SDK implementations. These
host checks prove build wiring; independent actual IDF links prove SDK linkage.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import importlib.util
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
COMPILER = ""
CMAKE = "cmake"
GENERATOR = "Ninja"
SPEC = importlib.util.spec_from_file_location("idf_identity", ROOT / "scripts/idf_identity.py")
assert SPEC is not None and SPEC.loader is not None
IDENTITY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(IDENTITY)


class IdfExampleTest(unittest.TestCase):
    """Run the production generator and build rules inside private renamed roots."""

    def setUp(self) -> None:
        """Allocate an isolated repository with no changes to the working checkout."""
        temporary = tempfile.TemporaryDirectory(prefix="tusb32x-idf-")
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)
        self.project = self.directory / "renamed-component"
        self.project.mkdir()
        self.output = self.directory / "identity/FirmwareBuild.h"

    def run_command(self, command: list[str], **keywords) -> str:
        """Run a bounded fixture command and preserve complete failure output."""
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, timeout=90, **keywords)
        self.assertEqual(result.returncode, 0, result.stdout)
        return result.stdout

    def git(self, *arguments: str) -> str:
        """Change only this fixture's Git state, never that of the library checkout."""
        return self.run_command(["git", "-C", str(self.project), *arguments]).strip()

    def repository(self) -> str:
        """Commit fixture inputs with explicit local identity and signing disabled."""
        self.git("init", "--quiet")
        (self.project / "tracked.txt").write_text("original\n", encoding="utf-8")
        self.git("add", ".")
        self.git("-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid",
                 "-c", "commit.gpgsign=false", "commit", "--quiet", "-m", "fixture")
        return self.git("rev-parse", "HEAD")

    def tokens(self, path: Path | None = None) -> dict[str, str]:
        """Decode exact emitted macro names and check timestamp and token bounds."""
        header = (path or self.output).read_text(encoding="utf-8")
        values = dict(re.findall(r'^#define TUSB32X_BUILD_(\w+) "([^"\n]*)"$',
                                 header, re.MULTILINE))
        self.assertEqual(set(values), {"GIT", "TREE", "UTC", "TARGET"})
        datetime.strptime(values["UTC"], "%Y%m%dT%H%M%SZ").replace(tzinfo=timezone.utc)
        self.assertRegex(values["TARGET"], r"\A[A-Za-z0-9_.-]{1,32}\Z")
        return values

    def generate(self, root: Path | None = None, target: str = "esp32s2") -> dict[str, str]:
        """Call the production generator with an exact checkout root and target."""
        IDENTITY.generate(root or self.project, self.output, target)
        return self.tokens()

    def test_exact_checkout_clean_dirty_and_untracked(self) -> None:
        """Real checkout changes are reflected without requiring CMake reconfiguration."""
        revision = self.repository()
        self.assertEqual(self.generate()["GIT"], revision)
        self.assertEqual(self.generate()["TREE"], "clean")
        (self.project / "tracked.txt").write_text("modified\n", encoding="utf-8")
        self.assertEqual(self.generate()["TREE"], "dirty")
        (self.project / "tracked.txt").write_text("original\n", encoding="utf-8")
        (self.project / "new.cpp").write_text("// untracked\n", encoding="utf-8")
        self.assertEqual(self.generate()["TREE"], "dirty")

    def test_absent_git_and_unrelated_parent_report_unknown(self) -> None:
        """Archive builds do not borrow an enclosing repository's firmware identity."""
        with patch.object(IDENTITY.subprocess, "check_output", side_effect=FileNotFoundError):
            values = self.generate()
        self.assertEqual((values["GIT"], values["TREE"]), ("unknown", "unknown"))
        self.repository()
        nested = self.project / "archive"
        nested.mkdir()
        values = self.generate(root=nested)
        self.assertEqual((values["GIT"], values["TREE"]), ("unknown", "unknown"))

    def test_target_is_sanitized(self) -> None:
        """Unsafe, long and empty target values cannot escape generated literals."""
        self.assertEqual(self.generate(target='bad"/name' + "x" * 80)["TARGET"],
                         ("bad__name" + "x" * 80)[:32])
        self.assertEqual(self.generate(target="")["TARGET"], "unknown")

    def prepare_cmake(self) -> tuple[Path, dict[str, str]]:
        """Copy actual build rules and replace SDK/application bodies with host inputs."""
        app = self.project / "examples/esp_idf/basic"
        main = app / "main"
        main.mkdir(parents=True)
        for relative in ("CMakeLists.txt", "main/CMakeLists.txt", "sdkconfig.defaults",
                         "sdkconfig.defaults.esp32s3"):
            shutil.copyfile(ROOT / "examples/esp_idf/basic" / relative, app / relative)
        (self.project / "scripts").mkdir()
        shutil.copyfile(ROOT / "scripts/idf_identity.py", self.project / "scripts/idf_identity.py")
        shutil.copyfile(ROOT / "library.json", self.project / "library.json")
        sources = [main / "main.cpp", app.parent / "ExampleApp.cpp",
                   app.parent / "Tusb32xIdf.cpp"]
        common = self.project / "examples/common"
        common.mkdir()
        sources.extend(common / name for name in
                       ("Tusb32xCli.cpp", "Tusb32xCliOutput.cpp", "ExampleOwner.cpp"))
        for index, source in enumerate(sources):
            source.write_text(f"int fixture{index}() {{ return {index}; }}\n", encoding="utf-8")
        (main / "main.cpp").write_text(
            '#include "FirmwareBuild.h"\n'
            'const char* fixtureTree() { return TUSB32X_BUILD_TREE; }\n'
            'static_assert(TUSB32X_BOARD_READY == 1);\n', encoding="utf-8")
        sdk = self.directory / "registration-sdk"
        cmake = sdk / "tools/cmake"
        cmake.mkdir(parents=True)
        (cmake / "project.cmake").write_text(
            "function(idf_build_set_property property value)\n"
            '  set(${property} "${value}" PARENT_SCOPE)\n'
            "endfunction()\n"
            "macro(project)\n"
            "  _project(${ARGV} LANGUAGES CXX)\n"
            '  file(WRITE "${SDKCONFIG}" "TARGET=${IDF_TARGET}\\n")\n'
            '  file(WRITE "${DEPENDENCIES_LOCK}" "TARGET=${IDF_TARGET}\\n")\n'
            '  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/main")\n'
            "endmacro()\n"
            "function(idf_component_register)\n"
            '  cmake_parse_arguments(R "" "" "SRCS;INCLUDE_DIRS;REQUIRES" ${ARGN})\n'
            '  file(WRITE "${CMAKE_BINARY_DIR}/requires.txt" "${R_REQUIRES}")\n'
            '  file(WRITE "${CMAKE_BINARY_DIR}/sources.txt" "${R_SRCS}")\n'
            '  add_library(fixture_main STATIC ${R_SRCS})\n'
            '  set(COMPONENT_LIB fixture_main PARENT_SCOPE)\n'
            "endfunction()\n", encoding="utf-8")
        environment = os.environ.copy()
        environment["IDF_PATH"] = str(sdk)
        self.repository()
        return app, environment

    def test_renamed_root_separate_configs_and_incremental_identity(self) -> None:
        """Actual CMake wiring derives REQUIRES, isolates targets and regenerates identity."""
        app, environment = self.prepare_cmake()
        outputs: dict[str, Path] = {}
        for target in ("esp32s2", "esp32s3"):
            build = self.directory / target
            self.run_command([CMAKE, "-S", str(app), "-B", str(build), "-G", GENERATOR,
                              f"-DCMAKE_CXX_COMPILER={COMPILER}", f"-DPYTHON={sys.executable}",
                              f"-DIDF_TARGET={target}", "-DTUSB32X_BOARD_READY=1"], env=environment)
            requirements = (build / "requires.txt").read_text(encoding="utf-8").split(";")
            self.assertEqual(requirements[0], self.project.name)
            self.assertIn("esp_driver_uart", requirements)
            self.assertIn("esp_driver_i2c", requirements)
            self.assertEqual(len((build / "sources.txt").read_text(encoding="utf-8").split(";")), 6)
            self.run_command([CMAKE, "--build", str(build), "--config", "Debug"], env=environment)
            outputs[target] = build
            values = self.tokens(build / "main/example_identity/FirmwareBuild.h")
            self.assertEqual((values["TARGET"], values["TREE"]), (target, "clean"))
        s2_config = (outputs["esp32s2"] / "sdkconfig").read_bytes()
        self.assertEqual((outputs["esp32s2"] / "sdkconfig").read_text(encoding="utf-8"),
                         "TARGET=esp32s2\n")
        self.assertEqual((outputs["esp32s3"] / "sdkconfig").read_text(encoding="utf-8"),
                         "TARGET=esp32s3\n")
        (app / "main/main.cpp").write_text(
            (app / "main/main.cpp").read_text(encoding="utf-8") + "// source changed\n",
            encoding="utf-8")
        self.run_command([CMAKE, "--build", str(outputs["esp32s2"]), "--config", "Debug"],
                         env=environment)
        self.assertEqual(self.tokens(outputs["esp32s2"] / "main/example_identity/FirmwareBuild.h")
                         ["TREE"], "dirty")
        self.assertEqual((outputs["esp32s2"] / "sdkconfig").read_bytes(), s2_config)
        self.assertFalse((app / "sdkconfig").exists())
        self.assertFalse((app / "dependencies.lock").exists())
        for target, build in outputs.items():
            self.assertEqual((build / "dependencies.lock").read_text(encoding="utf-8"),
                             f"TARGET={target}\n")


def main() -> int:
    """Use CMake's selected compiler for executable host build-rule checks."""
    global COMPILER, CMAKE, GENERATOR
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--generator", default="Ninja")
    arguments = parser.parse_args()
    COMPILER, CMAKE, GENERATOR = arguments.compiler, arguments.cmake, arguments.generator
    if shutil.which("git") is None:
        parser.error("Git is required for isolated checkout identity tests")
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(IdfExampleTest)
    return 0 if unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
