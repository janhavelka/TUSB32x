# Prompt 11 current owner-engine peer audit

Read-only source review on 2026-10-26 (Europe/Prague). Peer-prefixed paths are
relative to `C:/Users/Honza/Documents/Projects/`; other paths are relative to
TUSB32x. No sibling was edited, fetched or built. No remote workflow was queried,
dispatched or awaited. This is source evidence, not a peer test result or silicon
qualification.

## Snapshot and instructions

| Repository | Full local HEAD | Branch | Initial worktree/index |
| --- | --- | --- | --- |
| TUSB32x | `fe1f7caf732b723734d1b87ee50252b09d6928fe` | `promptset/tusb32x-implementation-20261023` | Clean |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `main` | Clean |
| LSM6DS3TR | `362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc` | `main` | Clean |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `main` | Clean |
| INA3221 | `41b6184b75f3c152611d4236f7ea41a5ce77bc2c` | `main` | Clean |
| FieldCore-node | `4b97894f11ba129079262131c353dbfd9bb438fb` | `rodtester` | Clean |

Read each repository's HEAD, branch, status and staged-diff statistics locally.
These observations do not establish upstream equality; the lead owns TUSB32x
fetch/integration/commit/push. Reopened parent/local and all five peer AGENTS,
COMMON, prompt index, full `11-owner-engine.md`, AGENT_WORKFLOW and relevant
baseline/API lifecycle, admission, result, target-association and time contracts.
Peer framework separation, fixed storage and application bus ownership remain
useful. Their sensor procedures, aliases, teardown and replay rules do not
override the selected TUSB32x contract.

## Current admission, polling and result implementations

| Exact sources reopened | Current evidence and selected difference |
| --- | --- |
| `LDC1614/include/LDC1614/LDC1614.h` lifecycle/job/result declarations; `src/LDC1614.cpp:370`, `:445`, `:495`, `:547`, `:597`, `:615` | Admission uses caller nonzero OperationId and reserves a result FIFO slot. Poll clips a share of the remaining absolute deadline across its transfer ceiling. Cancel is bus-silent and returns OK; take returns delivery OK. Adopt bounded callback accounting, admission reservation and outcome/delivery separation. Reject caller IDs/FIFO, duplicate-ID vocabulary, multiple callbacks using one Status-only timestamp, resetting statistics/results in end/bind and zeroing take output before checking availability. TUSB32x owns nonreused tokens and one terminal slot. |
| `LDC1614/test/test_basic.cpp:1231`, `:1261`, `:1476`, `:1513`, `:1711`, `:1829` | Fixed fake cases cover cancellation at every initialization phase, deadline clipping/wrap, FIFO backpressure, immediate replacement after cancellation, partial writes and reset failures/no retry. Adopt literal fake traces, per-boundary cancellation and conservative write evidence. Reject immediate replacement while a TUSB result remains retained. LDC initialization deliberately replays a complete sensor profile; TUSB initialization must perform reads only. |
| `LSM6DS3TR/include/LSM6DS3TR/LSM6DS3TR.h`; `src/LSM6DS3TR.cpp:594`, `:635`, `:647`, `:2155`, `:2322`, `:2364` | Atomic bind validates before active/result guards. One active job and one tokened terminal result prevent replacement; take preserves output on rejection and returns OK on delivery. The uint64 token allocator has explicit exhaustion. Adopt these small state distinctions. Reject zeroing caller tokens on rejected start, STALE_RESULT aliases, untokened poll/cancel, ordinary increasing-time comparisons and destructive unbind. TUSB32x validates arguments first, supports half-range rollover and retains terminal/effect evidence through end. |
| `LSM6DS3TR/test/test_basic.cpp:389`, `:424`, `:439`, `:834`, `:870`, `:906`, `:1488` | Fake cases cover zero/one transfer budgets, exact-once take, wrong token, active/pending backpressure and cancellation before/after configuration/reset effects. The tests explicitly expect rejected start tokens to become invalid; that convention is rejected. TUSB fixtures must retain caller sentinels, test tokened poll and preserve old historical evidence independently of current usability. |
| `TMP1x2/include/TMP1x2/TMP1x2.h`; `src/TMP1x2.cpp:19`; `src/TMP1x2Operations.cpp:7`, `:53`, `:190`, `:239`, `:245`; `test/jobs/test_jobs.cpp:169` | Admission assigns a uint32 token only after validation; unavailable/wrong-token take preserves caller output, terminal poll does no traffic, and result delivery returns OK. Adopt output sentinels and explicit fresh wait anchoring when no clock hook exists. Reject uint32 token space, BUSY conflation of active/result, initialize/apply replay, destructive unbind before config validation, cancellation returning CANCELLED and post-callback timeout replacing a backend failure. TUSB32x requires immutable uint64 time, atomic invalid bind and primary transport failure/detail. |
| `INA3221/include/INA3221/INA3221.h`; `src/INA3221.cpp:533`, `:679`, `:1140`, `:1156`, `:1719`; `test/test_owner_operations.cpp:527`, `:629`, `:765`, `:912`, `:946` | Owner jobs use request IDs, explicit transfer budgets and one retained result. Fake writes can commit before an error; a timeout keeps indeterminate state and is never replayed. Take returns OK while carrying the failed job. Adopt conservative effect evidence and timeout/cancel cases at each transfer. Reject request-ID allocation by callers, legacy operation paths, cancellation returning CANCELLED and unbind erasing pending results/dirty state. No compatibility facade is added to TUSB32x. |

