# Register access and encoding contract

Specified in prompt 3 on 2026-10-23 and implemented through prompt 17.
TUSB320LAI remains the primary target; physical qualification is separate.
Prompt 9 now implements its private access allowlist and identity comparisons;
prompt 10 implements pure codecs/composition. Prompts 11–16 implement owner
procedures; prompt 17 adds safe raw enforcement. Read the
[variant matrix](variant-matrix.md) for identity acceptance, board prerequisites
and unresolved gates, and [register vectors](register-vectors.md) for independent
expected bytes. The [device guide](device-behavior.md) now specifies procedures,
wait anchors and completion proof from prompt 4.

## Implemented pure codec boundary

`decodeRegister` is public in `<TUSB32x/CommandTable.h>` and links through
`TUSB32x`. It replaces its fixed result only on success, retaining the supplied
variant/register/raw byte and separate field-valid masks. Reserved advertisement,
preference and accessory codes keep only their affected flags clear. Base absent
controls have no validity; present identity/revision bytes remain raw only.
Single-byte CC1/CC2 and current codes establish no attachment applicability,
current measurement, USB PD contract or permission to draw power. Complete port
interpretation, including detached NOT_APPLICABLE, belongs to prompt 15.

Private `TUSB32xCodecs` encodes typed candidates and compares persistent settings,
including fieldwise TUSB322I DRP00/11 equivalence. Encoding a readable field does
not authorize writing it. The one `composeWrite` selects whole named fields,
preserves actual unrequested controls and rejects missing/invalid actual fields
even when the request would overwrite them. Explicit ACK/reset intents ignore
staged desired values and use the safe observed/trusted applied image. They may
operate while overall configuration is UNKNOWN; they do not erase older effects.

Ordinary settings cannot change mode. Base mode composition requires the detached
precheck; extended stages require distinct successful disable/mode steps and
the applicable wait proof. Recovery requires base-only stable-VDD qualification,
internal-zero precheck, and preceding disable/wait proof before its literal00.
These are supplied owner proofs, never a second operation engine or permission
for a public raw setter. Ordinary45 also requires known internal03=00: this is
the documented L/A default and conservative B policy. Preservation arithmetic
for a nonzero internal image is not qualification to write it.

An observed reset command bit1 rejects qualified composition; `DE -> 56` tests
field arithmetic separately, while settled `D6 -> 56` is a permitted debounce
composition. Ordinary writes emit no IRQ/reset bits. The registered protocol
suite checks literal bytes and manual `[register,value]` callback frames; it
executes no mode, reset, recovery, wait or IRQ job. TUSB322I08/09 writes and real
non-reset changes to DRP remain gated; equivalent existing DRP00 is preserved
during unrelated updates. No fault-clear API is introduced.

## Sources and notation

Hexadecimal bytes below omit `0x` inside tables. Ranges are inclusive. Page
numbers are printed PDF pages. B and L/A do not include V unless stated.

| Group | Exact variants | Archived TI authority for this map |
| --- | --- | --- |
| B | TUSB320, TUSB320I | SLLSEN9F, March 2022, §7.6 Tables 7-6/7-7 pp16–19 |
| L | TUSB320LI, TUSB320HI | SLLSEP2D, May 2017, §7.5 Tables 6–12, Figures 4–9 pp16–20 |
| A | TUSB320LAI, TUSB320HAI | SLLSEQ8D, May 2017, §7.5 Tables 6–12, Figures 4–9 pp16–20 |
| V | TUSB322I | SLLSEO5D, March 2021, §6.6 Tables 6-6–6-11, Figures 6-2–6-6 pp17–20 |

These exact files and original terms are in [source provenance](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/source/provenance.json).
The register tables were visually inspected, not inferred from public code.
**TI facts** are the field definitions below. **Library policy** selects access
restrictions, byte composition and validity handling. **Unresolved** identifies
a source conflict, not permission to guess. Hardware qualification: none.

