# P01 — Establish the exact TI family specification

Read [COMMON.md](COMMON.md), [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md)
and [MAINTAINABILITY.md](MAINTAINABILITY.md). This stage produces an implementable
specification and test vectors. It does not yet implement the driver.

## Re-audit before editing

Inspect current sibling files, revisions, dirty state, relevant examples, tests,
runners and CI. Record changes from these clean 2026-10-23 observations:

- PCA9555 `498178e4d9e4acb6fab7d8b816ad5021efe8806a`:
  `include/PCA9555/CommandTable.h`, `include/PCA9555/PCA9555.h`,
  `src/PCA9555.cpp`, `test/test_basic.cpp`, `test/test_public_contracts.cpp` and
  `.github/workflows/ci.yml`. Adopt named constants, access descriptions and
  explicit write-effect evidence. Its paired-register increment and pointer
  parking erratum are PCA9555 behavior and must not appear in this driver.
- LDC1614 `c0ab4c1712c4135b925354d70c4e6bed5cca62d8`:
  `include/LDC1614/CommandTable.h`, `src/LDC1614.cpp`,
  `test/support/FakeLdc1614Device.h`, `test/test_basic.cpp` and
  `examples/common/Ldc1614Cli.cpp`. Adopt explicit variant validation and
  independent fake-device expectations; reject sensor-specific conversions.
- ADS1115 `381236ebfed813cd7a9aca34520ce031d8a0ff6b`:
  `include/ADS1115/CommandTable.h`, `test/test_basic.cpp` and
  `examples/02_owner_safe_poll/main.cpp`. Borrow field names and fault-test
  structure where useful, not its 16-bit register framing or conversion timing.

Also refresh TMP1x2 variant handling and 24Cxx reference provenance, as selected
in P00. Sibling code is evidence of conventions, never evidence of TI behavior.

## Source documents and outputs

Verify archived bytes against `docs/reference/source/provenance.json`.
Recheck official product documentation for revisions, errata and relevant notes.
Keep old and new revisions if anything changes. Record URL, document number,
revision, retrieval UTC, byte size, SHA-256 and original licensing. Keep reference
PDFs and reference source out of production compilation and normal packages.

Use these local sources and visually inspect the relevant tables. Page numbers
below are printed pages, not zero-based PDF viewer indexes:

| Local file under `docs/reference/source/` | Required evidence |
| --- | --- |
| `TI-TUSB320-I-SLLSEN9F.pdf` | Pins p4; timing §6.7 p7; modes pp13–14, including dead-battery exit §7.4.3 p14; programming §7.5 pp15–16; Table 7-7 pp16–19; initialization p27 |
| `TI-TUSB320LI-HI-SLLSEP2D.pdf` | Pins pp3–4; timing p6; §7.3.2 p14; programming p15; Tables 7–12 pp16–20; initialization p28 |
| `TI-TUSB320LAI-HAI-SLLSEQ8D.pdf` | Pins p3; electrical/timing pp4–6; §7.3.2 p14; §7.4 p15; Tables 7–12 pp16–20; initialization p28 |
| `TI-TUSB322I-SLLSEO5D.pdf` | Pins p3; timing §5.7 p7; mode procedures §6.3.2; VCONN §6.3.8; programming §6.5 pp15–16; Tables 6-6–6-11 pp17–20 |
| `TI-TUSB321-SLLSEO6C.pdf` | GPIO-only interface, features p1 and pins p3 |
| `TI-TUSB32x-Application-Issues-SLAAE24.pdf` | Family Table 2-1 p3; startup p5; non-failsafe pins p7; interrupt clear p9 |
| `TI-CC-Selection-Guide-SDAA284.pdf` | Family comparison/selection pp8–9; compare summaries with exact datasheets |

Create `docs/variant-matrix.md`, `docs/register-map.md`,
`docs/device-behavior.md` and a feature-to-source/API/CLI/test coverage table.
Each fact needs document revision, section/table and page. Distinguish specified
maximum, nominal/typical value, board assumption and software policy. Preserve
`docs/audit/ti-family.md` as dated evidence; record subsequent corrections with
their audit date and source rather than silently replacing its conclusions.

## Concrete family matrix to verify

Start with all six named TUSB320 variants; LAI is the primary acceptance device.
Keep named public variants even where one small internal descriptor is shared.

| Exact variants | ADDR low/high, 7-bit | Enable | `0xA0` | Controls |
| --- | --- | --- | --- | --- |
| TUSB320, TUSB320I | `0x60 / 0x61` | Active low | Undocumented; never probe it | Base map; mode changes only while unattached; explicit dead-battery exit procedure for Rp |
| TUSB320LI, TUSB320HI | `0x47 / 0x67` | LI low; HI high | Documented `0x01` | Extended map; disable-term/mode/wait/enable-term procedure |
| TUSB320LAI, TUSB320HAI | `0x47 / 0x67` | LAI low; HAI high | Documented `0x02` | Extended map; same procedure; verify electrical/timing separately |
| TUSB322I extension | `0x47 / 0x67` | Active low | Documented `0x02` | Separate defaults, identity and VCONN; unresolved features stay gated |
| TUSB321 / TUSB321A | Not I2C targets | Board-only facts | Not applicable | Exclude from this I2C library; verify exact named part documentation |

