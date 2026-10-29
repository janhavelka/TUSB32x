# Software validation environment setup

Two separate environments address the remaining software checks: an ESP-IDF 6.1
environment builds the native ESP32-S2/S3 firmware, while a Linux host environment
runs the C++ tests with AddressSanitizer and UndefinedBehaviorSanitizer. Neither
requires a connected board. Installing one does not supply the other.

This is an optional setup recipe, reviewed on 2026-10-29. Its installation and
test commands have **NOT RUN** as part of this documentation change. Existing
PASS results and their exact source revisions remain in [validation](validation.md).
In particular, the earlier Linux sanitizer and IDF 6.1 CI results at
`c50f8ddda8809425ede2161586466c95fa650ae6` do not verify later application changes.

## SDK files, tools and Python environments

An ESP-IDF installation has several cooperating parts:

| Part | What it contains | Why it matters |
| --- | --- | --- |
| SDK directory (`IDF_PATH`) | ESP-IDF C/C++ sources, headers, components and build scripts, including `tools/idf.py` | Selects the framework revision compiled into the application |
| Native tools | The ESP32 cross-compiler, linker and supporting binaries; CMake and Ninja drive the build | Produce firmware for S2/S3 instead of an ordinary PC executable |
| Python virtual environment (`IDF_PYTHON_ENV_PATH`) | A Python interpreter environment with that SDK's Python packages | Runs `idf.py`, component management and the SDK's other Python tools |
| SDK constraint file | Allowed Python dependency versions for the chosen ESP-IDF release | Makes dependency checks specific to that release |

A virtual environment is a directory holding a Python environment and its
packages. It is separate from the SDK's source tree and from the ESP32 compiler.
Finding `idf.py` or the SDK headers therefore does not establish that the complete
build environment works. Espressif's installer prepares these pieces; its export
script selects them for the current terminal. Espressif documents separate tool
versions, Python environments and dependency checks in
[IDF tools](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/tools/idf-tools.html).

Keep the working managed ESP-IDF 5.5.5 installation and its Python environment.
For native 6.1 builds, install that exact SDK separately using the official
[ESP-IDF v6.1 setup instructions](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/get-started/index.html),
selecting S2/S3 tools. Use its own SDK directory and Python environment, and open
a separate SDK terminal when switching versions. This repository audits 5.5.5
and 6.1.0 (tag `v6.1`); another release needs separate review. A prior missing
local 6.1 installation is not a claim that Espressif does not provide it.

After activating the selected SDK in PowerShell, inspect it before building:

```powershell
$env:IDF_PATH
$env:IDF_PYTHON_ENV_PATH
Get-Command python, idf.py, cmake, ninja
python -c "import sys; print(sys.executable); print(sys.version)"
idf.py --version
python "$env:IDF_PATH/tools/idf_tools.py" check
python "$env:IDF_PATH/tools/idf_tools.py" check-python-dependencies
```

