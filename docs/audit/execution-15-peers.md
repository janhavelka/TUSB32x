# Prompt 15 current peer and port publication review

Read-only source review on 2026-10-26 for complete port observations and
freshness. Reopened current files rather than inheriting earlier reports. No
sibling was changed, fetched, built or tested. Workflow review inspected checked-in
definitions only; remote CI was not dispatched or polled.

## Repository state and instructions

TUSB32x began this review at `caba75b21bab02e5bdc7bff47c611f797b2ef3dc` on
`promptset/tusb32x-implementation-20261023`, tracking its same-name origin branch,
with an empty `git status --short`. The lead owns Git mutations and integration;
subagents have disjoint source/test/documentation write scopes.

Reopened `../AGENTS.md`, repository `AGENTS.md`, prompt index, COMMON, full
`15-port-snapshots.md`, AGENT_WORKFLOW, MAINTAINABILITY, baseline port/value/time
contracts, current API/register/vector/variant/architecture guides, progress and
design decisions. Current sibling AGENTS files retain injected transport,
application ownership, precise error/effect evidence and truthful qualification
claims. TUSB's prompt-34-only remote CI exception governs this execution.

| Current peer | Branch | HEAD | Index/worktree |
| --- | --- | --- | --- |
| BME280 | main | d538f2e203ee26d6ddbc86f86d2b0cf4c73a6e98 | Clean |
| LDC1614 | main | 1db38b63cb93a561b6e0bea51017ab94102ebd24 | Clean |
| INA228 | main | b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b | Clean |

States come from fresh `rev-parse HEAD`, `branch --show-current` and
`status --short`, without fetching siblings. Their AGENTS files were read.

## Current behavior adopted and rejected

BME280 `include/BME280/BME280.h::ConfigSyncState/SampleEnvelope`,
`src/BME280.cpp::_updateHealth/_markHardwareConfigDirty/_clearHardwareConfigDirty`
and `test/test_basic.cpp::test_sample_freshness_stale_when_hardware_config_dirty`
keep transport success separate from dirty configuration. `_updateHealth` can
return READY after a successful callback without repairing config state;
`_markHardwareConfigDirty` retains the old sample and revokes readiness.
`test_staged_apply_advances_generation_without_freshening_old_sample` compares the
old envelope unchanged after full apply, with a newer configuration generation
and stale classification. Both tests appear in RUN_TEST registration. Adopt
separate communication/configuration/observation facts, retained historical values
and generation provenance. Reject BME measurement prerequisites, synchronous
conveniences, health-semantic failures and calibration/conversion timing as
TUSB port policy. TUSB refresh needs binding only and never repairs trust.

LDC1614 `include/LDC1614/LDC1614.h::SampleBatch/ChannelSample`,
`src/LDC1614.cpp::_finishJob/_buildAcquisition` and
`test/test_basic.cpp::test_acquire_failure_at_every_phase_preserves_prior_complete_publication`
stage raw samples privately and attach a complete batch only on success. That
test fails all six callbacks of a two-channel batch while retaining a queued
prior complete result; it checks original error/detail and no retries.
`test_acquire_cancel_every_phase_is_silent_atomic_and_restartable` and
`test_ldc1614_acquire_failure_cancel_deadline_and_fresh_stale_paths` independently
cover cancellation, ten-callback four-channel failures, deadlines and stale
quality. All are registered. Adopt candidate-only acquisition and independent
raw/valid/fresh evidence. Reject chip-side MSB/LSB latching as evidence of atomic
TUSB status, LDC unread conversion flags as TUSB freshness, completion-time
stamping as acquisition time, multi-callback polls and its result FIFO. TUSB uses
fixed 09/08/09 bookends, captures time before the first read, publishes final09
and retains one terminal result. Matching E0 bookends cannot prove atomicity or
exclude a change-and-return between samples.

INA228 `include/INA228/INA228.h::DiagAlert/DiagAlertSnapshot`,
`src/INA228.cpp::_readDiagAlertTracked/_captureDiagAlert/_writeDiagAlertConfig`
and `test/test_basic.cpp::test_public_read_diag_alert_is_destructive_and_preserved`
distinguish live destructive DIAG_ALRT reads from cached retained evidence.
`test_diagnostic_event_acknowledgement_and_caller_timestamps_are_deterministic`
checks caller-time provenance and cache-only software acknowledgement, with no
extra callback. `test_alert_config_setters_do_not_read_live_diag_alrt` prevents
an accidental destructive read-modify-write. These tests are registered. Adopt
explicit observation versus side-effect boundaries, raw evidence and cached
access without hidden traffic. Reject INA clear-on-read semantics, sticky event
merging and software acknowledgement as TUSB IRQ rules. TUSB refresh records a
sampled IRQ level, writes nothing and does not ACK. TUSB322I raw09 bit3 retains
its unresolved RCU/access context; read-only does not prove non-destructive fault
observation. IRQ ACK/service belongs to prompt 16, with all 322I 09 writes gated.

