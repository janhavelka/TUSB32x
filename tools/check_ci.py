#!/usr/bin/env python3
"""Audit the declared CI workflow and optionally its actual CTest inventory.

The workflow parser needs PyYAML 6.0.3; the pure audit helpers use only Python
3.10+. No command, SDK build, network request or hardware action is executed.
Exit zero means the declared structure passes, one means a failed audit, and
two means an unavailable parser or invalid invocation. This is source evidence,
not proof that any remote job or firmware image has passed.
"""

from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import re
import shlex
import sys

import validate


ROOT = Path(__file__).resolve().parents[1]
JOBS = {"check", "host", "python", "docs", "package", "arduino", "idf"}
HOST_ROWS = {
    ("gcc-debug", "g++-14", "Debug", False),
    ("gcc-release", "g++-14", "Release", False),
    ("clang-debug", "clang++-18", "Debug", False),
    ("clang-release", "clang++-18", "Release", False),
    ("clang-sanitized", "clang++-18", "Debug", True),
}
IDF_ROWS = {(version, target) for version in ("5.5.5", "6.1")
            for target in ("esp32s2", "esp32s3")}
# Official image digests audited with the SDK releases on 2026-10-28.
IDF_IMAGES = {
    "5.5.5": "espressif/idf:v5.5.5@sha256:"
             "a9231d0697ab8f7517cc072e93b7c83e04907bfbfba80b6440d7dbbf90665cf2",
    "6.1": "espressif/idf:v6.1@sha256:"
           "81893c71bb5e570088901f21def8684c25cd2a9020281bd01b843a7655edb18c",
}
SANITIZE_ARGUMENT = "${{ matrix.sanitize && '--sanitize' || '' }}"
SHA = re.compile(r"[0-9a-f]{40}")
ACTION_LINE = re.compile(r"^\s*-?\s*uses:\s*([^\s#]+)(?:[ \t]+#[ \t]*(.*))?$", re.MULTILINE)
# Official release/tag dereferences audited on 2026-10-28; changing a pin also
# updates this declared policy after checking the official action repository.
ACTION_PINS = {
    "actions/checkout": ("3d3c42e5aac5ba805825da76410c181273ba90b1", "v7.0.1"),
    "actions/setup-python": ("6d4fd22b20465ccc0f7354476588e8df7c9ca0e0", "v7.1.0"),
    "actions/upload-artifact": ("cf430e030ddbb5b0abf93d22962f4752f3646cd9", "v7.0.2"),
    "actions/download-artifact": ("9000827ccba6bdab643e8b6fd33ac0654aef8333", "v8.0.2"),
}


class AuditError(ValueError):
    """A checked-in workflow or actual test inventory violates the CI contract."""


def require(condition, message):
    """Stop at the first concrete audit failure, retaining a readable cause."""
    if not condition:
        raise AuditError(message)


def integer(value, minimum, maximum):
    """Accept a finite bounded YAML integer, including the parser's string form."""
    return not isinstance(value, bool) and re.fullmatch(r"[0-9]+", str(value)) is not None \
        and minimum <= int(value) <= maximum


def boolean(value):
    """Normalize YAML true/false without treating an arbitrary string as true."""
    if value is True or value == "true":
        return True
    if value is False or value == "false":
        return False
    raise AuditError("matrix sanitize must be true or false")


def parse_workflow(source):
    """Parse real YAML with duplicate keys rejected and the GitHub `on` key intact."""
    try:
        import yaml
    except ImportError as error:
        raise RuntimeError("PyYAML 6.0.3 is required for workflow parsing") from error
    if yaml.__version__ != "6.0.3":
        raise RuntimeError("workflow parsing requires pinned PyYAML 6.0.3")

    class WorkflowLoader(yaml.BaseLoader):
        """Keep scalar spellings; YAML 1.1 boolean rules would rename `on` to True."""

    def mapping(loader, node):
        """Refuse duplicate job, step, permission or matrix fields before auditing."""
        result = {}
        for key_node, value_node in node.value:
            key = loader.construct_object(key_node, deep=True)
            if key in result:
                raise AuditError(f"duplicate YAML key: {key}")
            result[key] = loader.construct_object(value_node, deep=True)
        return result

    WorkflowLoader.add_constructor("tag:yaml.org,2002:map", mapping)
    try:
        return yaml.load(source, Loader=WorkflowLoader)
    except yaml.YAMLError as error:
        raise AuditError(f"invalid workflow YAML: {error}") from error


def commands(job):
    """Return the finite literal run steps in one job for command ownership checks."""
    return [step["run"] for step in job["steps"] if "run" in step]


