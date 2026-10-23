# 31 — Implement reset, fault and console recovery suites

Finish runner coverage with reset-termination, fault and console suites. These
plans test uncertainty and recovery evidence without automatic replay or board
power policy. Finish their software/tests even when physical evidence is NOT RUN.

## Inputs and fresh audit

Read [COMMON](COMMON.md), [AGENT_WORKFLOW](AGENT_WORKFLOW.md),
[CLI_CONTRACT](CLI_CONTRACT.md), and reset/recovery, effects, timing and trust
sections of [ENGINEERING_BASELINE](ENGINEERING_BASELINE.md) and
[ENGINEERING_RULES](ENGINEERING_RULES.md). Prompts 28–30 supply the runner/session,
artifact/verdict rules, observation plans and explicit fixture/cleanup handling.

Re-audit TMP1x2/24Cxx failed-cleanup and interruption handling, INA228 transfer
fault/owner tests and applicable console backpressure tests. Record revisions,
dirty state, exact sources and decisions. Read TUSB reset, cancellation, late
callback, original-part recovery and trust-reconciliation tests before planning
fault injection. Verify relevant TI requirements against the archived sources.

## Complete the remaining suites

1. Add reset-termination, fault and console to explicit repeated --suite selection
   with finite plans, fixture identity, declared setup and mutation prerequisites.
   Reuse the existing identity/checkpoint/result-taking/cleanup paths. No generic
   all-mutating selection, hidden reset, auto-resume, retry loop or new run token.
2. Reset-termination checks explicit sequence/reset waits, timeout/cancel effects,
   termination uncertainty and legal observation/reconciliation. Preserve separate
   disableTerm and disableRdRp controls; observing one enabled does not establish
   overall termination when the other or its evidence remains unresolved. Test
   no automatic profile replay and no unrelated read repairing unknown trust.
3. For original TUSB320/TUSB320I only, include recover-dead-battery under verified
   stable-VDD and source-capable startup prerequisites. Invoke the typed CLI/core
   operation; do not issue the sequence through raw writes in the runner. Verify
   the qualified 0x45=0x04, minimum 30 ms, 0x00 and readback behavior through the
   available evidence, with baseline clock margin. Deny this action on later
   variants. A final cleared register alone cannot prove the historical sequence
   or erase an interrupted recovery's residual uncertainty.
4. Fault covers operator/fixture-induced NACK, disconnect and power cycle, trust
   invalidation, owner responsiveness, fresh identity after recovery and failed
   assumptions. Do not infer failure phase from generic backend errors or perform
   automatic supply/VBUS switching. Explicit fixture actions must be documented
   and separately authorized; unknown physical effects stay unknown.
5. Console covers authorized physical reconnect/backpressure, loss counters,
   reboot detection and fresh framing after reconnect. Distinguish native zero-
   write/backpressure modeling from actual USB/serial behavior. A reconnect or
   boot invalidates old request/result correlation; stale success cannot certify
   the new session. Revalidate firmware identity before cleanup or further writes.
6. At every mutation boundary save the plan/baseline, then outcome and cleanup
   evidence. A failed test remains failed after successful cleanup. Interrupted
   or successful work with missing required cleanup stays incomplete. Never
   blindly replay uncertain reset/role/apply/recovery or restore an old source
   role after attachment/target/supply assumptions changed. Give concrete manual
   reconciliation facts when cleanup is blocked; do not imply cancellation undid
   an I2C write or USB event.

## Prove failures and evidence retention

Use [CHIP_EVIDENCE](CHIP_EVIDENCE.md) to distinguish reset effects: 322 EN_N
high-to-low or software reset restores DRP (SLLSEO5D section 6.3.2, p9), without
proving every register returned to power-on defaults. Test explicit role evidence
and no automatic replay. Base current refresh requires explicit reset under
SLLSEN9F section 7.3.1.2, p11; never turn a fresh read into proof of fresh CC
detection or insert an automatic refresh reset. Preserve exact supply/VBUS_DET
requirements, including 322 sensing, and separate 225/300/375 mA VCONN min/typ/max
from a guaranteed current. No physical claim follows from the fake.

Extend fake-serial/clock tests for every plan and every guard's rejection path:
fixture/identity absence, expected disconnection, changed target, fresh boot,
capture caps, timeout before/after writes, late success, interrupted cleanup,
exception/Ctrl-C and preserved primary failure. No parser test needs real sleeps.

Extend the persistent actual-CLI path for modeled reset/termination/recovery
timeouts and retained effects, failed writes, invalidation, output-full result
take and reconnect framing. The native device model proves software handling,
not 30 ms electrical timing or actual CC behavior. Reuse existing CTest/unittest
registrations, finite timeouts and the built fixture path. Run the focused runner
and transcript checks plus affected core tests if a confirmed core defect is fixed.

Audit all nine suite families from prompts 29–31 against the original contract:
readonly, soak, orientation, role-current, interrupt, reset-termination, fault,
console and accessory. Each has a real plan, explicit prerequisites, finite end,
tested failure path and truthful report. Do not add successful placeholders or
mark physical checks complete because their instructions exist.

## Delegation and finish

Optional: a general coding agent can add console/reconnect tests in disjoint files;
use an independent stronger-reasoning reviewer for original-part recovery and
cleanup/effect decisions. The lead keeps shared verdict/identity rules consistent,
reviews evidence and owns Git operations.

Finish `docs/hil-runner.md` and `docs/hardware-validation.md` for all suites,
including fixture wiring, power constraints, commands, cleanup and NOT RUN limits.
Maintain docstrings, README, CHANGELOG, coverage, validation and progress. Save
physical evidence only when actually run on an authorized known target; HIL never
runs implicitly from normal tests or the later validator. Commit and sync checked
units and the finished prompt on the same branch, verify upstream equality and
report software versus hardware evidence separately.
