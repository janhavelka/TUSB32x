# Variant capabilities and board prerequisites

Prompt 2 evidence review: 2026-10-23. TUSB320LAI is the primary acceptance target.
This source-backed capability specification distinguishes implemented software
from physical qualification. Register encodings are specified in the
[register guide](register-map.md); prompt 4 completed the
[device procedures](device-behavior.md).
Public values, validators and supplied-byte `decodeRegister` now exist, with
private transport/identity and pure codec/composition checks through prompt 10.
The owner now implements initialization, profile observation and read-only
reconciliation, explicit apply and desired verification for all seven I2C
variants, subject to the gates below. Full identity/control/effect proof can
establish KNOWN actual settings;
this does not grant write permission or exact-part identification from a shared
signature. Other device operations and CLI names below remain planned contracts.
Apply changes only differing fields, verifies the complete final image and
retains independent termination/effect evidence after interruption. Pure decoding
retains raw TUSB322I fault evidence; no 08/09 write gate is closed by decoding
or comparison. Existing equivalent DRP00/11 is preserved during unrelated
changes; a real non-reset return to DRP remains unavailable.

## Evidence and current sources

**TI fact** means a statement in the cited exact TI document, subject to its
conditions and conflicts below. **Code observation** describes a pinned public
implementation. **Library policy** is our selected behavior, not a silicon claim.
**Unresolved** names an affected feature and evidence needed to close it.
**Hardware result:** none. An ACK or a passing fake does not qualify a device.

Printed page numbers are used throughout. These source IDs identify exact files
in [the source archive](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/README.md); full hashes, original terms,
retrieval and current-download verification times are in
[provenance.json](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/source/provenance.json).

| ID | Exact source | Relevant source locations |
| --- | --- | --- |
| B | SLLSEN9F, March 2022, TUSB320/TUSB320I | Pins p4; electrical pp5–7; modes pp10–14; programming/map pp15–19; initialization p27 |
| L | SLLSEP2D, May 2017, TUSB320LI/TUSB320HI | Revision history p2; pins pp3–4; electrical pp4–6; modes pp10–14; programming/map pp15–20; initialization p28 |
| A | SLLSEQ8D, May 2017, TUSB320LAI/TUSB320HAI | Pins p3; electrical pp4–6; modes pp10–14; programming/map pp15–20; initialization p28 |
| V | SLLSEO5D, March 2021, TUSB322I | Pins p3; electrical pp4–7; modes/VCONN pp9–14; programming/map pp15–20 |
| G0 | SLLSEO6C, August 2018, TUSB321 | Features p1, pins p3: GPIO only |
| G1 | SLLSER2A, May 2017, TUSB321AI | Features p1, pins p3: separate GPIO-only part, called TUSB321A in summaries |
| N | SLAAE24, May 2021 | Family Table 2-1 p3; startup §4.1 p5; non-failsafe pins §5 p7; IRQ §8 p9 |
| G | SDAA284, March 2026 | Family Figure 2-1 p8; comparison Figure 2-2 and selection Figure 2-3 p9 |

