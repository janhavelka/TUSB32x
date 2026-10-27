# Transport and silicon procedure contract

Initially specified in prompt 4 on 2026-10-23; implemented through explicit interrupt acknowledgement/service
in prompt 16 on 2026-10-26. Host checks
prove software behavior, not adapter timing or physical hardware qualification.
Read the [variant matrix](variant-matrix.md), [register map](register-map.md),
[encoding vectors](register-vectors.md) and [owner baseline](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/prompts/ENGINEERING_BASELINE.md).
The API guide identifies delivered methods. Admission/result contracts were
completed in prompts 5–6; one operation engine implements them in prompts 11–17.
The [protocol conformance matrix](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md)
maps these procedure, interrupt and interruption rules to current named host
tests. Both possible outcomes of failed raw writes/ACKs retain uncertainty;
an unrelated successful read cannot establish configuration trust.

## Evidence and boundaries

Hexadecimal register/data bytes omit `0x` in tables. Page numbers are printed
pages. **B** means TUSB320/TUSB320I, **L** TUSB320LI/TUSB320HI, **A**
TUSB320LAI/TUSB320HAI and **V** TUSB322I. No other variant is implied.

| Source | Framing | Procedures and effects | Timing |
| --- | --- | --- | --- |
| B: SLLSEN9F, March 2022 | §7.5 pp15–16 | §7.3.1.2 p11, §7.3.2 p12, §§7.4.1–3 p14, Table 7-7 pp18–19 | §§6.6–6.7 p7 |
| L: SLLSEP2D, May 2017 | §7.4 p15 | §§7.3.1–3 pp13–14, Tables 9–11 pp18–20 | §§6.6–6.7 p6 |
| A: SLLSEQ8D, May 2017 | §7.4 p15 | §§7.3.1–3 pp13–14, Tables 9–11 pp18–20 | §§6.6–6.7 p6 |
| V: SLLSEO5D, March 2021 | §6.5 pp15–16 | §§6.3.2–3 pp9–11, §6.3.8 p13, §6.4.4 p14, Tables 6-9–6-10 pp18–20 | §5.6 p6, §5.7 p7 |
| N: SLAAE24, May 2021 | Not a substitute for exact part framing | Startup §4.1 p5; IRQ clear §8 p9 | Application examples, not new family deadlines |

Sources and original terms are retained in [provenance](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/source/provenance.json).
Official B/L/A/V downloads matched the archived bytes during the prompt 4 source
audit. No applicable separate erratum was found in the inspected product lists
and targeted TI search; this is not proof that none exists. Unresolved claims
below remain gated. TI facts, the selected library procedure and future hardware
evidence are distinct. [Peer evidence](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-4-peers.md) supplies
engineering patterns only.

The application establishes valid power, enable/ADDR/PORT straps, bus electrical
conditions and exclusive configuration ownership of each target. It owns VBUS,
VCONN supply, IRQ/GPIO handling, clock, locks and retry/recovery decisions. The
core controls none of those resources. A serialized shared bus does not prevent
an uncoordinated second writer or cable attachment. External changes require
invalidation and new evidence.

All managed writes (apply, reset, recovery, ACK/service) and raw writes require
current validated identity; otherwise NOT_INITIALIZED. The baseline's argument,
binding and slot precedence still applies. Read-only initialize, read-profile,
refresh and reconcile remain available with a valid binding even without trust;
verify additionally needs retained desired input. Admission/cancel/end/cached
methods are bus-silent; managed callbacks occur only in poll. ACK/reset may read
their safe control image; apply requires known actual controls at admission.

## Transport framing and callback contract

Prompt 9 implements the [private terminal transport](transport.md) and its host
protocol checks. The facade and owner procedures through interrupt service are
implemented in prompts 11-16. SDK adapters remain planned;
physical framing/timing qualification is separate and has not been performed.

Every logical read selects a one-byte pointer, including repeated reads of the
same register. Select **STOP-separated pointer/write then read** for the standard
application adapters, directly following the sources above:

| Primitive | Requested callback buffers | Selected wire sequence |
| --- | --- | --- |
| R(reg,n) | writeRead: TX `[reg]`, RX n bytes | START, address+W, pointer, STOP; START, address+R, n bytes, final master NACK, STOP |
| W(reg,value) | write: TX `[reg,value]`, no RX | START, address+W, pointer, data, STOP |

The address parameter is seven-bit; adapters add the direction bit. `45` is a
register pointer at that address, not another target address. The adapter holds
its transaction lock across both read phases, including the intervening STOP.
Respect bus-free timing. No lock survives callback return or a hardware wait.
Repeated START is an alternative requiring explicit adapter/part evidence; it
is not mandated by the `writeRead` callback name or public Linux/Rust examples.
Physical framing, final NACK and aggregate timing need traces on each selected
backend. They are not proved by the documentation or a successful mock.

TI documents sequential bytes, but does not promise an atomic register snapshot.
Nonempty reads may remain wholly inside 00..0A, or read one present 45/A0.
Reject holes, wrapping, absent registers and zero lengths before callbacks.
Use R(00,8) for identity, scalar reads for this guide's configuration and port
plans, and only one-byte data writes. Public raw access is scalar. This avoids
depending on implicit pointer persistence, conflicting implicit-pointer wording,
pointer parking or cross-register write effects. Range allowlists do not authorize
crossing into 0B or accessing base A0/V45.

A callback is terminal and returns OK or a documented I2C_* status. Normalize
other statuses to I2C_ERROR while preserving backend detail. RX is scratch until
the callback succeeds completely. Neither buffer may be retained; no pending
DMA, asynchronous completion or reentry is allowed. Do not infer NACK phase from
a generic error. An ambiguous failed write may already have affected the chip.

