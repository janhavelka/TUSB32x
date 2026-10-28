"""Exercise generated Arduino identity and compile BoardConfig override contracts."""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
from pathlib import Path
import re
import runpy
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
COMPILER = ""


class BuildEnvironment:
    """Minimal SCons environment recording the script's actual generated include directory."""

    def __init__(self, project: Path, build: Path, target: str = "esp32s2dev"):
        self.values = {"$PROJECT_DIR": str(project), "$BUILD_DIR": str(build), "$PIOENV": target}
        self.includes: list[str] = []

    def subst(self, value: str) -> str:
        """Resolve only variables used by the production identity script."""
        return self.values[value]

    def Append(self, **values) -> None:  # noqa: N802 - actual SCons method spelling.
        """Record emitted include paths and reject unexpected environment mutation."""
        if set(values) != {"CPPPATH"}:
            raise AssertionError(f"Unexpected SCons values: {values}")
        self.includes.extend(values["CPPPATH"])


class ArduinoExamplesTest(unittest.TestCase):
    """Run the real generator and C++ header using isolated temporary inputs."""

    def setUp(self) -> None:
        """Allocate a private temporary tree; never change the workspace's Git state."""
        self.temporary = tempfile.TemporaryDirectory(prefix="tusb32x-arduino-")
        self.addCleanup(self.temporary.cleanup)
        self.directory = Path(self.temporary.name)
        self.project = self.directory / "project"
        self.project.mkdir()
        self.build = self.directory / "build"

    def git(self, *arguments: str) -> str:
        """Run a bounded Git command only inside this test's private repository."""
        return subprocess.check_output(
            ["git", "-C", str(self.project), *arguments], text=True,
            stderr=subprocess.STDOUT, timeout=20).strip()

    def repository(self) -> str:
        """Create one real commit without depending on global author or signing settings."""
        self.git("init", "--quiet")
        (self.project / "tracked.txt").write_text("original\n", encoding="utf-8")
        self.git("add", "tracked.txt")
        self.git("-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid",
                 "-c", "commit.gpgsign=false", "commit", "--quiet", "-m", "fixture")
        return self.git("rev-parse", "HEAD")

    def generate(self, project: Path | None = None, target: str = "esp32s2dev") -> dict[str, str]:
        """Execute production SCons script and inspect its emitted C++ identity tokens."""
        environment = BuildEnvironment(project or self.project, self.build, target)

        def import_environment(name: str) -> None:
            """Assert the real script requests the expected injected SCons variable."""
            self.assertEqual(name, "env")

        before = datetime.now(timezone.utc).replace(microsecond=0)
        runpy.run_path(str(ROOT / "scripts/arduino_identity.py"),
                       init_globals={"env": environment, "Import": import_environment})
        after = datetime.now(timezone.utc).replace(microsecond=0)
        destination = self.build / "example_identity"
        self.assertEqual(environment.includes, [str(destination)])
        header = (destination / "FirmwareBuild.h").read_text(encoding="utf-8")
        tokens = dict(re.findall(r'^#define TUSB32X_BUILD_(\w+) "([^"\n]*)"$', header, re.MULTILINE))
        self.assertEqual(set(tokens), {"GIT", "TREE", "UTC", "TARGET"})
        stamp = datetime.strptime(tokens["UTC"], "%Y%m%dT%H%M%SZ").replace(tzinfo=timezone.utc)
        self.assertLessEqual(before, stamp)
        self.assertLessEqual(stamp, after)
        self.assertRegex(tokens["TARGET"], r"\A[A-Za-z0-9_.-]{1,32}\Z")
        return tokens

    def test_actual_clean_head_then_dirty_tree(self) -> None:
        """Generated HEAD is real and modifying a tracked file changes only tree evidence."""
        revision = self.repository()
        clean = self.generate()
        self.assertEqual((clean["GIT"], clean["TREE"]), (revision, "clean"))
        (self.project / "tracked.txt").write_text("changed\n", encoding="utf-8")
        dirty = self.generate()
        self.assertEqual((dirty["GIT"], dirty["TREE"]), (revision, "dirty"))

    def test_untracked_file_marks_tree_dirty(self) -> None:
        """Untracked source input cannot be described as a clean firmware build."""
        self.repository()
        (self.project / "new.cpp").write_text("// untracked source\n", encoding="utf-8")
        self.assertEqual(self.generate()["TREE"], "dirty")

    def test_nested_unrelated_repository_is_not_firmware_identity(self) -> None:
        """A copied project nested under another Git root reports unknown identity."""
        self.repository()
        nested = self.project / "copied_library"
        nested.mkdir()
        tokens = self.generate(nested)
        self.assertEqual((tokens["GIT"], tokens["TREE"]), ("unknown", "unknown"))

    def test_missing_git_still_generates_build_identity(self) -> None:
        """Generator handles unavailable Git without requiring a fake repository identity."""
        with patch("subprocess.check_output", side_effect=FileNotFoundError("git absent")):
            tokens = self.generate()
        self.assertEqual((tokens["GIT"], tokens["TREE"]), ("unknown", "unknown"))

    def test_target_token_is_sanitized_and_bounded(self) -> None:
        """Unsafe and long environment names cannot escape generated C++ string literals."""
        tokens = self.generate(target='board "bad"/name' + "x" * 80)
        self.assertEqual(tokens["TARGET"], ("board__bad__name" + "x" * 80)[:32])

    def compile_config(self, definitions: list[str], expect_success: bool) -> None:
        """Compile the real header with independent expected override values; never link SDK stubs."""
        source = self.directory / "config.cpp"
        source.write_text(
            '#include "examples/arduino/BoardConfig.h"\n'
            "using B = tusb32x_arduino::BoardConfig;\n"
            "static_assert(B::address == TUSB32X_ADDRESS);\n"
            "static_assert(B::secondAddress == TUSB32X_SECOND_ADDRESS);\n"
            "static_assert(B::variant == TUSB32x::Variant::TUSB32X_VARIANT);\n"
            "static_assert(B::secondVariant == TUSB32x::Variant::TUSB32X_SECOND_VARIANT);\n"
            "int main() { return 0; }\n", encoding="utf-8")
        compiler_name = Path(COMPILER).name.lower()
        if compiler_name in {"cl", "cl.exe"}:
            command = [COMPILER, "/nologo", "/std:c++17", "/W4", "/WX", "/c", str(source),
                       f"/I{ROOT}", f"/I{ROOT / 'include'}", f"/Fo{self.directory / 'config.obj'}"]
            command.extend("/D" + item for item in definitions)
        else:
            command = [COMPILER, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
                       "-fsyntax-only", "-I", str(ROOT), "-I", str(ROOT / "include"), str(source)]
            command.extend("-D" + item for item in definitions)
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=30, cwd=self.directory)
        if expect_success:
            self.assertEqual(result.returncode, 0, result.stdout)
        else:
            self.assertNotEqual(result.returncode, 0, "Invalid configuration compiled successfully")
            self.assertRegex(result.stdout.lower(), r"static assertion|static_assert", result.stdout)

    def test_legal_board_overrides_compile(self) -> None:
        """Default, original 320 and distinct 322 settings are valid C++17 configuration values."""
        cases = [[], ["TUSB32X_VARIANT=TUSB320", "TUSB32X_ADDRESS=0x60",
                      "TUSB32X_SECOND_VARIANT=TUSB320I", "TUSB32X_SECOND_ADDRESS=0x61"],
                 ["TUSB32X_VARIANT=TUSB322I", "TUSB32X_ADDRESS=0x67"],
                 ["TUSB32X_BUS_HZ=400000", "TUSB32X_IRQ=6", "TUSB32X_SECOND_IRQ=7"],
                 ["TUSB32X_BOARD_READY=1", "TUSB32X_MAX_OVERHEAD_US=2000",
                  "TUSB32X_ACCEPT_SDK_CLEANUP=1", "TUSB32X_RUN_MS=1000"]]
        for definitions in cases:
            with self.subTest(definitions=definitions):
                self.compile_config(definitions, True)

    def test_invalid_overrides_fail_before_narrowing(self) -> None:
        """Unsafe address, timing, speed and optional-pin overrides cannot silently become defaults."""
        invalid = ["TUSB32X_ADDRESS=0x147", "TUSB32X_SECOND_ADDRESS=-1",
                   "TUSB32X_CALLBACK_MS=0", "TUSB32X_CALLBACK_MS=4294967296ULL",
                   "TUSB32X_OPERATION_MS=-1", "TUSB32X_MAX_OVERHEAD_US=-1",
                   "TUSB32X_RUN_MS=9223372036854775808ULL", "TUSB32X_RUN_MS=-1",
                   "TUSB32X_IRQ=-2", "TUSB32X_SECOND_IRQ=-2", "TUSB32X_ENABLE=-2",
                   "TUSB32X_BUS_HZ=0", "TUSB32X_BUS_HZ=400001",
                   "TUSB32X_BUS_HZ=4295067296ULL", "TUSB32X_VARIANT=UNSPECIFIED",
                   "TUSB32X_SECOND_VARIANT=UNSPECIFIED", "TUSB32X_ADDRESS=0x60",
                   "TUSB32X_SDA=4294967295ULL", "TUSB32X_SCL=4294967295ULL",
                   "TUSB32X_IRQ=4294967295ULL", "TUSB32X_SECOND_IRQ=4294967295ULL",
                   "TUSB32X_ENABLE=4294967295ULL"]
        for definition in invalid:
            with self.subTest(definition=definition):
                self.compile_config([definition], False)
        self.compile_config(["TUSB32X_VARIANT=TUSB320", "TUSB32X_ADDRESS=0x47"], False)
        self.compile_config(["TUSB32X_ENABLE=6", "TUSB32X_IRQ=6"], False)
        self.compile_config(["TUSB32X_ENABLE=7", "TUSB32X_SECOND_IRQ=7"], False)


def main() -> int:
    """Require the active CMake compiler and Git, then execute registered behavioral checks."""
    global COMPILER
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", required=True, help="CMake's actual C++ compiler path")
    arguments = parser.parse_args()
    COMPILER = arguments.compiler
    if shutil.which("git") is None:
        parser.error("Git is required for the isolated actual-checkout identity test")
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(ArduinoExamplesTest)
    return 0 if unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
