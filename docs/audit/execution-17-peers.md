# Prompt 17 current diagnostics and health peer review

Read-only source inspection on 2026-10-26. Reopened current public declarations,
implementation, examples/CLI, Python runners, test registration, build definitions
and AGENTS for the five requested peers. No sibling was edited, fetched, built or
tested. Workflow review means checked-in source inspection; no remote CI was
dispatched, polled or required. This report supplies design evidence, not test PASS.

## Starting state and instructions

TUSB32x started at `df0fc2253cb01cfcfdb0973d14791b7d345f5a95` on the existing
`promptset/tusb32x-implementation-20261023` branch, with clean worktree and index.
The lead had fetched and verified upstream equality before delegation. The audit
agent performed read-only Git inspection and owns only this report; the lead owns
integration, commits and synchronization. No branch or worktree was created.

Re-read parent/local AGENTS, prompt index, COMMON, complete
`17-diagnostics-health.md`, AGENT_WORKFLOW, MAINTAINABILITY, baseline Sync/access/
accounting/health sections, progress and design decisions. Each sibling AGENTS was
reopened. Their shared guidance keeps transport/application ownership, bounded
callbacks, static Status messages, retained uncertain effects and hardware
qualification separate. Their CI policies do not override TUSB's explicit
prompt-34-only remote CI exception.

| Peer | Branch | Current HEAD | Index/worktree |
| --- | --- | --- | --- |
| LDC1614 | main | 1db38b63cb93a561b6e0bea51017ab94102ebd24 | Clean |
| LSM6DS3TR | main | 362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc | Clean |
| TMP1x2 | main | bafc34067530eb340e55c42dae3fad4eb795b6e9 | Clean |
| ADS1115 | main | 235af3e8c722661a89a44766a4ccd903a654f6fe | Clean |
| INA3221 | main | 41b6184b75f3c152611d4236f7ea41a5ce77bc2c | Clean |

States were freshly checked with `rev-parse HEAD`, `branch --show-current`,
`status --porcelain=v1` and `diff --cached --name-status`. Historical baseline
revisions were navigation aids, not substitutes for these checks.

## Counter units and adopted boundaries

| Current peer source | Actual counted unit | TUSB decision |
| --- | --- | --- |
| LDC1614 `include/LDC1614/LDC1614.h::TransportStats`; `src/LDC1614.cpp::_readRegister/_writeRegister/_tracked` | One returned I2C callback outcome, including direct diagnostics and engine work. Three uint32 counters saturate; latest Status replaces prior status. `_tracked` increments attempts after callback return. | Adopt neutral names and saturation. TUSB's existing sole `invoke` boundary increments attempts before invoking and normalizes terminal callback codes. Retain last failure separately from latest status. Do not copy lifecycle clearing or describe a combined callback as a known physical transaction count. |
| LSM6DS3TR `include/LSM6DS3TR/LSM6DS3TR.h::DriverDiagnostics`; `src/LSM6DS3TR.cpp::_read/_writeByte`, `diagnosticReadBlock/diagnosticWriteRegister` | Saturating successes/failures include both production jobs and advanced diagnostics because both use the same wrappers. Last failed Status/time persists after success. Diagnostic work sets a one-callback limit. | Adopt shared transport accounting, result backpressure and retained last failure. Add no parallel transport path. TUSB's `managedTransport` must explicitly filter raw Sync rather than treat these all-source counters as managed health. |
| TMP1x2 `include/TMP1x2/TMP1x2.h::DriverState/HealthSnapshot`; `src/TMP1x2.cpp::track/read/write/probe/readRegisterRaw/writeRegisterRaw` | Tracked callback successes and a selected `healthFailure` error set; raw/probe call with `tracked=false`. uint32 totals and uint8 streak saturate. Clock-valid flags qualify timestamps. | Adopt the four state names, explicit raw exclusion from managed evidence and time-validity concept. Retain separate all-callback source totals here; raw exclusion must not erase traffic. Do not copy its filtered error list, uint8 streak, embedded Config threshold or bind-time reset of history. |
| ADS1115 `src/ADS1115.cpp::_i2cWriteReadRaw/_i2cWriteRaw/_i2cWriteReadTracked/_i2cWriteTracked/_updateHealth`, `readRegister16/writeRegister16/_probeRaw` | Tracked callbacks, including direct register diagnostics, contribute to health; public probe bypasses it. Wrappers exclude INVALID_CONFIG/INVALID_PARAM status values. uint32 totals and uint8 streak saturate. | Adopt passive transitions and successful-callback streak reset. Reject classifying by method spelling or excluding an invoked callback because it returned an owner/validation code: TUSB normalizes those codes to I2C_ERROR and counts them. TUSB raw Sync is deliberately outside managed health. |
| INA3221 `src/INA3221.cpp::_updateHealth/_recordFailure/_pollProfileJob`, `readRegister16/writeRegister16/probe` | Mostly tracked callback outcomes, including direct register access, with raw probe excluded. In addition, successful manufacturer/die reads followed by identity mismatch call `_recordFailure` (lines 1317/1335), increasing failure totals/streak without a failed callback. | Adopt passive health and preserved write effects. Reject this synthetic failure unit for TUSB transport statistics: identity mismatch is separate protocol evidence, while the successful callback counts once as success. |

