# P02 — Design the portable API and state contracts

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** LDC1614 and
LSM6DS3TR owner APIs/diagnostics, ADS1115/INA3221 profile/effect separation,
TMP1x2 public naming, PCA9555 mixed-register state, and FieldCore I2cTask contracts.

## Work

1. Write a compact architecture/design decision document before implementation.
   Include public declarations or a precise signature table for variant/address,
   transport, desired profile, observed status, trust, operation tokens/results,
   raw access, identity, diagnostics and optional health projection.
2. Separate Transport (callbacks/context/timeout constraints), device/variant
   selection, desired silicon Profile, and optional application diagnostic policy.
   Do not put pins, task priorities, Wire, IDF handles or FieldCore queues in core.
3. Specify binding validation atomicity: invalid replacement leaves the previous
   valid binding/configuration and retained results intact. Define end/unbind,
   external-state invalidation and explicit evidence-forgetting behavior.
4. Define start/poll/cancel/result operations for identification/initialization,
   profile read/apply/verify/reconcile, status refresh, interrupt service and
   explicit reset. One active job, at most one terminal callback per default poll, zero-
   budget silence, token uniqueness/reuse, terminal result lifetime and time bounds.
5. Provide passive binding plus read-only observation as a useful first workflow.
   Applying settings/reset is explicit. Define which operations require trust and
   which can observe/reconcile an untrusted device; avoid recovery deadlocks.
6. Specify exact error/status mappings and transport evidence. Unsupported feature
   and invalid argument return without traffic. Configuration mismatch is not an
   I2C failure. Unknown write effect persists after cancellation/timeout.
7. Define port snapshots with raw bytes, validity, local/peer role, orientation,
   current/accessory evidence, acquisition timestamp/sequence, generation and
   coherence qualification. A detached port is valid normal state; no fake sample.
8. Define INT service acknowledgement ownership, pending evidence retention and
   bounded recheck strategy. Decide how application-supplied GPIO observations
   supplement register evidence without owning GPIO or promising every edge.
9. Include a tiny compile-intended pseudo-consumer for bare-metal and a bounded
   owner loop. Demonstrate two devices on a shared bus and app-owned recovery.

## Deliver and check

- `docs/architecture.md`, `docs/api-contracts.md`, updated design decisions.
- State-transition tables for lifecycle, job, trust, IRQ, and health projection;
  an operation admission/transfer-budget table; ownership/lifetime table.
- Review every function for allocation, blocking, bus effects, ISR suitability,
  time requirements and error evidence. Avoid synchronous convenience helpers
  unless justified and bounded; they must share the same protocol/state machine.
- Resolve peers' differences deliberately; API familiarity does not require
  reproducing unsafe legacy semantics or exposing irrelevant chip features.
