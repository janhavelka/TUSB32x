"""Run explicit local release checks once, retaining logs and partial outcomes.

This entry point never opens serial ports, flashes, publishes, or contacts CI.
CTest owns behavioral suites; this module only selects and records their execution.
--checks selects the declared groups; --configs defaults to Debug and Release.
Each invocation creates an exclusive run-* directory beneath --out (by default
build/validation), with command logs, CTest JUnit and atomic report.json checkpoints.
--timeout bounds each host child, not the whole invocation; inventory commands
have 60 seconds and the firmware-link child has 7200 seconds. Retained command
logs are capped at 16 MiB and overflow fails. Builds/package extraction write local
artifacts; installed firmware tools may fetch their declared build dependencies.
Exit 0 means the requested scope passed, 1 failed, 2 incomplete/invalid invocation,
and 130 interrupted with available evidence retained. Missing tools never pass.
"""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]
CHECKS = ("native", "contracts", "doxygen", "python", "package", "arduino", "idf-s2", "idf-s3")
CTEST_CHECKS = CHECKS[:5]
FIRMWARE_CHECKS = CHECKS[5:]
MAX_LOG_BYTES = 16 * 1024 * 1024
REQUIRED_TESTS = {
    "native": {"tusb32x_validators", "tusb32x_protocol", "tusb32x_operations",
               "tusb32x_port", "tusb32x_diagnostics", "tusb32x_cli", "tusb32x_console",
               "tusb32x_example_owner", "tusb32x_example_integration", "tusb32x_output_mailbox",
               "tusb32x_host_basic", "tusb32x_fault_fuzz", "tusb32x_arduino_wire_1000", "tusb32x_arduino_wire_100",
               "tusb32x_idf_transport_1000", "tusb32x_idf_transport_100",
               "tusb32x_arduino_app_startup", "tusb32x_arduino_app_shutdown",
               "tusb32x_arduino_app_usb_sda", "tusb32x_arduino_app_usb_irq",
               "tusb32x_idf_app_startup", "tusb32x_idf_app_shutdown-active",
               "tusb32x_idf_app_backpressure", "tusb32x_idf_app_diagnostic",
               "tusb32x_idf_app_irq-configured", "tusb32x_idf_app_pin_conflict",
               "tusb32x_idf_app_runtime"},
    "contracts": {"tusb32x_compile_contracts", "tusb32x_portable_consumer",
                  "cli_contract", "cli_contract_negative", "core_contracts",
                  "core_contracts_negative"},
    "doxygen": {"doxygen", "doxygen_negative"},
    "python": {"hil_runner", "cli_transcript", "tusb32x_version_sync", "validate_runner",
               "firmware_guard", "tusb32x_idf_example_config", "tusb32x_arduino_examples_config",
               "tusb32x_boundary_fuzz"},
    "package": {"tusb32x_package_safety", "tusb32x_package_consumer",
                "tusb32x_package_origins"},
}
if os.name == "nt":
    REQUIRED_TESTS["python"].add("tusb32x_pio_launcher")
REQUIRED_TESTS["native"].update("tusb32x_arduino_app_" + scenario for scenario in (
    "startup", "startup-failure", "begin-failure", "preparation-failure", "cleanup-failure",
    "end-failure", "existing-bus", "shutdown", "usb_sda", "usb_irq",
    "rx-overflow", "rx-overflow-after-read"))
REQUIRED_TESTS["native"].update("tusb32x_idf_app_" + scenario for scenario in (
    "startup", "startup-failure", "shutdown-active", "shutdown-startup", "backpressure",
    "bus-failure", "device-failure", "uart-config-failure", "uart-pin-failure",
    "uart-install-failure", "console-already-owned", "device-remove-failure",
    "bus-delete-failure", "uart-delete-failure", "rx-overflow", "raw-error",
    "rx-overflow-after-read", "rx-event-flood", "device-cleanup-failure",
    "teardown-fifo-pending", "teardown-fifo-stalled", "existing-bus", "irq-configured",
    "irq-config-failure", "irq-cleanup-failure", "diagnostic", "pin_conflict", "runtime"))


@dataclass
class Result:
    """One command's verdict, diagnostics and retained output; no inferred evidence."""

    name: str
    status: str
    detail: str
    command: list[str] | None = None
    log: str | None = None
    returncode: int | None = None
    seconds: float = 0.0


