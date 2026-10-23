# Execution validation

## Prompt 1: ecosystem audit, 2026-10-23

Scope: source and documentation review only. The lead reviewed all three agent
reports and the selected decision table. Owner and tooling reviewers independently
cross-checked that table. Corrections included the actual 24Cxx package-checker
name and FieldCore's current branch/revision. No runtime behavior is implemented.

| Check | Result | Evidence and limits |
| --- | --- | --- |
| Inventory | PASS | 17 unique libraries, 30 exclusions; all 47 immediate directories accounted for; empty EEPROM_24Cxx distinguished from active 24Cxx |
| Current revisions and state | PASS | Full hashes/branches/manifests checked; 17 libraries and separate FieldCore consumer clean and unchanged at final comparison; no sibling remote-sync claim |
| Source paths | PASS | 757 indexed source/header/test/tool/review paths and all exclusion evidence paths exist; decision paths cross-reviewed with the reports |
| Baseline consistency | PASS | Exact names/state/ownership retained; explicit rejected lifecycle, health, build and Doxygen behavior documented; no framework dependency introduced |
| Markdown | PASS | 68 Markdown files and 358 local links/anchors checked; UTF-8, balanced fences, no corrupt text; 38 indexed numbered prompts retain their shared completion requirements |
| Artifact format | PASS | New Markdown/JSON is UTF-8 without BOM, LF/final newline; JSON parses; no trailing whitespace; git diff --check |
| Progress and branch | PASS | Recorded synchronized base and one execution branch; prompt 1 complete, prompts 2-38 NOT STARTED |
| Tool discovery | PASS | Python, host compiler, CMake/CTest, Ninja, Doxygen and managed PlatformIO versions queried; native IDF not located; availability is not build proof |
| Scope preservation | PASS | Original preparation reports and vendor artifacts unchanged; no driver, SDK installation or sibling edits |

Checks used ad hoc session Python commands (pathlib/json/re), read-only git
revision/status commands and git whitespace checking. These are not a checked-in
validator or an implemented TUSB32x test suite.
They validate documentation and current source evidence, not the proposed driver.
Tool resolution and exact versions are in [library comparison](library-comparison.md).

Driver compilation/tests, firmware links, source Doxygen generation, remote CI
and physical hardware checks are not applicable to completing this source-free
prompt and were not run. Later prompts must establish their own evidence. Native
IDF availability and board fixtures remain real future prerequisites, not assumed
passes. Missing TMP1x2 CONTRIBUTING.md is recorded; LDC1614 supplies the inspected
release-process reference instead.

The lead commits this complete unit and verifies fetch/push/upstream equality.
The handoff gives the resulting commit and sync result; the next progress update
records that commit, avoiding a self-referencing hash edit.

## Independent prompt 1 review, 2026-10-23

Reviewed the original prompt, completed commit `18aba0b` and its diff from base
`ca0cfe9`. Three parallel reviewers checked coverage, inventory and source claims;
the lead checked their findings against the actual files. No driver work or later
prompt was started. The inventory and selected engineering baseline remain valid.

Corrected five evidence or wording issues:

- TMP1x2's console drain defaults to 256 bytes; callers can supply a larger
  budget. The planned TUSB32x work bound remains its own explicit requirement.
- TMP1x2 end is bus-silent but remains bound; its separate unbind clears callback
  associations. TUSB32x adopts bus silence, not that retained binding.
- ADS1115 DriverConfig is declared in Config.h, not ADS1115.h.
- LDC1614/INA228 supply strict Doxygen settings, not demonstrated missing-comment
  negative fixtures. Those fixtures remain required TUSB32x work in prompt 33.
- The documentation checks above were session commands, not a repository tool.

Repeated local checks: 17 library revisions/manifests and the FieldCore consumer,
all 47 directory classifications, 757 indexed paths, 68 Markdown files and 359
local links/anchors passed. Prompt numbering, completion requirements, encoding,
JSON parsing and whitespace checks passed. Historical preparation/vendor files
were unchanged. Sibling repositories remained clean at their recorded revisions.
No peer tests, driver tests, firmware builds, hardware checks or remote CI ran.

## Prompt 2: family capabilities and provenance, 2026-10-23

Scope: complete variant/source boundary, not driver implementation or physical
qualification. The lead checked the two TI-generation reviews and peer/public
source report against exact files and the final matrices. No prompt 3 work ran.