One aggregate callback timeout covers bus-lock acquisition, pointer phase,
inter-phase delay, receive/send and terminal cleanup. Use the lesser of the
configured cap and positive owner-deadline remainder, narrowing safely. Never
invoke a zero-timeout callback. No phase gets a renewed full timeout. The core
and adapter add no retries; SDK-internal cleanup/retry behavior needs an explicit
backend audit before claiming the bound. A late-return check cannot bound the
blocking that already occurred.

Each attempted callback counts once even if no wire traffic occurred or the
backend failed. Under the selected framing, a read normally contains two bus
transactions and a write one; reported callback counts are not physical
transaction counts. Byte ceilings below count requested TX/RX buffer lengths,
including pointers and excluding address phases, ACK bits and START/STOP.
Failed callbacks may transfer fewer physical bytes; never label requested bytes
as measured bus traffic.

## Time and evidence rules

Use owner-supplied uint64 milliseconds, resolution at most 1 ms, with immutable
OperationTiming. Admission requires unsigned deadline-minus-now in `(0,2^63)`.
Successive poll deltas below 2^63 are valid, including equal ticks and natural
rollover; backward/half-range input is INVALID_PARAM without traffic or state
mutation. Equal ticks do not advance waits. The adapter must extend a wrapping
32-bit clock correctly; a cast is insufficient.

Status-only callbacks supply no completion timestamp. Stop the poll turn after
each callback, even with a larger budget. At the next owner sample taken after
return, first apply deadline/error rules, then use that sample as the conservative
completion boundary. Arm any required wait there. Fresh can have the same integer
value; it must be a new sample. Do not use pre-callback time or estimate elapsed
time from byte count. A zero callback budget never calls transport. Stalled time
cannot complete a wait or prove a deadline; the owner must advance its clock or
cancel. No sleep, busy wait, automatic retries or unbounded state loops.

| Purpose | Exact TI duration | Library scheduling policy |
| --- | --- | --- |
| L/A mode write before enable | At least 5 ms, §7.3.2 p14 | 6 elapsed ticks from the fresh completion boundary |
| B recovery 04 before 00 | At least 30 ms, §7.4.3 p14 | 31 elapsed ticks from the fresh completion boundary |
| Reset observation | 95 ms specified maximum, switching table | Wait 96 elapsed ticks before the bounded post-reset observation |

`OWNER_CLOCK_MARGIN_MS = 1` is the separate quantization allowance, not a new TI
duration. Do not add it to, renew or extend the owner deadline. A successful
callback does not permit a late terminal success: check the next fresh time
before publishing time-sensitive success. A real callback failure remains the
primary status/detail even if the next sample shows deadline expiry. Cancel/end/
external invalidation and raw callbacks lack a fresh owner timestamp; retain
their evidence immediately with completion time invalid, never the last poll time.

Before any possible port-affecting write, invalidate current configuration/port
usability while preserving historical bytes, timestamps and desired settings.
No-write cancellation preserves proved actual state. NONE means no write could
have affected the device; PARTIAL retains confirmed earlier effects without a
complete final proof; INDETERMINATE retains ambiguous acceptance. A successful
full write procedure and its postconditions can establish CONFIRMED. A failed
callback never becomes success because later cleanup succeeded: there is no
cleanup write in a failed operation. End/cancel/result take cannot undo effects.

## Fixed observations and shared proof

These are library-selected, acyclic plans. They intentionally use one common
proof path, fixed storage and scalar controls instead of a generic register
cache or retries until stable.

Implemented through prompt 16: `startInitialize`, `startReadProfile`,
`startApplyProfile`, `startVerifyProfile`, `startReconcile`, `startReset`,
`startRecoverDeadBattery`, `startRefreshPort`, `startAcknowledgeInterrupt` and
`startServiceInterrupt`. IRQ operations are implemented in prompt 16.
In `src/TUSB32xOperations.cpp`, `startOperation`
shares admission, `acceptRead` stages each byte and `publishConfiguration`
commits the complete P. `observeControl` invalidates contradicted proof before
the next owner turn; `compareObservedProfile` shares semantic actual/desired
comparison for apply, verification and reconciliation. It uses the pure codec comparison, including
TUSB322I DRP00/DRP11 equivalence, rather than byte equality.

| Plan | Ordered callbacks | Purpose |
| --- | --- | --- |
| I | R(00,8), then R(A0,1) for L/A/V only | Exact signature/revision check; configured variant remains external knowledge |
| P | R(08,1), R(09,1), R(0A,1), then R(45,1) for B/L/A only | Complete raw/decoded persistent-control observation |
| F | I then P | Complete identity/control proof; effect history still qualifies trust |
| S | R(09,1), R(08,1), R(09,1) | Port observation; final 09 published, bookends compared under E0 |

Initialize runs F read-only, with no reset, ACK or profile adoption. ReadProfile
runs P without replacing desired or promoting trust. Verify runs F and compares
retained desired; reconcile runs F without writing or replaying. Failed required
reads preserve the old complete configuration observation. Complete reserved
codes retain raw bytes but invalidate their typed fields. Verify/reconcile can
publish KNOWN actual controls and CONFIGURATION_MISMATCH together when all
identity/control/effect proof is complete; desired remains unchanged. A successful
unrelated read cannot supply missing effect proof.

