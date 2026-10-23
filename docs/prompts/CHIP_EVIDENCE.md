# TUSB32x chip evidence and public-code review contract

Reviewed 2026-10-23. This is a shared reference for the 38 implementation prompts,
not another implementation stage. Read the sections needed for the current task.
It supplies concrete starting facts and counterexamples without making each
numbered prompt repeat the research. No driver or hardware was tested here.

Execution prompt 2 rechecked these sources. Use the completed
[variant matrix](../variant-matrix.md) and [feature coverage](../feature-coverage.md)
for the exact revision policy, current LI/HI DFP/DRP recommendation, electrical
conflicts and named capability gates. The 322I FET resistance below is corrected
to maximum after independently checking the table columns. Earlier dated audits
retain explicit correction notes rather than silently losing their history.

## Verify inherited claims before coding

For each register, address, timing, procedure or support claim being implemented:

1. Identify the exact configured part and applicable silicon revision. Re-open
   its TI datasheet, relevant errata and the actual pinned implementation file.
   Check current official documentation for changes. Read tables and footnotes;
   visually inspect ambiguous columns, units and access labels.
2. Record document/revision/section/table/printed page and source commit/path/
   symbol, the observed value, units, conditions and interpretation. Trace called
   helpers far enough to establish emitted bytes, timeout units and error paths.
   A function name, code comment or search snippet is not sufficient evidence.
3. Distinguish a **datasheet fact**, **implementation observation**, **library
   policy**, **hardware observation**, and **unresolved conflict** in the existing
   feature/decision records. These are review labels, not new runtime statuses.
   A TI-owned repository does not make every line a TI silicon specification.
   Several copies of one driver are not independent confirmations.
4. Derive expected bytes and failure outcomes independently of the implementation
   and fake device. Never invent a register, reset value, access rule, timing,
   successful observation or qualification result to finish a prompt. Passing
   an emulator or a test copied from the same code cannot resolve a silicon gap.
5. When evidence contradicts this promptset, correct affected contracts, constants,
   vectors, implementation, tests, Doxygen and guides together. Record the old
   claim, evidence and correction. Do not silently choose a convenient source,
   whitelist unexplained values or weaken tests to match copied behavior.
6. Keep only the dependent operation/claim unavailable when evidence is unresolved.
   Continue independently verified work. A hardware result needs exact part and
   board, firmware identity, supply/straps, adapter settings, fixture and retained
   traces. It establishes that tested configuration, not every family member.

Use the existing variant matrix, register guide, device-behavior guide, feature
coverage, decisions and validation records; do not build another audit framework.
Correct locally resolvable findings during the current prompt. Request official
clarification or a specific physical experiment when documents cannot settle one.

## Sources, ownership and reading order

Read the [TI source archive](../reference/README.md) and its
[PDF provenance](../reference/source/provenance.json) first. The exact revisions
used below are B = SLLSEN9F (320/I), L = SLLSEP2D (LI/HI), A = SLLSEQ8D (LAI/HAI),
V = SLLSEO5D (322I), N = SLAAE24 and G = SDAA284. Full page/condition tables and
conflicts are in [additional TI evidence](../audit/additional-ti-evidence.md).

Public sources are pinned in [public-implementations.json](../reference/public-implementations.json).
Metadata includes ownership, license evidence, full commit, path, Git blob SHA,
retrieval UTC, byte size and SHA-256. Downloaded source is not vendored here.
Re-fetch and verify the relevant bytes when using a finding; record newer
revisions separately. Audit facts and independently implement this library.
Any actual source reuse needs a compatible, verified license and attribution.

