# P05 - Implement the externally timed owner operations

Read [COMMON.md](COMMON.md), [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md),
[MAINTAINABILITY.md](MAINTAINABILITY.md) and [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md).
Use P01 silicon evidence, P02 contracts and P04 transport/register helpers.
Implement one readable engine, preferably in src/TUSB32xOperations.cpp; lifecycle
and cached access belong with the facade. Small adjacent private responsibilities
may share a source unit under the baseline. Do not create a second synchronous engine.

## Fresh evidence

Re-audit LDC1614/LSM6DS3TR actual headers, admission/poll/cancel/take sources, fake
tests and owner examples; INA3221 PollContext/HardwareEffect and owner tests;
ADS1115 token/profile/result code and owner example; TMP1x2Operations.cpp and
its jobs tests for output preservation, timeout and cancellation. Read current
examples/CLI, Python runners, native registration and CI. Re-read FieldCore
owner-buffer/lifetime contracts as read-only consumer evidence. Record current
paths/revisions/dirty state and concrete adopted/rejected behavior.

## Work 1: bus-silent lifecycle and admission

1. Implement pure validation and atomic bind. Invalid replacement preserves old
   binding, desired/applied evidence and results. Active work returns BUSY;
   terminal slot returns RESULT_PENDING. All paths invoke zero callbacks.
2. Implement start methods with copied arguments, one slot, nonzero uint64 token,
   immutable OperationTiming and baseline exact signatures. Reject bad time,
   profile or unsupported variant before traffic. Allocate token only on accepted
   admission; failed start leaves caller token and prior evidence unchanged.
3. Token-check poll/cancel/resultSnapshot/takeResult. cancel returns OK for local
   cancellation while the retained result contains CANCELLED. takeResult returns
   OK for successful delivery even when the operation failed. Never drop an old
   result or let a stale owner command affect replacement work.
4. end performs no I2C: cancel into retained result, unbind, invalidate usability
   and retain possible effects. invalidateDeviceState records external uncertainty
   and preserves evidence. forgetDeviceEvidence is explicit local forgetting with
   unbound/inactive/no-result prerequisites, never hardware cleanup.
5. Keep one variant/address/context effect record across same-target rebind.
   Changing target with unresolved effects needs explicit forgetting; no registry,
   hidden old-target cleanup or claim that reused context proves physical identity.

## Work 2: small named phases and honest timing

1. Give phases clear names: read identity/control, compose/write, await fresh owner
   time, wait silicon interval, read verification, compare and finish. Keep each
   phase invariant/transition near its switch; no numeric switch-offset protocol.
   Expose only coarse progress labels; private phases are not a public contract.
2. maxTransfers counts terminal callback attempts. Zero means none, default one
   means at most one, failure consumes one. Time-only phases are CPU-bounded and
   hold no bus lock/buffer across polls. No delay, spin, retry or hidden work loop.
3. Use unsigned uint64 half-range comparisons, intervals below 2^63 and immutable
   deadlines. Stalled time cannot finish a wait. Validate backward time per P02.
   Owner time resolution is at most 1 ms and may be floored. Use the named
   OWNER_CLOCK_MARGIN_MS = 1 for hardware scheduling, separate from TI values and
   deadline comparison. TI 5 ms needs 6 elapsed ticks; 95 ms needs 96 ticks.
   The base dead-battery recovery 30 ms wait needs 31 elapsed ticks.
4. Clip callback timeout to remaining operation time and driver cap, including
   lock and all bus phases. Never submit zero timeout or refresh full timeout
   independently for pointer selection/read/verification.
5. With Status-only callbacks, return after each callback. Require fresh owner time
   before time-sensitive terminal success or wait anchoring. A callback starting
   before but ending after deadline still records its actual transport result.
   A successful callback followed by fresh time past deadline ends with TIMEOUT
   without publishing late success. A failed callback retains its first I2C
   error/detail as the primary outcome; later deadline evidence must not overwrite
   I2C_TIMEOUT, I2C_BUS or another actual transport failure.
6. Larger budget is a ceiling, not a promise to consume it with stale time.
   Publish exact per-variant callback totals/buffer bounds from P04 sequences and
   named CPU transition limits; do not build a configurable scheduler subsystem.

## Work 3: operation behavior and effects

| Operation | Required behavior |
| --- | --- |
| startInitialize | Read identity and initial configuration; no apply/reset/role change/ACK. |
| startReadProfile | Observe persistent configuration without desired replacement or trust promotion. |
| startApplyProfile | Retain desired candidate at admission; apply exact variant sequence; trust only full verified image. |
| startVerifyProfile | Read and compare retained desired image using persistent masks; exclude RO/W1C/reset. |
| startReconcile | Read-only identity/config/termination/reset observation after uncertainty; no role/reset replay. |
| startReset | One explicit reset write, required wait and bounded observation; no automatic profile restoration. |
| startRecoverDeadBattery | Explicit base 320/I post-VDD-return recovery with valid identity and caller-owned prerequisite/need; no automatic initialization writes. Other variants reject before traffic. |
| Port/IRQ methods | P06 uses this same engine; ACK is an explicit effect-bearing write. |

1. Base TUSB320 mode change requires observed unattached state. Test that check
   and document an attachment can race it; software cannot promise race elimination.
2. LI/HI/LAI/HAI change follows disable termination, set mode, externally timed
   5 ms wait, then enable termination from P01. Do not substitute 0x45. TUSB322I
   has its separate reviewed sequence; the 320 delay is not its datasheet fact.
   Schedule that TI wait with the 1 ms software margin above. Apply separately
   establishes any requested final disableTerm/disableRdRp state using P01's
   reviewed sequence; do not enable a requested-disabled final profile by accident.
