"""Generate example-only firmware identity from the actual build checkout and UTC time."""
from datetime import datetime, timezone
from pathlib import Path
import re
import subprocess

Import("env")  # type: ignore[name-defined]  # SCons supplies the active firmware environment.


def git_value(*args):
    """Read this checkout without depending on Git in downstream core consumers."""
    try:
        return subprocess.check_output(
            ["git", "-C", env.subst("$PROJECT_DIR"), *args],
            text=True, stderr=subprocess.DEVNULL, timeout=10).strip()
    except (OSError, subprocess.SubprocessError):
        return None


project = Path(env.subst("$PROJECT_DIR")).resolve()
git_root = git_value("rev-parse", "--show-toplevel")
own_checkout = git_root is not None and Path(git_root).resolve() == project
revision = git_value("rev-parse", "HEAD") if own_checkout else None
if revision is not None and re.fullmatch(r"[0-9a-f]{40}", revision) is None:
    revision = None
changes = git_value("status", "--porcelain") if own_checkout else None
tree = "unknown" if changes is None else ("dirty" if changes else "clean")
build = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
target = re.sub(r"[^A-Za-z0-9_.-]", "_", env.subst("$PIOENV"))[:32]
destination = Path(env.subst("$BUILD_DIR")) / "example_identity"
destination.mkdir(parents=True, exist_ok=True)
(destination / "FirmwareBuild.h").write_text(
    "// Generated build identity for Arduino examples only.\n#pragma once\n"
    f'#define TUSB32X_BUILD_GIT "{revision or "unknown"}"\n'
    f'#define TUSB32X_BUILD_TREE "{tree}"\n'
    f'#define TUSB32X_BUILD_UTC "{build}"\n'
    f'#define TUSB32X_BUILD_TARGET "{target}"\n', encoding="utf-8")
env.Append(CPPPATH=[str(destination)])