| Source | What to extract and what it cannot establish |
| --- | --- |
| [TI-hosted Linux audit](../audit/ti-public-implementations.md): `TexasInstruments/ti-linux-kernel`, commit `97becb13b81c314c1d17fcf2226aac9a32086f3d` | Driver, binding, regmap helpers/backend and board descriptions. The driver is National Instruments/Michael Auchter code under GPL-2.0, hosted by TI. Inspect helpers; do not transplant Linux ownership, role policy or blocking sleeps. |
| [Community audit](../audit/community-public-implementations.md): USB armory Go driver | Base address and raw identity/current reads. Its current comment and default-current display are misleading. |
| Same audit: Betrusted Rust driver | Extended masks and encodings, but a reserved Try.SRC constant, unexplained revision, unbounded retries and unsafe whole-byte writes. |
| Same audit: Tiliqua Rust 322 driver | Raw 322 identity reads and field decoding; destructive role writes and reset submission without completion. It does not define an expected signature or resolve 322 access conflicts. |
| Same audit: Xous/Renode emulator and Yocto WIP patch | Useful source examples, with incomplete protocol/side-effect models. Neither emulator behavior nor a `meta-ti` path establishes TI ownership or silicon truth. |

Searches covered official TI GitHub first, selected TI SDKs, upstream/vendor
Linux, U-Boot, Zephyr, Espressif and community projects. Scope and access failures
are recorded in the reports. No result is not proof that no library exists.

## Address, identity and pin boundary — prompts 2, 9, 23–26

| Configured part | ADDR low/high, seven-bit | Corresponding write/read address bytes | Revision register |
| --- | --- | --- | --- |
| TUSB320 / TUSB320I | `60 / 61` | `C0/C1`, `C2/C3` | A0 undocumented: do not probe |
| TUSB320LI / TUSB320HI | `47 / 67` | `8E/8F`, `CE/CF` | A0 = `01` |
| TUSB320LAI / TUSB320HAI | `47 / 67` | `8E/8F`, `CE/CF` | A0 = `02` |
| TUSB322I | `47 / 67` | `8E/8F`, `CE/CF` | A0 = `02`, different identity |

All numbers in this table are hexadecimal. Callback addresses remain seven-bit.
`0x45` is a register offset on documented 320 parts, not another device address.
Ascending ID bytes at `00..07` are `30 32 33 42 53 55 54 00` for the six 320 parts
and `32 32 33 42 53 55 54 00` for 322I. Compare all eight bytes, including NUL.
The datasheets display them in descending offset order. Neither an ACK, a shared
signature nor revision `02` proves an exact model or enable polarity.

Floating ADDR selects GPIO operation. Low-enable variants are base 320/I, LI,
LAI and 322I; HI/HAI use active-high enable. The industrial `I` suffix does not
change base polarity. PORT is a sampled startup/reset selection on 320 parts,
including software reset; a later GPIO transition is not a continuous role
command. The 320 pin tables assign pin 3 to PORT and pin 4 to VBUS_DET; 322I
assigns pin 3 to VBUS_DET and pin 4 to DIR. It has no PORT pin and is not a
pin-for-pin substitution. TUSB321/321A are outside this I2C driver.

Public code's alternate LAI revision `06` has no supporting TI provenance in this
audit. Retain unexpected raw revisions for diagnosis and apply the specified
future-revision policy; do not silently add `06` to accepted revisions.

## Register facts and independent vectors — prompts 3, 10, 18

Sources: B Table 7-7 pp16–19, L/A Tables 7–12 pp16–20, V Tables 6-7–6-11 pp17–20.
Use explicit per-variant masks from prompt 3. Hex masks below are already shifted;
field codes are unshifted. An enum ordinal need not equal either.

