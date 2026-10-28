# Prompt 32 extracted framework consumer audit

Inspected on 2026-10-28 at TUSB32x
`458706ae7ebedeb24d43e2e7a7f6a48732865ba5`, branch
`promptset/tusb32x-implementation-20261023`; the initial worktree was clean.
This delegated scope owns this report and, once the lead freezes an archive,
framework build evidence beneath separately assigned workspace roots. The lead
owns manifests, integration and Git. No SDK installation, sibling edits, hardware
access, flashing or remote CI is part of this work.

Read parent/local AGENTS, COMMON, AGENT_WORKFLOW, prompt 32, relevant
ENGINEERING_RULES and MAINTAINABILITY; inspected root CMake, library.json,
idf_component.yml, platformio.ini, scripts/pio.cmd, both firmware identity
scripts, development/Arduino/native guides, native application CMake and
sdkconfig defaults, and the Arduino/native SDK fixture configurations.
Revisited prompt 25/26 audit and validation evidence, then independently checked
the installed versions and native Python dependency check. There is currently
no checked-in `.github/workflows` directory.

## Existing tools

| Tool | Inspected location | Actual version |
| --- | --- | --- |
| Managed PlatformIO Core | `%USERPROFILE%/.platformio/penv/Scripts/pio.exe`, through scripts/pio.cmd | 6.1.19 |
| Arduino platform | `%USERPROFILE%/.platformio/platforms/espressif32/platform.json` | pioarduino 55.03.311 |
| Arduino framework | `%USERPROFILE%/.platformio/packages/framework-arduinoespressif32/package.json` | 3.3.11 |
| Arduino precompiled SDK | sibling framework-arduinoespressif32-libs/package.json | 5.5.5+sha.b774170ff46 |
| Native SDK | `C:/pio/packages/framework-espidf/{package.json,tools/cmake/version.cmake}` | package 3.50505, IDF 5.5.5 |
| Native Python | `C:/pio/penv/.espidf-5.5.5/Scripts/python.exe` | 3.12.10 |
| Native component manager | installed dist-info metadata | 2.4.11 |
| Native CMake | `C:/pio/packages/tool-cmake/bin/cmake.exe` | 4.0.3 |
| Native Ninja | `C:/pio/packages/tool-ninja/ninja.exe` | 1.13.1 |
| Xtensa compiler | `C:/pio/packages/toolchain-xtensa-esp-elf/bin/xtensa-esp32s2-elf-g++.exe` and managed user package S3 compiler | GCC 14.2.0, esp-14.2.0_20260121 |
| Ordinary host tools | current PATH | Python 3.12.10, CMake 4.0.1 |

Native IDF is installed under C:/pio. The user-managed launcher executes the
existing Core; the inherited PLATFORMIO_CORE_DIR=C:/pio selects the actual
Arduino platform/packages there. Their logged versions match the user-package
preflight inventory above. The shell initially has no IDF_PATH or
idf.py activation. Reuse prompt 26's explicit environment rather than assuming
a standalone Espressif installation or installing another Core:

```powershell
$env:IDF_PATH = 'C:\pio\packages\framework-espidf'
$env:IDF_PYTHON_ENV_PATH = 'C:\pio\penv\.espidf-5.5.5'
$env:IDF_PYTHON_CHECK_CONSTRAINTS = 'no'
$env:ESP_ROM_ELF_DIR = 'C:\pio\packages\tool-esp-rom-elfs'
$env:PATH = 'C:\pio\packages\tool-cmake\bin;C:\pio\packages\tool-ninja;C:\pio\packages\toolchain-xtensa-esp-elf\bin;C:\pio\penv\.espidf-5.5.5\Scripts;' + $env:PATH
$idfPython = 'C:\pio\penv\.espidf-5.5.5\Scripts\python.exe'
& $idfPython "$env:IDF_PATH/tools/idf.py" --version
& $idfPython "$env:IDF_PATH/tools/idf_tools.py" check-python-dependencies
```

The existing managed environment lacks standalone constraints-cache files.
Without this existing `IDF_PYTHON_CHECK_CONSTRAINTS=no` selection, idf.py reports
the missing cache. This selection still checks installed SDK requirements; it
does not set PYTHON_DEPS_CHECKED or skip dependency validation. Retain its explicit
record in final evidence. SDK Git metadata is unavailable, so the SDK may warn
before reporting the source-defined 5.5.5 version.

## Content and path decisions

The current export and IDF inclusion lists contain both firmware identity scripts,
the root PlatformIO configuration/launcher, actual application sources, shared
CLI/owner/mailbox files, native CMake and both native sdkconfig default files.
No immediate missing framework build input was found in those lists. Keep
scripts/arduino_identity.py and scripts/idf_identity.py required; a blanket Python
exclusion would break real firmware builds. scripts/generate_version.py is a
maintainer tool and is unnecessary for core or extracted example compilation.