R permits software reading; W permits writing; U marks autonomous hardware
updates. B Table 7-6 defines S as write-one set and C as write-one clear, with
zero having no such effect. L/A explicitly describe IRQ write-one clear and
self-clearing reset in Tables 9/10. V explicitly describes IRQ write-one clear,
but its fault RCU label does not define the clearing transaction. Do not transfer
B's C definition to that V fault as if it were V's own statement.

RU is live hardware evidence, not a guarantee of a new physical measurement on
each read. RW settings are persistent relative to an ordinary register access;
this does not promise retention across reset, power loss or external changes.
All runtime identity, trust and lifetime prerequisites still apply.

## Mapped registers and permitted access

| Register | B | L/A | V | Purpose, access and source |
| --- | --- | --- | --- | --- |
| 00..07 | R | R | R | Eight identity bytes; B Table 7-7 p16, L/A Table 7 p16, V Table 6-7 p17 |
| 08 | R, qualified W | R, qualified W | R; every W gated | Advertisement and port information; B p17, L/A Table 8 p17, V Table 6-8 pp17–18 |
| 09 | R, qualified W/ACK | R, qualified W/ACK | R; every W gated | Attachment, direction, IRQ and controls; B p18, L/A Table 9 p18, V Table 6-9 pp18–19 |
| 0A | R, qualified W/reset | R, qualified W/reset | R, qualified W/reset | Mode/debounce and reset; B p19, L/A Table 10 p19, V Table 6-10 pp19–20 |
| 45 | R, qualified W | R, qualified W | Absent; no access | Separate Rd/Rp control and protected internal bits; B p19, L/A Table 11 p20 |
| A0 | Undocumented; no access | R | R | Revision; L/A Table 12 p20, V Table 6-11 p20 |

All other offsets are outside the allowlist. Never scan a hole or probe base A0
or V45. Identity and revision are never writable, even with their existing value.
TUSB321/TUSB321AI are not I2C targets. Addresses remain seven-bit: B 60/61,
L/A/V 47/67; register45 is not an additional target address.

The map permits reads within contiguous 00..0A, or a single present 45/A0.
A private range read must have nonzero length, fit wholly inside one permitted
range and not wrap. Thus 07+4 bytes is mapped; 08+4 crosses into an unmapped hole.
This is a range allowlist, not proof of atomicity; the device guide specifies
the bus procedure. Explicit raw public reads remain single-register operations.
Every read selects its pointer; no inherited pointer or PCA9555 parking logic.

Reading identity, revision or ordinary controls does not command a change.
Reading B/L/A status is not an IRQ ACK: its clear action is an explicit write of
one. V fault-read side effects remain unresolved. Changing 08 advertisement
changes the local source's CC current indication, not a VBUS switch or measured
load. Changing 09 duty/accessory settings changes those CC-policy controls;
explicit IRQ ACK clears the sampled latch without promising lossless events.
B p18 and V p19 describe IRQ assertion on CSR changes broadly; L/A Table 9 p18
specifically names RU-field changes. Do not turn either wording into an invented
guarantee that every settings write creates an interrupt. V fault assertion is
separate evidence. The 0A and 45 field tables below state their distinct reset,
termination and role effects; observation/recheck must establish actual outcomes.

### Identity and documented defaults

| Group | Ascending 00..07 | A0 |
| --- | --- | --- |
| B | 30 32 33 42 53 55 54 00 | Not read |
| L | 30 32 33 42 53 55 54 00 | 01 |
| A | 30 32 33 42 53 55 54 00 | 02 |
| V | 32 32 33 42 53 55 54 00 | 02 |

Compare all eight bytes including NUL. Raw evidence, transfer success, signature
match and configured variant are separate facts. Exact revision acceptance is
the existing variant policy: an unexpected value, including 06, cannot establish
write permission. Common signatures never identify enable polarity or exact BOM.