| Field | Mask and meaning |
| --- | --- |
| 08 advertisement | `C0`; codes 0 default, 1 1.5 A, 2 3 A, 3 reserved; 322 write access remains unresolved |
| 08 detected current | `30`; codes 0 default, 1 medium, 2 charge-through 500 mA, 3 high |
| 08 accessory | `0E`; codes 0 none, 1–3 reserved, 4 audio, 5 charge-through; base 6 generic debug / 7 reserved; extended 6 local DFP / 7 local UFP debug |
| 08 active cable | `01`; retain raw flag and source-role applicability; not proof of powered VCONN |
| 09 attachment | `C0`; codes 0 detached, 1 local SOURCE/DFP, 2 local SINK/UFP, 3 accessory |
| 09 direction / IRQ | `20`: 0 CC1 / 1 CC2, subject to validity; `10`: IRQ W1C, zero has no clear effect |
| 09 duty / accessory disable | `06`: codes 0/1/2/3 = 30/40/50/60% source duty; bit `01` is reserved on base, UFP-accessory disable on extended |
| 0A debounce | `C0`: base nominal 133/116/151/168 ms; extended and 322 nominal 168/118/134/152 ms |
| 0A mode / reset | `30`: 320 codes 0 PORT, 1 UFP, 2 DFP, 3 DRP; 322 code 0 is DRP; reset command `08` is independent |
| 0A preference / disable term | Extended/322 `06`: codes 0 normal, 1 Try.SNK, 2 reserved, 3 Try.SRC; `01` disables termination/state machine; base bits 2:0 reserved |
| 45 disable Rd/Rp | `04`, distinct from 0A bit 0; internal bits 1:0 must not be changed from defaults; no documented 45 on 322 |
| 322 fault | 09 bit `08`; RCU label alone does not establish a clear transaction |

Startup `08=00`, `09=20`, `0A=00` requires exact variant confirmation; soft reset
is not proof of a complete power-on register image. Direction's reset bit cannot
be published as a connected CC2 orientation. Reserved fields retain raw evidence
without fabricated semantic values. Base debug maps to `Accessory::DEBUG`.
Orientation names the chip's CC1/CC2 inputs. The application maps those to its
connector and mux wiring; swapped routing changes that mapping. Core decoding
must not infer board orientation or silently exchange the input names.
For the initial 322 extension, retain fault bit 09[3] in valid raw observations.
The no-argument IRQ ACK does not request fault clearing. Do not add or simulate a
fault-clear action before its exact transaction and independent API are specified.

Use these independently checked examples alongside exhaustive field cases:

| Input or intent | Required result |
| --- | --- |
| `08=90` | Advertisement code 2 (3 A), detection code 1 (medium/1.5 A): separate facts |
| `08=20` versus `08=00` | Charge-through detected-current category versus default; do not collapse them |
| Try.SRC | Field code 3, shifted byte `06`; reject code 2 / byte `04` as a requested preference |
| Base `08=0C` / `0E` | Generic debug / reserved; extended `0E` is local UFP debug |
| Extended actual `09=B5`; explicit ACK | Write `15`, using actual persistent bits plus W1C, not raw `B5` or staged settings |
| Base actual `09=B4`; explicit ACK | Write `14`; base bit 0 is not an accessory setting |
| Extended actual controls `09=06`; ACK | Write `16`, not fixed `11` that changes duty/accessory controls |
| Reset request | Actual output must set `08`; `value=1` with `mask=08` yields `1 & 8 = 0`, which does not request reset |
| Role change from actual `0A=C2` to DFP | Preserve other valid controls: `C3 -> E3 -> E2`. Extended 320 requires the 5 ms wait before re-enable; 322I's own DFP procedure specifies the same three writes without an added wait |

The TI-hosted driver contains that reset mask/value error. The Betrusted driver
contains the Try.SRC error. These are source-level findings, not measured failures
on their hardware. Assertions must inspect outgoing bytes and effects, not merely
whether a method named reset, ACK or set-mode was called.

## Procedures, failure evidence and interpretation — prompts 4, 13–16, 18–19

- Reads use one-byte explicit pointers. TI describes STOP after pointer selection;
  Linux and Rust combined transfers show another implementation choice, not a
  universal repeated-START requirement. Verify selected adapter framing; do not
  assume implicit pointer persistence, wrapping or readable register holes.
- Base MODE_SELECT changes require unattached state; the restriction is not a
  blanket ban on other settings while attached. B §7.3.2 p12 allows current
  advertisement changes after attachment. A status read cannot prevent a cable
  race. LI/HI/LAI/HAI require separate disable-term, mode write while disabled,
  at least 5 ms after that write completes, then re-enable. The sequence applies
  attached or unattached. A first combined disable-and-change write is insufficient.
  Do not run a mode-change sequence merely to apply an unchanged mode.
