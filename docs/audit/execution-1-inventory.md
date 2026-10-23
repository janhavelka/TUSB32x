# Prompt 1: current ecosystem inventory

Audit date: 2026-10-23. This is a new execution snapshot; the original dated
reports remain unchanged. All 17 selected I2C libraries were clean on `main`.
No sibling fetch, modification, build, test or hardware action was performed.
The build columns identify checked-in definitions, not verified build support.

The [machine-readable inventory](execution-1-inventory.json) records full HEADs,
manifest authors/license, namespaces, language flags, source/layout paths, example
and test files, tools, environment names, dependency pins and export rules.
Indexed paths are a navigation inventory, not a claim of exhaustive line-by-line
review. The focused implementation/test observations below were read beside
declarations, AGENTS, manifests and build/workflow definitions.

## Selection and metadata

Enumerated 47 immediate directories in Projects: 17 independent I2C libraries
and 30 exclusions (including this repository and the separately audited consumer).
Do not count nested `.pio/libdeps`, archives or alternate worktrees as peers.
None of the selected roots uses a `.git` worktree pointer file.

All 17 manifests declare MIT, with LICENSE files present. Fifteen name Thymos
Solution s.r.o and Jan Havelka; RV3032-C7 and SSD1315 name Jan Havelka alone.
Keep those source-specific attributions: they do not license FieldCore source,
whose manifest says UNLICENSED. Preserve TUSB32x maintainer metadata through the
later packaging stage and do not invent vendor-source license rights.

| Library / version | HEAD | Namespace | Defined build entry points |
| --- | --- | --- | --- |
| TMP1x2 1.0.0 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `TMP1x2` | PlatformIO native + Arduino S2/S3; IDF component; standalone root CMake; native IDF example |
| 24Cxx 1.0.0 | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | `EEPROM24Cxx` | PlatformIO native + Arduino S2/S3; IDF component; standalone root CMake; native IDF example |
| INA228 3.0.4 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | `INA228` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| INA3221 3.2.0 | `e45622dbaa5bb0b3151d980f30a1b6ea1cdb1a1b` | `INA3221` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| ADS1115 2.0.2 | `381236ebfed813cd7a9aca34520ce031d8a0ff6b` | `ADS1115` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| OPT4001 1.2.2 | `dab818e575fc25a0949d962bc0df600eb2ed5200` | `OPT4001` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| LDC1614 3.2.0 | `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` | `LDC1614` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| BME280 2.2.0 | `d538f2e203ee26d6ddbc86f86d2b0cf4c73a6e98` | `BME280` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| SHT3x-main 1.9.0 | `f65f15573e3c9b1ef04cce72ad08f307d021e563` | `SHT3x` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| LSM6DS3TR 2.1.0 | `513c5641d9b7dda63fddc27267ea62aa8d03ff93` | `LSM6DS3TR` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| MB85RC 4.2.0 | `1bf7c293549b1038b2d8f3b7a8d655fa74201e6e` | `MB85RC` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| MCP45HVX1 2.0.0 | `d3eae4a01113d3badee1b6156571666bffe91ba2` | `MCP45HVX1` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| PCA9555 3.0.3 | `498178e4d9e4acb6fab7d8b816ad5021efe8806a` | `PCA9555` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| RV3032-C7 3.1.0 | `aabd8d9204e75072403fefb60cf8d2679a7f56de` | `RV3032` | PlatformIO native + Arduino S2/S3 |
| SCD41 1.3.2 | `c0bf1bba282d12b4aa3196d9f861915fe9b22551` | `SCD41` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| SSD1315 4.0.3 | `62561b163c6005900797c097fa92d5bf723c83b4` | `SSD1315` | PlatformIO native + Arduino S2/S3; IDF component; native IDF example |
| TCA9548A 1.1.0 | `ce425e1984ac269e7934dd811a06ec08b91e307c` | `TCA9548A` | PlatformIO native + Arduino S2/S3; IDF component; standalone root CMake; native IDF example |

