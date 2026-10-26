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

## Independent prompt 3 review, 2026-10-23

Reviewed `a7cf07055ff46bde2cf460d4cee3e2066b30c77c` and its actual diff from
`cd591d4b56b83bda4f3f641bf5ee1c86f5d8a08c`, after rereading the full original
prompt. Three fresh-context reviewers independently checked requirements/scope,
TI register semantics and verification/peer evidence. The lead checked findings
against actual contracts and source code. No production code exists to execute
in this documentation stage.

| Finding | Correction and source |
| --- | --- |
| Rejection accounting could be misread | Clarified the vector introduction and N18: admission rejection is bus-silent, but base recovery first reads 45 and counts that callback before rejecting nonzero internal bits with CONFIGURATION_UNKNOWN and zero writes. This follows the existing baseline recovery rule and prompt 4 prerequisite, without changing the procedure. |
| Inaccurate peer retention wording | PCA9555 tools/run_i2c_hil.py::extract_evidence retains at most ten classifier evidence lines; its report explicitly says raw CLI output is not retained. Corrected the peer report's claim of bounded transcripts. |

Repeated verification:

- The silicon reviewer visually inspected the archived B pp16–19, L/A pp16–20
  and V pp17–20. Register/default/access/encoding claims and unresolved V gates
  agree with those tables; no silicon or encoding correction was identified.
- All eight archived PDF byte counts/SHA-256 match provenance, with no changed
  archive bytes or metadata. All 13 cached pinned public files passed size,
  SHA-256 and Git-blob checks; reset-mask, IRQ and reserved Try.SRC helper behavior
  was inspected. This review did not repeat historical official download checks.
- Eight disjoint, complete mask partitions and all 21 C compositions passed
  independent arithmetic. D01–D36 and N01–N25 were checked against source and
  baseline contracts; all 82 vector IDs remain unique and consecutive. These are
  specification checks, not executed driver behavior tests.
- PCA9555, LDC1614 and ADS1115 remain clean at their recorded revisions. Relevant
  declarations, implementations, examples, runner meanings and all 18 cited test
  definitions/registrations were checked without sibling fetches or builds.
- Local Markdown links/anchors, prompt numbering, JSON, UTF-8/LF/final-newline
  and whitespace checks were repeated after the corrections. The existing 38
  numbered prompts and source artifacts are unchanged.

The review found no further in-scope defect or unnecessary implementation to
remove. Driver/CLI behavior, firmware/package builds and hardware qualification
remain NOT RUN. Remote CI was not dispatched or polled. Prompt 4 is NOT STARTED.

## Prompt 4: transport, procedures and timing, 2026-10-23

Scope: complete the device procedure specification, independently derived bounds
and failure/time vectors. Started from synchronized prompt 3 review commit
`ff0d6a21b65579b7bc3b9b454b16a14b21b74257` on the existing execution branch.
Two strong reasoning reviewers checked silicon and time/effects separately; a
coding reviewer re-audited peers and checked coverage. All reviewed the actual
guide and affected contracts. The lead checked the findings against PDF pages,
peer implementations, pinned helper bytes and the scoped diff.

| Check | Result | Evidence and limits |
| --- | --- | --- |
| Exact TI sources | PASS | Visually inspected B pp7,11,12,14,15,18,19; L/A pp6,14,15,18,19; V pp6,7,9,10,11,15,19,20. Lead independently rendered B p14 and A pp6,14 and read the other relevant procedure/register text. No device measurements implied |
| Current source check | PASS | Fresh B/L/A/V official downloads at 2026-10-23T15:34:18Z matched archived bytes. Current product lists still link F/D/D/D; inspected lists and targeted TI search found no applicable separate erratum, without claiming exhaustive absence |
| Archive and pinned helpers | PASS | Eight PDF sizes/SHA-256 and archive coverage match provenance; archive/metadata unchanged. Four freshly fetched TI-hosted Linux/regmap files passed 12 size/SHA-256/Git-blob comparisons; lead repeated them and checked reset-mask arithmetic, error-losing cleanup and combined framing |
| Procedure ceilings | PASS | Independent Python expanded ordered R/W primitives for all 13 table rows and34 applicable variant triples. Counts include pointers and exclude address phases; failures shorten plans. Four-callback recovery is 6 TX/2 RX, service 5 callbacks/6 TX/4 RX |
| Timing arithmetic | PASS | 11,988 independent Fraction-based cases:999 fractional completion offsets ×4 fresh-sample delays ×3 intervals. Six/31/96 ticks never shorten 5/30/95 ms; omitted-margin counterexamples checked. Literal role/reset/ACK bytes and uint64 rollover checked separately |
| Future vector inventory | PASS | P01–P30 and T01–T12 unique/consecutive;30 procedure/failure and12 time cases. Write-by-write failure/cancel/deadline/end matrix specified. These are specification checks, not 42 executed driver tests |
| Focused peers | PASS with recorded concurrent docs changes | Initial four HEADs/branches clean;18 named test definitions and RUN_TEST registrations checked. Later LDC/ADS docs-only edits were left untouched; inspected source/test/build files and all four revisions unchanged. No sibling fetch/build |
| Documentation and scope | PASS |76 Markdown files,422 local links/anchors;38 numbered prompts unchanged. UTF-8/LF/final newline, JSON and whitespace checks; no production code, generated PDF images or downloaded source cache added |