def selections(value):
    """Return unique ordered names; reject empty, unknown or mixed-all selections."""
    names = value.split(",")
    if names == ["all"]:
        return CHECKS
    if not names or any(name not in CHECKS for name in names):
        raise argparse.ArgumentTypeError(
            "checks must be all or comma-separated: " + ",".join(CHECKS))
    return tuple(dict.fromkeys(names))


def positive_seconds(value):
    """Accept a finite subprocess bound of 1..7200 seconds."""
    try:
        number = int(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError("timeout must be an integer") from error
    if not 1 <= number <= 7200:
        raise argparse.ArgumentTypeError("timeout must be 1..7200 seconds")
    return number


def stop_process(process):
    """Terminate a timed-out/interrupted child tree without touching other processes."""
    if process.poll() is not None:
        return
    if os.name == "nt":
        try:
            subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                           timeout=10, check=False)
        except (OSError, subprocess.SubprocessError):
            pass
        if process.poll() is None:
            process.kill()
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    process.wait(timeout=10)


def run_command(name, command, directory, timeout, cwd=ROOT, environment=None,
                incomplete_codes=()):
    """Execute argument-list child once with a deadline and 16 MiB retained log.

    Missing executables are INCOMPLETE. Timeouts and unsuccessful children fail.
    Output overflow stops the child, retains the bounded prefix and fails even
    after exit zero. Polling may transiently exceed the retained byte limit.
    KeyboardInterrupt is propagated after terminating the child; the owner saves it.
    Completion observed at or after the deadline fails. Process creation cannot
    be interrupted portably; terminating a child uses separate finite cleanup waits.
    """
    command = [str(item) for item in command]
    log = directory / (name + ".log")
    started = time.monotonic()
    try:
        with log.open("wb") as output:
            options = ({"creationflags": subprocess.CREATE_NEW_PROCESS_GROUP}
                       if os.name == "nt" else {"start_new_session": True})
            process = subprocess.Popen(command, cwd=cwd, env=environment, stdout=output,
                                       stderr=subprocess.STDOUT, **options)
            overflow = False
            try:
                while True:
                    if os.fstat(output.fileno()).st_size > MAX_LOG_BYTES:
                        stop_process(process)
                        code = process.returncode
                        break
                    remaining = timeout - (time.monotonic() - started)
                    if remaining <= 0:
                        raise subprocess.TimeoutExpired(command, timeout)
                    try:
                        code = process.wait(timeout=min(0.25, remaining))
                    except subprocess.TimeoutExpired:
                        continue
                    if time.monotonic() - started >= timeout:
                        raise subprocess.TimeoutExpired(command, timeout)
                    break
            except BaseException:
                stop_process(process)
                raise
            finally:
                # Check again after exit: a fast successful child may overflow
                # between polls. Preserve only the diagnostic prefix on disk.
                if os.fstat(output.fileno()).st_size > MAX_LOG_BYTES:
                    overflow = True
                    output.truncate(MAX_LOG_BYTES)
        status = "FAIL" if overflow else (
            "PASS" if code == 0 else ("INCOMPLETE" if code in incomplete_codes else "FAIL"))
        detail = (f"output exceeded {MAX_LOG_BYTES} bytes; retained prefix" if overflow
                  else "completed" if code == 0 else f"child exit {code}")
        result = Result(name, status, detail, command, str(log), code)
    except subprocess.TimeoutExpired:
        result = Result(name, "FAIL", f"timeout after {timeout}s", command, str(log))
    except OSError as error:
        result = Result(name, "INCOMPLETE", str(error), command, str(log))
    result.seconds = round(time.monotonic() - started, 3)
    return result


