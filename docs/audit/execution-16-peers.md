# Prompt 16 current peer and IRQ service review

Read-only source review on 2026-10-26. Current declarations, implementation,
registered tests, examples/CLI, Python runners, build definitions and instructions
were reopened for this prompt. No sibling was edited, fetched, built or tested.
Workflow review inspected checked-in definitions; no remote CI was dispatched,
polled or required.

## Current state and instructions

TUSB32x started this review at
`e964fe5d584100e9e98c3a20f09e5cd38b673f1b` on
`promptset/tusb32x-implementation-20261023`, with clean index/worktree. The lead
owns Git mutations, production phases and integration; documentation and tests
have disjoint write scopes. No new branch or worktree was created.

Reopened parent/repository AGENTS, prompt index, COMMON, full
`16-interrupt-service.md`, AGENT_WORKFLOW, MAINTAINABILITY, baseline IRQ/port/effect/
identity/time contracts, current API/architecture/variant/feature guides,
implementation progress and design decisions. The sibling AGENTS files still
require application-owned transport, precise status/effect evidence, bounded work
and separation of software checks from physical qualification. TUSB's first-run
prompt-34-only remote CI exception applies here.

| Peer | Branch | Current HEAD | Index/worktree |
| --- | --- | --- | --- |
| INA228 | main | b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b | Clean |
| PCA9555 | main | 498178e4d9e4acb6fab7d8b816ad5021efe8806a | Clean |
| LDC1614 | main | 1db38b63cb93a561b6e0bea51017ab94102ebd24 | Clean |

These states come from fresh `rev-parse HEAD`, `branch --show-current` and
`status --short` calls. Historic reports were used for navigation only.

## Observation and acknowledgement

INA228 `include/INA228/INA228.h::DiagAlertSnapshot/DiagnosticEvents`,
`src/INA228.cpp::_readDiagAlertTracked/_captureDiagAlert/_writeDiagAlertConfig`
and `acknowledgeDiagnosticEvents` retain destructive-read evidence separately
from current configuration. `_captureDiagAlert` merges sticky event evidence;
software acknowledgement clears matching cached bits with no bus callback.
`test/test_basic.cpp::test_public_read_diag_alert_is_destructive_and_preserved`,
`test_alert_config_setters_do_not_read_live_diag_alrt`,
`test_alert_config_write_failure_preserves_cache_and_marks_diag_dirty` and
`test_diagnostic_event_acknowledgement_and_caller_timestamps_are_deterministic`
are in RUN_TEST registration. They verify destructive read behavior, deliberate
config-only bytes, original write failure and caller-time provenance.

Adopt separate raw observation, hardware effect, software retention and result
outcome, with cached access doing no hidden traffic. Reject INA's clear-on-read,
software-only acknowledgement, merged sticky-event model and sensor timing as
TUSB rules. TUSB IRQ bit `0x10` has explicit W1C; plain refresh writes nothing.
ACK attempt/acceptance and sampled latch state remain independent. TUSB322I bit3
has unresolved RCU/fault access context; the abbreviation does not establish
clear-on-read or a fault-ACK byte.

## Preserving actual writable controls

PCA9555 `include/PCA9555/PCA9555.h::shadowValidPairs/uncertainPairs`,
`src/PCA9555.cpp::_shadowStatus/_writePair/setOutputBits/setInvertBits` distinguish
usable protocol shadow from uncertain or observation-only state. Nonzero masked
helpers require valid, non-uncertain actual pair evidence and preserve the other
pair bits. `test/test_basic.cpp::test_invalid_output_shadow_fences_all_output_rmw_paths_without_io`,
`test_invalid_direction_and_polarity_shadows_fence_relevant_rmw_paths`,
`test_matching_verify_reconciles_an_ambiguous_full_commit_without_reapply` and
`test_mixed_verify_reconciles_matches_and_keeps_mismatch_fenced` cover local fences,
observation and ambiguity. The odd-start write/read regression checks canonical
pair ordering before a preserving masked write. Tests are registered in RUN_TEST.

Adopt qualification of actual controls, preservation of unrelated fields and
explicit uncertainty after possible writes. Reject PCA's pair-wide RW mirror,
input-read clearing and mandatory pointer-park cleanup as TUSB behavior. TUSB09
mixes RO/RW/W1C, so replaying a raw byte or desired profile is invalid. Use the
existing composer with fresh actual09: extended B5 becomes15, actual06 becomes16,
base B4 becomes14. Actual50% must survive desired30%; no new shadow abstraction
or settings proof obligation is needed for a pure IRQ ACK.

## Bounded work and publication