| Check | Result | Evidence and limits |
| --- | --- | --- |
| Archived TI sources | PASS | All eight PDF byte counts/SHA-256 match provenance; no unlisted PDF; seven previous byte streams and original metadata remain unchanged |
| Current official sources | PASS | Seven fresh official downloads were byte-identical; separately fetched TUSB321AI SLLSER2A added; current product/revision/errata search recorded in reference README |
| Original terms | PASS | Final-page Important Notice checked in all eight PDFs; per-file rights/terms summaries and current-source verification UTC added without removing original notices |
| Public implementations | PASS | All 13 pinned downloads match byte count, SHA-256 and Git blob SHA-1 (39 comparisons); lead independently checked downloaded bytes and relevant regmap/identity helpers |
| Focused peers | PASS | TMP1x2/24Cxx/LDC1614 remain clean at captured revisions; declarations, tests, AGENTS, CLI/runner and build/package rules re-audited; peer archive guard passed 15 documents/28 artifacts/66 checksums |
| Capability boundary | PASS | Six explicit 320 variants plus separate 322I gates; exact address/identity/revision restrictions; GPIO-only parts excluded; no claimed hardware support |
| Source review | PASS | Exact electrical/pin/identity tables visually inspected; source/code/policy/unresolved/hardware labels separate; conflicting summaries and operation gates preserved |
| Scope and preservation | PASS | Dated TI preparation reports preserved with appended corrections; no production code, SDK install or sibling mutation; compilation/package exclusion remains explicit for future builds |
| Documentation | PASS | 71 Markdown files, 388 local links/anchors; all 38 prompt numbers and completion requirements intact; JSON parses; changed text UTF-8/LF/final newline and whitespace checks pass |

Visual source review covered B printed pp4–7,11,13–19,27; L pp2–6,10,13–20,28;
A pp3–6,16,20; V pp3–4,6–7,17,20; G0/G1 p3; N pp3,5,7; G pp8–9.
Other cited passages were read in the actual PDF text. The lead additionally
checked the A/V identity tables, V resistance column, B/L/A pull-up columns,
summary-table contradictions and public helper behavior. A reviewer's proposed
4.7 kΩ/200 kΩ maximum labels were rejected: rendered tables and PDF coordinates
place both in TYP. VCONN resistance 1.25 Ω is in MAX and was corrected.

Other completed corrections/decisions: current LI/HI DFP/DRP recommendation,
base commercial temperature, separate 321AI source, B/L/A/V Type-C 1.0 DRP-peer
exception, strict unknown-revision handling, and named board/322 capability
gates. No dedicated errata was found in the inspected lists/searches. A separate
TUSB320I product-page request failed; its shared exact datasheet was fetched and
verified, so it does not prevent this source boundary from being completed.

Checks used ad hoc session Python (pathlib/json/hashlib/re/PyMuPDF), fresh HTTPS
downloads, read-only sibling Git commands and `git diff --check`. The source
hash check is reproducible without a test framework:

```python
from pathlib import Path
import hashlib, json

root = Path("docs/reference/source")
entries = json.loads((root / "provenance.json").read_text(encoding="utf-8"))
assert {e["file"] for e in entries} == {p.name for p in root.glob("*.pdf")}
for entry in entries:
    data = (root / entry["file"]).read_bytes()
    assert len(data) == entry["bytes"], entry["file"]
    assert hashlib.sha256(data).hexdigest() == entry["sha256"], entry["file"]
```

Hardware qualification, driver/CLI tests, firmware links and package extraction
were NOT RUN: those implementations and fixtures are not part of prompt 2.
Remote CI was not dispatched or polled. Full register encodings, procedure
proofs and each named silicon/board gate remain explicitly assigned to their
later stages; no unresolved value was invented to claim completion.

## Independent prompt 2 review, 2026-10-23

Reviewed `30bc8a88d47bb599c92db8a8ddd1faa234e464af` and its actual diff from
`9670c53b15b3e8c338479134e539ef5b4dd441d4`, after rereading the full original
prompt. Three fresh-context reviewers independently checked requirements and
contracts, TI silicon evidence, and provenance/peer claims. The lead checked
their findings against source pages and code; two reviewers then checked the
correction diff. No production implementation was present or introduced.

| Finding | Source and correction | Verification |
| --- | --- | --- |
| Missing VCONN supply condition | SLLSEO5D §5.3 p4 and §6.3.8 p13: record 5 V at VDD after Attached.SRC and VCONN 4.75–5.5 V with VDD at 5 V; keep power policy outside the core | Rendered tables/text checked; matrix, feature qualification, shared chip evidence and decisions agree |
| Incomplete shared-register write gate | SLLSEO5D Table 6-9 pp18–19: fault bit3 shares 09 with IRQ, duty and UFP-accessory controls; gate every 09 write until safe handling/composition is established | Checked all affected feature rows and raw-write boundary; unchanged proved 09 controls still permit independently supported 0A work |
| Visual review gap | A Tables 8–10 pp17–19 and V Tables 6-8–9 pp18–19 had only text review in the completed work | Lead rendered and inspected all five pages from archived bytes; additionally checked V pp4,13 for the supply condition |
| Imprecise peer wording | TMP1x2 src/BusOperations.cpp::decodeAlertCause validates an ARA response against the selected model, rather than discovering a model | Compared the actual function and refined the peer report; selected variant/identity policy is unchanged |

Repeated verification:

- All eight archived PDF sizes/SHA-256 and archive coverage passed. The provenance
  reviewer fetched all eight official URLs again at 2026-10-23 15:00:32 UTC;
  every byte stream matched. Exact source revisions and original terms remain
  unchanged. Seven pre-prompt PDFs and every original metadata field still match
  the prompt 1 review commit; both dated TI reports preserve their old content.
