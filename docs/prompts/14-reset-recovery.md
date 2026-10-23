# 14 — Implement explicit reset and original-device recovery

Complete [prompt 13](13-profile-application.md) first. Read [COMMON](COMMON.md),
the reset/recovery and timing sections of
[ENGINEERING_BASELINE](ENGINEERING_BASELINE.md), and the cited variant-specific TI
procedures. Deliver `startReset` and `startRecoverDeadBattery` in the existing
engine. Both are explicit owner requests, never initialization or hidden cleanup.

## Focused audit and delegation

Re-audit LSM6DS3TR uncertain-write tests, ADS1115 callback-time fixtures and
INA3221 retained HardwareEffect handling. Check current declarations, examples,
CLI/runner assumptions, test registration, CI and AGENTS. Record revisions/dirty
state and selected behavior. Re-read original TUSB320 SLLSEN9F §7.4.3 p14 and
Table 7-7 p19 together; the other generations do not prescribe this workaround.

Keep shared engine changes with the lead. A coding subagent may own fixed fault
fixtures after sequences are fixed; use a strong reasoning reviewer for wait
anchors, literal writes and retained sequence evidence. Follow
[AGENT_WORKFLOW](AGENT_WORKFLOW.md), disjoint files and lead-only Git.

## Implement

1. Reset composes the explicit command from proved actual persistent controls.
   Attempt it once, retain uncertainty if acceptance is ambiguous, wait according
   to the exact variant specification and perform bounded observation. Command
   ACK is not reset completion or POR. Reset may resample PORT and change role;
   document defaults/retention and any supported device-side VCONN effects.
2. Use the existing fresh-time rule: the selected 95 ms reset maximum scheduling
   policy requires 96 elapsed ticks with the separate 1 ms clock margin. Do not
   invent that timing for an unverified variant or turn it into attachment-ready
   evidence. The immutable operation deadline never grows. Never restore desired
   settings automatically; retain them separately for later owner policy.
3. `startRecoverDeadBattery` is only for original TUSB320/TUSB320I after stable
   VDD returns when the application determines source-capable startup needs it.
   Valid identity and the caller-owned prerequisite are required. VDD absent
   without a cable can also require that startup procedure; no battery monitor,
   power-policy field or new startup state machine belongs in the core.
4. Read `0x45`; if internal bits `value & 0x03` are nonzero, finish with
   `CONFIGURATION_UNKNOWN` before writing. Then write literal `[0x45,0x04]`,
   wait the TI 30 ms plus software margin from fresh completion evidence, write
   literal `[0x45,0x00]`, and read back `(value & 0x07)==0`. Bound the whole
   procedure to four callbacks, six TX bytes and two RX bytes excluding address
   phases. The delay requires 31 elapsed integer ticks, never 30 floored ticks.
5. Reject LI/HI, LAI/HAI and TUSB322I recovery before transport. Their dead-battery
   descriptions do not prescribe this sequence. Keep ordinary `0x45` internal-bit
   preservation distinct from this qualified literal sequence. Raw Sync access
   later must not offer a shortcut around it.
6. Both operations preserve first error/detail, token/result and possible effects
   through cancellation, deadline, end/rebind and result take. Mark reset/
   termination/configuration uncertainty before possible writes and invalidate
   port usability without erasing historical fields. Do not publish transient
   port observations sampled between recovery writes.
7. Retain interrupted recovery kind/uncertainty in the existing effect evidence,
   visible through the diagnostic view completed in prompt 17 as well as result.
   A later clear bit does not prove the required writes and wait occurred.
   Current termination controls and historical sequence completion are separate.
   No hidden-history admission condition: only a new explicit owner request may
   attempt recovery again. Reconciliation observes rather than replays.

## Verify and finish this prompt

Extend `tusb32x_operations` with exact reset composition/one-attempt behavior,
no desired replay, variant readiness/retention and recovery's full read/04/wait/
00/read sequence. Reject unsupported variants, missing identity and initial
internal values `0x01/0x02/0x03` before writes. Verify final mismatch and bounds.

For each read/write/wait and final-time boundary, inject error, cancel, deadline
and end; failed writes run both possibly-applied cases. Preserve original errors
and retained partial/indeterminate effects. Reconcile after interruption and prove
clear controls alone cannot certify sequence history. Exercise fresh-time absence,
sub-ms completion, stalled time and deadlines for 31/96 ticks independently of
driver arithmetic; no callback at budget zero. Check existing result evidence
contains no transient port observation; prompt 15 also tests published snapshots.

Update Doxygen, reset/recovery prerequisites and source citations near phases,
ownership/variant/API guides, operation ceilings, feature matrix, README/CHANGELOG,
progress and validation. Run focused owner/protocol and documentation checks.
Review, commit and sync the same execution branch, verify upstream equality and
report hardware qualification separately from the tested software procedure.
