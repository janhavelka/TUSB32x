# TI family audit and implementation evidence

Audit date: 2026-10-23. This is preparation for an implementation promptset, not a driver or a claim of hardware validation. Read the exact selected device's complete datasheet before implementation. Register facts below are intentionally compact; the source-location index identifies the material that must be re-audited.

Correction audit, 2026-10-23: re-reading the archived base datasheet and visually checking printed p14 identified the previously omitted dead-battery exit sequence. The requirements below now include that procedure and distinguish a complete port observation from later interrupt-service failure. No driver or hardware validation was performed.

## Source provenance

Official PDF bytes are archived in [reference/source](../reference/source/). [provenance.json](../reference/source/provenance.json) records source URLs, document identifiers, revision dates, retrieval timestamps in UTC, byte counts, and SHA-256 hashes. TI retains ownership of these documents. A filename or a shared marketing prefix is not evidence of register compatibility.

| Evidence | Revision | Locations to re-audit |
| --- | --- | --- |
| [TUSB320/TUSB320I datasheet](https://www.ti.com/lit/ds/symlink/tusb320.pdf) | SLLSEN9F, March 2022 | Pin table p4; timing p7; modes pp13-14, dead-battery exit §7.4.3 p14; programming §7.5 pp15-16; Table 7-7 pp16-19; initialization p27 |
| [TUSB320LI/TUSB320HI datasheet](https://www.ti.com/lit/ds/symlink/tusb320li.pdf) | SLLSEP2D, May 2017 | Pins pp3-4; timing p6; modes pp13-14; programming p15; Tables 7-12 pp16-20; initialization p28 |
| [TUSB320LAI/TUSB320HAI datasheet](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf) | SLLSEQ8D, May 2017 | Pins p3; electrical pp4-6; modes pp13-14; programming p15; Tables 7-12 pp16-20; initialization p28 |
| [Application issues](https://www.ti.com/lit/an/slaae24/slaae24.pdf) | SLAAE24, May 2021 | Family Table 2-1 p3; CC versus PD p4; power sequence p5; non-failsafe pins p7; interrupt clear p9 |
| [CC controller selection guide](https://www.ti.com/lit/pdf/sdaa284) | SDAA284, March 2026 | Family diagram p8; generation comparison and selection flowchart p9 |
| [TUSB321 datasheet](https://www.ti.com/lit/ds/symlink/tusb321.pdf) | SLLSEO6C, August 2018 | Features p1; pin table p3; GPIO operation |
| [TUSB322I datasheet](https://www.ti.com/lit/ds/symlink/tusb322i.pdf) | SLLSEO5D, March 2021 | Pins p3; modes pp13-14; Tables 6-6 through 6-11 pp17-20 |

Page numbers are printed PDF page numbers, not zero-based viewer indexes. Critical timing and register pages were also rendered and visually checked during this audit.

## Family boundary

Recommended initial implementation scope: all six explicitly named TUSB320 variants below. Treat TUSB322I as a separately reviewed extension; do not silently include it under `TUSB32x`. The broader name is acceptable only with a prominent supported-device matrix and unsupported-feature behavior.

| Variant | ADDR low/high, 7-bit | Enable | Revision register |
| --- | --- | --- | --- |
| TUSB320, TUSB320I | 0x60 / 0x61 | Low | Undocumented |
| TUSB320LI, TUSB320HI | 0x47 / 0x67 | LI low; HI high | 0xA0 = 0x01 |
| TUSB320LAI, TUSB320HAI | 0x47 / 0x67 | LAI low; HAI high | 0xA0 = 0x02 |

Sources: the three datasheet pin tables and register maps linked above. Floating ADDR selects GPIO operation and disables I2C. The ID string is shared across these six parts; it cannot determine enable polarity. Use explicit variant configuration and report identity/revision separately.

TUSB321/TUSB321A are GPIO controllers, not I2C targets. TUSB322I supports I2C and VCONN; it is not a GPIO-only exclusion. Other devices, including HD3SS3220 and TUSB422, need independent scope and register audits. [Selection guide, Figures 2-1 through 2-3](https://www.ti.com/lit/pdf/sdaa284)

## Compact register evidence

### Shared base

Ascending ID bytes, 0x00..0x07: `30 32 33 42 53 55 54 00`.

| Register:bits | Meaning / encoding |
| --- | --- |
| 08:7-6 | Advertise: 0 default; 1 1.5A; 2 3A; 3 reserved |
| 08:5-4 | Detect: 0 default; 1 medium; 2 charge-through 500mA; 3 high |
| 08:3-1 | Accessory: 0 none; 1-3 reserved; 4 audio; 5 charge-through; 6 debug |
| 08:0 | Active cable |
| 09:7-6 | Attachment: 0 none; 1 local source; 2 local sink; 3 accessory |
| 09:5 | Orientation: 0 CC1; 1 CC2 |
| 09:4 | Interrupt, W1C |
| 09:2-1 | DRP source duty: 30/40/50/60% |
| 0A:5-4 | Mode: PORT/UFP/DFP/DRP |
| 0A:3 | Self-clearing reset command |
| 45:2 | Disable Rd/Rp; preserve 45:1-0 |

Base-only: `09:0`, `0A:2-0`, and accessory code 7 are reserved; mode changes require unattached state. Debounce `0A:7-6` encodes 133/116/151/168ms. [SLLSEN9F, Table 7-7, pp16-19](https://www.ti.com/lit/ds/symlink/tusb320.pdf)

Base TUSB320/TUSB320I also requires a separate dead-battery exit procedure to present Rp on both CC pins after VDD returns: write `0x45=0x04`, wait 30 ms, then write `0x45=0x00`. Ignore status flags during those steps. Dead battery includes VDD absent with no cable connected; this is not solely a discharged-battery case. [SLLSEN9F, §7.4.3 p14](https://www.ti.com/lit/ds/symlink/tusb320.pdf)

Selected software contract: `startRecoverDeadBattery(timing, token)` executes that procedure explicitly in the existing operation engine after successful identity validation. The application owns stable supply/I2C readiness and decides when source-capable startup needs it. Read-only initialization does not perform or prove recovery. Read `0x45` first and reject nonzero internal bits `[1:0]` before writing; then the literal bytes also respect the internal-default rule. Wait at least 31 elapsed millisecond ticks from a fresh completion anchor for the 30 ms TI interval, including the separate 1 ms clock margin. Verify `[2:0]==0` after the final write. Retain possible effects on failure/cancellation, never publish transient status as ordinary port observations, and never infer timed completion from a final zero alone. This contract is an engineering choice; it adds neither automatic recovery nor a separate power-policy state machine.

Do not apply that workaround to LI/HI, LAI/HAI or TUSB322I. Their dead-battery sections omit it, and TUSB322I does not document `0x45`; the proposed operation is unsupported there. [SLLSEP2D, §7.3.3 p14](https://www.ti.com/lit/ds/symlink/tusb320li.pdf), [SLLSEQ8D, §7.3.3 p14](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf), [SLLSEO5D, §6.4.4 p14](https://www.ti.com/lit/ds/symlink/tusb322i.pdf)

### Second generation additions

LI/HI adds `09:0` UFP-accessory disable; `0A:2-1` source preference 0 normal/1 Try.SNK/2 reserved/3 Try.SRC; `0A:0` termination/state-machine disable. Accessory 6/7 distinguishes local DFP/UFP debug. Debounce becomes 168/118/134/152ms. Mode change: set termination-disable; set mode; wait 5ms; clear termination-disable. `45:2` remains separate. [SLLSEP2D, section 7.3.2 and Tables 8-12, pp14,17-20](https://www.ti.com/lit/ds/symlink/tusb320li.pdf)

### Third generation and timing

LAI/HAI retains that control map. I2C maximum: 400kHz. Enable-to-I2C availability maximum: 100ms. Soft reset: 26/49/95ms minimum/typical/maximum. Advertising 3A requires VDD >=3.5V. Shutdown retains Rd; it does not mean disconnected CC pins. PORT is sampled at enable/power-up and soft reset. Transactions use an 8-bit register pointer; documented pointer-setting includes STOP. [SLLSEQ8D, sections 5, 6.5-6.7, 7.3-7.5](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf)

### TUSB322I extension gate

TUSB322I uses 0x47/0x67, active-low enable, ID ascending `32 32 33 42 53 55 54 00`, revision 0x02. It adds `09:3` VCONN fault (RCU); has no PORT pin; mode 00 means DRP. Its map does not document 0x45. Supply: 4.5-5.5V. Table 6-8 labels advertisement access R while its figure says RW and description says programmable: resolve before claiming write support. [SLLSEO5D, pin table and Tables 6-6 through 6-11](https://www.ti.com/lit/ds/symlink/tusb322i.pdf)

Enable/soft reset defaults to DRP. Sections 6.3.2.1-2 prescribe disable-term/set-mode/enable-term, without the TUSB320LI/LAI 5ms step. Timing gives 100ms I2C availability maximum and 95ms reset maximum. VCONN is autonomous upon qualifying source attachment. [SLLSEO5D, sections 5.7, 6.3.2, 6.3.8](https://www.ti.com/lit/ds/symlink/tusb322i.pdf)

Recommendation: make the extension audit mandatory, then implement supported operations with variant-specific behavior if the evidence closes. Otherwise explicitly list its unresolved operations and keep them unavailable. The observed differences justify a capability branch, not a different framework or duplicated driver. Do not call the whole device incompatible merely because some evidence needs clarification.

## Conflicts and unresolved points

1. **Enable startup level:** LAI/LI pin text says externally controlled EN_N stays low for 50ms after valid supply; SLAAE24 section 4.1 says EN_N stays high for at least 50ms. The latter also permits permanently grounding EN_N. Record this conflict explicitly; do not encode an invented universal GPIO startup pulse. [Application issues p5](https://www.ti.com/lit/an/slaae24/slaae24.pdf) and [TI response confirming the high-level rule for TUSB320I](https://e2e.ti.com/support/interface-group/interface/f/interface-forum/1158566/tusb320-en_n-functionality). A board adapter must document its reviewed supply/enable sequencing; externally managed power is a valid core contract.
2. **Current changes while attached:** the LAI sink description says detection happens once per attachment, while the newer selection guide lists active current detection for second generation onward. Do not promise a dynamic tracking guarantee without resolving exact part/silicon behavior. [SDAA284 p8](https://www.ti.com/lit/pdf/sdaa284). Read actual registers, preserve raw information, and add a hardware experiment covering source current changes without unplugging.
3. **Overview errors:** SLAAE24 Table 2-1 says HI enable is low; its datasheet says high. Some overview supply and leakage figures also differ from datasheets. Pin/electrical specifications for the exact part take precedence over a portfolio summary; retain discrepancies in the implementation evidence log. [Application issues Table 2-1](https://www.ti.com/lit/an/slaae24/slaae24.pdf)
4. **Read transaction choice:** the programming prose has a contradictory read-cycle label on an R/W=0 pointer write, and descriptions of the implicit pointer differ. Explicitly set the pointer for every read. Document STOP-separated versus repeated-START behavior in the port contract, verify the chosen adapter on hardware, and never claim repeated-START is mandated by this datasheet.
5. **Snapshot atomicity:** this audit found no specified latch guaranteeing atomic capture across status bytes. A burst read alone must not be advertised as an atomic event snapshot. Nor does a single interrupt bit provide an event queue or event count. This is a limit of the available evidence, not a newly asserted chip behavior.
6. **Reset effects:** audit affected fields and PORT re-sampling separately from a complete power-on-default reset. Do not assume soft reset clears all configuration or proves that the port is ready immediately when the bit changes.
7. No dedicated silicon errata document was established by this audit. That is not evidence that none exists. Re-check official product documentation and relevant TI support at implementation time, especially before resolving the above conflicts.

## Requirements inferred for the library and its tests

These are engineering recommendations derived from the evidence, not TI-specified software architecture.

### Capability and identity tests

- Encode a capability table keyed by explicit variant. Separate address validity, revision-register availability, UFP accessory support, role-change procedure, debounce encoding, enable polarity, and default-mode meaning.
- Check the complete byte-order-specific signature. A bus ACK is not a valid identity. Test reversed ID, partial reads, unrelated device responses, future revision values, variant/address mismatch, and identical signatures across variants.
- Do not touch undocumented registers on unsupported variants, including revision probing on the base device. No broad bus scan as a normal initialization side effect.
- Report unsupported operations before any transaction. Avoid aliases that silently flatten different debounce milliseconds or electrical behavior.

### Register write tests

- Derived configuration masks for six TUSB320 variants: `08=0xC0`; base `09=0x06`, extended `09=0x07`; base `0A=0xF0`, extended `0A=0xF7`; reset command `0A=0x08`; acknowledgement `09=0x10`. Re-derive these from the selected source before implementing.
- Generic read-modify-write must never replay a set W1C bit while changing duty/accessory settings. Explicit acknowledgement must preserve configuration, and an unrelated setter must not acknowledge an event.
- Acknowledgement preserves current observed/trusted applied RW fields, not unapplied desired settings. Test a staged profile different from the hardware profile while an interrupt is pending. If current RW state is unknown, observe/reconcile it before composing ACK or report that the operation cannot proceed.
- Treat self-clearing command bits separately from persistent configuration. Changing debounce/preferences must not replay a reset read as set.
- Preserve TI-internal reserved fields in 0x45. The explicit base recovery operation first requires them to be zero and then uses the prescribed literal writes; unexpected values are rejected without attempted repair. Do not replace the documented mode-change procedure with this different register.
- Test every documented encoding, reject reserved writes, retain unknown raw status values, and cover a hardware-updated status byte changing between the read and write phases.

### State, timing, and transport tests

- Use a fake monotonic clock and scripted transport. Verify no reads before readiness, reset completion/timeout, the required mode-switch pause, clock wraparound, cancellation, and no holding a shared bus lock across a timed wait.
- Base-mode change while attached must fail without writing. Extended-mode change must exercise each step and each injected failure. A partial write sequence must surface uncertain configuration/state instead of pretending the requested role became active.
- Base dead-battery recovery tests require identity first, exact `0x45` reads/writes, rejection of nonzero internal bits and other variants, the 30 ms hardware wait plus clock margin, no publication of transient status, and failures/cancellation after each write. Readback of zero alone is not evidence that the timed procedure ran.
- A setter changes requested configuration; it does not prove the actual attachment role. Model configured role, observed local role, orientation validity, current advertisement, detected-current validity, and transport health separately.
- A failed observation read cannot create a synthetic detach, zero-current result, or healthy status. A complete successful observation read may publish its snapshot; a later ACK or recheck failure retains that observation with its timestamp and reports the service failure independently. Partial reads never publish, and older observations retain their freshness metadata. The transient flags during base dead-battery recovery are not ordinary publishable observations.
- Acknowledge interrupts explicitly, read back acknowledgement state, and bound any retry/service loop. Keep unserviced work visible when new interrupts continue arriving. Test reassertion during acknowledgement and after the last read. Document coalescing and observation limits.
- This audit found no specified minimum delay between interrupt-clear attempts in the cited interrupt descriptions. Do not transfer the separate role-change 5ms requirement into interrupt handling. A chosen retry interval is software policy, not a TI timing guarantee.
- No I2C in an ISR. An ISR may signal a worker; the application owns scheduling and synchronization. Distinguish polling operation from optional pin notifications.
- Test pointer writes, transfer ordering, stop behavior, short transfers, NACK, timeout, adapter failures, and per-instance independence. Backend callbacks must preserve native error information according to sibling-library conventions.

### Product and integration boundaries

- Keep bus ownership, locks, power rails, GPIO enable control, interrupt setup, VBUS switching, USB controller role setup, charge limits, and recovery policy outside the portable device core.
- Default-current status is a category; do not turn it into an unconditional 500mA or 900mA permission. Board/application policy decides usable current. High-current advertisement also needs the documented supply condition and real source capability.
- No automatic bus recovery, power cycle, aggressive reset-on-read-error, source enable, charger reconfiguration, or PD policy inside the library. Attach/detach is normal device state, not transport health failure.
- A mode reconfiguration is not a USB PD role-swap implementation. This controller family must not be advertised as PD negotiation, e-marker discovery, a VBUS ADC, or a VCONN power controller for the six TUSB320 variants.
- Provide examples for polling, interrupt-to-worker service, nonblocking reset/mode change, and an externally owned shared bus. The portable core must compile with no Arduino/ESP-IDF/RTOS headers. Hardware examples must disclose straps, supply/enable assumptions, selected exact variant, and what they actually validated.
- A future TUSB322I extension needs its own VCONN fault acknowledgement tests, default-mode semantics, supported-register allowlist, identity tests, electrical prerequisites, and conflict resolution. Reusing common code is not evidence of compatibility.
- For that extension, distinguish external power-path policy from autonomous silicon effects. A supported mode/termination/reset operation can change the chip's VCONN behavior; ownership separation must not erase that effect from diagnostics or documentation.
- Board bring-up documentation must separately check I2C pull-up voltage against VDD, rail ramp/ordering, VBUS_DET wiring, and non-failsafe pins in each exact datasheet. A successful register read is not verification of these hardware prerequisites.

## Evidence required before implementation completion

Produce a source-to-requirement-to-test matrix, with exact document revision/page/table and explicit unresolved items. Keep host test results, adapter compile checks, and physical hardware results separate. Record which variant and revision were physically tested. No release claim may convert an untested variant or unresolved datasheet conflict into verified support.
