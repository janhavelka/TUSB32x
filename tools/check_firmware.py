#!/usr/bin/env python3
"""Link and inspect the declared Arduino and native IDF5.5.5 package consumers.

Use an already validated, renamed extraction with --package and a disjoint --out.
--checks accepts arduino,idf-s2,idf-s3; Arduino includes all four shipped examples.
Existing tools are required; this helper installs nothing and never opens hardware.
Each build has an 1800-second limit and a 16 MiB saved log; inspection children
have 60 seconds. Exit 0 means requested links and evidence passed, 1 means failure,
2 means invocation/missing tools/incomplete, and 130 preserves interrupted logs.
The saved firmware-result.json identifies the requested scope and actual artifacts.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import tempfile
import time

from check_package import PackageError, make_dependencies


ROOT = Path(__file__).resolve().parents[1]
CHECKS = ("arduino", "idf-s2", "idf-s3")
ARDUINO_ROWS = ("esp32s2dev", "esp32s3dev", "esp32s2_owner_service", "esp32s3_owner_service")
CORE_SOURCES = (
    "src/TUSB32x.cpp", "src/TUSB32xProtocol.cpp", "src/TUSB32xCodecs.cpp",
    "src/TUSB32xOperations.cpp", "src/TUSB32xDiagnostics.cpp",
    "examples/common/Tusb32xCli.cpp", "examples/common/Tusb32xCliOutput.cpp",
    "examples/common/ExampleOwner.cpp",
)
MAX_LOG_BYTES = 16 * 1024 * 1024


class FirmwareError(RuntimeError):
    """A requested build or its required evidence failed verification."""


class Incomplete(FirmwareError):
    """A required installed tool or supported SDK is unavailable."""


def selections(value):
    """Return unique supported check names, rejecting empty and unknown labels."""
    names = value.split(",")
    if not names or any(name not in CHECKS for name in names):
        raise argparse.ArgumentTypeError("checks must select arduino,idf-s2,idf-s3")
    return tuple(dict.fromkeys(names))


def snapshot(package):
    """Hash extracted files so builds cannot silently modify their source input."""
    return {path.relative_to(package).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in package.rglob("*") if path.is_file()}


def require_file(path):
    """Return an absolute existing tool path or report incomplete prerequisites."""
    path = Path(path).resolve()
    if not path.is_file():
        raise Incomplete(f"required file is missing: {path}")
    return path


def validate_paths(package, output):
    """Reject checkout input, relative paths and overlapping build/source trees."""
    if not package.is_absolute() or not output.is_absolute():
        raise Incomplete("--package and --out must be absolute")
    package, output = package.resolve(), output.resolve()
    if package == ROOT or package.is_relative_to(output) or output.is_relative_to(package):
        raise Incomplete("package must be a renamed extraction disjoint from --out")
    for name in ("library.json", "platformio.ini", "scripts/pio.cmd",
                 "examples/esp_idf/basic/CMakeLists.txt"):
        require_file(package / name)
    if (package / ".git").exists():
        raise Incomplete("package must be an extraction without its own Git metadata")
    return package, output


def stop_process(process):
    """Stop a child process tree after timeout, output overflow or interruption."""
    if process.poll() is not None:
        return
    if os.name == "nt":
        taskkill = Path(os.environ.get("SystemRoot", "C:/Windows")) / "System32/taskkill.exe"
        try:
            subprocess.run([str(taskkill), "/PID", str(process.pid), "/T", "/F"],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                           check=False, timeout=20)
        finally:
            if process.poll() is None:
                process.kill()
    else:
        os.killpg(process.pid, signal.SIGKILL)
    process.wait(timeout=20)


def run_logged(command, cwd, environment, log, timeout=60, input_text=None):
    """Execute one argument list with saved bounded output, including failed children."""
    command = [str(item) for item in command]
    log.parent.mkdir(parents=True, exist_ok=True)
    evidence = {"command": command, "cwd": str(cwd), "timeout_seconds": timeout,
                "environment": {name: value for name, value in environment.items()
                                if name.startswith(("IDF_", "PLATFORMIO_", "ESP_ROM_", "CCACHE_"))}}
    log.with_suffix(".command.json").write_text(
        json.dumps(evidence, indent=2) + "\n", encoding="utf-8")
    started = time.monotonic()
    with log.open("wb") as stream:
        try:
            process = subprocess.Popen(
                command, cwd=cwd, env=environment, stdout=stream, stderr=subprocess.STDOUT,
                stdin=subprocess.PIPE if input_text is not None else subprocess.DEVNULL,
                start_new_session=os.name != "nt")
        except OSError as error:
            raise Incomplete(f"cannot execute {command[0]}: {error}") from error
        try:
            if input_text is not None:
                process.stdin.write(input_text.encode("utf-8"))
                process.stdin.close()
            while process.poll() is None:
                if time.monotonic() - started >= timeout:
                    raise FirmwareError(f"child timed out after {timeout}s; log: {log}")
                if log.stat().st_size > MAX_LOG_BYTES:
                    raise FirmwareError(f"child output exceeded 16 MiB; log: {log}")
                try:
                    process.wait(timeout=0.25)
                except subprocess.TimeoutExpired:
                    pass
        except BaseException:
            stop_process(process)
            raise
    if log.stat().st_size > MAX_LOG_BYTES:
        raise FirmwareError(f"child output exceeded 16 MiB; log: {log}")
    if process.returncode:
        raise FirmwareError(f"child exited {process.returncode}: {command[0]}; log: {log}")
    return log.read_text(encoding="utf-8", errors="replace")


def tool(core, package, name):
    """Find a tool in the selected installed Core package or the current PATH."""
    suffix = ".exe" if os.name == "nt" else ""
    candidates = (core / "packages" / package / "bin" / (name + suffix),
                  core / "packages" / package / (name + suffix))
    for candidate in candidates:
        if candidate.is_file():
            return candidate.resolve()
    found = shutil.which(name)
    if found:
        return Path(found).resolve()
    raise Incomplete(f"required installed tool is missing: {name} ({core})")


def environment_for(args, work):
    """Prepare installed SDK tools without inheriting external compiler injection."""
    environment = dict(os.environ)
    for name in ("CPATH", "CPLUS_INCLUDE_PATH", "C_INCLUDE_PATH", "LIBRARY_PATH",
                 "CXXFLAGS", "CPPFLAGS", "LDFLAGS", "CL", "_CL_", "PYTHONPATH",
                 "CMAKE_PREFIX_PATH", "CMAKE_MODULE_PATH", "CMAKE_TOOLCHAIN_FILE"):
        environment.pop(name, None)
    environment.update(PLATFORMIO_CORE_DIR=str(args.core_dir), CCACHE_DISABLE="1",
                       PLATFORMIO_WORKSPACE_DIR=str(work / "arduino-workspace"),
                       PLATFORMIO_BUILD_DIR=str(work / "arduino-build"),
                       PLATFORMIO_LIBDEPS_DIR=str(work / "arduino-libdeps"))
    return environment


def idf_settings(args, environment):
    """Require audited IDF5.5.5 and an explicit or known installed Python environment."""
    sdk = (args.idf_path or Path(environment.get(
        "IDF_PATH", args.core_dir / "packages/framework-espidf"))).resolve()
    require_file(sdk / "tools/idf.py")
    version = require_file(sdk / "version.txt").read_text(encoding="utf-8").strip()
    if version != "5.5.5":
        raise Incomplete(f"native adapter requires audited IDF5.5.5; found {version}")
    python_root = Path(environment.get(
        "IDF_PYTHON_ENV_PATH", args.core_dir / "penv/.espidf-5.5.5"))
    python = require_file(args.idf_python or python_root / (
        "Scripts/python.exe" if os.name == "nt" else "bin/python"))
    environment["IDF_PATH"] = str(sdk)
    environment["IDF_PYTHON_ENV_PATH"] = str(python.parent.parent)
    environment["IDF_PY_BUILD_JOBS"] = "4"
    if args.accept_missing_constraints:
        environment["IDF_PYTHON_CHECK_CONSTRAINTS"] = "no"
    else:
        environment.pop("IDF_PYTHON_CHECK_CONSTRAINTS", None)
    rom = args.core_dir / "packages/tool-esp-rom-elfs"
    if rom.is_dir():
        environment["ESP_ROM_ELF_DIR"] = str(rom)
    return sdk, python


def idf_prerequisites(sdk, python, environment, work):
    """Check installed Python requirements and offline constraints without repairing them."""
    checker = require_file(sdk / "tools/idf_tools.py")
    try:
        run_logged([python, checker, "check-python-dependencies"], work, environment,
                   work / "idf-python-prerequisites.log")
    except FirmwareError as error:
        raise Incomplete(f"installed IDF Python prerequisites are incomplete: {error}") from error


def arduino_platform(core):
    """Require the pinned installed pioarduino platform before invoking PlatformIO."""
    for manifest in (core / "platforms").glob("*/platform.json"):
        metadata = json.loads(manifest.read_text(encoding="utf-8"))
        if metadata.get("name") == "espressif32" and metadata.get("version") == "55.03.311":
            return manifest.parent.resolve()
    raise Incomplete(f"installed pioarduino55.03.311 platform is missing: {core}")


def compiler_roots(compiler, environment, work, row):
    """Probe actual compiler system includes and preserve its diagnostic evidence."""
    text = run_logged([compiler, "-E", "-x", "c++", "-", "-v"], work, environment,
                      work / f"{row}-compiler-includes.log", input_text="")
    match = re.search(r"#include <\.\.\.> search starts here:(.*?)End of search list\.",
                      text, flags=re.DOTALL)
    if not match:
        raise FirmwareError(f"compiler did not report include roots: {row}")
    roots = [Path(line.strip()).resolve(strict=True)
             for line in match[1].splitlines() if line.strip()]
    if not roots or not all(root.is_dir() for root in roots):
        raise FirmwareError(f"invalid compiler include roots: {row}")
    return roots


def expected_sources(row):
    """Return the exact current source set each actual example must compile."""
    if row.startswith("idf-"):
        return {*CORE_SOURCES, "examples/esp_idf/ExampleApp.cpp",
                "examples/esp_idf/Tusb32xIdf.cpp", "examples/esp_idf/basic/main/main.cpp"}
    entry = "02_owner_service" if "owner_service" in row else "01_basic_bringup_cli"
    return {*CORE_SOURCES, "examples/arduino/Tusb32xWire.cpp",
            "examples/arduino/ExampleApp.cpp", f"examples/{entry}/main.cpp"}


def ninja_dependencies(text):
    """Read one valid Ninja dependency record; stale or absent evidence fails."""
    lines = text.splitlines()
    if not lines or not re.fullmatch(r".+: #deps [0-9]+, deps mtime [0-9]+ \(VALID\)", lines[0]):
        raise FirmwareError("missing or stale Ninja dependency evidence")
    paths = [line[4:] for line in lines[1:] if line.startswith("    ")]
    if not paths:
        raise FirmwareError("empty Ninja dependency record")
    return paths


def verify_dependencies(records, package, build, approved, source_names, cwd):
    """Check every used header and exact extracted source set against approved roots."""
    if len(records) != len(source_names) or any(not record for record in records):
        raise FirmwareError("missing or duplicate compiler dependency records")
    used, sources = set(), set()
    for record in records:
        for name in record:
            path = Path(name)
            path = (path if path.is_absolute() else cwd / path).resolve(strict=True)
            if not any(path.is_relative_to(root) for root in approved):
                raise FirmwareError(f"compiler dependency escaped package/SDK: {path}")
            if path.is_relative_to(package) and path.suffix in (".cpp", ".c", ".cc", ".cxx"):
                sources.add(path.relative_to(package).as_posix())
            used.add(str(path))
    if sources != source_names:
        raise FirmwareError(f"compiled extracted sources differ: {sorted(sources ^ source_names)}")
    if not any(Path(path).is_relative_to(package / "include") for path in used):
        raise FirmwareError("no extracted public headers in compiler dependencies")
    evidence = {"approved_roots": [str(path) for path in approved],
                "compiled_sources": sorted(sources), "used_paths": sorted(used)}
    (build / "firmware-dependencies.json").write_text(
        json.dumps(evidence, indent=2) + "\n", encoding="utf-8")
    return evidence


def verify_identity(path, target):
    """Require unknown archive Git identity and the actual framework target token."""
    text = path.read_text(encoding="utf-8")
    for name, value in (("GIT", "unknown"), ("TREE", "unknown"), ("TARGET", target)):
        pattern = rf'^#define TUSB32X_BUILD_{name} "{re.escape(value)}"$'
        if not re.search(pattern, text, re.MULTILINE):
            raise FirmwareError(f"incorrect extracted firmware identity: {path}: {name}")
    return text


def verify_symbols(text, native):
    """Require linked core/CLI/adapter/application and reject Arduino in native images."""
    required = ["tusb32x_cli::Cli::tick()", "TUSB32x::TUSB32x::poll("]
    if native:
        required += ["app_main", "tusb32x_idf::loopExample()", "tusb32x_idf::writeRead(",
                     "uart_read_bytes", "uart_tx_chars", "i2c_master_transmit",
                     "i2c_master_receive"]
        forbidden = ("tusb32x_arduino::", "TwoWire::", "HardwareSerial::", "USBSerial::")
        if any(name in text for name in forbidden):
            raise FirmwareError("Arduino linkage in native IDF image")
    else:
        required += ["setup()", "tusb32x_arduino::loopExample()", "tusb32x_arduino::writeRead("]
    missing = [name for name in required if not re.search(
        rf"(?m)^\S+\s+\S\s+{re.escape(name)}" + (r".*$" if name.endswith("(") else r"$"), text)]
    if missing:
        raise FirmwareError(f"required linked symbols missing: {missing}")
    return required


def artifact(path):
    """Record a nonempty actual linked artifact with content hash and byte count."""
    if not path.is_file() or not path.stat().st_size:
        raise FirmwareError(f"linked artifact is absent or empty: {path}")
    return {"path": str(path), "bytes": path.stat().st_size,
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}


def inspect_row(args, row, work, environment, sdk, ninja, nm):
    """Inspect actual image, identity, sources, compiler inputs and native configuration."""
    native = row.startswith("idf-")
    target = "esp32s2" if "s2" in row else "esp32s3"
    build = work / row if native else work / "arduino-build" / row
    stem = "tusb32x_idf_cli" if native else "firmware"
    elf = build / f"{stem}.elf"
    artifacts = [artifact(build / f"{stem}.{extension}") for extension in ("elf", "bin", "map")]
    symbols = run_logged([nm, "-C", "--defined-only", elf], work, environment,
                         work / f"{row}-symbols.log")
    required = verify_symbols(symbols, native)
    identity = build / ("esp-idf/main/example_identity/FirmwareBuild.h" if native
                        else "example_identity/FirmwareBuild.h")
    identity_text = verify_identity(identity, target if native else row)
    compiler = tool(args.core_dir, "toolchain-xtensa-esp-elf", f"xtensa-{target}-elf-g++")
    approved = [args.package, build.resolve(),
                *compiler_roots(compiler, environment, work, row), *sdk]
    records = []
    if native:
        database = json.loads((build / "compile_commands.json").read_text(encoding="utf-8"))
        selected = []
        for entry in database:
            source = Path(entry["file"]).resolve(strict=True)
            if "arduino" in source.as_posix().lower():
                raise FirmwareError("Arduino source in native compile database")
            if not any(source.is_relative_to(root) for root in (args.package, build, *sdk)):
                raise FirmwareError(f"native compiled source escaped package/SDK: {source}")
            if source.is_relative_to(args.package):
                selected.append(entry)
        names = {Path(entry["file"]).resolve().relative_to(args.package).as_posix()
                 for entry in selected}
        if names != expected_sources(row) or len(selected) != len(names):
            raise FirmwareError("native compile database omitted or duplicated extracted sources")
        for index, entry in enumerate(selected):
            text = run_logged([ninja, "-C", build, "-t", "deps", entry["output"]],
                              work, environment, work / f"{row}-deps-{index}.log")
            records.append(ninja_dependencies(text))
        project = json.loads((build / "project_description.json").read_text(encoding="utf-8"))
        component = project["build_component_info"][args.package.name]["dir"]
        if (project["target"] != target
                or Path(project["project_path"]).resolve() != (
                    args.package / "examples/esp_idf/basic")
                or Path(project["config_file"]).resolve() != build / "sdkconfig"
                or Path(component).resolve() != args.package):
            raise FirmwareError("native project/target/component/sdkconfig provenance differs")
        config = (build / "sdkconfig").read_text(encoding="utf-8")
        for setting in (f'CONFIG_IDF_TARGET="{target}"', "CONFIG_ESP_CONSOLE_NONE=y",
                        "CONFIG_FREERTOS_HZ=1000", "CONFIG_ESP_MAIN_TASK_AFFINITY_CPU0=y"):
            if setting not in config.splitlines():
                raise FirmwareError(f"missing native application setting: {setting}")
        artifacts += [artifact(build / "sdkconfig"), artifact(build / "dependencies.lock")]
    else:
        depfiles = sorted((build / "src").rglob("*.cpp.d"))
        records = [make_dependencies(path.read_text(encoding="utf-8")) for path in depfiles]
    dependency = verify_dependencies(records, args.package, build, approved,
                                     expected_sources(row), build if native else args.package)
    return {"row": row, "result": "PASS", "artifacts": artifacts,
            "identity": identity_text, "required_symbols": required,
            "compiled_sources": dependency["compiled_sources"],
            "dependency_evidence": str(build / "firmware-dependencies.json")}


def execute(args, work, results):
    """Build requested rows once each, then verify every successful linked image."""
    environment = environment_for(args, work)
    nm = tool(args.core_dir, "toolchain-xtensa-esp-elf", "xtensa-esp-elf-nm")
    ninja = None
    if any(name.startswith("idf-") for name in args.checks):
        ninja = tool(args.core_dir, "tool-ninja", "ninja")
        cmake = tool(args.core_dir, "tool-cmake", "cmake")
        sdk, python = idf_settings(args, environment)
        compiler = tool(args.core_dir, "toolchain-xtensa-esp-elf", "xtensa-esp-elf-g++")
        environment["PATH"] = os.pathsep.join((str(cmake.parent), str(ninja.parent),
                                               str(compiler.parent), str(python.parent),
                                               environment.get("PATH", "")))
        idf_prerequisites(sdk, python, environment, work)
    for check in args.checks:
        if check == "arduino":
            arduino_platform(args.core_dir)
            for target in ("esp32s2", "esp32s3"):
                tool(args.core_dir, "toolchain-xtensa-esp-elf", f"xtensa-{target}-elf-g++")
            arduino = args.core_dir / "packages/framework-arduinoespressif32"
            metadata = json.loads(
                require_file(arduino / "package.json").read_text(encoding="utf-8"))
            if metadata["version"] != "3.3.11":
                raise Incomplete("Arduino adapter requires audited Arduino3.3.11")
            libs = args.core_dir / "packages/framework-arduinoespressif32-libs"
            if not libs.is_dir():
                raise Incomplete(f"Arduino SDK libraries missing: {libs}")
            if os.name == "nt":
                require_file(Path.home() / ".platformio/penv/Scripts/pio.exe")
                command = [str(Path(os.environ["SystemRoot"]) / "System32/cmd.exe"),
                           "/d", "/c", str(args.package / "scripts/pio.cmd")]
            else:
                executable = shutil.which("pio")
                if not executable:
                    raise Incomplete("installed PlatformIO executable is missing")
                command = [executable]
            command += ["run", "-d", str(args.package), "-j", "4"]
            for row in ARDUINO_ROWS:
                command += ["-e", row]
            run_logged(command, work, environment, work / "arduino-build.log", timeout=1800)
            for row in ARDUINO_ROWS:
                results.append(inspect_row(args, row, work, environment,
                                           [arduino.resolve(), libs.resolve()], ninja, nm))
        else:
            target = "esp32s2" if check == "idf-s2" else "esp32s3"
            build = work / check
            command = [python, sdk / "tools/idf.py", "-C", args.package / "examples/esp_idf/basic",
                       "-B", build, f"-DSDKCONFIG={build / 'sdkconfig'}", f"-DIDF_TARGET={target}",
                       "-DCCACHE_ENABLE=0", "build"]
            run_logged(command, work, environment, work / f"{check}-build.log", timeout=1800)
            results.append(inspect_row(args, check, work, environment, [sdk], ninja, nm))


def main(argv=None):
    """Run explicit package-link checks and retain scope, failure and partial evidence."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--checks", type=selections, default=CHECKS)
    parser.add_argument("--core-dir", type=Path, default=Path(os.environ.get(
        "PLATFORMIO_CORE_DIR", Path.home() / ".platformio")))
    parser.add_argument("--idf-path", type=Path)
    parser.add_argument("--idf-python", type=Path)
    parser.add_argument("--accept-missing-constraints", action="store_true",
                        help="explicitly permit IDF_PYTHON_CHECK_CONSTRAINTS=no for existing SDK")
    args = parser.parse_args(argv)
    result = {"checks": list(args.checks), "result": "INCOMPLETE", "rows": [],
              "hardware": "NOT RUN", "accept_missing_constraints": args.accept_missing_constraints}
    code = 2
    try:
        args.package, args.out = validate_paths(args.package, args.out)
        args.core_dir = args.core_dir.resolve()
        try:
            args.out.mkdir(parents=True, exist_ok=True)
            work = Path(tempfile.mkdtemp(prefix="firmware-", dir=args.out)).resolve()
        except OSError as error:
            raise Incomplete(f"cannot create firmware evidence directory: {error}") from error
        result.update(package=str(args.package), work=str(work))
        before = snapshot(args.package)
        try:
            execute(args, work, result["rows"])
            expected = [row for check in args.checks
                        for row in (ARDUINO_ROWS if check == "arduino" else (check,))]
            if ([row.get("row") for row in result["rows"]] != expected
                    or any(row.get("result") != "PASS" for row in result["rows"])):
                raise FirmwareError("requested image checks did not all complete exactly once")
        finally:
            if snapshot(args.package) != before:
                raise FirmwareError("firmware build modified the extracted package")
        result["result"], code = "PASS", 0
    except Incomplete as error:
        result["error"] = str(error)
    except KeyboardInterrupt:
        result["error"], code = "interrupted; logs and completed rows retained", 130
    except (FirmwareError, PackageError, OSError, ValueError, KeyError,
            subprocess.SubprocessError) as error:
        result["result"], result["error"], code = "FAIL", str(error), 1
    if args.out.is_absolute() and args.out.is_dir():
        try:
            temporary = args.out / "firmware-result.json.tmp"
            temporary.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
            temporary.replace(args.out / "firmware-result.json")
        except OSError as error:
            print(f"INCOMPLETE: cannot save firmware report: {error}")
            return code if code in (1, 130) else 2
    print(json.dumps(result, indent=2))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
