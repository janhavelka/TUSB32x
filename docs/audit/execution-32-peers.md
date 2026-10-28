# Prompt 32 peer and package consumer audit

Fresh read-only inspection on 2026-10-28. No sibling build, edit, fetch, remote
CI query or hardware access was performed. Each peer's `AGENTS.md` was read;
the shared parent `../AGENTS.md` and TUSB32x `AGENTS.md` govern this work.
All paths below are relative to the sibling named in the first column.

| Peer | Current HEAD | `git status --short` |
| --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | Empty; clean |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | Empty; clean |
| OPT4001 | `dab818e575fc25a0949d962bc0df600eb2ed5200` | Empty; clean |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | Empty; clean |
| INA3221 | `41b6184b75f3c152611d4236f7ea41a5ce77bc2c` | Empty; clean |

## Sources and decisions

TMP1x2: inspected `tools/check_package.py`, `scripts/generate_version.py`,
`CMakeLists.txt`, `include/TMP1x2/TMP1x2.h`,
`test/core_no_arduino/compile_main.cpp` and `.github/workflows/ci.yml`.
The package checker enumerates required public/core/shared CLI/native example
members, manually writes regular file contents into a temporary renamed library,
then configures, builds and executes a CMake consumer through `add_subdirectory`
and the named target. Adopt the renamed-root consumer, actual execution and
explicit example prerequisites. Its consumer tests a rejected unbound start;
TUSB32x instead binds callbacks and completes a real cooperative operation.
Do not copy its unbounded `getmembers()`/member reads, incomplete portable-name
checks, or subprocesses without timeouts. Its generator also probes Git and
timestamps; preserve TUSB32x's separate stable version and example identity
paths instead. Its core CMake places Python discovery inside tests ON and leaves
tests OFF by default; adopt that downstream boundary. Its checked-in workflow
uses PlatformIO 6.1.19 and S2/S3 IDF 5.3.2/5.5.1/6.0.1 rows. These are source
observations, not executed results or TUSB32x support declarations.

24Cxx: inspected `tools/check_package_contents.py`, `library.json`,
`CMakeLists.txt`, `include/EEPROM24Cxx/EEPROM24Cxx.h`,
`test/core_no_arduino/compile_main.cpp` and `.github/workflows/ci.yml`.
Adopt explicit export includes, exclusions for generated firmware files and
actual package Markdown-link checks. Its checker permits missing archived
reference links by counting them separately. TUSB32x must label such paths
repository-only or replace them with supported links; an unqualified broken
package link does not pass. Its dictionary of archive names can collapse
duplicates, and its workflow invokes external `tar` after checking only some
unsafe-name forms. Reject both as extraction safety mechanisms. The workflow
configures/builds the extracted library itself; TUSB32x adds a separate consumer
that links the public target and executes behavior. Public compatibility aliases
and EEPROM-specific APIs are not copied into the TUSB32x fixture.

OPT4001: inspected `tools/check_clean_consumer_package.py`, `library.json`,
`include/OPT4001/OPT4001.h`, `test/core_no_arduino/compile_main.cpp` and
`.github/workflows/ci.yml`. Its script packs the library through the existing
Windows wrapper, creates an external native PlatformIO project, points
`lib_deps` at the archive and directly includes all five public headers. Adopt
the separate workspace and complete public include set. It compiles the native
consumer but does not run that executable in this script; do not call this
runtime proof. The script lacks archive member safety inspection and subprocess
deadlines; those remain TUSB32x checker responsibilities. Do not copy its broad
manifest exclusions as a complete package-content policy or use its
espressif32-only manifest claim for TUSB32x's portable core.

