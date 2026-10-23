# Prompt 4 focused peer re-audit

Reviewed 2026-10-23 for transport, procedure timing and hardware-effect contracts.
Read repository/parent AGENTS.md, COMMON, the prompt index, full prompt 4,
ENGINEERING_BASELINE transport/state/time sections, progress/decisions and
execution-3-peers / execution-1-owner. Re-read current peer code and test bodies;
the earlier reports are comparison evidence only. All sibling paths below are
relative to `Projects/`. No sibling edits, fetches, builds, CI queries or hardware
actions were performed. Tests described below were inspected, not executed.

## Captured state

All four peers have empty `git status --short`, branch `main`, upstream
`origin/main` and local `HEAD...@{upstream}` ahead/behind `0/0`. This comparison
does not establish the current server state because siblings were not fetched.

| Peer | Full HEAD |
| --- | --- |
| PCA9555 | `498178e4d9e4acb6fab7d8b816ad5021efe8806a` |
| LDC1614 | `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` |
| ADS1115 | `381236ebfed813cd7a9aca34520ce031d8a0ff6b` |
| LSM6DS3TR | `513c5641d9b7dda63fddc27267ea62aa8d03ff93` |

At this audit's first edit TUSB32x was clean, including the staged diff, on
`promptset/tusb32x-implementation-20261023` at
`ff0d6a21b65579b7bc3b9b454b16a14b21b74257`, tracking the same-name origin branch.
The lead owns synchronization and shared-contract edits.

At the lead's final recheck, LDC1614 and ADS1115 had concurrent documentation
changes: modified `docs/README.md` and untracked `docs/RETAINED_BRANCH_AUDIT.md`.
Their HEADs and all inspected source/test/build files were unchanged; PCA9555
and LSM6DS3TR remained clean. These sibling changes were left untouched and
are not part of this task. The clean-state table above records the initial audit.

Each peer's root AGENTS.md requires injected application-owned transport,
bounded cooperative work, preserved partial effects and framework separation.
Their chip-specific cleanup, physical-transaction vocabulary and CI policies
do not override the TUSB32x baseline or prompt-34-only remote CI exception.

## PCA9555: guarded byte composition and retained evidence

Read `PCA9555/include/PCA9555/Config.h::TransportResult/WriteEffect`,
`CommandTable.h`, `PCA9555.h`, and `src/PCA9555.cpp::_writeRegs/_writePair/
_writePort/writeRegister/writeRegisters`. `_writeRegs` validates access and length
before creating a fixed three-byte payload. A possible write invalidates its
shadow pair; establishing a full pair requires confirmed complete commitment.
`writeRegisters` rejects raw direction writes because they bypass output preload,
invalidates a permitted pair before a raw attempt, and deliberately handles
odd-start pair wrapping. `_writePort` preserves a companion byte only when the
whole pair was already trusted.

This is a useful guarded composition and provenance pattern. PCA9555's writable
register pairs are not TUSB32x mixed RO/RW/W1C/reset bytes: no inspected PCA helper
proves that echoing a TUSB32x status byte is safe. Keep the TI-derived composer
from prompt 3, preserve actual controls for ACK, and keep command bits clear on
unrelated writes. Reject pair wrapping, port byte order, pointer parking and raw
direction policy as TUSB32x silicon rules. PCA's richer `WriteEffect` callback
cannot be inferred from TUSB32x's Status-only callbacks.

Read `PCA9555/test/test_basic.cpp` bodies and their `RUN_TEST` entries:
`test_ambiguous_apply_write_is_terminal_at_each_write_phase` fails each of the
three write phases, checks exactly failedPhase+1 callbacks, completed/uncertain
pairs and terminal provenance; `test_raw_configuration_writes_are_rejected_without_io`
and `test_partial_raw_write_invalidates_whole_pair_until_full_pair_write` cover
the raw guard. `test_operation_callback_timeouts_share_remaining_deadline_budget`
checks summed callback allowances; `test_near_deadline_callback_count_respects_minimum_timeout`
uses three 1 ms allowances then requires zero callbacks at expiry. Summed
allowances do not establish actual callback completion time or SDK lock bounds.
PCA's cancellation may finish required pointer cleanup: do not import that into
TUSB32x's bus-silent cancellation.

## LDC1614: behavioral transport and independent failure boundaries

Read `LDC1614/include/LDC1614/Config.h` callback contracts and
`src/LDC1614.cpp::buildConfigRegister/buildMuxConfigRegister/
configurationComparisonMask/_readRegister/_writeRegister`. Builders start from
required reserved images. Transport uses one pointer byte and two payload bytes,
with callback storage bounded to the synchronous call. The 16-bit register
framing and sensor initialization replay are rejected for TUSB32x.

