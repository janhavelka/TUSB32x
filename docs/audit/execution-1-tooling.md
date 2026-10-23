# Prompt 1 execution: examples, tooling and distribution audit

Audited 2026-10-23. This is a read-only inspection of current sibling source,
not a claim that their builds, tests or hardware passed. No sibling fetch,
source modification, build, remote CI query or hardware operation was performed.
Paths below are relative to the parent Projects directory; symbols identify the
actual implementations inspected beside their declarations and tests.

## Revisions and metadata

All five checkouts were on `main` with empty `git status --short` when inspected.
No uncommitted source was used. Their AGENTS.md files were reread; their general
commit/check discipline is useful, but TUSB32x's single execution branch and
remote-CI-only-at-prompt-34 rules control this execution.

| Peer | Full HEAD | Manifest version | License |
| --- | --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | 1.0.0 | MIT |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | 1.0.0 | MIT |
| SHT3x-main | `f65f15573e3c9b1ef04cce72ad08f307d021e563` | 1.9.0 | MIT |
| LDC1614 | `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` | 3.2.0 | MIT |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | 3.0.4 | MIT |

All five `library.json` files name Thymos Solution s.r.o and maintainer Jan
Havelka; none lists a runtime library dependency. Preserve this evidenced
metadata rather than inventing an author. Vendor reference terms remain separate.
TMP1x2/24Cxx/SHT3x omit platform/framework restrictions, LDC1614 uses `*`, and
INA228 restricts to Arduino/ESP-IDF on espressif32. Choose unrestricted metadata
for the standard C++ core; list tested framework combinations separately.

## Concrete choices

Revision labels in this table resolve to the full clean revisions above.
Checks are future acceptance checks unless explicitly identified as source review.