- 322I V §6.3.2.1–2 pp10–11 explicitly prescribes disable-term, mode selection,
  enable-term for DFP/UFP: clean bytes `01 -> 21 -> 20` / `01 -> 11 -> 10`.
  Preserve non-target controls; the source adds no 5 ms wait. This corrects the
  earlier community audit's description of that sequence as merely conditional.
  V §6.3.2.3 p11 and Table 6-10 p20 establish DRP operation/codes, while §6.3.2
  p9 establishes reset-to-DRP. They do not list a separate direct return-to-DRP
  procedure. Qualify that transition independently; do not silently reset during
  profile apply or import a neighboring chip's sequence as a TI fact.
- Base dead-battery Rp recovery is explicit: configured device, register `45`,
  byte `04`, at least 30 ms, byte `00`, with TI's transient-status restriction.
  The library's identity prerequisite, pre-read/internal-bit-zero check and final
  verification are conservative library rules, not extra steps quoted from TI.
  Later variants do not inherit this recovery operation.
- Reset completion uses the specified timing and later evidence; a successful
  transfer, self-cleared bit or sleep alone does not prove reset/configuration/
  attachment readiness. Initialization stays read-only. No probe-time reset,
  role/current change, implicit ACK, hidden cleanup or desired-profile replay.
  Specifically, 322I returns to DRP on EN_N high-to-low or software reset
  (V §6.3.2 p9 and applications pp25, 27); do not assume DFP/UFP survives reset
  or infer an exact register reset byte from this operating-mode statement.
- The TI-hosted driver's cleanup can overwrite a failed role write with success;
  its final IRQ ACK result is ignored. Preserve the original failure and possible
  hardware effects. Fail before and after every write. A complete observation
  survives a later ACK/recheck failure; a failed required read publishes no
  invented complete snapshot and cannot silently become successful IRQ service.
- Extended 320 IRQ text names changed RU fields; base/322 text says CSR changes
  more broadly. Keep exact source distinctions in the fake and guide. ACK can
  race a new event, and the latch coalesces events. No atomic snapshot, lossless
  event queue or separate mandatory 5 ms IRQ-clear delay is established.
- Sink attachment needs qualified CC and detected VBUS. Default current is a
  category, not an unconditional 500/900 mA allowance. Try.SRC/Try.SNK is a DRP
  preference, not a guaranteed role or PD role swap. Separate local and peer roles.
  A fresh I2C read does not prove a fresh physical current detection while attached.
  Base B §7.3.1.2 p11 recommends periodic software reset to update detection from
  CC; GPIO OUT1/2 retain their initial indication. Record this as application
  policy requiring an explicit reset, never a hidden refresh/init/IRQ action or
  a recommendation automatically applied to later generations.
- Six-part 320 active-cable information does not establish integrated VCONN. 322
  autonomously supplies the unused CC line after source attachment with Ra and
  removes it on detach; there is no evidenced software VCONN-enable register.
  Do not copy Linux's inferred VCONN role into measured board-power claims.

## Timing and electrical facts — prompts 2, 4, 19, 23–26, 30–31

These are adapter/board constraints, not reasons to add bit-banging, GPIO ownership
or busy waits to the core. See the [full TI table](../audit/additional-ti-evidence.md)
for exact pages and conditions. Preserve dimensions and min/typ/max labels.