Floating ADDR selects GPIO operation. PORT selects the startup role on the six
320 parts; 322I has no PORT pin. Specify strap levels and sampling times from
each pin/mode section. Shared address or identity does not establish enable
polarity or an exact model. Check full ascending `0x00..0x07` signatures:

- Six TUSB320 parts: `30 32 33 42 53 55 54 00`.
- TUSB322I: `32 32 33 42 53 55 54 00`.

Define future-revision policy without granting unverified capabilities. Report
configured variant, ACK, signature match and observed revision separately.

## Register facts and golden vectors

Re-derive this map from the exact tables; do not copy an undifferentiated family
mask. For every register record reset fields, access classes, legal writes,
reserved handling, applicability, prerequisites and observable side effects.

| Register | Six-part baseline | Base 320/I difference | Extended LI/HI/LAI/HAI |
| --- | --- | --- | --- |
| `0x00..0x07` | Read-only signature bytes | Same signature | Same signature |
| `0x08` | Advertisement RW `0xC0`; live status `0x3F`; documented fields reset to zero | Accessory code 7 reserved | Accessory 6/7 distinguishes local DFP/UFP debug |
| `0x09` | Attachment `0xC0`, direction `0x20`, IRQ W1C `0x10`, duty `0x06`; direction reset is 1, not an attached-port claim | RW settings `0x06`; bit 0 reserved | RW settings `0x07`; bit 0 disables UFP accessory support |
| `0x0A` | Debounce `0xC0`, mode `0x30`, reset command `0x08` | Persistent RW `0xF0`; bits 2:0 reserved | Persistent RW `0xF7`; preference `0x06`, termination disable `0x01` |
| `0x45` | Rd/Rp disable `0x04`; bits 1:0 TI-internal RW, do not change; bits 7:3 reserved RO | Same named control | Distinct from `0x0A` termination/state-machine disable |
| `0xA0` | Revision only where documented | Unavailable | Read only; expected values above |

Record `0x08=0x00`, `0x09=0x20`, `0x0A=0x00` as documented/derived startup
field values where supported by the exact source. Do not assume soft reset
restores every byte to these values. `0x45` internal bits are not user settings.

Specify and test these encodings explicitly:

| Field | Unshifted encoding and meaning |
| --- | --- |
| Advertisement `08[7:6]` | 0 default category; 1 1.5 A; 2 3 A; 3 reserved |
| Detected current `08[5:4]` | 0 default; 1 medium; 2 charge-through accessory 500 mA; 3 high |
| Accessory `08[3:1]` | 0 none; 1–3 reserved; 4 audio; 5 charge-through; 6 debug (extended: local DFP); 7 extended debug/local UFP (base reserved) |
| Active cable `08[0]` | Raw flag plus documented applicability |
| Attachment `09[7:6]` | 0 detached; 1 local source/DFP; 2 local sink/UFP; 3 accessory |
| Direction `09[5]` | 0 CC1; 1 CC2; expose not-applicable/unknown separately from the raw bit |
| DRP duty `09[2:1]` | 0/1/2/3 = 30/40/50/60% source advertisement time |
| Debounce `0A[7:6]`, base | 0/1/2/3 = nominal 133/116/151/168 ms |
| Debounce `0A[7:6]`, extended | 0/1/2/3 = nominal 168/118/134/152 ms |
| Mode `0A[5:4]` | 0 PORT selection; 1 UFP; 2 DFP; 3 DRP starting unattached.SNK |
| Source preference `0A[2:1]`, extended | 0 normal DRP; 1 Try.SNK; 2 reserved; 3 Try.SRC |

Do not interpret default current as an unconditional 500 mA or 900 mA allowance.
The owner decides usable current. Advertisement is not measured current or USB
PD. Requested mode is not actual attachment. Preserve reserved status as raw
evidence with explicit validity rather than substituting a plausible value.

## Transport, sequences and unresolved gates

1. Specify one-byte pointers, supported contiguous reads, exact lengths and
   legal boundaries. Every logical read sets its pointer. SLLSEN9F §7.5,
   SLLSEP2D/SLLSEQ8D §7.4 and SLLSEO5D §6.5 describe STOP after pointer
   selection; do not mandate repeated START.
   Record chosen adapter framing and hardware validation still required. Bound
   the complete callback, including lock acquisition and both bus phases.
2. Separate RO/live, persistent RW, W1C and self-clearing command bits. IRQ
   ACK preserves observed/trusted **applied** settings, never a staged profile.
   Define read/reconciliation when applied settings are unknown. Unrelated
   settings writes use zero for IRQ and reset commands. Document `0x45` internal
   preservation without substituting it for the mode-change procedure.