For TUSB, source totals count every invoked terminal callback once: raw Sync,
initialize, reset, recovery and IRQ ACK/service included. Managed evidence is the
explicit production subset, excluding raw Sync. Callback I2C_BUSY is a failure;
local BUSY, invalid inputs, waits, detach, mismatch, cancellation and callback-free
deadline expiry do not add transport failures. Saturated independent totals need
not satisfy attempts = successes + failures.

The existing TUSB `src/TUSB32xProtocol.cpp::invoke` already owns normalization,
attempt accounting, managed filtering and last failed Status. Extend that path,
not a second wrapper. Basic callbacks return Status only; neither their input
deadline nor a later poll proves callback completion time. Keep callback-time
validity false, including raw failures with no owner-time argument. LSM's supplied
call time and sensor peers' platform clocks are not equivalent evidence.

## Raw access and retained effects

LSM `src/LSM6DS3TR.cpp::diagnosticReadBlock/diagnosticWriteRegister` rejects active
and pending-result ownership, validates bounded access and uses the normal
wrapper. Its write invalidates configuration before invocation. Adopt these
boundaries and one-callback admission. Its block-read buffer is passed directly
to the callback; TUSB's scalar read must instead publish only on success. LSM's
register masks/bank/FIFO behavior is not TUSB silicon evidence.

TMP `src/TMP1x2.cpp::writeRegisterRaw/markDirty` retains configuration dirtiness
after raw writes independently of health. ADS `src/ADS1115.cpp::writeRegister16`,
`_writeRegister16Tracked` and `_requireConversionReconciliation` retain possible
conversion effects after an uncertain write. INA `src/INA3221.cpp::writeRegister16`,
`_handleResetWriteEffect` and `_markRegisterUnknown` distinguish possible reset
acceptance from ordinary settings uncertainty. Adopt uncertainty surviving later
successful reads, result delivery and source-stat clearing. Reject copying their
sensor reset/format/conversion procedures or allowing a TUSB reset/role/recovery
write through an unrestricted scalar diagnostic API.

TUSB mixed RO/RW/W1C composition must use existing proved actual controls or reject
without traffic when a safe image is missing. One callback cannot first read and
then write. Raw reads cannot establish managed identity or certify a historical
recovery sequence. TUSB322I unresolved access stays gated; sensor clear-on-read
and reset patterns supply no authority for it.

## Passive health and test evidence inspected

TMP's `test/test_core.cpp::health` exercises probe exclusion, failures to OFFLINE,
retained error detail and success to READY; its named-case table invokes `health`.
LSM's `test/test_basic.cpp::test_diagnostic_bounds_and_raw_write_invalidation_are_explicit`,
`test_diagnostic_sync_writes_invalidate_and_reconcile_exact_addresses` and
`test_ambiguous_diagnostic_write_invalidates_without_hidden_refresh` are registered
with RUN_TEST. Its diagnostics test explicitly expects five raw writes to add five
transport successes. LDC's registered
`test/test_basic.cpp::test_transport_stats_are_diagnostic_only_and_failures_never_suppress_requests`
checks repeated failures followed by another allowed callback.

