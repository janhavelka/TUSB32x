"""Build copied public artifacts and execute the independent consumer and sentinels.

Python orchestrates this maintainer check only. The child CMake project imports a
previously built library and has no generator, Python or repository dependency.
Keep the temporary tree for inspecting failures; each invocation gets a new tree.
"""

import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def run(command, environment, expected=0):
    """Run a bounded command and require its exact exit code, including sentinels."""
    print(subprocess.list2cmdline([str(item) for item in command]), flush=True)
    result = subprocess.run(command, env=environment, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            timeout=60, check=False)
    print(result.stdout, end="", flush=True)
    if result.returncode != expected:
        raise RuntimeError(f"exit {result.returncode}, expected {expected}")
    return result.stdout


def main():
    """Copy only delivered artifacts, build two TUs, and prove checks are live."""
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("source", "include", "library", "output", "cmake", "compiler",
                 "make-program", "generator", "config", "toolchain-flags"):
        parser.add_argument("--" + name, required=True)
    args = parser.parse_args()
    root = Path(tempfile.mkdtemp(prefix="portable-", dir=args.output)).resolve()
    shutil.copytree(args.source, root / "source")
    shutil.copytree(args.include, root / "public")
    (root / "library").mkdir()
    library = Path(shutil.copy2(args.library, root / "library"))
    flags = Path(shutil.copy2(args.toolchain_flags, root / "flags.cmake"))
    environment = os.environ.copy()
    if os.name == "nt":
        system = Path(os.environ["SystemRoot"])
        # Keep runtime DLLs available without also exposing compiler-directory tools.
        runtime = root / "runtime"
        runtime.mkdir()
        for pattern in ("libgcc*.dll", "libstdc++*.dll", "libwinpthread*.dll",
                        "libmcfgthread*.dll", "libssp*.dll"):
            for dll in Path(args.compiler).parent.glob(pattern):
                shutil.copy2(dll, runtime)
        environment["PATH"] = os.pathsep.join(map(str, (
            system / "System32", system, runtime)))
        for tool in ("git", "python", "python3", "doxygen", "pio"):
            if shutil.which(tool, path=environment["PATH"]):
                raise RuntimeError(f"unexpected downstream tool on isolated PATH: {tool}")
        print("Child PATH excludes Git, Python, Doxygen and PlatformIO.")
    else:
        print("Non-Windows: ordinary PATH retained; tool absence is not claimed.")
    config = args.config or "Debug"
    build = root / "build"
    configuration = [] if ("Multi-Config" in args.generator or
                           args.generator.startswith("Visual Studio") or
                           args.generator == "Xcode") else [f"-DCMAKE_BUILD_TYPE={config}"]
    run([args.cmake, "-C", flags, "-S", root / "source", "-B", build, "-G", args.generator,
         f"-DCMAKE_CXX_COMPILER={args.compiler}",
         f"-DCMAKE_MAKE_PROGRAM={args.make_program}",
         f"-DTUSB32X_PUBLIC_INCLUDE_DIR={root / 'public'}",
         f"-DTUSB32X_LIBRARY_FILE={library}",
         "-DTUSB32X_WRAP_C_ALLOCATIONS=ON", *configuration], environment)
    run([args.cmake, "--build", build, "--config", config], environment)
    filename = "tusb32x_portable_consumer" + (".exe" if os.name == "nt" else "")
    executable = build / config / filename
    if not executable.exists():
        executable = build / filename
    output = run([executable], environment)
    match = re.search(r"Portable consumer: ([1-9][0-9]*) checks, 0 failures", output)
    if not match:
        raise RuntimeError("consumer did not report executed successful checks")
    run([executable, "--fail-check"], environment, 1)
    run([executable, "--allocate-probe"], environment, 86)
    wrapped = "C allocation wrapping: enabled" in output
    run([executable, "--malloc-probe"], environment, 86 if wrapped else 77)
    if not wrapped:
        print("NOT RUN: C allocator trap unavailable; C++ allocation trap passed.")
    print(f"PASS: isolated consumer ({match[1]} checks) and exact-exit sentinels; {root}")


if __name__ == "__main__":
    main()
