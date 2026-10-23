# 12 — Implement profile observation and read-only reconciliation

Complete [prompt 11](11-owner-engine.md) first. Read [COMMON](COMMON.md),
the profile/generation/effect sections of
[ENGINEERING_BASELINE](ENGINEERING_BASELINE.md), and the TI register/control proof
matrix. Deliver `startReadProfile` and `startReconcile` through the existing engine.
This prompt creates the shared observation/proof path used by prompt 13's apply
and verify operations; it must already be independently useful and tested.

## Focused audit and delegation

Re-audit ADS1115 desired/observed/applied snapshots, LSM6DS3TR verification and
INA3221 reconciliation/effect code. Inspect matching current tests, owner examples,
CLI/runner semantics, registration, CI and AGENTS. Record revisions, dirty state
and specific choices. INA3221's profile-replaying reconciliation must not be
copied: this library's reconciliation is read-only.

The lead owns shared proof/publication helpers. A coding subagent may own focused
observation tests; a strong reasoning reviewer can inspect what evidence proves
trust and what remains unknown. Use [AGENT_WORKFLOW](AGENT_WORKFLOW.md), disjoint
writes, current runtime models and no subagent Git changes.

## Implement

1. Add fully linked `startReadProfile` and `startReconcile` methods using the same
   copied arguments, token, deadline, callback budget and retained result slot.
   Both perform reads only. Reconciliation remains available when initialization
   or configuration trust is missing, under the baseline prerequisites.
2. `startReadProfile` stages complete persistent observations and publishes only
   valid evidence. It neither replaces desired settings nor promotes trust by
   itself. Keep raw observed bits, decoded values and availability distinct.
   Failed/partial reads preserve prior complete evidence with honest validity.
3. `startReconcile` reads identity plus every applicable persistent control and
   termination/reset fact needed for the proof. One successful unrelated read
   cannot repair an uncertain configuration. Shared helpers must make the proof
   explicit instead of introducing a generic verification framework.
4. `KNOWN` means proved actual persistent configuration and required effect
   evidence; it does not mean actual equals desired. Complete proof publishes
   the actual verified image/generation. When a retained desired image exists,
   compare it without replacement and return `CONFIGURATION_MISMATCH` when
   different. Prompt 13 supplies that desired image through explicit apply.
   Do not add a public desired setter or invent one in initialization to support
   tests. Exercise the desired-mismatch recovery path once apply exists.
5. Share the semantic comparison helper with later verification. Exclude RO,
   W1C/reset command bits from persistent comparison; use the specification's
   semantic equivalence for TUSB322I DRP encodings, preserving raw differences.
6. Require all applicable termination controls: clearing `0x0A` while `0x45`
   disables Rd/Rp cannot prove `ENABLED`. Keep `TerminationState` and `ResetState`
   separate. Proven disabled control remains visible; incomplete transition or
   reset proof remains `UNKNOWN`. Reading a clear recovery bit cannot prove a
   historical multi-step sequence occurred. No role/reset/recovery replay or
   cleanup write is permitted.
7. Respect saturating desired/verified generation semantics and copied result
   provenance. Read-only work has `HardwareEffect::NONE`; it does not rewrite an
   old result or certify old success. Configuration readback cannot clear port
   invalidation: only a complete fresh port observation will do that later.
8. Preserve capability gates and identity policy. Unknown variant/unsupported
   access rejects before traffic; ambiguous TUSB322I controls cannot become
   trusted merely because bytes were readable.

## Verify and finish this prompt

Extend `tusb32x_operations` with exact allowed-read sequences, initialization
missing/failed, complete initial reconciliation, `startReadProfile` without trust
promotion, all applicable termination combinations, reserved controls and partial
read failure. Prove output/generation preservation, no writes, no desired adoption
and no false reset/sequence completion. Use public operations and explicit external
invalidation to create uncertainty; do not expose private state for convenience.

Exercise zero/one/larger budgets and cancel/error/deadline at each read and final
fresh-time boundary. Check first backend error preservation and result take.
Run focused owner/protocol suites with strict warnings and public link checks.

Document the proof table and expected callback/byte ceilings with actual symbols.
Update source/member/method Doxygen, API/ownership/variant guides, feature matrix,
decisions, README/CHANGELOG, progress and validation. Record the future apply
mismatch case as owned by prompt 13 rather than claim it ran here. Review this
complete read-only feature, commit and sync on the existing execution branch,
verify upstream equality and report actual checks and limitations.