| Quantity | Evidence to carry into the relevant guide/test |
| --- | --- |
| I2C rate / setup / hold | Maximum 400 kHz; SDA setup at least 100 ns and hold at least 10 ns |
| START setup/hold, STOP setup | Minimum 0.6 us each; STOP-to-START bus-free minimum 1.3 us |
| Rise/fall, bus capacitance | Maximum 300 ns each; up to 400 pF at <=100 kHz versus 100 pF at 400 kHz, with exact table conditions |
| Data/ACK valid delay | Extended/322 maximum 0.9 us; base prints 0.9 ns, an unresolved unit conflict |
| DRP cycle and VBUS debounce | 50/75/100 ms min/typ/max; VBUS debounce 2 ms typical, not a maximum |
| I2C availability after valid supply and enable | 100 ms maximum; not USB attachment or configuration readiness |
| Software reset | 26/49/95 ms min/typ/max; retain separate owner-clock policy margin |
| Software clock policy | Existing `OWNER_CLOCK_MARGIN_MS=1`: 5/30/95 ms become 6/31/96 elapsed ticks, anchored to known completion/fresh owner time; not TI-specified 6/31/96 ms waits |
| LAI/HAI supply | Exact operating table 2.7–5.0 V; local DFP 3 A advertisement needs >=3.5 V; remote 3 A detection as UFP does not impose that local minimum |
| 322 supply/VCONN | 4.5–5.5 V; VCONN limit 225/300/375 mA min/typ/max; FET resistance 1.25 ohm maximum; bulk capacitance 10–200 uF |
| I2C pull-up rail and startup | 1.65–3.6 V; 3.3 V I2C footnote requires VDD >3.0 V; L/A ramp <=25 ms; retain exact startup-polarity conflict below |
| VBUS sensing | Resistor min/typ/max 855/887/920 kohm versus common 900 kohm prose; system threshold 2.95/3.3/3.8 V with specified network; a 28 V system range is not the pad rating |
| LAI/HAI Rd | Dead-battery 4.1/5.1/6.1 kohm; ordinary UFP/DRP 4.6/5.1/5.6 kohm, min/typ/max |
| LAI/HAI CC source currents | Default 64/80/96 uA, medium 166/180/194 uA, high 304/330/356 uA min/typ/max; signaling currents, not VBUS delivery |

Linux's 95 ms sleep corroborates a chosen wait, not write success. A TI board's
700 ms direction GPIO debounce belongs to TUSB321/SerDes integration, not this
chip's reset/role/IRQ timing. Community 1 ms retry policies and Android 25 ms
reset spacing are not general TI timing guarantees. Do not convert typical CC
debounce or DRP values into hard attachment deadlines.

## Unresolved and corrected source claims — prompts 2–4, 30–31, 35–38

Carry these into existing coverage/decision records with the affected behavior
and required evidence. A public implementation does not close any of these gates.

| Claim | Required disposition |
| --- | --- |
| 322 advertisement RW figure/prose versus R table | Keep advertisement writes gated pending authoritative resolution |
| 322 VCONN fault RCU without explicit clear mechanism | Do not infer W1C, read-clear or clearing by IRQ ACK; gate dependent consuming/clear behavior |
| 322 direct return to DRP | DRP00/11 and explicit reset-to-DRP are documented. Record evidence for a non-reset transition procedure separately from the documented DFP/UFP recipes; no hidden reset fallback |
| L/A EN_N text says low for 50 ms; application note says high | Preserve board startup uncertainty; base Rev F already says high. Active-high EN has its own requirement |
| Base timing table labels EN high for industrial part | Use exact active-low pin definition; do not invent an industrial enable-polarity variant |
| L/A 5.0 V maximum versus note's 5.5 V summary | Use exact recommended-operating limit for guidance; never use absolute maximum as operating permission |
| Base 0.9 ns versus extended/322 0.9 us | Record and seek clarification; do not silently repair the source or claim physical validation |
| Attached-current once-per-attach prose versus newer active-detection comparison | Preserve raw reads and uncertainty; qualify exact part with controlled live advertisement changes |
| Application note calls HI active low | Exact HI pin table says active high; retain correction and source locations |
| Implicit pointer and 322 debug-role prose | Explicit pointer avoids persistence ambiguity; distinguish peer DFP from local UFP before declaring a role contradiction |

For electrical and fixture work, distinguish the sensed system VBUS rail from
the VBUS_DET pad, source advertisement from current delivery, and CC detection
from completed attachment. Keep real traces and physical qualification separate
from host tests. Final reviews must check that no unresolved fact became accepted
merely because it appeared in a prompt, implementation, generated test or README.
The documented DFP-only VBUS_DET omission exception belongs to 320 variants;
322I §6.3.6 p13 requires its resistor connection and states no such exception.