TUSB32x deliberately separates same-target association from current binding and
from result delivery. Association includes variant, address, context and both
callback functions, including when context is null. No inspected peer authorizes
losing unresolved old-target evidence on a different-target bind. Ordinary
read-only initialization cannot establish that a prior write sequence completed.

## FieldCore consumer and buffer ownership

Reopened `FieldCore-node/docs/repository_map.md`,
`docs/guidelines/runtime_connections.md`, `docs/guidelines/i2c_peripherals.md`,
`include/TunnelMonitor/i2c/I2cOwnerTransport.h`, `I2cBackend.h`, `I2cLifetime.h`,
`src/i2c/I2cTask.cpp`, `IdfI2cBackend.cpp`, `src/cli/CliPendingResults.cpp`,
`test/native/support/FakeI2cBackend.h` and
`test/native/test_owner_i2c/{suite_main.cpp,test_i2c_task_fake.cpp,test_i2c_contracts.cpp}`.

Transfer values borrow write/read buffers for one synchronous call. The owner
owns routing, scheduling, physical bus recovery and copied foreground results.
Deadline clipping precedes backend work. The owner result identity and protected
delivery lifetime are distinct from an inner device operation; its documented
6000 ms reclaim policy must not become a TUSB terminal-result timeout. Runtime
connections explicitly distinguish its retained storage example from I2C sensor
retention. Adopt exact copied results, no retained transfer pointers and no
deadline renewal. Reject importing FieldCore queues, DeviceId/Deadline64 types,
recovery/retry, PSRAM allocation or reclaim policy into the portable driver.
The fake retry/recovery tests prove application policy exists; they are not a
reason to add hidden driver recovery. No FieldCore dependency is introduced.

## Examples, CLI and Python runner assumptions

Inspected LDC `examples/common/Ldc1614Cli.{h,cpp}` accepted job tracking,
`takeResult` drain/cancel handling and `tools/ldc1614_hil_runner.py` operation and
CLI-session record patterns, with `tools/test_ldc1614_hil_runner.py` registration
evidence. Inspected LSM `examples/01_basic_bringup_cli/main.cpp`,
`examples/02_owner_soak/main.cpp`, `examples/idf/basic/README.md`,
`tools/check_cli_contract.py`, `run_hil.py` and `run_owner_soak.py` contracts.
The examples retain the accepted instance/token and drain a terminal result
before scheduling the next request. The LDC firmware also owns repeated sessions;
TUSB's later Python runners own those repeated sequences.

Inspected TMP `examples/common/Tmp1x2CliDiagnostics.cpp` start/result/cancel and
restoration paths and `tools/hil_tmp1x2_runner.py` token/kind correlation. Its
firmware diagnostic/restoration scheduler is not adopted. Inspected INA
`examples/01_basic_bringup_cli/main.cpp`, `examples/esp_idf/basic/main/main.cpp`,
their owner result draining, and `tools/hil_cli_runner.py` OWNER-013 through
OWNER-019 and legacy cancellation transfer checks. Adopt observable admission,
result outcome and bus-silent cancellation. Reject carrying their command
aliases, legacy scheduler or automatic profile restoration into this stage.

FieldCore `src/cli/CliPendingResults.cpp`, `scripts/hil_i2c_startup.py` and
`test/python/test_hil_i2c_startup.py` keep foreground delivery and hardware
evidence at the application layer. All such CLI/runner work is source precedent
only: TUSB prompt 11 publishes the owner engine, not a CLI, runner or framework
firmware.

## Registration and checked-in workflow review

Reopened all four driver root CMakeLists and platformio.ini, plus their
`.github/workflows/ci.yml`. LDC/LSM/INA register native Unity cases with RUN_TEST
and PlatformIO `test_build_src`; LDC includes its common CLI source. TMP CMake
registers jobs alongside core/CLI/console/variant/helper/bus and Python guards.
Reopened current TUSB root/test CMakeLists: tests default OFF, explicit sources,
strict warnings, active Release assertions and finite CTest timeouts already
exist. Register genuine `tusb32x_operations` behavior in the same mechanism;
compile-only future sketches do not count as runtime verification.

LDC source commands pin Core 6.1.19, review native/contract/CLI/runner/package/
documentation checks and native IDF v6.0.2. LSM and INA use Core 6.1.19 and
Arduino platform 55.03.311 with native tests and timing/CLI/IDF guards. TMP uses
Ubuntu 24.04 strict CMake/CTest with ASan/UBSan, Core 6.1.19 and IDF
v5.3.2/v5.5.1/v6.0.1. FieldCore `platformio.profiles.generated.ini` selects
`test_owner_i2c`; `.github/workflows/verify.yml` uses Python 3.12,
Core 6.1.19 and `scripts/verify.py`. Its documentation workflows use separate
reference and Doxygen checks. TUSB still has no checked-in workflow directory.
These are reviewed source definitions, not green-run evidence. No remote status
was requested and no peer suite was executed.

The lead's validation/progress records own actual TUSB commands and outcomes,
including protocol regressions, public linkage and documentation. This report
does not claim a build, firmware link, hardware transaction or physical pass.
