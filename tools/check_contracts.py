#!/usr/bin/env python3
"""Check maintained source and metadata boundaries without building or running code.

Run with Python 3.10+ and optional --root PATH (the checkout by default). Exit 0
means the static checks passed, 1 means a contract violation, and 2 means invalid
arguments or an unreadable root. It reads files without writes, network, hardware
or subprocesses. These checks supplement compiled headers and the allocation-sentinel
consumer; they cannot establish runtime allocation freedom or C++ portability.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys

from check_package import PackageError, package_files


ROOT = Path(__file__).resolve().parents[1]
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx"}
HEADER_SUFFIXES = {".h", ".hh", ".hpp", ".hxx"}
# Review additions explicitly: an unknown system include must not silently grant
# a platform dependency or an allocating standard-library implementation.
CORE_SYSTEM_HEADERS = {
    "array", "cassert", "cinttypes", "climits", "cstddef", "cstdint", "cstring",
    "initializer_list", "limits", "type_traits", "utility",
}
COMMON_SYSTEM_HEADERS = CORE_SYSTEM_HEADERS | {"atomic", "cctype", "cstdarg", "cstdio"}
INCLUDE = re.compile(r'^[ \t]*#[ \t]*include\s*([<"])([^>"\n]+)[>"]\s*$', re.M)
NON_CODE = re.compile(
    r'/\*.*?\*/|//[^\n]*|'
    r'(?:u8|u|U|L)?R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimiter)"|'
    r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S,
)
FORBIDDEN = {
    "framework API": re.compile(
        r"\b(?:Arduino|TwoWire|String|Serial[0-9]*|Wire[0-9]*|"
        r"esp_\w+|ESP_LOG\w+|[vx]Task\w+|xSemaphore\w+|gpio_\w+|i2c_\w+)\b"),
    "platform time/wait": re.compile(
        r"\b(?:millis|micros|delay|delayMicroseconds|yield|sleep|usleep|nanosleep|"
        r"sleep_for|sleep_until|clock|time|clock_gettime|gettimeofday)\s*\(|"
        r"\b(?:chrono|this_thread|steady_clock|system_clock)\s*::"),
    "allocation": re.compile(
        r"\b(?:new|delete)\b|\b(?:malloc|calloc|realloc|free|aligned_alloc|"
        r"strdup|strndup|make_unique|make_shared|allocate_shared)\s*\(|"
        r"\bstd\s*::\s*(?:allocator|vector|string|deque|list|map|unordered_map)\b"),
    "exception control flow": re.compile(r"\b(?:throw|try|catch)\b"),
}
CORE_OUTPUT = re.compile(
    r"\b(?:printf|fprintf|sprintf|snprintf|vprintf|vfprintf|vsprintf|vsnprintf|"
    r"puts|fputs|putchar|perror)\s*\(|\bstd\s*::\s*(?:cout|cerr|clog)\b")


def masked(text: str, keep_strings: bool = False) -> str:
    """Remove comments and optionally literals while preserving line boundaries.

    Match comments and literals together so comment markers inside diagnostic
    strings cannot hide following code. Splice continued physical lines first,
    as the C++ preprocessor does. Raw string contents are not executable tokens.
    """
    text = text.replace("\\\r\n", "").replace("\\\n", "")

    def replace(match: re.Match) -> str:
        """Mask one non-code token without moving subsequent directive positions."""
        token = match[0]
        if keep_strings and not token.startswith(("//", "/*")):
            return token
        return "".join("\n" if char == "\n" else " " for char in token)

    return NON_CODE.sub(replace, text)


def source_errors(root: Path) -> list[str]:
    """Check portable core/common sources and public-to-private include direction.

    The include allowlist is deliberately explicit. Public headers may include
    other public headers and approved C++17 headers; private core and common
    examples may additionally include files in their own respective directories.
    Header independence and allocation behavior remain compiled/runtime checks.
    """
    errors = []
    public = root / "include"
    core = root / "src"
    common = root / "examples/common"
    for folder in (public, core, common):
        if not folder.is_dir():
            errors.append(f"missing source directory: {folder.relative_to(root)}")
            continue
        for path in sorted(folder.rglob("*")):
            if path.suffix.lower() not in SOURCE_SUFFIXES | HEADER_SUFFIXES:
                continue
            name = path.relative_to(root).as_posix()
            if path.is_symlink() or not path.is_file():
                errors.append(f"source is not a regular checkout file: {name}")
                continue
            text = path.read_text(encoding="utf-8")
            code = masked(text)
            is_common = folder == common
            if folder == public and path.suffix.lower() in SOURCE_SUFFIXES:
                errors.append(f"implementation source in public include tree: {name}")
            # Deleted copy/move operations express ownership without allocation.
            executable = re.sub(r"=\s*delete\s*;", ";", code)
            for label, pattern in FORBIDDEN.items():
                if pattern.search(executable):
                    errors.append(f"{label} in portable source: {name}")
            if not is_common and CORE_OUTPUT.search(code):
                errors.append(f"logging/output in core: {name}")
            if folder == public and re.search(r"\busing\s+namespace\b", code):
                errors.append(f"using namespace in public header: {name}")
            if path.suffix.lower() in HEADER_SUFFIXES and not (
                re.search(r"^\s*#\s*pragma\s+once\b", code, re.M)
                or re.search(r"^\s*#\s*ifndef\s+(\w+)\s*\n\s*#\s*define\s+\1\b",
                             code, re.M)
            ):
                errors.append(f"missing include guard: {name}")
            directives = masked(text, keep_strings=True)
            # Keep quoted include names but ignore include-looking lines inside
            # multiline literals. Both masks preserve character positions.
            includes = [match for match in INCLUDE.finditer(directives)
                        if re.match(r"^[ \t]*#[ \t]*include\b", code[match.start():])]
            if len(includes) != len(re.findall(
                    r"^[ \t]*#[ \t]*(?:include(?:_next)?|import)\b", code, re.M)):
                errors.append(f"include must use a literal header name: {name}")
            approved = COMMON_SYSTEM_HEADERS if is_common else CORE_SYSTEM_HEADERS
            for match in includes:
                header = match[2]
                if match[1] == "<" and header in approved:
                    continue
                search = [path.parent / header, public / header]
                allowed = (public, common) if is_common else (public, core)
                if folder == public:
                    allowed = (public,)
                candidates = [candidate.resolve() for candidate in search]
                if not any(candidate.is_file() and candidate.suffix.lower() in HEADER_SUFFIXES
                           and any(
                    candidate.is_relative_to(base) for base in allowed
                ) for candidate in candidates):
                    errors.append(f"unapproved or missing include: {name}: {header}")
    return errors


def metadata_errors(root: Path) -> list[str]:
    """Compare explicit build inputs with real core sources and package metadata.

    The maintained CMake subset uses one literal source variable for both plain
    CMake and IDF. Dynamic source expressions and globbing require review rather
    than silently enlarging the library. Package-file safety/parity reuses the
    existing package guard; version values belong to generate_version.py check.
    """
    errors = []
    manifest = json.loads((root / "library.json").read_text(encoding="utf-8"))
    if not isinstance(manifest, dict):
        return ["library.json must contain an object"]
    if manifest.get("name") != "TUSB32x" or manifest.get("license") != "MIT":
        errors.append("library metadata name/license must be TUSB32x/MIT")
    expected_authors = [
        {"name": "Thymos Solution s.r.o", "email": "info@thymos.cz",
         "url": "https://www.thymos.cz"},
        {"name": "Jan Havelka", "maintainer": True},
    ]
    if manifest.get("authors") != expected_authors:
        errors.append("library metadata authors/maintainer differ from maintained ownership")
    public_headers = sorted(path.relative_to(root / "include").as_posix()
                            for path in (root / "include").rglob("*")
                            if path.suffix.lower() in HEADER_SUFFIXES)
    if sorted(manifest.get("headers", [])) != public_headers or not public_headers:
        errors.append("library metadata headers must list every public header exactly once")
    if any(not name.startswith("TUSB32x/") for name in public_headers):
        errors.append("public headers must remain under include/TUSB32x")
    actual_sources = sorted(path.relative_to(root).as_posix()
                            for path in (root / "src").rglob("*")
                            if path.suffix.lower() in SOURCE_SUFFIXES)
    if not actual_sources:
        errors.append("core source list is empty")
    cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
    cmake = re.sub(r"#[^\n]*", "", cmake)
    source_lists = re.findall(r"\bset\s*\(\s*TUSB32X_SOURCES\s+([^)]*)\)", cmake)
    sources = source_lists[0].split() if len(source_lists) == 1 else []
    if sorted(sources) != actual_sources:
        errors.append("CMake TUSB32X_SOURCES must register every core source exactly once")
    library = re.findall(r"\badd_library\s*\(\s*TUSB32x\s+([^)]*)\)", cmake)
    component = re.findall(r"\bidf_component_register\s*\(([^)]*)\)", cmake)
    if not (len(library) == 1 and library[0].split() == ["STATIC", "${TUSB32X_SOURCES}"]
            ) or not (
        len(component) == 1 and component[0].split() == [
            "SRCS", "${TUSB32X_SOURCES}", "INCLUDE_DIRS", '"include"']
    ):
        errors.append("CMake and IDF must consume only the shared explicit core source list")
    if re.search(r"\b(?:file\s*\(\s*GLOB|target_sources\s*\(|"
                 r"list\s*\(\s*(?:APPEND|PREPEND)\s+TUSB32X_SOURCES)", cmake, re.I):
        errors.append("core source registration must not use globbing or supplementary lists")
    if len(re.findall(r"\bTUSB32X_SOURCES\b", cmake)) != 3:
        errors.append("shared core source variable must have one definition and two consumers")
    build = manifest.get("build", {})
    filters = build.get("srcFilter", "") if isinstance(build, dict) else ""
    expected_filters = [f"+<{Path(name).relative_to('src').as_posix()}>"
                        for name in actual_sources]
    if not isinstance(filters, str) or sorted(filters.split()) != expected_filters:
        errors.append("PlatformIO srcFilter must register every core source exactly once")
    if not isinstance(build, dict) or (build.get("includeDir"), build.get("srcDir")) != (
            "include", "src"):
        errors.append("PlatformIO includeDir/srcDir must be include/src")
    try:
        exported = package_files(root)
        core_files = set(actual_sources) | {f"include/{name}" for name in public_headers}
        core_files.update(path.relative_to(root).as_posix()
                          for path in (root / "src").rglob("*")
                          if path.suffix.lower() in HEADER_SUFFIXES)
        if not core_files <= set(exported):
            errors.append("package export omits core sources or headers")
    except (PackageError, IndexError, KeyError, TypeError, ValueError) as error:
        errors.append(f"package metadata contract: {error}")
    return errors


def check(root: Path) -> list[str]:
    """Return all static boundary violations; unreadable inputs raise OSError.

    Malformed JSON and invalid metadata shapes are violations, not successful
    empty scans. Root existence is checked by the CLI before this function runs.
    """
    errors = source_errors(root)
    try:
        errors.extend(metadata_errors(root))
    except (ValueError, KeyError, TypeError, AttributeError) as error:
        errors.append(f"invalid metadata: {error}")
    return errors


def main(argv: list[str] | None = None) -> int:
    """Run read-only static checks with explicit PASS/FAIL/INCOMPLETE exit codes."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT,
                        help="checkout to inspect (default: this script's repository)")
    args = parser.parse_args(argv)
    root = args.root.resolve()
    if not root.is_dir():
        print(f"INCOMPLETE: root is not a directory: {root}", file=sys.stderr)
        return 2
    try:
        errors = check(root)
    except (OSError, UnicodeError) as error:
        print(f"INCOMPLETE: cannot read contract inputs: {error}", file=sys.stderr)
        return 2
    if errors:
        print("FAIL: static source/metadata contracts", file=sys.stderr)
        print("\n".join(errors), file=sys.stderr)
        return 1
    print("PASS: static source/metadata contracts (runtime portability is checked separately)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