def save_report(directory, requested, configurations, results, interrupted=False):
    """Replace directory/report.json and return its scope/verdict/results mapping.

    directory already exists; requested/configurations describe admitted work,
    results contains completed Result values, and interrupted leaves work
    incomplete. A fixed report.json.tmp is replaced atomically, so serialize
    callers. File errors propagate; this helper never executes pending checks.
    """
    completed = {result.name for result in results}
    pending = [f"{configuration.lower()}-{group}" for group in requested if group in CTEST_CHECKS
               for configuration in configurations
               if f"{configuration.lower()}-{group}" not in completed and group not in completed]
    if any(group in requested for group in FIRMWARE_CHECKS) and "firmware" not in completed:
        pending.append("firmware")
    status = "FAIL" if any(r.status == "FAIL" for r in results) else (
        "INCOMPLETE" if interrupted or pending or any(r.status == "INCOMPLETE" for r in results)
        else "PASS")
    full = set(requested) == set(CHECKS) and set(configurations) == {"Debug", "Release"}
    payload = {"scope": "all local release checks" if full else "focused checks",
               "requested": list(requested), "configurations": list(configurations),
               "status": status, "interrupted": interrupted,
               "pending": pending,
               "results": [asdict(result) for result in results],
               "hardware": "NOT RUN", "remote_ci": "NOT RUN"}
    temporary = directory / "report.json.tmp"
    temporary.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    temporary.replace(directory / "report.json")
    return payload


def registration(data):
    """Validate CTest's actual inventory, unique ownership and finite test bounds."""
    groups = {name: [] for name in CTEST_CHECKS}
    tests = data.get("tests", [])
    seen = set()
    if not tests:
        raise ValueError("CTest discovered zero tests")
    for test in tests:
        name = test["name"]
        properties = {item["name"]: item["value"] for item in test.get("properties", [])}
        owners = [label.removeprefix("validation_") for label in properties.get("LABELS", [])
                  if label.startswith("validation_")]
        if name in seen or len(owners) != 1 or owners[0] not in groups:
            raise ValueError(f"duplicate or unowned CTest registration: {name}")
        timeout = properties.get("TIMEOUT", 0)
        if not isinstance(timeout, (int, float)) or not 0 < timeout <= 7200:
            raise ValueError(f"missing finite CTest timeout: {name}")
        if properties.get("DISABLED", False):
            raise ValueError(f"disabled required CTest: {name}")
        seen.add(name)
        groups[owners[0]].append(name)
    for group, required in REQUIRED_TESTS.items():
        absent = required - set(groups[group])
        if absent:
            raise ValueError(f"missing {group} tests: {sorted(absent)}")
    return groups


def verify_execution(path, expected):
    """Reject zero, skipped, duplicated or missing executions in actual CTest JUnit."""
    root = ET.parse(path).getroot()
    cases = list(root.iter("testcase"))
    names = [case.attrib["name"] for case in cases]
    if len(names) != len(set(names)) or set(names) != set(expected):
        raise ValueError("CTest executed a different set of tests than requested")
    if any(case.find("skipped") is not None or case.find("failure") is not None
           or case.find("error") is not None or case.get("status", "run") != "run"
           for case in cases):
        raise ValueError("CTest contains a skipped or failed required test")


def parser():
    """Describe local scope and explicit tool/configuration overrides, never HIL."""
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--checks", type=selections, default=CHECKS,
                        help="all (default), or comma-separated: " + ",".join(CHECKS))
    result.add_argument("--configs", nargs="+", choices=("Debug", "Release"),
                        default=["Debug", "Release"], help="host configurations (default: both)")
    result.add_argument("--out", type=Path, default=ROOT / "build/validation")
    result.add_argument("--cmake", default="cmake")
    result.add_argument("--ctest", default="ctest")
    result.add_argument("--compiler", default=shutil.which("g++") or shutil.which("cl") or "c++")
    result.add_argument("--make-program", default="ninja")
    result.add_argument("--generator", default="Ninja")
    result.add_argument("--doxygen", default="doxygen")
    result.add_argument("--sanitize", action="store_true",
                        help="Linux host ASan/UBSan; never applied to firmware builds")
    result.add_argument("--timeout", type=positive_seconds, default=900,
                        help="host command bound in seconds; CTest has per-test bounds too")
    result.add_argument("--core-dir", type=Path,
                        default=Path(os.environ.get("PLATFORMIO_CORE_DIR",
                                                    str(Path.home() / ".platformio"))))
    result.add_argument("--idf-path", type=Path, default=None)
    result.add_argument("--idf-python", type=Path, default=None)
    result.add_argument("--accept-missing-idf-constraints", action="store_true",
                        help="explicitly accept managed SDK's missing standalone constraint cache")
    return result