All seven previously archived PDFs matched fresh official downloads byte for
byte. G1 was added without replacing an old source. Current TI product pages
still link the revisions above. No separate applicable silicon errata was found
in the inspected product document lists and targeted TI searches; that is a
bounded search result, not proof of absence. See the dated
[current-source review](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/README.md#prompt-2-current-source-review).

## Exact part boundary

Addresses and revision bytes are hexadecimal; addresses are seven-bit values.
These are **TI facts**, with the unsupported-access restrictions selected as
**library policy**. Each named variant remains explicit even where a small
internal descriptor can share verified behavior.

| Exact variant | ADDR low / high | Enable, pin 11 | Revision at A0 | Operating VDD; free-air temperature | Capability boundary and source |
| --- | --- | --- | --- | --- | --- |
| TUSB320 | 0x60 / 0x61 | EN_N, active low | Undocumented; never probe | 2.7–5.0 V; 0–70 °C | Base map, unattached mode change, explicit post-dead-battery Rp recovery; B pp4–7,14–19 |
| TUSB320I | 0x60 / 0x61 | EN_N, active low | Undocumented; never probe | 2.7–5.0 V; −40–85 °C | Same base boundary; industrial suffix changes temperature, not enable polarity; B pp4–7,14–19 |
| TUSB320LI | 0x47 / 0x67 | EN_N, active low | 0x01 | 2.7–5.0 V; −40–85 °C | Extended termination/mode controls, Try preferences and UFP accessories; TI selection caution below; L pp3–6,10–20 |
| TUSB320HI | 0x47 / 0x67 | EN, active high | 0x01 | 2.7–5.0 V; −40–85 °C | LI capability map, distinct enable voltage/polarity; same selection caution; L pp3–6,10–20 |
| TUSB320LAI | 0x47 / 0x67 | EN_N, active low | 0x02 | 2.7–5.0 V; −40–85 °C | Primary target; extended map with independently checked electrical limits; A pp3–6,10–20 |
| TUSB320HAI | 0x47 / 0x67 | EN, active high | 0x02 | 2.7–5.0 V; −40–85 °C | LAI capability map, distinct enable voltage/polarity; A pp3–6,10–20 |
| TUSB322I | 0x47 / 0x67 | EN_N, active low | 0x02 | 4.5–5.5 V; −40–85 °C | Separate identity, reset-to-DRP, DIR/VCONN, no PORT or 45; per-operation gates below; V pp3–7,9–20 |
| TUSB321 (TUSB321I in G summary) | Not I2C targets | No enable pin | Not applicable | Outside this I2C API | GPIO pins replace the interface; G0 pp1,3 and G Figure 2-1 p8 |
| TUSB321AI (summary name TUSB321A) | Not an I2C target | No enable pin | Not applicable | Outside this I2C API | Separate exact-part source, no SDA/SCL/ADDR/EN; G1 pp1,3 |

TI currently advises against LI/HI in DFP or DRP configurations on the
[LI product page](https://www.ti.com/product/TUSB320LI) and
[HI product page](https://www.ti.com/product/TUSB320HI). This is a current
**TI product recommendation**, not evidence that the documented registers do not
exist. **Library policy:** retain their documented capability definitions for
existing consumers, expose the caveat in capability/help/integration documentation,
and use LAI for new DFP/DRP examples and acceptance. Do not claim LI/HI DFP/DRP
qualification or add a speculative override API. A staged DRP profile is a
software candidate, not a recommendation or an automatic hardware write.

## Identity and revision policy

**TI facts:** ascending offsets `00..07` are
`30 32 33 42 53 55 54 00` on all six 320 variants (B Table 7-7 p16; L/A Table 7
p16), and `32 32 33 42 53 55 54 00` on 322I (V Table 6-7 p17). TI prints them
in descending offset order. The 322 vector has no ASCII `I`. L/A Table 12 p20
and V Table 6-11 p20 give the revisions in the matrix.

**Library policy:**

- The application selects the exact variant and strap address. `bind` validates
  that pair without traffic; it never scans, guesses a variant or changes pins.
- `startInitialize` (`init` / `initialize` / `probe`) reads and compares all eight
  identity bytes, including the terminator, and reads A0 only where documented.
  Report configured part, transfer result, signature validity/match and raw
  revision validity/value separately. A successful transfer is not proof of a
  positive address-ACK phase unless the backend actually supplies that evidence.
- Accept only the configured part's documented revision: LI/HI 01, LAI/HAI and
  322I 02. An unexpected revision, including public code's 06, remains visible
  but fails initialization with `CHIP_ID_MISMATCH`; it cannot establish KNOWN
  configuration or enable writes. A failed read retains its transport error and
  does not manufacture a zero revision. Base parts have no revision expectation.
- A mismatch stops normal initialization after identity evidence; do not proceed
  to speculative control reads or writes. Cached diagnostics and explicit raw
  reads of documented identity bytes/A0 remain available. No raw-write escape
  bypasses a known identity/revision mismatch. A later successful explicit
  initialization can replace the failed identity evidence.
- Future revisions require an updated exact-source audit and tests before
  acceptance. No numeric greater-than rule, automatic fallback or user flag
  expands the capability set. Neither 02 nor a shared signature distinguishes
  enable polarity, package wiring, exact model or board supply.

Keep this within the existing initialization, diagnostics and status contract;
do not introduce a second probe API, discovery manager or plugin registry.

## Pins, straps and power ownership

**TI facts:** the six 320 parts use CC1/CC2 pins 1/2, PORT3, VBUS_DET4, ADDR5,
INT_N/OUT3 pin6, SDA/OUT1 pin7, SCL/OUT2 pin8, ID9, GND10, enable11, VDD12.
322I replaces pin3 with VBUS_DET and pin4 with DIR; the other numbers agree.
There is no PORT pin on 322I despite a stray PORT mention in V Table 6-4 p13.
Shared 12-pin packaging is not proof of board interchangeability.
Sources: B Table 5-1 p4, L/A §5 p3, V Table 4-1 p3.

| Board input | TI fact | Library/integration rule |
| --- | --- | --- |
| ADDR | Low/high selects the matrix address; floating selects GPIO and disables I2C. Pull high to device VDD. B p4, L/A/V p3 | Stable strap before enable; no floating-ADDR I2C selection or live address-change promise. No precise resampling time beyond the cited evidence is invented. |
| PORT on six 320 parts | Low=UFP, floating=DRP, high=DFP. Sampled at active enable with valid VDD and after software reset; B p4/§7.4.1 p14, L/A p3/§7.3.1 p13 | Owner records strap and startup role. A later pin transition is not a continuous role command. |
| Tri-level inputs | Low ≤0.4 V; middle 0.28–0.56 × VDD; high ≥VDD−0.3 V (L/A/V upper bound VDD). External pull-up 4.7 kΩ typical. B §6.5 p6, L §6.5 p5, A/V §6.5/§5.5 pp5–6 | Use these pin-specific values, not generic MCU logic thresholds. Six-part PORT/ADDR internal pulls are removed after enable sampling (B p7, L/A p5 footnote). Keep straps valid; do not assume an active internal bias remains. |
| Active-low enable | EN_N low enables, high/floating shuts down; internal pull-up 1.1 MΩ typical. B pp4,6; L/A pp3–5,14; V pp3,5,14 | Board-owned; unresolved L/A externally driven startup sequence below. |
| HI/HAI active-high enable | Low/floating disables; EN high range 1.05–3.65 V; internal pull-down 500 kΩ typical; EN absolute max 4 V. L pp4–5, A pp3–5 | Do not tie EN to a 5 V VDD merely because EN_N and ADDR use VDD-level high. L initialization p28 names 1.8/3.3 V EN; A electrical p5 independently sets the same EN range. |
| VDD ramp / readiness | Six-part VDD ramp ≤25 ms (B p4, L p4, A p3); I2C available within 100 ms of active enable and valid VDD (B §6.7 p7, L/A §6.7 p6, V §5.7 p7) | Owner supplies already-ready hardware before initialization. 100 ms is not attachment readiness or proof of a valid startup waveform. N §4.1 p5 gives family ramp guidance; do not invent a 322-specific EN hold specification. |
| INT_N / ID / DIR | Open-drain observations; INT_N active low in I2C mode; DIR exists only on 322I within this I2C scope | Application owns pull-ups, IRQ input and deferred service. Core does no GPIO setup or I2C in an ISR. |

## Electrical and dead-battery boundary

These are **TI facts** or explicitly named conflicts, not runtime measurements.
Values labelled min/typ/max retain that ordering. Absolute maxima are stress
limits, never recommended operating targets. The core controls no rail, VBUS
switch, charger, mux, enable signal or USB power/data policy.

| Quantity | Exact boundary | Source |
| --- | --- | --- |
| Local 3 A advertisement | All six 320 parts require VDD ≥3.5 V; 322I still requires its full 4.5–5.5 V operating range | B §6.5 footnote p7; L/A §6.5 footnote p5; V §§5.3/5.5 pp4,6 |
| Remote 3 A detection | The 3.5 V footnote qualifies local DFP advertisement, not UFP detection. Do not prohibit LAI detection at a qualified 3.3 V supply | A §6.5 p5 and §7.2.2 p11 |
| I2C rail and input | Pull-up rail 1.65–3.6 V; input high minimum 1.05 V, input low maximum 0.4 V. For 3.3 V I2C require VDD >3.0 V throughout use | B §6.5 p7; L/A §6.5 p6; V §5.5 p6; stricter electrical footnote governs over adjacent ≥3 V prose |
| Non-I2C open-drain pull-up | 200 kΩ typical for INT_N/OUT3 and ID; not an I2C pull-up recommendation or a stated maximum | B §6.5 p6; L §6.5 p5; A §6.5 p6; V §5.5 p6 |
| VBUS network | System VBUS 4/5/28 V min/nom/max; external resistor 855/887/920 kΩ min/typ/max (900 kΩ in prose); internal pulldown 95 kΩ typical | B §§6.3/6.5 pp5,7; L/A §§6.3/6.5 pp4,6; V §§5.3/5.5 pp4,6 |
| VBUS detection | System threshold 2.95/3.3/3.8 V min/typ/max with that network; VBUS_DET pad absolute maximum 4 V (V also states recommended 0–4 V) | Same sources, absolute-max tables B p5, L/A/V p4 |
| DFP-only sensing exception | Six 320 datasheets permit omitting VBUS_DET connection only for exclusively DFP use, never a later UFP/DRP role. No corresponding exception established for 322I | B §7.3.5 p13; L/A §7.2.5 p12; V §6.3.6 p13 |
| Dead-battery Rd | 4.1/5.1/6.1 kΩ min/typ/max, both CC pins; VDD absent with or without cable | B §6.5 p6/§7.4.3 p14; L/A §6.5 p5/§7.3.3 p14; V §5.5 p5/§6.4.4 p14 |
| Ordinary Rd | 4.6/5.1/5.6 kΩ min/typ/max for UFP/DRP | B §6.5 p6; L/A §6.5 p5; V §5.5 p5 |
| Shutdown | Supply remains present, enable deasserted; Rd remains. Shutdown is not disconnected CC pins | B Table 7-4 p14/§7.4.4 p15; L/A §7.3.4 p14; V §6.4.3 p14 |
| Post-dead-battery recovery | Explicit Rp recovery is documented only for base 320/I. No transfer is possible merely because dead-battery Rd exists | B §7.4.3 p14; L/A §7.3.3 p14; V §6.4.4 p14 |
| CC source signaling | Default 64/80/96 µA; medium 166/180/194 µA; high 304/330/356 µA min/typ/max | B §6.5 p6; L/A §6.5 p5; V §5.5 p5; these are not VBUS load amps |
| VCONN on 322I | Autonomous output from VDD to unused CC after source attachment with Ra; removed on detach. §6.3.8 requires a 5 V source at VDD after Attached.SRC; §5.3 lists VCONN 4.75–5.5 V with VDD at 5 V. Current limit 225/300/375 mA; FET resistance 1.25 Ω maximum; VDD bulk 10–200 µF | V §5.3 p4, §5.5 p6 and §6.3.8 p13; board-owned supply prerequisite, not a core switching sequence or a VCONN guarantee over the whole 4.5–5.5 V device range. No software-enable register established |
| VCONN on six 320 parts | Not provided; active-cable indication is not measured/powered VCONN | N Table 2-1 p3, G Figure 2-2 p9, exact B/L/A feature descriptions |

| Generation | Exact non-failsafe list and supply-off constraints |
| --- | --- |
| B: 320/I | VBUS_DET, ADDR, PORT, ID and OUT1–3; follow device-VDD pull-ups, resistor-isolated VBUS sensing and I2C rail sequencing. B p4, §7.4.3 p14, initialization p27 |
| L: LI/HI | Same listed pins as B; L p3, §7.3.3 p14, initialization p28 |
| A: LAI/HAI | VBUS_DET, ADDR, PORT and OUT1–3; ID omitted from the list and a leakage condition is specified at VDD=0, ID=5 V. A pp3,5,14,28. Do not replace the exact list with N's generic ID warning |
| V: 322I | DIR, VBUS_DET, ADDR and OUT1–3; ID omitted and leakage condition specified at VDD=0, ID=5 V. V §6.4.4 p14/§5.5 p5; no PORT |

A/V ID supply-off evidence does not erase their absolute-max table's ID entry
at VDD+0.3 V. Keep that inconsistency visible: general supply-off 5 V ID wiring
qualification needs TI clarification. Device-VDD pull-ups avoid depending on
that exception. V §5.3's broad 5.5 V control-line row also does not override the
specific 3.6 V I2C rail limit or VDD-referenced pad absolute maxima.

## Timing boundary for later procedure and adapter work

| Quantity | TI value and meaning | Source |
| --- | --- | --- |
| SCL | 400 kHz maximum | B §6.6 p7; L/A §6.6 p6; V §5.6 p6 |
| Data setup / hold | 100 ns / 10 ns minimum | Same tables |
| START setup / hold; STOP setup | 0.6 µs minimum each | Same tables |
| STOP-to-START bus free | 1.3 µs minimum | Same tables |
| Rise / fall | 300 ns maximum each | Same tables |
| Bus capacitance | 400 pF maximum at ≤100 kHz; 100 pF at 400 kHz (B labels the second row ≤400 kHz) | Same tables; retain exact table conditions |
| Data / ACK valid | L/A/V 0.9 µs maximum; B prints 0.9 ns and remains unresolved | Same tables |
| DRP period / default source duty | 50/75/100 ms min/typ/max; 30% typical default source duty | B §6.7 p7; L/A §6.7 p6; V §5.7 p7 |
| CC / VBUS debounce | CC default typical 133 ms B, 168 ms L/A/V; VBUS typical 2 ms, no maximum established by that row | Same switching tables |
| Reset duration | 26/49/95 ms min/typ/max | Same switching tables; not proof of full attachment readiness |

The owner-clock 1 ms margin is **library policy**, not another TI limit. The
device guide defines wait anchors, fixed procedures and retained effects.

## Unresolved gates and source conflicts

Each row names the dependent claim. Independent identity/read-only work continues.
No unspecified capability becomes enabled because another family member has it.

| ID | Evidence/conflict | Selected gate or correction; evidence needed |
| --- | --- | --- |
| STARTUP_EN | L p4/A p3 say externally driven EN_N low for ≥50 ms after valid VDD; N §4.1 p5 says high. B p4 says high; HI/HAI EN separately says low | No universal GPIO startup recipe or qualified externally driven LI/LAI startup claim. Board owner supplies a documented, qualified power/enable sequence or obtains exact-part TI clarification. Core remains pin-neutral and starts only after owner-established readiness |
| SUPPLY_SUMMARY | L/A §6.3 p4 says 5.0 V max; N Table 2-1 p3 and G Figure 2-2 p9 say 5.5 V | Use exact 5.0 V operating ceiling. Above-5.0 V operation is unqualified until authoritative correction; 6 V absolute max is irrelevant to this permission |
| ENABLE_SUMMARY | N/G same comparison tables call HI active low; B §6.7 p7 calls TUSB320I EN high | Exact pin tables establish HI active high and both base parts active low. No industrial-suffix polarity inference |
| L_CC_RATING | L p4 lists CC1/CC2 under both VDD+0.3 and 6 V; p2 revision history says A→B changed them to 6 V | Preserve both and the intended correction. No unqualified pad-tolerance claim beyond the conservative table constraints without clarified TI rating |
| ID_OFF_RATING | A/V p4 lists ID under VDD+0.3; A/V p5 specifies leakage at VDD=0, ID=5 V; N/G identify ID fail-safe | Do not claim unrestricted independent ID pull-up supply; require clarification for a design relying on supply-off tolerance. Ordinary VDD-tracking pull-ups remain usable |
| BASE_VALID_TIME | B §6.6 p7 prints 0.9 ns, L/A/V print 0.9 µs | Do not silently amend source or claim validated 0.9 ns timing. Exact-part TI correction/qualified physical timing needed for that claim |
| ATTACHED_CURRENT | B §7.3.1.2 p11 recommends explicit periodic reset; L/A §7.2.1.2 p10 and V pp10–11 describe one report while attached; G Figure 2-1 p8 advertises active detection from L generation | Fresh register reads are not proof of fresh physical current detection. No hidden reset in read/init/IRQ. Clarification or controlled exact-part attached-current traces needed |
| REVISION_06 | Betrusted code names 06 in a commented-out revision assertion; L/A/V exact tables establish only 01/02 | Reject unexpected revision for initialization/writes under the policy above. Need TI revision identification and capabilities before accepting it |
| V_ADVERTISEMENT | V Figure 6-3 p17 says RW while Table 6-8 says R for advertisement | Raw observation remains available; changing 322 source-current advertisement is unavailable. Require resolved TI access evidence; unchanged proved field must not block unrelated supported changes |
| V_FAULT_CLEAR | V Table 6-9 pp18–19 places RCU fault bit3, W1C IRQ bit4, RW duty bits2:1 and RW UFP-accessory bit0 in the same byte; only IRQ has an explicit clear transaction | Retain raw fault; no claim that read, IRQ ACK or a guessed write clears it. Read-only means no software write, not proved non-destructive fault observation. Gate every 09 write, including ACK/service, duty, UFP-accessory and raw writes, until exact fault handling and safe byte composition are established in prompts 3/4. Unchanged proved 09 controls must not block independently supported changes in other registers |
| V_RETURN_DRP | V §6.3.2 p9 states reset-to-DRP; pp10–11 give explicit DFP/UFP recipes; Table 6-10 p20 defines DRP codes without a separate return recipe | Direct non-reset return to DRP unavailable until separately justified. No hidden reset fallback in apply. Explicit reset has its own documented role effect |
| V_PORT_TEXT | V Table 6-4 p13 mentions PORT; pin table p3 has none | Reject FOLLOW_PORT_PIN on 322I. Do not invent a pin; exact mode00 is DRP |
| TYPE_C10_DRP | B §7.3.1.1 p11, L/A §7.2.1.1 p10 and V §6.3.2.1 p10 exclude a Type-C 1.0 DRP peer with local DFP despite broad front-page compatibility text | Do not promise universal Type-C 1.0 interoperability; retain exact exception in examples/qualification |

**Code observations:** the verified TI-hosted Linux driver is GPL-2.0 code
attributed to National Instruments/Michael Auchter, not TI-authored silicon
guidance. Its reset-mask and cleanup/error behavior, the Betrusted 06/Try.SRC
claims and incomplete emulator behavior are counterexamples, not test oracles.
See [focused peer/public source review](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-2-peers.md) and the
[pinned manifest](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/public-implementations.json). No external code was
copied into a driver. The [feature coverage](feature-coverage.md) maps each
planned capability and gate to its implementation-stage evidence.