| Group | Register | Documented default mask | Value under mask | Source and limit |
| --- | --- | --- | --- | --- |
| B | 08 | FE | 00 | B p17 names advertisement/detection/accessory defaults; no bit0 default |
| B | 09 | E6 | 20 | B p18 names attachment/direction/duty defaults; no IRQ or reserved default |
| B | 0A | F0 | 00 | B p19 names debounce/mode defaults; no reset-command or reserved default |
| B | 45 | 04 | 00 | B p19 names normal Rd/Rp default; no numeric internal/reserved defaults |
| L/A | 08 / 09 / 0A / 45 | FF each | 00 / 20 / 00 / 00 | Explicit Reset columns, Tables 8–11 pp17–20 |
| V | 08 / 09 / 0A | FF each | 00 / 20 / 00 | Explicit register/field resets, Tables 6-8–6-10 pp17–20 |

Only L/A explicitly assign 45 internal bits 1:0 default 00. B has no full-byte
reset column; do not fill its unknown bits with zero and call that TI evidence.
V Table 6-8 prints the three-bit accessory reset with a two-bit literal; the
full-byte reset 00 and NONE description independently establish zero, not a
two-bit accessory field. Runtime observations must come from successful reads,
never from these defaults or zero-initialized buffers.

The tables do not establish that I2C software reset restores every register to
its listed reset value. Reset may affect detected current, active cable,
accessory, attachment and direction. V additionally returns to DRP (V §6.3.2 p9).
Direction default1 does not prove an attached CC2 cable. Reset completion,
PORT sampling and recovery sequence proof remain prompt 4 work.

## Separate masks by access and effect

The masks partition each byte. `Protected RW` is not user configuration.
`Unresolved` means readable raw evidence whose write behavior is not established.

| Group | Register | User persistent RW | Protected RW | RU | IRQ W1C | Reset command | Reserved R | Unresolved |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B/L/A | 08 | C0 | 00 | 3F | 00 | 00 | 00 | 00 |
| B | 09 | 06 | 00 | E0 | 10 | 00 | 09 | 00 |
| L/A | 09 | 07 | 00 | E0 | 10 | 00 | 08 | 00 |
| B | 0A | F0 | 00 | 00 | 00 | 08 | 07 | 00 |
| L/A/V | 0A | F7 | 00 | 00 | 00 | 08 | 00 | 00 |
| B/L/A | 45 | 04 | 03 | 00 | 00 | 00 | F8 | 00 |
| V | 08 | 00 | 00 | 3F | 00 | 00 | 00 | C0 |
| V | 09 | 07 | 00 | E0 | 10 | 00 | 00 | 08 |

All bits in identity and present A0 are R. Their read-only mask is FF, with no
write effect. V09's known RW/W1C masks are field facts, not permission to write
the byte: every transfer would also write unresolved bit3. V08's C0 is readable
advertisement with conflicting R/RW claims, not a usable write mask.

Reserved R bits are retained in raw observations but emitted as zero in composed
writes. This does not promise their readback is zero. In contrast, 45[1:0] are
protected writable bits: observe and preserve them; never expose a setting or
repair them to an assumed default. An abnormal internal image does not become
qualified merely because the arithmetic preserves it. Base recovery's required
zero pre-read is a conservative library prerequisite for TI's literal 04/00
recipe, not a claim that base internal bits reset to 00.

Profile comparison has its own masks. It includes readable requested settings
even where changing them is gated, and excludes protected internal bits from
user profile equality. Internal-bit preservation and transient-effect proof
remain separate requirements.

| Group | Compare08 | Compare09 | Compare0A | Compare45 |
| --- | --- | --- | --- | --- |
| B | C0 | 06 | F0 | 04 |
| L/A | C0 | 07 | F7 | 04 |
| V | C0 | 07 | F7, with semantic DRP equivalence | Absent |

Only successfully decoded applicable fields participate. A reserved value is
not equality with a default. V mode normalization is fieldwise as defined below;
none of these comparison masks grants write access or proves reset completion.

## Explicit field encodings