| Concern | Peer file/symbol | Revision | Selected rule | Reason | Check |
| --- | --- | --- | --- | --- | --- |
| Shared CLI | TMP1x2 `examples/common/Tmp1x2Cli.cpp` (`Cli::processCommand`, `Cli::feed`) and SHT3x-main `examples/common/Sht3xCli.cpp` (`processCommandString`) | TMP1x2, SHT3x-main above | One portable shared dispatcher, thin framework front ends; ordinary standard headers in common code | Both current shared implementations use standard C++ includes; sharing a folder alone is insufficient proof | Compile actual common sources in host CLI tests and both firmware builds |
| Help | TMP1x2 `examples/common/Tmp1x2CliOutput.cpp` (`Cli::printHelp`) and SHT3x-main `examples/common/Sht3xCli.cpp:1993` (`printHelpHeader`, `printHelpSection`, `printHelpItem`) | TMP1x2, SHT3x-main | Cyan title/syntax, green groups, 32-character minimum syntax field; cached/live/staged/write labels | Familiar terminal layout without copying sensor commands | Host help/alias/topic/color-off checks; incremental output remains responsive |
| Strict parsing | TMP1x2 `examples/common/Tmp1x2Cli.cpp:14` (`integer`, `boolean`, `Cli::feed`) and 24Cxx `test/test_cli/test_cli.cpp:33` | TMP1x2, 24Cxx | Fixed buffers; explicit decimal or byte hex; reject signs, overflow, suffixes, excess args; poison malformed line through newline | TMP decimal/hex selection avoids base-zero octal; 24Cxx tests verify rejection causes no callbacks | Actual CLI tests for overflow, backspace after poison, CRLF, zero callback deltas |
| Simplicity | SHT3x-main `examples/common/Sht3xCli.cpp` (`CliString`, `requireConfirmation`, `performMeasurementMilliBlocking`, `runStress`) and TMP1x2 `examples/common/Tmp1x2CliDiagnostics.cpp` | SHT3x-main, TMP1x2 | Do not copy general string wrapper, confirmation state machine, blocking stress loops or a second diagnostic scheduler/result cache | TUSB already has one operation engine; host runners can repeat ordinary commands | One poll call site per turn; retained result never implicitly consumed; no firmware bulk scheduler |
| Console bounds | TMP1x2 `examples/common/ConsoleBuffer.h` (`ConsoleBuffer::format`, `drain`, `lost`) and 24Cxx `examples/common/ConsoleOutput.h`; TMP1x2 `test/console/test_console.cpp` | TMP1x2, 24Cxx | Fixed queue, whole-record drops, saturating loss counters, one bounded nonblocking drain | TMP implementation drops oversized 512-byte formatted messages and caps drain at 256 bytes; its default queue is 16384, not the TUSB choice | TUSB starts at 8192 queue/512 record/256 drain; backpressure, record maxima and reservation-before-take tests |
| Arduino boundary | TMP1x2 `examples/01_basic_bringup_cli/main.cpp` (`writeI2c`, `readI2c`, `mapWire`) and 24Cxx `examples/common/WireTransportHelpers.h` | TMP1x2, 24Cxx | Application owns Wire, pins, clocks and lock; callback counts are attempts; no guessed NACK phase on short read | TMP maps precise endTransmission codes and leaves short-read cause generic | Audit pinned Wire locks and aggregate timing before adopting adapter; mocked phases alone do not prove SDK bound |
| Native IDF boundary | TMP1x2 `examples/esp_idf/basic/main/main.cpp` (`deviceHandle`, `readI2c`) and INA228 `examples/esp_idf/basic/main/Ina228IdfI2cTransport.cpp` (`validateContext`, `ina228IdfSelectDeviceAddress`, `ina228IdfI2cWriteRead`) | TMP1x2, INA228 | Borrow a synchronous device handle and verify configured address; owner changes handle while ended/idle; no Arduino facade | IDF device handle owns address. Selection/creation belongs to application, not driver core | Reject mismatch/async configuration before SDK; all bus users serialized; audit lock/wait/cleanup total, not only timeout argument |
| Pins | TMP1x2 `examples/common/BoardConfig.h` and 24Cxx `examples/common/BoardConfig.h` | TMP1x2, 24Cxx | Example-only overridable pins; IRQ input opt-in; validate variant/address and pin conflicts | TMP defaults SDA8/SCL9, 400 kHz, timeout50 ms, IRQ disabled; these are board examples, not chip facts | TUSB-specific strap/address/capability checks; board wiring documented; no pin defaults in core |
| Host runner contract | LDC1614 `tools/ldc1614_cli_contract.py` (`CommandSpec`, `COMMAND_SPECS`) and INA228 `tools/run_i2c_hil.py` (`version_contract_errors`, `strip_hilrun_frame`) | LDC1614, INA228 | One host grammar/schema, fresh nonce/request/token correlation, firmware provenance and explicit NOT RUN | LDC distinguishes execution/fixture classes; INA verifies flashed commit/version and searches end marker after begin | Reject stale frames, duplicate keys, resets, missing required identity and mismatched origin; host checkout never proves flashed image |
| Runner limits | LDC1614 `tools/ldc1614_hil_runner.py` (`MAX_RESPONSE_BYTES`, `MAX_TRANSCRIPT_BYTES`) and INA228 `tools/run_i2c_hil.py` (`read_until_hilrun_end`, `run_step`) | LDC1614, INA228 | Finite capture/whole-run deadlines; retain failure evidence; callbacks not bus bytes; restore only owned local settings | Both expose bounds/framing but their human transcript protocols are not TUSB's schema | Fake-serial failure tests and Python-to-real-C++ transcript test; no foreign cancellation/take |
| Portable build | TMP1x2 `CMakeLists.txt` and 24Cxx `CMakeLists.txt` | TMP1x2, 24Cxx | Dual ordinary CMake/IDF registration, C++17, tests OFF, Python found only for enabled development checks | Both have real plain-CMake library paths; reject LDC1614 top-level unconditional Python REQUIRED and INA228 IDF-only root CMake as general consumer templates | Extracted renamed consumer compiles/links without Git/Python/Doxygen/framework headers or siblings |
| Actual test registration | TMP1x2 `CMakeLists.txt:18` and 24Cxx `CMakeLists.txt:10` | TMP1x2, 24Cxx | Register host core/CLI/console/runner/consumer suites; assertions active in Release | TMP registers core/CLI/console/jobs/variants/helpers/bus/core_no_arduino and Python guards; 24 registers CLI transcript against real executable | CTest discovery must contain expected nonzero suites; deliberate failures detected; do not treat source grep as execution |
| Version truth | TMP1x2 and LDC1614 `scripts/generate_version.py` (`_expected_outputs`, `main`) plus their AGENTS version sections | TMP1x2, LDC1614 | `library.json` source; sync/check/bump/set; generated header committed; development check outside downstream configure | Generators propagate header/component/Doxygen version; runtime build identity separate from release version | Missing/stale generated output fails development check; clean consumer needs no generator; no per-prompt release bump |
| Packages | TMP1x2 `tools/check_package.py` (`main`, `forbidden`) and 24Cxx `tools/check_package_contents.py` (`main`, `REQUIRED`) | TMP1x2, 24Cxx | Validate actual archive, expected production files, documented excluded references, then isolated renamed consumer | TMP rejects absolute/drive/colon/backslash/traversal/nonregular/duplicate entries and builds extracted consumer; 24 checks packaged Markdown links | Add case-insensitive collisions, reserved Windows names and trailing-dot/space aliases; no vendor/test/build payload leakage |
| Documentation | LDC1614/INA228 `Doxyfile`; TMP1x2 `tools/check_doxygen.py` (`check_html`, `check_api`) and `tools/test_check_doxygen.py` | LDC1614, INA228, TMP1x2 | EXTRACT_ALL=NO, undocumented/parameter/doc warnings fail; public visibility and HTML links additionally checked | TMP currently has EXTRACT_ALL=YES: useful visibility checker, unsuitable missing-comment policy. LDC/INA use NO | Remove public comment/type/link in negative fixtures and prove failure; do not expose private implementation merely to satisfy docs |
| Style | LDC1614 and INA228 `.clang-format`, AGENTS naming sections | LDC1614, INA228 | Two-space Google-derived format, attached braces, 100 columns, no short if/loops; PascalCase types/camelCase functions/private underscore/CAPS constants | Consistent readable source without transplanting framework-first include rules into the core | Format future source with chosen supported clang-format; clear Doxygen ownership/units/bounds |