## Examples, CLI/runners and build registration

Reopened BME280 `examples/01_basic_bringup_cli/main.cpp` freshness/settings output
and `tools/run_i2c_hil.py` cache-only freshness checks and firmware identity guard;
LDC1614 `examples/common/Ldc1614Cli.cpp::printBatch/handleSessionOperationResult`
and `tools/ldc1614_hil_runner.py` raw-before/after freshness/mask checks; INA228
`examples/01_basic_bringup_cli/main.cpp` diagsnap/diagraw help and exact result take,
and `tools/run_i2c_hil.py` destructive-read versus cache-only steps. LDC output
explicitly labels SEQUENTIAL_READOUT and rejects successful acquisition results
without the complete batch. Adopt clear cached/live labels, independent validity,
exact result correlation and firmware-reported identity for later TUSB tooling.
Reject sensor freshness criteria and application session/retry policy in core.
No TUSB CLI, fixture runner or framework frontend is delivered by this prompt.

Reopened all three root CMakeLists/platformio.ini and `.github/workflows/ci.yml`.
BME registers native and native_sanitized Unity environments; its workflow pins
PlatformIO 6.1.19, runs native/sanitized tests, timing/CLI/HIL/package guards,
version check and strict Doxygen. LDC registers native Unity, CLI/readiness/core
boundary/clean-consumer guards and Doxygen; its root IDF CMake invokes Python
version validation. INA registers native Unity, owner/CLI/timing guards and
strict Doxygen with PlatformIO 6.1.19. Adopt registered behavioral suites, strict
documentation and honest source-versus-runtime evidence. Reject a consumer-build
Python requirement, framework-only root CMake and any peer remote CI requirement
for this prompt. TUSB keeps tests OFF consumer independence, active Release
assertions and adds `tusb32x_port` to its native CTest registration. Its checked-in
workflow implementation and remote checkpoint remain prompt 34.

## Port contract and physical evidence limits

The current register guide preserves local attachment codes, variant-specific
base versus extended debug meanings, audio charge-through current, reserved
advertisement/accessory validity and separate raw active-cable status. Pure
single-register decode establishes codes only; the complete paired observation
establishes applicability. Ordinary sink detection requires accessory0 and codes
0/1/3; charge-through requires code2. Extended local-UFP debug current stays
UNKNOWN as conservative policy. Contrary persistent08/09 evidence invalidates
current configuration trust, independently of whether a complete port sample
publishes. Failed refresh still preserves the old cached port exactly. DEFAULT current is not unconditional 500/900 mA,
source advertisement is separate from local-sink detection, and neither is
measured load, USB PD or board power permission. Detached NOT_APPLICABLE is a
valid interpretation; unavailable/invalid data stays UNKNOWN.

Source citations remain B SLLSEN9F pp11/17-18; L SLLSEP2D and A SLLSEQ8D pp10/17-18;
V SLLSEO5D pp10-11/17-19, with exact artifacts/hashes in the reference provenance.
The lead/reviewer records the actual TI source recheck for this implementation
in the silicon/validation evidence.
B's explicit reset recommendation for updated attached-current detection never
becomes a refresh side effect. Extended once-per-attach versus active-tracking
wording stays unresolved. Connector mapping, current tracking, CC/VBUS behavior
and electrical effects require identified exact-part/board/firmware/trace evidence.
CC alone cannot establish Attached.SNK; raw active-cable status is not powered
VCONN proof. Existing 322 advertisement/09-write/non-reset-DRP-return gates remain.

Snapshot publication is complete-only and preserves its first-read acquisition
boundary and configuration provenance. Failed later reads, cancellation and
expiry retain all old complete fields/sequence/time. End, external invalidation
and possible port-affecting writes invalidate historical usability without
erasing those fields. Configuration verification cannot refresh a port sample.
Sequence counts publications, starts at zero and saturates; tokens identify
operations. No host assertion or documentation review qualifies physical behavior.
Actual implementation checks and the lead's commit/sync belong in validation
and progress; this review does not claim unchecked PASS results.