Semantic names follow [ENGINEERING_BASELINE](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/prompts/ENGINEERING_BASELINE.md).
Enum ordinals do not encode registers. The shifted values below are field
contributions, not stand-alone safe write bytes.

| Register/field | Mask; shift | Semantic → unshifted code → shifted contribution | Variants and source |
| --- | --- | --- | --- |
| 08 advertisement | C0; 6 | SourceCurrent DEFAULT→0→00, MA_1500→1→40, MA_3000→2→80; code3/C0 reserved | All readable; B p17, L/A Table 8 p17, V Table 6-8 p17; V writes gated |
| 08 detected current | 30; 4 | DetectedCurrent DEFAULT→0→00, MA_1500→1→10, CHARGE_THROUGH_500_MA→2→20, MA_3000→3→30 | All; B p17, L/A Table 8 p17, V Table 6-8 p18 |
| 08 accessory | 0E; 1 | NONE→0→00; codes1/2/3→02/04/06 reserved; AUDIO→4→08; CHARGE_THROUGH→5→0A | All; same status sources. CHARGE_THROUGH specifically means audio charged-through accessory |
| 08 debug accessory | 0E; 1 | B DEBUG→6→0C, code7/0E reserved; L/A/V DEBUG_LOCAL_DFP→6→0C, DEBUG_LOCAL_UFP→7→0E | B p17, L/A Table 8 p17, V Table 6-8 p18; local, not peer, port type |
| 08 active cable | 01; 0 | false→0→00, true→1→01 | All; same status sources; applicability still required |
| 09 attachment | C0; 6 | DETACHED→0→00, LOCAL_SOURCE→1→40, LOCAL_SINK→2→80, ACCESSORY→3→C0 | B p18, L/A Table 9 p18, V Table 6-9 p18 |
| 09 direction | 20; 5 | CC1→0→00, CC2→1→20 | Same sources; raw chip inputs, not board connector labels |
| 09 IRQ | 10; 4 | sampled clear→0→00, pending→1→10; explicit ACK contributes10 | B Tables 7-6/7-7 pp16,18; L/A Table 9 p18; V Table 6-9 p19, byte write gated |
| 09 VCONN fault | 08; 3 | raw0: no fault indicated; raw1: over-current fault indicated with INT_N asserted | V Table 6-9 p19 only; no typed fault-ACK API or assumed clear action |
| 09 DRP duty | 06; 1 | PERCENT_30→0→00, PERCENT_40→1→02, PERCENT_50→2→04, PERCENT_60→3→06 | All; B p18, L/A Table 9 p18, V Table 6-9 p19; V writes gated |
| 09 disable UFP accessory | 01; 0 | false→0→00, true→1→01 | L/A Table 9 p18, V Table 6-9 p19; absent B, V writes gated |
| 0A debounce, B | C0; 6 | MS_133→0→00, MS_116→1→40, MS_151→2→80, MS_168→3→C0 | B Table 7-7 p19; nominal time, not a hard completion deadline |
| 0A debounce, L/A/V | C0; 6 | MS_168→0→00, MS_118→1→40, MS_134→2→80, MS_152→3→C0 | L/A Table 10 p19, V Table 6-10 p19; nominal time |
| 0A mode, B/L/A | 30; 4 | FOLLOW_PORT_PIN→0→00, UFP→1→10, DFP→2→20, DRP→3→30 | B p19, L/A Table 10 p19; UFP starts unattached.SNK, DFP unattached.SRC, DRP unattached.SNK |
| 0A mode, V | 30; 4 | DRP decodes from0/3→00/30; UFP→1→10, DFP→2→20; FOLLOW_PORT_PIN unavailable | V Table 6-10 p20; both DRP codes start unattached.SNK; no PORT pin |
| 0A reset | 08; 3 | Ordinary write→0→00; explicit reset command→1→08 | B p19, L/A Table 10 p19, V Table 6-10 p20; hardware self-clears |
| 0A preference | 06; 1 | NORMAL→0→00, TRY_SNK→1→02, code2/04 reserved, TRY_SRC→3→06 | L/A Table 10 p19, V Table 6-10 p20; B has no field |
| 0A disable termination | 01; 0 | false→0→00, true→1→01 | L/A Table 10 p19, V Table 6-10 p20; disables CC termination and holds state machine disabled; B absent |
| 45 disable Rd/Rp | 04; 2 | false→0→00, true→1→04 | B p19, L/A Table 11 p20; separate from disableTerm; V absent |

