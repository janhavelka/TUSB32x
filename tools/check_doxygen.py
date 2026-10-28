#!/usr/bin/env python3
"""Generate and verify local C++ documentation with real Doxygen (Python 3.10+).

Exit 0 means documentation passed, 1 means bad documentation or generation, and
2 means an unavailable executable or invalid invocation. Runs at most 120 seconds;
retains configuration, stdout, stderr, HTML and XML beneath --out (default
build/doxygen-check). TUSB32X_DOXYGEN selects the default executable; --doxygen
has precedence. Reads trusted local sources; no network, firmware or Git.
Public API visibility is checked independently of Doxygen warnings. Private
nontrivial helpers need useful adjacent comments, not full public contracts.
"""

from __future__ import annotations

import argparse
from html.parser import HTMLParser
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
from urllib.parse import unquote, urlsplit
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
TIMEOUT_SECONDS = 120
_SOURCE_SUFFIXES = {".h", ".hpp", ".cpp", ".cc", ".cxx", ".ino"}
_IGNORED_DIRS = {"build", ".pio", ".git", "managed_components", "__pycache__"}
_REQUIRED_CONFIG = {
    "EXTRACT_ALL": "NO", "EXTRACT_PRIVATE": "NO",
    "HIDE_UNDOC_MEMBERS": "NO", "HIDE_UNDOC_CLASSES": "NO",
    "WARN_IF_UNDOCUMENTED": "YES", "WARN_IF_DOC_ERROR": "YES",
    "WARN_IF_INCOMPLETE_DOC": "YES", "WARN_NO_PARAMDOC": "YES",
    "WARN_IF_UNDOC_ENUM_VAL": "YES", "WARN_AS_ERROR": "YES",
    "GENERATE_HTML": "YES", "GENERATE_XML": "YES",
}


class HtmlPage(HTMLParser):
    """Collect generated local links and anchors without fetching external URLs."""

    def __init__(self) -> None:
        """Start an empty page with decoded HTML attributes."""
        super().__init__(convert_charrefs=True)
        self.links: list[str] = []
        self.anchors: set[str] = set()

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        """Record href/src targets and both modern and legacy anchor forms."""
        values = dict(attrs)
        for name in ("href", "src"):
            if values.get(name):
                self.links.append(values[name])
        if values.get("id"):
            self.anchors.add(values["id"])
        if tag == "a" and values.get("name"):
            self.anchors.add(values["name"])


def check_html(html: Path) -> tuple[dict[Path, HtmlPage], set[str]]:
    """Return parsed pages and missing local files/anchors; do not follow URLs."""
    pages: dict[Path, HtmlPage] = {}
    errors: set[str] = set()
    for path in html.rglob("*.html"):
        page = HtmlPage()
        page.feed(path.read_text(encoding="utf-8"))
        pages[path.resolve()] = page
    if (html / "index.html").resolve() not in pages:
        errors.add("Missing HTML landing page")
    for path, page in pages.items():
        for link in page.links:
            url = urlsplit(link)
            if url.scheme or url.netloc:
                continue
            target = (path.parent / unquote(url.path)).resolve() if url.path else path
            label = f"{path.relative_to(html)}: {link}"
            if not target.is_relative_to(html.resolve()) or not target.is_file():
                errors.add(f"Missing local document or asset: {label}")
            elif (url.fragment and target in pages
                  and unquote(url.fragment) not in pages[target].anchors):
                errors.add(f"Missing local anchor: {label}")
    return pages, errors


def source_files(root: Path) -> list[Path]:
    """List maintained core/example C++ inputs, pruning local SDK/build trees."""
    files = []
    for directory in ("include", "src", "examples"):
        for parent, dirs, names in os.walk(root / directory):
            dirs[:] = sorted(name for name in dirs if name not in _IGNORED_DIRS)
            files.extend(Path(parent) / name for name in sorted(names)
                         if Path(name).suffix in _SOURCE_SUFFIXES)
    return files