def validation_commands(job, host=False):
    """Read canonical validate.py arguments without executing or expanding shell code."""
    found = []
    for command in commands(job):
        for line in command.replace("\\\n", " ").splitlines():
            if "tools/validate.py" not in line:
                continue
            words = shlex.split(line)
            require(words.count("tools/validate.py") == 1,
                    "validation must invoke the existing literal tools/validate.py")
            require("--checks" in words and "--configs" in words,
                    "CI validation requires explicit checks and configuration")
            if host:
                require(words.count("--compiler") == 1 and
                        words.index("--compiler") + 1 < len(words) and
                        words[words.index("--compiler") + 1] == "${{ matrix.compiler }}",
                        "host validation must use the matrix compiler")
                require(line.count(SANITIZE_ARGUMENT) == 1 and "--sanitize" not in
                        shlex.split(line.replace(SANITIZE_ARGUMENT, "")),
                        "host validation must use the matrix sanitizer selection")
            try:
                checks = words[words.index("--checks") + 1]
                configuration = words[words.index("--configs") + 1]
            except IndexError as error:
                raise AuditError("validation argument has no value") from error
            found.append((checks, configuration))
    return found


def audit_registration(data):
    """Reuse the validator's ownership checks and require the complete declared count."""
    groups = validate.registration(data)
    expected = {"native": 53, "contracts": 6, "doxygen": 2,
                "python": 8 if sys.platform == "win32" else 7, "package": 3}
    actual = {name: len(names) for name, names in groups.items()}
    require(actual == expected, f"unexpected CTest registration counts: {actual}; expected {expected}")
    return actual