Invalid typed values and unsupported variant/time pairs are rejected locally.
On B, NORMAL/false placeholders for absent preference/accessory-disable/
disableTerm cause no access to reserved fields; other values reject. On V,
disableRdRp=false is only a neutral placeholder, not permission to access 45.
An absent field has no observed-validity bit. A reserved encoded value retains
raw evidence and clears only the affected interpretation's validity. It does
not become a successful default or an I2C failure.

Single-register decoding reports codes. A complete port observation separately
establishes applicability and coherence: detached orientation/current can be
valid NOT_APPLICABLE; failed/unavailable observations are UNKNOWN. Normal source
and sink imply opposite peer roles, but accessory state alone does not establish
a normal peer power role. Base debug must not acquire extended UFP support from
its shared code. Charge-through current is distinct from ordinary USB default;
default is not unconditional 500/900 mA permission. An active-cable flag does not
measure VCONN. CC1/CC2 remain chip pin names; board mapping stays external.

### Complete port interpretation (implemented prompt 15)

`detail::decodePort` reuses `decodeRegister` for field codes, then qualifies them
with final attachment and the fixed bookend comparison. The following is the
library's conservative applicability policy, not a claim that every combination
can occur electrically. Invalid interpretations keep their raw bytes. UNKNOWN
has no validity flag; NOT_APPLICABLE is a valid interpretation, not usable power.

| Final attachment / accessory code | Local / peer role | Orientation | Detected current | Accessory / active-cable validity |
| --- | --- | --- | --- | --- |
| Detached | NOT_APPLICABLE / NOT_APPLICABLE | NOT_APPLICABLE | NOT_APPLICABLE | Accessory NOT_APPLICABLE only for code0, otherwise UNKNOWN; cable invalid |
| Normal source | SOURCE / SINK | Final CC1/CC2 | NOT_APPLICABLE | Accessory NONE and cable valid only for code0; contradictory/reserved accessory UNKNOWN |
| Normal sink | SINK / SOURCE | Final CC1/CC2 | Codes0/1/3 valid only with accessory0; charge-through code2 and contradictory accessory context UNKNOWN | Accessory NONE only for code0; cable invalid |
| Accessory audio4 | NOT_APPLICABLE / NOT_APPLICABLE | UNKNOWN | NOT_APPLICABLE | AUDIO valid; cable invalid |
| Accessory charge-through5 | NOT_APPLICABLE / NOT_APPLICABLE | UNKNOWN | Only code2 gives CHARGE_THROUGH_500_MA; other codes UNKNOWN | CHARGE_THROUGH valid; cable invalid |
| Accessory debug6 | NOT_APPLICABLE / NOT_APPLICABLE | UNKNOWN | NOT_APPLICABLE | B DEBUG; L/A/V DEBUG_LOCAL_DFP; cable invalid |
| Accessory debug7 | NOT_APPLICABLE / NOT_APPLICABLE | UNKNOWN | UNKNOWN | B reserved/UNKNOWN; L/A/V DEBUG_LOCAL_UFP; cable invalid |
| Accessory code0 or reserved1-3 | NOT_APPLICABLE / NOT_APPLICABLE | UNKNOWN | UNKNOWN | UNKNOWN; cable invalid |

All four final attachment codes and IRQ bit4 are independently valid. Local
advertisement codes0/1/2 are valid independently of attachment; reserved3 keeps
an invalid typed placeholder. `activeCable` retains raw08 bit0 even without its
validity flag. Source-only cable applicability with accessory0 is conservative
library policy; it does not add a reserved silicon code. Extended UFP-debug
current is withheld as unresolved paired applicability, not declared inapplicable.
Debug-local context remains in `Accessory`, not an inferred ordinary peer role.

