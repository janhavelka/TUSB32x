#!/usr/bin/env python3
"""Check the Windows launcher using a temporary executable, without PlatformIO.

The fixture copies Windows CMD as pio.exe beneath a temporary profile. It never
replaces the managed Core or changes the caller's environment. Child processes
have a ten-second timeout; unittest returns nonzero for failures. Other hosts
skip these Windows-specific checks.
"""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


LAUNCHER = Path(__file__).resolve().parents[1] / "scripts/pio.cmd"


@unittest.skipUnless(os.name == "nt", "Windows CMD launcher")
class PioLauncherTests(unittest.TestCase):
    """Exercise exit propagation and missing-install diagnostics in isolation."""

    def setUp(self):
        """Create a fake installation under a profile containing shell characters."""
        temporary = tempfile.TemporaryDirectory(prefix="tusb32x launcher & (profile) ")
        self.addCleanup(temporary.cleanup)
        self.profile = Path(temporary.name)
        self.cmd = Path(os.environ["SystemRoot"]) / "System32/cmd.exe"
        executable = self.profile / ".platformio/penv/Scripts/pio.exe"
        executable.parent.mkdir(parents=True)
        shutil.copyfile(self.cmd, executable)

    def _run(self, status, shadow=None, missing=False):
        """Run actual launcher with controlled child status and local environment."""
        environment = dict(os.environ)
        environment.pop("ERRORLEVEL", None)
        if shadow is not None:
            environment["ERRORLEVEL"] = shadow
        environment["USERPROFILE"] = str(self.profile / "missing" if missing else self.profile)
        command = f'"{self.cmd}" /d /v:off /s /c ""{LAUNCHER}" /d /c exit /b {status}"'
        return subprocess.run(
            command, env=environment, capture_output=True, text=True,
            timeout=10, check=False,
        )

    def test_child_exit_status_ignores_inherited_errorlevel_variable(self):
        """An environment shadow must never turn failed Core commands into success."""
        for shadow in (None, "0", "91", "text"):
            for status in (0, 1, 37, 255):
                with self.subTest(shadow=shadow, status=status):
                    result = self._run(status, shadow=shadow)
                    self.assertEqual(result.returncode, status, result.stdout + result.stderr)
                    self.assertEqual(result.stdout + result.stderr, "")

    def test_missing_installation_reports_path_and_exit_one(self):
        """Missing managed Core fails clearly even with an inherited status shadow."""
        result = self._run(37, shadow="0", missing=True)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertEqual(result.stdout, "")
        self.assertIn(str(self.profile / "missing"), result.stderr)
        self.assertIn("VS Code-managed PlatformIO was not found at:", result.stderr)


if __name__ == "__main__":
    unittest.main()