The fresh official sources are [B](https://www.ti.com/lit/ds/symlink/tusb320.pdf),
[L](https://www.ti.com/lit/ds/symlink/tusb320li.pdf),
[A](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf) and
[V](https://www.ti.com/lit/ds/symlink/tusb322i.pdf). Original terms and archived
source metadata were preserved. The four public helper records remain pinned
to Linux commit 97becb13b81c314c1d17fcf2226aac9a32086f3d in the existing manifest.

Review corrections made before completion:

- Qualified ordinary 45 writes with known internal 03=00. The base prerequisite
  is explicitly conservative policy, not an invented numeric default. Preserve
  independent read/other-field work; reject attempts to repair protected bits.
- Recovery rejects a different unresolved reset/mode transition, while an explicit
  new request can restart its own uncertain recovery. No new callback is hidden
  in its four-callback plan.
- Defined reset settling evidence separately from command acceptance and desired
  equality. Reset's own success cannot erase older recovery uncertainty. No-op
  apply still performs full comparison; zero planned writes cannot imply success.
- Any observed asserted reset bit retains pending evidence, including profile,
  preparatory and raw reads. A later clear bit cannot bypass the 96-tick interval;
  first discovery during F ends unknown without appending another proof pass.
- Corrected the application-note date and base W1C citation; aligned the exact
  completedTimeValid field name and shared write-identity prerequisites.

Checks used ad hoc session Python, PyMuPDF rendering, primary-source browsing,
read-only peer inspection and `git diff --check`. Bounds are reproducible from
R(n)=(1 callback,1 TX,nRX), W=(1,2,0), I=R(8)+optionalR(1),
P=three scalar reads+optional45, F=I+P. Add each procedure's stated writes once;
waits add no transfers. Time checks use rational physical timestamps and compare
earliest integer execution directly with real completion, independently of future
production helpers. No reusable test framework was added for this source-free step.

Unresolved V advertisement/fault/direct-DRP writes, startup polarity, attached-
current freshness and base valid-time units remain explicit gates/prerequisites.
Adapter aggregate bounds/framing traces, driver/CLI behavior, firmware/package
builds and hardware qualification are NOT RUN. Remote CI was not dispatched or
polled. Prompt 5 remains NOT STARTED.

## Independent prompt 4 review, 2026-10-23

Reviewed `d5884da8f1a5b17d38f506b440a66a710c75e7fb` and its actual diff from
`ff0d6a21b65579b7bc3b9b454b16a14b21b74257`, after rereading the full original
prompt. Three fresh-context reviewers independently audited coverage/contracts,
TI procedures and bounds/verification/peer claims. The lead checked the actual
guide, scoped diff and peer deltas and repeated independent arithmetic.

No silicon/procedure defect, omitted requirement or incorrect bound was found.
The guide now expands the reset and ACK mask alternatives into explicit
hexadecimal expressions per variant and names the implicit-pointer wording
conflict precisely. Its ACK formula previously contained an unescaped bitwise-OR
delimiter inside a Markdown table cell; moving the formula below the table
restores its three columns. There is no change to enabled procedures or gates.

Repeated checks:

- Visually inspected B pp7,11–12,14–16,18–19; L/A pp6,10,13–15,18–20;
  V pp6–7,9–16,18–20; N pp5,9. The silicon reviewer freshly downloaded all four
  exact B/L/A/V PDFs at 2026-10-23T15:57:28Z; every SHA-256 matched the archive.
  The review confirms the cited source revisions and does not add hardware proof.
- All eight PDF byte counts/SHA-256 and archive coverage passed. Four cached
  pinned Linux/helper files passed 12 size/SHA-256/Git-blob comparisons; helper
  contents agree with the documented reset-mask/cleanup/IRQ counterexamples.
  No archive byte, provenance field or numbered prompt changed.
- All 34 applicable callback/TX/RX triples across 13 plan rows passed independently
  expanded R/W arithmetic. Literal role/reset/ACK bytes, all 30 P vectors and 12 T
  vectors, 11,988 fractional-time cases and uint64 rollover/half-range boundaries
  passed. These remain specification checks, not executed driver tests.
- All 18 named peer test definitions and actual RUN_TEST registrations passed
  inspection. Relevant implementation, test-body and workflow claims agree.
  The current peer revisions below are clean on main with equal local upstream
  refs; siblings were not fetched. Their changes since the first prompt 4 audit
  affect only README/documentation indexes and retained-branch audit documents.
- All 76 Markdown files and 422 local links/anchors passed; JSON parses and
  UTF-8/LF/final-newline/whitespace checks pass after the clarifications.
- All 118 procedure-guide table rows have consistent column counts. The ACK row
  now has the intended three columns, and all four explicit reset/ACK formulas
  reproduce their existing literal vectors. A separate reviewer confirmed the
  final four-file diff changes no procedure semantics or transfer bounds.

| Peer | Current HEAD | Change to inspected implementation/test/build files |
| --- | --- | --- |
| PCA9555 | 498178e4d9e4acb6fab7d8b816ad5021efe8806a | None |
| LDC1614 | 1db38b63cb93a561b6e0bea51017ab94102ebd24 | None |
| ADS1115 | 235af3e8c722661a89a44766a4ccd903a654f6fe | None |
| LSM6DS3TR | e262bf22f3d0cf035fbbca10f45b3e0752bf2bac | None |

The dated original peer audit is preserved as historical evidence. No source
code, new framework or runtime abstraction was introduced. Driver/adapter and
hardware checks remain NOT RUN; remote CI was not dispatched or polled.
Unresolved silicon/board gates remain explicit. Prompt 5 is NOT STARTED.

## Prompt 5: portable values and API inventory, 2026-10-23

Scope: implemented passive public headers and constexpr helpers; exact planned
callable inventory, architecture and reusable host/compile-only fixtures.
GCC 15.1.0 (MinGW-W64 x86_64 UCRT) and Doxygen 1.13.2 were executed locally.
No framework SDK is needed by these headers. No driver, validator or decoder
behavior is inferred from compiling a declaration sketch.

| Check | Result | Evidence |
| --- | --- | --- |
| Focused peer re-audit | PASS | All five current clean local HEADs, actual source/test bodies and registration in execution-5-peers; no sibling fetch/build |
| Independent delivered headers | PASS | Status.h, Config.h, TUSB32x.h and CommandTable.h each compiled alone in strict C++17 |
| Real passive consumer | PASS | test_compile_contracts.cpp linked with status_other_tu.cpp and executed with NDEBUG; checks remain active in Release |
| Status/error contract | PASS | All 23 exact ordered names from both TUs, constexpr helpers, explicit bool, invalid codes 23/127/255, full int32 extremes and wide cross-TU detail |
| Fixed values and defaults | PASS | All 17 structs, callback signatures, uint64 token/time, exact seven-bit addresses, default configuration/port/IRQ/diagnostics and independent validity |
| Planned methods | PASS, syntax only | PlannedApi.h and planned_api.cpp outside include/; exact baseline inventory and noncopy/nonmove checked, never linked |
| Source Doxygen | PASS | EXTRACT_ALL=NO, undocumented/enum/parameter/incomplete/doc warnings treated as errors; all 17 public structs present in XML index |
| Documentation and scope | PASS | Local links/anchors, UTF-8/LF/final newlines, JSON, whitespace and no reference-byte/numbered-prompt edits; final counts below |
| Independent contract review | PASS | Header/fixture/API/architecture inspection against original prompt and baseline; findings corrected before final checks |

Actual C++ commands, executed from the repository root using PowerShell's
`$p5Flags` and `$p5Output` as defined in [test instructions](../test/README.md):

```text
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -I include -x c++ -fsyntax-only -
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -I include -DNDEBUG test/test_compile_contracts.cpp test/consumer/status_other_tu.cpp -o <TEMP>/tusb32x-p5-checks/passive-contracts.exe
<TEMP>/tusb32x-p5-checks/passive-contracts.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -I include -fsyntax-only test/contract/planned_api.cpp
```

The first command ran four times, each with one `#include <TUSB32x/header.h>`
on stdin. The executable printed:
`PASS passive values, 23 error names, constexpr Status and multiple translation units`.
No expected-failure compilation or static assertions were suppressed.

The final Doxygen command was `doxygen -`, with this stdin configuration
(absolute paths resolved from the checkout and TEMP):

```text
PROJECT_NAME = TUSB32x
OUTPUT_DIRECTORY = "<TEMP>/tusb32x-p5-checks/doxygen"
INPUT = "<checkout>/include/TUSB32x"
RECURSIVE = YES
EXTRACT_ALL = NO
WARN_IF_UNDOCUMENTED = YES
WARN_IF_DOC_ERROR = YES
WARN_IF_INCOMPLETE_DOC = YES
WARN_NO_PARAMDOC = YES
WARN_IF_UNDOC_ENUM_VAL = YES
WARN_AS_ERROR = YES
GENERATE_XML = YES
GENERATE_HTML = YES
GENERATE_LATEX = NO
QUIET = YES
```

XML inspection independently required exactly the 17 published structs: Status,
DriverConfig, DeviceProfile, OperationToken, OperationTiming, PortSnapshot,
ConfigurationSnapshot, IdentitySnapshot, InterruptSnapshot, OperationResult,
OperationProgress, TransportStats, ManagedTransportEvidence, HealthPolicy,
HealthSnapshot, DriverDiagnostics and RegisterDecode. This is a direct stage check;
the reusable Doxygen guard/negative fixtures remain assigned to later prompts.

Review fixes clarified operation-local IRQ results versus historical diagnostic
IRQ evidence, mode settling versus proof of an old sequence, adapter-added retry
scope, health-helper staging and a test's ABI wording. No silicon rule changed.
No driver behavior or packaging metadata was added. The only
new implemented methods are complete constexpr Status/token helpers; all remaining
callables stay in docs/contract fixtures until their owning implementation stage.

Final documentation checks passed for 80 Markdown files and 452 local links/anchors.
All 38 numbered prompts retain their contents and numbering. Archived TI sources
and provenance are unchanged. The first author Doxygen attempt needed its missing
output parent created, then passed; the lead's final run used TEMP and passed.
Generated output under build/ is ignored and not part of the deliverable.

NOT RUN: validators, register decoding, driver transport/state/health behavior,
framework or package builds and physical hardware qualification. Those methods
are not implemented in prompt 5. Remote CI was neither dispatched nor polled.
Prompt 6 remains NOT STARTED. This stage's passed checks establish the portable
value contract and planned signature syntax only.

## Independent prompt 5 review, 2026-10-23

The full original prompt and actual `d29cb36b3e0f5ed57dc5512964ce40e2b5326ae2`
to `c5fcc983e36cd3c79005a420fc4db7b752c73346` diff were independently reviewed.
Three fresh-context reviewers checked coverage/simplicity, contract correctness
and edge cases, and executable verification/peer evidence. The lead inspected
the code and contracts directly and repeated checks, rather than relying on
the previous completion report. No confirmed defect or missing in-scope
deliverable was found; no source/API/test correction was needed.

Fresh checks passed:

- All four delivered headers compiled independently with GCC 15.1.0, C++17,
  Wall/Wextra/Wpedantic/Wconversion/Wsign-conversion and Werror.
- The two-TU consumer compiled, linked and executed with NDEBUG. Its runtime
  checks stay active: all 23 names/order, invalid error names, explicit bool,
  constexpr Status, full int32 detail, callback shapes and fixed/default values.
  The lead reran the commands recorded above with output under
  `<TEMP>/tusb32x-p5-independent-lead/`; the expected PASS line was printed.
- The planned API fixture passed syntax-only compilation. All 26 baseline
  facade signatures match the guide and fixture; the ordered 23-code Err set
  matches the baseline. No missing callable definition is published in include/.
- Strict Doxygen 1.13.2 passed with the same warning settings recorded above;
  XML showed exactly all 17 public structs. The lead's new output is under
  `<TEMP>/tusb32x-p5-independent-lead/doxygen/`. Two reviewers independently
  reproduced compilation, linkage and documentation results in separate TEMP paths.
- All 80 Markdown files and 452 local links/anchors passed, as did JSON parsing,
  UTF-8/LF/final-newline/whitespace and scoped diff checks. All 38 numbered prompts,
  archived sources, production headers and test fixtures are unchanged by this review.

The peer reviewer rechecked all five HEADs, branches, clean states and local
upstream refs; all match execution-5-peers.md with local ahead/behind 0/0.
Relevant declarations, implementation and test bodies, examples, runner records,
AGENTS and registration were inspected again and support the report. Siblings
were neither fetched nor modified; local refs do not prove server state.

Reviewed boundaries include static-message lifetime, exact error origins,
terminal callback ownership, raw versus interpreted validity, identity versus
current initialization, operation-local IRQ versus historical diagnostics,
independent recovery/mode/reset uncertainty and single-byte decoder field codes.
No driver stub, duplicate decoder, framework dependency or speculative hierarchy
was introduced. Compile-only signatures remain clearly separate from real behavior.

Only audit/progress/changelog records changed. Detailed owner transitions,
validators, decoder and device operations remain future work. Protocol behavior,
framework/package builds and hardware checks are NOT RUN; remote CI was not
dispatched or polled. Prompt 6 is NOT STARTED.

## Prompt 6: owner, state, time and evidence contracts, 2026-10-25

Scope: normative future method/state behavior, complete ownership/publication
tables, compile-only owner examples and one additional passive diagnostic flag.
The starting source was 87979dc07fc319603f0bd6ed21224b45ea20d1d8 on the existing
execution branch. No operation engine, adapter, validator or health function was
implemented or linked. The lead reread the original prompt and baseline and
verified returned reviews against source, test bodies and the actual diff.

| Check | Result | Actual evidence |
| --- | --- | --- |
| Focused peer audit | PASS, source review | Six clean local revisions; exact code/test/runner/registration sources in execution-6-peers. Lead also read TMP exact take/registered sentinel tests, INA replay path and FieldCore wrapping-time helper |
| Four independent public headers | PASS | MinGW GCC15.1.0, C++17, Wall/Wextra/Wpedantic/Wconversion/Wsign-conversion/Werror; include each alone via stdin |
| Real two-TU passive consumer | PASS | Compile/link/run with NDEBUG; all23 errors, constexpr/explicit bool/int32 details/defaults, independent raw/effect storage including configurationWriteUnresolved |
| Planned callable inventory | PASS, syntax only | planned_api.cpp; no future method linked or implemented |
| External owner examples | PASS, syntax only | owner_examples.cpp: passive observation, explicit apply, cancelled possible write, exact take and two devices sharing an external bus/context; fake callbacks return I2C_ERROR |
| Integer clock specification | PASS, compile time | time_arithmetic.cpp: 60 static assertions, including half-range/equal/backward/wrap, wide timeout clamp, 6/31/96 waits and checked uint32 extension/output preservation |
| Source Doxygen | PASS | Doxygen1.13.2, EXTRACT_ALL=NO and all documented warning checks fatal; XML index exposes all17 public structs including updated diagnostic value |
| Procedure bounds and physical-time arithmetic | PASS, specification only | 13 existing table rows,34 variant count triples;30 P and12 T vectors, literal bytes/rollover and11,988 exact Fraction cases for sub-ms completion; no procedure count changed |
| Source provenance | PASS | All8 archived PDFs retain size/SHA-256 and unchanged Git bytes |
| Documentation/source integrity | PASS | Local Markdown links/anchors, complete38-prompt order, UTF-8/final-newline/whitespace and actual scoped diff; no source/library build promises added |
| Independent final reviews | PASS, source review | Separate coverage/simplicity and strong time/effect reviewers; corrected initialization baseline, incomplete-F preservation, raw uncertainty and precise publication/invalidation boundaries |

Executed commands use the full recipe in [test/README](../test/README.md).
The final run used `$env:TEMP/tusb32x-p6-checks` for outputs:

```powershell
$contractOutput = Join-Path $env:TEMP 'tusb32x-p6-checks'
New-Item -ItemType Directory -Force -Path $contractOutput | Out-Null
$contractFlags = @('-std=c++17','-Wall','-Wextra','-Wpedantic',
                   '-Wconversion','-Wsign-conversion','-Werror','-I','include')
foreach ($header in @('Status.h','Config.h','TUSB32x.h','CommandTable.h')) {
    "#include <TUSB32x/$header>" | & g++ @contractFlags -x c++ -fsyntax-only -
    if ($LASTEXITCODE -ne 0) { throw "Header failed: $header" }
}
& g++ @contractFlags -DNDEBUG test/test_compile_contracts.cpp `
    test/consumer/status_other_tu.cpp -o "$contractOutput/passive-contracts.exe"
if ($LASTEXITCODE -ne 0) { throw 'Passive consumer compile/link failed' }
& "$contractOutput/passive-contracts.exe"
if ($LASTEXITCODE -ne 0) { throw 'Passive consumer checks failed' }
& g++ @contractFlags -fsyntax-only test/contract/planned_api.cpp `
    test/contract/owner_examples.cpp test/contract/time_arithmetic.cpp
if ($LASTEXITCODE -ne 0) { throw 'Planned contract compilation failed' }
```

Doxygen ran from stdin (`doxygen -`) with INPUT=include/TUSB32x,
OUTPUT_DIRECTORY pointing to that TEMP directory's doxygen subdirectory,
RECURSIVE=YES, EXTRACT_ALL=NO, WARN_IF_UNDOCUMENTED=YES, WARN_IF_DOC_ERROR=YES,
WARN_IF_INCOMPLETE_DOC=YES, WARN_NO_PARAMDOC=YES, WARN_IF_UNDOC_ENUM_VAL=YES,
WARN_AS_ERROR=YES, GENERATE_XML=YES, GENERATE_HTML=YES, GENERATE_LATEX=NO and
QUIET=YES. The generated XML index was parsed and required exactly17 public
TUSB32x structs. No generated files or tool dependencies enter the library.

The reused independent Python procedure check derives I/P/F/S read/write plans,
compares their length/TX/RX counts to the maintained table, checks the30/12 vector
IDs and literal byte equations, and uses exact Fraction arithmetic for completion
fractions1–999 microseconds with four post-return delays for each5/30/95 ms wait.
It also verifies archive byte lengths/hashes and unchanged tracked PDF bytes.
This is specification verification; later code still must meet those bounds.

Reviews found and fixed the recovery identity/trust deadlock and the ordinary
raw-write uncertainty gap. The latter needed one public passive boolean rather
than hidden gate history or inferred old effects. Further corrections preserve
old actual proof on unsuccessful read-only verification, freeze callback failures
without made-up time, defer successful publication until a fresh deadline check,
retain immediate adverse evidence/ACK acceptance, and order active end/invalidate
before freezing cancelled snapshots. O01–O25 record the required future runtime
regressions; none is reported as a passed driver behavioral test.

NOT RUN: runtime driver/transport/health behavior, real adapter timing, firmware,
package consumption and physical hardware. The contract/example syntax and
integer arithmetic prove none of these. No remote CI was dispatched or polled,
as required for this first promptset outside prompt34. Prompt7 is NOT STARTED.

## Independent prompt 6 review, 2026-10-25

Audited the full original prompt and actual `87979dc..b129c37` diff on the same
execution branch, starting clean at b129c37be1feeba9c4474f8b1788a9b42c9ac86e.
Fetch confirmed local/upstream equality and zero ahead/behind. Three fresh-context
reviewers independently covered correctness/edge cases, requirements/simplicity,
and verification/peer provenance. The lead checked the source and diff directly.

No confirmed code or contract defect was found. Every required method, admission,
result, lifetime, independent state, time, publication and health rule is present.
The new settings-uncertainty flag has a specific retained-evidence purpose.
Examples remain compile-only, with no driver definitions, successful fake-device
behavior or framework dependency. The initialization-readiness refinement is
consistent across the baseline, API guide, values and recovery prerequisites.

Repeated checks passed:

- GCC 15.1.0 strict C++17 standalone compilation of all four public headers, linked
  NDEBUG two-translation-unit passive test execution, and syntax checks of
  planned_api.cpp, owner_examples.cpp and time_arithmetic.cpp (60 static assertions).
  Commands are the prompt 6 recipe above; review output directories are under TEMP.
- Doxygen 1.13.2 with the same fatal warnings and EXTRACT_ALL=NO; generated XML
  contains all 17 public structs. The source Doxygen agrees with the contracts.
- Current peer HEAD/branch/dirty states match all six entries in execution-6-peers.
  Direct implementation/test/registration checks confirmed exact-token delivery,
  delayed callback boundaries, desired/verified separation, rejected replay and
  checked uint32 wrap conversion. Peer tests/builds/fetch and remote CI were not run.
- Independent procedure-bound and timing arithmetic, archived PDF byte/hash
  checks, Markdown links/anchors and scoped Git whitespace checks passed. These
  remain specification/evidence checks, not future driver behavioral results.

The review identified a reproducibility gap in the written validation record:
the fractional-time calculation lacked its complete command. This is the exact
standalone calculation used for the 11,988 cases, runnable from PowerShell with
Python's standard library (executed with Python 3.12.10):

```powershell
@'
from fractions import Fraction

cases = 0
for duration_ms in (5, 30, 95):
    for fraction_us in range(1, 1000):
        completed_ms = Fraction(108000 + fraction_us, 1000)
        for delay_ms in (0, 1, 2, 10):
            fresh_sample_ms = completed_ms + Fraction(1, 10000) + delay_ms
            anchor_ms = int(fresh_sample_ms)
            ready_ms = anchor_ms + duration_ms + 1
            assert Fraction(ready_ms) - completed_ms >= duration_ms
            if delay_ms == 0:
                assert Fraction(anchor_ms + duration_ms) - completed_ms < duration_ms
            cases += 1
assert cases == 11988
print(f'PASS {cases} exact fractional-time cases')
'@ | python -
if ($LASTEXITCODE -ne 0) { throw 'Fractional-time specification check failed' }
```

The command proves the stated integer scheduling relationship for the listed
grid of completion fractions and fresh-sample delays. It does not prove that an
adapter samples freshly, meets its timeout, or that a future engine obeys the
contract. Original procedure ceilings and TI sources are unchanged; no physical
qualification, runtime state machine, firmware or package result is claimed.
Prompt 7 remains NOT STARTED. This review changes documentation only.

## Prompt 7: portable host foundation, 2026-10-26

Started from cdbb36cda836ef1cb7c4d66864d93c5ec2b31418 on the recorded execution
branch. The worktree/index were clean and fetch confirmed same-name upstream
commit equality, zero ahead/behind. Prompts 1-6 and their checks were revisited.
The [focused audit](audit/execution-7-peers.md) records current full revisions,
clean states, actual declarations/code/tests/examples/runners/workflows and
adopted/rejected conventions for all six requested peers. No sibling was changed,
fetched or built. TUSB32x had no workflow definitions at the starting revision;
remote CI remains confined to prompt 34.

Tool versions actually executed: CMake/CTest 4.0.1, Ninja 1.12.1, GCC 15.1.0
(MinGW-W64 x86_64 UCRT MCF SEH), Doxygen 1.13.2, Python 3.12.10. Python and
Doxygen are maintainer checks, not library build dependencies. MSVC and Clang
were not available on PATH and were NOT RUN.

| Check | Result | Evidence and limit |
| --- | --- | --- |
| Native Debug | PASS | Explicit-source C++17 static library, four independent header TUs and three planned-contract object fixtures compile; both registered CTest suites execute |
| Native Release | PASS | Same build/checks; exactly two discovered/executed tests, strict private warnings and -UNDEBUG on runtime suites |
| Validator behavior | PASS | 76,452 cases: seven known variants, every underlying address/invalid variant byte, missing callbacks, null/non-null context, timeout 0/1/20/UINT32_MAX, full legal-enum profile grid, malformed field bytes, invalid-before-unsupported precedence, unchanged inputs and retained status text; zero callback attempts |
| Passive contracts | PASS | Existing all23 error names/order, full int32 backend detail, constexpr/explicit bool, default validity and multiple-TU ODR/link checks; callbacks are not executed |
| NDEBUG fixtures | PASS | Test agent independently compiles/runs validators using strict GCC with -DNDEBUG; explicit checks stay active |
| Ordinary consumer | PASS | Separate Ninja Release configure/build/run with tests default OFF; two application TUs link actual validators and report OK/OK/zero fake callbacks |
| Clean isolated consumer | PASS | Only root CMake, four headers, one source and consumer copied to a renamed tree under unrelated parent Git metadata; no own .git, tests or docs; Git/Python/Doxygen/PlatformIO absent from build PATH; strict Release build/run succeeds |
| Public standard propagation | PASS | Isolated consumer configured with CMAKE_CXX_STANDARD=11; all three effective compile commands use C++17 inherited from TUSB32x, without consumer include paths or feature overrides |
| Source Doxygen | PASS | Fatal undocumented/enum/parameter/incomplete/error warnings, EXTRACT_ALL=NO; 17 public structs plus both validators/errorName visible in generated XML; includes production/example sources |
| Exact TI sources | PASS | All eight archived PDF sizes/hashes match provenance; four fresh official PDFs byte-identical to archives; relevant p15 address tables and B/L/A/V profile fields reopened, no rule/gate changed |
| Documentation/review | PASS | Full scoped code/docs review and independent reviewer; stale coverage disclaimer corrected, real validators removed from planned-only declarations/inventory; Markdown links/anchors, UTF-8 and whitespace checked |
| ASan/UBSan | BLOCKED | Minimal GCC link probe fails: cannot find -lasan and -lubsan. Installed MinGW supplies neither runtime; WSL is not installed. No sanitizer pass claim |
| Driver, firmware, packages, hardware | NOT RUN | No device facade/transport/operations or adapters implemented; no install/export/archive qualification or physical fixture in this stage |

Exact registered commands, from the repository root in PowerShell:

```powershell
$env:CMAKE_GENERATOR = 'Ninja Multi-Config'
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Release -N
& build/native/test/Release/tusb32x_validators.exe
```

CTest discovers exactly tusb32x_compile_contracts and tusb32x_validators. Each
has a 30-second limit. The compile-only fixtures have no CTest entry and no
unimplemented facade definition. Test/example fake callbacks fail if invoked;
no successful fake device or hardware initialization is advertised.

The test agent also ran strict direct compilation/link/execution with
`g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror
-DNDEBUG -I include test/test_validators.cpp src/TUSB32x.cpp -o <TEMP>/...`.
The standalone ordinary commands were:

```powershell
cmake -S examples/host/basic -B "$env:TEMP/tusb32x-p7-host-basic" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$env:TEMP/tusb32x-p7-host-basic"
& "$env:TEMP/tusb32x-p7-host-basic/tusb32x_host_basic.exe"
```

Its cache confirms TUSB32X_BUILD_TESTS:BOOL=OFF. The stronger isolated consumer
check is reproducible with this Python session command (`python -` from the
root). Python prepares/launches the check; no child build process can find it:

```python
from pathlib import Path
import json, os, shutil, subprocess
root = Path.cwd()
parent = (root / 'build/isolated-parent').resolve()
source = parent / 'renamed-library'
parent.mkdir(parents=True, exist_ok=True)
subprocess.run(['git', 'init', '--quiet', str(parent)], check=True, timeout=20)
files = [Path('CMakeLists.txt'), *Path('include/TUSB32x').glob('*.h'),
         Path('src/TUSB32x.cpp'), *Path('examples/host/basic').glob('*')]
for relative in files:
    if relative.is_file():
        destination = source / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(root / relative, destination)
assert not (source / '.git').exists()
cmake, ninja, cxx = map(shutil.which, ['cmake', 'ninja', 'g++'])
env = dict(os.environ)
windows = Path(os.environ['SystemRoot'])
env['PATH'] = str(windows / 'System32') + ';' + str(windows)
for tool in ['git', 'python', 'doxygen', 'pio']:
    assert shutil.which(tool, path=env['PATH']) is None
build = parent / 'consumer-build'
subprocess.run([cmake, '-S', str(source / 'examples/host/basic'), '-B', str(build),
    '-G', 'Ninja', f'-DCMAKE_MAKE_PROGRAM={ninja}', f'-DCMAKE_CXX_COMPILER={cxx}',
    '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_CXX_STANDARD=11',
    '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
    '-DCMAKE_CXX_FLAGS=-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror'],
    env=env, check=True, timeout=60)
subprocess.run([cmake, '--build', str(build)], env=env, check=True, timeout=60)
subprocess.run([str(build / 'tusb32x_host_basic.exe')], env=env, check=True, timeout=60)
assert 'TUSB32X_BUILD_TESTS:BOOL=OFF' in (build / 'CMakeCache.txt').read_text()
commands = json.loads((build / 'compile_commands.json').read_text())
assert len(commands) == 3
assert all('-std=c++17' in c['command'] or '-std=gnu++17' in c['command']
           for c in commands)
```

Doxygen used `doxygen -` with the exact prompt5 configuration above, changing
OUTPUT_DIRECTORY to build/prompt7-doxygen and INPUT to
`include/TUSB32x src examples/host/basic`. Generation is bounded to 60 seconds;
XML index parsing requires exactly17 TUSB32x structs and members
validateDriverConfig, validateProfile and errorName. The first attempt found an
undocumented example main; its brief/exit semantics were added and generation
then passed without suppressing warnings. The reusable guard/negative fixtures
remain later work.

Sanitizer availability was tested with the exact PowerShell command:

```powershell
'int main() { return 0; }' | & g++ -x c++ '-fsanitize=address,undefined' -o build/sanitizer-probe.exe -
```

The linker exits1 with missing -lasan/-lubsan; no SDK/tool installation was made.
Independent review found no functional/build defect; its coverage documentation
finding was fixed. Source, examples and all affected working/API/coverage guides
were reviewed together. No callback normalization, state engine, write permission
or hardware result is claimed by validator success. All generated artifacts stay
under ignored build/ or TEMP. Final commit/sync evidence is reported in the
handoff and next progress update, without a self-referencing hash edit.

## Independent prompt 7 review, 2026-10-26

Audited the complete original prompt 7 and actual cdbb36c..d221635 diff from
clean synchronized starting commit `d22163546c927a03717322f2f47aeb0798d4b1c6`.
Three fresh-context read-only reviewers independently checked coverage/scope,
correctness/edge cases/simplicity, and tests/current peers. The lead confirmed
their findings against source and exact archived TI tables. The refreshed
[peer audit](audit/execution-7-peers.md#independent-prompt-7-review-2026-10-26)
records unchanged full revisions, clean states, inspected paths and choices.

No functional defect, missing prompt 7 deliverable, material test gap or needless
abstraction was found. Fixed a source comment's TI table reference (SLLSEO5D
Table 6-6 is the register inventory; Table 6-7 describes identity). Corrected
damaged range punctuation in evidence, clarified the Ninja consumer Release
configure flag, and updated the feature-table description. No API, validator
logic, CMake behavior, test assertion or silicon rule changed.

Tools were freshly queried and remain CMake/CTest 4.0.1, Ninja 1.12.1, GCC 15.1.0
MinGW-W64 UCRT MCF SEH, Doxygen 1.13.2 and Python 3.12.10. Exact lead commands:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug --clean-first
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Release --show-only=json-v1
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -DNDEBUG -O2 -I include test/test_validators.cpp src/TUSB32x.cpp -o build/p7-independent-audit/validators-ndebug.exe
& build/p7-independent-audit/validators-ndebug.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -DNDEBUG -O2 -I include test/test_compile_contracts.cpp test/consumer/status_other_tu.cpp -o build/p7-independent-audit/contracts-ndebug.exe
& build/p7-independent-audit/contracts-ndebug.exe
```

The existing build/native generator is Ninja Multi-Config. PASS: both suites in
Debug and Release, exactly two registered tests with 30-second limits, and both
direct NDEBUG executables. Validators report 76,452 cases and zero callbacks;
passive checks report all 23 error names, constexpr Status and multiple TUs.
Four independent public-header TUs and three planned-contract object fixtures
also compile; no build-only fixture has a behavioral CTest entry.

The lead repeated the isolated consumer Python command from the prompt 7 record
above, using parent `build/p7-independent-audit/parent`. PASS: only current root
CMake, four public headers, the source unit and consumer files in a renamed tree;
unrelated parent Git metadata, no own .git or test directory; child PATH contains
only Windows system directories and cannot find Git/Python/Doxygen/PlatformIO.
Tests default OFF. Strict Ninja Release configure/build/run succeeds. An explicit
C++11 consumer baseline becomes C++17 in all three compile commands through the
library target's public requirement; no framework or sibling path is needed.

Doxygen was rerun with the exact prior fatal-warning configuration and INPUT,
OUTPUT_DIRECTORY=`build/p7-independent-audit/doxygen`. PASS: no warning; XML
contains the exact 17 expected public struct names and validateDriverConfig,
validateProfile and errorName. All eight archived PDF sizes/SHA-256 match
provenance. The lead reopened p15 addresses and relevant control/profile tables
in B/L/A/V, including V p17's corrected inventory citation. Local links/anchors,
UTF-8, range text and scoped whitespace/diff checks pass.

Each reviewer additionally ran its own fresh Release build/CTest and consumer;
the requirements reviewer also ran Debug and isolated consumption without
maintainer tools. Artifacts are confined to ignored build/p7-audit-* and
build/p7-independent-audit. These are executed local checks, separate from peer
source inspection and from still-planned driver behavior.

The fresh sanitizer command below fails to link -lasan/-lubsan, so ASan/UBSan
remain BLOCKED rather than passed. No toolchain was installed:

```powershell
'int main() { return 0; }' | & g++ -x c++ '-fsanitize=address,undefined' -o build/p7-independent-audit/sanitizer-probe.exe -
```

MSVC/Clang, firmware, archive/package qualification and physical hardware remain
NOT RUN. No remote CI was dispatched or polled, and no sibling was edited or
built. The same execution branch is retained; final commit/sync evidence is
reported in the handoff and next progress update.

## Prompt 8: deterministic metadata and native integration, 2026-10-26

Started at clean synchronized 93f2d31374dc16bcc95800d0e0c516c4372faa4a on the recorded
execution branch, zero ahead/behind after fetch. Prompts 1-7 and prerequisite native
checks were revisited. The [focused peer audit](audit/execution-8-peers.md) records
actual current source/AGENTS/manifests/generators/tests/guards/workflows, full
revisions, clean states and adopted/rejected choices. No sibling was edited,
fetched or built, and no remote CI was dispatched or polled.

Executed tools: CMake/CTest 4.0.1, Ninja 1.12.1, GCC 15.1.0 MinGW-W64 UCRT MCF SEH,
Python 3.12.10, Doxygen 1.13.2 and existing VS Code-managed PlatformIO Core 6.1.19.
The native environment pins platformio/native 1.2.1; Core resolved and installed
tool-scons 4.40801.0 from its configured source. This is a managed dependency,
not a second Core. A preliminary package-list command hit the console's cp1252
Unicode limitation; subsequent actual Core commands used process-local
PYTHONIOENCODING=utf-8, without changing account settings or the launcher contract.

| Check | Result | Evidence and limits |
| --- | --- | --- |
| Version commands | PASS | sync/check exit 0; no-op sync preserves bytes and nanosecond mtimes for manifest/header/CMake/component/Doxygen |
| Registered generator suite | PASS | 17 isolated unittest cases; exact exits 0/1/2, ASCII SemVer/suffix/uint32 bounds, every field drift, preflight/I/O rejection, bump/set, repeated sync, no Git, enclosing Git, dirty example, process audit events and original-checkout preservation |
| Native Debug/Release | PASS | 3/3 actual CTest suites each; five independent headers, compiled planned objects, runtime Status/values/inline version ODR and 76,452 validator cases; Release checks remain active |
| Plain tests-OFF consumer | PASS | Independent strong reviewer builds/runs the CMake consumer; no Python/Git consumer dependency |
| Native PlatformIO | PASS | Existing launcher/Core builds and executes real main.cpp/profile.cpp plus core library; prints 0.1.0-dev.1 and zero fake callbacks |
| Real normal archive | PASS | pio pkg pack produces 29 regular unique files; every export pattern matches, all public headers and the one explicit source present; no vendor/reference/audit/prompt/test/tool/script/build payload |
| Extracted clean consumer | PASS | Renamed actual archive under unrelated Git, no own .git/scripts/test, restricted child PATH without Git/Python/Doxygen/PIO; strict Release compile/link/run, tests OFF and public C++17 inherited from C++11 baseline |
| Component registration | PASS, host fixture only | Explicit register args/source/include and public C++17 checked with a host function capturing idf_component_register; actual source and consumer link/run; no SDK or firmware proof |
| Launcher | PASS | Six direct-vs-wrapper CRT argv-byte comparisons, exit 37 preserved, spaces/empty args/shell characters/quotes/trailing slash, temp profile containing spaces/metacharacters; missing executable reports stderr and returns 1 |
| Doxygen | PASS | Installed Doxyfile runs with fatal missing/enum/parameter/doc warnings and EXTRACT_ALL=NO; XML has 17 public structs, real helpers/validators and all four version constants; generated Version.h HTML exists |
| Source/document consistency | PASS | Python syntax/docstrings/100-column style, local Markdown paths/anchors, UTF-8/LF and whitespace; eight unchanged vendor hashes |
| Independent review | PASS | Fresh strong reviewer checked actual generator/test/build/package/launcher boundaries and executed 17 fixtures/ordinary consumer; no confirmed defect or unnecessary abstraction |
| Later evidence | NOT RUN | IDF manager archive tool, linked IDF/Arduino firmware, full hardened distribution matrix, complete omission guard/negative fixtures, remote CI and hardware remain later scope |

Exact current commands from the repository root:

```powershell
python scripts/generate_version.py sync
python scripts/generate_version.py check
python tools/test_version_sync.py -v
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
doxygen Doxyfile
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd --version
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x.tar.gz
```

Version fixture processes are bounded to 10 seconds, registered suite to 120;
C++ suites retain 30-second bounds. All fixture mutations are TEMP; generated
integration/review artifacts are under ignored build/ and examples/host/basic/.pio.
Version header output has no Git/time/firmware identity; an audit-hook fixture
proves the CLI never even attempts a child process. No-op fields are compared as
bytes/mtime, not only normalized text.

Actual archive validation used tarfile member inspection, requiring unique
regular relative paths, presence of every manifest export pattern/public header,
one core source and absence of forbidden directories. Files were copied only
after traversal/root containment checks into build/p8-clean-parent/renamed-library.
The prior clean-consumer command was reused with that archive source, absolute
CMake/Ninja/GCC executables, CMAKE_CXX_STANDARD=11, strict warning flags and child
PATH restricted to Windows system directories. It builds/runs the ordinary
consumer and verifies three effective C++17 compile commands/tests OFF.

The host-only component fixture in build/p8-component-fixture verifies the exact
SRCS/INCLUDE_DIRS and absence of extra args, checks that the host target was not
created, then compiles/links the actual library consumer. It cannot verify the
real SDK's dependency scanning, component archive or firmware link. Official
IDF filter semantics and peer minima are source evidence only; admission metadata
does not claim SDK/board qualification.

Launcher evidence came from an ignored ad hoc build/p8-packaging-review fixture:
a compiled temporary pio.exe prints argv bytes and returns 37, and CMD direct
invocation is compared to scripts/pio.cmd for six cases. No real Core is replaced
or installed. `python build/p8-packaging-review/review_launcher.py` passed in the
lead and independent review. This artifact is session evidence, not a promised
checked-in guard. The full guard stage remains 33.

Initial Doxygen visibility inspection assumed the wrong capitalized filename;
the actual generated XML refid resolves _version_8h.html. Corrected the check,
verified the page and every required symbol, and suppressed no warning. The
source configuration and generated comments were already correct.

MSVC/Clang and clang-format were unavailable on PATH and NOT RUN. Host ASan/UBSan
remain unavailable in this MinGW distribution; no new sanitizer pass is claimed.
Vendor notices/license terms were preserved, no release/tag/publication occurred,
and no unavailable driver/firmware behavior was stubbed. Final commit/sync and
clean-state evidence belongs in the handoff and next progress update.

## Prompt 8 independent audit, 2026-10-26

Started clean at synchronized `af0d2d44b812dfda006e5513e18b6c669451bb6d` on
`promptset/tusb32x-implementation-20261023`, zero ahead/behind after fetch. Read
the original prompt in full and inspected its actual implementation diff from
`93f2d31374dc16bcc95800d0e0c516c4372faa4a`, rather than accepting prior summaries.
Three parallel read-only reviewers independently checked requirements/docs/scope,
generator/edge cases/tests/simplicity, and package/component/launcher/consumer
boundaries. Current peer revisions/clean states and source paths were refreshed
in [execution-8-peers](audit/execution-8-peers.md). The lead verified findings
against actual code and isolated reproductions and owns all edits/Git.

Confirmed findings and corrections:

- Alternate-form duplicate declarations escaped the strict replacement regex.
  A copied fixture returned check exit 0, then CMake actually reported conflicting
  TUSB32X_VERSION=9.9.9. The generator now counts declarations independently of
  canonical replacement and rejects ambiguity with exit 2 before writes. Existing
  version fixtures now include quoted/unquoted, trailing-comment, mixed-case and
  multiline duplicate declarations, preserving fixture bytes/mtimes on rejection.
- An inherited ERRORLEVEL variable shadowed CMD's actual child status: a fake
  executable returning 37 instead returned inherited 0 or 91. Clearing the shadow
  inside setlocal fixes this without changing argument forwarding or caller state.
  A new Windows-only registered suite uses temporary CMD copies as pio.exe, needs
  no Core/compiler and checks 16 status/shadow combinations plus missing install.
  Independent negative control against the original launcher failed ten cases.
- The actual archive contained 25 relative guide links to excluded repository
  evidence. Converted them to source-host URLs; retained relative shipped-guide
  links and the same reviewed package set. Removed the stale Unreleased statement
  that version tooling was still planned.

Executed tool versions are unchanged and freshly queried: CMake/CTest 4.0.1,
Ninja 1.12.1, GCC 15.1.0 MinGW-W64 UCRT MCF SEH, Python 3.12.10, Doxygen 1.13.2,
managed PlatformIO Core 6.1.19 and pinned native platform 1.2.1.

| Check | Result | Evidence |
| --- | --- | --- |
| Version sync/check/no-op | PASS | Exit 0; manifest/generated bytes and nanosecond mtimes preserved on repeated sync |
| Generator fixtures | PASS | 18 cases directly, in independent TEMP snapshot and through CTest; original suite also passed before fixes |
| Windows launcher fixtures | PASS | Two registered cases, 16 child-status/shadow subcases and missing-install failure; original-launcher negative control detects bug |
| Strict native Debug/Release | PASS | 4/4 actual CTests each; independent headers, constexpr/ODR values and existing 76,452 pure validator cases preserved |
| Native PlatformIO | PASS | Clean then real compile/link/run; current version and zero fake callback attempts |
| Launcher argument forwarding | PASS | Six direct-vs-wrapper CRT byte/status comparisons, including spaces, empty argument, shell characters, quotes and trailing slash |
| Actual archive | PASS | 29 unique regular source-equal files; every export pattern represented, excluded payload absent; IDF include set matches; remaining relative Markdown links resolve |
| Extracted ordinary consumer | PASS | Renamed real archive under unrelated Git, tests OFF, no maintainer tools on child PATH; strict Release compile/link/run and three C++17 TUs from C++11 baseline |
| Host component registration | PASS, host only | Fresh Release fixture validates explicit registration/public requirements and links/runs actual core/consumer |
| Doxygen | PASS | Fatal warnings enabled; 17 public structs, validators/errorName/four version constants visible in XML and Version.h HTML present |
| Source/docs review | PASS | Full scoped diff/whitespace, local paths/anchors and own source-host links, Python syntax/docstrings/style, UTF-8/LF and eight archived hashes |

Lead commands from repository root:

```powershell
python scripts/generate_version.py sync
python scripts/generate_version.py check
python tools/test_version_sync.py -v
python tools/test_pio_launcher.py -v
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
doxygen Doxyfile
python build/p8-packaging-review/review_launcher.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd run -d examples/host/basic -e native -t clean
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x-reaudit.tar.gz
python build/p8-reaudit-package.py
python build/p8-reaudit-docs.py
cmake -S build/p8-component-fixture -B build/p8-component-fixture/reaudit -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/p8-component-fixture/reaudit
& build/p8-component-fixture/reaudit/registration_consumer.exe
```

The ignored session-only package script validates actual archive paths/member
types/source bytes and guide links, then creates a fresh unrelated Git parent
and renamed source extraction. It invokes absolute CMake/Ninja/GCC paths with
child PATH restricted to Windows system directories, CMAKE_CXX_STANDARD=11,
CMAKE_CXX_EXTENSIONS=OFF, strict warnings and Release. It confirms tests default
OFF and all three compile commands require C++17. Ignored review artifacts are
session evidence, not promised checked-in package/omission guards.

No public C++ behavior/version/dependency floor/source list changed. Real IDF
manager archives, linked Arduino/native-IDF firmware, full distribution matrix,
complete omission guard/negative fixtures and hardware remain NOT RUN/later work.
MSVC/Clang/clang-format remain unavailable on PATH; this MinGW lacks the previously
probed sanitizer libraries, so no new sanitizer pass is claimed. No sibling was
edited and no remote CI was dispatched or polled. Final commit, upstream equality
and clean task state are reported in the handoff and next progress update.

## Prompt 9: private transport and exact identity, 2026-10-26

Started clean and synchronized at `d7fbd59a3604d1af70dc9c751b44fa55742fb878` on
`promptset/tusb32x-implementation-20261023`; fetch showed equal upstream, zero
ahead/behind. Read the full original prompt and applicable contracts. Current
peer source, examples, runner/test expectations, AGENTS and workflow definitions
were re-audited; full revisions/clean states/adopted-rejected choices and exact
TI/pinned-public sources are in [execution-9-peers](audit/execution-9-peers.md).
No sibling was edited/built and no remote CI was dispatched, polled or queried.

Tools freshly queried: CMake/CTest 4.0.1, Ninja 1.12.1, GCC 15.1.0 MinGW-W64
UCRT MCF SEH, Python 3.12.10 and Doxygen 1.13.2. Native PlatformIO uses existing
managed Core 6.1.19 and pinned platformio/native 1.2.1, with no second Core.

| Check | Result | Evidence and boundary |
| --- | --- | --- |
| TI source and pinned counterexamples | PASS | Current B/L/A/V PDF downloads match archived SHA-256; all eight archives intact; address/framing/ID/revision/map pages read, four identity tables visually inspected; pinned Go/Rust/Renode hashes/source bodies verified |
| Registered protocol | PASS | 161,461 active checks, fixed-size scripted expectations, all variants/address bytes/register offsets/Err representations/revisions, ranges/wrap/length/budget boundaries and exact pointer framing |
| Failure/effect/accounting | PASS | Partial RX never published, all legal native detail/text retained, owner/invalid statuses normalized, identical error with/without applied write remains POSSIBLE, saturating independent totals/streaks, retained failures, raw/managed/time/lifecycle separation and independent instances |
| Identity transport and pure checks | PASS | Exact eight-byte ascending signatures including NUL, separate 322I identity, wrong/reversed/partial/null/oversize inputs, exact revision policy including 06 mismatch, raw byte retained after successful callback, no base A0 |
| Strict native Debug/Release | PASS | 5/5 genuine CTests each; public headers independently compile, planned-only fixtures remain build checks, 76,452 validator cases and existing values/version/launcher suites preserved |
| Independent and direct builds | PASS | Strong reviewer rebuilt registered Release protocol: 161,461 checks/1 CTest; author direct GNU strict C++17 Debug and optimized -O2 -DNDEBUG builds pass with checks active |
| Version and docs | PASS | sync/check/no-op preserve stable 0.1.0-dev.1; strict Doxyfile without suppressed warnings, 17 public structs/real helpers/four version constants and four private protocol functions visible |
| Explicit sources/package | PASS | Two real CPP units shared host/component/PIO; real 32-file archive includes private protocol header and transport guide, no vendor/audit/test/tool/build payload; IDF include set matches |
| Tests-OFF consumer | PASS | Renamed archive beneath unrelated Git, no Git/Python/Doxygen/PIO on child PATH, strict Release compile/link/run, C++17 propagated from C++11 baseline; four compiled TUs |
| Native PlatformIO | PASS | Clean real native host build compiles both core units, links/runs existing consumer with version and zero fake attempts |
| Component registration | PASS, host fixture only | Captures exact two-source/public include/C++17 registration and compiles/links/runs actual source plus consumer; no SDK/firmware claim |
| Source/docs consistency | PASS | Complete scoped diff/whitespace, UTF-8/LF/final newline, local/own-source links and anchors, unchanged public callable inventory and archived terms/hashes |
| Sanitizer/toolchain limits | BLOCKED/NOT RUN | Fresh ASan/UBSan probe cannot link -lasan/-lubsan; cl, clang++ and clang-format absent from PATH |

Commands from the repository root:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug -R '^tusb32x_protocol$' --output-on-failure
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd run -d examples/host/basic -e native -t clean
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x-p9.tar.gz
python build/p9-package.py
cmake -S build/p9-component-fixture -B build/p9-component-fixture/out -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/p9-component-fixture/out
& build/p9-component-fixture/out/registration_consumer.exe
python build/p9-docs.py
'int main() { return 0; }' | & g++ -x c++ '-fsanitize=address,undefined' -o build/p9-sanitizer-probe.exe -
```

The final command is the unavailable-sanitizer probe, not a passed core check.
Package/session fixtures live only under ignored build/. The package script
checks 32 unique regular relative source-equal archive members and all remaining
relative guide links before extraction. It creates a fresh unrelated Git parent,
renames the extracted library, configures the real ordinary consumer with tests
default OFF, CMAKE_CXX_STANDARD=11, CMAKE_CXX_EXTENSIONS=OFF and strict warnings,
then builds/runs with absolute CMake/Ninja/GCC paths and child PATH limited to
Windows system directories. No parent Git identity enters library linkage.
These session scripts are evidence, not the future hardened package guard.

Receive storage is 11 bytes, TX is 1 pointer or 2 write bytes, each helper invokes
zero/one terminal callback, pure comparisons invoke none. Status-only adapters
must detect short transfers; callbacks cannot prove elapsed time or phase/byte
acceptance. Hardware STOP/START/final NACK, SDK locks/internal waits/cleanup and
one aggregate timeout still need their own adapter/fixture qualification.

Three parallel agents supplied the peer audit, independently derived scripted
tests and strong reasoning review of actual TI/production/test sources. Lead
reviewed/integrated all changes. No public operation, raw diagnostic method,
decoder/composer, owner scheduling, reset/recovery/IRQ engine, firmware or hardware
qualification is claimed. Unresolved TUSB322I write gates remain enforced. Final
commit/upstream equality and clean-state evidence belong in the handoff and next
progress update.

## Prompt 9 independent audit, 2026-10-26

Started at clean synchronized `84dd5c269318f50b377243843bdc9306590fe517` on the
existing `promptset/tusb32x-implementation-20261023` branch, zero ahead/behind
after fetch. Reread original prompt 9 completely and reviewed actual production,
tests and diff from `d7fbd59a3604d1af70dc9c751b44fa55742fb878`. Three fresh parallel
read-only reviewers independently checked requirements/scope/simplicity,
correctness/TI/edge semantics and tests/current peers. Lead verified the actual
code/evidence/results. No confirmed gap or bug was found; no production, test,
API, package/source-list or silicon change was needed.

Fresh tools: CMake/CTest 4.0.1, Ninja 1.12.1, GCC 15.1.0 MinGW-W64 UCRT MCF SEH,
Python 3.12.10, Doxygen 1.13.2 and existing managed PlatformIO Core 6.1.19 with
native 1.2.1. New strict builds used build/p9-independent-audit; independent
reviewer builds used build/p9-reaudit-requirements, -correctness and -tests.

| Check | Result | Evidence |
| --- | --- | --- |
| Fresh strict Debug/Release | PASS | Clean configure and compilation of both real core units, all public-header/planned fixtures, 5/5 genuine CTests each |
| Registered protocol | PASS | 161,461 active checks; independent reviewer native builds and direct optimized NDEBUG execution also pass |
| Supplemental edge checks | PASS | 542 ignored-only checks: maximum 11-byte partial reads at every fill boundary, raw/managed origins, output sentinels, extreme budgets/details and additional fake rejection guards |
| Intentional defect injection | PASS | Existing suite rejects all seven isolated mutants: failed-read publication, illegal status passthrough, raw/managed leakage, false no-effect write evidence, wrapping counters, early timeout narrowing and bypassed 322I write gate |
| TI and pinned source bytes | PASS | Current B/L/A/V downloads match archive; all eight archived hashes intact; exact pinned Go/Rust/Renode bytes verified and relevant bodies reread; four identity tables visually inspected |
| Version/Doxygen | PASS | check and byte/mtime no-op; strict Doxyfile, 17 public structs, real public helpers/four version values and private protocol functions visible |
| Native PlatformIO | PASS | Clean native compile/link/run with both actual core CPP units; version and zero fake attempts printed |
| Actual package/consumer | PASS | 32 unique regular source-equal files; IDF selection matches, relative shipped links resolve, excluded content absent; renamed extraction under unrelated Git compiles/runs strict Release tests OFF with no maintainer tools on child PATH and C++17 propagated from C++11 |
| Component registration | PASS, host only | Fresh two-source registration fixture compiles/links/runs actual library and consumer, without an SDK or firmware claim |
| Documentation/source consistency | PASS | Original/full scoped diff whitespace, local paths/anchors/own-source links, UTF-8/LF/final newline and unchanged vendor terms/hashes |

Lead commands:

```powershell
cmake -S . -B build/p9-independent-audit -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p9-independent-audit --config Debug
ctest --test-dir build/p9-independent-audit -C Debug --output-on-failure
cmake --build build/p9-independent-audit --config Release
ctest --test-dir build/p9-independent-audit -C Release --output-on-failure
python scripts/generate_version.py check
doxygen Doxyfile
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd run -d examples/host/basic -e native -t clean
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x-p9.tar.gz
python build/p9-package.py
cmake -S build/p9-component-fixture -B build/p9-component-fixture/independent-audit -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/p9-component-fixture/independent-audit
& build/p9-component-fixture/independent-audit/registration_consumer.exe
& build/p9-reaudit-tests/protocol-direct-ndebug.exe
& build/p9-reaudit-tests/supplemental.exe
python build/p9-docs.py
'int main() { return 0; }' | & g++ -x c++ '-fsanitize=address,undefined' -o build/p9-independent-sanitizer.exe -
```

Lead inspected the supplemental source and each mutant diff/failure log, then
ran the direct/supplemental executables. Mutant copies/expected failing logs are
isolated under build/p9-reaudit-tests, never compiled into the library or shipped.
These session artifacts are review evidence, not new maintained guards or API.
The package script validates actual archive selection and creates a fresh renamed
tests-OFF extraction beneath an unrelated Git parent with child PATH restricted
to Windows system directories. It proves portable linkage only.

The final sanitizer probe fails because this MinGW lacks -lasan/-lubsan; ASan/
UBSan remain BLOCKED. MSVC/Clang/clang-format are absent from PATH. Hardware
framing/aggregate SDK timing, real IDF archive tooling, framework firmware and
later package/omission qualification remain NOT RUN. No sibling was edited or
remote CI dispatched/polled. Audit records and Unreleased are the only changes;
final commit, clean state and upstream equality are in the handoff/next progress
update without a self-referencing commit-hash update.

## Prompt 10 codecs and safe composition, 2026-10-26

Started clean at `dd82a796e51dcfefa77db36a921d69aec0b12346` on
`promptset/tusb32x-implementation-20261023`; initial fetch showed equal upstream
and zero ahead/behind. Fresh peer audit and exact TI/public source verification
are recorded in [progress](implementation-progress.md#prompt-10-execution-record)
and [execution-10-peers](audit/execution-10-peers.md). No copied vendor/reference
implementation, sibling dependency, framework header or owner executor was added.

Actual tools: CMake/CTest 4.0.1, Ninja 1.12.1, GCC 15.1.0 MinGW-W64 UCRT MCF SEH,
Python 3.12.10, Doxygen 1.13.2, existing managed PlatformIO Core 6.1.19 and
platformio/native 1.2.1. Native CMake uses the existing Ninja Multi-Config directory;
the isolated consumer/component builds use Ninja Release.

| Check | Result | Evidence and boundary |
| --- | --- | --- |
| Strict host Debug/Release | PASS | 5/5 registered CTests per configuration, including compile contracts, validators, protocol/codecs, 18 version fixtures and Windows launcher fixtures; assertions/checks active |
| Pure codec/composer suite | PASS | 8,602,548 independent checks: all seven variants/every mapped byte, typed domains/invalid casts, reserved and absent fields, full output preservation, semantic comparison, requested fields, command isolation and stage/capability rejection |
| Protocol boundary | PASS | 161,670 checks, including manual composed [09,15]/[09,14]/[09,02] and reset frames; pure calls leave counting callbacks at zero; these are not executed owner jobs |
| Critical vectors | PASS | Advertisement/detection90 separated, charge-through20, base0C debug/0E reserved, Try.SRC06, ACK06->16, explicit reset08, DE->56 arithmetic with reset-pending write rejection, settledD6->56, staged01/31/30 composition, 45 qualification and literal04/00, 322I DRP equivalence/gates |
| Independent review | PASS | General reviewer separate strict Release build 5/5, public CommandTable-only consumer without src include; coding agent strict optimized NDEBUG run; strong reviewer actual-source/test/TI review and focused Release rerun |
| Package/ordinary consumer | PASS | Real 34-file source-equal archive; IDF include selection matches, exclusions and shipped relative links checked. Fresh renamed tests-OFF strict Release consumer under unrelated Git, no Git/Python/Doxygen/PIO on child PATH, five TUs and C++17 propagated from C++11 |
| Native PlatformIO | PASS | Clean real compile/link/run of all three explicit core CPP units and public decoder consumer, zero fake attempts |
| Component registration | PASS, host only | Exact same three sources and public include/C++17 requirements captured, compiled, linked and executed; no native-IDF firmware claim |
| Version/docs | PASS | sync/check and byte/mtime-stable repeated sync, strict Doxygen, 17 public structs and real public decoder/validators visible; Markdown links/anchors, UTF-8/LF/whitespace and all eight archived vendor hashes |
| Sanitizers | BLOCKED | Fresh minimal ASan/UBSan link probe fails: installed MinGW lacks -lasan/-lubsan |
| Other toolchains/hardware | NOT RUN | MSVC/Clang/clang-format absent on PATH; owner procedures/timing, complete port decoding, SDK adapters, firmware and physical qualification are later stages |

Lead commands (all applicable commands returned success except the documented
sanitizer probe):

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Release -R tusb32x_protocol --output-on-failure
& build/native/test/Release/tusb32x_protocol.exe
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
python build/p10-doxygen.py
python build/p10-evidence.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd run -d examples/host/basic -e native -t clean
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x-p10.tar.gz
python build/p10-package.py
cmake -S build/p10-component-fixture -B build/p10-component-fixture/native -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/p10-component-fixture/native
& build/p10-component-fixture/native/registration_consumer.exe
& build/p10-peer-review/public_consumer.exe
python build/p10-docs.py
'int main() { return 0; }' | & g++ -x c++ '-fsanitize=address,undefined' -o build/p10-sanitizer-probe.exe -
git diff --check
```

The ignored build/p10-* scripts/fixtures are session evidence, not new maintained
guards or production dependencies. They inspect actual source/archive/XML and
create fresh temporary consumer trees; the package's child environment excludes
maintainer tools. Current official B/L/A/V downloads match the archived exact
revisions and hashes; pinned Linux driver/regmap and Betrusted Rust bytes match
their provenance and the relevant helper bodies were reread. No silicon access
conflict was resolved by tests, and pure byte/stage composition proves no elapsed
wait, managed operation, fault clearing or hardware effects. No remote CI was
dispatched/polled and no sibling was changed. Lead reviews/stages/commits/syncs
this checked unit on the same branch; final equality and clean state are reported
in the handoff, with the commit recorded in the next progress update.

## Prompt 10 independent audit, 2026-10-26

Started clean at synchronized `80995089454db7bd4cc4f28e1c66c62a98469266` on
`promptset/tusb32x-implementation-20261023`, zero ahead/behind after fetch.
Reread the original prompt in full from `dd82a79`, then inspected its actual
implementation diff and current source/tests/contracts. Three fresh parallel
reviewers independently checked coverage/scope/simplicity, silicon/correctness
and tests/current peers. Lead verified code, source evidence and actual checks.
No confirmed gap or bug required production, test, API or build/package changes.

Pre-existing contracts at the original diff base already distinguish DE->56
field arithmetic from reset-pending write qualification, require ordinary45
internal03=00, and require single-byte raw direction codes rather than complete
port applicability. Their interpretation was independently checked against the
archived TI tables; no acceptance requirement or capability gate was weakened.

Fresh tools: CMake/CTest 4.0.1, Ninja 1.12.1, GCC 15.1.0 MinGW-W64 UCRT MCF SEH,
Python 3.12.10, Doxygen 1.13.2, existing managed PlatformIO Core 6.1.19 and native
1.2.1. Lead used a fresh Ninja Multi-Config tree at build/p10-independent-audit;
reviewers used separate build/p10-reaudit-requirements, -correctness and -tests
directories. These isolated review artifacts are not shipped or maintained guards.

| Check | Result | Evidence |
| --- | --- | --- |
| Fresh strict Debug/Release | PASS | 5/5 registered CTests in each configuration, all real core sources and five independent public-header TUs; Release checks active |
| Codec/protocol | PASS | 8,602,548 codec checks and 161,670 protocol checks, no failures; literal oracle independence, complete output sentinels, field/intent/stage rejection reviewed; strong reviewer also compiled/executed strict optimized NDEBUG directly |
| Ordinary public linkage | PASS | Independent fresh tests-OFF consumers and actual extracted package compile/link/run, supplied08=90 interpreted correctly, zero fake attempts |
| Actual package | PASS | 34 unique source-equal files, explicit CMake/PIO/IDF core sources and matching include selection; vendor/audit/test/tool/build contents excluded; shipped relative guide links resolve |
| Maintainer-tool independence | PASS | Fresh renamed archive extraction under unrelated Git, tests OFF, Git/Python/Doxygen/PIO absent from child PATH; C++17 propagated from a C++11 consumer request |
| Native PlatformIO/component | PASS | Clean native build/run of all three actual core CPP units; separate fresh three-source host component fixture links/runs without a firmware claim |
| Source evidence | PASS | Current B/L/A/V downloads match archive hashes; exact pinned Linux/regmap/Rust bytes and relevant helper bodies reread; all eight archived PDFs intact |
| Version/docs | PASS | version check, repeated byte/mtime no-op sync, strict Doxygen and visible real decoder/17 public structs, local links/anchors, UTF-8/LF and diff whitespace |
| Sanitizers | BLOCKED | Fresh minimal probe exits1: installed MinGW cannot link -lasan/-lubsan; no sanitizer pass claimed |
| Other tools/later stages | NOT RUN | MSVC/Clang/clang-format absent; owner sequencing/timing, complete port interpretation, SDK adapters, linked firmware and physical hardware remain later work |

Lead commands:

```powershell
cmake -S . -B build/p10-independent-audit -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p10-independent-audit --config Debug
ctest --test-dir build/p10-independent-audit -C Debug --output-on-failure
cmake --build build/p10-independent-audit --config Release
ctest --test-dir build/p10-independent-audit -C Release --output-on-failure
& build/p10-independent-audit/test/Release/tusb32x_protocol.exe
python scripts/generate_version.py check
doxygen Doxyfile
python build/p10-doxygen.py
python build/p10-evidence.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd run -d examples/host/basic -e native -t clean
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x-p10.tar.gz
python build/p10-package.py
cmake -S build/p10-component-fixture -B build/p10-component-fixture/reaudit -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/p10-component-fixture/reaudit
& build/p10-component-fixture/reaudit/registration_consumer.exe
python build/p10-docs.py
'int main() { return 0; }' | & g++ -x c++ '-fsanitize=address,undefined' -o build/p10-reaudit-sanitizer.exe -
git diff --check
```

The sanitizer command is the expected unavailable-tool probe; every applicable
positive check passed. No sibling was changed, remote CI dispatched/polled, or
hardware behavior inferred from the fake. Audit evidence and Unreleased are the
only tracked changes. Lead reviews/commits/synchronizes these on the existing
execution branch and reports final equality/clean state in the handoff, recording
the completed commit in the next progress update without a hash-edit loop.


## Prompt 11 owner engine (2026-10-26)

Started at synchronized `fe1f7caf732b723734d1b87ee50252b09d6928fe` on the existing
execution branch. The lead inspected the actual code and all agent diffs. Current
peer evidence is [execution-11-peers](audit/execution-11-peers.md). A separate
strong reviewer checked time, tokens and retained evidence. Its two confirmed
findings were fixed and covered by runtime regressions: termination must remain
independent of unrelated reserved settings, and matching identity evidence must
wait for fresh time before publication. Complete adverse evidence remains immediate.

Tools: GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1, Python 3.12.10,
Doxygen 1.13.2; existing managed PlatformIO Core 6.1.19/native 1.2.1. Strict GNU
warnings and Release-active checks remain enabled. No framework dependency,
allocation, clock, retry, sleep or write is added by initialization.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict native Debug and Release | PASS | Final lead runs: 6/6 CTests each, including compile contracts, validators, protocol/codecs, operations, 18 version fixtures and Windows launcher fixtures |
| Owner behavior and public linkage | PASS | 20,138 checks per run; all 15 new methods called, all seven variants, exact no-write F plans, rejected outputs, result/token precedence, atomic binding, failure/reinit/end/invalidate at every read boundary, null-context callback changes and two independent devices |
| Owner time | PASS | Budgets 0/1/255, clipped timeouts, pending completion without fresh poll, exact deadline/late success, immediate original backend failure/detail, equal/backward/half-range and wrapping time, bounded token exhaustion helper |
| Evidence regressions | PASS | Reserved settings with independent disable proof; matching identity/revision stays staged through cancel/end/invalidate/expiry; adverse raw signature survives deadline expiry; pending reset remains unresolved through same-target rebind/take |
| Previous protocol regressions | PASS | 161,670 protocol checks and 8,602,548 codec checks; fixed literal expectations, access gates, mixed-access composition and accounting |
| Headers and planned contracts | PASS | Five independently included real headers, compile-only future fixture/examples, no unimplemented delivered declarations or test-only seed API |
| Ordinary extracted consumer | PASS | Real 36-file source-equal archive; matching IDF allowlist, no vendor/audit/test/build content, relative guide links resolve; tests OFF in renamed tree beneath unrelated Git; child PATH excludes Python/Git/Doxygen/PIO and C++17 propagates to C++11-requesting consumer |
| PlatformIO native | PASS | Actual native source filter compiles Operations and links/runs ordinary consumer; no SDK firmware claim |
| Documentation/version | PASS | Strict Doxygen, XML exposes all 15 owner methods and excludes later jobs; existing real decoder/17 public structs remain visible; version check/no-op sync, local links/anchors, UTF-8/LF and diff whitespace |
| Sanitizers/other compilers | NOT RUN | Earlier same-day independent audit established installed MinGW lacks ASan/UBSan runtimes; MSVC/Clang/clang-format unavailable; no changed tool installation or sanitizer pass claimed |
| Later work | NOT RUN | Apply/reset/IRQ/port/raw/health operations, SDK adapter/firmware and physical hardware remain later prompts. Remote CI intentionally not dispatched/polled |

Commands run from the repository root (the p10/p11 Python files below are
session-only inspection scripts under ignored build, not delivered guards):

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
& build/native/test/Release/tusb32x_operations.exe
& build/native/test/Release/tusb32x_protocol.exe
doxygen Doxyfile
python build/p10-doxygen.py
python scripts/generate_version.py check
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd pkg pack . --output build/TUSB32x-p11.tar.gz
python build/p11-package.py
scripts\pio.cmd run -d examples/host/basic -e native -t exec
git diff --check
```

The package inspection configures/builds/runs the real extracted host consumer
with explicit available compiler/CMake/Ninja paths and strict warnings. The
lead additionally inspected generated Doxygen XML for the real facade and all
15 method names. No absent later method is represented as implemented.

An early concurrent full-suite run detected a real checkout-change guard failure
when the lead edited metadata while the version fixtures ran. The 18 fixture
cases themselves passed. After edits stopped, the final lead Debug and Release
full runs passed that guard and every other suite. Initial package link checks
also exposed repository-only references, corrected before the successful repack.
No check was suppressed, weakened or relabeled to obtain these passes.
