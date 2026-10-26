# Independent register vectors

Prompt 3, 2026-10-23. These are literal expected values derived from the exact TI
tables in [register-map](register-map.md), independently of future production
helpers. They are inputs for later behavioral suites, not passed driver tests.
Hexadecimal bytes omit `0x`. B/L/A/V have the exact variant meanings and sources
defined in that guide. All non-target fields and prerequisites are assumed valid
unless the case explicitly says otherwise; no vector overrides a capability gate.

## Identity, ranges and field decoding

| ID | Variant/input | Required result |
| --- | --- | --- |
| D01 | Each of six 320 parts; 00..07 = 30 32 33 42 53 55 54 00 | Complete signature match; exact configured variant still not inferred |
| D02 | V; 00..07 = 32 32 33 42 53 55 54 00 | Complete V signature match; no ASCII I byte |
| D03 | Six 320 configured, V vector; or V configured,320 vector | CHIP_ID_MISMATCH after complete successful identity read |
| D04 | Reversed320 vector = 00 54 55 53 42 33 32 30 | Mismatch, not a host-endian string match |
| D05 | Correct first seven320 bytes, last byte01 | Mismatch; terminator participates |
| D06 | L revision01; A/V revision02 | Accepted documented revision after correct signature |
| D07 | L revision02; A/V revision01; any extended/V revision06 | Raw revision retained; CHIP_ID_MISMATCH, no write permission |
| D08 | Failed/partial identity transfer | Preserve transport failure; never compare an invented zero-filled identity |
| D09 | All; read00 length8,08 length3,07 length4,0A length1 | Inside the contiguous map; framing proof belongs to prompt 4 |
| D10 | All;08=90 | Advertisement MA_3000; detected MA_1500; distinct fields |
| D11 | All;08=20 versus00 | CHARGE_THROUGH_500_MA versus DEFAULT detection category |
| D12 | All;08=C0 | Reserved advertisement code; rawC0 retained, advertised-field validity clear |
| D13 | All;08=02/04/06 | Reserved accessory codes1/2/3; raw retained, accessory validity clear |
| D14 | All;08=08 /0A | AUDIO /CHARGE_THROUGH (audio charged-through accessory) |
| D15 | B;08=0C /0E | DEBUG /reserved; never infer extended local-UFP debug |
| D16 | L/A/V;08=0C /0E | DEBUG_LOCAL_DFP /DEBUG_LOCAL_UFP |
| D17 | All;08=01 | Raw active-cable flag1; no powered-VCONN claim or context inferred |
| D18 | All;09=00/40/80/C0 | DETACHED /LOCAL_SOURCE /LOCAL_SINK /ACCESSORY |
| D19 | All;09=40/60 | Local-source raw direction CC1/CC2; connector mapping stays external |
| D20 | All;09=20 | Raw direction1 but detached; full observation uses NOT_APPLICABLE orientation/current |
| D21 | All;09=00/02/04/06 | DRP source-duty30/40/50/60%; independent of actual attachment |
| D22 | V;09=BD | LOCAL_SINK, rawCC2, IRQ1, fault1, duty50%, UFP-accessory disabled; retain full raw byte, no safe ACK byte |
| D23 | B;09=BD | Same attachment/direction/IRQ/duty; bits3/0 reserved, not V fault or accessory controls |
| D24 | B;0A=00/40/80/C0 | Nominal debounce133/116/151/168 ms |
| D25 | L/A/V;0A=00/40/80/C0 | Nominal debounce168/118/134/152 ms |
| D26 | Six320;0A=00/10/20/30 | FOLLOW_PORT_PIN /UFP /DFP /DRP requested modes, not observed attachment |
| D27 | V;0A=00/10/20/30 | DRP /UFP /DFP /DRP; preserve raw00 versus30 |
| D28 | L/A/V;0A=00/02/04/06 | NORMAL /TRY_SNK /reserved /TRY_SRC preference |
| D29 | L/A/V;0A=01 | disableTerm true; termination and state machine disabled |
| D30 | B/L/A;45=04 | disableRdRp true; distinct from0A disableTerm |
| D31 | L/A;0A=00 and45=04 | Clear disableTerm does not establish enabled Rd/Rp |
| D32 | V; compare0A02 with32 | All persistent settings equal, including equivalent DRP mode |
| D33 | V; compare0A02 with30 | Mode equal but preference differs; whole settings unequal |
| D34 | Six320; compare0A00 with30 | Modes unequal: FOLLOW_PORT_PIN versus DRP |
| D35 | L/A/V;0A04 reserved preference | Retain raw04; invalid preference must not manufacture a complete valid profile |
| D36 | B; defaults08 underFE,09 underE6,0A underF0,45 under04 | Compare only documented defaults00/20/00/00; other bits unspecified, not required zero |