def run_ctests(args, directory, record):
    """Build once per configuration and execute disjoint, verified CTest groups."""
    requested = [name for name in args.checks if name in CTEST_CHECKS]
    if not requested:
        return
    missing = [tool for tool in (args.cmake, args.ctest, args.compiler, args.make_program)
               if shutil.which(tool) is None]
    if missing:
        for group in requested:
            record(Result(group, "INCOMPLETE", "missing tools: " + ", ".join(missing)))
        return
    # Clean consumers deliberately remove maintainer tools from PATH. Record
    # absolute tool paths in CMake so their children still find the selected tool.
    for key in ("cmake", "ctest", "compiler", "make_program"):
        # Preserve launcher names: clang++ and venv Python symlinks carry semantics.
        setattr(args, key, os.path.abspath(shutil.which(getattr(args, key))))
    if "doxygen" in requested and shutil.which(args.doxygen) is None:
        record(Result("doxygen", "INCOMPLETE", f"missing tool: {args.doxygen}"))
        requested.remove("doxygen")
    elif "doxygen" in requested:
        args.doxygen = os.path.abspath(shutil.which(args.doxygen))
    if not requested:
        return
    environment = os.environ.copy()
    environment["TUSB32X_DOXYGEN"] = args.doxygen
    for configuration in dict.fromkeys(args.configs):
        build = directory / configuration
        prefix = configuration.lower()
        sanitizer = (["-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer",
                      "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined"]
                     if args.sanitize else [])
        configure = record(run_command(prefix + "-configure", [args.cmake, "-S", ROOT, "-B", build,
            "-G", args.generator, "-DTUSB32X_BUILD_TESTS=ON", f"-DCMAKE_BUILD_TYPE={configuration}",
            f"-DCMAKE_CXX_COMPILER={args.compiler}", f"-DCMAKE_MAKE_PROGRAM={args.make_program}",
            f"-DPython3_EXECUTABLE={sys.executable}", *sanitizer],
            directory, args.timeout, environment=environment))
        if configure.status != "PASS":
            for group in requested:
                record(Result(f"{prefix}-{group}", "INCOMPLETE", "configuration did not complete"))
            continue
        built = record(run_command(prefix + "-build", [args.cmake, "--build", build,
            "--config", configuration, "--parallel", "4"], directory, args.timeout,
            environment=environment))
        if built.status != "PASS":
            for group in requested:
                record(Result(f"{prefix}-{group}", "INCOMPLETE", "build did not complete"))
            continue
        inventory = record(run_command(prefix + "-inventory", [args.ctest, "--test-dir", build,
            "-C", configuration, "--show-only=json-v1"], directory, 60, environment=environment))
        if inventory.status != "PASS":
            continue
        try:
            groups = registration(json.loads(Path(inventory.log).read_text(encoding="utf-8")))
        except (ValueError, KeyError, OSError) as error:
            record(Result(prefix + "-registration", "FAIL", str(error)))
            continue
        for group in requested:
            names = groups[group]
            junit = directory / f"{prefix}-{group}.xml"
            command = [args.ctest, "--test-dir", build, "-C", configuration, "--verbose",
                       "--no-tests=error", "--output-junit", junit, "-R",
                       "^(" + "|".join(re.escape(name) for name in names) + ")$"]
            checked = run_command(f"{prefix}-{group}", command, directory, args.timeout,
                                  environment=environment)
            if checked.status == "PASS":
                try:
                    verify_execution(junit, names)
                    checked.detail = f"{len(names)} tests executed once"
                except (ValueError, OSError, ET.ParseError, KeyError) as error:
                    checked.status, checked.detail = "FAIL", str(error)
            record(checked)