ADS's registered `test/test_basic.cpp` cases include
`test_probe_failure_does_not_update_health`,
`test_tracked_failure_reaches_passive_offline_diagnostic_threshold`,
`test_passive_offline_diagnostic_does_not_gate_authorized_read`,
`test_no_clock_health_timestamps_are_marked_unavailable` and
`test_owner_safe_health_is_passive_and_recovery_transport_is_not_offline_gated`.
INA's registered `test/test_basic.cpp` covers probe exclusion, identity mismatch
accounting, offline requests and bus-silent end. Its
`test/test_owner_operations.cpp::test_raw_reset_updates_both_profile_certainty_families`
and `test_owner_health_diagnostics_never_suppress_owner_requested_io` are registered
by `runOwnerOperationTests`, called from the basic suite.

Adopt behavioral tests for continued explicit work while OFFLINE, success clearing
the managed streak and separate trust/effect evidence. TUSB health is a pure
projection of copied managed evidence with default threshold 3 and any positive
uint32 threshold. Invalid policy leaves output unchanged. UNINIT takes precedence
when initialization proof is absent, even if historical failures remain. READY
can coexist with UNKNOWN configuration; an unrelated success repairs no trust.
`clearTransportStats` clears only source totals/latest status, leaving health,
last failure, age, effects and retained result intact. Do not copy peer private
state seeding as a public test API or stateful recovery/admission policy.

## CLI, runner and workflow source inspection

LDC `examples/common/Ldc1614Cli.cpp::printDriver/printDiagnostics` prints core
attempt/success/failure values; `transferStatsSinceBaseline` separately labels
adapter writes/writeReads/discoveries. `tools/ldc1614_hil_runner.py` parses core
transport fields and target firmware Git/clean markers. LSM
`examples/01_basic_bringup_cli/main.cpp` prints passive diagnostic Status/detail
and session counter deltas; `tools/run_hil.py` checks diagnostics and raw-write
invalidation. These are source inspections, not executed CLI/HIL evidence.

TMP `examples/common/Tmp1x2CliOutput.cpp::printTransferStats/printRunStats`
separates tracked health deltas from adapter attempts including raw/scan/probe,
and labels saturation deltas as lower bounds. `tools/hil_tmp1x2_runner.py` parses
the separate records and checks raw/probe exclusion. ADS
`examples/01_basic_bringup_cli/main.cpp` and `tools/run_i2c_hil.py` explicitly check
probe leaves health unchanged; the runner checks flashed firmware identity.
INA `examples/01_basic_bringup_cli/main.cpp`, native
`examples/esp_idf/basic/main/main.cpp` and `tools/hil_cli_runner.py` expose health,
explicit probe semantics and offline diagnostics. Adopt honest units and firmware
provenance in later tooling; do not import sensor-specific commands, framework
helpers, a CLI scheduler or a second result store into this prompt.

Reopened all five root `CMakeLists.txt`, `platformio.ini` and
`.github/workflows/ci.yml`. LDC/LSM/ADS/INA native environments use Unity with
`test_build_src=true`; TMP CMake registers core/CLI/console/jobs/variants/helpers/
bus/consumer and Python tooling checks. Workflows invoke native tests, version/
contract guards, documentation and separate firmware builds. PlatformIO Core is
6.1.19; Arduino platform sources pin 55.03.311. Native-IDF workflow sources select
LDC 6.0.2, LSM 5.4.4, INA 6.0.1 and TMP's 5.3.2/5.5.1/6.0.1 matrix. These pins
were inspected, not installed or exercised.

TUSB's current root/test CMake files register strict C++17 runtime suites and keep
assertions active in Release; compile-only planned contracts have no CTest entry.
No checked-in `.github/workflows` directory exists at this starting revision.
Prompt 17 must register its real diagnostics suite and preserve tests-OFF consumer
independence; peer tools and sibling include paths are not dependencies.

## Verification limits

PASS for this report: current source/state inspection and local whitespace/path
consistency review. Peer test source and registration were inspected; peer tests,
firmware builds and physical hardware checks were NOT RUN. No peer result is used
to claim TUSB behavior, aggregate backend timing or electrical qualification.
