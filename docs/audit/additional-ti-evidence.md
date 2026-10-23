# Additional TI silicon evidence for the promptset

Reviewed 2026-10-23. This is a source audit for future implementation, not driver
or hardware validation. Exact archived datasheet tables were inspected with
PyMuPDF; the electrical and timing tables were also rendered to verify column
alignment and units. Current official TI PDF links and a TI support clarification
were checked. Register facts are requirements only for their stated variants.

## Sources and evidence rules

| ID | Official source | Relevant printed pages |
| --- | --- | --- |
| B | [TUSB320/TUSB320I, SLLSEN9F, March 2022](https://www.ti.com/lit/ds/symlink/tusb320.pdf) | Pins 4; conditions 5; electrical/timing 6–7; functionality 10–14; programming/map 15–19 |
| L | [TUSB320LI/HI, SLLSEP2D, May 2017](https://www.ti.com/lit/ds/symlink/tusb320li.pdf) | Pins 3–4; conditions 4; electrical/timing 5–6; functionality 10–14; programming/map 15–20 |
| A | [TUSB320LAI/HAI, SLLSEQ8D, May 2017](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf) | Pins 3; conditions 4; electrical/timing 5–6; functionality 10–14; programming/map 15–20; initialization 28 |
| V | [TUSB322I, SLLSEO5D, March 2021](https://www.ti.com/lit/ds/symlink/tusb322i.pdf) | Pins 3; conditions 4; electrical/timing 5–7; functionality 11–14; programming/map 15–20 |
| N | [Application issues, SLAAE24, May 2021](https://www.ti.com/lit/an/slaae24/slaae24.pdf) | Family comparison 3; startup 5; non-failsafe pins 7; interrupt clear 9 |
| G | [CC controller selection, SDAA284, March 2026](https://www.ti.com/lit/pdf/sdaa284) | Generations and family differences 8–9 |
| E | [TI E2E: TUSB320LAI UFP datasheet clarification](https://e2e.ti.com/support/interface-group/interface/f/interface-forum/1529213/tusb320lai-datasheet-clarification-for-ufp-application) | Ryan Kitto's final correction, after checking internally; web discussion, no PDF page |

Archived PDF hashes and retrieval metadata remain in
[provenance.json](../reference/source/provenance.json). Source IDs below refer to
these exact revisions. A support reply supplements the datasheet; it is not a
new datasheet specification. Later replies can correct earlier ones, including
an earlier reply by the same TI engineer. Record the author and final applicable
answer rather than copying a search snippet.

The E2E page's direct web reader returned HTTP 403 during lead verification. Its
indexed content exposed the final corrected TI response; it was used only to
corroborate A's exact DFP supply footnote, not to establish a new specification.
Re-open the complete discussion when relying on it during implementation.

The implementation agent must check each inherited fact against the exact part,
source revision, page, field access, unit and condition. Public driver code is
evidence of what its author implemented, not proof that the silicon behaves that
way. Resolve contradictions explicitly. Keep an unsupported operation gated;
never substitute an invented constant, guessed reset value, fake observation or
an undocumented register transaction. Correct every affected specification,
constant, test vector, example and guide when evidence changes.

## Concrete additions for prompts 2–4 and their implementation stages

| Claim | Exact evidence | Consequence for prompts, code or tests |
| --- | --- | --- |
| Seven-bit addresses are `60/61` for base 320/I and `47/67` for LI/HI, LAI/HAI and 322I. | B §7.5 p15; L/A §7.4 p15; V §6.5 p15 | Store seven-bit addresses. Derived address-phase bytes are `C0/C1`, `C2/C3`, `8E/8F`, `CE/CF` for write/read respectively. Do not pass these eight-bit values to callback APIs expecting seven bits. |
| `0x45` in the base recovery recipe is a register offset, not an additional I2C target address. | B §7.4.3 p14 and Table 7-7 p19 | Recovery writes the configured base device, pointer `45`, then data `04` or `00`. Add a transaction vector rejecting accidental access to device address `45`. |
| Floating ADDR selects GPIO mode; there is no responding I2C interface in that mode. | A pin table p3; L pin table p3; B pin table p4; V pin table p3 | No software scan or identity retry can repair a board's floating ADDR strap. Report board prerequisites in examples. |
| PORT is sampled at enable/reset/power-up, including software reset. It is not a continuously polled role input. | A pin table p3, §7.3.1 p13; L §7.3.1 p13; B pin table p4 and §7.4.1 p14 | Distinguish sampled startup selection from explicit I2C mode settings; do not promise immediate mode changes from a later PORT pin transition. |
| ID bytes are described in descending offset order in the datasheets. | B Table 7-7 p16; L/A Table 7 p16; V Table 6-7 p17 | Ascending reads are `30 32 33 42 53 55 54 00` for 320 or `32 32 33 42 53 55 54 00` for 322. The 322 bytes contain no ASCII `I`, despite the accompanying part-name prose. Test both byte order and terminator. |
| Extended revision values are `01` for LI/HI and `02` for LAI/HAI; 322I also has `02`. | L/A Table 12 p20; V Table 6-11 p20 | Revision alone cannot distinguish LAI/HAI from 322I or determine enable polarity. Base 320/I has no documented A0 register. |
| Sequential transfers and a one-byte pointer are documented; pointer-selection prose ends with STOP. | B §7.5 pp15–16; L/A §7.4 p15; V §6.5 pp15–16 | Specify the selected adapter transaction explicitly. Do not require repeated START solely because another driver uses it, and do not rely on a prior read's implicit pointer. No auto-increment wrap or access to undocumented holes is proved. |
| Software reset resets digital logic and may affect detected current, active cable, accessory, attachment and direction. | B Table 7-7 p19; L/A Table 10 p19; V Table 6-10 p20 | Self-clear of bit `0A[3]` is not proof that all control bytes returned to power-on defaults. Do not invent bulk register reset behavior or automatically reapply the profile. |
| 322I returns to DRP on EN_N high-to-low or software reset. | V §6.3.2 p9; DFP/UFP application descriptions pp25, 27 | Reset cannot promise to retain requested DFP/UFP operation. This specific effect does not establish a complete power-on register image or exact 0A reset byte. |
| `45[1:0]` are writable TI-internal bits with an explicit restriction against changing their default value. Extended tables show default `00`. | B Table 7-7 p19; L/A Table 11 p20 | Never expose them as profile settings. The base recovery operation's pre-read/nonzero rejection is a conservative library rule, not an additional TI-mandated step. Do not silently repair an observed nonzero internal value. |
| `0A[0]` disables CC termination and holds the CC state machine disabled; `45[2]` disables Rd/Rp. | L/A Tables 10–11 pp19–20 | Keep these two controls distinct. A clear `0A[0]` does not establish enabled termination if `45[2]` is set or unknown. 322I has the first control but no documented `45`. |
| The 5 ms role-change wait follows the mode write, with termination disabled before the write and restored afterward. The procedure applies while attached or unattached. | L/A §7.3.2 p14 | Test separate writes and the wait anchor. Setting mode and disabling termination in the same first write is not the stated sequence. Base 320/I mode writes remain restricted to unattached state. |
| A valid sink attachment needs both CC qualification and VBUS detection. | A §7.2.1.2 p10 and §7.2.5 p12; B §7.3.1.2 p11 and §7.3.5 p13; V §6.3.2 p11 | CC presence alone is not a completed attachment. Fake fixtures should model VBUS/CC independently rather than instantly producing Attached.SNK. |
| Active-cable detection is a DFP/source-role feature on the six 320 variants. | A/L Table 1 p10; B Table 7-1 p11 | Retain the raw flag, but qualify semantic applicability by the observed local role. It does not prove VCONN is available: the six 320 parts do not provide it. |
| Base accessory support is not equivalent to the extended variants' UFP accessory support. | B §7.3.3 p12 and Table 7-4 pp13–14; L/A §7.2.3 p11 and Tables 4, 8–9 | Do not infer base UFP debug/audio support from shared accessory codes. Base accessory code `7` stays reserved. |
| Try.SRC/Try.SNK is a preference during DRP connection, not a guaranteed role or a negotiated USB PD role swap. Equal preferences at both ends do not guarantee the requested outcome. | A/L §7.2.1.3 p11; V §6.3.2 p11 | Tests distinguish requested preference from actual observed role and do not treat a legal opposite-role attachment as transport failure. |
| Default-current indication does not encode whether the system is USB 2 or USB 3. | A/L §7.2.2 p11; B §7.3.2 p12; V §6.3.3 p11 | Preserve a default category; do not synthesize an unconditional 500 or 900 mA authorization. Detected code `2` is the special charge-through 500 mA category, not 3 A. |
| Base TUSB320/I recommends periodic software reset to update detected current from CC while attached; GPIO OUT1/2 keep their initial indication. | B §7.3.1.2 p11 | A fresh register read does not refresh physical detection. Document TI's recommendation as application policy using an explicit reset, never hidden behavior in refresh, initialization or IRQ service; do not generalize it to later generations. |
| A 3.5 V supply minimum applies to the local device advertising 3 A as DFP, not merely detecting a remote 3 A advertisement as UFP. | A §6.5 footnote p5 and §7.2.2 p11; E final corrected response | Examples may use 3.3 V in a qualified LAI UFP design. Do not add a false API restriction forbidding 3 A detection with 3.3 V local supply. Core code cannot measure or prove supply voltage. |
| 322I's DIR output reports CC orientation; it replaces PORT, and role `00` selects DRP. | V pin table p3, §6.3.7 p13, Table 6-10 p20 | Do not carry over a PORT strap or PORT-selected profile mode. DRP encodings `00` and `11` have the same stated starting role. |
| 322I supplies VCONN from VDD onto the unused CC line only after source attachment and detection of Ra on that line; detach removes it. | V §6.3.8 p13 | Observed active cable, local source attachment and autonomous VCONN behavior are distinct. Do not add a fictitious VCONN software-enable register or claim that reading an active-cable bit powers the cable. |
| 322I VCONN current limit is min/typ/max `225/300/375 mA`; FET on resistance is typical `1.25 Ω`. VDD bulk capacitance is `10–200 µF`. | V §5.5 p6, visually checked | Hardware fixture design can use these as qualified limits. They are neither programmable driver settings nor precision current readings. Preserve min/typ/max labels. |
| Interrupt status `09[4]` is W1C; writing zero has no clearing effect. | B access-tag Table 7-6 p16 with Table 7-7 p18; A/L access-tag Table 6 p16 with Table 9 p18; V Table 6-9 p19; N §8 p9 | Test explicit clear with actual control preservation. The appnote's wording about clearing INT_N must not be misread as pulling the active-low physical pin low to deassert it. Observe/recheck the register/pin rather than promising no simultaneous reassertion. |
| The extended 320 interrupt description specifically refers to changed RU fields; base 320 and 322I text refers more broadly to CSR changes. | L/A Table 9 p18; B Table 7-7 p18; V Table 6-9 p19 | Do not build an all-variant fake in which every successful settings write necessarily causes the same interrupt event. Keep each stated source and any unverified effects visible. |
| VBUS_DET is a resistor-coupled sensing input, not a pin permitted direct connection to 28 V. | A pin table p3, absolute maximum p4 and §7.2.5 p12; V pin table p3 and §5.3 p4 | System VBUS's 28 V range depends on the resistor network. A's VBUS_DET absolute maximum is 4 V. Board examples must not confuse the system rail limit with the pad limit. The documented DFP-only omission exception applies to 320 variants (A/L §7.2.5, B §7.3.5); V §6.3.6 p13 requires the resistor connection and supplies no such exception. |

## Timing and electrical values to carry into the source matrix

These values establish adapter and board requirements. They are not instructions
to add bit-banged bus timing, power control or delays to the portable core.

| Quantity | Verified value and qualification | Evidence |
| --- | --- | --- |
| I2C frequency | Maximum 400 kHz | B §6.6 p7; L/A §6.6 p6; V §5.6 p6 |
| Data setup and hold | Minimum 100 ns and 10 ns | Same timing tables |
| START setup, START hold, STOP setup | Minimum 0.6 µs each | Same timing tables |
| Bus-free interval between STOP and START | Minimum 1.3 µs | Same timing tables |
| SDA/SCL rise and fall | Maximum 300 ns each | Same timing tables |
| Bus capacitance | Maximum 400 pF at up to 100 kHz; 100 pF at 400 kHz | Same timing tables; retain each table's exact frequency wording |
| Data/ACK valid delay | L/A/V maximum 0.9 µs; base B prints 0.9 ns | Explicit conflict below; do not normalize silently |
| DRP cycle | Minimum/typical/maximum 50/75/100 ms | B §6.7 p7; L/A §6.7 p6; V §5.7 p7 |
| Default DRP source duty | Typical 30%; programmable codes are 30/40/50/60% | Switching tables plus register tables |
| VBUS debounce | Typical 2 ms, not a specified maximum | Same switching tables; do not make it an owner-operation timeout guarantee |
| Default CC debounce | Typical 133 ms base, 168 ms extended/322I | Same switching tables; register debounce choices remain nominal |
| Enable-to-I2C availability | Maximum 100 ms with enable asserted and valid VDD | Same switching tables; does not promise completed USB attachment |
| Software reset duration | Minimum/typical/maximum 26/49/95 ms | Same switching tables; the library's 1 ms clock margin is an independent software choice |
| VDD ramp | At most 25 ms | A pin table p3; L pin table p4; N §4.1 p5 |
| I2C pull-up rail | 1.65–3.6 V; tables identify 1.8 V and 3.3 V operation | A/L §6.5 p6; B §6.5 p7; V §5.5 p6 |
| 3.3 V I2C condition | Datasheet footnotes require VDD above 3.0 V; some adjacent prose says at least 3.0 V | Same tables and respective I2C-control sections; retain the conservative board prerequisite rather than inventing a precise equality guarantee |
| VBUS detect external resistor | Min/typ/max 855/887/920 kΩ; pin/application prose commonly says 900 kΩ | B §6.5 p7; L/A §6.5 p6; V §5.5 p6; do not confuse a typical table value with an exact mandatory resistor value |
| System VBUS detection threshold | Min/typ/max 2.95/3.3/3.8 V with the specified network | Same electrical tables; not a direct VBUS_DET pad voltage rating |
| LAI/HAI VDD operating range | 2.7–5.0 V per exact recommended-operating table | A §6.3 p4; conflicts with N's summary 5.5 V upper value |
| 322I VDD operating range | 4.5–5.5 V | V §5.3 p4; do not inherit LAI's lower operating voltage |
| LAI/HAI dead-battery Rd | Min/typ/max 4.1/5.1/6.1 kΩ | A §6.5 p5; not a driver-calibration input |
| LAI/HAI ordinary UFP/DRP Rd | Min/typ/max 4.6/5.1/5.6 kΩ | A §6.5 p5; distinguish from dead-battery tolerance |
| LAI/HAI CC source-current examples | Default 64/80/96 µA; medium 166/180/194 µA; high 304/330/356 µA, min/typ/max | A §6.5 p5; these are CC signaling currents, not delivered VBUS current |

## Conflicts that must remain visible

| Conflict | Evidence and disposition |
| --- | --- |
| Supply maximum | L/A §6.3 p4 specifies 5.0 V maximum; N Table 2-1 p3 summarizes 5.5 V. Preserve the exact datasheet limit for library integration guidance pending an authoritative correction; record both. A 6 V absolute maximum is not an operating rating. |
| Active-high versus active-low startup | A/L EN_N pin text says low for 50 ms after valid VDD, while B pin table p4 and N §4.1 p5 say high for externally controlled EN_N. EN's active-high variant has its own low requirement. Do not impose one universal GPIO sequence; document the unresolved board requirement. |
| Base enable label | B §6.7 p7 describes TUSB320I EN high, but its pin table p4 identifies active-low EN_N. This is not evidence that industrial-temperature suffix I changes enable polarity. |
| Base timing unit | B §6.6 p7 visibly prints 0.9 ns for both data-valid and ACK-valid delay; L/A/V corresponding tables print 0.9 µs. Record the apparent unit inconsistency and seek clarification; do not claim 0.9 ns as a validated software/hardware requirement or silently amend the vendor document. |
| Attached-current refresh | A §7.2.1.2 p10 describes one report in Attached.SNK, while G Figure 2-1 p8 attributes active current detection to LI/HI generation. The promptset already identifies this issue. A fresh register read proves a fresh read, not necessarily a freshly measured peer advertisement. Qualification needs the exact variant and controlled attached-current changes. |
| 322I advertisement access | V Figure 6-3 p17 labels 08[7:6] RW and the prose describes programming it, while Table 6-8 says R. Existing narrow write gate remains appropriate until resolved with stronger evidence. |
| 322I fault clear | V Table 6-9 p19 explicitly defines write-one clearing for IRQ bit 4 but leaves VCONN_FAULT bit 3 at RCU without an equally explicit clearing mechanism. Do not infer read-clear, W1C, or a combined IRQ/fault clear from another family or generic enum naming. Keep dependent operations gated. |
| Pointer persistence | B/L/A/V read prose allows the previous sub-address plus one, while following notes describe offset zero absent addressing. Explicit pointer setup for every logical read avoids relying on either ambiguous implicit state. |
| Appnote HI enable summary | N Table 2-1 p3 calls TUSB320HI active low, contradicting L's exact pin table. Prefer the exact part's pin table and preserve the summary error in the source audit. |
| 322I debug accessory prose | V §6.3.4.2 p12 describes peer DFP in the `111` case; the register table p18 labels that same case local UFP. Keep peer and local roles explicit. These descriptions need not conflict once perspective is named. |

## Review and regression implications

- Prompts 2–4 should seed exact source facts and named uncertainties from this
  review, then independently verify them. Facts must not become true merely
  because this audit or a public library repeats them.
- Prompts 9–10 need address-phase, identity order, supported-range, mixed-access,
  reserved-code and reset-write vectors; derive expected bytes independently of
  the codec being tested.
- Prompts 13–16 need variants' separate role/recovery/reset sequences, local/peer
  role meanings, current applicability, effect retention and IRQ reassertion.
- Prompts 23–26 and 30–31 need real board supply/strap/framing prerequisites and
  fixtures that distinguish CC indication from actual VBUS/VCONN behavior. Mark
  physical checks NOT RUN when no suitable fixture exists.
- Prompts 35–38 should check every public support claim against the final source
  matrix, operation gates and actual test evidence. Do not replace unknowns with
  plausible defaults to finish a prompt.

No implementation was compiled, no device was accessed, and no physical timing,
current, voltage or USB interoperability claim was validated in this review.