C++17 is the chosen TUSB32x baseline, not a universal peer property. ADS1115
declares C++11 for its component/native tests and C++17 for Arduino/example code.
TCA9548A has a C++11 core smoke CMake project while its root target requests
C++17. MB85RC, MCP45HVX1, RV3032-C7 and SSD1315 show C++17 in PlatformIO flags;
this does not add a language-feature declaration to CMake where absent. Exact
positive and unflag evidence remains in the JSON; `-std=gnu++11` in INA228
`build_unflags` removes an old flag rather than selecting that standard.

## Focused implementation and test observations

Paths below are relative to Projects. Each row uses the full revision above.
These comparisons confirm the chosen TUSB32x contract; they do not import peer
silicon procedures or broaden a claimed family capability.

| Peer declaration / implementation / test | Finding and selected boundary |
| --- | --- |
| `TMP1x2/include/TMP1x2/TMP1x2.h`; `TMP1x2/src/TMP1x2.cpp`; `TMP1x2/test/test_core.cpp` | Split source by responsibility; bus-silent admission and bounded owner operations. Reuse fake-bus accepted-write fault cases. Reject copying unbind/result-clearing behavior or TIMEOUT classification into the stricter TUSB32x evidence contract. |
| `24Cxx/include/EEPROM24Cxx/EEPROM24Cxx.h`; `24Cxx/src/EEPROM24Cxx.cpp`; `24Cxx/test/test_core/test_core.cpp` | Bus-silent bind/end preserves write-pending evidence across end. Read-only probe/begin and EEPROM write-cycle geometry remain device-specific. Strong standalone package consumer reference. |
| `INA228/include/INA228/INA228.h`; `INA228/src/INA228.cpp`; `INA228/test/test_basic.cpp` | Separate cooperative jobs and compatibility synchronous APIs. Preserve backend error detail and bounded work; do not add compatibility facades, calibration or measurement machinery to TUSB32x. |
| `INA3221/include/INA3221/INA3221.h`; `INA3221/src/INA3221.cpp`; `INA3221/test/test_basic.cpp` | TransportConfig/DeviceProfile split and retained owner results are useful. JobTerminalState and DEADLINE_EXPIRED are peer vocabulary, not additional TUSB32x aliases. |
| `ADS1115/include/ADS1115/ADS1115.h`; `ADS1115/src/ADS1115.cpp`; `ADS1115/test/test_basic.cpp` | OperationToken and bind/startInitialize/poll tests demonstrate no-I2C admission. C++11 core is a useful portability reference, but TUSB32x explicitly chooses C++17. |
| `OPT4001/include/OPT4001/OPT4001.h`; `OPT4001/src/OPT4001.cpp`; `OPT4001/test/test_basic.cpp` | Useful contract/package guards; reject bind cancelling old work and end power-down writes. TUSB32x requires result/effect preservation and bus-silent teardown. |
| `LDC1614/include/LDC1614/LDC1614.h`; `LDC1614/src/LDC1614.cpp`; `LDC1614/test/test_basic.cpp` | Useful neutral transport counters, uint64 owner time and precise documentation. Reject result queue, end clearing counters/results and mandatory downstream Python version checking. |
| `BME280/include/BME280/BME280.h`; `BME280/src/BME280.cpp`; `BME280/test/test_basic.cpp` | Byte-register access and budget tests are useful; begin reads chip ID, and sensor reset/calibration/conversion procedures are unrelated. No copied automatic setup or sensor health policy. |
| `SHT3x-main/include/SHT3x/SHT3x.h`; `SHT3x-main/src/SHT3x.cpp`; `SHT3x-main/test/test_basic.cpp` | Shared CLI and owner polling are useful references. Synchronous begin attempts BREAK/reset recovery; TUSB32x initialization remains read-only. |
| `LSM6DS3TR/include/LSM6DS3TR/LSM6DS3TR.h`; `LSM6DS3TR/src/LSM6DS3TR.cpp`; `LSM6DS3TR/test/test_basic.cpp` | DriverConfig/DeviceProfile, retained result and separate configuration state align with the chosen baseline. Do not copy sensor settling/FIFO/measurement machinery. |
| `MB85RC/include/MB85RC/MB85RC.h`; `MB85RC/src/MB85RC.cpp`; `MB85RC/test/test_basic.cpp` | Busy/result retention and variant-specific addressing are useful comparisons. FRAM address pointer, memory geometry and ID mechanisms do not establish TUSB32x behavior. |
| `MCP45HVX1/include/MCP45HVX1/MCP45HVX1.h`; `MCP45HVX1/src/MCP45HVX1.cpp`; `MCP45HVX1/test/test_basic.cpp` | Registers, uncertainty and native CLI tests provide another concrete comparison. end clears runtime uncertainty/counters, so do not copy its evidence lifetime. |
| `PCA9555/include/PCA9555/PCA9555.h`; `PCA9555/src/PCA9555.cpp`; `PCA9555/test/test_basic.cpp` | Register image/validity and cooperative operations are useful byte-register references. bind sets READY immediately; TUSB32x remains UNINIT until verified initialization. Do not copy GPIO pair/shadow or cleanup procedures. |
| `RV3032-C7/include/RV3032/RV3032.h`; `RV3032-C7/src/RV3032.cpp`; `RV3032-C7/test/test_native/test_datetime.cpp` | Useful parser/package guards and separate HIL fixtures; RTC EEPROM queue, calendar and primary-cell timing are unrelated. No root native-IDF component metadata support is inferred from its manifest. |
| `SCD41/include/SCD41/SCD41.h`; `SCD41/src/SCD41.cpp`; `SCD41/test/test_basic.cpp` | Explicit operation/effect and reconciliation evidence are useful. end derives completion from last owner time; TUSB32x requires an invalid completion timestamp when no fresh sample exists. |
| `SSD1315/include/SSD1315/SSD1315.h`; `SSD1315/src/SSD1315.cpp`; `SSD1315/test/native/test_basic.cpp` | Post-callback owner time anchors panel waits; this supports conservative wait anchoring. Framebuffer/flush and synchronous compatibility flows are unrelated. |
| `TCA9548A/include/TCA9548A/TCA9548A.h`; `TCA9548A/src/TCA9548A.cpp`; `TCA9548A/test/test_driver/test_driver.cpp` | Small core, scripted transport and split host CLI/console/transport suites are useful. recover disables all mux channels, which must not become generic recovery for a USB-C controller. |

