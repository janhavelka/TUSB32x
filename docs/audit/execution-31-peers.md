# Prompt 31 peer and recovery evidence audit

Read-only source review on 2026-10-28. The delegated write scope is this report
only. No sibling edit/build, Git mutation, hardware access or remote CI inspection
was performed. Paths below are relative to the named checkout under
`C:/Users/Honza/Documents/Projects/`.

## Entry state and contracts

`git rev-parse HEAD` and `git status --short` reported these revisions and clean
worktrees at audit entry. TUSB32x also had no staged changes.

| Checkout | Full HEAD |
| --- | --- |
| TUSB32x | `b8eb52fa52e800fe3b53d33f4e53384d37f37544` |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` |

The execution branch is `promptset/tusb32x-implementation-20261023`. The lead
owns fetch/integration, staged review, commits and verified synchronization;
these local reads establish no upstream equality.

Read parent/local and all three peer root `AGENTS.md`, promptset index, COMMON,
prompt 31 and AGENT_WORKFLOW; inspected the reset/recovery, effects, termination,
timing, target lifetime and runner/console sections of ENGINEERING_BASELINE,
ENGINEERING_RULES and CLI_CONTRACT. Prior audit reports were source indexes,
not substitutes for reading current tests. TUSB32x has no `.github/workflows/`
directory at entry. Each peer's `.github/workflows/ci.yml` was inspected as
source; remote runs remain outside prompt 31.

## TMP1x2: interrupted capture and conditional cleanup

Inspected `include/TMP1x2/TMP1x2.h`,
`examples/common/Tmp1x2Cli.cpp` (console telemetry),
`tools/hil_tmp1x2_runner.py` (`SerialSession.command`, `Runner.restore`,
`Runner.stop_owned_work`, `Runner.run`, `build_report`) and
`tools/test_hil_tmp1x2_runner.py`.

- Adopt `test_interrupted_command_retains_partial_output`: Ctrl-C after admission
  preserves the partial IN_PROGRESS evidence and invalidates framing. A new
  success line must not finish the old request after reconnect.
- Adopt `test_lost_serial_framing_defers_cleanup_without_further_writes` and
  `test_reboot_prevents_automatic_restore_to_unchecked_target`: both assert no
  cleanup command after lost framing/restart, retain the captured profile and
  report restoration unverified. Presentation restoration is independently
  gated against a later framing loss.
- Adopt `build_report` retaining any failed result/suite despite cleanup and
  requiring verified restoration for a mutated campaign. Keep partial capture,
  primary outcome, cleanup outcome and manual work visible independently.
- `test_real_cli_cleanup_after_partial_write_cancellation` exercises the actual
  CLI's stop path and cancellation result; it is stronger evidence than a parser
  accepting CANCELLED text. TUSB must retain exact request/origin/token ownership
  and explicitly take only its own result before another operation.
- Reject copying sensor `restore_commands`, automatic `recover`, polling probes
  until reconnection, conversion sleeps or desired-profile replay. TUSB physical
  attachment/supply assumptions can change without a firmware reboot. A fresh
  serial frame alone cannot authorize restoring an old source role.
- Adopt its console distinction between bytes accepted by the backend and
  verified host receipt. A drained native queue cannot certify USB delivery.

The peer workflow runs CMake/CTest with strict conversion warnings and ASan/UBSan,
documentation checks and separate firmware links. Those are useful evidence
layers, not physical recovery evidence or tested TUSB framework versions.

## 24Cxx: durable boundary and preservation of primary failure

Inspected `include/EEPROM24Cxx/EEPROM24Cxx.h`, `tools/hil_runner.py`
(`run_plan`, `capture_failure`, checkpoint/failure cleanup) and
`tools/test_hil_runner.py`, plus `examples/common/Eeprom24CxxCli.cpp`
(`printConsole`, `stop`).

- Adopt `test_checkpoint_precedes_command_and_preserves_completed_steps` and
  `test_checkpoint_failure_prevents_next_command`: a durable RUNNING checkpoint
  precedes transmission. Failure to save mutation intent/baseline prevents that
  mutation rather than leaving an apparently passing final report.
- Adopt `test_backup_io_failure_prevents_programming_and_never_restores` and
  `test_changed_durable_backup_prevents_programming`: baseline evidence cannot
  be missing or silently replaced before writes.
- Adopt `test_preflight_failure_never_cancels_foreign_work`: a foreign active job
  produces failure and cleanup NOT_SENT. The finite runner stops after failure,
  records remaining steps NOT_RUN and sends no speculative cleanup bytes after
  a partial command or already complete terminal response.
- Adopt `test_serial_close_error_preserves_proven_failure` and
  `test_cleanup_error_preserves_original_capture_reason`: cleanup/close failures
  cannot replace a proven FAIL or its original capture reason. Add interrupted
  cleanup and artifact-write variants to the TUSB suite tests.
- Reject EEPROM scratch-byte replay, CRC-based restoration and ACK-poll busy
  classifications. TUSB register readback cannot undo a USB event or establish
  earlier reset acceptance, stable VDD, completed timing or physical CC behavior.
- Keep explicit telemetry availability: its console reports whether RX overflow
  is observable. Missing physical loss observation must remain unavailable,
  rather than a fabricated zero-loss result.

The peer workflow registers native strict/sanitized tests, contract/reference
guards, isolated package consumption and separate Arduino/native-IDF links.
Source registration is not a claim that any peer check ran during this audit.

## INA228: per-transfer faults, effects and owner exclusion

Inspected `include/INA228/INA228.h`, `src/INA228.cpp` (`_failJob`, `_cancelJob`,
`takeJobResult`, `invalidateHardwareState`), `test/test_basic.cpp` and
`examples/01_basic_bringup_cli/main.cpp` (explicit `recover` routing).

- Adopt `test_sample_failure_injection_covers_every_transfer_stage_without_retry`
  and `test_reset_and_accumulator_failure_injection_cover_every_transfer_stage`:
  each failed stage stops at the exact callback count, preserves its terminal
  status and classifies write uncertainty separately from partial success.
  Failure injection should include accepted writes followed by error, rather
  than assuming a transport error prevented hardware acceptance.
- Adopt `test_cancel_and_timeout_are_bus_silent_with_precise_effects`: cancellation
  before work has NONE; confirmed writes before cancellation leave PARTIAL and
  require resynchronization. Cancellation itself is no hardware rollback.
- Adopt `test_job_identity_exactly_once_and_rebinding_do_not_leak_context` and
  `test_active_job_excludes_other_hardware_apis_but_allows_cache_access`: wrong
  IDs cannot consume results, retained results exclude new hardware work and
  rebinding cannot leak the old callback context. Cached evidence stays available.
- Adopt invalidation preserving the first dirty cause and existing dirty-register
  evidence. A fresh successful scalar read cannot silently repair configuration.
- Reject copying INA228 reset/reinitialize configuration replay, destructive
  DIAG_ALRT behavior, job timing/error names or legacy synchronous convenience
  behavior. TUSB uses its established read-only initialize/reconcile and typed
  explicit reset/recovery operations.

The peer workflow uses PlatformIO native tests with captured artifacts and
separate firmware, contract/documentation/package jobs. Preserve its separation
of outcomes; do not substitute it for the current TUSB CTest registrations.

## Current TUSB reset, termination and trust evidence

Revisited `test/test_reset_recovery.cpp`, `test/test_profile_observation.cpp`,
`test/test_owner_scheduling.cpp`, `test/cli/test_cli_operations.cpp`,
`tools/test_cli_transcript.py`, `tools/tusb32x_hil_runner.py` and
`tools/test_tusb32x_hil_runner.py`.

- `literalSuccessAndDesiredRetention` proves reset preserves desired state but
  observes resulting DRP and performs no desired replay. Recovery uses exactly
  four callbacks with independent literal 0x45 expectations. Software proof is
  not an electrical trace or evidence that stable-VDD prerequisites held.
- `everyCallbackFault` and `everyInterruptionBoundary` enumerate callback errors,
  accepted/error writes, cancellation, end, invalidation and deadline boundaries.
  Keep original backend detail and expected PARTIAL/INDETERMINATE effects; do not
  turn all failures into timeout or presumed address NACK.
- `independentPhysicalWaits`, `clippedProcedureTimeouts` and
  `wrappedWaitAndDisabledControls` cover completion-origin waits, sub-ms callback
  completion and rollover. Required scheduling is 95+1 ms for reset and 30+1 ms
  for original recovery. Fake time proves software scheduling only.
- `recoveryHistoryAndExplicitRestart` confirms interrupted recovery history
  survives end/rebind, initialize, reconcile and a later successful reset. A clear
  45 read alone cannot clear it. Only a full explicit new recovery resolves that
  history; full reconciliation then separately restores configuration trust.
- `resetAcceptanceAndOtherTransitionGate` preserves ambiguous reset acceptance
  even after later read-only completion proof and another successful reset.
  Earlier immutable results keep their original effect/detail. Unresolved mode
  transition denies original recovery before traffic; explicit reset is a
  separate legal operation and must never be an implicit runner repair.
- `reservedAndTerminationProof` checks both 0A disableTerm and 45 disableRdRp.
  `partialTerminationContradictionWithoutTrust` immediately makes termination
  UNKNOWN when one observed control contradicts old evidence; cancellation or
  failure before complete paired proof leaves UNKNOWN and does not overwrite the
  old observation. Fixtures must assert aggregate termination plus individual
  controls, including missing/invalid observations.
- `observationAndGenerations` separates profile observation from full reconcile:
  a successful readProfile after external invalidation leaves initialization and
  trust unresolved; a complete read-only reconcile provides fresh proof.
- `literalLateWriteKeepsOutcomeAndEffect` confirms a late successful write ends
  TIMEOUT/PARTIAL, whereas a late failed callback retains its error/detail and
  INDETERMINATE effect. Neither case changes the other device's proof.
- `cancelledApplyNeedsResultTakeAndSeparateReconcile` and
  `externalPowerEventNeedsFreshProofAndPortPublication` are existing owner-level
  seeds for fault-suite ownership, invalidation and post-recovery identity/port
  checks. Power loss is a modeled external event, not an authorized supply action.

## Console evidence and registration

Read `test/console/test_console.cpp`, `test/test_output_mailbox.cpp`,
`test/cli/test_cli_operations.cpp`, `test/idf_app/test_app.cpp` and
`test/CMakeLists.txt`.

- `shortZeroAndInvalidWrites` bounds zero/short writes to one backend call per
  turn and retains queued bytes. `fullQueueAtomicLossAndWrap` and
  `diagnosticRecordsStayWhole` prove whole-record loss with cumulative counters,
  preserved order and a later LOSS marker; there is no plausible truncated ACK.
- `stalledWorkerAndPartialOrder` and `producerProgressWhileWorkerIsBlocked` keep
  borrowed mailbox bytes stable while the owner continues. Actual application
  `backpressure`/`teardown-fifo-stalled` cases report abandoned shutdown output
  while keeping teardown bounded. These model native stalls, not USB reconnect.
- CLI `outputLossAndTakeReservation` leaves the result pending when the complete
  response cannot be reserved; after drain it can be queried and explicitly
  taken. Congested cancel/end still preserve PARTIAL/UNKNOWN without rollback.
  `immutableTimeAndLateSuccess` prevents late read bytes from publication and
  enforces reset wait/deadline behavior while output is stalled.
- The persistent transcript already covers original recovery success and later-
  variant denial, reset success, invalidation and output-full take ownership.
  Prompt 31 must add actual-CLI modeled reset/recovery interrupted effects,
  failure boundaries and fresh-session framing after reconnect. Request/origin/
  token state cannot be carried into a newly validated session.
- Reuse `hil_runner` (90 s) and the single `cli_transcript` (30 s), which receives
  `$<TARGET_FILE:tusb32x_cli_host>`. Core reset/profile/owner tests run through
  `tusb32x_operations`; console and mailbox have independent 30 s registrations.
  Do not create duplicate transcript entries or count compile-only contracts as
  runtime evidence.

## Review validation and limits

The audit's validation is source inspection and documentation checks only. No
peer tests, C++ build, firmware link, serial port, supply switching, physical
reconnect, CC timing or board qualification was performed. Hardware is NOT RUN.
Exact archived TI source confrontation and the prompt's implementation/test
results are recorded separately by the lead and silicon/effect reviewer.