LDC1614: inspected `tools/check_clean_consumer_compile.py`, `CMakeLists.txt`,
`include/LDC1614/LDC1614.h`, `.github/workflows/ci.yml`,
`examples/esp_idf/basic/CMakeLists.txt`,
`examples/esp_idf/basic/main/CMakeLists.txt` and
`examples/esp_idf/basic/main/main.cpp`. Its guard directly compiles individual
public headers and runs a linked consumer against extracted code. Adopt real
compile/link/run evidence and bounded subprocesses; reject its older-Python
unfiltered `extractall` fallback and package version-generator prerequisite.
Its component CMake unconditionally discovers Python and runs the generator;
TUSB32x core consumers must not do this. Native example CMake declares real
IDF components and the source defines `app_main` with native I2C, GPIO and
FreeRTOS headers. Its workflow declares real S2/S3 IDF 6.0.2 builds via
`espressif/esp-idf-ci-action@v1`. Adopt genuine native image builds as a distinct
layer of evidence, while retaining TUSB32x's audited IDF 5.5.5 configuration.
The workflow declaration itself proves no successful link here.

INA3221: inspected `CMakeLists.txt`, `include/INA3221/INA3221.h`,
`.github/workflows/ci.yml`, `examples/esp_idf/basic/CMakeLists.txt`,
`examples/esp_idf/basic/main/CMakeLists.txt`,
`examples/esp_idf/basic/components/INA3221/CMakeLists.txt` and
`examples/esp_idf/basic/main/main.cpp`. Native component registration links
the actual driver source and example I2C adapter, requires native I2C/GPIO/time/
FreeRTOS components and exposes `app_main`. Its workflow runs real IDF 6.0.1
S2/S3 `set-target` and `build` commands. Adopt native linkage and explicit source
ownership; do not mistake these source declarations for executed firmware
evidence. Preserve TUSB32x's separate absolute build/SDKCONFIG paths instead
of copying the workflow's repeated in-tree target configuration. The owner's
explicit result APIs informed the runtime test, but its replaying initialize
semantics do not apply to TUSB32x read-only initialization.

## TUSB32x consumer boundary and local verification

Read current `CMakeLists.txt`, `scripts/generate_version.py`, all five
`include/TUSB32x/` public headers, `src/TUSB32xOperations.cpp`,
`examples/host/basic/CMakeLists.txt`, `examples/host/basic/main.cpp` and
`test/consumer/portable/CMakeLists.txt`. Also read the prompt index, COMMON,
prompt 32, AGENT_WORKFLOW, MAINTAINABILITY, applicable packaging rules/baseline,
current progress and design decisions. There is currently no checked-in
`.github/workflows/` directory in this checkout; no existing TUSB32x workflow
command was treated as present. Prompt 34 owns the remote matrix.

The new `test/package_consumer/` fixture has one required input:
`TUSB32X_PACKAGE_ROOT`, an absolute renamed extracted library directory.
It calls `add_subdirectory` and links only `TUSB32x`. It checks the target's
source directory, both public/private include directory properties and every
core source real path against that root. The generated build-local
`package-consumer-origin.txt` records those checked paths for the parent
package checker's evidence. No checkout/sibling path, discovery, private
production include, framework, generation script or test hook is embedded.

`main.cpp` directly includes all five public headers and binds explicit
simulated terminal callbacks. It proves bus-silent bind/admission, a zero-budget
poll, at most one callback per owner turn, exactly six ordered LAI identity/
configuration reads, no writes, exact-token delivery, wrong-token rejection,
take-once ownership, successful operation status with NONE hardware effect and
value snapshots retained across bus-silent teardown. Checks use executable
branches and remain active under `NDEBUG`. The simulated bytes and owner time
do not qualify hardware or adapter timing.

Provisional local check on 2026-10-28: CMake 4.0.1, GNU C++ 15.1.0 and Ninja;
configure/build/Release execution PASS, one registered consumer test passed.
Negative configuration checks also PASS: a missing package root and an explicit
relative existing root (`.`) both fail with the required-root diagnostic.
Commands were `cmake -S test/package_consumer -B build/package-consumer-provisional
-DTUSB32X_PACKAGE_ROOT=C:/Users/Honza/Documents/Projects/TUSB32x
-DCMAKE_BUILD_TYPE=Release`, then `cmake --build
build/package-consumer-provisional --config Release`, and `ctest --test-dir
build/package-consumer-provisional -C Release --output-on-failure`.
This source-checkout run checks the fixture only. The lead owns the real archive,
safe extraction, copied clean fixture, renamed extracted execution, firmware
matrix, final validation documentation and Git synchronization. Their evidence
must be recorded separately before claiming package completion.