LDC1614 `include/LDC1614/LDC1614.h` operation/batch declarations and
`src/LDC1614.cpp::_finishJob/_buildAcquisition` keep candidate data private until
complete acquisition. `test/test_basic.cpp::test_acquire_failure_at_every_phase_preserves_prior_complete_publication`
fails every callback of the two-channel read, preserves the previous queued
complete batch and checks original error/detail, callback count and no retry.
`test_acquire_cancel_every_phase_is_silent_atomic_and_restartable`,
`test_ldc1614_acquire_failure_cancel_deadline_and_fresh_stale_paths` and
`test_initialize_exact_transfer_counts_and_zero_one_large_budgets` cover bounded
phase failures, cancellation, deadlines and budgets. All are registered.

Adopt independent failure/cancel/deadline boundaries, finite callback counts and
complete-only observation publication. Reject sensor MSB/LSB latching as TUSB
atomicity evidence, completion-time stamping as acquisition time, larger-budget
multi-callback polls and LDC's result FIFO. TUSB uses one retained terminal result
and a fresh owner poll after each Status-only callback. Service publishes complete
09/08/09 data before ACK, then preserves it after later failure. The final recheck
updates sampled IRQ only. Matching E0 bookends cannot exclude change-and-return.

## CLI, runners and build sources

Reopened INA228 `examples/01_basic_bringup_cli/main.cpp` diagsnap/diagraw help and
handlers, plus `tools/run_i2c_hil.py` cache-only/destructive-read steps and target
firmware identity checks. Reopened PCA9555
`examples/01_basic_bringup_cli/main.cpp` bounded owner loop/exact result take,
native-IDF input-read commentary and `tools/run_i2c_hil.py` explicit interrupt
clear/pointer-park labels. Reopened LDC1614
`examples/common/Ldc1614Cli.cpp::printBatch/handleSessionOperationResult` and
`tools/ldc1614_hil_runner.py` complete-batch parsing, fresh result correlation
and firmware-reported identity checks. LDC rejects a nominally successful sample
result with no complete batch and labels readout SEQUENTIAL_READOUT.

Adopt clear cached/live/write labels, exact result ownership and target-reported
firmware provenance for later tooling. Reject peer sensor freshness criteria,
session scheduling, retries and retained CLI result stores as new core policy.
No CLI, runner, GPIO/ISR frontend or framework firmware is delivered here.

Reopened all three root `CMakeLists.txt`, `platformio.ini` and
`.github/workflows/ci.yml`. PCA uses PlatformIO6.1.19, native tests,
timing/CLI/HIL/package/version guards, Doxygen and native-IDF5.4 links. INA uses
PlatformIO6.1.19, native tests and native-IDF6.0.1 links, with retained logs and
separate validation jobs. LDC uses pinned requirements, native tests,
timing/CLI/IDF/readiness/hygiene/runner/clean-consumer/version checks; root IDF
configuration requires Python version validation. These are source observations,
not new peer build results. Adopt registered suites, exact pins and strict docs;
reject a Python/framework dependency for an ordinary TUSB tests-OFF consumer.
TUSB `test/CMakeLists.txt` registers the existing port/owner/protocol suites with
active Release checks. TUSB `.github/workflows/` is absent at this stage; adding
the supported remote matrix remains prompt34.

## Selected service contract and evidence limits

Both operations require current identity but allow UNKNOWN configuration with a
safe fresh actual-control image. TUSB322I rejects both before traffic while
V_FAULT_CLEAR gates every09 write. ACK-only is R09/W09: two callbacks, three
requested TX bytes and one RX byte. Service is R09/R08/R09 publication, W09, R09:
five callbacks, six TX bytes and four RX bytes. Zero budget invokes no callback;
larger budgets still allow at most one per poll. No retries or guessed ACK delay.

Complete pre-ACK fields/sequence/time survive ACK/recheck failure, cancellation
and expiry. ACK does not invalidate historical port data; its sampled IRQ field
is not post-ACK truth. Fresh contrary controls can revoke configuration proof
without replacing complete port history. Pending-before, attempt, callback
acceptance and pending-after do not collapse into one success flag. A failed ACK
can already have cleared hardware; an OK ACK can race reassertion. Pending recheck
still permits bounded success. No recheck proves lossless events or an absence of
changes after its sample. Core polling needs no pin, queue or application policy.

Exact TI IRQ/fault wording and source bytes are reviewed separately by the lead
and silicon reviewer; maintained register/device guides retain the citations and
unresolved gates. Host race fixtures state their fake-device latch assumptions.
Physical INT_N pull-ups, asserted/deasserted levels, wiring, timing and electrical
behavior require exact-part/board/firmware/fixture/trace evidence. This audit
does not claim hardware qualification or unchecked test success. Actual local
checks, commit and synchronization evidence belong in validation/progress.