## Build, generated files and workflow decisions

Every selected peer has AGENTS.md, library.json, platformio.ini, a version
generator and checked-in workflow definitions. Inspect the exact paths listed
in JSON when implementing the corresponding stage. Most root CMake files are
IDF-only: use TMP1x2/24Cxx/TCA9548A for the dual-build boundary, not an
unconditional `idf_component_register` copied from a sensor.

- Use `library.json` as version truth and tracked `include/TUSB32x/Version.h`
  generated by sync/check/bump/set. TMP1x2 and 24Cxx root CMake require Python
  only inside their enabled test paths. LDC1614 root CMake invokes a Python
  version check during component consumption; reject that downstream dependency.
- TMP1x2 and 24Cxx `CMakeLists.txt` default tests OFF and register CTest suites.
  Other peers largely use PlatformIO Unity. Reuse behavior-focused fake transports
  and explicit registration; do not confuse source-file counts with executed tests.
- TMP1x2 and 24Cxx `.github/workflows/ci.yml` pin PlatformIO 6.1.19 and run
  strict-warning sanitizer/CTest and package checks. Several older workflows use
  moving action tags or unpinned native platforms; do not copy those as pinned.
- Most Arduino examples currently select PioArduino 55.03.311; SCD41 selects
  53.03.13 and some peers retain explicit older compatibility environments. Keep
  TUSB32x tool selection explicit and validate actual selected SDK behavior in
  adapter prompts. Presence of a matching URL proves no timeout guarantee.
- Export rules differ materially: LDC1614/BME280/SCD41 include vendor PDFs,
  and several packages include generation/development tools. TUSB32x keeps vendor
  artifacts outside production compilation and verifies an extracted consumer;
  it must not require those tools to compile/link the core.