def audit_workflow(document, source):
    """Check triggers, immutable actions, bounded artifacts and the exact supported rows."""
    require(isinstance(document, dict), "workflow must be a YAML mapping")
    triggers = document.get("on")
    require(isinstance(triggers, (dict, list)) and set(triggers) ==
            {"push", "pull_request", "workflow_dispatch"}, "workflow triggers changed")
    if isinstance(triggers, dict):
        require(all(value in (None, "", {}) for value in triggers.values()),
                "CI triggers must cover every push and pull request")
    require(document.get("permissions") == {"contents": "read"},
            "workflow requires only contents: read")
    jobs = document.get("jobs")
    require(isinstance(jobs, dict) and set(jobs) == JOBS, "CI job inventory changed")
    source_actions = ACTION_LINE.findall(source)
    for action, comment in source_actions:
        repository, separator, pin = action.partition("@")
        require(separator and SHA.fullmatch(pin) is not None,
                f"action requires a full immutable SHA: {action}")
        require(repository in ACTION_PINS and pin == ACTION_PINS[repository][0],
                f"action differs from the audited official release: {action}")
        require(comment.split()[:1] == [ACTION_PINS[repository][1]],
                f"action requires the audited release-name comment: {action}")
    parsed_actions = []
    for name, job in jobs.items():
        require(isinstance(job, dict), f"invalid job: {name}")
        require(integer(job.get("timeout-minutes"), 1, 90), f"missing finite job timeout: {name}")
        require("if" not in job and job.get("continue-on-error", False) in (False, "false"),
                f"required job may not be skipped or allowed to fail: {name}")
        require("permissions" not in job or job["permissions"] == {"contents": "read"},
                f"job expands workflow permissions: {name}")
        require(job.get("runs-on") == "ubuntu-24.04", f"runner pin changed: {name}")
        steps = job.get("steps")
        require(isinstance(steps, list) and steps, f"job has no steps: {name}")
        uses = 0
        uploads = 0
        retained_failures = 0
        for step in steps:
            require(isinstance(step, dict), f"invalid step: {name}")
            require(step.get("continue-on-error", False) in (False, "false"),
                    f"step may not suppress failure: {name}")
            is_upload = isinstance(step.get("uses"), str) and \
                step["uses"].startswith("actions/upload-artifact@")
            require(is_upload or "if" not in step,
                    f"required run/action step may not be conditional: {name}")
            if "uses" not in step:
                continue
            uses += 1
            action = step["uses"]
            parsed_actions.append(action)
            require(isinstance(action, str) and "@" in action and
                    SHA.fullmatch(action.rsplit("@", 1)[1]) is not None,
                    f"action requires a full immutable SHA: {action}")
            if action.startswith("actions/upload-artifact@"):
                uploads += 1
                options = step.get("with", {})
                package_payload = name == "package" and options.get("name") == "tusb32x-package"
                require(step.get("if") in ("always()", "${{ always() }}") or package_payload and
                        step.get("if") in (None, "success()", "${{ success() }}"),
                        f"artifact upload condition may not omit required evidence: {name}")
                if step.get("if") in ("always()", "${{ always() }}"):
                    retained_failures += 1
                require(integer(options.get("retention-days"), 1, 14),
                        f"artifact retention must be bounded to 14 days: {name}")
                require(options.get("if-no-files-found") == "error" and
                        isinstance(options.get("path"), str) and options["path"].strip(),
                        f"artifact paths must be explicit and required: {name}")
        require(uses > 0, f"job has no pinned actions: {name}")
        require(uploads > 0, f"job must preserve bounded evidence artifacts: {name}")
        require(retained_failures > 0, f"artifacts must survive failed jobs: {name}")
    require(Counter(parsed_actions) == Counter(action for action, _ in source_actions),
            "each action use requires its own audited pin and release-name comment")
    host = jobs["host"]
    require(isinstance(host.get("strategy"), dict), "missing host matrix strategy")
    host_matrix = host.get("strategy", {}).get("matrix", {})
    require(isinstance(host_matrix, dict) and set(host_matrix) == {"include"},
            "host requires an explicit five-row include matrix")
    rows = host_matrix["include"]
    require(isinstance(rows, list) and len(rows) == len(HOST_ROWS), "host matrix row count changed")
    require(all(isinstance(row, dict) for row in rows), "invalid host matrix row")
    require({(row.get("name"), row.get("compiler"), row.get("config"),
              boolean(row.get("sanitize"))) for row in rows} == HOST_ROWS,
            "host compiler/configuration/sanitizer matrix changed")
    require(host.get("strategy", {}).get("fail-fast") in (False, "false"),
            "host matrix must retain independent failed-row evidence")
    idf = jobs["idf"]
    require(isinstance(idf.get("strategy"), dict), "missing IDF matrix strategy")
    idf_matrix = idf.get("strategy", {}).get("matrix", {})
    require(isinstance(idf_matrix, dict) and set(idf_matrix) == {"include"},
            "IDF requires an explicit four-row include matrix")
    rows = idf_matrix["include"]
    require(isinstance(rows, list) and len(rows) == len(IDF_ROWS) and
            all(isinstance(row, dict) for row in rows) and
            {(row.get("version"), row.get("target")) for row in rows} == IDF_ROWS,
            "IDF SDK/target matrix changed")
    require(all(row.get("image") == IDF_IMAGES[row["version"]] for row in rows),
            "IDF rows must select the audited immutable SDK images")
    require(isinstance(idf.get("container"), dict) and
            idf["container"].get("image") == "${{ matrix.image }}",
            "IDF jobs must execute in the matrix SDK image")
    require(idf.get("strategy", {}).get("fail-fast") in (False, "false"),
            "IDF matrix must retain independent failed-row evidence")
    for name in ("check", "python", "docs", "package", "arduino"):
        require("strategy" not in jobs[name], f"job must execute only once: {name}")
    expected_commands = {"host": [("native,contracts", "${{ matrix.config }}")],
                         "python": [("python", "Release")],
                         "docs": [("doxygen", "Release")],
                         "package": [("package", "Release")],
                         "check": [], "arduino": [], "idf": []}
    for name, expected in expected_commands.items():
        require(validation_commands(jobs[name], host=name == "host") == expected,
                f"CTest group must have exactly one declared CI owner: {name}")
    for name in ("arduino", "idf"):
        needs = jobs[name].get("needs", [])
        require(needs == "package" or isinstance(needs, list) and "package" in needs,
                f"firmware job must consume the identified package: {name}")
        require(any(step.get("uses", "").startswith("actions/download-artifact@")
                    for step in jobs[name]["steps"]), f"missing package download: {name}")
    return {"jobs": len(jobs), "host_rows": len(HOST_ROWS), "idf_rows": len(IDF_ROWS)}


def main(argv=None):
    """Audit checked-in YAML and optional saved CTest JSON without executing either."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--workflow", type=Path, default=ROOT / ".github/workflows/ci.yml")
    parser.add_argument("--ctest-json", type=Path)
    args = parser.parse_args(argv)
    try:
        source = args.workflow.read_text(encoding="utf-8")
        result = audit_workflow(parse_workflow(source), source)
        if args.ctest_json:
            result["ctest"] = audit_registration(json.loads(args.ctest_json.read_text(encoding="utf-8")))
    except RuntimeError as error:
        print(f"INCOMPLETE: {error}", file=sys.stderr)
        return 2
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print("PASS: declared CI structure " + json.dumps(result, sort_keys=True) +
          "; remote execution and hardware NOT RUN")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