def _code(text: str) -> str:
    """Blank C++ comments/string literals while preserving offsets and lines."""
    pattern = (r'//[^\n]*|/\*.*?\*/|R"([^ ()\\\t\r\n]*)\(.*?\)\1"'
               r'|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
    return re.sub(pattern, lambda match: re.sub(r"[^\n]", " ", match[0]),
                  text, flags=re.DOTALL)


def _documented(node: ET.Element) -> bool:
    """Whether a generated XML declaration has nonempty prose documentation."""
    return any("".join(node.itertext()).strip() for node in
               [*node.findall("briefdescription"), *node.findall("detaileddescription")])


def _relative_file(root: Path, value: str) -> str:
    """Normalize a Doxygen file location relative to the trusted source root."""
    path = Path(value.replace("\\", "/"))
    if path.is_absolute():
        try:
            return path.resolve().relative_to(root).as_posix()
        except ValueError:
            return path.as_posix()
    return path.as_posix()


def check_api(root: Path, output: Path, pages: dict[Path, HtmlPage],
              sources: list[Path]) -> tuple[set[str], set[tuple[str, int]]]:
    """Verify XML/API HTML visibility; return errors and documented function bodies.

    Includes public fields, typedefs, functions and enum values. XML alone can
    contain private or undocumented declarations, so HTML anchors are also checked.
    """
    errors: set[str] = set()
    documented_bodies: set[tuple[str, int]] = set()
    public_types: set[str] = set()
    files: set[str] = set()
    html, xml = output / "html", output / "xml"
    compounds = ET.parse(xml / "index.xml").getroot().findall("compound")
    for entry in compounds:
        refid = entry.attrib["refid"]
        node = ET.parse(xml / f"{refid}.xml").getroot().find("compounddef")
        if node is None:
            errors.add(f"Missing XML compound: {refid}")
            continue
        location = node.find("location")
        file = _relative_file(root, location.get("file", "")) if location is not None else ""
        kind, name = node.get("kind"), node.findtext("compoundname", "")
        if kind == "file":
            files.add(file)
            if not _documented(node):
                errors.add(f"Missing file documentation: {file}")
        page = (html / f"{refid}.html").resolve()
        public = file.startswith("include/") and node.get("prot") != "private"
        if public and kind in {"class", "struct", "union"}:
            public_types.add(name.split("::")[-1])
            if not _documented(node):
                errors.add(f"Missing public type documentation: {name}")
            if page not in pages:
                errors.add(f"Public API page missing: {name}")
        for member in node.findall("sectiondef/memberdef"):
            location = member.find("location")
            member_file = (_relative_file(root, location.get("file", ""))
                           if location is not None else "")
            member_name = member.findtext("qualifiedname") or member.findtext("name", "")
            if member.get("kind") == "function" and _documented(member) and location is not None:
                body_file = _relative_file(root, location.get("bodyfile", ""))
                body_line = int(location.get("bodystart", "0"))
                documented_bodies.add((body_file, body_line))
            if location is not None and location.get("declfile"):
                member_file = _relative_file(root, location.get("declfile", ""))
            if (not member_file.startswith("include/") or member.get("prot") != "public"
                    or node.get("prot") == "private"):
                continue
            if not _documented(member):
                errors.add(f"Missing public member documentation: {member_name}")
            anchor = member.get("id", "").removeprefix(refid + "_1")
            if page not in pages or anchor not in pages[page].anchors:
                errors.add(f"Public member missing in HTML: {member_name}")
            for value in member.findall("enumvalue"):
                if not _documented(value):
                    errors.add("Missing enum value documentation: "
                               f"{member_name}::{value.findtext('name')}")
                value_anchor = value.get("id", "").removeprefix(refid + "_1")
                if page not in pages or value_anchor not in pages[page].anchors:
                    errors.add("Enum value missing in HTML: "
                               f"{member_name}::{value.findtext('name')}")
    for source in sources:
        relative = source.relative_to(root).as_posix()
        if relative not in files:
            errors.add(f"Documentation input missing: {relative}")
        if relative.startswith("include/"):
            code = _code(source.read_text(encoding="utf-8"))
            for match in re.finditer(r"\b(?:class|struct|union)\s+(\w+)\s*(?:final\s*)?[:{]", code):
                if code[max(0, match.start() - 8):match.start()].rstrip().endswith("enum"):
                    continue
                if match[1] not in public_types and not match[1].startswith("_"):
                    # Nested private types are verified through their own XML protection.
                    private = any(entry.findtext("name", "").endswith("::" + match[1])
                                  for entry in compounds)
                    if not private:
                        errors.add(f"Public type absent from XML: {relative}: {match[1]}")
    return errors, documented_bodies


def check_sources(root: Path, sources: list[Path],
                  documented_bodies: set[tuple[str, int]]) -> set[str]:
    """Require file docs and adjacent comments for nontrivial private helpers.

    A helper is nontrivial when it branches/loops or has several statements.
    Trivial forwarding/accessor bodies need no boilerplate. This is
    a conservative source guard in addition to real Doxygen warnings and XML;
    review and behavioral tests still establish the comments' semantic accuracy.
    """
    errors: set[str] = set()
    signature = re.compile(r"^[ \t]*(?:[\w:*&<>]+[ \t]+)+(?P<name>(?:\w+::)*~?\w+)\s*"
                           r"\([^;{}]*\)(?:\s+(?:const|noexcept|override|final))*\s*\{",
                           re.MULTILINE)
    for path in sources:
        text = path.read_text(encoding="utf-8")
        relative = path.relative_to(root).as_posix()
        if not re.search(r"(?:@|\\)file\b", text) or not re.search(r"(?:@|\\)brief\s+\S", text):
            errors.add(f"Missing @file/@brief documentation: {relative}")
        if relative.startswith("include/") and re.search(
                r"(?:@|\\)(?:internal|cond|if|ifnot|private|protected|"
                r"privatesection|protectedsection)\b", text):
            errors.add(f"Public documentation hiding directive: {relative}")
        code = _code(text)
        if relative.startswith("include/"):
            # A whole-file include guard is harmless. Other public conditional APIs
            # need an explicit inventory design before this documentation gate can
            # establish their visibility under every supported configuration.
            lines = [line.strip() for line in code.splitlines() if line.strip()]
            opening = re.fullmatch(r"#\s*ifndef\s+(\w+)", lines[0]) if lines else None
            if (opening and len(lines) >= 3
                    and re.fullmatch(r"#\s*define\s+" + re.escape(opening[1]), lines[1])
                    and re.fullmatch(r"#\s*endif", lines[-1])):
                lines = lines[2:-1]
            if any(re.match(r"#\s*(?:if|ifdef|ifndef|elif|else|endif)\b", line) for line in lines):
                errors.add(f"Unsupported conditional public API documentation: {relative}")
        for match in signature.finditer(code):
            name = match["name"]
            if name in {"if", "while", "for", "switch", "constexpr", "noexcept"}:
                continue
            line = text.count("\n", 0, match.start()) + 1
            if (relative, line) in documented_bodies:
                continue
            depth, end = 1, match.end()
            while depth and end < len(code):
                depth += (code[end] == "{") - (code[end] == "}")
                end += 1
            body = code[match.end():end - 1]
            if body.count(";") <= 2 and not re.search(r"\b(if|switch|for|while)\b", body):
                continue
            prefix = text[:match.start()].rstrip()
            if (prefix.endswith("*/")
                    or (prefix and prefix.splitlines()[-1].lstrip().startswith("//"))):
                continue
            errors.add(f"Missing source helper comment: {relative}:{line}: {name}")
    return errors


def check_generated(root: Path) -> set[str]:
    """Verify generated public version documentation and stable manifest identity.

    Full generated byte equality belongs to test_version_sync, not this guard.
    The marker identifies the owner of these comments; docs are fixed at source.
    """
    path = root / "include/TUSB32x/Version.h"
    if not path.is_file():
        return {"Missing generated public header: include/TUSB32x/Version.h"}
    text = path.read_text(encoding="utf-8")
    errors: set[str] = set()
    marker = "Generated by scripts/generate_version.py from library.json. Do not edit manually."
    if marker not in text:
        errors.add("Generated Version.h documentation producer marker missing")
    version = json.loads((root / "library.json").read_text(encoding="utf-8"))["version"]
    if not re.search(r'\bVERSION\[\]\s*=\s*"' + re.escape(version) + r'"\s*;', text):
        errors.add("Generated Version.h does not identify the manifest version")
    for name in ("VERSION", "VERSION_MAJOR", "VERSION_MINOR", "VERSION_PATCH"):
        if not re.search(r"\binline\s+constexpr\s+[^;=]+\b" + name + r"\b", text):
            errors.add(f"Generated public version constant missing: {name}")
    return errors


def check_configuration(configuration: str) -> set[str]:
    """Reject warning suppression/extract-all instead of silently fixing it."""
    values = {}
    for line in configuration.splitlines():
        match = re.match(r"\s*(\w+)\s*(\+?=)\s*([^#]*)", line)
        if match:
            value = match[3].strip().strip('"')
            if match[2] == "+=":
                values[match[1]] = (values.get(match[1], "") + " " + value).strip()
            else:
                values[match[1]] = value
    errors = {f"Doxyfile requires {key} = {value}" for key, value in _REQUIRED_CONFIG.items()
              if values.get(key) != value}
    if re.search(r"^\s*@INCLUDE", configuration, re.MULTILINE):
        errors.add("Doxyfile must not override warning settings through @INCLUDE")
    for key in ("EXCLUDE_SYMBOLS", "INPUT_FILTER", "FILTER_PATTERNS",
                "FILTER_SOURCE_PATTERNS", "PREDEFINED"):
        if values.get(key):
            errors.add(f"Doxyfile must not hide declarations through {key}")
    return errors


def main(argv: list[str] | None = None) -> int:
    """Run bounded local generation, retain evidence, and return 0/1/2."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--out", type=Path, default=None)
    parser.add_argument("--doxygen", default=os.environ.get("TUSB32X_DOXYGEN", "doxygen"))
    args = parser.parse_args(argv)
    executable = shutil.which(args.doxygen)
    if executable is None:
        print(f"Doxygen required; executable unavailable: {args.doxygen}", file=sys.stderr)
        return 2
    root = args.root.resolve()
    out = (args.out or root / "build/doxygen-check").resolve()
    try:
        configuration = (root / "Doxyfile").read_text(encoding="utf-8")
        sources = source_files(root)
        errors = check_configuration(configuration) | check_generated(root)
        if not sources:
            errors.add("No C++ documentation inputs found")
        out.mkdir(parents=True, exist_ok=True)
        # Each run uses fresh output so old pages cannot conceal a hidden API.
        output = Path(tempfile.mkdtemp(prefix="run-", dir=out))
        configuration += (f'\nOUTPUT_DIRECTORY = "{output.as_posix()}"\n'
                          'HTML_OUTPUT = html\nXML_OUTPUT = xml\nWARN_LOGFILE =\n')
        (output / "Doxyfile").write_text(configuration, encoding="utf-8", newline="\n")
        result = subprocess.run([executable, "-"], input=configuration, cwd=root,
                                text=True, capture_output=True, encoding="utf-8",
                                errors="replace", timeout=TIMEOUT_SECONDS)
        (output / "stdout.log").write_text(result.stdout, encoding="utf-8")
        (output / "stderr.log").write_text(result.stderr, encoding="utf-8")
        if result.returncode or result.stderr.strip():
            errors.add(f"Doxygen failed or emitted diagnostics (exit {result.returncode})")
            print(result.stdout + result.stderr, file=sys.stderr)
        if (output / "xml/index.xml").is_file():
            pages, html_errors = check_html(output / "html")
            api_errors, documented_bodies = check_api(root, output, pages, sources)
            errors |= html_errors | api_errors | check_sources(root, sources, documented_bodies)
        else:
            errors.add("Doxygen XML index missing")
        (out / "latest.json").write_text(json.dumps({"output": str(output),
            "errors": sorted(errors)}, indent=2) + "\n", encoding="utf-8")
        if errors:
            print("\n".join(sorted(errors)), file=sys.stderr)
            return 1
        print(f"Doxygen PASS: {len(sources)} C++ files; "
              "HTML/XML, public visibility, links and helpers")
        print(f"Evidence: {output}")
        return 0
    except subprocess.TimeoutExpired as error:
        # TimeoutExpired can retain bytes even when run(text=True) was requested.
        for name, content in (("stdout.log", error.stdout), ("stderr.log", error.stderr)):
            if isinstance(content, bytes):
                content = content.decode("utf-8", errors="replace")
            (output / name).write_text(content or "", encoding="utf-8")
        message = f"Doxygen exceeded {TIMEOUT_SECONDS} seconds"
        (out / "latest.json").write_text(json.dumps({"output": str(output),
            "errors": [message]}, indent=2) + "\n", encoding="utf-8")
        print(f"{message}; evidence: {output}", file=sys.stderr)
        return 1
    except (OSError, ValueError, KeyError, ET.ParseError) as error:
        print(f"Doxygen invocation/check failed: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
