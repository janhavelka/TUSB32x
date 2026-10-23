# 2 — Establish family capabilities and source provenance

Prerequisite: prompt 1 is complete. Read [COMMON](COMMON.md),
[ENGINEERING_BASELINE](ENGINEERING_BASELINE.md) and the current comparison and
decision records. Finish the variant/source boundary in this prompt. Register
encodings and silicon procedures are complete separate tasks in prompts 3 and 4;
do not implement a driver here.

## Focused re-audit and evidence

Recheck current TMP1x2 variant handling and 24Cxx reference provenance, including
their AGENTS, declarations, tests and relevant build/package rules. Inspect
LDC1614 capability checks and nearby fake-device expectations. Record current
HEAD, dirty state, exact paths and adopted/rejected conventions in progress.
Sibling conventions cannot establish TI silicon facts.

Verify every archived byte against `docs/reference/source/provenance.json`.
Check current official TI product documentation for revisions, errata and notes.
Keep superseded revisions when adding new sources; record URL, document number,
revision, retrieval UTC, byte size, SHA-256 and original terms. Reference artifacts
must remain outside production compilation and ordinary library packages.

Use these sources and visually inspect relevant tables. Page references are
printed pages, not zero-based PDF indexes.

| Local file under docs/reference/source | Review focus |
| --- | --- |
| TI-TUSB320-I-SLLSEN9F.pdf | Pins p4; timing §6.7 p7; modes pp13–14; programming §7.5 pp15–16; Table 7-7 pp16–19; initialization p27 |
| TI-TUSB320LI-HI-SLLSEP2D.pdf | Pins pp3–4; timing p6; §7.3.2 p14; programming p15; Tables 7–12 pp16–20; initialization p28 |
| TI-TUSB320LAI-HAI-SLLSEQ8D.pdf | Pins p3; electrical/timing pp4–6; §7.3.2 p14; §7.4 p15; Tables 7–12 pp16–20; initialization p28 |
| TI-TUSB322I-SLLSEO5D.pdf | Pins p3; timing §5.7 p7; §6.3.2 mode; §6.3.8 VCONN; §6.5 programming pp15–16; Tables 6-6–6-11 pp17–20 |
| TI-TUSB321-SLLSEO6C.pdf | GPIO-only interface, features p1 and pins p3 |
| TI-TUSB32x-Application-Issues-SLAAE24.pdf | Family Table 2-1 p3; startup p5; non-failsafe pins p7; IRQ clear p9 |
| TI-CC-Selection-Guide-SDAA284.pdf | Family comparison pp8–9; reconcile summaries with exact datasheets |

## Complete the capability matrix

Read [CHIP_EVIDENCE](CHIP_EVIDENCE.md) and its TI-hosted/community audits. Verify
the pinned source bytes, actual helpers and licenses; official TI hosting does
not make the Linux driver TI-authored silicon guidance. Classify claims as TI
facts, code observations, library policy, hardware results or unresolved. Correct
inherited prompt errors with exact evidence instead of inventing missing data.

Keep the six named TUSB320 variants explicit; TUSB320LAI is the primary acceptance
device. A small internal descriptor may be shared only where behavior agrees.

| Exact variant | ADDR low/high, seven-bit | Enable | Revision at 0xA0 | Initial capability boundary |
| --- | --- | --- | --- | --- |
| TUSB320 / TUSB320I | 0x60 / 0x61 | Active low | Undocumented; never probe | Base map; unattached mode change; explicit post-dead-battery Rp recovery |
| TUSB320LI / TUSB320HI | 0x47 / 0x67 | LI low / HI high | 0x01 | Extended mode/termination procedure |
| TUSB320LAI / TUSB320HAI | 0x47 / 0x67 | LAI low / HAI high | 0x02 | Extended map; separately verified electrical/timing limits |
| TUSB322I | 0x47 / 0x67 | Active low | 0x02 | Separate identity/defaults/VCONN; unresolved operations gated |
| TUSB321 / TUSB321A | Not I2C targets | Verify exact part | Not applicable | GPIO-only; outside the I2C API |

Verify ascending identity bytes `0x00..0x07`: the six 320 parts use
`30 32 33 42 53 55 54 00`; 322I uses `32 32 33 42 53 55 54 00`. Report configured
variant, ACK, signature match and observed revision separately. A common address,
signature or package suffix cannot prove exact model, enable polarity or future
capabilities. Define an explicit future-revision policy without enabling unproved
features.

Document floating ADDR selecting GPIO operation, exact strap levels/sampling,
PORT selecting startup role on the six 320 parts, and 322I having no PORT pin.
Record pin mapping: 320 pin 3 is PORT and pin 4 VBUS_DET; 322I pin 3 is
VBUS_DET and pin 4 DIR. Shared packaging does not imply board interchangeability.
Record VDD, I2C pull-up voltage, non-failsafe pins, VBUS_DET resistor/input limits,
dead-battery Rd and enable behavior per variant. LAI/HAI 3 A advertisement needs
at least 3.5 V; 322I uses 4.5–5.5 V. Recheck exact limits rather than inheriting
them by family name. The core owns none of these pins or external power policy.

The 3.5 V condition concerns local DFP advertisement, not UFP detection of a
remote 3 A source. Exact L/A operating tables give 5.0 V maximum despite the
application note's 5.5 V summary. Separate VBUS system voltage from VBUS_DET pad
rating and CC signaling microamps from VBUS load amps. Public code's revision
`0x06` is an unresolved clue, not permission to expand LAI support. Carry these
source conflicts and the detailed electrical table into the variant guide.

Identify the EN_N 50 ms startup-polarity conflict between exact pin text and
SLAAE24 §4.1. Record a board prerequisite or gate the dependent claim where
evidence remains insufficient. Other unresolved claims get a named affected
feature and required evidence; do not stop independently verified work.

## Deliver, review and commit

Complete `docs/variant-matrix.md` and source provenance. Start or update
`docs/feature-coverage.md` with feature, variant, source revision/section/page,
planned API/CLI and required test evidence. Label planned/unavailable entries
honestly; later prompts fill their assigned details. Preserve dated
`docs/audit/ti-family.md` conclusions and record corrections with date/evidence.

Optional delegation: separate TI generations among strong reasoning agents;
use a fast agent for hash/provenance checks. Sources are read-only, and only one
owner edits each matrix. The lead resolves conflicting evidence and Git.

Check hashes, exact variant names, address/revision restrictions, identity vectors
and every support claim against source pages. Update decisions, progress,
validation and affected README/CHANGELOG. No C++ documentation is due before
source exists. Finish with a reviewable matrix and explicit unresolved gates,
not a claim of physical qualification. Commit and sync checked work, verify the
same branch/upstream, and report checks and limitations before prompt 3.