| Complete evidence / implemented path | Publication and outcome |
| --- | --- |
| P with every applicable field valid | Publish raw bytes, typed validity and acquisition time. OK; no initialization, desired replacement or verified-generation increment. Matching old KNOWN proof remains; contrary evidence invalidates it. |
| P with a reserved applicable field | Publish complete raw bytes and only valid typed fields; CONFIGURATION_UNKNOWN, no new full trust. Independent termination controls may still be proved. |
| Failed/partial P or F | Preserve the last complete profile/raw/time and verified generation. Complete adverse identity/control evidence still invalidates the affected current usability immediately. |
| Matching I plus complete raw P | Establish initialized usability; reserved settings or unresolved effects may still keep configuration UNKNOWN. |
| F with all applicable typed fields and settled effect evidence | Publish KNOWN actual verified profile and saturating verified generation. Retained desired, if any, stays unchanged; apply/verify/reconcile comparison mismatch returns CONFIGURATION_MISMATCH while actual remains KNOWN. |
| All applicable disable controls observed and effects settled | Publish independent ENABLED or DISABLED termination, even without full configuration trust. A later contrary disable bit invalidates this proof immediately, including when a subsequent read fails or the operation is cancelled. |
| Reset pending before reconciliation | First fresh poll anchors 96 ticks, then one F. Complete F with reset clear establishes reset COMPLETED; unrelated reserved profile fields can still prevent KNOWN. Newly asserted reset remains MAY_BE_PENDING. |
| Interrupted base recovery | Neither clear45, elapsed time nor F can prove the missing historical sequence. Keep that obligation; only explicit recovery or local forgetting addresses it. |

