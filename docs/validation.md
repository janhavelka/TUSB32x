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
