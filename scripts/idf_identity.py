"""Generate native-example firmware identity on every clean or incremental build.

Only example CMake invokes this script. The portable core has no Python or Git
dependency. Tokens use the same spelling and unknown-checkout rules as Arduino.
Required --root, --output and --target select the checkout, generated header and
SDK target. Up to three local Git calls each have a 10-second timeout. The output
parent is created and the header replaced; no tracked package version is changed.
Exit 0 means written, argparse exits 2 for invalid arguments, and unhandled file
errors fail the build. No network or device access occurs.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
from pathlib import Path
import re
import subprocess


def git_value(root: Path, *arguments: str) -> str | None:
    """Return stripped Git output for root, or None after error/10-second timeout.

    Arguments are passed as separate local subprocess arguments. The caller
    checks checkout association before using returned revision/tree metadata.
    """
    try:
        return subprocess.check_output(
            ["git", "-C", str(root), *arguments], text=True,
            stderr=subprocess.DEVNULL, timeout=10).strip()
    except (OSError, subprocess.SubprocessError):
        return None


def generate(root: Path, output: Path, target: str) -> None:
    """Replace output with root's revision/tree, current UTC and sanitized target.

    Create output's parent if needed and return None. A missing Git executable,
    failed Git call or different enclosing repository produces unknown evidence.
    Target characters outside ASCII letters/digits/_.- become underscores; at
    most 32 remain, with unknown for an empty target. Filesystem errors propagate.
    """
    root = root.resolve()
    git_root = git_value(root, "rev-parse", "--show-toplevel")
    own_checkout = git_root is not None and Path(git_root).resolve() == root
    revision = git_value(root, "rev-parse", "HEAD") if own_checkout else None
    if revision is not None and re.fullmatch(r"[0-9a-f]{40}", revision) is None:
        revision = None
    changes = git_value(root, "status", "--porcelain") if own_checkout else None
    tree = "unknown" if changes is None else ("dirty" if changes else "clean")
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    target = re.sub(r"[^A-Za-z0-9_.-]", "_", target)[:32] or "unknown"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        "/// @file FirmwareBuild.h\n"
        "/// @brief Generated build identity for the native ESP-IDF example only.\n#pragma once\n"
        f'#define TUSB32X_BUILD_GIT "{revision or "unknown"}"\n'
        f'#define TUSB32X_BUILD_TREE "{tree}"\n'
        f'#define TUSB32X_BUILD_UTC "{stamp}"\n'
        f'#define TUSB32X_BUILD_TARGET "{target}"\n', encoding="utf-8")


def main() -> int:
    """Read process arguments, generate the explicit header and return 0 on success.

    argparse exits 2 for invalid syntax; generation errors propagate to the build.
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--target", required=True)
    arguments = parser.parse_args()
    generate(arguments.root, arguments.output, arguments.target)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