## Byte composition

These cases check arithmetic only. `Actual` is the observed byte; the selected
persistent/protected mask removes live/read-only bits. `Update` is a complete
user field mask and `Field` is its already-shifted contribution. An ordinary
write has Command00; explicit ACK/reset uses only its own command mask. The
expected value is fixed source-derived data, not a call to the implementation's
encoder. Runtime admission, identity, complete control validity, transient-state
proof and prompt 4 procedures remain separate requirements.

| ID | Group | Reg | Actual | Update | Field | Command | Expected | Purpose |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| C01 | B | 08 | BF | C0 | 40 | 00 | 40 | 1.5 A advertisement; do not echo live3F |
| C02 | L/A | 08 | BF | C0 | 00 | 00 | 00 | Default advertisement; no status echo |
| C03 | L/A | 09 | B5 | 00 | 00 | 10 | 15 | ACK actual05 controls, regardless of staged desired02 |
| C04 | B | 09 | B5 | 00 | 00 | 10 | 14 | Base bit0 is reserved, not preserved user control |
| C05 | L/A | 09 | 06 | 00 | 00 | 10 | 16 | Fixed11 would change actual duty/accessory controls |
| C06 | L/A | 09 | B5 | 06 | 02 | 00 | 03 | Duty40%, preserve accessory-disable1, IRQ0 |
| C07 | B | 09 | BD | 06 | 02 | 00 | 02 | Drop reserved09 and sampled IRQ10 |
| C08 | L/A | 09 | B5 | 01 | 00 | 00 | 04 | Enable UFP accessories, preserve duty50%, no ACK |
| C09 | L/A | 0A | DE | C0 | 40 | 00 | 56 | Debounce code1; never replay reset as5E |
| C10 | B | 0A | E7 | C0 | 40 | 00 | 60 | Base debounce code1; drop reserved07 |
| C11 | L/A | 0A | E2 | 00 | 00 | 08 | EA | Reset bit set, actual persistent fields retained |
| C12 | B | 0A | E7 | 00 | 00 | 08 | E8 | Reset with base persistentF0 only |
| C13 | V | 0A | C2 | 00 | 00 | 08 | CA | Explicit V reset encoding, not completion proof |
| C14 | V | 0A | 02 | 06 | 06 | 00 | 06 | Try.SRC only; retain equivalent DRP00, not gratuitous36 |
| C15 | L/A | 0A | 32 | 06 | 06 | 00 | 36 | Try.SRC with existing DRP11 retained |
| C16 | L/A | 0A | 36 | 01 | 01 | 00 | 37 | Set disableTerm without losing preference/mode |
| C17 | L/A | 45 | 03 | 04 | 04 | 00 | 07 | Preserve protected03; arithmetic is not qualification of an abnormal image |
| C18 | B | 45 | 07 | 04 | 00 | 00 | 03 | Clear disable only; no repair of protected03 |
| C19 | B | 45 | 00 | 04 | 04 | 00 | 04 | Literal recovery first byte only with independently qualified prerequisites |
| C20 | B | 45 | 04 | 04 | 00 | 00 | 00 | Literal recovery final byte; preceding wait/procedure still required |
| C21 | B | 0A | C0 | 30 | 20 | 00 | E0 | DFP encoding retains168 ms debounce; actual mode change needs unattached state |

C09's observed reset bit and C17/C18's internal bits deliberately exercise the
composer's preservation/exclusion arithmetic. They do not authorize an ordinary
write while reset is unresolved or an abnormal internal image is unqualified.
C19/C20 are not a two-write shortcut to the full base recovery sequence.

For each C row, verify the expected byte against the exact variant masks and
independent integer arithmetic. Also verify no bits outside the legal output
mask are set, and no ordinary write includes IRQ/reset. These checks were run
in prompt 3; actual callback transcripts and operation effects are future tests.

## Required rejection and failure vectors