3. Keep TerminationState and ResetState separate. Failed write can make them
   UNKNOWN. Proven disabled termination remains visible after cancel/end/rebind.
4. Reset can resample PORT and alter actual role. Use variant-specific maximum
   timing/default/retention facts; command ACK is not reset completion or POR.
   Never resend ambiguously accepted reset and never erase retained desired input.
5. Desired, observed and verified generations/publication points are separate.
   Failed/cancelled apply cannot leave old image claimed current. Only complete
   verify/reconcile repairs evidence it actually proves; unrelated reads do not.
   Verify/reconcile may establish KNOWN actual controls differing from desired, publishing
   the actual verified image without replacing desired. Report CONFIGURATION_MISMATCH
   when comparing them. Permit a subsequent explicit apply from that known state;
   matching an interrupted desired profile must not be a recovery prerequisite.
6. Set possible HardwareEffect before callback, refine only with positive evidence.
   Preserve first terminal failure/detail and NONE/CONFIRMED/PARTIAL/INDETERMINATE
   independently of operation outcome. No blind rollback from cancel/end/timeout.
7. Explicit cleanup is new owner-selected policy after observation. If P01 needs
   a separate termination control, define a small typed operation with precise
   prerequisites/tests. Do not add generic recovery that guesses USB role.
8. Implement base dead-battery recovery in this same engine: read 0x45, require
   internal bits 1:0 zero or fail CONFIGURATION_UNKNOWN before writes, write
   literal 0x04, wait 30 ms plus the clock margin from fresh completion evidence,
   write literal 0x00, and verify `(reg45 & 0x07) == 0`. Bound it to four callbacks,
   six TX bytes and two RX bytes (bus address phases excluded). No transient port
   publication. Retain interrupted sequence/effect/termination evidence; a later
   clear bit alone does not prove the sequence's history. Do not replay it without
   a new explicit owner request or add a startup/power-policy state machine.
   Existing diagnostics must expose retained unresolved sequence/effect evidence
   after the result is taken; no hidden recovery-history admission restriction.
9. Set PortSnapshot.invalidated before possible port-affecting writes and on
   end/external invalidation. Preserve historical fields/time/provenance; only a
   complete fresh observation clears it. Readback of configuration alone does not.

## Behavioral tests

Add test/test_operations.cpp and fixed fake support; register and execute it.
Check exact bytes/order and retained facts, not source tokens alone:

- Atomic valid/invalid rebind, BUSY/RESULT_PENDING, unchanged token/output failures,
  resultSnapshot repeat, exact take once, old/stale token poll/cancel/take.
- Tokens survive end/bind/forget; exhaustion fixture without huge real iterations.
- Initialize/verify/reconcile perform no writes; reset never reapplies desired.
- Reconcile a partial apply to a known actual image different from desired, retain
  desired/mismatch, then successfully admit explicit apply without a trust deadlock.
- Verify a different actual image with complete identity/control/effect proof:
  publish it as KNOWN while returning CONFIGURATION_MISMATCH and retaining desired.
  Repeat with an unresolved transition; that incomplete proof remains UNKNOWN.
- Base dead-battery recovery: exact 0x45 read/04/wait/00/read order, wrong variant
  and missing identity rejection, unexpected internal bits with no writes, at most
  four callbacks, and no transient port publication. Inject cancel/error/deadline
  at each boundary, including the 31-tick wait; preserve effects and never retry.
  A later clear 0x45 bit must not certify completion of an interrupted sequence.
- Every phase with budgets 0/1/larger, failed attempt charging, total ceilings and
  bounded local work; no lock held through waits.
- Before-first-call/equal/short/late-callback deadline, uint64 rollover, invalid
  half-range/backward time, stalled time and no deadline renewal.
- Sub-ms callback completion with floored owner time; minimum waits cannot shorten.
  Completion at owner boundary 108 schedules earliest 114; absent completion time,
  next fresh boundary 110 schedules earliest 116 for the TI 5 ms wait. For physical
  completion at 108.1/108.9, no follow-up write at 113; use a sub-ms fake clock.
- Cancel/fault at every boundary: disable, mode, wait, enable, reset, readback and
  final fresh-time completion; retained partial/uncertain effect in each case.
- Backend address/data/generic/timeout/bus faults without invented phase evidence;
  old confirmed writes remain meaningful after later failure.
- end/invalidate in every incomplete phase, result after unbinding, same-target
  effects and explicit target-change forgetting; independent instance state.
- Shared fake owner advances a second device between TUSB timed waits, with no
  core retries or cross-instance token confusion.

## Documentation and completion

Update public header Doxygen for implemented methods/types/enums: delivery vs
operation outcome, token lifetime, callback budget/deadline layer and retained
effects. Add source briefs and useful comments at wait anchors, trust invalidation,
write-effect updates and phase invariants. Use simple engineering English.
Document software clock-resolution margins separately from quoted TI timing.

Update docs/api-contracts.md, docs/ownership.md, docs/architecture.md and decisions
with real symbols, state tables, ceilings and reconciliation limits. Update the
feature/source/test matrix, progress and docs/validation.md with exact commands.
Run focused native checks and applicable Doxygen/link checks now. Documentation
is part of completion; it cannot be deferred to P13. Completion requires exact
result delivery, zero-I2C lifecycle and no unsafe replay across failed writes.