P costs at most 4 callbacks/4 TX/4 RX bytes for B/L/A, or 3/3/3 for V.
F costs 5/5/12 for B, 6/6/13 for L/A, and 5/5/12 for V. Each scalar
read sends one pointer; I reads eight identity bytes plus a revision where
documented. Waits add no callbacks. Counts exclude address phases and do not
claim a physical transaction count. `poll` yields after each terminal callback;
zero budget permits only local wait/deadline/publication transitions. The next
fresh timestamp checks the immutable deadline before accepting successful data.
No P/F path writes, clears port invalidation, or rewrites an old result.
The [owner contract](api-contracts.md#lifecycle-and-operation) distinguishes
current identity readiness from KNOWN configuration: matching I and complete
raw P can establish initialized even when an unresolved recovery sequence or
reserved control makes the operation return CONFIGURATION_UNKNOWN. This permits
explicit recovery after end/rebind without making initialization write anything.

For an interrupted L/A mode write, reconciliation must first allow the 6-tick
settling interval from a retained valid completion boundary, or conservatively
from its first fresh owner sample if none exists. Then F may establish the actual
disabled or enabled configuration. It does not claim the old requested sequence
succeeded. For a possibly pending reset use the corresponding 96-tick interval
and F with reset clear. These are bounded read-only waits, not write retries.
No repeated F is performed if the proof remains incomplete.
Any successful observation of asserted 0A reset bit3 retains MAY_BE_PENDING
and invalidates affected current trust, including P, reset's preparatory R0A
and raw diagnostics. Raw reads can still return OK with the requested byte;
they have no fresh owner time and cannot create a completion anchor. If F first
discovers this pending state, finish CONFIGURATION_UNKNOWN. Do not append a wait
and second F; a later explicit reconciliation can wait and observe once. A later
clear bit cannot silently discard the retained pending evidence.

ResetState::COMPLETED means the reset settling interval and postconditions were
observed; after ambiguous command acceptance it does not prove that a reset was
accepted. Retain that acceptance uncertainty and the old operation result
separately. A clear bit without the interval and F leaves MAY_BE_PENDING/UNKNOWN.
Interrupted B recovery is different: elapsed time and a clear45 cannot establish
its historical 04/wait/00 sequence. Retain that unresolved obligation and UNKNOWN
trust through reconciliation, reinitialization, end/rebind and result take. Only
a successful explicitly requested recovery supplies that sequence evidence;
explicit local forgetting discards evidence without claiming hardware recovery.

Refresh runs S once. On complete acquisition, use the owner timestamp before
the first R09, not publication time. Changed E0 bookends clear cross-register
current/accessory/active-cable validity; final 09 fields and 08 advertisement can
remain interpretable. No retry until stable. Failed reads leave old data/time
unchanged. Detached NOT_APPLICABLE is valid, not an error or health failure.
Complete publication may clear invalidated but does not promise atomicity,
attachment stability or fresh physical current detection.

## Apply: ordinary controls and role changes

Implemented by `planProfile`, `beginApplyProof` and the existing `poll` path in
`src/TUSB32xOperations.cpp`. A fixed six-entry private write plan is composed
and validated before admission; it is not a queue of operations. `poll` executes
each entry once, then uses the existing F publication/comparison helpers.
The plan's composer flags describe preceding planned steps; runtime ordering,
the base detached precheck and fresh completion time establish those facts
before the dependent write. Every mode write, including the single base mode
write, records unresolved mode proof until F completes. Base/322 reconciliation
adds no mode wait; L/A reconciliation waits six ticks before F.

Admission changes only desired input/generation and clears the old desired
mismatch flag. Configuration becomes APPLYING immediately before the first
possible write; cancellation before that point retains the old actual proof.
Every failed write is INDETERMINATE even when the callback reports NACK or BUSY;
the callback cannot prove the phase or absence of effect. Successful writes are
PARTIAL until matching complete F makes the apply CONFIRMED. Complete mismatch
publishes KNOWN actual controls but leaves the operation PARTIAL (NONE for a
no-op) and desired unchanged. A later read-only operation has effect NONE; it
cannot rewrite the consumed result or erase unresolved sequence history.
`DriverDiagnostics.hardwareEffect` stays paired with the previous terminal
kind/status during active work; `finish` publishes the new terminal effect.
Live configuration/mode uncertainty is held in its separate flags.

Admission validates the whole candidate, identity, known actual controls and
resolved transition evidence before replacing desired. Reject a gated requested
change before any callback. Compare semantic fields, including V DRP00/11
equivalence. Never reject unchanged V08/09 solely because writes there are gated.
A semantic no-op performs F, zero writes. Apply changes only differing fields,
except the distinct writes required by a role procedure. Preserve actual
unrequested controls using the register guide's composer; do not echo status,
reserved R, IRQ or reset bits. Validate the complete write plan before starting.
A no-op still compares F against desired: it can return CONFIGURATION_MISMATCH
or CONFIGURATION_UNKNOWN if later evidence contradicts the admitted image.

For any ordinary W45, require known actual internal bits 03=00 before admission.
L/A documents that internal default; on B this zero requirement is conservative
library policy, not an inferred numeric reset default. Nonzero/unknown internal
bits give CONFIGURATION_UNKNOWN without a write; do not repair them. The gate
affects a requested 45 change, not independent reads or permitted changes elsewhere.
Keep the composer's preservation arithmetic separate from this qualification.

| Operation case | Prerequisites and ordered changes | Required verification/effects |
| --- | --- | --- |
| Ordinary B/L/A advertisement or duty/accessory setting | One W08 and/or W09 for changed fields; IRQ bit is zero. B has no accessory-disable field | F compares the final complete profile, not live status; current advertisement is not a VBUS-power command |
| Ordinary debounce / L/A/V preference / disableTerm, unchanged mode | At most one composed W0A, no reset; no role sequence just for unchanged mode | F; changing disableTerm changes CC/state-machine availability, not 45; no additional TI wait is specified for a standalone control change |
| Ordinary B/L/A disableRdRp | At most one W45, preserving actual internal 03; set disable early, clear it last after any mode wait/changes | F checks45 and all other applicable controls. No inherited 30 ms delay except explicit B recovery. This direct control cannot clear recovery history |
| B actual MODE_SELECT change | Owner/fixture keeps cable unattached for the transition. First R09 must show detached; otherwise CONFIGURATION_UNKNOWN, zero writes. Then W0A changes mode and any requested debounce together, before other setting writes | B §§7.4.1–2 p14/Table 7-7 p19. F verifies controls; it cannot prove the port stayed unattached between the precheck and write |
| L/A actual mode change | Separate W0A setting only disableTerm; separate W0A with new mode and requested other 0A controls, still disabled; wait 6 ticks; clear disableTerm in a separate W0A only if final requested state is enabled | L/A §7.3.2 p14. F verifies controls. A requested disabled final state stays disabled; holding it there follows Table 10's disabled-state definition, not a claim that an enabled role has been entered |
| V actual mode change to DFP/UFP | Separate W0A disable; separate mode/control W0A while disabled; separate enable W0A only if requested. No added 5 ms wait | V §§6.3.2.1–2 pp10–11/Table 6-10 p20. F; source attachment may autonomously activate VCONN. Final disabled request holds the disabled state |
| V actual non-reset mode change to DRP | Unavailable under V_RETURN_DRP | No callback, no hidden reset. Existing DRP00/11 remains readable/equivalent |

For B with a mode change, perform the detached precheck before **any** write,
then the mode write. For other cases start with any requested 45 disable, then
changed08/09, then the 0A step(s); a requested 45 enable comes last. With B mode
already written, continue that order without a second 0A write. Each register 45
change occurs once, either early disable or late enable. For L/A/V the first
disable write preserves all other actual 0A fields; the mode write may combine
other requested 0A fields, and the final enable changes only disableTerm. Even
if already disabled, preserve the source's separate mode-procedure writes.
All plans end with F, with no extra port refresh or IRQ ACK.

Both disable controls are retained independently. ENABLED requires all applicable
disable bits clear plus settled transition/reset evidence. One effective disable
can prove DISABLED; unknown conflicting/transient evidence stays UNKNOWN. Clearing
0A while45 remains set cannot report ENABLED. Neither setting measures electrical
termination. A mode sequence can detach or change a port; cancellation cannot
restore its old role, cable state, VCONN or power relationship.

The base detached precheck is advisory evidence, not an atomic interlock. The
caller must prevent the cable race with an external fixture or operating policy;
the core cannot detect or enforce that condition. Do not generalize this mode
restriction to attached advertisement changes allowed by B §7.3.2 p12. The board
must already support the requested current and supply conditions.

## Explicit original-part dead-battery recovery

B §7.4.3 p14 requires the 04/wait/00 sequence after VDD returns when source-capable
operation needs Rp recovery. Dead battery includes VDD absent with no cable.
The caller establishes stable VDD/I2C readiness and decides when this is required;
read-only initialization alone does not perform or prove it. `startRecoverDeadBattery`
requires current validated identity. A different unresolved reset/mode transition
gives CONFIGURATION_UNKNOWN before callbacks; reconcile it first. This gate does
not reject the operation's own retained recovery uncertainty, which an explicit
new attempt may resolve. Other variants reject UNSUPPORTED_FEATURE
without traffic: L/A §7.3.3 p14 and V §6.4.4 p14 do not prescribe this workaround.

| Step | Action | Failure/evidence boundary |
| --- | --- | --- |
| 1 | R(45,1); require internal 03 zero | Conservative library precheck; otherwise CONFIGURATION_UNKNOWN, one counted read, zero writes |
| 2 | W(45,04), literal data | Mark possible effects before invocation; ambiguous failure retains uncertainty and ends without00 cleanup |
| 3 | Next fresh timestamp anchors wait 31 ticks | Owner deadline can end the operation; no early00, no extension and no intermediate port publication |
| 4 | W(45,00), literal data | Failure may leave either termination state; no replay |
| 5 | R(45,1), require `(value & 0x07) == 0`; fresh completion check | Mismatch returns CONFIGURATION_MISMATCH and leaves incomplete evidence; full success proves this recorded sequence |

TI requires ignoring transient status between the writes. There is no port read
in this plan. A later zero45 proves current control bits only. The operation
retains historical sequence uncertainty through cancel/end/result consumption,
visible in diagnostics; a new explicit owner request may retry the whole sequence.
The recovery readback does not revalidate unrelated controls or promote an
otherwise UNKNOWN configuration to KNOWN. A later F can establish full trust
after the recovery obligation is resolved. Keep one operation engine.
Successful recovery returns OK/CONFIRMED for its own completed sequence, while
configuration remains UNKNOWN until full proof; success does not adopt desired.
The final zero control proves base termination ENABLED. Final 04 proves DISABLED
but leaves sequence history unresolved; nonzero internal bits leave termination
UNKNOWN. This scalar observation does not replace the historical full profile.

## One-shot reset and post-reset proof

For each B/L/A/V variant, R(0A,1) obtains actual valid persistent controls; reject
reserved/unsafe controls or a currently asserted reset bit with
CONFIGURATION_UNKNOWN before writing. Compose `(actual0A & 0xF0) | 0x08` for B,
or `(actual0A & 0xF7) | 0x08` for L/A/V, and attempt W0A exactly once.
No retries, automatic role restoration, ACK or desired-profile replay.
On callback failure retain possible reset effects
and the original error; bounded reconciliation is a new explicit operation.

After a successful command, use the next fresh timestamp and wait 96 ticks,
then F. Require valid identity and full actual control interpretation, reset
bit clear, and no unresolved transition/recovery obligation for KNOWN trust.
An asserted reset bit after that bounded observation gives CONFIGURATION_UNKNOWN;
do not keep polling until clear. V must additionally show semantic DRP00/11;
otherwise, with a complete valid profile and clear reset bit, report
CONFIGURATION_MISMATCH, retain raw evidence and do not assert
the documented reset-role postcondition. Requiring a DRP mode readback is a
conservative library consistency check: V p9 promises DRP operation, while p20's
affected-register list omits MODE_SELECT. Neither establishes a full reset image
or qualifies contradictory mode readback. Never infer a complete default byte
from this operating-role rule. Existing desired remains separate and is not the
reset operation's success comparison; an explicit verify can compare it later.
An asserted reset bit or incomplete profile interpretation instead returns
CONFIGURATION_UNKNOWN. A separately valid non-DRP mode still prevents COMPLETED
even when another field is reserved. Retain MAY_BE_PENDING and UNKNOWN trust;
later explicit reconciliation can establish current state without rewriting the
failed reset result. Historical ambiguous command acceptance is never erased by
later successful reset or reconciliation.

The digital reset can affect detection/accessory/attachment/direction (B p19,
L/A p19, V p20). B/L/A sample PORT at reset/power-up (§7.4.1/§7.3.1); V always
returns to DRP (§6.3.2 p9). No exact source establishes that every settings byte
returns to its power-on table value or retains its old value. Observe controls
again. Timing plus clear command plus F establishes settled evidence, not USB
attachment, fresh port data or qualified VCONN. Keep port invalidated until a
separate complete refresh. Prior unresolved recovery history is not erased by reset.
A reset satisfying its own command/wait/identity/control/role postconditions
returns OK/CONFIRMED even if older recovery history keeps configuration UNKNOWN.
Retain that older obligation separately from the new reset result; do not overwrite
it merely because another operation kind completed. An actual failed callback or
missing reset postcondition still fails reset with its original error or the
specified mismatch/unknown status.

## Complete port refresh (implemented prompt 15)

`startRefreshPort` uses the existing admission, token, deadline and retained
result slot. A valid binding suffices; no initialization or known configuration
is required. `poll` executes S as three separate scalar reads:09,08,09. Each
callback sets its own pointer and returns before the next owner turn. There are
at most three callbacks, three TX and three RX payload bytes, with zero writes,
retries, ACKs or resets. Larger budgets remain ceilings; zero budget makes no
callback. Completion requires the existing fresh owner-time/deadline check.

The first09 callback captures owner time, verified configuration generation and
configuration state before invoking transport. `detail::decodePort` uses the
[complete interpretation matrix](register-map.md#complete-port-interpretation-implemented-prompt-15).
Only final complete publication assigns `_port`, advances its saturating sequence
and clears invalidated. Failed reads, cancel and deadline preserve the entire
old snapshot. End/external invalidation and possible port-affecting writes retain
its history but mark it invalidated. Reset/recovery never publish transient data.
`portSnapshot` copies cached evidence with no callbacks, clock reads or policy.

A successfully read contradictory control still invalidates current configuration
trust immediately through `observeControl`, including when a later refresh read
fails. During refresh this does not alter the old port snapshot's invalidated
flag: it remains a historical publication with its original provenance. A new
complete snapshot records pre-first-read configuration provenance, not the later
current configuration state; consult `configurationSnapshot` for current trust.
Neither refresh success nor an unrelated read promotes configuration proof.

The acquisition timestamp is a conservative boundary, not a precise electrical
sample. Bookend equality is not atomic capture. Sequence counts publications,
not IRQ edges, and repeats at UINT64_MAX. Tokens correlate operations; age,
connector wiring, VBUS readiness and USB policy remain external decisions.

## IRQ observation, acknowledgement and service

B Table 7-7 p18 describes CSR changes broadly; L/A Table 9 p18 specifically names
RU-field changes. V Table 6-9 pp18–19 uses broad CSR text and separately reports
VCONN overcurrent. Preserve these distinctions without inventing an exact IRQ
for every settings write. N §8 p9 confirms software must clear the interrupt
to permit subsequent notification; it does not establish an inter-clear 5 ms delay.
B Table 7-6 p16 defines its write-one-clear action and zero-write no-effect;
Table 7-7 p18 identifies the RCU interrupt field.

| Implemented operation | Exact B/L/A plan | Evidence and limit |
| --- | --- | --- |
| Refresh | S, no write | Historical sampled IRQ; no implied ACK |
| ACK-only | R09, then W09 using the variant's ACK formula below | Fresh actual duty/accessory bits, never desired. Callback acceptance is not a sampled-clear guarantee; no recheck in this API |
| Service | S, publish complete observation, compose ACK from final 09, W09, one R09 recheck | At most five callbacks. Recheck updates separate IRQ evidence, not the complete port snapshot |

For ACK-only and service, compose `(actual09 & 0x06) | 0x10` on B or
`(actual09 & 0x07) | 0x10` on L/A. Both use the observed actual controls.

V ACK/service and every09 write remain unavailable under V_FAULT_CLEAR.
V09 reads retain raw fault bit3/validity with unresolved consuming semantics;
IRQ10 ACK is not a fault ACK. No separate fault-clear method is invented.

A successful service can return with recheck pending=true: the bounded pass
finished, but more work may remain for owner policy. Do not loop or automatically
start another operation. IRQ may reassert during ACK, before recheck or after
the final read. A zero recheck cannot prove no event was lost; the latch coalesces
events and sequential observations are not an edge queue. ACK failure may already
have cleared it. Preserve a complete published S and its acquisition time after
ACK/recheck failure; retain original error, write acceptance and remaining-latch
uncertainty separately. Cancellation/deadline has the same preservation rule.

The implemented `prepareInterruptAck` shares the existing `composeWrite` ACK
intent. ACK-only publishes its fresh09 sample after the owner deadline check;
service publishes IRQ-before evidence only with its complete S. A partial first09
does not replace older diagnostic IRQ history. Admission clears only the new
operation's IRQ record, not the previous cached diagnostic record.

| Boundary | Result and diagnostic evidence | Port/configuration evidence |
| --- | --- | --- |
| No complete pre-ACK observation | No new IRQ publication; operation effect NONE | Keep old port; successful contrary control reads may revoke current configuration trust |
| Fresh complete observation, before ACK callback | observationValid and pendingBefore; no attempted/confirmed write | Service publishes complete S, ACK-only retains old port |
| ACK callback about to run | acknowledgeAttempted; effect INDETERMINATE | Preserve port and configuration; no mode/reset/recovery obligation created |
| ACK callback fails, before or after physical latch clear | Original backend error/detail; no acknowledgeConfirmed, no recheck; effect INDETERMINATE | Complete pre-ACK S/history retained |
| ACK callback returns OK | acknowledgeConfirmed immediately; effect PARTIAL until operation completion | Callback acceptance does not prove sampled-clear or electrical state |
| Later cancellation/deadline or service read failure | Retain partial IRQ evidence and PARTIAL effect; no cleanup/retry | Complete pre-ACK observation survives; end/external invalidation still marks it historical |
| Fresh ACK-only completion or service recheck completion | Operation OK/CONFIRMED; service records pendingAfterValid/pendingAfter even if pending=true | Recheck never replaces S or repairs configuration proof |

Each evidence publication also updates the cached diagnostic IRQ record. A new
operation that obtains no IRQ evidence leaves that history untouched; successful
plain refresh changes only its PortSnapshot IRQ field. Contradictory actual09
controls invalidate current configuration trust immediately, including on service
recheck, but never overwrite the pre-ACK port's acquisition provenance or change
its invalidated flag. These facts remain separate from normal lifecycle invalidation.

ACK uncertainty is retained in result/IRQ history and creates no persistent
configuration obligation or binding block. Existing mode/reset/recovery obligations
remain intact. No IRQ pin is required; owners may separately timestamp external
pin evidence and signal from an ISR, but core performs no GPIO/ISR work. Fixtures
inject events explicitly: L/A RU-change wording and B/V broad CSR wording do not
justify assuming every write generates an event or inferring fault-clear behavior.

## Callback and requested-byte ceilings

These maxima follow the fixed plans above, including final F. Failures and
earlier rejections shorten them; waits/cached methods add zero callbacks/bytes.
No automatic retry, poll-until-clear, extra verification read or rollback fits
inside these numbers. Later implementations must check actual paths against
them and explain any evidenced change before changing the contract.

| Plan / operation | B callbacks/TX/RX | L/A callbacks/TX/RX | V callbacks/TX/RX |
| --- | --- | --- | --- |
| I | 1 / 1 / 8 | 2 / 2 / 9 | 2 / 2 / 9 |
| P / ReadProfile | 4 / 4 / 4 | 4 / 4 / 4 | 3 / 3 / 3 |
| F / Initialize / Verify / Reconcile / no-op Apply | 5 / 5 / 12 | 6 / 6 / 13 | 5 / 5 / 12 |
| Apply, no mode change, maximum differing controls | 9 / 13 / 12 | 10 / 14 / 13 | 6 / 7 / 12 |
| Apply with mode change, final disableTerm=false | 10 / 14 / 13 | 12 / 18 / 13 | 8 / 11 / 12 |
| Apply with mode change, final disableTerm=true | Unsupported field | 11 / 16 / 13 | 7 / 9 / 12 |
| Reset: R0A + W0A + wait + F | 7 / 8 / 13 | 8 / 9 / 14 | 7 / 8 / 13 |
| RecoverDeadBattery | 4 / 6 / 2 | Unavailable | Unavailable |
| S / RefreshPort | 3 / 3 / 3 | 3 / 3 / 3 | 3 / 3 / 3 |
| ACK-only | 2 / 3 / 1 | 2 / 3 / 1 | Gated |
| ServiceInterrupt | 5 / 6 / 4 | 5 / 6 / 4 | Gated |
| Raw scalar read | 1 / 1 / 1 | 1 / 1 / 1 | 1 / 1 / 1 |
| Qualified raw scalar write | 1 / 2 / 0 | 1 / 2 / 0 | 1 / 2 / 0 |

For apply: B has at most four writes and, only for mode change, one extra R09.
L/A has at most four ordinary writes; a mode change replaces one 0A write with
three (or two if held disabled). V has one ordinary 0A write or three/two mode
writes;08/09 changes stay gated and 45 is absent. Add F once, never per write.
The same role waits apply before final F when a profile stays disabled.
Only L/A receives the 6-tick mode wait; V gets no invented 5 ms requirement.
The implemented apply/verify/reset/recovery/refresh/IRQ paths meet these ceilings. Each `poll` is a direct
bounded pass: validate time, consume at most one pending completion, process
at most one wait boundary, evaluate completion, select a callback and invoke
it at most once. There is no transition-draining loop. Admission compares eight
profile fields and composes at most six writes; F compares at most four scalar
controls. Zero-budget polls can advance local proof/time state but never call
transport. The default or larger budget still yields after one callback.
Raw writes still reject every multi-step mode/reset/termination/recovery shortcut,
missing safe actual image or variant gate before their one callback.

## Electrical timing and application limits

| Quantity | B | L | A | V |
| --- | --- | --- | --- | --- |
| Maximum SCL | 400 kHz | 400 kHz | 400 kHz | 400 kHz |
| SDA setup / hold minimum | 100 / 10 ns | 100 / 10 ns | 100 / 10 ns | 100 / 10 ns |
| START setup / hold and STOP setup minimum | 0.6 µs each | 0.6 µs each | 0.6 µs each | 0.6 µs each |
| Bus-free minimum | 1.3 µs | 1.3 µs | 1.3 µs | 1.3 µs |
| Rise / fall maximum | 300 / 300 ns | 300 / 300 ns | 300 / 300 ns | 300 / 300 ns |
| Data / ACK valid maximum | Printed 0.9 ns; unresolved | 0.9 µs | 0.9 µs | 0.9 µs |
| Bus capacitance maximum | 400 pF at≤100 kHz;100 pF at≤400 kHz | 400 pF at≤100 kHz;100 pF at 400 kHz | Same L | Same L |
| Enable + active VDD to I2C availability maximum | 100 ms; timing-row polarity typo | 100 ms | 100 ms | 100 ms |
| Reset min / typ / max | 26 / 49 / 95 ms | 26 / 49 / 95 ms | 26 / 49 / 95 ms | 26 / 49 / 95 ms |
| DRP period min / typ / max | 50 / 75 / 100 ms | 50 / 75 / 100 ms | 50 / 75 / 100 ms | 50 / 75 / 100 ms |
| Default CC debounce typical | 133 ms | 168 ms | 168 ms | 168 ms |
| VBUS debounce typical | 2 ms | 2 ms | 2 ms | 2 ms |

Sources are the timing columns cited at the start; table values are not adapter
software deadlines. Default DRP source duty is30% typical. Nominal programmed CC
debounce encodings remain in the register guide. Typical2 ms VBUS debounce and
75 ms DRP do not bound successful attachment. The public 700 ms GPIO delay belongs
to a TUSB321/SerDes board; public 1 ms retries and 25 ms reset spacing do not create
TI requirements. The Linux 95 ms sleep is not proof its reset mask was correct.

The100 ms availability maximum starts after the documented supply/enable
conditions; the application establishes readiness before invoking the core.
Keep L/A STARTUP_EN polarity conflict and base timing-row typo visible: exact
pins govern polarity, but no universal disputed50 ms startup recipe is claimed.
The base 0.9 ns unit conflict requires TI clarification/physical timing evidence.
The full supply/pull-up/pad limits remain in the variant guide.

B §7.3.1.2 p11 recommends periodic explicit software reset to update detected
current while attached. A fresh read does not perform it. L/A and V once-per-
attachment prose conflicts with the selection guide's active-detection summary;
retain ATTACHED_CURRENT and require controlled exact-part traces before claiming
fresh physical detection. Do not add reset to initialize, refresh or IRQ service.
V autonomously powers the unused CC line after Ra/source attachment and removes
VCONN on detach (§6.3.8 p13); its board-owned5 V VDD prerequisite remains. Neither
the core nor a raw active-cable bit supplies or proves board power.

## Independent procedure and failure vectors

These literal specifications supplement the register vectors. They are future
scripted-transport/owner/hardware tests, not tests already executed against a
driver. Non-target controls are valid and proved unless the case states otherwise.

| ID | Input / trace | Required outcome |
| --- | --- | --- |
| P01 | L/A actual 0A=00, requested DRP enabled | W0A01, W0A31, wait 6 ticks after fresh completion anchor, W0A30, F |
| P02 | L/A actual 0A=C2, requested DFP enabled | C3→E3→wait 6→E2, F; debounce/preference preserved |
| P03 | Same as P02, requested disableTerm=true | C3→E3→wait 6, F; never E2 |
| P04 | L/A actual mode already desired; only 08 changes | One W08, F; no 0A/45/IRQ write or artificial role wait |
| P05 | V actual 0A=00, request DFP / UFP | 01→21→20 / 01→11→10, F; no added 5 ms wait |
| P06 | V actual 0A=C2, request DFP | C3→E3→E2, F; other controls preserved; no 5 ms inheritance |
| P07 | V actual 0A=02, desired equivalent DRP32 | No mode write; compare preference separately; F only if otherwise unchanged |
| P08 | V actual UFP, desired DRP; or changed08/09 field | Gated rejection before callbacks; no reset fallback |
| P09 | B mode change, R09=40/80/C0 | CONFIGURATION_UNKNOWN after one counted read, zero writes |
| P10 | B R09=00 then fixture attaches before W0A | Software cannot guarantee safe role change; fixture/owner prerequisite violated, never claim race prevention |
| P11 | B attached, unchanged mode, new advertisement80 | W0880 and F; no blanket attached rejection or IRQ ACK |
| P12 | L/A actual 0A00/45=04, request0A enabled with 45 still disabled | No write merely to clear an already clear0A; final termination DISABLED, not ENABLED |
| P13 | B recovery initial 45=00 | R45,W4504,wait 31,W4500,R45=00; four callbacks,6TX/2RX |
| P14 | B recovery initial 45=01/02/03 | CONFIGURATION_UNKNOWN after pre-read; zero writes; no masked literal workaround |
| P15 | Recovery final 45=04 or internal 03 nonzero | CONFIGURATION_MISMATCH; no successful sequence proof or retry |
| P16 | Cancel after recovery04; later read 45=00 | Retain interrupted-history uncertainty after result take/reconcile; no fabricated successful recovery |
| P17 | Reset actual B0A=E0 / L/A0A=E2 | CommandE8 / EA exactly once, wait 96, F; no desired replay |
| P18 | V reset from modeDFP, postread mode00 or 11 | Semantic DRP postcondition; do not demand an invented whole-byte00 reset |
| P19 | Reset wait complete but0A reset bit still1 | CONFIGURATION_UNKNOWN, no repeat/reset polling loop |
| P20 | P/F/reset-precheck/raw read sees reset bit1, later reads0 without elapsed proof; or wrong signature/incomplete P | Retain MAY_BE_PENDING/UNKNOWN after any discovery; raw read may deliver OK but supplies no completion time; no full trust or extra F; preserve original read error where applicable |
| P21 | Extended09=B5, staged desired controls02 | ACK15, not 13 or rawB5; baseB4 gives 14 |
| P22 | Service S succeeds, ACK fails after possible clear | Complete S survives, original error/uncertain ACK retained, no recheck or retry |
| P23 | Service ACK succeeds, recheck fails or reports IRQ1 | Failure preserves S; successful pending 1 reports remaining work without repeated ACK |
| P24 | S bookends differ underE0; or second read fails | Complete changed image invalidates cross-register fields; failed acquisition preserves old snapshot/time |
| P25 | IRQ event during ACK or after final recheck | No lossless-event claim, no invented 5 ms delay or automatic follow-up |
| P26 | V fault raw 09 bit3 set/clear | Preserve raw validity; no inferred fault-clear action and no 09 write even when fault read 0 |
| P27 | Read pointer TX09 then RX1 under STOP framing | One callback,1TX/1RX, two nominal wire transactions; one lock and timeout across both |
| P28 | Any failed callback / partial scratch RX / illegal callback IN_PROGRESS | Count once, retain exact legal backend error or normalize illegal code, never publish partial RX or infer NACK phase |
| P29 | Ordinary requested 45 change with actual internal 03 nonzero/unknown | CONFIGURATION_UNKNOWN before writes; do not normalize protected bits or qualify them through preservation arithmetic |
| P30 | Base recovery with a different pending reset/mode transition | CONFIGURATION_UNKNOWN before callbacks; explicit reconcile first; own recovery uncertainty alone does not reject a new attempt |

### Independent physical-time vectors

Times in the second column are a fake owner's real millisecond time, including
fractions; callback samples are floored integers. Check elapsed physical time
independently of production deadline/wait helpers.

| ID | Completion and next fresh sample | Required boundary |
| --- | --- | --- |
| T01 | L/A mode completes 108.1 or 108.9; fresh sample at 108.95 gives 108 | Not 113; earliest 114. Physical wait at 114.0 is5.9 or 5.1 ms |
| T02 | Mode completes 108.9; next fresh sample 110.2 gives 110 | Earliest116, not 114; late polling cannot shorten the wait |
| T03 | Recovery04 completes 200.9; sample 200.95 gives 200 | Not 230; earliest 231, at least30.1 ms physically |
| T04 | Reset command completes 300.9; sample 300.95 gives 300 | No post-reset proof read at 395; earliest 396, at least95.1 ms |
| T05 | Pre-callback now100; mode callback completes 108.9 | Never arm from 100 or enable106; the next fresh sample supplies the anchor |
| T06 | Deadline 114 equals L/A readyAt 114 | TIMEOUT before enable, not a renewed deadline; zero late callback |
| T07 | Last read starts before deadline, succeeds after it | Next fresh poll returns TIMEOUT; do not publish a late terminal success |
| T08 | Same read/write returns I2C_TIMEOUT before next time proves expiry | Preserve I2C_TIMEOUT/detail, never replace it with owner TIMEOUT |
| T09 | Same integer time repeatedly; backward delta or exactly2^63 | Equal ticks cannot finish wait; invalid delta leaves state/output/traffic unchanged |
| T10 | Anchor UINT64_MAX-2, six-tick wait | ReadyAt wraps to3; valid elapsed arithmetic accepts3, not 2 |
| T11 | maxTransfers0 /1 /larger; callback fails | 0 invokes none;1/larger invoke at most one with Status-only callbacks; failure consumes one |
| T12 | cancel/end/invalidate after a write, no fresh timestamp | Retain effect/result, completedTimeValid=false; no rollback or invented last-poll completion time |

For **every** W08/W09/W0A/W45 step above, inject failure before effect and after
effect, cancellation/end before the step and after callback return, deadline at
the next fresh sample, and failure at each following verification read. Include
role disable, mode, wait, enable, recovery04/00, reset command and ACK separately.
Assert no subsequent write after failure, unchanged desired input, correct
PARTIAL/INDETERMINATE evidence, retained terminal result, byte/callback ceilings
and no premature trust. An unambiguous address-NACK guarantee can prove no effect
for that attempt only; it cannot erase an earlier write.

Implementation owners: transport9, codecs10, engine11, observation/proof12,
apply13, reset/recovery14, snapshots15, IRQ16, diagnostics17, regression/timing18–19,
adapters23/25 and physical fixtures30–31. Future source Doxygen and phase comments
must carry exact revision/section/page, units, effect limits and policy distinctions.
No TODO silicon rule, hidden replay engine or unspecified retry remains in these
enabled plans. Named unresolved gates and physical qualification remain explicit.