The paths must identify the selected installation and its environment, and the
checks must succeed. Do not repair a 6.1 dependency error by upgrading packages
inside the working 5.5.5 environment. Use the SDK's own installation procedure
and constraints. The [native application guide](../examples/esp_idf/basic/README.md)
provides build commands with separate absolute build and `SDKCONFIG` paths for
each SDK and target. The [local validation entry point](validation.md#local-validation-entry-point)
accepts `--idf-path` and `--idf-python` to select an existing SDK and interpreter
explicitly for extracted-package validation. A successful source application
build and an extracted-package check are separate evidence.

Windows PlatformIO work continues through the existing `scripts\pio.cmd` and
VS Code-managed Core. No second PlatformIO Core is needed for the Linux host
tests below. The host Python environment below is also separate from both SDK
Python environments.

## What ASan and UBSan check

Sanitizers add checks while compiling a test executable, then report detected
errors when that executable runs. AddressSanitizer (ASan) detects memory errors
such as accesses outside a buffer, use of freed memory and invalid frees. The
Linux configuration below also enables leak detection. See LLVM's
[AddressSanitizer documentation](https://clang.llvm.org/docs/AddressSanitizer.html).

UndefinedBehaviorSanitizer (UBSan) detects selected operations for which C++ does
not define a valid result, such as signed integer overflow, invalid shifts and
misaligned or null-pointer dereferences. See LLVM's
[UndefinedBehaviorSanitizer documentation](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).

Here they run the host C++ tests on the computer. They are not inserted into the
ESP32 firmware by the validator. A clean run means the executed cases produced
no enabled sanitizer report; it does not prove the absence of bugs in unexecuted
paths, correct physical I2C timing or correct electrical behavior. Keep ordinary
host tests, firmware links and board qualification as distinct evidence.

## Optional Linux host on Windows with WSL 2

WSL runs a Linux environment alongside Windows. Ubuntu 24.04 matches the Linux
distribution selected by the checked-in CI workflow. These commands are intended
for the user to run; the documentation change does not install WSL or restart
Windows. Microsoft's current command-based setup requires Windows 11 or Windows
10 version 2004/build 19041 or later.
[Microsoft WSL installation](https://learn.microsoft.com/en-us/windows/wsl/install)

First inspect existing installations in PowerShell:

```powershell
wsl --status
wsl --list --verbose
wsl --list --online
```

If WSL or Ubuntu 24.04 is missing, open PowerShell **as Administrator** and use
the distribution name reported by the online list:

```powershell
wsl --install -d Ubuntu-24.04
```

Restart Windows if requested, launch Ubuntu, and complete its Linux username and
password setup. Then `wsl --list --verbose` should show that distribution with
version `2`. If an existing Ubuntu installation uses WSL 1, the documented
conversion command is `wsl --set-version Ubuntu-24.04 2`. WSL 2 needs hardware
virtualization enabled and the Virtual Machine Platform feature; installation
errors may require enabling virtualization in firmware or a restart. Follow
[Microsoft's troubleshooting steps](https://learn.microsoft.com/en-us/windows/wsl/troubleshooting#installation-issues)
for the actual error.

Launch the Linux shell from ordinary PowerShell:

```powershell
wsl -d Ubuntu-24.04
```

The following blocks run in **Ubuntu Bash**, not PowerShell. Install host tools:

```bash
sudo apt update
sudo apt install build-essential git python3 python3-venv clang-18 llvm-18 libclang-rt-18-dev
```

`clang-18` supplies the compiler, `libclang-rt-18-dev` supplies its sanitizer
runtimes, and `llvm-18` supplies the symbolizer for readable stack traces. Ubuntu
24.04 provides [Clang 18](https://packages.ubuntu.com/noble/clang-18) and
[compiler-rt 18](https://packages.ubuntu.com/libclang-rt-18-dev) in its `universe`
repository. If apt cannot find them, inspect the configured Ubuntu release and
repositories before choosing a different compiler.

Keep the Linux checkout, virtual environment and build output in the Linux
filesystem. Microsoft recommends this for Linux-tool performance; see
[WSL filesystem guidance](https://learn.microsoft.com/en-us/windows/wsl/filesystems).
Do not reuse Windows virtual environments or CMake build directories under
`/mnt/c`. Clone the intended branch into a new directory:

```bash
mkdir -p ~/src
git clone --branch promptset/tusb32x-implementation-20261023 https://github.com/janhavelka/TUSB32x.git ~/src/TUSB32x
cd ~/src/TUSB32x
git rev-parse HEAD
git status --short
python3 -m venv ~/.venvs/tusb32x-host
source ~/.venvs/tusb32x-host/bin/activate
python -m pip install cmake==4.0.3 ninja==1.13.0 PyYAML==6.0.3
```

Use the source revision you intend to validate and record it; a remote clone
does not include uncommitted Windows edits. If this Linux checkout already exists,
inspect its branch and local changes instead of cloning over it. On later sessions,
enter the checkout and activate the same host virtual environment.

CMake 4.0.3, Ninja 1.13.0 and PyYAML 6.0.3 are the checked-in CI package pins.
PyYAML is used by the explicit workflow audit below; the CI host job itself only
installs CMake and Ninja. CI also pins Python **3.12.16**. Ubuntu's `python3`
package may provide a different patch version: this local recipe requires Python
3.10+ and records its actual version, so it does not reproduce every CI tool byte.
Do not replace Ubuntu's system Python to make that claim. The full matrix and
other job-specific pins are in [development metadata](development.md#declared-ci-matrix).

Verify that the shell finds Linux tools and the host virtual environment:

```bash
cat /etc/os-release
uname -m
command -v python cmake ctest ninja clang++-18 llvm-symbolizer-18
python --version
python -c 'import sys; print(sys.executable)'
python -m pip check
cmake --version
ctest --version
ninja --version
clang++-18 --version
llvm-symbolizer-18 --version
python tools/check_ci.py
python scripts/generate_version.py check
```

Keep the `clang++-18` driver name and the virtual environment's `python` path
when invoking tools. Resolving their symlinks to underlying binaries can lose
C++ driver or Python environment selection. `check_ci.py` audits the local YAML;
it neither dispatches nor checks a remote run.

Run the existing sanitizer selection from the repository root:

```bash
export PATH="/usr/lib/llvm-18/bin:$PATH"
export ASAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer-18)"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
python tools/validate.py --checks native,contracts --configs Debug --compiler clang++-18 --sanitize --out build/validation/wsl-sanitized
```

This uses the same sanitizer flags and runtime options as the CI host selection.
The validator creates a fresh run directory, configures and builds the tests,
verifies registration, and runs the native/contract groups with finite timeouts.
Its printed evidence directory contains command logs, JSON reports and CTest
JUnit XML. Exit `0` means the requested checks passed; `1` means failure and `2`
means incomplete checks or an invocation problem. Missing runtime libraries or
a failed build must be resolved before recording a sanitizer PASS.

Retain the tested Git revision, dirty state, tool versions, report and logs with
the result. This command does not run every validation group, install ESP-IDF,
build firmware, dispatch CI, open a serial port or flash hardware. WSL setup and
the sanitizer result remain **NOT RUN** until these steps actually execute.