Admission-time argument/access/capability rejections produce no callback or write,
preserve output arguments and do not increment transport totals. Validation during
polling may reject after a required read; that callback still counts even when
no write follows, as in N18. N24/N25 instead exercise actual callback failures.
These are future test requirements. Method-specific status precedence remains
the baseline and prompt6 contract; this table adds no alternate error vocabulary.

| ID | Request or condition | Required rejection/evidence |
| --- | --- | --- |
| N01 | Any variant; register0B,44,46,9F,A1 orFF | Unmapped register; no guessed contiguous map |
| N02 | B; read/write A0 | Absent revision access, never a probe |
| N03 | V; read/write45 | No Rd/Rp register; false profile placeholder cannot authorize access |
| N04 | Any; write00..07 or presentA0 | Read-only target, even for a no-change byte |
| N05 | Range length0;08 length4;0A length2;45 length2;A0 length2;FF length2 | Empty range, hole, absent successor or wrap; no partial transfer |
| N06 | B; Try.SNK/Try.SRC, disableTerm=true, disableUfpAccessory=true | Unsupported fields; not writes to reserved bits |
| N07 | B; debounce118/134/152; L/A/V debounce116/133/151 | Unsupported variant/time pair |
| N08 | Typed advertisement outside DEFAULT/MA_1500/MA_3000; raw advertisementC0 | Invalid/reserved setting; retain a raw read as evidence instead |
| N09 | L/A/V requested preference field04 | Reserved code2; Try.SRC is06 |
| N10 | V FOLLOW_PORT_PIN | No PORT pin and mode00 is DRP |
| N11 | V any08 write, including unchanged byte or default00 | V_ADVERTISEMENT unresolved access; no raw-write loophole |
| N12 | V any09 write: ACK, duty, UFP-accessory or raw | V_FAULT_CLEAR; no guessed fault-preserving byte |
| N13 | V any09 write after successful read20 with fault0 | Same rejection; a fault may arise between observation and transfer |
| N14 | V actual08/09 proved unchanged, apply changes only supported0A field | Do not reject solely because08/09 are gated; no08/09 writes, independent procedure rules still apply |
| N15 | V non-reset request changing actual mode to DRP | V_RETURN_DRP gate; no hidden reset fallback |
| N16 | A required persistent field is unknown/invalid | Apply/raw write rejects until required evidence exists; managed ACK/reset may read their safe image during polling, but cannot write an unproved image or use staged/default bits |
| N17 | Raw write sets RO/reserved bits or changes45 protected03 | Reject the requested image; do not silently mask or normalize it |
| N18 | Base recovery observes 45 protected bits nonzero | After the required pre-read, CONFIGURATION_UNKNOWN with zero writes; count the completed read callback |
| N19 | Raw mode/reset/termination/recovery write needing a multi-step procedure | Reject shortcut; one callback cannot carry the required operation proof |
| N20 | Ordinary settings operation while observed reset remains unresolved | Composition can omit08, but operation must not declare reset complete or replay it |
| N21 | Linux reset update: actualC2 orCA, mask08,value01 | Helper emitsC2 in both cases; negative oracle: bit3 is never asserted |
| N22 | Extended actual09=06; proposed ACK11 | Wrong image changes controls; correct explicit ACK is16 |
| N23 | Correct signature but unexpected revision; later write request | Failed initialization cannot authorize writes or KNOWN configuration |
| N24 | Successful read followed by failed write/ACK | Preserve original failure and possible effects; no success claim from prior read or automatic retry |
| N25 | Read callback fails after modifying its scratch buffer | Caller output/previous complete observation unchanged; partial data is not published |

N14 is a positive admission boundary placed beside the gated cases to prevent
over-broad rejection. N21 is a helper defect demonstration, not a library call.
Prompt 9 tests N01-N05/N11-N13 access boundaries and N25 scratch-output
preservation through the private transport. Its write fixtures inject before/after
effects with the same backend error; the managed multi-step N24 sequence remains later work.

Later test owners: protocol/identity ranges in prompts9/18; exhaustive codecs and
write images in10/18; profile/reset/recovery/IRQ effects in13/14/16/18; port
applicability in15/18; safe raw CLI access in21/22. Extend these seeds with every
small field code and failure boundary. Do not generate expected values with the
production codec or let a permissive fake define silicon behavior.