The root Arduino environments are esp32s2dev, esp32s3dev,
esp32s2_owner_service and esp32s3_owner_service. Their explicit source filters
compile the five core sources and extracted shared/application files, with one
entry point per environment. Root-relative `-Iinclude` and identity script paths
require `-d` to select the extracted root. They do not name this source checkout.

The native project derives its root from examples/esp_idf/basic, adds that exact
directory through EXTRA_COMPONENT_DIRS, and derives the required component name
from the root basename. Retain this behavior when renaming the extracted library.
Its CMake writes SDKCONFIG and dependencies.lock under the build directory, not
the source application. Use an absolute SDKCONFIG argument anyway, independently
for S2/S3. Root component registration includes only the five portable sources.

Both identity scripts verify that Git's top level equals the exact extracted
library root before accepting HEAD/status. An archive without .git therefore
reports Git/tree unknown even beneath an unrelated enclosing repository. Generated
FirmwareBuild.h remains example-only and build-local. Verify those generated
headers after linking rather than trusting archive origin or host checkout HEAD.

Potential leakage checks for the frozen build: inspect compile databases (IDF),
dependency files/verbose source commands (Arduino), project_description.json,
map files and final symbols. All TUSB32x core/shared/application source paths must
resolve beneath the assigned extracted root. SDK/tool paths may point to the
existing installed packages. Native images must contain app_main, core poll,
shared CLI/owner and native transport/UART APIs and no Arduino/Wire linkage.
Fresh build/cache paths are required; do not reuse the old prompt 26 outputs.

## Commands to apply to the frozen archive

With lead-assigned absolute `$packageRoot` and separate output roots, use the
launcher shipped by that extracted package:

```powershell
$env:PLATFORMIO_BUILD_DIR = $arduinoBuild
& "$packageRoot/scripts/pio.cmd" run -d $packageRoot -e esp32s2dev -e esp32s3dev -e esp32s2_owner_service -e esp32s3_owner_service
& $idfPython "$env:IDF_PATH/tools/idf.py" -C "$packageRoot/examples/esp_idf/basic" -B $idfS2Build "-DSDKCONFIG=$idfS2Build/sdkconfig" -DIDF_TARGET=esp32s2 -DCCACHE_ENABLE=0 build
& $idfPython "$env:IDF_PATH/tools/idf.py" -C "$packageRoot/examples/esp_idf/basic" -B $idfS3Build "-DSDKCONFIG=$idfS3Build/sdkconfig" -DIDF_TARGET=esp32s3 -DCCACHE_ENABLE=0 build
```

## Frozen archive and completed framework links

The final delivery artifact is the real managed-Core archive
`build/TUSB32x-p32-final.tar.gz`, SHA-256
`97549728fd29eae2660630a011a0005991e39bc99ce2ad62aea703821a357316`.
All framework builds consumed the same renamed root:
`C:/Temp/tusb32x-p32-delivery/package-9uqaz8ky/renamed-tusb32x`.
Their fresh output parent was `C:/Temp/tusb32x-p32-final2-firmware`, outside the
extraction. SHA-256 comparison before and after all six links proved the archive
unchanged and all 76 extracted file names/bytes unchanged. The builds created no
configuration, identity header, lock, cache or other file in the extracted root.
The lead created an unrelated Git repository at C:/Temp/tusb32x-p32-delivery,
HEAD `73027de1db658cd1e1e05bdd2d7ac4093e3ba2d1`. The library has no .git;
all six builds still report Git/tree unknown. This proves the identity scripts
do not inherit the enclosing repository's identity.

Actual commands, from the output parent after the native environment above:

```powershell
$packageRoot = 'C:/Temp/tusb32x-p32-delivery/package-9uqaz8ky/renamed-tusb32x'
$firmwareRoot = 'C:/Temp/tusb32x-p32-final2-firmware'
$env:PLATFORMIO_CORE_DIR = 'C:\pio'
$env:CCACHE_DISABLE = '1'
$env:PLATFORMIO_WORKSPACE_DIR = "$firmwareRoot/arduino-workspace"
$env:PLATFORMIO_BUILD_DIR = "$firmwareRoot/arduino-build"
$env:PLATFORMIO_LIBDEPS_DIR = "$firmwareRoot/arduino-libdeps"
& "$packageRoot/scripts/pio.cmd" run -d $packageRoot -j 4 -e esp32s2dev -e esp32s3dev -e esp32s2_owner_service -e esp32s3_owner_service
$env:IDF_PY_BUILD_JOBS = '4'
& $idfPython "$env:IDF_PATH/tools/idf.py" -C "$packageRoot/examples/esp_idf/basic" -B "$firmwareRoot/idf-s2" "-DSDKCONFIG=$firmwareRoot/idf-s2/sdkconfig" -DIDF_TARGET=esp32s2 -DCCACHE_ENABLE=0 build
& $idfPython "$env:IDF_PATH/tools/idf.py" -C "$packageRoot/examples/esp_idf/basic" -B "$firmwareRoot/idf-s3" "-DSDKCONFIG=$firmwareRoot/idf-s3/sdkconfig" -DIDF_TARGET=esp32s3 -DCCACHE_ENABLE=0 build
```