def run_firmware(args, directory, record):
    """Pack once and reuse the hardened clean consumer for all selected image links."""
    selected = [name for name in args.checks if name in FIRMWARE_CHECKS]
    if not selected:
        return
    if os.name != "nt":
        # The established launcher intentionally uses the existing Windows managed Core.
        record(Result("firmware", "INCOMPLETE", "Windows managed scripts/pio.cmd is required"))
        return
    managed = Path.home() / ".platformio/penv/Scripts/pio.exe"
    if not managed.is_file():
        record(Result("firmware", "INCOMPLETE", f"missing managed PlatformIO: {managed}"))
        return
    archive = directory / "TUSB32x.tar.gz"
    environment = os.environ.copy()
    environment["PLATFORMIO_CORE_DIR"] = str(args.core_dir.resolve())
    packed = record(run_command("firmware-pack", ["cmd.exe", "/d", "/c", ROOT / "scripts/pio.cmd",
        "pkg", "pack", ".", "--output", archive], directory, args.timeout, environment=environment))
    if packed.status != "PASS":
        return
    package = record(run_command("firmware-package-consumer", [sys.executable,
        ROOT / "tools/check_package.py", "--archive", archive, "--out", directory / "distribution",
        "--cmake", args.cmake, "--compiler", args.compiler, "--make-program", args.make_program,
        "--generator", args.generator, "--config", "Release"], directory, args.timeout,
        incomplete_codes=(2,)))
    if package.status != "PASS":
        return
    try:
        # The checker emits human build progress before its final JSON object.
        output = Path(package.log).read_text(encoding="utf-8")
        report = json.loads(output[output.rfind('\n{') + 1:])
        extracted = Path(report["extracted"]).resolve(strict=True)
        command = [sys.executable, ROOT / "tools/check_firmware.py", "--package", extracted,
                   "--out", directory / "firmware", "--checks", ",".join(selected),
                   "--core-dir", args.core_dir.resolve()]
        for key in ("idf_path", "idf_python"):
            value = getattr(args, key)
            if value:
                command.extend(["--" + key.replace("_", "-"), value.resolve()])
        if args.accept_missing_idf_constraints:
            command.append("--accept-missing-constraints")
        record(run_command("firmware", command, directory, 7200,
                           environment=environment, incomplete_codes=(2,)))
    except (OSError, ValueError, KeyError) as error:
        record(Result("firmware", "FAIL", f"invalid package evidence: {error}"))


def main(argv=None):
    """Return 0 requested PASS, 1 failure, 2 incomplete/invocation, 130 saved Ctrl-C."""
    args = parser().parse_args(argv)
    args.configs = tuple(dict.fromkeys(args.configs))
    try:
        args.out.mkdir(parents=True, exist_ok=True)
        directory = Path(tempfile.mkdtemp(prefix="run-", dir=args.out.resolve()))
        save_report(directory, args.checks, args.configs, [])
    except OSError as error:
        print(f"INCOMPLETE: cannot create evidence directory: {error}", file=sys.stderr)
        return 2
    results = []

    def record(result):
        """Save each completed child immediately, including failed/missing evidence."""
        results.append(result)
        save_report(directory, args.checks, args.configs, results)
        print(f"{result.status}: {result.name}: {result.detail}", flush=True)
        return result

    scope = "all local release checks" if (set(args.checks) == set(CHECKS)
            and set(args.configs) == {"Debug", "Release"}) else ",".join(args.checks)
    print(f"Scope: {scope}")
    print(f"Evidence: {directory}", flush=True)
    try:
        run_ctests(args, directory, record)
        run_firmware(args, directory, record)
        if not results:
            record(Result("validation", "INCOMPLETE", "no checks executed"))
        report = save_report(directory, args.checks, args.configs, results)
    except KeyboardInterrupt:
        results.append(Result("interruption", "INCOMPLETE",
                              "interrupted; remaining checks NOT RUN"))
        try:
            save_report(directory, args.checks, args.configs, results, interrupted=True)
        except OSError as error:
            print(f"Interrupted; cannot save updated report: {error}; "
                  f"prior checkpoints and command logs: {directory}", file=sys.stderr)
        return 130
    except (OSError, ValueError) as error:
        results.append(Result("validation", "INCOMPLETE", str(error)))
        print(f"INCOMPLETE: {error}", file=sys.stderr)
        try:
            save_report(directory, args.checks, args.configs, results)
        except OSError as save_error:
            print(f"Cannot save report: {save_error}; command logs: {directory}", file=sys.stderr)
        return 1 if any(result.status == "FAIL" for result in results) else 2
    print(f"{report['status']}: requested scope; hardware/remote CI NOT RUN")
    if any(result.status == "FAIL" for result in results):
        return 1
    return 2 if report["status"] != "PASS" else 0


if __name__ == "__main__":
    raise SystemExit(main())