- Thirteen freshly downloaded pinned public files passed 39 byte-count/SHA-256/
  Git-blob comparisons; the lead independently repeated these checks on the new
  download cache. The reviewer checked helpers, attribution and three supplemental
  LICENSE hashes against the actual fetched content.
- TMP1x2, 24Cxx and LDC1614 remain clean on main at the full revisions recorded in
  execution-2-peers.md, with matching local upstream refs. Their declarations,
  tests, AGENTS and package/build rules were rechecked without sibling mutations
  or fetches. The read-only 24Cxx archive guard again passed 15 documents,
  28 source artifacts and 66 checksums.
- All 71 Markdown files and 388 local links/anchors passed. All 38 numbered prompts
  remain unchanged. Changed documents pass UTF-8, LF, final-newline and whitespace
  checks; JSON parses and `git diff --check` passes. No generated PDF images or
  downloaded public code entered the repository.

The fresh review closes the confirmed prompt 2 documentation and verification
gaps. Full register semantics/procedures and their behavioral tests remain due
in the assigned later prompts. Hardware, driver/CLI tests, firmware builds and
package extraction were NOT RUN; remote CI was not dispatched or polled.
Unresolved silicon gates are retained, not treated as proved capabilities.

## Prompt 3: register contracts and vectors, 2026-10-23

Scope: complete register specification and independent expected bytes, without
production declarations, codecs or operation procedures. Started from synchronized
`cd591d4b56b83bda4f3f641bf5ee1c86f5d8a08c` on the existing execution branch.
Two independent silicon reviewers checked B/L/A and V; a peer reviewer audited
source conventions. All three then reviewed the actual guide/vector drafts;
the lead checked source evidence, masks, literals and the full scoped diff.

| Check | Result | Evidence and limits |
| --- | --- | --- |
| Exact TI tables | PASS | Visually inspected B pp16–19, L/A pp16–20 and V pp15–20 from archived PDFs; lead additionally inspected B pp18–19 and A p20 for partial defaults/protected bits |
| Current source/provenance | PASS | Eight archive sizes/SHA-256 verified; no archived byte/metadata changed. Current official B/L/A/V URLs still return F/D/D/D. Fresh V download matched 1,586,968 bytes/SHA-256 7cbd82a8… at 2026-10-23T15:07:44.442351+00:00 |
| Public helper evidence | PASS | Reviewer freshly fetched the pinned Linux driver, regmap header/core/I2C backend and matched hashes. Lead independently checked those cached bytes plus Betrusted (15 size/SHA/blob comparisons) and actual reset/IRQ/Try.SRC code; TI hosting does not establish authorship or silicon authority |
| Access/encoding contract | PASS | Complete map, default validity masks, separate persistent/protected/live/command/reserved masks, comparison masks and semantic tables reviewed against exact pages; V access/return gates remain explicit |
| Independent arithmetic | PASS | Eight complete/disjoint byte-mask partitions; all 21 C vectors match independent integer arithmetic, legal output masks and command exclusion; selected field extraction, reset-mask defect and DRP equality/inequality checked |
| Vector inventory | PASS | D01–D36, C01–C21 and N01–N25 unique and consecutive: 82 literal cases; requirements include future callback/error tests, not 82 executed driver tests |
| Focused peers | PASS | PCA9555/LDC1614/ADS1115 clean at recorded HEADs/main/local upstream; 18 named test definitions and RUN_TEST registrations verified. No sibling fetch/edit/build |
| Documentation/scope | PASS | All 74 Markdown files and 408 local links/anchors checked; 38 numbered prompts unchanged. Changed/new text UTF-8/LF/final newline, JSON and whitespace checks pass; no generated images/download cache or production code in the diff |

The independent arithmetic check parsed the literal composition rows and used
separate integer masks: 08=C0, 09=06 (B) or 07 (extended), 0A=F0 (B) or F7
(extended/V), and 45=07 with protected 03. It checked requested field containment,
legal command bits,
`(actual & persistent & ~update) | field | command`, expected bytes and preserved
internal fields. This is specification arithmetic, not an implemented composer
or runtime test. Three reviewers also independently checked the 21 C rows.

Source corrections: base defaults are documented only under FE/E6/F0/04 masks,
not complete zero-filled bytes; 45 internal defaults are numeric only in L/A;
accessory 5 means audio charged-through; V accessory reset has a width typo.
Managed ACK/reset may obtain a safe control image through their own reads, while
apply/raw admission retains its stronger existing evidence prerequisites.
Readable profile comparison is distinct from write permission, especially on V.
No authoritative resolution of V advertisement/fault semantics was found in the
bounded current TI search; gates remain rather than inventing legal writes.

Checks used ad hoc Python and read-only source/Git inspection, including
`git diff --check`. Driver/CLI behavior, procedure timing, firmware builds,
package consumption and hardware qualification were NOT RUN: those
implementations belong to later prompts. Remote CI was not dispatched or polled.
Prompt 4 remains NOT STARTED.