The output helper `run_build.py` saved structured command/environment/exit/log
metadata to arduino.json, idf-s2.json and idf-s3.json. Each invocation had an
1800-second process limit and 16MiB output limit; actual logs were 45863,
140944 and 147598 bytes respectively. The Arduino four-row command exited 0
in 69.69s; native S2 and S3 each exited 0 in 70.73s and 72.89s. Commands use
SDK-supported IDF_PY_BUILD_JOBS=4. No production source/configuration changed
during this final matrix.

| Extracted consumer | Result | Linked ELF SHA-256 |
| --- | --- | --- |
| Arduino esp32s2dev | PASS | `0c7dacda33a9f59ca3db4f366f2588735bf7e568ccff806fc87eb369a42f693d` |
| Arduino esp32s3dev | PASS | `0775fae447361a198f96cabcf77d95ebba59463b57adafe6e10fda09fde0d92d` |
| Arduino esp32s2_owner_service | PASS | `6acca89964873e37f10b435d533704b7e0bab9cec595ca1b62c0b36e46b2888d` |
| Arduino esp32s3_owner_service | PASS | `d53e571cb0467c884093fdbf2c79fd26a7d7193be53797b870150da837adab16` |
| Native IDF S2 | PASS | `195fceed2240a0224efce52042e5fc700111c17306195819e36e0c0666c97ceb` |
| Native IDF S3 | PASS | `0c8c071079ca29f348057ec4ab49872758ae544f79b0a3a665f367e52611d82e` |

Arduino ELF/BIN/map files are in arduino-build/<environment>/firmware.*.
Native ELF/BIN/map files are in idf-s2/tusb32x_idf_cli.* and
idf-s3/tusb32x_idf_cli.*. The six ELF files are nonempty real linked images;
BIN/map files are separately present and hashed in
`C:/Temp/tusb32x-p32-final2-firmware/image-evidence.json`. The bounded verification helper
`verify_images.py` passed all six rows and saved symbol lists beside that JSON.
This output-only helper is evidence tooling, not a shipped build dependency.

Each Arduino environment compiled 11 core/shared/application sources. DWARF
compilation-unit directories in every one of those objects equal the exact
renamed extraction, and each recorded source resolves beneath that root.
Dependency files and maps contain no original checkout path. Linked symbols
include setup(), the actual Arduino loopExample/writeRead, shared Cli::tick and
core poll. These checks make accidental source-checkout consumption observable.

Each native compile database contains exactly 11 sources beneath the extraction,
with no Arduino source. Project metadata names renamed-tusb32x as the actual core
component and uses the extracted application's project path. Compile databases,
project metadata and maps contain no original checkout path. Native linked
symbols include app_main, loopExample, writeRead, shared Cli::tick, core poll,
uart_read_bytes, uart_tx_chars, i2c_master_transmit and i2c_master_receive. Defined
symbol lists contain no tusb32x_arduino, TwoWire, HardwareSerial or USBSerial
linkage. Real native SDK compilation/linkage is established independently from
the Arduino framework's precompiled IDF libraries.

Generated FirmwareBuild.h reports Git/tree unknown in all six images, with the
correct distinct framework target token. Native S2/S3 config_file paths point
to their own absolute build-local sdkconfig; both target values, CPU0 task
affinity, console NONE and HZ1000 were checked after both builds completed.
Configuration SHA-256 values differ as expected, and each dependencies.lock is
build-local. No project-source sdkconfig/lock exists. The immutable file check
also proves the two target builds did not rewrite each other's source defaults.

All six default diagnostic firmware links PASS. Physical UART/USB interaction,
electrical behavior, real transfer timing, IRQ, CC/VBUS/VCONN, flashing and board
qualification remain NOT RUN. No remote CI was dispatched or queried.

## Earlier artifacts and cancelled attempt

The first complete six-row matrix used archive SHA-256
`230c31cafbfce9932ff4732075f1f8619f39855adaf0b0f50688c148d1454e08`
and passed under C:/Temp/tusb32x-p32-firmware. Its full command/image evidence is
retained there. Only README.md, docs/development.md and examples/README.md differ
between its 76-file extraction and the final delivery artifact; the final matrix
above nevertheless rebuilt every row from fresh outputs and the final archive.
Initial native argument checks on that earlier matrix rejected unsupported
`idf.py -j` before configuration; the argument-preflight logs remain preserved.

A subsequent candidate archive SHA-256
`3883dc5ecdf9edce4bffd20ee8a8a9db88fd117e51aa4e5e6bc32e1ee0b15d96`
was retracted by the lead for one remaining README correction after its builds
had started. Its three exact task-wrapper process trees were stopped on that
instruction. Partial outputs/logs remain under C:/Temp/tusb32x-p32-final-firmware;
that attempt is CANCELLED / NOT RUN to completion and supplies no final success
claim. No cached outputs from either earlier attempt were reused by the final
delivery matrix.