Read all `test/support/FakeLdc1614Device.h`: `write` can apply a write before
returning failure; `writeRead` logs bounded transfers; `scheduleConversionAfter`
injects an event between transfers; `readValue` latches the LSB at an MSB read;
`applyReadSideEffects` consumes unread/error/INTB evidence; `applyWrite` models
reset. These are useful mechanisms for future TUSB32x IRQ coalescing, W1C,
command and ambiguous-write tests, using TUSB32x-specific semantics.

Read registered `test/test_basic.cpp` suites
`test_behavioral_fake_models_shadow_status_unread_and_intb`,
`test_initialize_fails_once_at_every_transfer_with_full_provenance` and
`test_apply_exact_counts_failure_cancel_and_ambiguous_write_provenance`. The first
requires the old `0x4567` LSB after a new conversion; the apply suite injects a
failure at every transfer and requires the exact failed-transfer count with the
original detail. Zero-budget polls require no transfer.

Limits matter: this fake imports production constants, permits most array writes,
and applies read-consuming effects only on successful reads, although `readValue`
can already latch data. It is not independent silicon evidence and does not
exhaust possible failed-read effects. Future TUSB32x tests need independent TI
literal vectors and explicitly injected before/after effects. Do not import
LDC's consuming STATUS semantics, sensor coherency, initialization writes or
multi-result FIFO.

## ADS1115 and LSM6DS3TR: completion clocks and effects

Read `ADS1115/include/ADS1115/Config.h`, owner declarations in `ADS1115.h`,
`examples/02_owner_safe_poll/main.cpp::lockBus/ownerWrite/ownerWriteRead/
consumeTerminalResult/loop`, and the current owner tests. The example puts
pins, bus setup, mutex, time and retry/recovery policy in the application;
`poll(nowMs,1)` advances bounded work and `takeResult(activeToken,result)` consumes
the matching result before replacement work. Adopt those ownership boundaries.

Reject direct adapter copying. Its read uses `endTransmission(false)` followed
by `requestFrom`, not the STOP-separated pointer framing described by TI for
TUSB32x. `lockBus` rounds a sub-tick timeout up to a tick; per-phase Wire timeout
selection and a lateness check cannot prove the SDK's internal locks, waits and
cleanup honor one aggregate callback budget. Callback validation statuses
INVALID_PARAM/INVALID_CONFIG and Wire error mapping also need TUSB32x's stricter
normalization: only OK/documented I2C_* may enter the core. A generic backend
error cannot establish a NACK phase or more specific fault. Do not copy bounded
application recovery attempts into core policy.

Read registered ADS suites
`test_owner_safe_ambiguous_delayed_start_uses_post_callback_quiet_interval`,
`test_owner_start_wait_uses_post_callback_boundary_across_wrap`,
`test_owner_safe_poll_clamps_callback_timeout_to_deadline_remaining`,
`test_owner_safe_cancel_after_ambiguous_start_preserves_transport_error` and
`test_owner_safe_tokened_read_publishes_atomic_sample_exactly_once`.
The delayed-start vector advances the fake clock from 200 to 209 during a failed
write that still applies, arms a later quiet boundary, performs no wait I/O and
retains I2C_TIMEOUT/detail -202. The rollover vector advances a callback eight
milliseconds across UINT32_MAX and forbids ready reads until the fresh boundary
plus its ADC interval. Adopt the delayed-callback and original-error checks;
reject ADC quiet periods, 32-bit time, conversion retries and reapply recovery.

Read `LSM6DS3TR/include/LSM6DS3TR/Config.h`, `LSM6DS3TR.h::OperationTiming/
OperationResult`, and `src/LSM6DS3TR.cpp::_read/_writeByte/_stepResetBoot/
poll/cancelActiveJob/takeResult`. `_writeByte` marks possible effects only after
its callback-ceiling guard. `_stepResetBoot` enters a new stage after the command,
arms the wait from a later owner sample, and adds a separately named 1 ms clock
quantization margin. Adopt those causal boundaries and explicit effect evidence.
Reject its fixed per-callback timeout without remaining-deadline clamping,
direct `nowMs >= deadlineMs`, saturating wait timestamps, sensor SETTLING state,
reset polling/profile replay and automatic restoration as TUSB32x contracts.

