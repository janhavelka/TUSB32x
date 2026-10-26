# Prompt 14 current peer and reset/recovery review

Read-only source review on 2026-10-26 for explicit reset and original TUSB320/I
dead-battery recovery. Earlier evidence was revisited by reopening the current
files below. No sibling was changed, fetched, built or tested. Workflow review
inspected checked-in definitions only; remote CI was not dispatched or polled.

## Repository state and instructions

TUSB32x began this review at `a6bcccfe3353461fcc8ee838fcdb0f946a81e821` on
`promptset/tusb32x-implementation-20261023`, with an empty `git status --short`.
The lead owns synchronization, Git mutations and integration. Documentation and
test authors use disjoint write scopes on that same branch.

Reopened `../AGENTS.md`, repository `AGENTS.md`, `docs/prompts/README.md`,
`COMMON.md`, `AGENT_WORKFLOW.md`, full `14-reset-recovery.md`, baseline
admission/effect/time/recovery sections, `api-contracts.md`, `device-behavior.md`,
progress and decisions. Current peer instructions preserve injected transport,
external serialization, bounded work and truthful uncertainty. Their recurring
CI policy does not override this execution's prompt-34-only remote checkpoint.

| Current peer | Branch | HEAD | Index/worktree |
| --- | --- | --- | --- |
| ADS1115 | main | 235af3e8c722661a89a44766a4ccd903a654f6fe | Clean |
| LSM6DS3TR | main | 362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc | Clean |
| INA3221 | main | 41b6184b75f3c152611d4236f7ea41a5ce77bc2c | Clean |

States come from fresh `rev-parse HEAD`, `branch --show-current` and
`status --short`, without fetching siblings. Their `AGENTS.md` files were read.

## Current behavior adopted and rejected

LSM6DS3TR `include/LSM6DS3TR/LSM6DS3TR.h`,
`src/LSM6DS3TR.cpp::_writeByte/_stepConfigure` and
`test/test_basic.cpp::test_ambiguous_write_effect_is_observable_and_never_retried`
retain a write that took effect before I2C_TIMEOUT/detail -88, stop after three
callbacks and expose UNKNOWN state. The same suite registers
`test_configuration_settle_arms_after_readback_with_changing_budgets`,
`test_reconcile_is_read_only_and_only_full_verification_clears_unknown` and
`test_reconcile_preserves_an_existing_unexpired_settle_gate`.
Adopt independent applied-then-error fixtures, no implicit retry and retained
wait obligations. Reject sensor settling durations, desired-dependent actual
proof and encoding uncertainty into terminal state. TUSB retains operation
outcome, HardwareEffect, reset acceptance and recovery history separately.

ADS1115 `include/ADS1115/ADS1115.h`, `src/ADS1115.cpp` and
`test/test_basic.cpp::test_fake_transport_checks_address_and_models_read_time`,
`test_direct_start_timing_uses_post_callback_timestamp`,
`test_owner_start_wait_uses_post_callback_boundary_across_wrap` and
`test_slow_read_callback_cannot_cross_owner_deadline_into_writes` use callback
clock advancement to distinguish pre-call time from completion. The wrap case
arms its interval at a later poll; the late-read case performs no subsequent
write. These tests are present in `RUN_TEST` registration. Adopt that independent
clock model, fresh completion boundary and immutable deadline. Reject conversion
timing, uint32 token/clock policy, multi-callback owner turns and recovery's
profile replay. TUSB uses uint64 owner time, at most one callback per poll and
separate 31/96 elapsed-tick waits for the TI 30/95 ms intervals.

INA3221 `include/INA3221/INA3221.h::HardwareEffect/JobResult`,
`src/INA3221.cpp::_finishJob/_finishJobFailure/_finishJobSuccess/cancelJob/takeJobResult`
and `test/test_owner_operations.cpp::
test_owner_every_managed_write_ambiguity_is_indeterminate_without_retry` retain
terminal status/detail and distinguish NONE, PARTIAL and INDETERMINATE. That
test loops over eleven managed writes with
`test/support/ScriptedTransport.h::WriteEffect::COMMIT_BEFORE_STATUS`, verifies
the changed register and full timeout detail after result delivery, and permits
no retry. Cancellation cases are registered in the same suite. Adopt independent
write acceptance/effect and frozen result evidence. Reject its sensor register
masks, terminal-state vocabulary and desired-replaying reconciliation. TUSB's
result take cannot erase an independent unresolved recovery obligation, and a
clear control read cannot certify the missing historical writes/wait.

## Examples, CLI/runners and build registration