- Native IDF directory names differ: `examples/esp_idf/basic`,
  `examples/idf/basic` and `examples/espidf_basic`. Adopt the TUSB32x baseline
  `examples/esp_idf/` paths, not aliases. RV3032-C7 lacks root CMake and exports
  no IDF manifest; Arduino portability is not a native-IDF build claim.
- Review workflow source in prompt 1; do not dispatch/poll remote CI. The initial
  TUSB32x promptset runs its remote checkpoint only in prompt 34.

## Consumer snapshot

FieldCore-node is a consumer, not an I2C library or a source dependency.
Current manifest version `1.3.0`, clean branch `rodtester`,
HEAD `4e6a05e769383b95046df3936eee81984f4d9aa7`. Manifest author is FieldCore;
license is UNLICENSED. The ownership audit reviews its actual integration
contracts separately. Keep its product policy and implementations out of the core.

## Excluded immediate directories

| Directory | Reason |
| --- | --- |
| `_archive` | Dated project archives, not current independent libraries. |
| `ADS1261_ESP32` | Non-I2C library; library.json and README identify its transport or service. |
| `AsyncSD` | Non-I2C library; library.json and README identify its transport or service. |
| `AT21CS11` | Non-I2C library; library.json and README identify its transport or service. |
| `CO2Control` | Firmware application; library.json description identifies the application. |
| `EasyEDA` | EDA project container. |
| `EE871-E2` | Non-I2C library; library.json and README identify its transport or service. |
| `EEPROM_24Cxx` | Empty directory; active EEPROM library is 24Cxx. |
| `ESP32_Interfaces` | Interface probe application (platformio.ini and src). |
| `ESP32_LM_mainboard` | Board firmware (.ino entry point), not an independent library. |
| `ESP32CloudflareWifi` | WiFi/Cloudflare proof application. |
| `ESS23-RS-p08` | Non-I2C library; library.json and README identify its transport or service. |
| `FieldCore-node` | Firmware consumer; separately audited, not an independent I2C library. |
| `KiCAD` | EDA project container. |
| `LGClimateLink` | Climate-link application; README and application tree. |
| `MAX31865` | Non-I2C library; library.json and README identify its transport or service. |
| `MotorControl-RS` | Non-I2C library; library.json and README identify its transport or service. |
| `NextionNX3224T028_UART` | UART display test application. |
| `PistonNode` | Firmware application; library.json description identifies the application. |
| `RadarNode` | Firmware application; library.json description identifies the application. |
| `SensorNodeFGI` | Sensor-node application (src and examples), not an independent library. |
| `SHZK-PT` | Non-I2C library; library.json and README identify its transport or service. |
| `SIM7080G-Core` | Non-I2C library; library.json and README identify its transport or service. |
| `StatusLED` | Non-I2C library; library.json and README identify its transport or service. |
| `SystemChrono` | Non-I2C library; library.json and README identify its transport or service. |
| `TLP300Node` | Firmware application; library.json description identifies the application. |
| `TunnelMonitor-Cloudflare` | Cloudflare backend. |
| `TUSB32x` | Current planning repository, not an implemented peer. |
| `VibWire-108` | Non-I2C library; library.json and README identify its transport or service. |
| `VTN4xx` | Non-I2C library; library.json and README identify its transport or service. |

`EEPROM_24Cxx` contains zero entries. `24Cxx` is the active EEPROM24Cxx library.
Hardware directories and application dependency trees were not recursively
counted as independent libraries. No unclassified immediate directory remains.

## Verification and limits

PASS: JSON parse; 17 unique peer records; full 40-character revisions; current
read-only Git status; existing referenced source/build/test/tool paths; all 47
immediate directories classified. Full text/link validation is recorded by the
lead in the prompt completion report.

NOT RUN: sibling builds/tests, remote CI, firmware links and physical hardware.
The local snapshots establish source evidence only. Later prompts re-audit the
relevant peers before implementing behavior; toolchain/adapter and TI gates
remain required at their assigned stages.
