# 3 — Specify register access and encoding vectors

Prerequisites: prompts 1–2 are complete, including exact variant/source choices.
Read [COMMON](COMMON.md), [ENGINEERING_BASELINE](ENGINEERING_BASELINE.md),
`docs/variant-matrix.md` and the archived TI tables. This prompt finishes
`docs/register-map.md` and independently derived positive/negative vectors.
Procedures and timing belong to prompt 4; production register code comes later.

## Focused re-audit

Check current PCA9555 `CommandTable.h`, write composition and protocol/public
tests; LDC1614 `CommandTable.h` and fake-device tests; ADS1115 field validation and
tests. Inspect relevant AGENTS, examples and registration. Record revisions,
dirty states, exact symbols and choices. Borrow named constants, access/effect
descriptions and independent expected bytes. Reject PCA9555 pointer parking or
paired-register quirks and ADS1115 16-bit framing/conversion timing.

## Derive the map from TI evidence

Visually verify the exact archived tables identified by prompt 2. For every
register record source revision/section/table/page, reset fields, RO/live/RW/W1C/
command access, legal writes, reserved handling, variants and observable effects.
Do not treat one family-wide mask or a generic read-modify-write as sufficient.

| Register | Shared six-part facts | Base 320/I | Extended LI/HI/LAI/HAI |
| --- | --- | --- | --- |
| 0x00..0x07 | Read-only signature | Same 320 signature | Same 320 signature |
| 0x08 | Advertisement RW 0xC0; status 0x3F; documented fields reset zero | Accessory code 6 generic debug, 7 reserved | Debug codes 6/7 identify local DFP/UFP |
| 0x09 | Attachment 0xC0; direction 0x20; IRQ W1C 0x10; duty 0x06 | Persistent RW 0x06; bit 0 reserved | Persistent RW 0x07; bit 0 disables UFP accessory support |
| 0x0A | Debounce 0xC0; mode 0x30; reset command 0x08 | Persistent RW 0xF0; bits 2:0 reserved | Persistent RW 0xF7; preference 0x06; disableTerm 0x01 |
| 0x45 | disableRdRp 0x04; bits 1:0 TI-internal RW, preserve; bits 7:3 reserved RO | Same named control | Distinct from 0x0A termination/state-machine control |
| 0xA0 | Revision only where documented | Unsupported | Read-only; variant-specific expected value |

Record startup fields `08=00`, `09=20`, `0A=00` only where exact sources support
them. Direction reset 1 does not prove an attached orientation. Do not infer that
soft reset restores every byte. Internal 0x45 bits are not user profile settings.

| Field | Unshifted encoding and meaning |
| --- | --- |
| 08[7:6] advertisement | 0 default category; 1 1.5 A; 2 3 A; 3 reserved |
| 08[5:4] detected current | 0 default; 1 medium; 2 charge-through 500 mA; 3 high |
| 08[3:1] accessory | 0 none; 1–3 reserved; 4 audio; 5 charge-through; 6 debug/local DFP on extended; 7 local UFP on extended, reserved on base |
| 08[0] active cable | Raw flag plus variant/context applicability |
| 09[7:6] attachment | 0 detached; 1 local SOURCE/DFP; 2 local SINK/UFP; 3 accessory |
| 09[5] direction | 0 CC1; 1 CC2; typed unknown/not-applicable is separate |
| 09[2:1] DRP duty | 0/1/2/3 = 30/40/50/60% source advertisement time |
| 0A[7:6] base debounce | 0/1/2/3 = nominal 133/116/151/168 ms |
| 0A[7:6] extended debounce | 0/1/2/3 = nominal 168/118/134/152 ms |
| 0A[5:4] six-part mode | 0 PORT selection; 1 UFP; 2 DFP; 3 DRP starting unattached.SNK |
| 0A[2:1] extended preference | 0 normal DRP; 1 Try.SNK; 2 reserved; 3 Try.SRC |

Define explicit semantic-to-byte tables; enum ordinal need not equal encoding.
Reserved values preserve raw evidence and invalidate affected interpretations.
Detached orientation/current can validly decode to NOT_APPLICABLE. Default
current is not unconditional 500/900 mA permission, measured load or USB PD.
Requested mode, actual attachment and peer/local role remain distinct.

## Resolve dangerous access ambiguities

Recheck [CHIP_EVIDENCE](CHIP_EVIDENCE.md) against TI and pinned implementations.
Include independent vectors: `08=90` separates 3 A advertisement from 1.5 A
detection; `08=20` is charge-through detection, not ordinary default. Try.SRC is
shifted `06`, while community `04` is reserved. Base accessory `0C` is generic
debug and `0E` reserved; extended `0E` is local UFP debug. A reset sets bit `08`
in the composed `0A` byte while preserving permitted actual controls:
the TI-hosted call's mask `08`, value `1` cannot set bit 3. Inspect helper
arithmetic, not method names. Correct affected inherited tables and expected
vectors explicitly; do not copy a driver's mistakes into the test oracle.

Specify separate masks for actual persistent settings, status, W1C and reset.
An unrelated settings write emits zero for IRQ/reset commands. IRQ ACK preserves
actual observed/trusted applied controls, never staged desired values. Unknown
controls need observation/reconciliation or local rejection, not a guessed image.
Derive positive and negative byte vectors independently of future encoder code,
including reserved preservation, accidental ACK/reset prevention and variant
rejection. Identify which operations require a procedure rather than one write.

Audit 322I separately: its signature differs, mode 00 means DRP rather than
PORT selection, and no 0x45 is documented. Define semantic equivalence of DRP00
and DRP11 for readback comparison. Resolve 08[7:6] Table 6-8 R versus figure RW
and programmable prose before enabling advertisement writes. VCONN fault 09[3]
is RCU; its consuming/clear meaning is independent of IRQ W1C. Keep the precise
unresolved operations unavailable and retain them in the coverage matrix.

## Deliver, review and commit

Complete the register guide and vector tables, update feature coverage and record
source conflicts with affected API and evidence needed. Optional delegation: a
strong reviewer independently checks mixed-access/322I tables; another agent may
check encodings, with one matrix editor. The lead resolves findings and owns Git.

Check every mask/vector against PDFs and simple independent byte arithmetic;
check links, provenance and consistency with variant gates. Keep expected vectors
available to later behavioral tests without claiming those tests already exist.
Update decisions/progress/validation and affected README/CHANGELOG. Future
CommandTable declarations must carry these citations and validity Doxygen.
Commit and sync the complete register contract on the same branch, verifying
upstream and reporting actual checks/limits before prompt 4.