3. Base mode changes require unattached state. A preceding status read is not
   proof attachment cannot race the write: define the owner/fixture prerequisite
   and expose the limitation. Extended procedure: disable termination, write
   desired mode while disabled, wait at least 5 ms after that write completes,
   then enable termination. Each write is separately observable and may fail
   after application. Cancellation cannot restore a previous port state.
   Separately, base TUSB320/I requires the dead-battery exit sequence in
   SLLSEN9F §7.4.3 p14 to present Rp on both CC pins after VDD returns:
   write `0x45=0x04`, wait at least 30 ms, then write `0x45=0x00`.
   Dead battery includes VDD absent with no cable attached. The application
   owns stable supply/I2C readiness and decides when this source-capable startup
   prerequisite applies; the library executes it only through explicit
   `startRecoverDeadBattery(timing, token)` after successful identity validation.
   Initialization remains read-only and neither performs nor proves recovery.
   First read `0x45` and require internal bits `[1:0]==0`; otherwise fail
   without writing. This reconciles the literal sequence with Table 7-7 p19's
   prohibition on changing the internal defaults. Verify `0x45[2:0]==0`
   after the final write. Do not publish intermediate status as port observations:
   TI says status flags during the sequence must be ignored. Preserve write
   effects on failure/cancellation and never replay or clean up automatically.
   A final zero register alone cannot prove the required timing history.
   Use the existing operation engine; add no startup engine or power policy.
   The operation is unsupported on LI/HI, LAI/HAI and TUSB322I: their respective
   SLLSEP2D/SLLSEQ8D §7.3.3 p14 and SLLSEO5D §6.4.4 p14 do not specify it.
4. For LAI/HAI, verify 400 kHz maximum, enable-to-I2C availability maximum 100 ms
   and reset 26/49/95 ms min/typ/max in §6.6–6.7. Recheck corresponding values
   per other variant; never inherit timing by similar name. Waits must start at
   known callback completion or a later fresh owner timestamp. A stale timestamp
   passed before a slow write cannot start the 5 ms, 30 ms or reset wait.
   Reset readiness and operation deadline are separate; a cleared command bit alone is insufficient
   evidence of configuration trust or port readiness.
   The selected owner clock has resolution at most 1 ms and may return ordinary
   floor-valued millisecond timestamps. Define a named 1 ms software margin for
   hardware waits: the 5 ms TI mode wait requires at least 6 elapsed ticks from
   the fresh anchor; the base 30 ms recovery wait needs at least 31 ticks;
   relying on the 95 ms reset maximum requires at least 96. SLLSEN9F §6.7 p7,
   SLLSEP2D/SLLSEQ8D §6.7 p6 and SLLSEO5D §5.7 p7 independently specify
   the same 95 ms maximum; it does not guarantee attachment or profile trust.
   This margin prevents an early expiry caused by timestamp quantization. Keep
   the TI value and software margin separate in constants, citations and docs.
   Do not add this margin to the owner's operation deadline or extend that deadline.
5. Specify IRQ sources per generation, explicit W1C, bounded read/ACK/recheck,
   and reassertion/coalescing limits. No evidence establishes atomic capture
   across status bytes or lossless edges. No specified inter-clear 5 ms delay
   was found; do not confuse the mode-change wait with IRQ policy.
6. Resolve or retain these open questions with named dependent features: conflicting
   EN_N 50 ms startup polarity (exact pin text versus SLAAE24 §4.1); dynamic
   current detection while attached (datasheet prose versus SDAA284); pointer
   prose contradictions; precise soft-reset effects; current official errata.
   Record an external board prerequisite or disable the dependent claim when
   evidence is insufficient. Do not block independent features.
7. Audit TUSB322I separately: ID differs, mode 00 means DRP, no documented
   `0x45`, VCONN fault at `09[3]` is RCU, and the mode procedure does not state
   the 320 extended 5 ms step. Resolve the `08[7:6]` access conflict (Table 6-8
   says R; figure says RW; prose says programmable) before write support.
   Specify fault-clear semantics independently from IRQ clear. Keep exact
   unresolved operations unavailable; do not silently drop the extension audit.
   Explicit mode/reset/termination operations may change autonomous VCONN.
8. Document VDD and 3 A advertisement conditions (LAI/HAI needs at least 3.5 V),
   I2C pull-up voltage, non-failsafe pins, VBUS_DET resistor/input requirements,
   dead-battery Rd and enable behavior. Record exact variant values, including
   322I's 4.5–5.5 V supply. The core owns no pins, external power switch or policy.

## Documentation and acceptance now

Write the documents in simple engineering English. Include positive and negative
byte vectors, sequence diagrams/tables only where useful, and a short unresolved
items table with source, affected API and evidence needed. Define exact TI
citations that P04 constants and source comments will carry. Public capability
and register types introduced later need Doxygen descriptions of validity,
reserved values, side effects and variant limits from these documents.

Check archive hashes, cross-check masks/encodings against the PDFs, and review
the source-to-test matrix for every named variant. Update progress and decision
records. This stage passes when a developer can implement each enabled operation
from its cited contract without inventing protocol facts; it does not establish
physical hardware validation.