Rechecked printed TI pages on 2026-10-26: B SLLSEN9F Table7-1 and section7.3.1.2 p11,
Table7-7 pp17-18; L SLLSEP2D / A SLLSEQ8D Tables8-9 pp17-18 and accessory
sections7.2.3.1-2 pp11-12; V SLLSEO5D Tables6-8/9 pp18-19, section6.3.4.2 p12 and
section6.3.8 p13. B identifies active-cable detection in DFP context and waits for
VBUS as well as CC before Attached.SNK. Both-CC accessory detection does not
qualify connector orientation. V debug6/7 identifies local DFP/UFP, excludes
charge-through debug support in DRP or UFP mode and has separate autonomous VCONN conditions.
A raw cable flag cannot establish those conditions or prove powered VCONN.

Compare first/final09 with `0xE0`. A changed bookend clears current/accessory/
cable flags and makes current/accessory UNKNOWN even if a detached interpretation
would otherwise be NOT_APPLICABLE. Final09 role/direction/IRQ and08 advertisement
remain independently decodable. Matching bookends do not prove atomicity or that
08 stayed unchanged between reads. Physical current tracking after attachment
remains limited by the documented base-reset recommendation and unresolved
extended wording; refresh never performs that reset or an IRQ acknowledgement.

### TUSB322I mode comparison

Decode mode 00 and mode 11 to the same `PortMode::DRP`. The canonical standalone
encoding of a newly requested DRP is code3, but that is not permission for the
unresolved non-reset return procedure. If mode is already semantically DRP,
preserve its observed raw mode bits while changing an unrelated supported field.
Compare every other applicable persistent field normally; ignore sampled status
and commands. Thus 0A values 02 and 32 are equivalent settings, while 02 and 30 differ in
preference. Keep both original raw images in evidence. No whole-byte masking or
automatic canonicalization may hide other configuration mismatches.

## Write composition and operation boundary

This section is **library policy**, derived from the access masks. Use one small
composer with per-register rules, not unrestricted read-modify-write. Inputs
are proved actual controls plus an explicit requested change. Staged desired
settings are never the source for preserving unrelated bits or ACK composition.
Unknown required controls require observation/reconciliation or local rejection.
Only the requested persistent fields may change; a semantic no-op performs no
settings write. A successful write callback alone does not prove the operation.
Managed ACK/reset may read their required safe image during polling, as the
baseline permits. Apply admission requires known actual controls; raw writes
cannot add a preparatory read within their one-callback bound.

1. Validate variant, address, register, complete field encodings and capability
   gates before any write. A field mask is not permission to write a reserved
   code or an unresolved byte. Raw diagnostics obey the same restrictions.
2. Retain actual persistent fields outside the requested update. Emit zero for
   live/read-only/reserved R bits. Ordinary writes emit zero for IRQ/reset
   command bits even when the observed bit is one.
3. Explicit B/L/A ACK is `(actual09 & persistentMask) | 10`, where the mask is
   06 on B and 07 on L/A. It changes no duty/accessory setting. Do not echo the
   whole observed status byte or use a fixed 11. V has no safe ACK byte yet.
4. Explicit reset is `(actual0A & persistentMask) | 08`, with F0 on B and F7
   on L/A/V. Complete valid controls and the reset operation's prerequisites
   still apply. Merely clearing an observed command bit in arithmetic is not
   proof that an in-progress reset finished.
5. Ordinary qualified 45 composition is `(actual45 & 03) | disableBit`. Preserve
   protected bits without exposing them. Base recovery's literal 04/00 sequence
   separately requires observed internal bits zero and qualified VDD readiness.
   Prompt 4 selects a concrete ordinary-write gate too: known internal03=00.
   L/A documents that default; B's zero prerequisite is conservative policy,
   not a numeric default claim. Unknown/nonzero rejects CONFIGURATION_UNKNOWN
   for a requested45 change, without blocking independent permitted fields.