## Checked-in workflow definitions

Read `.github/workflows/ci.yml` in TMP1x2, 24Cxx, LDC1614 and INA228.
These are source definitions, not observed successful jobs.

- TMP1x2 and 24Cxx use Ubuntu 24.04, SHA-pinned checkout
  `9c091bb21b7c1c1d1991bb908d89e4e9dddfe3e0`, PlatformIO 6.1.19,
  CMake/CTest with warnings and address/undefined sanitizers, S2/S3 Arduino,
  and native IDF v5.3.2/v5.5.1/v6.0.1 on esp32s2/esp32s3.
- LDC1614 uses PlatformIO requirements, native PlatformIO tests, timing/CLI/IDF/
  readiness/hygiene/version/clean-consumer guards, Doxygen, and IDF v6.0.2.
  INA228 uses PlatformIO 6.1.19, native tests, parser regression/dry-run checks,
  Doxygen, package contents plus source compilation, and IDF v6.0.1.
- Choose the TUSB local checks and firmware matrix already specified in prompts;
  inspect supported tool inputs again at implementation. Peer matrix presence
  does not establish this library's compatibility. Native-IDF target builds must
  isolate both build directories and SDKCONFIG, including S2/S3/S2 repetition.
  The peer `set-target` command in a fresh CI checkout is not proof of isolation
  for reused local directories.
- Keep remote CI inspection/execution at prompt 34. Do not dispatch or poll now.
  LDC1614 `CONTRIBUTING.md` points to `docs/RELEASING.md` for the exact
  checked-commit/annotated-tag sequence. TMP1x2 has no `CONTRIBUTING.md`; do
  not invent that source or copy a missing release checklist. Use the evidenced
  LDC1614 release ordering, but publishing
  releases/tags is outside prompt 1 and ordinary prompt commits are not releases.

## Limits and handoff

All named source paths above exist in the inspected checkouts. No selected
convention requires a framework dependency in the core. Current shared CLI code
has standard includes, but the new implementation must still prove its own
host link and behavior. SDK aggregate blocking limits, exact native-IDF version
support and actual board pins remain implementation/board qualification checks;
this source audit cannot certify them. No firmware, driver test, serial fixture
or silicon behavior was executed in prompt 1. The chosen contract remains
[ENGINEERING_BASELINE](../prompts/ENGINEERING_BASELINE.md) with
[CLI_CONTRACT](../prompts/CLI_CONTRACT.md), including its stronger result
ownership, evidence and bounded-work rules.
