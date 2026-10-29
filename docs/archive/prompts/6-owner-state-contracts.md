# 6 — Finish owner, state, time and evidence contracts

Prerequisites: prompts 1–5 are complete. Read [COMMON](../../prompts/COMMON.md), the baseline's
method, state, time and diagnostics sections, and the TI procedure guide. Finish
the planned API's precise behavior here; the operation engine is implemented in
prompt 11 and later focused operation prompts. Do not add callable production
declarations without implementation or fake hardware-success stubs.

## Focused re-audit

Recheck LDC1614/LSM6DS3TR tokens, poll timing, effects and diagnostics;
ADS1115/INA3221 desired/observed/verified separation; TMP1x2 exact-token take;
FieldCore-node I2cOwnerTransport/I2cDeviceBinding/I2cLifetime. Read matching
tests, examples, runner meanings, AGENTS and registration. Record revisions,
dirty states and accepted/rejected differences, especially peer replay policies.

## Finish method and admission tables

For every baseline method record prerequisites, deterministic error precedence,
output preservation, allowed UNKNOWN state, callback/CPU bounds and effects.
Use the baseline's exact precedence, not a new competing selection.

- validate/bind/start/cancel/end/invalidate/forget and cached/result methods are
  bus-silent. Only poll advances managed I2C; raw Sync methods have their explicit
  one-callback limit and cannot bypass multi-step silicon procedures.
- Initialize observes identity/configuration only. ReadProfile does not adopt
  desired settings or promote full trust. Apply retains the candidate at
  admission, separately from observed and verified settings.
- Verify compares retained desired settings. Reconcile observes identity,
  persistent controls and interrupted-effect evidence without write/replay.
  Share proof rules: complete proof establishes KNOWN actual settings even when
  comparison returns CONFIGURATION_MISMATCH; desired stays unchanged. Incomplete
  transient proof remains UNKNOWN. A later explicit apply can use known actual
  controls without first matching the interrupted desired profile.
- ACK-only and service are explicit writes; service also observes/publishes and
  rechecks within a bound. Reset never reapplies desired settings. Base-only
  RecoverDeadBattery uses prompt 4's qualified sequence and caller VDD readiness,
  after valid identity; reject other variants without traffic.
- One active slot rejects admission with BUSY; one retained terminal result
  rejects replacement with RESULT_PENDING until exact take. Tokens are nonzero
  instance-local uint64 values, never reused through end/bind/forget; exhaustion
  is TOKEN_EXHAUSTED. Stale tokens cannot advance/cancel/consume represented work.
- resultSnapshot copies; takeResult consumes. Successful delivery returns OK
  even for a failed operation; inspect result.status. Failed calls preserve
  output arguments. Repeated exact take returns RESULT_NOT_AVAILABLE.

## State, time and effects

Complete separate lifecycle, operation, configuration, termination, reset, IRQ
and health tables. CANCELLED with INDETERMINATE effect and READY with UNKNOWN
configuration are valid. Detach is normal. State names and coarse public phases
follow the baseline; do not expose private switch offsets or combine unrelated
states into a cross-product enum.

Binding validates before replacement. end cancels locally, unbinds, retains
result/effects and prevents later use of the borrowed bus. Retain the one
variant/address/context/both-callbacks association; same-target rebinding preserves uncertainty.
Changing target with unresolved evidence requires explicit local forgetting
while unbound/inactive/result-free. Context reuse is not physical identity proof.
Result consumption does not erase interrupted recovery history from diagnostics.
Changed targets cannot inherit usable old identity/configuration/port evidence.
Use one configuration owner per physical target, even on a serialized shared bus.

Use owner uint64 milliseconds and immutable OperationTiming with intervals below
2^63, tested unsigned comparisons, invalid/backward/stalled-time behavior and
correct extension of wrapping 32-bit clocks. Default poll uses at most one
callback; zero budget none; failed callbacks consume budget. Status-only callbacks
require stopping the turn and using a fresh next poll for time-sensitive
completion/wait anchors. Larger budget is a ceiling, not permission to reuse
stale time. Keep the 1 ms hardware margin separate from owner deadlines.
Preserve actual transport failure over a later owner-timeout observation. Define
equal-tick, backward/half-range and unknown completion-time cases exactly as the
baseline; cancel/end/invalidate cannot invent fresh time from the last poll.

Specify aggregate callback timeout including lock/all phases, buffer lifetime,
no pending DMA/retries/reentry and bounded CPU transitions. Callback attempts
are not physical transactions. Use prompt 4's sequence-derived ceilings; later
implementation must verify actual paths against them, not invent lower limits.

## Snapshot and health contract

Use fixed 0x09/0x08/0x09 sampling with final 0x09 and the baseline's 0xE0 comparison;
changed bookends clear cross-register field validity, with no retry until stable.
Failed reads preserve old data/time. `invalidated` starts
true, clears only on fresh complete publication and becomes true on end, external
invalidation or a possible port-affecting write. Historical fields remain.
Use the owner timestamp before the first required read as the conservative
acquisition boundary, never a later publication time. Detached NOT_APPLICABLE
fields are valid interpretations. A complete observation survives later ACK or
recheck failure; retain pending/ACK uncertainty separately, without an event bus.

Keep desired, observed and verified generations/publication points distinct;
compare persistent writable semantics only, including 322I DRP equivalence.
An unrelated successful read cannot repair configuration trust.
Apply admission changes desired input only; pre-write cancellation preserves proved
actual state. Reinitialization clears current identity/initialized usability and
cannot erase interrupted effects. Follow the baseline's shared reconciliation proof.

Specify saturating all-callback TransportStats plus explicitly filtered
managedTransport. Raw Sync calls affect source totals but not managed health;
CLI probe is managed initialize. Optional UNINIT/READY/DEGRADED/OFFLINE is the
baseline pure projection, with separate validated HealthPolicy and unchanged
output on invalid policy. Clearing totals cannot clear live failure streak,
repair trust, consume results or erase effects. OFFLINE neither blocks nor retries.

## Deliver, review and commit

Complete `docs/api-contracts.md`, ownership/lifetime tables, decisions and feature
coverage. Include compile-only passive-observation, explicit-apply and cancelled-
write examples, plus two devices on an external fake owner without FieldCore.
Keep planned methods in docs/contract fixtures until implemented. Update Doxygen
on existing values and affected README/CHANGELOG/progress/validation.

Optional delegation: a strong reviewer checks time/effects and another reviews
admission/snapshot examples read-only. The lead resolves shared contracts and
Git. Compile contract fixtures as C++17 where available; review every rejection,
result-retention and interruption row against baseline. Mark behavioral tests
as future requirements, not passed execution. Commit and sync this complete
contract on the same branch and report checks/limits before prompt 7.