6. A raw requested byte must already obey the legal image and preservation
   rules; reject attempts to set RO/reserved bits, change protected bits, or
   bypass procedure gates. Do not silently mask an invalid request into success.
   With its one-callback bound, raw writing cannot first fetch missing controls.

Prompt 17 implements these raw boundaries in `TUSB32xDiagnostics.cpp`. A successful
ordinary raw write, even with an unchanged byte, invalidates configuration before
invocation and requires complete proof afterward. The managed semantic no-op rule
above does not suppress explicitly requested raw traffic. Pure ACK preserves
actual controls and profile/port trust. A contradictory protected45 read revokes
that register's write permission independently of user-profile equality; both raw
and typed writes require renewed full proof. See [diagnostics](diagnostics.md).

| Intent | Register contract | Procedure contract |
| --- | --- | --- |
| Change advertisement, duty or debounce | Exact field/table/byte composition; V08/V09 gates apply | Owner-known controls and apply publication/error rules; no hidden ACK/reset |
| Change base mode | Only while unattached (B Table 7-7 p19) | Device guide detached precheck and owner/fixture race prerequisite; current-only changes are distinct |
| Change extended or V mode/termination | Separate disableTerm and mode fields; V return-to-DRP gate remains | Device guide transition sequence, final disable state and waits; no raw shortcut |
| Change45 / recover base dead battery | Internal bits protected; V register absent | Device guide qualified termination/recovery procedures; no direct raw recovery |
| Reset | Explicit command08, preserve permitted controls | Device guide timing, self-clear and effect proof; never automatic replay |
| ACK / service | Explicit W1C with actual controls; V09 gate remains | Bounded observation/ACK/recheck and race/error rules in prompts 4/16 |

Transport errors preserve output and original error evidence. An attempted write
may have affected hardware even if the callback fails; access/composition tables
do not authorize retry or rollback. No core GPIO, bus recovery or USB policy is
introduced by these contracts.

## Conflicts, source corrections and next checks

| Issue | Decision and required evidence |
| --- | --- |
| V08 Figure 6-3 RW and programmable prose versus Table 6-8 R | Keep every V08 write unavailable under V_ADVERTISEMENT. Read/decode remains available; require authoritative exact-part access clarification before enabling writes |
| V09 fault RCU with unspecified clear transaction | Keep every V09 write unavailable under V_FAULT_CLEAR, including ACK/service, duty, UFP-accessory and raw writes. Even a currently clear fault can change before a write. Read-only does not imply non-destructive fault observation; need exact transaction semantics and safe composition evidence |
| V mode00/11 | Semantically equivalent DRP encodings, not proof of the V_RETURN_DRP procedure; unchanged proved08/09 controls do not block independent supported0A changes |
| Base defaults missing complete reset bytes | Retain only the masked defaults above; fake devices must not use unknown zero bits as source-backed facts |
| V accessory reset width typo | Field mask0E and width3 remain; full register reset and NONE default establish zero |
| TI-hosted Linux reset mask08/value01 | Its actual regmap helper computes `(old & ~08) | (01 & 08)`, so no reset bit is set; keep the negative vector, do not copy the call |
| Public raw IRQ echo / reserved Try.SRC04 | Echoing status is not safe composition; Try.SRC is06. Public code is evidence to audit, not the encoding oracle |

Public file revisions/hashes/licenses remain in the
[pinned manifest](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/public-implementations.json) and
[chip evidence](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/prompts/CHIP_EVIDENCE.md).
Implemented codec masks carry TI revision/table/page citations, and the public
decoder documents raw validity, error preservation and zero-I2C bounds.
The independent [vectors](register-vectors.md) now have pure codec and transport
checks through prompt 10. Owner procedures, timing and complete port observations
also have registered host checks through prompt 16, including bounded IRQ
acknowledgement/service. Physical hardware qualification remains separate.