Reopened ADS1115 `examples/02_owner_safe_poll/main.cpp`,
`examples/01_basic_bringup_cli/main.cpp` and `tools/run_i2c_hil.py`;
LSM6DS3TR `examples/02_owner_soak/main.cpp`, `examples/common/ProfileCli.h` and
`tools/run_owner_soak.py`; INA3221 `examples/01_basic_bringup_cli/main.cpp` and
`tools/hil_cli_runner.py`. ADS takes the exact result before its bounded
application recovery decision. LSM takes its pending token and checks
sequence/generation. INA prints HardwareEffect, consumes the retained result
and labels reconcile as reapply/verify; its runner requires matching token and
sequence frames. Adopt explicit result consumption, fresh correlation and
firmware-reported identity for future TUSB tooling. Reject automatic application
retry policy and replaying reconcile semantics in core. No TUSB CLI, fixture
runner or firmware frontend is implemented by prompt 14.

Reopened each peer's root `CMakeLists.txt`, `platformio.ini` and
`.github/workflows/ci.yml`. Their registered native tests, strict guards and
Doxygen commands are source evidence only. Workflows pin PlatformIO Core
6.1.19; ADS uses digest-pinned IDF v5.3.5, LSM v5.4.4 and INA v6.0.1.
Current TUSB root/test CMake registers four C++ suites including
`tusb32x_operations`, Python version fixtures and the Windows launcher suite.
Strict warnings, active Release assertions and tests-OFF ordinary consumption
remain established conventions. No TUSB workflow exists yet; its implementation
and the remote checkpoint remain prompt 34.

## Exact TI reset/recovery evidence

Fresh official downloads on 2026-10-26 matched all four archived PDFs
byte-for-byte. Sources, full hashes and retained vendor terms are in
`docs/reference/source/provenance.json`:

| Official source | Archive revision | SHA-256 |
| --- | --- | --- |
| [TUSB320/I](https://www.ti.com/lit/ds/symlink/tusb320.pdf) | SLLSEN9F, March 2022 | 1306e448c3f3b169f3b90854710033b9a1aede1d04ba35088f3d2e3ed260d2c4 |
| [TUSB320LI/HI](https://www.ti.com/lit/ds/symlink/tusb320li.pdf) | SLLSEP2D, May 2017 | 6e570c562b3bc9ee0c822353f3abebfad692ab27bfd862ff1c07da57e8c0ff81 |
| [TUSB320LAI/HAI](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf) | SLLSEQ8D, May 2017 | 62f7b3f65338e25ebd6fd46ef04474ba806b06b1cc04bc0b3d7b8191b01ca87d |
| [TUSB322I](https://www.ti.com/lit/ds/symlink/tusb322i.pdf) | SLLSEO5D, March 2021 | 7cbd82a81e2b32f56caeca413a6be83372c91e401cae32a0cf85db0646dc9683 |

Reopened actual PDF pages with pypdf: B pp7/14/19, L/A pp6/14/19 and
V pp7/9/13/20. B §7.4.3 p14 prescribes literal 04 to pointer 45, at least 30 ms,
then literal 00 after VDD returns; transient status must be ignored. The same
page includes VDD absent with no cable. Table 7-7 p19 distinguishes disable
bit 2 from TI-internal bits 1:0. The internal-zero precheck and final masked
readback are conservative library qualification beyond TI's recipe. The
configured seven-bit device address remains unchanged; 45 is a register pointer.
L/A §7.3.3 p14 and V §6.4.4 p14 do not prescribe that base-only workaround.

All four switching tables specify reset 26/49/95 ms min/typ/max. B Table 7-7 p19,
L/A Table 10 p19 and V Table 6-10 p20 define reset bit 3 as self-clearing and a
write of 1 as the command. The outgoing data must contain 08; masking 08 with
literal 1 cannot request reset. The existing qualified composer preserves actual
persistent controls, and an uncertain command is attempted only once. The
95 ms maximum scheduling policy becomes 96 elapsed owner ticks with the separate
one-tick quantization margin; callback ACK alone proves neither completion nor POR.

B §7.4.1 p14 and L/A PORT/mode prose preserve PORT resampling effects. V §6.3.2
p9 explicitly returns to DRP on reset; no exact whole 0A reset image follows from
that semantic statement. V §6.3.8 p13 gives autonomous VCONN after Ra/source
attachment, board-owned 5 V VDD and removal on detach. Reset can alter role and
attachment behavior; no core VCONN setter, power policy or attachment-ready
claim is inferred. Desired settings remain separate and are never replayed.

Current downloads establish source availability/revision only, not a renewed
exhaustive errata search or physical verification. No evidence closes existing
322 advertisement, 09-write or non-reset DRP-return gates. Host reset/recovery
logic, framework firmware links, remote CI and hardware qualification remain
separate evidence classes. Lead validation/progress records contain the actual
implementation check outcomes and commit/sync result.