Read registered LSM suites
`test_reset_orders_required_modes_and_enforces_no_i2c_gate`,
`test_reset_boot_and_recover_failures_are_bounded_at_every_transfer_stage`,
`test_reset_and_boot_wait_deadlines_saturate_at_uint64_max`,
`test_cancel_before_and_after_configuration_effect_is_bus_silent` and
`test_ambiguous_write_effect_is_observable_and_never_retried`.
The reset wait test arms 50 ms after its command, prohibits callbacks through
arm+15, and permits them at arm+16. The saturation test deliberately expires
at UINT64_MAX; it is evidence of a different time contract, not a rollover
implementation to copy. TUSB32x uses unsigned half-range intervals and natural
uint64 rollover, fresh post-callback time, immutable deadlines and its own
OWNER_CLOCK_MARGIN_MS. Integer-only peer vectors do not establish TUSB32x's
required sub-millisecond completion cases.

## Two-peer comparison for material choices

| Choice | Current peers compared | TUSB32x decision |
| --- | --- | --- |
| Safe write path | PCA `_writeRegs/writeRegisters`; LDC reserved-image builders | One deliberate byte composer; guard raw escape paths, preserve observed controls; neither chip supplies TUSB32x W1C/reset semantics. |
| Callback completion and bounds | ADS owner adapter and timeout tests; LSM `_read/_writeByte/poll` | One terminal aggregate callback budget includes locks/phases; clamp to deadline; Status alone requires a fresh owner turn. No physical-transaction count claim. |
| Hardware wait origin | ADS delayed-start/wrap tests; LSM `_stepResetBoot` and reset-gate test | Anchor after callback completion, with separate 1 ms margin; TI supplies each minimum/maximum. No copied sensor wait. |
| Failed write/cancellation | PCA phase fault tests; LDC commit-before-failure fixture; LSM effect guard | Preserve successful earlier effects and uncertain acceptance with original error/detail; cancellation has no cleanup or rollback. |
| Clock domain | ADS uint32 wrap test; LSM saturation test | Owner uint64 half-range comparisons; reject casting a wrapping clock or saturating it. Test repeated ticks, rollover and fractional completion. |
| Retained result | ADS tokened owner example/test; LSM takeResult | One instance-local token and terminal slot until taken; delivery success is separate from stored operation status. |

## Examples, runners and actual suite registration

Read each peer's `platformio.ini`, root `CMakeLists.txt` and
`.github/workflows/ci.yml`. All four select Unity with `test_build_src=true` and
register `pio test -e native`; named suites above occur in each test's actual
`RUN_TEST` list. Root CMake files register IDF components, not standalone CTest.
PCA/LDC/LSM use C++17; ADS native/IDF C++11 is rejected. ADS's owner example is
included by `owner_safe_s2/s3`; both environments appear in its workflow matrix.
The workflows register timing guards and CLI/native-IDF checks. They separately
declare firmware builds (PCA IDF5.4, LDC6.0.2, ADS5.3.5 container, LSM5.4.4),
documentation/package checks and runner tests. This source audit proves no build
or remote run passed and does not change TUSB32x's selected toolchain.

Re-read relevant CLI/runner sources:

- PCA `examples/01_basic_bringup_cli/main.cpp` bounds `wreg/wregs` to 02..05;
  `tools/run_i2c_hil.py` explicitly rejects identity proof from ACK. Adopt guarded
  dispatch and honest evidence labels; reject firmware stress scheduling.
- LDC `examples/common/Ldc1614Cli.cpp::printRegisterValue` and REG/WREG dispatch
  call guarded register APIs; `tools/ldc1614_hil_runner.py` checks firmware Git/
  clean records, complete identity records and NOT_RUN without a serial fixture.
  Reject its second on-device session scheduler for TUSB32x.
- ADS CLI consumes owned results and labels raw writes dirty;
  `tools/run_i2c_hil.py::firmware_clean_commit` compares firmware-reported clean
  commit with checkout and retains NOT_RUN rows. Its workflow registers that
  runner's self-test. A checkout alone is not flashed-firmware identity.
- LSM `examples/01_basic_bringup_cli/main.cpp` polls and takes pending tokens;
  `tools/run_hil.py` checks version and WHO_AM_I/address, while
  `tools/run_owner_soak.py` captures a separate physical harness. Its workflow
  registers runner parsing tests. These are validation layers, not portable
  TUSB32x host-runner framing or hardware qualification.

## Review boundary

No peer establishes TUSB32x timing, framing or role/reset/IRQ sequence facts.
The authoritative device guide must retain TI source/page citations, four
callbacks / six TX / two RX bytes for base dead-battery recovery, separate
termination controls, conservative ambiguous-write effects and one immutable
deadline. Later guide review should check that ACK never uses desired settings,
that a final zero register cannot prove a historical wait/sequence, and that
fresh-time deferral cannot silently renew the deadline. No library behavior,
firmware support or physical qualification was implemented by this audit.
