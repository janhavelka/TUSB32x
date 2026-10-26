#!/usr/bin/env python3
"""Test the version CLI in temporary extracted trees, never repository metadata.

Run with ``python tools/test_version_sync.py`` or unittest discovery. Every child
Python invocation has a 10-second timeout. Fixtures copy only the generator and
its managed inputs/outputs; temporary files are removed by unittest cleanup.
Exit zero means all tests passed; unittest returns nonzero for failures/errors.
The original checkout's bytes and modification times are checked after the suite.
"""

import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
FIXTURE_FILES = (
    "library.json",
    "CMakeLists.txt",
    "idf_component.yml",
    "Doxyfile",
    "include/TUSB32x/Version.h",
    "scripts/generate_version.py",
)
OUTPUT_FILES = FIXTURE_FILES[1:5]
TIMEOUT_SECONDS = 10


def _snapshot(root, names=FIXTURE_FILES):
    """Return bytes and nanosecond mtimes for existing named files under root."""
    return {
        name: ((root / name).read_bytes(), (root / name).stat().st_mtime_ns)
        for name in names
        if (root / name).is_file()
    }


class VersionSyncTests(unittest.TestCase):
    """Exercise version truth, strict CLI failures and Git-independent output."""

    @classmethod
    def setUpClass(cls):
        """Record checkout state before any temporary fixture subprocess runs."""
        cls.checkout_before = _snapshot(ROOT)
        missing = set(FIXTURE_FILES) - cls.checkout_before.keys()
        if missing:
            raise AssertionError(f"Missing version fixture inputs: {sorted(missing)}")

    @classmethod
    def tearDownClass(cls):
        """Fail if a fixture changed checkout metadata, even without byte drift."""
        if cls.checkout_before != _snapshot(ROOT):
            raise AssertionError("Version fixtures modified the library checkout")

    def setUp(self):
        """Create one isolated copy; individual matrix cases may create more."""
        self.root = self._make_fixture()

    def _make_fixture(self, parent=None):
        """Copy required inputs beneath a temporary root, outside checkout Git."""
        temporary = tempfile.TemporaryDirectory(prefix="tusb32x-version-", dir=parent)
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        for name in FIXTURE_FILES:
            destination = root / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, destination)
        return root

    def _run(self, *arguments, root=None, environment=None, cwd=None):
        """Run the copied CLI with a bounded wait and captured diagnostics."""
        root = self.root if root is None else root
        return subprocess.run(
            [sys.executable, str(root / "scripts/generate_version.py"), *arguments],
            cwd=root if cwd is None else cwd,
            env=environment,
            capture_output=True,
            text=True,
            timeout=TIMEOUT_SECONDS,
            check=False,
        )

    def _assert_exit(self, expected, *arguments, root=None, **kwargs):
        """Return CLI results after asserting the documented exact exit code."""
        result = self._run(*arguments, root=root, **kwargs)
        self.assertEqual(result.returncode, expected, result.stdout + result.stderr)
        return result

    def _manifest(self, root=None):
        """Read the temporary manifest as JSON for a single fixture case."""
        root = self.root if root is None else root
        return json.loads((root / "library.json").read_text(encoding="utf-8"))

    def _write_manifest(self, metadata, root=None):
        """Write JSON only in the specified temporary fixture."""
        root = self.root if root is None else root
        (root / "library.json").write_text(
            json.dumps(metadata, indent=2) + "\n", encoding="utf-8"
        )

    def _assert_versions(self, version, root=None):
        """Verify full/core versions independently in every managed artifact."""
        root = self.root if root is None else root
        core = version.split("-", 1)[0].split("+", 1)[0]
        self.assertEqual(self._manifest(root)["version"], version)
        cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn(f'set(TUSB32X_VERSION "{version}")', cmake)
        self.assertIn(f"project(TUSB32x VERSION {core} LANGUAGES CXX)", cmake)
        component = (root / "idf_component.yml").read_text(encoding="utf-8")
        self.assertRegex(component, rf'(?m)^version:\s*"{re.escape(version)}"\s*$')
        doxyfile = (root / "Doxyfile").read_text(encoding="utf-8")
        self.assertRegex(doxyfile, rf'(?m)^PROJECT_NUMBER\s*=\s*"{re.escape(version)}"\s*$')
        header = (root / "include/TUSB32x/Version.h").read_text(encoding="utf-8")
        for name, value in zip(("MAJOR", "MINOR", "PATCH"), core.split(".")):
            self.assertRegex(
                header,
                rf"inline\s+constexpr\s+(?:std::)?uint32_t\s+VERSION_{name}"
                rf"\s*=\s*{value}[uU]?\s*;",
            )
        self.assertRegex(
            header, rf'inline\s+constexpr\s+char\s+VERSION\[\]\s*=\s*"{re.escape(version)}"\s*;'
        )
        for volatile in ("__DATE__", "__TIME__", "GIT_COMMIT", "GIT_BRANCH", "BUILD_TIMESTAMP"):
            self.assertNotIn(volatile, header)

    def test_checkout_copy_is_current(self):
        """The committed artifact set must already match manifest truth."""
        self._assert_exit(0, "check")
        self._assert_versions(self._manifest()["version"])

    def test_manifest_change_reports_all_stale_paths_and_syncs(self):
        """Check reports drift without writing; sync repairs every version field."""
        metadata = self._manifest()
        metadata["version"] = "7.8.9-rc.2+fixture.01"
        self._write_manifest(metadata)
        before = _snapshot(self.root)
        result = self._assert_exit(1, "check")
        self.assertEqual(before, _snapshot(self.root))
        diagnostics = (result.stdout + result.stderr).replace("\\", "/")
        for name in OUTPUT_FILES:
            self.assertIn(name, diagnostics)
        self._assert_exit(0, "sync")
        self._assert_versions(metadata["version"])
        self._assert_exit(0, "check")

    def test_each_managed_field_detects_and_repairs_drift(self):
        """Full/core CMake and each other artifact independently cause exit one."""
        mutations = (
            ("CMakeLists.txt", r'(?m)^set\(TUSB32X_VERSION .*$', 'set(TUSB32X_VERSION "9.9.9")'),
            ("CMakeLists.txt", r'(?m)^project\(TUSB32x .*$',
             "project(TUSB32x VERSION 9.9.9 LANGUAGES CXX)"),
            ("idf_component.yml", r'(?m)^version:.*$', 'version: "9.9.9"'),
            ("Doxyfile", r'(?m)^PROJECT_NUMBER\s*=.*$', 'PROJECT_NUMBER = "9.9.9"'),
            ("include/TUSB32x/Version.h", r"VERSION_MAJOR\s*=\s*\d+[uU]?", "VERSION_MAJOR = 999"),
        )
        for name, pattern, replacement in mutations:
            with self.subTest(path=name, pattern=pattern):
                root = self._make_fixture()
                path = root / name
                changed, count = re.subn(pattern, replacement, path.read_text(encoding="utf-8"))
                self.assertEqual(count, 1)
                path.write_text(changed, encoding="utf-8")
                before = _snapshot(root)
                result = self._assert_exit(1, "check", root=root)
                self.assertIn(name, (result.stdout + result.stderr).replace("\\", "/"))
                self.assertEqual(before, _snapshot(root))
                self._assert_exit(0, "sync", root=root)
                self._assert_versions(self._manifest(root)["version"], root)
                self._assert_exit(0, "check", root=root)

    def test_missing_header_is_reported_and_regenerated(self):
        """An extracted tree may regenerate an absent header without Git."""
        path = self.root / "include/TUSB32x/Version.h"
        expected = path.read_bytes()
        path.unlink()
        self._assert_exit(1, "check")
        self.assertFalse(path.exists())
        self._assert_exit(0, "sync")
        self.assertEqual(path.read_bytes(), expected)
        self._assert_exit(0, "check")

    def test_repeated_sync_and_same_set_preserve_bytes_and_mtimes(self):
        """No-op commands must not rewrite any manifest or generated file."""
        before = _snapshot(self.root)
        for arguments in (("sync",), ("sync",), ("set", self._manifest()["version"]), ("check",)):
            self._assert_exit(0, *arguments)
            self.assertEqual(before, _snapshot(self.root))

    def test_missing_invalid_and_unreadable_manifest_errors(self):
        """Metadata failures return two and leave all other artifacts untouched."""
        malformed = ("{}", '{"version": null}', '{"version": 1}', "[]", "{", '{"version": true}')
        manifest = self.root / "library.json"
        outputs = _snapshot(self.root, OUTPUT_FILES)
        for content in malformed:
            with self.subTest(content=content):
                manifest.write_text(content, encoding="utf-8")
                self._assert_exit(2, "sync")
                self._assert_exit(2, "check")
                self.assertEqual(outputs, _snapshot(self.root, OUTPUT_FILES))
        manifest.write_bytes(b"\xff")
        self._assert_exit(2, "sync")
        manifest.unlink()
        self._assert_exit(2, "sync")
        self.assertEqual(outputs, _snapshot(self.root, OUTPUT_FILES))

    def test_malformed_semvers_are_rejected_without_mutation(self):
        """Reject non-ASCII, leading zero, suffix and uint32 range violations."""
        invalid = (
            "", "1", "1.2", "1.2.3.4", "v1.2.3", "01.2.3", "1.02.3", "1.2.03",
            "1.2.3-", "1.2.3+", "1.2.3-01", "1.2.3-alpha.01", "1.2.3-a..b",
            "1.2.3+a..b", "1.2.3-a_b", "1.2.3+abc_def", "1.2.3\n", " 1.2.3",
            "1.2.3 ", "\u0661.2.3", "1.2.3-\u03b2", "4294967296.0.0", "0.4294967296.0",
            "0.0.4294967296",
        )
        metadata = self._manifest()
        for version in invalid:
            with self.subTest(version=version):
                metadata["version"] = version
                self._write_manifest(metadata)
                before = _snapshot(self.root)
                self._assert_exit(2, "sync")
                self.assertEqual(before, _snapshot(self.root))
        self._write_manifest({**metadata, "version": "1.2.3"})
        self._assert_exit(0, "sync")
        for version in invalid:
            with self.subTest(set_version=version):
                before = _snapshot(self.root)
                self._assert_exit(2, "set", version)
                self.assertEqual(before, _snapshot(self.root))

    def test_set_accepts_semver_suffixes_and_uint32_max(self):
        """Set preserves full SemVer text while CMake uses the numeric core."""
        for version in (
            "0.0.0", "1.2.3-alpha.0.1-x+001.build-2", "3.2.1+001",
            "4294967295.4294967295.4294967295",
        ):
            with self.subTest(version=version):
                self._assert_exit(0, "set", version)
                self._assert_versions(version)
                self._assert_exit(0, "check")

    def test_bump_resets_lower_fields_and_discards_suffixes(self):
        """Each bump changes the selected core component without release suffixes."""
        for part, expected in (("patch", "2.7.14"), ("minor", "2.8.0"), ("major", "3.0.0")):
            with self.subTest(part=part):
                self._assert_exit(0, "set", "2.7.13-rc.1+build.4")
                self._assert_exit(0, "bump", part)
                self._assert_versions(expected)
                self._assert_exit(0, "check")

    def test_version_updates_preserve_unmanaged_content(self):
        """Set/bump preserve package policy and unrelated build/template settings."""
        metadata = self._manifest()
        metadata["fixture-extension"] = {"text": "keep this", "numbers": [1, 2, 3]}
        self._write_manifest(metadata)
        unrelated = {}
        for name in ("CMakeLists.txt", "idf_component.yml", "Doxyfile"):
            path = self.root / name
            unrelated[name] = "# Fixture comment: preserve unrelated settings.\n"
            path.write_text(path.read_text(encoding="utf-8") + unrelated[name], encoding="utf-8")
        for arguments in (("set", "3.8.1-rc.2+test"), ("bump", "minor")):
            self._assert_exit(0, *arguments)
            actual = self._manifest()
            self.assertEqual(
                {key: value for key, value in metadata.items() if key != "version"},
                {key: value for key, value in actual.items() if key != "version"},
            )
            for name, comment in unrelated.items():
                self.assertTrue((self.root / name).read_text(encoding="utf-8").endswith(comment))

    def test_bump_overflow_is_error_two_without_partial_updates(self):
        """A selected uint32-max component cannot wrap or modify metadata."""
        cases = (
            ("patch", "1.2.4294967295"), ("minor", "1.4294967295.2"),
            ("major", "4294967295.1.2"),
        )
        for part, version in cases:
            with self.subTest(part=part):
                self._assert_exit(0, "set", version)
                before = _snapshot(self.root)
                self._assert_exit(2, "bump", part)
                self.assertEqual(before, _snapshot(self.root))

    def test_cli_requires_explicit_commands_and_exact_arity(self):
        """Unknown commands/options, missing operands and extras return two."""
        invalid = (
            (), ("unknown",), ("sync", "extra"), ("check", "extra"), ("bump",),
            ("bump", "PATCH"), ("bump", "other"), ("bump", "patch", "extra"),
            ("set",), ("set", "1.2.3", "extra"), ("--root", ".", "sync"),
            ("sync", "--root", "."), ("--invalid",),
        )
        before = _snapshot(self.root)
        for arguments in invalid:
            with self.subTest(arguments=arguments):
                result = self._assert_exit(2, *arguments)
                self.assertTrue(result.stdout or result.stderr)
                self.assertEqual(before, _snapshot(self.root))
        self._assert_exit(0, "--help")
        self.assertEqual(before, _snapshot(self.root))

    def test_missing_duplicate_and_unreadable_templates_preflight_all_commands(self):
        """Bad managed declarations are diagnosed before set/bump/sync writes."""
        declarations = (
            ("CMakeLists.txt", r'(?m)^set\(TUSB32X_VERSION .*\n?'),
            ("CMakeLists.txt", r'(?m)^project\(TUSB32x .*\n?'),
            ("idf_component.yml", r'(?m)^version:.*\n?'),
            ("Doxyfile", r'(?m)^PROJECT_NUMBER\s*=.*\n?'),
        )
        for name, pattern in declarations:
            for fault in ("missing", "duplicate"):
                with self.subTest(path=name, fault=fault):
                    root = self._make_fixture()
                    path = root / name
                    original = path.read_text(encoding="utf-8")
                    match = re.search(pattern, original)
                    self.assertIsNotNone(match)
                    if fault == "missing":
                        changed = re.sub(pattern, "", original)
                    else:
                        changed = original + "\n" + match[0]
                    path.write_text(changed, encoding="utf-8")
                    before = _snapshot(root)
                    for arguments in (("sync",), ("check",), ("set", "8.1.2"), ("bump", "patch")):
                        self._assert_exit(2, *arguments, root=root)
                        self.assertEqual(before, _snapshot(root))
        for name in ("CMakeLists.txt", "idf_component.yml", "Doxyfile"):
            with self.subTest(unreadable=name):
                root = self._make_fixture()
                (root / name).write_bytes(b"\xff")
                before = _snapshot(root)
                self._assert_exit(2, "set", "8.1.2", root=root)
                self.assertEqual(before, _snapshot(root))

    def test_output_io_failure_returns_two(self):
        """A directory in place of Version.h is an error rather than success."""
        header = self.root / "include/TUSB32x/Version.h"
        header.unlink()
        header.mkdir()
        before = _snapshot(self.root)
        self._assert_exit(2, "sync")
        self.assertEqual(before, _snapshot(self.root))

    def test_alternate_duplicate_declarations_are_rejected(self):
        """Alternate quoting, comments or layout cannot hide version overrides."""
        overrides = (
            ("CMakeLists.txt", 'set(TUSB32X_VERSION "9.9.9") # override'),
            ("CMakeLists.txt", '  SET (\n TUSB32X_VERSION 9.9.9)'),
            ("CMakeLists.txt", 'set("TUSB32X_VERSION" 9.9.9)'),
            ("CMakeLists.txt", 'PROJECT(TUSB32x VERSION 9.9.9 LANGUAGES CXX)'),
            ("CMakeLists.txt", 'project("TUSB32x" VERSION 9.9.9 LANGUAGES CXX)'),
            ("idf_component.yml", "version: '9.9.9'"),
            ("idf_component.yml", '"version": "9.9.9"'),
            ("Doxyfile", "PROJECT_NUMBER = 9.9.9"),
            ("Doxyfile", '  PROJECT_NUMBER = "9.9.9" # override'),
        )
        for name, declaration in overrides:
            with self.subTest(path=name, declaration=declaration):
                root = self._make_fixture()
                path = root / name
                with path.open("a", encoding="utf-8", newline="\n") as stream:
                    stream.write("\n" + declaration + "\n")
                before = _snapshot(root)
                for arguments in (("check",), ("sync",), ("set", "8.1.2"), ("bump", "patch")):
                    self._assert_exit(2, *arguments, root=root)
                    self.assertEqual(before, _snapshot(root))

    def test_no_git_path_and_unrelated_working_directory(self):
        """An extracted copy uses its script location with no Git on PATH."""
        environment = dict(os.environ, PATH="")
        self._assert_exit(
            0, "set", "5.6.7-dev.0+archive", environment=environment, cwd=self.root.parent
        )
        self._assert_versions("5.6.7-dev.0+archive")
        self._assert_exit(0, "check", environment=environment, cwd=self.root.parent)

    def test_generator_never_attempts_child_processes(self):
        """Observe process audit events so swallowed Git errors cannot pass."""
        marker = self.root / "child-process-attempted"
        # The wrapper observes normal public CLI execution; it does not import
        # private helpers or modify the copied generator's implementation.
        wrapper = self.root / "observe_processes.py"
        wrapper.write_text(
            "import pathlib, runpy, sys\n"
            "marker = pathlib.Path(sys.argv[1])\n"
            "def observe(event, arguments):\n"
            "    if event in ('subprocess.Popen', 'os.system', 'os.posix_spawn', 'os.exec'):\n"
            "        marker.write_text(event, encoding='ascii')\n"
            "        raise RuntimeError('Version generation must not start child processes')\n"
            "sys.addaudithook(observe)\n"
            "sys.argv = sys.argv[2:]\n"
            "runpy.run_path(sys.argv[0], run_name='__main__')\n",
            encoding="utf-8",
        )
        for arguments in (("sync",), ("check",), ("set", "4.5.6+archive"), ("bump", "patch")):
            with self.subTest(arguments=arguments):
                result = subprocess.run(
                    [sys.executable, str(wrapper), str(marker),
                     str(self.root / "scripts/generate_version.py"), *arguments],
                    cwd=self.root,
                    capture_output=True,
                    text=True,
                    timeout=TIMEOUT_SECONDS,
                    check=False,
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertFalse(marker.exists(), "Generator attempted a child process")

    def test_nested_git_and_dirty_example_do_not_change_version_text(self):
        """Parent Git identity and changed application code cannot enter Version.h."""
        enclosing = tempfile.TemporaryDirectory(prefix="tusb32x-unrelated-git-")
        self.addCleanup(enclosing.cleanup)
        parent = Path(enclosing.name)
        git = parent / ".git"
        (git / "objects").mkdir(parents=True)
        (git / "refs/heads").mkdir(parents=True)
        (git / "HEAD").write_text("ref: refs/heads/unrelated-application\n", encoding="ascii")
        (git / "refs/heads/unrelated-application").write_text("a" * 40 + "\n", encoding="ascii")
        (git / "config").write_text(
            "[core]\n\trepositoryformatversion = 0\n\tbare = false\n", encoding="ascii"
        )
        root = self._make_fixture(parent)
        expected = (root / "include/TUSB32x/Version.h").read_bytes()
        source = root / "examples/host/basic/main.cpp"
        source.parent.mkdir(parents=True)
        source.write_text(
            "// Changed application; not flashed firmware evidence.\n", encoding="utf-8"
        )
        source_before = source.read_bytes(), source.stat().st_mtime_ns
        before = _snapshot(root)
        self._assert_exit(0, "sync", root=root)
        self._assert_exit(0, "check", root=root)
        self.assertEqual(expected, (root / "include/TUSB32x/Version.h").read_bytes())
        self.assertEqual(before, _snapshot(root))
        self.assertEqual(source_before, (source.read_bytes(), source.stat().st_mtime_ns))


if __name__ == "__main__":
    unittest.main()
