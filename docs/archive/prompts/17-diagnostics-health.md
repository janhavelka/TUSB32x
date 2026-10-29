# 17 — Complete raw diagnostics, counters and passive health

Complete [prompt 16](16-interrupt-service.md) first. Read [COMMON](../../prompts/COMMON.md),
the Sync access, accounting and health sections of
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md), and the capability/access matrix.
Deliver bounded `readRegisterSync`/`writeRegisterSync`, the complete diagnostics
view and pure health projection. Reuse accounting installed in prompt 9.

## Focused audit and delegation

Re-audit LDC1614 TransportStats wrappers, LSM6DS3TR raw/managed diagnostics,
TMP1x2 DriverState/HealthSnapshot and ADS1115/INA3221 passive health/effects.
Inspect current declarations, code, examples, CLI/runner accounting, tests, CI
and AGENTS. Record revisions/dirty state and explicit all-source versus managed
choices; a similarly named peer counter may measure a different unit.

A coding subagent may own pure projection tests after types are fixed. Use a
strong reasoning reviewer for raw-write safety and retained effects; the lead
owns shared wrappers/state. Follow [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md), disjoint
file scopes, runtime-supported models and lead-only Git.

## Implement

1. Raw Sync methods validate arguments/map/variant first and reject while active
   work or a retained result owns the slot. Each invokes at most one terminal
   callback. Read output changes only on success. Writes obey mixed-register
   composition and invalidate affected trust/evidence before possible effect.
2. A one-callback write cannot read and then write. Use a current proved safe
   image or reject without traffic. Reject raw changes requiring multi-step role,
   reset, termination or recovery procedures and direct users to typed operations.
   Do not introduce a bypass or second engine. Document any verified consuming
   raw-read semantics; unresolved TUSB322I access remains gated.
3. Expose `TransportStats` saturating uint32 `totalAttempts`, `totalSuccess`,
   `totalFailures`, `lastStatus`. All callbacks count exactly once, including
   raw, initialize, ACK and reset. A callback `I2C_BUSY` counts; local `BUSY`,
   waiting, cancel, deadline without callback, mismatch and detach do not.
4. `DriverDiagnostics.managedTransport` is the explicit filtered production
   subset, excluding raw Sync. Keep source counters single and names honest.
   The later CLI `probe` alias calls initialize and thus remains managed traffic.
   Preserve original last failed Status/detail/time validity after later success;
   no fabricated timestamp or physical byte/transaction count. Raw Sync has no
   fresh owner-time argument: mark an unknown failure time invalid rather than
   reuse an older managed callback's timestamp.
5. Expose binding/initialization, trust, last terminal status and retained effect
   evidence, including unresolved reset/recovery kind after result consumption.
   Diagnostic reads do not repair configuration trust or certify a historical
   sequence. `clearTransportStats` clears source totals/lastStatus only, leaving
   managed streak, last failure, trust, port age, effects and result untouched.
6. Implement pure `validateHealthPolicy` and `projectHealth` with the exact
   `UNINIT/READY/DEGRADED/OFFLINE` vocabulary. Threshold default is 3, valid at
   least 1; invalid policy returns `INVALID_CONFIG` with output unchanged.
   Uninitialized/end/external invalidation projects UNINIT even with errors.
   Initialized zero managed-failure streak is READY; positive streak below
   threshold is DEGRADED; at/above threshold OFFLINE. Any later managed callback
   success clears the streak. Raw traffic, detach and source-stat clear do not.
7. Projection owns no I2C, clock, mutation, retries, admission or recovery policy.
   READY can coexist with UNKNOWN configuration or invalid port fields. OFFLINE
   cannot reject legal explicit work. Threshold changes affect display only.
   Keep transport, state, freshness, attachment and operation result independent.

## Verify and finish this prompt

Register `tusb32x_diagnostics`. Test raw read/write limits, invalid map/variant,
active/result backpressure, unchanged read output, unsafe-sequence rejection,
possible failed write and safe-image absence. Count every callback once; verify
raw all-source inclusion and managed exclusion, normalization of every callback
code outside OK/I2C_*, local versus callback BUSY, saturation and retained failure
detail. Exercise bounded arithmetic helpers at limits without public test seeds.

Test the complete projection table at thresholds 1/default/maximum, invalid
policy unchanged output, failed initialization, managed success after OFFLINE,
independent instances and legal diagnostics/managed work while OFFLINE. Prove
clear invariance and unresolved recovery diagnostics after result take. Tests
use runtime behavior; source-token searches cannot prove accounting or passivity.

Update file/type/member/function Doxygen, diagnostics/API/ownership guides,
feature matrix, README/CHANGELOG, progress and validation with actual commands.
Run focused suites and affected prior regressions plus documentation checks.
Review, commit and sync the same execution branch; verify upstream equality and
report real evidence and remaining hardware limitations.
