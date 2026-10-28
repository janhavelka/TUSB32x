# Prompt 25 peer re-audit

Inspected on 2026-10-28. Siblings were read-only. `git status --short` was empty
for each repository at the revisions below. No remote CI status was requested.

| Peer | Revision | Relevant current sources |
| --- | --- | --- |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `AGENTS.md`, root `CMakeLists.txt` and `idf_component.yml`, `examples/esp32/I2cMasterTransport.{h,cpp}`, `examples/esp_idf/basic/{CMakeLists.txt,README.md,main/CMakeLists.txt,main/main.cpp}`, `test/test_basic.cpp`, `tools/{check_idf_example_contract.py,ldc1614_hil_runner.py}`, `.github/workflows/ci.yml` |
| MB85RC | `1bf7c293549b1038b2d8f3b7a8d655fa74201e6e` | `AGENTS.md`, root `CMakeLists.txt` and `idf_component.yml`, `examples/common/IdfI2cTransport.h`, `examples/espidf_basic/{CMakeLists.txt,main/CMakeLists.txt,main/main.cpp}`, `test/test_public_contracts.cpp`, `tools/{check_idf_example_contract.py,hil_runner.py}`, `.github/workflows/ci.yml` |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `AGENTS.md`, root `CMakeLists.txt` and `idf_component.yml`, `examples/esp_idf/basic/{CMakeLists.txt,platformio.ini,sdkconfig.defaults,sdkconfig.defaults.esp32s2,sdkconfig.defaults.esp32s3,main/CMakeLists.txt,main/main.cpp}`, `tools/{test_idf_component_contract.py,hil_tmp1x2_runner.py}`, `.github/workflows/ci.yml` |

Adopted the peers' native `app_main`, application-side SDK dependencies and root
component registration. Adopted TMP1x2's relocatable `EXTRA_COMPONENT_DIRS` and
dynamic checkout-directory component name, including renamed archive consumers.
The new minimal consumer compiles its own main, the actual adapter and root core;
it does not compile a CLI frontend. Target environments have distinct build and
generated SDK configuration paths. Common defaults select 1000 Hz scheduling;
target defaults explicitly select UART console without depending on USB wiring.

Rejected LDC1614's transport-owned bus/device creation, recovery and speed edits:
TUSB32x's adapter borrows application handles. Rejected copying full timeout
arguments directly into native transactions: LDC1614 `write`/`writeRead`, MB85RC
`timeoutArg`/native dispatch and TMP1x2 `writeI2c`/`writeReadI2c` do not establish
one aggregate bound through SDK internal lock and command waits. LDC1614's coarse
transaction-error treatment is a useful precedent: an SDK error without byte/phase
evidence cannot establish which NACK occurred or that a write had no effect.

The peer Python contract checks are useful build/source boundary checks; they are
not linked SDK or physical timing evidence. TMP1x2's renamed-component CMake test
is a useful consumer precedent. Peer HIL runners preserve explicit missing-hardware
outcomes and firmware identity; this adapter-only fixture has no HIL runner or
flashing behavior. No peer scheduler or retained-result policy was copied.

Workflow source review found S2/S3 native builds on LDC1614 IDF 6.0.2, MB85RC
6.0.1/6.0.2, and TMP1x2 5.3.2/5.5.1/6.0.1. Those are peer coverage claims only.
This prompt selects installed IDF 5.5.5 (`framework-espidf` package `3.50505`,
`tools/cmake/version.cmake` major/minor/patch), with PlatformIO Core 6.1.19 and
pinned pioarduino `55.03.311`; other native SDK revisions remain unqualified here.
The SDK source/timeout audit and adapter contract are recorded separately by the
adapter owner. No sibling was changed and no remote workflow run was inspected.

The real native fixture uses the SDK warning policy for its adapter/main component.
An attempted strict `main` target build failed on the SDK's Newlib `#include_next`
wrappers and NVS conversions. The pinned PlatformIO builder's `get_app_flags` in
`builder/frameworks/espidf.py` globalizes main compile-group flags by language,
including flags assigned per source. The extra target flags were removed; no SDK
sources, warnings or assertions were changed. Strict host builds still compile
the actual adapter, and the core component retains its own strict target flags.
SDK `tools/cmake/kconfig.cmake` appends each existing
`sdkconfig.defaults.<target>` alongside the common defaults; initial real configure
output explicitly loaded both S2 and S3 target files in their isolated configs.
