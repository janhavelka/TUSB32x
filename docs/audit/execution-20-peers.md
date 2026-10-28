# Prompt 20 current peer audit

Read-only source audit on 2026-10-28 for the local CLI foundation. The TUSB32x
starting HEAD was `c07e4e5473ffd5719b81f0451c79f3b15fa41532`, clean on
`promptset/tusb32x-implementation-20261023`; the lead owns synchronization and
all Git mutations. This audit changes no sibling and runs no peer builds,
framework builds or remote CI. Paths below are relative to
`C:/Users/Honza/Documents/Projects/`.

The parent and local AGENTS, prompt 20, COMMON, AGENT_WORKFLOW, MAINTAINABILITY,
CLI_CONTRACT, current progress and design decisions were read. Current core
`include/TUSB32x/Config.h`, `include/TUSB32x/TUSB32x.h`,
`src/TUSB32xOperations.cpp`, `CMakeLists.txt` and `test/CMakeLists.txt` establish
the existing validated profile, bus-silent binding/evidence lifecycle and test
registration. No `.github/workflows/` or `examples/common/` existed at the
initial inspection. Missing current workflows are recorded, not assumed present.

## Revisions and exact source paths

All four `git status --short` outputs were empty at inspection. Each peer's
current `AGENTS.md`, `CMakeLists.txt` and `.github/workflows/ci.yml` was read.
SHT3x's actual directory is `SHT3x-main`, not an inferred `SHT3x` path.

| Peer | Current HEAD | Additional sources read |
| --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `examples/common/Tmp1x2Cli.h`, `Tmp1x2Cli.cpp`, `Tmp1x2CliOutput.cpp`, `ConsoleBuffer.h`; `test/cli/test_cli.cpp`; `test/console/test_console.cpp`; `tools/hil_tmp1x2_runner.py` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | `examples/common/Eeprom24CxxCli.h`, `Eeprom24CxxCli.cpp`, `ConsoleOutput.h`; `test/test_cli/test_cli.cpp`; `test/test_console/test_console.cpp`; `tools/test_cli_transcript.py`; `tools/hil_runner.py` |
| SHT3x-main | `f65f15573e3c9b1ef04cce72ad08f307d021e563` | `examples/common/Sht3xCli.h`, `Sht3xCli.cpp`; `tools/sht3x_cli_contract.py`; `tools/test_cli_health_rate.py`; `tools/run_sht3x_hil.py` |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `examples/common/Ldc1614Cli.h`, `Ldc1614Cli.cpp`; `test/test_cli.cpp`; `tools/ldc1614_cli_contract.py`; `tools/ldc1614_hil_runner.py` |

LDC1614 has advanced from the older `c0ab4c1` recorded in CLI_CONTRACT's source
table. Decisions here use the freshly inspected `1db38b6` source. The table in
CLI_CONTRACT remains historical specification provenance.

## Adopted and rejected conventions

- TMP1x2 provides a compact fixed queue, complete formatted-message rejection,
  saturating loss evidence and a single short-write-aware drain. Adopt these
  mechanisms with TUSB32x's 8192/512/256 bounds and dropped-record count. Its
  default queue is 16384 bytes, its direct parser uses `strlen`, its input loop
  does not enforce TUSB32x's per-turn limit, and setup starts initialize. Do not
  copy these differences or its sensor-specific diagnostic scheduler.
- 24Cxx directly rejects control/high-bit bytes before tokenization; both serial
  and direct-input tests protect against malformed writes. Adopt bounded whole
  input validation and the actual C++ fixture plus Python transcript path.
  Reject its startup probe, firmware watch/stress/scratch scheduler, larger
  argument count, implicit result handling and output-ready pause of new device
  transactions. TUSB32x output congestion must not stall owner work or teardown.
- SHT3x separates arity validation from dispatch and documents host command rows.
  Adopt explicit command classification and exact arity checks. Do not import
  its `CliString` compatibility layer, file-scope mutable owner state, generic
  bus-wide reset, confirmation protocol or measurement operations. Its dispatch
  can clear a reset-arm flag before arity rejection; TUSB32x rejects a malformed
  command before command-owned state changes.
- LDC1614 has explicit C++/Python command metadata and tests proving staged
  setters have zero transfers. Adopt direct rows, grouped presentation and
  transactional candidate edits. Reject its separate profile commit and init
  replay semantics, `confirm` vocabulary, and deliberately cross-field-invalid
  staging (`sensorbounds 0 5000000 100000` followed by validation failure).
  TUSB32x validates the entire candidate before replacing staging or generations.
- Peer workflows register actual native tests and strict warning/sanitizer
  commands. TMP1x2 and 24Cxx keep tests optional and register real CLI sources;
  24Cxx passes `$<TARGET_FILE:eeprom24cxx_cli_host>` into Python. Adopt this
  registration pattern with a finite TUSB32x timeout. SHT3x and LDC1614 native
  tests use PlatformIO; their component CMake and maintainer dependencies must
  not replace TUSB32x's tool-independent tests-OFF C++17 consumer path.

## Negative and backpressure cases inspected

TMP1x2 `test/cli/test_cli.cpp` constructs malformed raw writes with NUL, 0x01,
0x18 and ESC inserted inside tokens, verifies unchanged transfers, and checks
the following valid/backspace-edited command remains usable. Its console suite
tests zero acceptance, partial writes, ring wrap, full-message loss, a loss
marker after recovery, oversized formatting and a 256-byte single-call drain.

24Cxx `test/test_cli/test_cli.cpp` rejects extra tokens, signed/overflow values,
an overlong input line and direct vertical-tab/form-feed/CR/LF/high-bit bytes
without transfers. Its console suite retains exact ordering through 7-byte
writes and zero acceptance, verifies format loss and capacity loss, and tests
deadline progress while output readiness denies transactions. The last policy
is evidence to reject, not a TUSB32x requirement.

LDC1614 `test_cli_complete_staged_profile_surface_is_silent_and_transactional`
checks the real staging surface and rejected mode syntax; its explicit invalid
sensor-bounds staging illustrates why TUSB32x needs stronger precommit validation.

These cases were read, not executed in the sibling projects. TUSB32x behavior
and framing require its own registered execution tests; peer passing claims are
not transferred to this repository.
