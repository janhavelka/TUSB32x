# 37 — Independently review silicon and owner behavior

Read [COMMON.md](../../prompts/COMMON.md). Prerequisites: prompts 1–36. Deliver an independent
core review with locally resolvable defects fixed and regression-tested.
Prompt 38 reviews integration/distribution and produces the final handoff.

## Refresh evidence and assign reviewers

Re-enumerate Projects and compare the inventory with prompt 1. Inspect the
strongest current peers for transport, ownership, state/effect and health behavior;
record revisions and differences without editing siblings. Read the verified
TI register/procedure outputs, baseline and actual production/tests.

Use separate read-only protocol and owner/portability subagents where available.
Select strong reasoning for timing, W1C/reset, uncertain effects and lifetime
questions; a fast inventory agent is not their sole reviewer. The lead assigns
fixes in disjoint scopes, reviews evidence and owns Git under AGENT_WORKFLOW.
Reviewers must reproduce findings or cite exact contradictory code/source.

## Review and fix

1. Compare prompts 1–19 and relevant shared requirements with implementation.
   Check exact variant/address/identity gates, capability defaults, register 0x45/
   0xA0 availability, debounce/duty mappings, local versus peer roles, accessory/
   current semantics and unresolved TUSB322I access/fault wording.
2. Challenge mixed RO/RW/W1C/reset composition and role-transition order/waits.
   Can an unrelated setter ACK IRQ, replay reset or apply staged settings? Does
   each failed phase retain what may have happened to hardware? Original 320/I
   recovery requires the explicit 30 ms sequence, not a clear final bit.
3. Challenge terminal callbacks, aggregate lock/phase timeout, borrowed buffers,
   no framework/heap leakage, passive bind/end and no hidden retry or health-based
   admission. STOP-separated phases must not receive two full timeouts or be
   mislabeled one physical transaction.
4. Check zero/one/N callback budgets, bounded local work, rollover/backward time,
   fresh callback completion evidence, quantized waits and immutable deadlines.
   Stalled time cannot finish a wait; cancellation/timeout cannot undo effects.
5. Check token exhaustion/staleness, retained result backpressure, exact take,
   lifecycle/end/invalidate and preservation of outcome/effects. Delivery OK is
   distinct from hardware success. No local rejection increments I2C failure.
6. Check desired versus observed/verified configuration, KNOWN actual mismatch
   and subsequent explicit apply. Unrelated reads cannot repair trust. A failed
   or partial operation cannot lose unresolved reset/termination/sequence facts
   when its result is taken or binding ends.
7. Challenge port publication: failed reads cannot invent detach; completed
   observations survive later ACK/recheck failure; historical validity is
   separate from invalidated and age. Do not promise atomic captures or lossless
   events. Reasserted/stuck IRQ cannot create an unbounded service loop.
8. Check all-callback versus managed counters, saturation/reset behavior and pure
   health projection. OFFLINE cannot reject a legal request or start recovery.
   Clearing statistics cannot clear trust, retained result or hardware effects.
9. Review simplicity: one transport/composer/engine, fixed storage and direct
   typed facts. Remove duplicate protocol paths, unused compatibility aliases
   and speculative frameworks. Do not expose private fields solely for tests.

## Evidence and validation

Re-open [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md)'s pinned code and TI tables, tracing
helpers rather than trusting names. Independently check reset byte `0x08`,
Try.SRC `0x06` versus reserved `0x04`, preserved ACK controls and primary errors
surviving later failures. Check 322 reset-to-DRP explicitly; do not infer a full
power-on register image. Base periodic-reset current refresh must remain explicit
application work, with no hidden reset in reads. Confirm unexplained revision
`0x06`, 322 write/fault ambiguities and attached-current freshness remain correctly
gated. Correct inherited mistakes, including prompt/test expectations, and label
code findings separately from physical observations.

Start the core section of `docs/final-audit.md` with reviewed revision/dirty
state, reviewers and `severity | file/symbol | reproduction/evidence | fix |
regression | remaining limit` for findings. Missing physical fixtures are explicit
qualification gaps, not fabricated implementation bugs.

Fix locally resolvable findings with focused regression cases at the failing
boundary, including source Doxygen and affected guides/CHANGELOG. Run relevant
core/conformance/owner/portability checks with Release assertions active. Reuse
existing fakes; do not build a new testing framework for one defect. If a fix
affects adapters/CLI, run those affected checks and flag prompt 38's review scope.

## Finish this prompt

All discovered core defects that can be fixed locally are closed with evidence.
Update feature coverage, validation and progress; retain real silicon/hardware
gates. **Commit and sync** fixes on the existing execution branch and verify
upstream equality. Validate substantive fixes locally; do not dispatch or wait
for remote CI. Prompt 34 owns the initial promptset's remote checkpoint.
Do not claim the entire promptset complete before prompt 38.
