# 16 — Implement explicit interrupt acknowledgement and bounded service

Complete [prompt 15](15-port-snapshots.md) first. Read [COMMON](COMMON.md),
the IRQ, effect, port and owner sections of
[ENGINEERING_BASELINE](ENGINEERING_BASELINE.md), and the verified TI W1C map.
Deliver `startAcknowledgeInterrupt` and `startServiceInterrupt` in the existing
engine. Preserve observation, acknowledgement and remaining work as distinct facts.

## Focused audit and delegation

Re-audit INA228 observe/acknowledge paths, PCA9555 mixed-control preservation and
LDC1614 bounded operation tests. Check current public headers, implementation,
examples, CLI/runner expectations, tests, CI and AGENTS. Record revisions/dirty
state and selected behavior. Re-read TI IRQ/fault access wording; names or an
`RCU` abbreviation do not by themselves prove clear-on-read behavior.

A coding subagent may own IRQ race fixtures in an assigned test file. Use a strong
reasoning reviewer for W1C composition and observation retention. The lead owns
shared phases and reviewed contracts. Follow [AGENT_WORKFLOW](AGENT_WORKFLOW.md),
disjoint scopes, current runtime models and lead-only Git.

## Implement

1. Add both methods with normal token/deadline/budget/result rules. They are
   explicit effect-bearing writes; cached status and plain refresh never ACK.
   Keep pending observations, retained evidence, write acceptance and recheck
   result separate. No automatic service after a getter or initialization.
2. ACK-only reads fresh actual controls and uses the existing safe composer.
   Preserve observed/proved RW bits and write only the intended W1C command.
   Desired duty 30% while actual duty is 50% must preserve 50%. Never use desired
   staging, replay live RO bits, or apply a profile opportunistically. If a safe
   image cannot be established, fail explicitly rather than guess.
3. Full service observes and publishes a complete snapshot, explicitly ACKs,
   then performs the documented bounded recheck. Each callback remains one owner
   step. Preserve a successful pre-ACK observation if ACK or recheck later fails;
   retain its sequence/time and report service error and ACK uncertainty separately.
   Incomplete later reads cannot replace complete data.
4. A failed ACK may already have cleared the latch. Preserve effect uncertainty.
   Recheck establishes current sampled state, not proof that no event disappeared
   between phases. Coalesced events cannot yield a lossless edge count. Expose
   remaining work after the finite service budget; no repeated ACK-until-clear
   loop, guessed minimum ACK delay or automatic follow-up operation.
5. Preserve pending IRQ in unrelated configuration writes. Treat ACK-only traffic
   according to the baseline snapshot rule: historical sampled interruptPending
   is not a promise about the latch after ACK. Keep ACK effects distinct from
   port-configuration writes when applying invalidation rules.
6. Polling must work without an IRQ pin. Any owner-supplied optional pin evidence
   has its own timestamp/provenance. Core owns no GPIO/ISR; later examples signal
   an owner from an ISR without I2C, blocking or logging. No event dispatcher,
   dynamic queue or callback framework is needed.
7. For evidence-supported TUSB322I operations, distinguish VCONN fault evidence
   and acknowledgement from IRQ `0x10`; clearing one cannot silently clear the
   other. Preserve unresolved fault-clear wording as a feature gate and document
   read-side effects only when established by the exact source.

## Verify and finish this prompt

Review [CHIP_EVIDENCE](CHIP_EVIDENCE.md) against the TI IRQ descriptions and pinned
error paths. Fail any required read or truncate a burst; no fabricated complete
observation or successful service may follow. For sequential 09-then-08 sampling,
include success at 09 and failure at 08. Test actual controls `06 -> ACK 16`,
not public code's fixed `11`. Extended RU-change wording
and base/322 CSR wording require explicit fake-device assumptions. Do not import
ignored ACK errors, raw status echoes or an invented fault-clear mechanism.

Extend `tusb32x_port` with ACK `0xB5 -> 0x15` despite staged `0x02`, base
`0xB4 -> 0x14`, actual 50% versus desired 30%, and unknown applied evidence.
Test live RO changes and pending IRQ preservation in every unrelated setter.

Inject IRQ reassertion between observation/ACK, during ACK, before recheck and
after the last recheck. Cover stuck IRQ, coalescing and no-pin polling with finite
work bounds. Fail ACK both before and after simulated latch clearing. Verify
the complete pre-ACK snapshot survives later failures and the original service
error stays visible. Test budgets 0/1/larger, error/cancel/deadline after every
phase, retained effects/result and independent IRQ versus supported fault ACK.

Run focused port/owner/protocol regressions. Update source and public Doxygen,
interrupt/API/architecture guides, exact callback/byte ceilings, feature matrix,
README/CHANGELOG, progress and validation. State physical IRQ wiring/level tests
as separate evidence. Review this complete feature, commit and sync the same
execution branch; verify upstream equality and report actual checks/limitations.
