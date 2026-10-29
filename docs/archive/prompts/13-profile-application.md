# 13 — Implement explicit profile application and verification

Complete [prompt 12](12-profile-observation.md) first. Read [COMMON](../../prompts/COMMON.md),
the profile, effect and clock contracts in
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md), and exact per-variant TI role
sequences. Deliver `startApplyProfile` and `startVerifyProfile` with complete
failure handling. Reuse the existing engine, write composer and proof helpers.

## Focused audit and delegation

Re-audit ADS1115 profile admission/generations, LSM6DS3TR uncertain writes and
settle-anchor tests, INA3221 effect classification and PCA9555 preservation tests.
Inspect relevant current headers, examples, CLI/runner assumptions, registered
tests, CI and AGENTS. Record revisions/dirty state and concrete choices; do not
copy sensor settling, blind reconciliation replay or peer callback-count labels.

The lead owns operation phases and shared state. A coding subagent may own
disjoint scripted operation tests after the sequence is fixed. Use a strong
reasoning reviewer for waits, ambiguous writes and mismatch recovery. Follow
[AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md); the lead integrates and handles Git.

## Implement

1. Validate the whole candidate and variant before traffic or replacing desired
   evidence. Accepted apply copies the candidate and advances desired generation;
   admission returns `IN_PROGRESS`. Pre-write cancellation/expiry retains desired
   input and the still-proved old actual/port evidence. Invalidate before a possible
   mutating write. Apply differences only; a verified no-op writes nothing and has
   HardwareEffect::NONE. Unknown actual fields require observation/reconciliation.
2. Use proved actual controls for composition; no staged settings sneak into ACK
   or another operation. Base MODE_SELECT changes require an unattached precheck
   before any writes. An unchanged mode with an allowed current-only change need
   not reject attachment. Document the attachment race; software cannot enforce
   an atomic electrical condition.
3. Only actual LI/HI/LAI/HAI mode changes require disable termination, write mode,
   5 ms wait, then enable using the reviewed sequence. Clean `0x0A` DRP bytes are
   `0x01,0x31,0x30`; preserve actual non-mode fields. `0x45` is not a replacement
   for these steps. Establish the requested final disableTerm/disableRdRp state
   through the specified sequence; do not accidentally enable a disabled profile.
4. Anchor the hardware wait after completed mode write using fresh owner time.
   `OWNER_CLOCK_MARGIN_MS=1` makes the TI 5 ms require at least six elapsed
   integer ticks. Completion evidence at 108 means earliest 114; if only the next
   fresh owner timestamp 110 is available, earliest 116. Keep this software
   margin separate from the TI value and immutable deadline. No stale-time
   shortcut, sleeping or lock across polls.
5. TUSB322I DFP/UFP uses disableTerm, set mode, then enable per SLLSEO5D
   §6.3.2.1–2 pp10–11, preserving fields; it specifies no 5 ms wait. Gate an
   unresolved direct return-to-DRP procedure; never hide a reset. Reject actual
   advertisement changes locally while allowing unchanged proved values. Preserve
   raw fault/IRQ independence and document autonomous VCONN effects.
6. Before any possible port-affecting write, invalidate configuration trust and
   port usability while retaining historical data. Set possible HardwareEffect
   before invocation, refine only from positive evidence, and preserve the first
   actual backend error/detail. An error may follow applied data. Never retry,
   rollback or auto-clean up after cancel, timeout or `end`.
7. Verify the complete final persistent image through prompt 12's shared proof.
   `startVerifyProfile` requires retained desired input and performs no writes.
   Complete proof of a different actual image still publishes `KNOWN` actual
   evidence and returns `CONFIGURATION_MISMATCH`; keep desired unchanged. An
   unresolved transition remains `UNKNOWN`, even if some register values match.
8. A read-only reconcile after partial apply may establish a different known
   actual image, allowing a later explicit apply. Do not require the interrupted
   desired image already to match, erase effects, or claim the old job succeeded.
   Preserve distinct termination/reset evidence through result take/end/rebind.

## Verify and finish this prompt

Recheck [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md): the TI-hosted cleanup can replace a
failed mode write with success; reject that behavior and retain the first error.
For extended 320 actual `0A=C2` changing to DFP, require `C3 -> E3 -> E2` with the
5 ms hardware wait before re-enable. The similar 322 public-code example proves
field loss, not timing; use its own TI sequence. Separate disable and mode writes
even when initially unattached.

Extend `tusb32x_operations` with exact variant sequences, all profile validation,
nonzero preserved fields, requested-disabled final states, no-op apply and attached
base current-only success versus mode-change rejection. Test pre-write cancellation
and both no-application/applied-then-error outcomes at every write;
cancel/deadline at disable, mode, wait, enable, readback and final-time boundaries.

Use a sub-ms external fake clock with floor timestamps. Writes completing at
108.1/108.9 must never enable at 113; compare actual elapsed time independently
of production arithmetic. Check budgets 0/1/larger and failed-attempt charging.
Verify differing actual controls become KNOWN only with complete proof, desired
survives, and reconcile-partial-apply followed by new apply makes progress.
Check no writes in verify/reconcile and no blind replay after end/rebind.

Update exact per-variant callback/byte/CPU bounds, API/source Doxygen, effect and
ownership guides, feature matrix, decisions, README/CHANGELOG, progress and
validation. Run focused regressions and documentation checks. Review, commit and
sync this complete behavior on the same execution branch; verify upstream equality.
