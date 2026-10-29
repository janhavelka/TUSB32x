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


## Prompt 11 independent audit (2026-10-26)

Audited original prompt 11 in full, actual `fe1f7ca..2dd4304` diff and current
code, not the previous handoff. Started clean/synchronized at
`2dd43046cdfe7f2eed92d8fa372e27305cad58f4`. Three fresh reviewers and the lead
found no production defect. Fixed the bind validation return-code comment and
added all-variant reset-history reinitialization coverage. Current peer evidence
is in [execution-11-peers](audit/execution-11-peers.md).

Tools remain GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1,
Python 3.12.10 and Doxygen 1.13.2. Lead used a fresh Ninja Multi-Config tree.
Reviewers used distinct build/p11-fresh-correctness, -coverage and -tests trees.

| Check | Result | Evidence |
| --- | --- | --- |
| Final strict Debug/Release | PASS | 6/6 registered CTests each after the new regression; independent public headers/future fixtures compile; no assertion suppression |
| Owner suite | PASS | 20,490 checks in each final configuration; new clear-register reinitialization cannot discard pending reset or permit target replacement |
| Existing regressions | PASS | 161,670 protocol and 8,602,548 codec checks; validator/value/version/launcher suites pass unchanged |
| Extra public probes | PASS | 371 checks: reset obligation through clear reinit for all variants, near-half-range forward expiry and rollover; no private state mutation |
| Public/package boundary | PASS | Reviewer extracted original 36-file archive, compared every byte to audited commit and linked/called all 15 methods from public headers only; lead repacked current files and verified source equality, IDF selection and tool-independent tests-OFF consumer |
| Documentation | PASS | Strict Doxygen, all 15 delivered owner methods visible and future jobs absent; real decoder/17 public structs visible; version check/no-op, local links/anchors and whitespace |
| Other tool/hardware evidence | NOT RUN | No changed need to rerun native PlatformIO/IDF registration, firmware or hardware. Earlier same-day MinGW sanitizer probe lacks ASan/UBSan; no sanitizer pass claimed. Remote CI excluded by initial promptset policy |

Lead final commands:

```powershell
cmake -S . -B build/p11-independent-audit -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p11-independent-audit --config Debug
ctest --test-dir build/p11-independent-audit -C Debug --output-on-failure
cmake --build build/p11-independent-audit --config Release
ctest --test-dir build/p11-independent-audit -C Release --output-on-failure
& build/p11-independent-audit/test/Release/tusb32x_operations.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude -Isrc build/p11-fresh-correctness/additional.cpp build/p11-independent-audit/Release/libTUSB32x.a -o build/p11-independent-audit/additional.exe
& build/p11-independent-audit/additional.exe
doxygen Doxyfile
python build/p10-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd pkg pack . --output build/TUSB32x-p11.tar.gz
python build/p11-package.py
git diff --check
```

Files beneath build are session-only inspection scripts/probes, not new shipped
guards. The additional public probe was read before recompilation/execution;
its reset-history case is now retained in the real operations suite. The
package script builds/runs a renamed extraction beneath unrelated Git with
Git/Python/Doxygen/PIO absent from child PATH, requesting C++11 and receiving
C++17 from the library. No sibling source or tool dependency enters the build.
All required checks passed without weakened contracts; only the tested scope
and software evidence above are claimed.

## Prompt 12 profile observation (2026-10-26)

Implemented and independently reviewed the requested read-only P/F feature from
synchronized `921132461e676ea2f05f7c4e6ed88edc641a294d`. Current peer inspection
is in [execution-12-peers](audit/execution-12-peers.md). Official TI symlink PDFs
were reopened: tusb320 SLLSEN9F, tusb320li SLLSEP2D, tusb320lai SLLSEQ8D and
tusb322i SLLSEO5D agree with the archived revision identities. The archived
control tables and exact reset maxima remain the silicon evidence; the fixed
P/F plans and conservative owner-clock waits are library policy. No reference
bytes or unsupported write gates changed.

Tools: GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1, Python 3.12.10,
Doxygen 1.13.2 and the existing managed PlatformIO Core with native platform
1.2.1. Tests use strict warnings and active Release checks. Subagents used
separate build/p12-tests and build/p12-review trees; lead used build/native.

| Check | Result | Evidence |
| --- | --- | --- |
| Full strict native Debug/Release | PASS | 6/6 registered CTests each; independent public headers and compile-only future contracts build; every newly delivered method links through the real library |
| Read-only owner behavior | PASS | 34,042 new profile checks: all seven exact variants, P/F allowed-read traces, zero writes, no desired adoption, raw/typed validity, normal generation increments and preservation, no P trust promotion, full initial/invalidated reconciliation, independent termination and reserved controls |
| Interruptions/time | PASS | Zero/one/larger budgets; each read and final fresh-time boundary; first backend error/detail; cancel/end/expiry; partial receive bytes ignored; pending reset wait, rollover, deadline equality and no extra pass when reset stays asserted |
| Prior regressions | PASS | 20,490 owner checks; 161,670 protocol checks; 8,602,548 codec checks; value/validator/version/launcher suites unchanged and passing |
| Independent review regression | PASS | Public probe and retained all-variant tests disprove stale termination after a changed disable bit without full configuration trust; pre-fix tests failed, fixed tests pass; old complete raw/time/generation remains intact |
| Public/package boundary | PASS | Actual 36-file source-equal archive, matching IDF allowlist and resolvable relative guide links; extracted renamed tests-OFF consumer beneath unrelated Git, no Python/Git/Doxygen/PIO on child PATH; public C++17 requirement propagates |
| Native PlatformIO | PASS | Managed Core builds all real sources, links and runs the ordinary native consumer; no firmware claim |
| Documentation/version | PASS | Strict Doxygen; 17 public structs, real decoder/validators and all 17 delivered owner methods visible; future callable methods absent; links/anchors, UTF-8/LF/whitespace, eight vendor hashes and no-op version sync |
| Deferred public cases | NOT RUN | Desired-mismatch/recovery after apply belongs to prompt 13; actual interrupted mode/recovery-write histories belong to prompts 13-14. Generation saturation remains the reviewed unchanged bounded increment, without private-state injection or an impractical billions-operation test |
| Hardware/other tools | NOT RUN | No SDK firmware or physical board qualification. The recorded same-day MinGW probe lacks ASan/UBSan runtimes; MSVC/Clang/clang-format unavailable. Remote CI excluded by the initial promptset policy |

Commands from repository root:

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
python build/p12-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x-p12.tar.gz
python build/p12-package.py
git diff --check
```

The build/p10/p12 scripts are session-only inspection aids, not delivered guards.
The package script checks every archived byte against current source, source
selection and guide links, then builds/runs the real ordinary consumer with a
restricted child PATH. No framework or sibling include path is introduced.
The independent reviewer also compiled the owner tests directly with -O2,
-UNDEBUG and strict warnings, outside the lead's CMake tree.

An early test compile rejected aggregate assignment `fake = {}`; the fixture now
uses an explicit `Fake{}` value. A later independent probe exposed stale
termination proof; its regression failed before correction. Both were fixed
before final passing runs, without weakening checks or changing expected silicon
bytes. No host result above is physical timing or electrical evidence.

## Prompt 12 independent audit (2026-10-26)

Re-read original prompt 12 in full and reviewed actual `9211324..db66500`
changes plus current source/tests. Started clean and synchronized at
`db6650064690883a57c60ac6b349edffe495df61`. Three fresh independent reviewers
and the lead found no runtime defect, unsupported API or scope expansion.
The confirmed test gap was complete profile assembly: the pure decoder suite
covered every field encoding, but the operation suite did not directly check
every nondefault field after publication. Added 56 public P/F cases using four
literal vectors per variant/operation and independent semantic expectations.
Production code, API, register gates and metadata are unchanged.

Tool versions remain GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1,
Python 3.12.10 and Doxygen 1.13.2. The lead used a fresh Ninja Multi-Config
build/p12-independent-audit tree; reviewers used separate p12-fresh-* trees.

| Check | Result | Evidence |
| --- | --- | --- |
| Final native Debug/Release | PASS | 6/6 registered CTests each after the added cases; strict warnings, active Release checks and independent public headers |
| Profile and owner suites | PASS | 35,454 profile checks plus 20,490 owner checks per configuration; every valid field value, raw preservation, absent-field validity, independent termination and unchanged P generation/trust |
| Existing regressions | PASS | 161,670 protocol and 8,602,548 codec checks, plus value/validator/version/launcher suites |
| Independent byte probe | PASS | 6,912 scenarios: every byte value in every applicable configuration register for all seven variants, full proof/P status, reserved validity, termination/reset separation, bounded callbacks and zero writes |
| Independent full-profile probe | PASS | 43,008 complete valid P/F plans, 833,280 checks; all setting combinations, nonpersistent bits, exact semantic publication and TUSB322I DRP00 normalization; no production decoder as oracle |
| Package/public boundary | PASS | Reviewer verified the prior source-equal 36-file archive and linked/called both new methods through extracted public sources. Lead repacked final files and checked archive equality, package links and IDF selection, then built/ran tests-OFF consumer beneath unrelated Git with maintainer tools absent from child PATH |
| Doxygen and documentation | PASS | Strict generation and all 17 owner methods visible, future callable methods absent; links/anchors, UTF-8/LF, whitespace, eight vendor hashes and version/no-op sync |
| Current peers | PASS | ADS1115, LSM6DS3TR and INA3221 revisions/main/clean state and reviewed conventions unchanged; no sibling writes/fetch/build or remote CI inspection |
| Remaining evidence | NOT RUN | Desired mismatch and actual interrupted write histories belong to prompts 13-14. Firmware/hardware remain separate. Recorded same-day sanitizer probe lacks MinGW ASan/UBSan; no sanitizer pass claimed. Native PlatformIO/component registration were not repeated for this test/documentation-only change |

Lead commands from repository root:

```powershell
cmake -S . -B build/p12-independent-audit -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p12-independent-audit --config Debug
ctest --test-dir build/p12-independent-audit -C Debug --output-on-failure
cmake --build build/p12-independent-audit --config Release
ctest --test-dir build/p12-independent-audit -C Release --output-on-failure
& build/p12-independent-audit/test/Release/tusb32x_operations.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude build/p12-fresh-proof/probe.cpp build/p12-independent-audit/Release/libTUSB32x.a -o build/p12-independent-audit/proof-probe.exe
& build/p12-independent-audit/proof-probe.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude -Itest build/p12-fresh-tests/publication_probe.cpp build/p12-independent-audit/Release/libTUSB32x.a -o build/p12-independent-audit/publication-probe.exe
& build/p12-independent-audit/publication-probe.exe
doxygen Doxyfile
python build/p12-doxygen.py
python build/p10-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd pkg pack . --output build/TUSB32x-p12.tar.gz
python build/p12-package.py
git diff --check
```

Files beneath build are ignored session inspection/probe artifacts. Lead read
both probe sources before recompiling/running them; no private-state hook was
introduced. The compact permanent vectors cover the assembly gap without adding
the exhaustive exploratory loops to every test run. All required checks passed
without weakening assertions or changing silicon expectations. No hardware
qualification or future-operation test pass is inferred.

## Prompt 13 profile application (2026-10-26)

Implemented explicit apply and read-only desired verification through the same
owner engine, safe composer and F proof. Re-audited current peer source and exact
TI procedures; [execution-13-peers](audit/execution-13-peers.md) records revisions
and four fresh official downloads matching archived bytes. Lead reviewed the
production/test diff and independent probe, then ran the integrated checks below.
GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1, Python 3.12.10,
Doxygen 1.13.2 and the managed PlatformIO native platform 1.2.1 were used.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict native Debug/Release | PASS | 6/6 registered CTests each; active Release assertions, independent public headers and promoted real method linkage |
| Apply/verify suite | PASS | 87,978 checks: all seven variants, exact literal sequences, preserved controls, final disabled state, no-op, attached base current-only behavior, input/gate rejection, desired copy/generations, complete mismatch and later explicit progress |
| Error/lifetime boundaries | PASS | Every write and F read fails with six transport errors; both applied-then-error and no application; first detail/message survives, no cleanup/replay; cancel/end/invalidate/deadline and final fresh-time boundaries preserve appropriate proof/effects |
| Time/work bounds | PASS | Independent microsecond completion at 108.1/108.9 ms, fresh samples at 108/110, no early enable at 113; final disabled wait, 322 no added wait, uint64 rollover, zero/one/larger budgets and failed-attempt accounting |
| Existing owner/protocol regressions | PASS | 35,454 profile-observation, 20,490 owner, 161,670 protocol and 8,602,548 codec checks; values/validators/version/launcher also pass |
| Independent exhaustive probe | PASS | 46,592 public old/new valid 0A pairs across seven variants, independent semantic tables and literal write sequences; final proof/effect, preservation, unchanged mode, DRP equivalence/gate and exact per-variant wait |
| Review corrections | PASS | Base mode-write uncertainty retained until full reconciliation with no added base wait; active writes preserve previous terminal diagnostic effect/kind/status. Both new regressions demonstrated failure before the fix and success after |
| Documentation/version/provenance | PASS | Strict Doxygen and visibility of 17 namespace-level public structs, decoder/validators and all 19 delivered owner methods; future methods absent. Link/anchor, UTF-8/LF/whitespace, eight vendor hashes and no-op version sync |
| Package/consumer | PASS | Actual 36-file source-equal archive, matching IDF allowlist, relative guide links, renamed tests-OFF consumer beneath unrelated Git; child PATH lacks Python/Git/Doxygen/PIO; PUBLIC C++17 propagates despite consumer requesting C++11 |
| Native PlatformIO | PASS | Managed Core builds real sources and runs the ordinary host consumer; fake callback count remains zero, no firmware/hardware inference |
| Remaining limits | NOT RUN | Arduino/native-IDF firmware and physical hardware; recorded same-day MinGW ASan/UBSan runtimes unavailable, no MSVC/Clang/clang-format. Remote CI remains prompt 34. Port-freshness publication and reset/recovery history tests require their later implemented operations |

Commands from repository root:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
& build/native/test/Release/tusb32x_operations.exe
& build/native/test/Release/tusb32x_protocol.exe
g++ -std=c++17 -Wall -Wextra -Werror -Iinclude build/p13-review/profile_probe.cpp build/native/Release/libTUSB32x.a -o build/p13-review/profile_probe-final.exe
& build/p13-review/profile_probe-final.exe
doxygen Doxyfile
python build/p13-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x-p13.tar.gz
python build/p13-package.py
git diff --check
```

Ignored build scripts/probes are session aids, not newly delivered guards.
The Doxygen check extends the previous owner method list and counts public
namespace-level structs; a private nested plan record in the XML index is not
an additional public value. Initial old-script count failure was corrected
without suppressing Doxygen warnings or dropping public visibility checks.
The package check compares every exported byte with source before building the
ordinary consumer in a restricted child environment.

Early test iteration corrected literal callback timeout expectations at a tight
deadline (14/13 ms, rather than the normal 20 ms cap) and avoided a GCC aggregate
assignment ICE by resetting fixed fake segments explicitly. No production timeout
rule or test assertion was weakened. The two actual effect-recording defects
found during review were fixed and tested before the final passing matrix.
Host timing tests prove owner arithmetic and ordering, not physical I2C timing,
termination behavior, the base cable-race prerequisite or electrical compliance.

## Prompt 13 independent audit (2026-10-26)

Started clean at synchronized `7ea5583c52407e0e755dc89e185db1eab7512238` on
the same execution branch. Reread original prompt 13 and reviewed the actual
`724b766..7ea5583` implementation diff. Three fresh reviewers and the lead found
no production defect or unsupported scope expansion. The confirmed gaps were
permanent successful UFP and settings-only 0A tests. Added literal public-operation
cases using the existing fixture, without changing production code or the API.

Tool versions remain GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1,
Python 3.12.10 and Doxygen 1.13.2. Lead used a fresh Ninja Multi-Config tree;
reviewers used separate ignored probe/build directories.

| Check | Result | Evidence |
| --- | --- | --- |
| Final Debug/Release | PASS | 6/6 CTests each after the added cases; strict warnings, active Release checks and independent public-header contracts |
| Permanent application regressions | PASS | 89,140 apply/verify checks, including UFP literal sequences on all seven variants, base debounce-only and extended termination-only changes in both directions; exact callback totals and equal fresh ticks reject accidental waits/role sequences |
| Existing regressions | PASS | 35,454 profile-observation, 20,490 owner, 161,670 protocol and 8,602,548 codec checks; value/validator/version/launcher suites pass |
| Coverage mutation probes | PASS | Isolated UFP-byte corruption and omitted standalone termination-disable writes both survived the original 87,978-check suite; added regressions reject them with 160 and 40 failed assertions respectively. Real production passes |
| Independent state-machine probe | PASS | 46,592 valid 0A initial/target byte pairs: 44,288 completed applies and 2,304 expected 322 return-to-DRP rejections; independent register emulator, physical elapsed-time, raw-image, effect, callback-budget and bound checks |
| Independent recovery probe | PASS | 70 variant/write-boundary cases with applied/not-applied backend errors, preserved original failure, no write in reconciliation, complete actual proof and later successful explicit reapply |
| Peer/scope/package review | PASS | Current four peer revisions/main/clean states and source/test conventions unchanged; one engine/composer/transport, fixed six-write storage, unchanged public/variant gates; matching 36-file PlatformIO/IDF selection |
| Doxygen and documentation | PASS | 17 public structs and 19 delivered owner methods visible, future callable methods absent; links/anchors, UTF-8/LF/whitespace, eight archived vendor hashes and no-op version sync |
| Extracted consumer | PASS | Final source-equal 36-file archive and resolvable relative links; renamed tests-OFF ordinary consumer beneath unrelated Git builds/runs without Python/Git/Doxygen/PIO on child PATH; public C++17 propagates |
| Limits | NOT RUN | Firmware and physical hardware. Recorded same-day MinGW ASan/UBSan runtimes remain unavailable. Native PlatformIO was not rerun for this test/documentation-only audit; core/package source lists are unchanged. Remote CI remains the separate prompt-34 checkpoint |

Lead commands from repository root:

```powershell
cmake -S . -B build/p13-independent-audit -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p13-independent-audit --config Debug
ctest --test-dir build/p13-independent-audit -C Debug --output-on-failure
cmake --build build/p13-independent-audit --config Release
ctest --test-dir build/p13-independent-audit -C Release --output-on-failure
& build/p13-independent-audit/test/Release/tusb32x_operations.exe
& build/p13-independent-audit/test/Release/tusb32x_protocol.exe
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror -Iinclude build/p13-fresh-proof/probe.cpp build/p13-independent-audit/Release/libTUSB32x.a -o build/p13-fresh-proof/lead-probe.exe
& build/p13-fresh-proof/lead-probe.exe
doxygen Doxyfile
python build/p13-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd pkg pack . --output build/TUSB32x-p13.tar.gz
python build/p13-package.py
git diff --check
```

Lead inspected the new cases and independent probe before recompiling/running
them. The exhaustive probe constructs candidates with public `decodeRegister`;
its emulator and raw/time assertions are independent, but it is integration
evidence, not an independent semantic-decoder oracle. Permanent regression bytes
are literal and independent of production encoding. Mutation sources/executables
and inspection scripts remain ignored under build; no faulty implementation or
new test hook was delivered. No hardware qualification is inferred.

## Prompt 14: reset and original-device recovery (2026-10-26)

Started at synchronized `a6bcccfe3353461fcc8ee838fcdb0f946a81e821` on the same
execution branch. Tools: GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1,
Python 3.12.10, Doxygen 1.13.2; native PlatformIO platform 1.2.1.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict native Debug/Release | PASS | Six CTests each; independent public headers and compile contracts; Release checks active |
| Reset/recovery public operations | PASS | 86,298 assertions: all seven reset variants, both recovery variants, local rejection, literal commands, callback/byte bounds, every callback fault and applied/not-applied write, cancel/end/deadline/external invalidation at every phase, desired/history preservation |
| Timing | PASS | Independent microsecond callback completion, floor timestamps, fresh anchors, 31/96 elapsed ticks, no fresh sample, stalled time, zero/larger budgets and uint64 rollover; no physical timing claim |
| Prior regressions | PASS | 89,140 apply/verify; 35,454 profile observation; 20,490 owner; 161,670 protocol; 8,602,548 codec checks, plus values/validators/version/launcher |
| Independent literal probe | PASS | 1,792 reset pre-read byte/variant combinations and 512 final 322I control combinations; no production encoder oracle; one-attempt/bounds/postcondition checks |
| Doxygen | PASS | No warnings; 17 public structs and all 21 implemented owner methods visible with briefs; future methods absent |
| Native PlatformIO | PASS | Host consumer builds and runs; fake callback attempts remain zero |
| Package/ordinary consumer | PASS | 36 byte-equal exported files, matching IDF selection, package links; renamed tests-OFF consumer under unrelated Git builds/runs without Git/Python/Doxygen/PIO on child PATH; C++17 propagates |
| Documentation/provenance/version | PASS | Local links/anchors, UTF-8/LF/whitespace, eight vendor hashes, unchanged generated version; four current TI PDFs separately byte-match archived source |
| Separate evidence limits | NOT RUN | Arduino/IDF firmware, physical hardware, electrical/VCONN and board readiness; same-day installed MinGW ASan/UBSan runtimes remain unavailable; MSVC/Clang/clang-format unavailable. Remote CI reserved for prompt34 |

Commands from repository root:

```powershell
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
& build/native/test/Release/tusb32x_operations.exe
& build/native/test/Release/tusb32x_protocol.exe
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude build/p14-review/reset_probe.cpp build/native/Release/libTUSB32x.a -o build/p14-review/lead-probe.exe
& build/p14-review/lead-probe.exe
doxygen Doxyfile
python build/p14-doxygen.py
python build/p10-docs.py
scripts\pio.cmd run -d examples/host/basic -e native -t exec
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd pkg pack . --output build/TUSB32x-p14.tar.gz
python build/p14-package.py
git diff --check
```

The separate test agent also configured and passed all six CTests in a fresh
`build/p14-tests` strict Debug tree. Probe and inspection scripts under build are
session aids, not delivered guards. Lead inspected and reran the independent
probe against the final library.

During development, the timing fixture caught a missing WAITING flag immediately
after a successful command callback; readiness remains invalid until a fresh
owner sample. A reserved preference fixture incorrectly used valid Try.SRC36;
corrected to reserved34 from the register guide. Independent review caught
322I non-DRP evidence hidden by an unrelated reserved08: mode validity now guards
reset completion independently. Asserted reset/incomplete interpretation returns
UNKNOWN; complete non-DRP proof returns mismatch. Permanent regressions cover both.
Final passing results supersede these initial failures. No hardware-success stub,
retry, automatic cleanup or public test-only state hook was introduced.

## Prompt 14 independent audit (2026-10-26)

Fresh audit started at synchronized `a33af83acdb5a92cc6e95bef4e0cbc747232cf07`
on the same execution branch. Reviewed full original prompt and actual
`a6bcccf..a33af83` implementation with three independent reviewers. No production
behavior defect was found. Added literal public-operation coverage and corrected
stale class Doxygen/progress status without altering runtime code or contracts.

Tool versions remain GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1,
Python 3.12.10 and Doxygen 1.13.2. Lead configured a fresh Ninja Multi-Config tree.

| Check | Result | Evidence |
| --- | --- | --- |
| Fresh strict Debug/Release | PASS | 6/6 CTests each, active Release checks, independent header/contract compilation |
| Reset/recovery regressions | PASS | 88,779 assertions; fresh actual controls versus cached proof on all seven variants, immediate identity/revision contradiction handling and literal clipped timeout arguments added |
| Previous regressions | PASS | 89,140 apply/verify, 35,454 observation, 20,490 owner, 161,670 protocol and 8,602,548 codec checks; values/validators/version/launcher pass |
| Mutation evidence | PASS | Faulty reset debounce from cached rawConfig0A passed the original 86,298 checks; added cases reject it on all seven variants with 21 failures. Real production passes |
| Independent extra probe | PASS | 1,825 checks for fresh controls, identity/revision contradictions and zero-budget pending transitions; uses existing scripted fixture and literal bytes, no private-state injection |
| Fresh source review | PASS | Same three peer HEADs/main/clean states; current instructions/code/tests/examples/runners/registration rechecked; archived B/L/A/V reset/recovery pages agree |
| Documentation/package | PASS | Strict Doxygen with 17 public structs and 21 owner methods, future methods absent; links/anchors/UTF-8/LF/vendor hashes/version; 36 source-equal exported files and renamed tests-OFF consumer beneath unrelated Git without maintainer tools |
| Separate evidence | NOT RUN | Firmware/hardware and unavailable installed MinGW sanitizer runtimes; MSVC/Clang unavailable. Native PlatformIO not repeated for test/comment/docs-only changes; extracted source consumer rechecked. Remote CI remains prompt34 |

Lead commands:

```powershell
cmake -S . -B build/p14-independent-audit -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p14-independent-audit --config Debug
ctest --test-dir build/p14-independent-audit -C Debug --output-on-failure
cmake --build build/p14-independent-audit --config Release
ctest --test-dir build/p14-independent-audit -C Release --output-on-failure
& build/p14-independent-audit/test/Release/tusb32x_operations.exe
& build/p14-independent-audit/test/Release/tusb32x_protocol.exe
& build/p14-fresh-tests/stale-reset.exe original-suite
& build/p14-fresh-tests/stale-reset.exe
& build/p14-fresh-tests/stale-reset-regressions.exe original-suite
& build/p14-fresh-tests/probe.exe
doxygen Doxyfile
python build/p14-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts\pio.cmd pkg pack . --output build/TUSB32x-p14.tar.gz
python build/p14-package.py
git diff --check
```

The two mutant failure runs intentionally return 21; the old-suite mutant and
real independent probe return zero. Lead read the single-line mutation and
probe source before rerunning them. The mutation/executables are isolated ignored
build artifacts, never delivered production. Added checked-in tests reuse the
fixed fixture and are registered in the existing operations suite; no new runner
or state-machine abstraction. Reviewers separately built fresh Debug and isolated
Doxygen trees. Their checks support the audit; final results above are lead-run.

## Prompt 15 complete port observations (2026-10-26)

Started at synchronized `caba75b21bab02e5bdc7bff47c611f797b2ef3dc` on
`promptset/tusb32x-implementation-20261023`. Read full prompt 15, COMMON, owner/
port/time baseline and actual existing source/tests. Three parallel agents owned
focused tests, current peer/documentation work and independent TI/code review.
Lead owns engine/codecs, integrates reviewed changes and handles Git.

Exact printed TI pages were reopened: B SLLSEN9F Table7-1/section7.3.1.2 p11,
Table7-7 pp17-18; L SLLSEP2D and A SLLSEQ8D Tables8-9 pp17-18 and accessory
sections7.2.3.1-2 pp11-12; V SLLSEO5D Tables6-8/9 pp18-19, sections6.3.4.2 p12
and6.3.8 p13. The [matrix](register-map.md#complete-port-interpretation-implemented-prompt-15)
separates TI field codes from conservative applicability policy. Eight archived
vendor hash checks passed. Current BME280/LDC1614/INA228 revisions, declarations,
source/tests/examples/CLI/runners/workflow definitions and instructions are in
[execution-15-peers](audit/execution-15-peers.md); siblings remained read-only.

Tools: GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1, Python 3.12.10,
Doxygen 1.13.2 and the installed VS Code PlatformIO Core/native1.2.1 environment.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict Debug/Release | PASS | Seven CTests each; Release assertions active; independent public headers and future compile-only contracts build |
| Port suite | PASS | 494,257 checks: seven variants, every08 code/context, all final09 bytes, changed bookends, literal CC/VBUS fixture, raw/typed validity, complete-only publication, acquisition/provenance, saturation, every read fault/interruption/final-time boundary and public invalidation/write/reset/recovery paths |
| Independent public probe | PASS | 13,031,431 checks / 458,752 observations, all65,536 first/final09 pairs on each variant; exact3 callbacks, budget0/255, final09 evidence, E0 coherence, original acquisition time, no writes |
| Previous regressions | PASS | 35,454 profile observation; 89,140 apply/verify; 88,779 reset/recovery; 20,490 owner; 161,670 protocol; 8,602,548 codec checks, plus values/validators/version/launcher |
| Doxygen and documents | PASS | No warnings; 17 public structs and22 implemented owner methods visible with briefs; planned methods absent; links/anchors, UTF-8/LF/whitespace, vendor hashes and no-op version sync |
| Native PlatformIO | PASS | Native consumer compiles/links/runs; fake callbacks invoked zero times |
| Package consumption | PASS | 36 source-equal exported files, matching IDF selection, relative links; extracted renamed tests-OFF C++17 consumer beneath unrelated Git builds/runs with no Python/Git/Doxygen/PlatformIO on child PATH |
| Separate evidence | NOT RUN | Physical attachment/orientation/current tracking/VCONN/IRQ and framework firmware; installed MinGW sanitizer runtimes remain unavailable from same-day checks. MSVC/Clang/clang-format are absent |

Lead commands (existing native Ninja Multi-Config tree):

```powershell
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
& build/native/test/Debug/tusb32x_port.exe
& build/native/test/Release/tusb32x_operations.exe
& build/native/test/Release/tusb32x_protocol.exe
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude build/p15-review/probe.cpp build/native/Release/libTUSB32x.a -o build/p15-review/lead-probe.exe
& build/p15-review/lead-probe.exe
doxygen Doxyfile
python build/p15-doxygen.py
python build/p10-docs.py
scripts/pio.cmd run -d examples/host/basic -e native -t exec
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-p15.tar.gz
python build/p15-package.py
git diff --check
```

Session-only probe and documentation/archive scripts under ignored `build/` were
read and run by the lead; permanent tests are registered in `test/CMakeLists.txt`.
Review found one strict integer-promotion warning and two incorrect fixture
assumptions: token999 can be issued, and terminal polling preserves the previous
turn counter. Removed a redundant flag OR and corrected those fixture assumptions
without changing token/result behavior. Initial failures are superseded by the
passing checks above. Final source review found no remaining behavior defect.
Corrected a source citation's character encoding and qualified V charge-through
debug exclusion to the documented DRP/UFP modes. No physical-support claim,
automatic IRQ/reset behavior, future method stub or framework dependency was added.

## Prompt 15 independent audit (2026-10-26)

Fresh audit started at synchronized `c9b43d23f88af70139947d9399e92edcba283d51`
on the same execution branch. Re-read full prompt 15, baseline port/time/lifetime
contracts, actual implementation diff and source/tests rather than prior summaries.
Three independent reviewers checked production semantics, requirements/TI/current
peers and regression coverage. No production defect was confirmed. Test/docs
corrections remain within the original port-observation scope.

| Check | Result | Evidence |
| --- | --- | --- |
| Fresh strict Debug/Release | PASS | Seven CTests each; active Release checks and standalone public-header/contract compilation |
| Port regressions | PASS | 495,667 checks including final09-only persistent-control contradiction, immediate trust revocation, complete publication versus cancellation/expiry and rejected backward-time preservation |
| Mutation evidence | PASS | Skipping final09 observeControl passes all 494,257 original port checks; corrected suite rejects the mutant with 63 failed assertions; unchanged real engine passes |
| Extra independent probe | PASS | 1,326 checks across all seven variants for final09 contradiction, backward-time rejection and success/cancel/deadline outcomes |
| Previous suites | PASS | 35,454 profile observation, 89,140 apply/verify, 88,779 reset/recovery, 20,490 owner, 161,670 protocol and 8,602,548 codec checks; values/validators/version/launcher pass |
| Source evidence | PASS | Archived B/L/A/V status/applicability pages reread, all eight TI hashes checked; peer main/clean states and revisions match execution-15 report; relevant instructions/source/tests/examples/runners/workflow definitions reopened |
| Documentation/package | PASS | Strict Doxygen: 17 public structs and22 implemented owner methods, future methods absent; local links/anchors, UTF-8/LF/whitespace, version; source-equal36-file package and renamed tests-OFF consumer under unrelated Git without maintainer tools |
| Separate evidence | NOT RUN | Hardware and framework firmware; installed MinGW sanitizer runtimes remain unavailable from same-day checks, MSVC/Clang absent. Native PlatformIO was not repeated for test/docs-only changes; extracted consumption was rechecked |

Tool versions remain GCC15.1 MinGW-W64, CMake/CTest4.0.1, Ninja1.12.1,
Python3.12.10 and Doxygen1.13.2. Lead commands:

```powershell
cmake -S . -B build/p15-independent-audit -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p15-independent-audit --config Debug
ctest --test-dir build/p15-independent-audit -C Debug --output-on-failure
cmake --build build/p15-independent-audit --config Release
ctest --test-dir build/p15-independent-audit -C Release --output-on-failure
& build/p15-independent-audit/test/Debug/tusb32x_port.exe
& build/p15-independent-audit/test/Release/tusb32x_operations.exe
& build/p15-independent-audit/test/Release/tusb32x_protocol.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude -Isrc build/p15-audit-engine/probe.cpp build/p15-independent-audit/Debug/libTUSB32x.a -o build/p15-audit-engine/lead-probe.exe
& build/p15-audit-engine/lead-probe.exe
& build/p15-audit-tests/skip_final_control.exe
& build/p15-audit-tests/skip_final_control_regressed.exe
doxygen Doxyfile
python build/p15-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-p15.tar.gz
python build/p15-package.py
git diff --check
```

The mutant's new-suite run intentionally exits1 with63 assertions failing; its
old-suite run exits0. The only engine mutation is `if (_phase != Phase::PORT_LAST09)`
around `observeControl`. Source is retained under ignored
`build/p15-audit-tests/operations_skip_final_control.cpp`; old/new test copies and
executables are session evidence, never delivered code. The permanent regression
uses the existing public fixture and registered port suite. The independent
probe similarly uses public operations with no private state seeding.

Documentation now separates read-only configuration proof from fresh port
publication, states delivered owner/port support, and labels the InterruptSnapshot
ACK/service table as planned prompt16. Source/API/silicon behavior is unchanged.
No hardware qualification or remote CI evidence is inferred from these host checks.

## Prompt 16 explicit interrupt service (2026-10-26)

Implemented explicit ACK-only and bounded observe/ACK/recheck in the existing
engine from synchronized `e964fe5d584100e9e98c3a20f09e5cd38b673f1b`. The lead
reviewed full prompt 16, actual production/test changes and independent findings.
Current peer source evidence is in [execution-16-peers](audit/execution-16-peers.md).
No sibling mutation or remote CI dispatch/polling occurred.

Re-read archived TI SLLSEN9F Table 7-6 p16 and Table 7-7 p18, SLLSEP2D/SLLSEQ8D
Table 9 p18, and SLLSEO5D Table 6-9 p19. B/V CSR and L/A RU-change IRQ wording
remain distinct. Explicit IRQ W1C does not resolve VCONN fault RCU clear behavior;
TUSB322I ACK/service reject before traffic. The source reviewer checked current
official PDFs (B F, L D, A D, V D unchanged) and refetched five pinned public
implementation/helper sources with matching hashes. Linux ignored ACK errors/raw
status echoes and Betrusted fixed11/retry behavior were rejected after inspecting
actual helpers. None establishes hardware or TI-authored implementation guidance.

Tools: GCC 15.1.0 MinGW-W64, CMake/CTest 4.0.1, Ninja 1.12.1, Python 3.12.10,
Doxygen 1.13.2 and the installed VS Code PlatformIO Core/native 1.2.1 environment.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict Debug/Release | PASS | Seven CTests each; active Release checks, independent public headers and future compile-only contracts; new starts compile and link through real public calls |
| IRQ suite | PASS | 67,081 checks: exact bytes for all 256 observations on six writable variants, staged02 versus actual05, desired30 versus actual50, UNKNOWN trust, every callback/interruption/fresh-time boundary, both failed-write effects, budgets, original error/detail, finite races/coalescing/stuck latch and no-pin polling |
| Port/history | PASS | 495,667 prior checks plus new complete pre-ACK retention, final-recheck-only control contradiction, acquisition/sequence preservation, diagnostic history retention and raw322 fault/gates |
| Setter isolation | PASS | Each supported profile field and every role-change write preserves a pending fake latch; only explicit W1C acknowledges it; expectations are literal bytes independent of the composer |
| Independent probe | PASS | 66,048 checks over 3,072 ACK/service operations: all raw09 bytes on six variants, E0-changed bookends, budgets0/255, exact2/5 callbacks, actual controls, separate final09/recheck and effect/IRQ facts |
| Previous suites | PASS | 35,454 profile observation, 89,140 apply/verify, 88,779 reset/recovery, 20,490 owner, 161,670 protocol and 8,602,548 codec checks; values/validators/version/launcher pass |
| Documentation/source | PASS | Strict Doxygen without warnings; 17 public structs and24 implemented owner methods visible, future methods absent; local links/anchors, UTF-8/LF/whitespace, eight vendor hashes and no-op version sync |
| Native PlatformIO | PASS | Host consumer compiles, links and runs with zero fake callback attempts |
| Package consumption | PASS | 36 source-equal exported files, IDF selection and guide links; renamed extracted tests-OFF consumer beneath unrelated Git builds/runs without Python/Git/Doxygen/PlatformIO on child PATH |
| Separate evidence | NOT RUN | Physical IRQ wiring/levels/timing, electrical behavior and framework firmware. Installed MinGW sanitizer runtimes remain unavailable from same-day checks; MSVC/Clang/clang-format absent. Remote CI remains prompt34 |

Lead commands (existing Ninja Multi-Config tree):

```powershell
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
& build/native/test/Debug/tusb32x_port.exe
& build/native/test/Release/tusb32x_operations.exe
& build/native/test/Release/tusb32x_protocol.exe
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude -Itest build/p16-review/irq_probe.cpp build/native/Release/libTUSB32x.a -o build/p16-review/lead-probe.exe
& build/p16-review/lead-probe.exe
doxygen Doxyfile
python build/p16-doxygen.py
python build/p10-docs.py
scripts/pio.cmd run -d examples/host/basic -e native -t exec
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-p16.tar.gz
python build/p16-package.py
git diff --check
```

Permanent IRQ tests are in `test/test_interrupt.cpp`, registered as part of
`tusb32x_port`. Session-only review/probe/documentation/package scripts under
ignored `build/` were read and run by the lead. Initial fixture failures assumed
service published IRQ after its first read and that a changed advertisement left
configuration KNOWN. Corrected both to the frozen complete-publication and
negative-evidence contracts; no production workaround or suppressed assertion.
Final parallel review found no remaining production/test defect. Corrected stale
changelog wording and clarified that plain refresh preserves diagnostic IRQ history
while leaving its own operation IRQ record empty. Fake events are explicit inputs;
the model does not infer clear-on-read, lossless capture or electrical timing.

## Prompt 16 independent audit (2026-10-26)

Fresh audit started at synchronized `671134de40d90a0ebb3c6e30ca7eb54a11470c4d`
on the same execution branch. Re-read full prompt 16, baseline IRQ/port/time/
effect contracts, actual implementation diff and source/tests. Three independent
reviewers inspected engine correctness, requirements/TI/peers/docs and test oracles.
No production defect was confirmed. The lead inspected and reran the findings.

| Check | Result | Evidence |
| --- | --- | --- |
| Fresh strict Debug/Release | PASS | Seven CTests each after the regression additions; active Release checks and independent public-header/contract compilation |
| IRQ/port regressions | PASS | 71,617 IRQ checks plus 495,667 port checks; new service vectors vary first/final controls, IRQ and E0 bookends, verify literal final06/07 ACK16/17, field validity, zero-budget publication and retained data after ACK/recheck failure |
| Mutation evidence | PASS | Stale first09 control composition and falsely matching service bookends both pass the old 67,081 IRQ checks. New regressions reject them with 240 and 144 failed assertions; the real engine passes unchanged |
| Independent lifecycle probe | PASS | 144 stateful public-API cases: six variants, both operations, six transport errors, before/after simulated latch clear; take/end/same-target bind/reinitialize retain IRQ uncertainty without a settings obligation or ACK replay |
| Previous suites | PASS | 35,454 profile observation, 89,140 apply/verify, 88,779 reset/recovery, 20,490 owner, 161,670 protocol and 8,602,548 codec checks; values/validators/version/launcher |
| Source evidence | PASS | B pp16/18, L/A p18, V pp18–19 and SLAAE24 section8 p9 reread; eight TI hashes checked. INA228/PCA9555/LDC1614 remain main/clean at execution-16 revisions; relevant source/tests/examples/CLI/runners/instructions/workflow definitions reopened |
| Documentation/package | PASS | Strict Doxygen with 17 public structs and24 owner methods; future methods absent; links/anchors, UTF-8/LF/whitespace/version checks; source-equal36-file archive and renamed tests-OFF consumer under unrelated Git without maintainer tools |
| Separate evidence | NOT RUN | Physical IRQ/electrical behavior and framework firmware. Installed MinGW sanitizer runtimes remain unavailable from same-day checks; MSVC/Clang/clang-format absent. Native PlatformIO was not repeated for test/docs-only corrections; package consumption was rechecked |

Tools remain GCC15.1 MinGW-W64, CMake/CTest4.0.1, Ninja1.12.1, Python3.12.10
and Doxygen1.13.2. Lead commands:

```powershell
cmake -S . -B build/p16-independent-audit -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p16-independent-audit --config Debug
ctest --test-dir build/p16-independent-audit -C Debug --output-on-failure
cmake --build build/p16-independent-audit --config Release
ctest --test-dir build/p16-independent-audit -C Release --output-on-failure
& build/p16-independent-audit/test/Release/tusb32x_port.exe
& build/p16-independent-audit/test/Release/tusb32x_operations.exe
& build/p16-independent-audit/test/Release/tusb32x_protocol.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude build/p16-audit-engine/history_probe.cpp build/p16-independent-audit/Debug/libTUSB32x.a -o build/p16-audit-engine/lead-probe.exe
& build/p16-audit-engine/lead-probe.exe
& build/p16-audit-tests/stale-controls-mutant.exe
& build/p16-audit-tests/stale-controls-mutant-regressions.exe
& build/p16-audit-tests/coherence-mutant-before.exe
& build/p16-audit-tests/coherence-mutant-after.exe
doxygen Doxyfile
python build/p16-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-p16.tar.gz
python build/p16-package.py
git diff --check
```

Mutant runs with the new tests intentionally exit1; old-suite runs exit0. Their
only engine changes are in the service completion branch: substitute first09
low control bits while preserving final IRQ evidence, or pass final09 as both
decoder bookends. The isolated sources/executables under ignored
`build/p16-audit-tests/` are session evidence, never delivered code. The permanent
regression uses the existing public fixture and registered port suite. The
lifecycle probe similarly uses public operations without private state seeding.

Corrected stale API text that deferred implemented IRQ methods and added the
IRQ test file to the architecture inventory. No runtime behavior, public API,
silicon gate or dependency changed. Hardware qualification remains separate;
no remote CI was requested or checked.

## Prompt 17 raw diagnostics and passive health (2026-10-26)

Implemented from synchronized `df0fc2253cb01cfcfdb0973d14791b7d345f5a95` on the
same execution branch. Re-read full prompt17 and current Sync/access/accounting/
health contracts. Current read-only peer inspection is in
[execution-17-peers](audit/execution-17-peers.md); source differences in counter
units were adopted/rejected explicitly. No sibling edits or remote CI requests.

The existing sole callback wrapper already supplied accounting and normalization.
New raw methods and pure projection reuse it without a second engine or clock.
Independent strong safety review found typed45 apply needed to honor write-image
revocation after raw contradiction; a failing regression proved and now guards
the correction. Independent runtime/test review found no remaining code defect.
Final documentation review removed a duplicated changelog paragraph and stale
planned claims. Added an actual-versus-staged raw ACK case rather than claim it
from typed IRQ tests alone. Reviewed the complete production/test/docs/package diff.

Tools: GCC15.1.0 MinGW-W64 UCRT/MCF, CMake/CTest4.0.1, Ninja1.12.1, Python3.12.10,
Doxygen1.13.2 and installed VS Code-managed PlatformIO Core6.1.19/native1.2.1.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict Debug/Release | PASS | Eight registered CTests each; independent public-header and delivered API fixtures compile; Release runtime checks active |
| Diagnostics | PASS | 55,218 checks: seven variants/all256 offsets, exact one-callback frames, output preservation, slot backpressure, unsafe-sequence/access rejection, safe-image absence, same-value writes, pre-callback trust/effect changes and applied-then-error writes |
| Actual controls | PASS | All80 raw ACK variant/control/outcome cases stage differing duty/accessory with bus-silent cancellation, then preserve actual controls; protected45 contradiction blocks raw/typed45 only, independent current stays available, raw clear cannot renew proof, full reconciliation can |
| Health/accounting | PASS | Both raw directions normalize every callback Err byte with full detail; all-source versus managed exclusion, callback/local BUSY, independent saturating totals/streak and unknown timestamps; thresholds1/3/UINT32_MAX and invalid0 unchanged output; failed init, READY with UNKNOWN settings, OFFLINE explicit work and later managed success |
| Retained evidence | PASS | Clear leaves active/result, managed history/streak, last failure, trust/port age and unresolved recovery unchanged; result take/end/rebind retain sequence obligation; raw reset1 cannot supply a completion anchor, reconciliation waits200→296; historical settled reset ambiguity does not ban independent writes |
| Prior regressions | PASS | 35,454 profile observation,89,140 apply/verify,88,779 reset/recovery,20,490 owner,71,617 IRQ,495,667 port,161,670 protocol and8,602,548 codec checks; compile contracts/validators/version/Windows launcher pass |
| TI/source evidence | PASS | Fresh four official PDFs match archived B F/L D/A D/V D SHA256; exact mixed/control/procedure tables reopened; all eight archived TI hashes checked; unresolved322 advertisement/fault gates retained |
| Documentation/version | PASS | Strict Doxygen without warnings; 17 public structs,26 owner methods and real health functions visible in XML; local links/anchors, UTF-8/LF/whitespace and deterministic repeated sync/check |
| Ordinary/native consumer | PASS | Standalone CMake tests OFF and pinned native PlatformIO compile/link/run validators, decoder and UNINIT health with zero fake callback attempts |
| Packaging/component | PASS | Source-equal38-file archive, matching IDF inclusion and explicit five-source lists; renamed extracted Release consumer beneath unrelated Git compiles seven TUs with C++17, no Python/Git/Doxygen/PIO on child PATH; host-only component fixture links those same five sources |
| Sanitizers | BLOCKED | Fresh trivial GCC link probes fail: cannot find -lasan / -lubsan. No sanitizer run or result claimed; MSVC/Clang/clang-format absent |
| Separate evidence | NOT RUN | Framework firmware, physical bus/IRQ/electrical/current behavior and adapter aggregate blocking bounds. No hardware support or later conformance/CLI/omission-guard claim; remote CI belongs to prompt34 |

Lead commands (Ninja Multi-Config native tree; plain Ninja consumer):

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
& build/native/test/Release/tusb32x_diagnostics.exe
& build/native/test/Release/tusb32x_operations.exe
& build/native/test/Release/tusb32x_protocol.exe
& build/native/test/Release/tusb32x_port.exe

cmake -S examples/host/basic -B build/host-basic -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/host-basic --config Release
& build/host-basic/tusb32x_host_basic.exe
cmake -S build/p17-component-fixture -B build/p17-component-build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/p17-component-build
& build/p17-component-build/registration_consumer.exe

python scripts/generate_version.py sync
python scripts/generate_version.py check
python build/p10-evidence.py
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd run -d examples/host/basic -e native -t exec
scripts/pio.cmd pkg pack . --output build/TUSB32x-p17.tar.gz
python build/p17-package.py

g++ -std=c++17 -fsanitize=address build/p17-sanitizer.cpp -o build/p17-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p17-sanitizer.cpp -o build/p17-ubsan.exe
git diff --check
```

The last two compiler commands intentionally probe availability and both exit1;
all required available checks pass. Session-only extraction, XML/source and
component scripts under ignored `build/` were read and run by the lead. They
are validation evidence rather than shipped tools or fake firmware. The package
consumer script supplies absolute compiler/CMake/Ninja paths and verifies its
child PATH excludes maintainer tools; the package remains free of sibling paths,
reference/vendor artifacts and audit machinery. Final source lists remain explicit.

## Prompt 18 protocol conformance (2026-10-27)

Started at synchronized `8507681e92938b3eb8d0a7aa7308a1c8318ecbaf` on the same
execution branch. The [peer report](audit/execution-18-peers.md) records fresh
read-only evidence; [protocol conformance](protocol-conformance.md) maps every
required variant/protocol and cross-operation scenario to actual runtime cases.
Three parallel reviews found test gaps but no production defect. The lead checked
their findings against code/diffs, fixed the fake's successful-short-read loophole,
added public address and raw write-effect regressions and reviewed the final unit.

Tools: GNU15.1.0 MinGW-W64 UCRT/MCF, CMake/CTest4.0.1, Ninja1.12.1,
Python3.12.10 and Doxygen1.13.2. The installed PlatformIO wrapper packs the library;
native PlatformIO execution was not repeated for this test/documentation change.

| Check | Result | Evidence |
| --- | --- | --- |
| Focused regression checks | PASS | Protocol/operations/diagnostics selection 3/3; direct changed diagnostics and public operation runs pass |
| Full host Debug and Release | PASS | Eight registered CTests each; strict warnings and Release checks active; standalone headers/signatures compile |
| New public address coverage | PASS | 622 checks, seven variants at both legal addresses through initialize, refresh, ACK or 322I IRQ gates, raw read and bus-silent end |
| Protocol/codecs | PASS | 161,758 protocol and 8,602,548 codec checks; fake rejects all 0..7-byte short eight-byte receives marked OK before copying |
| Diagnostics | PASS | 74,962 checks; every supported ordinary raw failure has applied/not-applied cases; failed raw ACK has cleared/retained latch cases; unrelated successful read does not repair trust or erase original detail |
| Existing operation/port regressions | PASS | 20,490 owner, 35,454 observation, 89,140 apply/verify, 88,779 reset/recovery, 71,617 IRQ and 495,667 port checks; required interruptions/races retained without duplicate suites |
| Values/validators/tooling | PASS | Passive/constexpr/multiple-TU checks, 76,452 validator checks, 18 version-tool tests and 2 Windows-launcher tests |
| Address mutation | PASS, faulty code rejected | Isolated protocol copy changes managed 09 reads from high to low address. Old operations suites pass; new public regression rejects it with 112 failed assertions/exit 112 |
| Short-OK fixture probe | PASS | Eight-byte request with only 3 bytes supplied returns failure/errors 1/incomplete after the fix; permanent runtime fixture covers every short length |
| Documentation/provenance | PASS | Strict Doxygen without warnings; 17 public structs / 26 owner methods and pure functions visible; local links/anchors, UTF-8/LF, whitespace, eight archived TI SHA256 values and repeated no-op version sync |
| Package consumer | PASS | 38 source-equal exported files, IDF inclusion matches; renamed tests-OFF Release consumer under unrelated Git compiles/links seven TUs as C++17, runs with zero fake attempts and no Git/Python/Doxygen/PIO on child PATH |
| Sanitizers | BLOCKED | Fresh trivial link probes cannot find -lasan or -lubsan; no sanitizer runtime result claimed. MSVC/Clang/clang-format absent |
| Separate evidence | NOT RUN | Physical bus/IRQ/electrical/current/timing qualification, framework firmware and native PlatformIO execution. Remote CI remains prompt 34; unresolved TI/board gates unchanged |

Lead commands (focused checks preceded the final full suites):

```powershell
cmake -S . -B build/p18-conformance -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p18-conformance --config Debug
ctest --test-dir build/p18-conformance -C Debug -R 'tusb32x_(protocol|operations|diagnostics)$' --output-on-failure
cmake --build build/p18-conformance --config Debug --target tusb32x_diagnostics
& build/p18-conformance/test/Debug/tusb32x_diagnostics.exe
& build/p18-conformance/test/Debug/tusb32x_protocol.exe
& build/p18-conformance/test/Debug/tusb32x_operations.exe
cmake --build build/p18-conformance --config Debug
ctest --test-dir build/p18-conformance -C Debug --output-on-failure
cmake --build build/p18-conformance --config Release
ctest --test-dir build/p18-conformance -C Release --output-on-failure

& build/p18-test-audit/old_operations_address_mutant.exe
& build/p18-test-audit/public_address_mutant.exe
& build/p18-test-audit/short_ok_probe.exe
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-p18.tar.gz
python build/p18-package.py
g++ -std=c++17 -fsanitize=address build/p17-sanitizer.cpp -o build/p18-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p17-sanitizer.cpp -o build/p18-ubsan.exe
git diff --check
```

The address mutant and sanitizer probes intentionally fail as classified above.
Ignored build scripts/executables are session evidence, not shipped dependencies.
The lead inspected the isolated mutation/probe source and reran the executables;
the new regression also passes against the real library in both full builds.
Documentation clarifies that Status-only transport cannot detect a falsely
successful short receive, DE-to-56 is arithmetic rather than permission to write
during reset, and no explicit fault-clear write does not prove 322I read effects.

## Independent prompts 16-18 audit (2026-10-27)

Fresh audit from clean synchronized `91e8eb4a1b070c04ca5a62453916978a72da578d`.
Three independent reviewers read actual code/tests and original prompts. IRQ
composition, finite service, observation retention, accounting and passive health
matched their contracts. Two confirmed gaps were fixed: stale literal 322I mode
images after equivalent DRP observations, and missing 322I-specific mode-failure/
interruption cases. The lead reviewed all changes and independent probes.

The before-fix reproduction initializes 0A=02, reads 32, then writes 42. Old code
invokes that write despite having observed DRP11; fixed code returns
CONFIGURATION_UNKNOWN with zero callbacks. The registered regression covers
both directions, raw/partial managed observations, raw/typed writers and full
proof renewal. Compiling the final regression against the two unchanged old
production units produces 20 failed assertions; the fixed suite passes.

| Check | Result | Evidence |
| --- | --- | --- |
| Focused independent checks | PASS | IRQ protocol/operations/port 3/3, then current production protocol/operations/port/diagnostics 4/4; direct changed suites and probes |
| Full strict Debug/Release | PASS | Fresh Ninja Multi-Config tree, eight registered CTests each; public header/signature fixtures compile; Release checks active |
| Diagnostics regression | PASS | 75,426 checks; both 00/11 directions retain semantic KNOWN but reject stale literal writes, unchanged rejected tokens, no-op admission and complete reconciliation renewal |
| 322I mode failures/interruption | PASS | 93,440 apply checks total; new 12 write-failure cases, 36 cancellation/deadline cases and 2 final-fresh successes across DFP/UFP, preserving original error/effects without delay/retry/replay |
| Existing suites | PASS | 161,758 protocol; 8,602,548 codecs; 35,454 profile observation; 88,779 reset/recovery; 622 public addresses; 20,490 owner; 71,617 IRQ; 495,667 port; 76,452 validator checks; passive/constexpr/multiple-TU checks; 18 version and 2 launcher tests |
| Independent mode-image probe | PASS | 216 checks: completed no-op proof renews permission, fresh-precheck reset remains available, completed profile-only observation does not renew; both DRP directions |
| Before-fix regression | FAIL as intended | Old source with final diagnostics regression: 20 failed assertions. Isolated initial probe permits the forbidden stale write; fixed probe rejects before callbacks |
| Documentation/source evidence | PASS | Strict Doxygen, 17 public structs / 26 owner methods visible, local links/anchors, UTF-8/LF/whitespace, eight archived TI hashes, repeated no-op version sync/check; current peer states/source registrations rechecked |
| Consumers/package | PASS | Source-equal 38-file archive and matching IDF inclusion; extracted renamed tests-OFF consumer compiles/links/runs beneath unrelated Git with no maintainer tools on child PATH; native PlatformIO consumer runs with zero fake attempts |
| Sanitizers | BLOCKED | Fresh GCC link probes fail because libasan/libubsan are absent; no sanitizer result claimed |
| Hardware/framework firmware | NOT RUN | No electrical/IRQ/timing qualification or linked Arduino/IDF firmware. Existing 322I gates remain unresolved. Remote CI belongs to prompt 34 |

Tools remain GNU15.1.0 MinGW, CMake/CTest4.0.1, Ninja1.12.1, Python3.12.10,
Doxygen1.13.2, installed PlatformIO Core6.1.19/native1.2.1. MSVC/Clang/clang-format
remain absent. Lead final commands:

```powershell
cmake -S . -B build/audit161718-final -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/audit161718-final --config Debug
ctest --test-dir build/audit161718-final -C Debug --output-on-failure
cmake --build build/audit161718-final --config Release
ctest --test-dir build/audit161718-final -C Release --output-on-failure
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror -Iinclude -Isrc test/test_diagnostics.cpp src/TUSB32x.cpp src/TUSB32xCodecs.cpp src/TUSB32xProtocol.cpp build/audit161718-raw/TUSB32xOperations-before.cpp build/audit161718-raw/TUSB32xDiagnostics-before.cpp -o build/audit161718-raw/lead-before.exe
& build/audit161718-raw/lead-before.exe
& build/audit161718-raw/probe.exe
& build/audit161718-raw/lead-fixed-probe.exe
& build/audit161718-irq/mode_probe.exe
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-audit161718.tar.gz
python build/audit161718-package.py
scripts/pio.cmd run -d examples/host/basic -e native -t exec
g++ -std=c++17 -fsanitize=address build/p17-sanitizer.cpp -o build/audit161718-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p17-sanitizer.cpp -o build/audit161718-ubsan.exe
git diff --check
```

The old units were extracted read-only from HEAD with `git show`; no tracked
production file was reverted. Ignored probes and extraction/visibility scripts
were inspected and run by the lead. Permanent regressions use existing public
operations and fixed fixtures; no public state seeds or second engine were added.

## Prompt 19: owner portability (2026-10-28)

Started from synchronized `4207107e5fc9bf2d5b8f7464bb4342f7763317b9` on the
existing execution branch. The [peer audit](audit/execution-19-peers.md) and
[named coverage matrix](owner-portability.md) distinguish new executable evidence
from retained timing, IRQ and diagnostics regressions. No production timing defect
was found. Both generation increments now use one directly tested private helper.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict full Debug and Release | PASS | Nine registered CTests in each configuration, nonzero discovery; five independently included public headers and contract fixtures compile; six C++ suites plus isolated consumer and two tooling suites |
| External owner scheduler | PASS | 4,196 checks; two distinct targets, alternating turns during recovery wait/failure/repeated IRQ, two queued requests and one delivery slot per owner, retained core result under backpressure, exact request/token correlation, 0/1/255 budgets |
| Physical late-write case | PASS | Literal108 callback start/deadline110 clips timeout to2; callback physically returns111; pending success becomes TIMEOUT, all three backend error kinds remain primary with full detail and possible write effect |
| Generation boundary | PASS | Exact production helper at0/MAX-1/MAX; both desired/verified increments use it; owner suite now20,493 checks |
| Isolated public consumer | PASS | Two TUs, 369 checks, every current facade method and helper links/runs; only copied headers/library/fixture and inherited compile/link flags; child PATH excludes Git/Python/Doxygen/PlatformIO, with compiler runtime DLLs supplied separately |
| Allocation and live-check sentinels | PASS | Normal consumer0; deliberate check1; armed C++ allocation86; armed C allocation86 in Debug and Release; C wrapping enabled. Guard covers exercised core paths including construction/destruction, not dynamic runtime DLL internals |
| Existing regression counts | PASS | 8,602,548 codec; 161,758 protocol; 35,454 profile observation; 93,440 apply; 88,779 reset/recovery; 622 public conformance; 71,617 IRQ; 495,667 port; 75,426 diagnostics; 76,452 validator checks; passive/constexpr/TU checks; 18 version and 2 launcher tests |
| Documentation | PASS | Strict Doxygen and visibility of17 public structs/26 owner methods; local links/anchors, source hashes, UTF-8/LF and no-op version sync/check |
| Native PlatformIO | PASS | Existing VS Code-managed Core6.1.19, native1.2.1, real basic consumer builds/runs; zero fake callback attempts. Existing Core fetched its tool-scons4.40801.0 dependency; no second Core installed |
| ASan / UBSan | BLOCKED | Fresh GNU15.1 link probes cannot find `-lasan` / `-lubsan`; no sanitizer run claimed. Imported-consumer flags now preserve required runtime flags on supported hosts |
| Firmware / hardware | NOT RUN | No linked Arduino/IDF firmware, electrical timing/current resampling, IRQ wiring or physical qualification; existing silicon gates unchanged |
| Remote CI | NOT RUN | Explicit prompt34 scope; no dispatch/poll/wait |

Tool versions: GNU15.1.0 MinGW, CMake/CTest4.0.1, Ninja1.12.1, Python3.12.10,
Doxygen1.13.2 and PlatformIO Core6.1.19. MSVC, Clang and clang-format are absent.

Integration initially caught Windows environment casing and missing runtime DLLs
in the new isolation harness. Both were corrected without widening its tool PATH.
Independent review caught missing inherited compile/link flags, which are now
copied through a CMake initial-cache file including configuration-specific flags.
The final full builds above run the corrected harness, including Release NDEBUG
sentinels. No test failure was suppressed or counted as a successful consumer.

Commands (PowerShell, repository root):

```powershell
cmake -S . -B build/p19-owner -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p19-owner --config Debug
ctest --test-dir build/p19-owner -C Debug -R 'tusb32x_(operations|portable_consumer)$' --output-on-failure
ctest --test-dir build/p19-owner -C Debug --output-on-failure
cmake --build build/p19-owner --config Release
ctest --test-dir build/p19-owner -C Release --output-on-failure
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
scripts/pio.cmd run -d examples/host/basic -e native -t exec
g++ -std=c++17 -fsanitize=address build/p17-sanitizer.cpp -o build/p19-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p17-sanitizer.cpp -o build/p19-ubsan.exe
git diff --check
```

The two sanitizer probes fail for the stated unavailable runtimes. The registered
consumer test retains fresh directories under `build/p19-owner/test/portable-*`
with commands and output in CTest's `Testing/Temporary/LastTest.log`. Session-only
Doxygen/link/package scripts under ignored build directories are evidence helpers,
not downstream dependencies. The public consumer and coordinator are checked in.

Additional package regression PASS: `scripts/pio.cmd pkg pack . --output
build/TUSB32x-p19.tar.gz` then `python build/p19-package.py` verified38 source-equal
exported files, matching IDF selection and all exported relative guide links. A
renamed extracted tests-OFF consumer built/linked/ran beneath an unrelated Git
repository with no maintainer tools on child PATH. This checks the current
package boundary; prompt32 still owns the final distribution matrix.

## Prompt 20 bounded local CLI (2026-10-28)

Implemented from clean synchronized `c07e4e5473ffd5719b81f0451c79f3b15fa41532`, the
completed prompt 19, on the same execution branch. Read full prompt 20 and current common,
ownership, CLI grammar/parser/appearance/record contracts, public core methods and
test registration. Fresh source inspection is in
[execution 20](audit/execution-20-peers.md). The lead reopened malformed-control and
short/zero-write tests and verified rejected output-readiness polling pauses against
actual peer code. No sibling changes/builds or remote CI requests occurred.

Delivered one example-only dispatcher/staging value, fixed console and output/help
unit. All 27 local rows are complete and bus-silent; no startup command, device
handler, core poll or second engine was added. Private friend access in host fixtures
exercises mathematical limits without public seed setters or production commands.
Independent strong review and lead inspection corrected profile association masks,
base accessory capability, bounded string scanning, exhaustion admission/retry,
split-CRLF direct admission and complete row response metadata. Failed commands
report action=UNCHANGED, enforced by C++ rejection and Python schema tests. Kept existing core
behavior and extended the required registrations, not a build-only passing test.

Tools: GCC 15.1.0 MinGW-W64 UCRT/MCF, CMake/CTest 4.0.1, Ninja 1.12.1, Python 3.12.10,
Doxygen 1.13.2 and installed VS Code-managed PlatformIO Core 6.1.19/native 1.2.1.
The native Core run fetched its existing configured SCons tool 4.40801.0; no second
PlatformIO Core was installed. No tool/dependency pin or library version changed.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict Debug/Release | PASS | Twelve registered CTests each, active C++ checks in Release, independent public headers and prior portable allocation/failure sentinels |
| CLI routes and state | PASS | 133,669 checks: 27 canonical rows, listed aliases/topics, zero callback deltas, complete syntax/state preservation, candidate/core separation, desired discard, variant-specific controls and atomic LAI/0x47 to base/0x60 changes with 118/133 ms selections reset to 168 ms |
| Ownership/evidence | PASS | Bound/active/retained-result and unresolved-effect target fences; explicit forget only; borrowed-handle validation; local target edits retain old core profile attribution/masks rather than manufacture controls |
| Console | PASS | 17,787 console checks: all byte classifications, 191-byte line payload, sticky overflow/NUL/control poison, delete/CRLF fragmentation, queue 8192/record 512/drain 256, short/zero/wrapped writes, whole-record losses and saturation; CLI checks additionally enforce 12 tokens/32-byte token payload and cumulative input 64/one command |
| Framing/output | PASS | Real formatter maximum-width platform/ACK/PROFILE/PORT records with 32-character nonce, full counters/times/enums; all 17 schema maxima incl newline/NUL; plain machine records, minimum 32-column colored help, coloroff and bounded/sanitized application messages with full signed detail |
| Session/help fairness | PASS | New marker/stage requests interleave with finite original help and preserve its old request/nonce/one END; ID/generation exhaustion never wraps; full-queue exhaustion reason eventually emits, repeated blocked ticks do not add repeated loss |
| Persistent transcript | PASS | 51 requests in one actual shared-C++ process; firmware identity supplied as host-fixture/unknown, real HELP metadata parity with Python rows; missing/duplicate/invalid keys, bad statuses/values, nonce/request/closure contradictions, framing loss and maximum-width fixtures |
| Documentation/version | PASS | Strict Doxygen without suppressed warnings; XML shows 17 public core structs/26 owner methods plus 10 Cli and 12 Console callable methods; local links/anchors, UTF-8/LF/whitespace, Python docstrings/style and deterministic version sync/check |
| Package/consumer | PASS | 43 source-equal selected files, IDF inclusion parity and guide links; renamed ordinary tests-OFF and CLI applications beneath unrelated Git compile/link/run C++17 without Python/Git/Doxygen/PIO on child PATH; CLI has zero callbacks and 9280 host-ABI object bytes |
| Ordinary/native host | PASS | Standalone tests-OFF CMake and installed native PlatformIO consumer run with zero fake callbacks; common CLI is absent from core's unchanged explicit five-source compilation |
| Sanitizers | BLOCKED | Fresh GCC trivial-link probes cannot find -lasan/-lubsan; no sanitizer execution claimed. MSVC/Clang/clang-format unavailable |
| Separate evidence | NOT RUN | Arduino/native-IDF firmware, S2/S3 linked memory/stack budgets, physical console/I2C/IRQ/electrical behavior and adapter aggregate blocking bounds. These remain later integration/qualification work; remote CI belongs to 34 |

Lead commands:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Release -R "tusb32x_cli|tusb32x_console|cli_transcript" --output-on-failure
& build/native/test/Release/tusb32x_cli.exe
& build/native/test/Release/tusb32x_console.exe
python tools/test_cli_transcript.py --host build/native/test/Release/tusb32x_cli_host.exe
python -m py_compile tools/tusb32x_cli_contract.py tools/test_cli_transcript.py

cmake -S examples/host/basic -B build/host-basic -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/host-basic --config Release
& build/host-basic/tusb32x_host_basic.exe
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd run -d examples/host/basic -e native -t exec
scripts/pio.cmd pkg pack . --output build/TUSB32x-p20.tar.gz
python build/p20-package.py
python build/p20-final-check.py

g++ -std=c++17 -fsanitize=address build/p20-sanitizer.cpp -o build/p20-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p20-sanitizer.cpp -o build/p20-ubsan.exe
git diff --check
```

Both sanitizer availability probes exit1 for absent runtime libraries; all required
available checks pass. Session-only scripts under ignored `build/` were inspected
and run by the lead. XML inspection additionally checked named public Cli/Console
methods and their visible briefs. The package check supplies absolute compiler/
CMake/Ninja paths and strips maintainer tools from the child PATH. It checks an
independent application linking the shipped CLI sources as well as ordinary core
consumption; it is neither a framework stub nor a firmware result. Later operation,
device-command, runner/HIL and full omission-guard stages are not claimed here.

## Prompt 21 managed CLI operations (2026-10-28)

Started clean and synchronized after fetch at completed prompt 20 commit
`072015c5b5d977bfe81d42978bbb587b5b8e2fea`, on the same execution branch. Read
current instructions, full 21, grammar/record/result ownership, cooperative owner
time/trust/effect contracts, actual core declarations and registered tests. Fresh
peer evidence, failed-cleanup/budget cases and explicit adopted/rejected choices
are in [execution 21](audit/execution-21-peers.md). No sibling mutation/build or
remote CI request occurred.

Delivered 17 new rows, 44 total, with canonical core start APIs, one tick poll and
no CLI result store or register sequences. Core owns its one retained result.
Independent review corrected hidden nonterminal poll errors, conservative step
effect metadata, missing result-delivery payload validation and contradictory
cancelled/timed-out schemas. Lead reviewed actual diffs and fixtures and ran the
checks below; agent reports are not substituted for integration evidence.

Tools: GCC 15.1.0 MinGW-W64 UCRT/MCF, CMake/CTest 4.0.1, Ninja 1.12.1, Python
3.12.10, Doxygen 1.13.2 and installed managed PlatformIO Core 6.1.19/native
1.2.1. No tool pin, core source list or library version changed.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict framework-free Debug/Release | PASS | All 12 registered CTests each, with active Release checks and prior core/isolated-consumer/version/launcher regressions |
| Local CLI / framing | PASS | 149,227 checks, including all 44 executable rows/help topics, zero handler deltas, strict parsing, maximum-width real RESULT/DETAIL/END and continued completion after request exhaustion |
| Managed CLI behavior | PASS | 7,801 independent literal-byte checks: every route/alias, accepted desired capture only, zero/one/255 budgets, explicit zero overriding auto, immutable deadlines, timeout clipping, late success and backward/half-range clock rejection |
| Result ownership / correlation | PASS | Core slot retained through repeated query, stale token, lost announcements and failed reservation; near-full rejection delivers complete BUSY without fabricated record/byte loss; one take, delivery OK versus payload failure, original request 24 completion after request 25 and a changed marker |
| Hardware-effect logic | PASS | Read-only init/reconcile and verification aliases; actual 50% versus staged/desired 30% ACK; unrelated apply keeps pending W1C latch; failed write/readback retains signed INT32_MIN, uncertain effects and UNKNOWN trust; cancel/end still work with full output; deadline expiry still runs |
| Variant / waits | PASS | Reset for base/LAI/322I, explicit base 04/wait31/00 recovery and later-part denial, 322I fault/advertisement gates and allowed changed mode with unchanged proved advertisement; invalidated historical port remains visible |
| Console / persistent transcript | PASS | 17,787 unchanged console checks; 178 requests in one actual shared-C++ process plus two fault sessions, all 44 executed HELP rows, 20 bounded schemas and missing/duplicate/wrong token/origin/status/closure negatives |
| Documentation / version | PASS | Strict Doxygen; XML visibility of 17 core structs/26 owner methods plus 10 Cli/12 Console callable methods; Python docstrings/style, local links/anchors, UTF-8/LF/whitespace and no-op version sync/check |
| Package / ordinary consumption | PASS | Current 43-file source-equal archive, matching IDF inclusion and guide links; renamed tests-OFF core and CLI C++17 consumers beneath unrelated Git build/link/run without Python/Git/Doxygen/PIO on child PATH; CLI size 9416 host-ABI bytes, zero fake callbacks for local consumer commands |
| Native host environment | PASS | Standalone ordinary CMake consumer and existing managed PlatformIO native consumer; this does not link Arduino/native-IDF firmware |
| Sanitizers | BLOCKED | Fresh trivial ASan/UBSan link probes fail for unavailable -lasan/-lubsan; MSVC/Clang/clang-format absent |
| Physical / framework qualification | NOT RUN | Serial, CC/INT/electrical behavior, stable-VDD fixture, Arduino/native-IDF links, S2/S3 memory/stack and aggregate adapter blocking bounds remain separate later evidence |

Actual lead commands:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Debug -R "tusb32x_cli|tusb32x_console|cli_transcript" --output-on-failure
ctest --test-dir build/native -C Release -R "tusb32x_cli|tusb32x_console|cli_transcript" --output-on-failure
& build/native/test/Release/tusb32x_cli.exe
& build/native/test/Release/tusb32x_console.exe
python tools/test_cli_transcript.py --host build/native/test/Release/tusb32x_cli_host.exe
python -m py_compile tools/tusb32x_cli_contract.py tools/test_cli_transcript.py
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python build/p20-final-check.py
cmake -S examples/host/basic -B build/host-basic -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/host-basic --config Release
& build/host-basic/tusb32x_host_basic.exe
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd run -d examples/host/basic -e native -t exec
scripts/pio.cmd pkg pack . --output build/TUSB32x-p21.tar.gz
python build/p21-package.py
g++ -std=c++17 -fsanitize=address build/p20-sanitizer.cpp -o build/p21-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p20-sanitizer.cpp -o build/p21-ubsan.exe
git diff --check
```

Both sanitizer probes exit 1 for absent runtime libraries. Session-only checked
scripts under ignored build/ inspect XML/text and the exact source-equal archive;
they are not consumer dependencies. The package fixtures deliberately strip
maintainer tools from child PATH and use absolute CMake/Ninja/compiler paths.
No new SDK/framework dependency or successful hardware stub was added. Remaining
cached IRQ/pin, invalidation, register/decode and diagnostic command coverage is
prompt 22, not an unfinished operation route. Final commit/sync evidence belongs
in the handoff and next progress update, avoiding a self-referencing hash loop.

## Prompt 22 complete CLI diagnostics and parity (2026-10-28)

Started clean and synchronized after fetch at prompt 21 commit
`e1f5538f669af4f926af355f2bba904da9478049`, on the same
`promptset/tusb32x-implementation-20261023` branch. Read current instructions,
full 22, the fixed CLI grammar/records, raw access/accounting/health and example
boundaries, actual public APIs, registrations and shared sources. The prerequisite
CLI/console/transcript tests passed before editing. Fresh peer revisions, exact
paths and adopted/rejected choices are in [execution 22](audit/execution-22-peers.md).
No sibling mutation/build or remote CI request occurred.

Delivered the eleven remaining routes: all 55 fixed rows, canonical spellings,
aliases and help topics now execute. Raw access delegates to public Sync methods;
decoding delegates to the public decoder. No core source, register composer,
operation engine, library version or tool pin changed. Independent review found
missing configuration mismatch/acquisition output; CONFIG_DETAIL closes that gap
with real nonzero/distinct observed evidence. Lead reviewed actual changes and
ran the checks below rather than relying on agent summaries.

Tools: GCC 15.1.0 MinGW-W64 x86_64 UCRT/MCF, CMake/CTest 4.0.1, Ninja 1.12.1,
Python 3.12.10, Doxygen 1.13.2 and the installed managed PlatformIO Core
6.1.19/native platform 1.2.1.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict framework-free Debug/Release | PASS | All 14 registered CTests in each configuration, active Release assertions, including affected prior core/consumer/version/launcher regressions; eight C++ suites, one transcript, positive/negative parity, portable consumer, version and Windows launcher tests |
| CLI behavior and framing | PASS | 156,533 local/framing checks, 7,821 operation checks and 601,792 independent diagnostic checks (766,146 total); all canonical/alias/help routes, full invalid-input state preservation, zero cached/pure handler callbacks and independently progressing active work |
| Raw safety and effects | PASS | Exactly zero/one callbacks for local denial/transport, all seven variants and every callback code plus invalid enum; signed INT32_MIN preserved, normalization, unchanged failed-read output, accepted-before-error writes, safe-image/owner/result gates and forbidden sequenced changes |
| Accounting and health | PASS | All-source raw inclusion and managed exclusion, managed probe, OFFLINE recovery, saturation and exact equality rejection, idle-only reset invariance, retained failure detail/time validity, trust/effects and results preserved |
| Actual device evidence | PASS | Literal staged 30% versus applied 50% IRQ ACK behavior, preserved pending latch for unrelated writes, explicit invalidation/forget fences, mismatch register 8/expected 64/observed 128/mask 192/acquisition time 200 and retained result |
| Console and congestion | PASS | 24,146 checks including whole-record losses, short/zero writes, bounded drain/fairness; a failed raw write under a full queue invokes once, retains uncertain effects and never retries during 100 blocked turns |
| Persistent transcript | PASS | 248 requests in one actual shared-source C++ process, two managed-fault sessions, four application-input modes and two raw-fault sessions; typed detail, source labels, statuses, effects and normal closure checked |
| Executable parity and negatives | PASS | 55 rows/164 canonical, alias, topic and per-command help routes; actual syntax/metadata/required records/statuses and adapter callback/write deltas; deliberately broken alias and required-record-field dependencies reject; one shared help comparator and one transcript registration |
| Bounds and appearance | PASS | All 33 schema maxima and actual formatter paths with 32-character nonce, full-width integers/enums, newline/NUL; unknown/reserved/not-applicable distinction, cyan/green grouped help, minimum 32-column syntax, verbosity/error visibility and exact color-off behavior |
| Documentation and version | PASS | Strict Doxygen and XML visibility of 17 core structs/26 owner methods plus 10 Cli/12 Console callable methods; all three delivered Python modules have docstrings/style, local links/anchors and UTF-8/LF/whitespace checks; no-op version sync/check |
| Package and ordinary consumption | PASS | Current 43-file source-equal archive, normal exclusion policy, matching IDF inclusion and guide links; renamed tests-OFF core and CLI C++17 consumers beneath unrelated Git build/link/run with no Python/Git/Doxygen/PIO on child PATH; zero local fake callbacks and sizeof(Cli)=9416 host-ABI bytes |
| Native host environment | PASS | Ordinary standalone tests-OFF CMake and installed managed PlatformIO native consumers; no framework firmware claim |
| Sanitizers and other compilers | BLOCKED | Fresh trivial ASan/UBSan link probes exit 1: missing -lasan/-lubsan. MSVC, Clang and clang-format unavailable |
| Physical/framework qualification | NOT RUN | Serial/connector/CC/INT behavior, Arduino/native-IDF firmware links, adapter aggregate blocking/framing bounds, board RAM/stack, electrical power policy and stable-VDD recovery remain separate evidence |

Actual lead commands:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Debug -R "tusb32x_cli|tusb32x_console|cli_transcript|cli_contract" --output-on-failure
& build/native/test/Release/tusb32x_cli.exe
& build/native/test/Release/tusb32x_console.exe
python tools/check_cli_contract.py --host build/native/test/Release/tusb32x_cli_host.exe
python tools/check_cli_contract.py --host build/native/test/Release/tusb32x_cli_host.exe --self-test-negative
python tools/test_cli_transcript.py --host build/native/test/Release/tusb32x_cli_host.exe
python -m py_compile tools/tusb32x_cli_contract.py tools/test_cli_transcript.py tools/check_cli_contract.py
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python build/p22-final-check.py
cmake -S examples/host/basic -B build/host-basic -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/host-basic --config Release
& build/host-basic/tusb32x_host_basic.exe
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd run -d examples/host/basic -e native -t exec
scripts/pio.cmd pkg pack . --output build/TUSB32x-p22.tar.gz
python build/p22-package.py

g++ -std=c++17 -fsanitize=address build/p20-sanitizer.cpp -o build/p22-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p20-sanitizer.cpp -o build/p22-ubsan.exe
git diff --check
```

Both sanitizer availability probes exit 1; required available checks pass. The
inspected session-only scripts in ignored build/ check public XML, text and the
source-equal archive. Extracted consumer checks supply absolute compiler/CMake/
Ninja paths and remove maintainer tools from child PATH. They are independent
applications using the shipped example sources, not successful firmware stubs.
Finite CTest timeouts and explicit built fixture paths remain registered, with
13 tests on other hosts and the additional launcher test on Windows. Final Git
commit/sync equality and clean-state evidence belongs in the handoff and next
progress update, avoiding a self-referencing hash loop. Prompt 23 is unstarted.


## Independent CLI audit (2026-10-28)

Fresh review of prompts 20, 21 and 22 at entry `6d99a29`; three parallel reviewers
and lead inspection of actual source and diff. Findings and current peer evidence
are in [the audit](audit/cli-20-22-review.md). Only fixed help order, host transcript
closure/parity checking and focused regressions changed; core behavior is unchanged.

Tools: GCC 15.1.0, CMake/CTest 4.0.1, Ninja 1.12.1, Python 3.12.10, Doxygen 1.13.2,
managed PlatformIO Core 6.1.19 with native 1.2.1.

| Check | Result | Evidence |
| --- | --- | --- |
| Fresh strict Debug and Release | PASS | 14/14 registered tests in each; Release assertions active |
| CLI suites | PASS | 156,606 local + 8,027 operation + 601,792 diagnostic = 766,425 checks; no failures |
| Persistent transcript and parity | PASS | 248 primary-session requests plus existing fault/input sessions, 11 new ordering/EOF cases, all 55 rows/164 executable routes and existing alias/schema negatives |
| Reproduced original defects | PASS | New help assertions against original dispatch fail 17 times; original PersistentHost accepts duplicate END; ready job.step now requires exactly one callback rather than accepting expiry |
| Documentation | PASS | Strict Doxygen, 17 core structs/26 owner methods and 10 Cli/12 Console callable methods visible; Python style/docstrings, local links/anchors, source hashes and no-op version checks |
| Native and package consumers | PASS | Managed PlatformIO native execution; 43 source-equal archive files; extracted tests-OFF C++17 core and CLI compile/link/run beneath unrelated Git with maintainer tools removed from child PATH; zero fake callbacks |
| Sanitizers | BLOCKED | Fresh trivial probes cannot link -lasan/-lubsan with the installed GCC; no sanitizer pass claimed |
| Framework, serial and hardware | NOT RUN | Separate later qualification; prompt 23 remains unstarted |

Actual commands (session-only verification helpers remain in ignored `build/`):

```powershell
cmake -S . -B build/audit202122 -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/audit202122 --config Debug
ctest --test-dir build/audit202122 -C Debug --output-on-failure
cmake --build build/audit202122 --config Release
ctest --test-dir build/audit202122 -C Release --output-on-failure
& build/audit202122/test/Release/tusb32x_cli.exe
python tools/test_cli_transcript.py --host build/audit202122/test/Release/tusb32x_cli_host.exe
python tools/check_cli_contract.py --host build/audit202122/test/Release/tusb32x_cli_host.exe
python -m py_compile tools/tusb32x_cli_contract.py tools/test_cli_transcript.py tools/check_cli_contract.py
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python build/p22-final-check.py
scripts/pio.cmd run -d examples/host/basic -e native -t exec
scripts/pio.cmd pkg pack . --output build/TUSB32x-audit202122.tar.gz
python build/audit202122-package.py
g++ -std=c++17 -fsanitize=address build/p20-sanitizer.cpp -o build/audit202122-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p20-sanitizer.cpp -o build/audit202122-ubsan.exe
git diff --check
```

An overlong Python assertion line was wrapped and its style/transcript/parity
checks repeated. A session-only package helper initially decoded its source with
the Windows default encoding and failed before checking any package; replacing
it with an explicit UTF-8 read fixed the helper, and both extracted consumers
then passed. These were local verification issues, not suppressed failures.
Final commit/sync evidence is reported in the handoff without a self-referencing
commit-hash update loop. Remote CI was not queried under the initial promptset rule.

## Prompt 23: Arduino transport (2026-10-28)

Delivered example-only terminal Wire callbacks with the source-audited SDK subset
described in [integration](arduino-transport.md). Peer revisions, exact SDK source
hashes and review decisions are in [the audit](audit/execution-23-arduino.md).
No framework dependency was added to core or shared CLI compilation.

Tools: host GCC15.1.0, CMake/CTest4.0.1, Ninja1.12.1, Python3.12.10, Doxygen1.13.2;
managed PlatformIO6.1.19; pioarduino55.03.311; Arduino3.3.11;
IDF5.5.5+sha.b774170ff46; target GCC14.2.0+20260121.

| Check | Result | Evidence |
| --- | --- | --- |
| Strict Debug/Release host matrix | PASS | 16/16 registered tests in each, including all core/CLI/consumer regressions; assertions active in Release |
| Adapter contract | PASS | Final focused runs: 1,129 checks at 1000Hz, 1,106 at 100Hz; actual adapter compiled against recording SDK headers |
| Error and output edges | PASS | Exact pointer STOP/read STOP, short writes with flush/no transfer, partial/invalid receive retirement across target addresses, code2 generic and code5 timeout, signed detail, unchanged failed output, timeout restoration, no bus begin/end, gate contention and wrong-core/ISR rejection |
| Aggregate envelope | PASS | Independent fake time consumes derived SDK wait/cleanup envelopes; preflight rejection, remaining-phase clipping, coarse ticks, cap width, native time across millis rollover, saturation and deliberately late backend failures |
| Publication regression | PASS | Prior post-copy timeout path produces two expected failures; corrected path has no fallible check after copying caller output |
| ESP32-S2 SDK link | PASS | esp32-s2-saola-1; 17,888 bytes RAM, 250,632 bytes flash; real Wire/HAL/core/adapter linked |
| ESP32-S3 SDK link | PASS | esp32-s3-devkitc-1; 22,968 bytes RAM, 288,413 bytes flash; real Wire/HAL/core/adapter linked |
| SDK configuration evidence | PASS | Compilation databases show C++17 and required lock/log flags on Wire and adapter, same global flags on HAL C; target nm confirms all four adapter entry points |
| Documentation/version | PASS | Strict Doxygen and XML visibility of four adapter functions plus Context/WireStats; existing core visibility guards, local links/anchors, UTF-8/LF/whitespace, vendor hashes and no-op version check |
| Distribution boundary | PASS | 46 source-equal archive files, matching IDF export selection; tests/fakes/SDK fixtures excluded; isolated tests-OFF C++17 core and CLI consumers link/run without maintainer tools |
| Sanitizers | BLOCKED | Installed GCC lacks ASan/UBSan link runtimes; no sanitizer pass claimed |
| Physical/application qualification | NOT RUN | No flash, board scheduling/CPU bound, CC/INT/VBUS behavior, electrical timing or cleanup acceptability measured |

Actual commands:

```powershell
cmake -S . -B build/p23 -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p23 --config Debug
ctest --test-dir build/p23 -C Debug --output-on-failure
cmake --build build/p23 --config Release
ctest --test-dir build/p23 -C Release --output-on-failure
ctest --test-dir build/p23 -C Debug -R arduino_wire --output-on-failure
ctest --test-dir build/p23 -C Release -R arduino_wire --output-on-failure
& build/p23/test/Release/tusb32x_arduino_wire_1000.exe
& build/p23/test/Release/tusb32x_arduino_wire_100.exe
scripts/pio.cmd run -d test/arduino_sdk -e arduino_s2 -e arduino_s3
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python scripts/generate_version.py check
scripts/pio.cmd pkg pack . --output build/TUSB32x-p23.tar.gz
python build/p23-package.py
git diff --check
```

Final focused checks were repeated after the publication regression and
configuration guards were completed; the full preceding matrix had passed.
Session-only source/hash/XML/package checks remain in ignored build output.
The SDK fixture initially exposed Arduino macro collisions and a leftover SDK
C++20 flag; it now avoids the unnecessary Arduino.h include and explicitly
removes that flag before selecting C++17. Both real targets were rebuilt.

The declared bound is conditional on the integration prerequisites, especially
qualified whole-callback CPU/scheduling overhead and same-core exclusive ownership.
Receive failure retires the entire controller until explicit application
teardown/reinitialization; it does not silently recover. This conservative subset
is usable without making an unmeasured hardware guarantee. Remote CI remains
prompt34 only. Final commit/upstream equality is reported in the handoff.


## Prompt 24: Arduino applications (2026-10-28)

The [example guide](arduino-examples.md) documents usable CLI and two-device owner
applications, exact wiring/qualification, USB behavior and build commands.
The [audit](audit/execution-24-arduino-examples.md) records current peer and SDK evidence.
Core and adapter implementation remain unchanged; one example-only CLI record
hook shares the existing ordered bounded output queue.

Tools: GCC15.1.0, CMake/CTest4.0.1, Ninja1.12.1, Python3.12.10, Doxygen1.13.2;
VS Code-managed PlatformIO6.1.19; pioarduino55.03.311, Arduino3.3.11,
IDF5.5.5+sha.b774170ff46, target GCC14.2.0+20260121.

| Check | Result | Evidence |
| --- | --- | --- |
| Complete strict host matrix | PASS | Debug 20/20; Release 20/20, checks active; includes prior core/CLI/console/adapter suites |
| Application owner | PASS | 924 checks: exact startup reads/results, every read failure, no retry, budgets, rollover/deadline, shared-bus fairness and shutdown |
| Startup/CLI handoff | PASS | 277 checks: full/stalled console cannot prevent startup take, no duplicate poll, later CLI result retained, shutdown preserves external result |
| USB mailbox | PASS | 1574 checks: copied lifetime, bounds, short writes, order, real worker blocked while producer returns, two-second test watchdog |
| Firmware identity/configuration | PASS | Seven Python cases, five valid and 25 negative real header compilations; temporary Git clean/dirty/untracked HEAD, unrelated parent and missing Git |
| ESP32-S2 interactive CLI | PASS | esp32s2dev, real SDK/core/CLI/adapter link; one setup/loop |
| ESP32-S3 interactive CLI | PASS | esp32s3dev, real SDK/core/CLI/adapter link; one setup/loop |
| ESP32-S2 owner service | PASS | esp32s2_owner_service, separate source filter and main |
| ESP32-S3 owner service | PASS | esp32s3_owner_service, separate source filter and main |
| Enabled setup/IRQ/shutdown linkage | PASS | Separate S2/S3 owner builds with synthetic readiness/cleanup/overhead and IRQ/run-time overrides; compile evidence only |
| Documentation and sources | PASS | Strict Doxygen and existing public visibility checker; links/anchors, UTF-8/LF, provenance hashes, no-op version check and diff whitespace |
| Distribution boundary | PASS | 60 source-equal exported files, matching IDF inclusion, local links resolve; isolated extracted tests-OFF core/CLI consumers under unrelated Git with maintainer tools absent from PATH |
| Flash / serial / HIL | NOT RUN | No identified authorized board; no physical USB disconnect, GPIO/IRQ level, CC/VBUS, bus-cleanup or whole-callback timing qualification |

Actual commands (PowerShell):

```powershell
cmake -S . -B build/p24 -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p24 --config Debug
ctest --test-dir build/p24 -C Debug --output-on-failure
cmake --build build/p24 --config Release
ctest --test-dir build/p24 -C Release --output-on-failure
scripts/pio.cmd run -e esp32s2dev -e esp32s3dev -e esp32s2_owner_service -e esp32s3_owner_service
scripts/pio.cmd run -c build/p24-enabled.ini -e esp32s2_owner_service -e esp32s3_owner_service
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python scripts/generate_version.py check
scripts/pio.cmd pkg pack . --output build/TUSB32x-p24.tar.gz
python build/p24-package.py
git diff --check
```

The ignored enabled-path configuration copies root platformio.ini, sets a separate
`.pio/p24_enabled` build directory and appends these compile-only definitions:
`TUSB32X_BOARD_READY=1`, `TUSB32X_MAX_OVERHEAD_US=2000`,
`TUSB32X_ACCEPT_SDK_CLEANUP=1`, `TUSB32X_IRQ=6`, `TUSB32X_SECOND_IRQ=7`,
`TUSB32X_RUN_MS=10000`. The 2000us value is synthetic and is not a qualified
fixture bound. Do not flash that artifact as evidence of qualification.
SDK logs and generated FirmwareBuild.h files preserve actual environment,
UTC, revision and dirty state; implementation builds reported entry HEAD with
a dirty tree, rather than pretending to be clean firmware.

The first SDK pass exposed Arduino's cli() macro and the S2 compiler's lack of
an always-lock-free atomic<bool>. The global processor name and standard
atomic_flag handoff corrected these issues. Flags use one operation, never a
spin loop; the S2 SDK helper masks interrupts for the small exchange, while S3
uses word exchange. Subsequent actual images link successfully. Initial strict
Doxygen findings were corrected with complete macro/getter comments and explicit
constructor types, without suppressing warnings. Application review additionally
corrected startup IRQ loss, input validity before pin setup, raw configuration
narrowing and outcome/admission reporting.

Native software and linked SDK evidence do not establish physical qualification.
The prior unavailable host sanitizer runtimes remain a separate recorded limit;
this prompt does not claim a sanitizer run. No remote CI was required or queried.


## Prompt 25: native IDF transport, 2026-10-28

Toolchain: CMake4.0.1, Ninja1.12.1, host GCC15.1.0, Python3.12.10, Doxygen1.13.2;
managed PlatformIO Core6.1.19, pinned pioarduino55.03.311, native framework-espidf
3.50505.0 (IDF5.5.5), Xtensa GCC14.2.0+20260121. Native SDK builds use their
installed CMake4.0.3 and Ninja1.13.1 packages. The adapter version guard admits
only the audited SDK, S2/S3, disabled logging/PM/debug and 100..1000Hz ticks.

| Check | Result | Evidence / limit |
| --- | --- | --- |
| Debug complete host matrix | PASS | 22/22 registered suites, including all prior core/CLI/console/Arduino tests |
| Release complete host matrix | PASS | 22/22 with assertions active and strict host warnings |
| Native recording SDK contracts | PASS | Actual adapter; 1602 checks at 1000Hz, 1572 at 100Hz; fakes cannot prove physical ISR/cleanup timing |
| ESP32-S2 native configure/compile/link | PASS | test/idf_sdk environment idf_s2, isolated sdkconfig and output; core and all adapter functions retained |
| ESP32-S3 native configure/compile/link | PASS | test/idf_sdk environment idf_s3, isolated sdkconfig and output; core and all adapter functions retained |
| Native source/component boundary | PASS | Compile databases contain five core sources, real adapter and own app_main; no Arduino or CLI frontend; exact target/logging/tick configuration inspected |
| SDK source provenance | PASS | Local i2c_master.c equals official b774170ff46 bytes, 69303 bytes, SHA-256 recorded in integration guide |
| Documentation/source integrity | PASS | Strict Doxygen, public visibility, local links/anchors, UTF-8/LF/whitespace, eight reference hashes, version and diff checks |
| Distribution / ordinary consumers | PASS | 63 source-equal archive files; IDF include selection matches; renamed extracted tests-OFF core and CLI consumers under unrelated Git, maintainer tools absent from child PATH |
| Flash / serial / HIL / physical timing | NOT RUN | No identified authorized board; application CPU bound, same-core scheduling, bus cleanup waveform and electrical qualification remain prerequisites |

Actual commands:

```powershell
cmake -S . -B build/p25 -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p25 --config Debug
ctest --test-dir build/p25 -C Debug --output-on-failure
cmake --build build/p25 --config Release
ctest --test-dir build/p25 -C Release --output-on-failure
scripts/pio.cmd run -d test/idf_sdk -e idf_s2 -e idf_s3
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python scripts/generate_version.py check
scripts/pio.cmd pkg pack . --output build/TUSB32x-p25.tar.gz
python build/p25-package.py
git diff --check
```

Each SDK target produces `test/idf_sdk/.pio/build/<environment>/firmware.elf`,
`firmware.bin` and `firmware.factory.bin`. The final S2 image reports 9488 RAM /
154678 flash bytes; S3 reports 13408 RAM / 184633 flash bytes. Those are linker
usage figures, not complete file sizes. `nm -C` on each ELF shows validateContext,
write, writeRead, ownerNowMs and core bind; generated project/configuration files
show the intended distinct target and native framework. No upload was requested.

Initial configure while the adapter file was not yet present was rerun after
integration. A stricter main-component warning experiment exposed PlatformIO
propagation of those flags into unrelated SDK NVS sources and SDK libc's
include_next extension. The fixture retains normal native SDK warning settings;
strict actual-adapter warnings remain in both host builds and core warnings
remain strict in SDK builds. No SDK source patch, warning suppression or custom
build middleware was introduced. Final restored native builds are the evidence
above. Native idf.py commands are documented alternatives, NOT RUN here.

The source-derived envelope is conditional on truthful handle configuration,
shared serialization, nonmigrating same-core execution, uninterrupted RTOS tick
progress, disabled logging/PM and a separately qualified CPU/preemption bound.
A late-return check cannot replace those prerequisites. Host sanitizer runtime
availability remains as recorded in prompt 19; no new sanitizer result is claimed.
No remote workflow was dispatched or queried.

## Independent prompts 23–25 audit, 2026-10-28

The [fresh audit](audit/review-23-25.md) records corrected ownership, teardown,
USB pin, callback error and SDK timing restrictions. Tool versions remain:
host GCC15.1.0, CMake4.0.1, Ninja1.12.1, Python3.12.10, Doxygen1.13.2;
managed Core6.1.19, pioarduino55.03.311, Arduino3.3.11/IDF5.5.5,
Xtensa GCC14.2.0+20260121. Native SDK uses CMake4.0.3/Ninja1.13.1.

| Check | Result | Evidence / boundary |
| --- | --- | --- |
| Full strict Debug host matrix | PASS | 32/32 registered tests |
| Full strict Release host matrix | PASS | 32/32; checks remain active |
| Real application with recording SDK | PASS | Ten separately registered scenarios execute actual ExampleApp, adapter and CLI; no FreeRTOS or hardware timing claim |
| Wire adapter contracts | PASS | 1298 checks at1000Hz; 1275 at100Hz; zero-budget and rate rejection included |
| Native adapter contracts | PASS | 1806/1766 checks at1000/100Hz; default clock/rate gates and independent coarse-tick budget cases |
| Arduino adapter SDK links | PASS | arduino_s2 and arduino_s3 |
| Complete Arduino application SDK links | PASS | esp32s2dev, esp32s3dev, esp32s2_owner_service, esp32s3_owner_service |
| Enabled owner application SDK links | PASS | Additional S2/S3 builds with synthetic qualified setup/IRQ/run-time overrides; compile/link only, not measured board qualification |
| Native SDK component links | PASS | idf_s2 and idf_s3; separate targets/build directories, no Arduino/CLI source |
| Documentation/source integrity | PASS | Doxygen, 17 structs/26 public owner methods visible, local links/anchors, UTF-8/LF, vendor hashes, no-op version sync and diff whitespace |
| Extracted consumption | PASS | 63 source-equal archive files; manifest/component selection agrees; isolated renamed core/CLI consumers run without maintainer tools |
| Flash, serial, HIL, board timing | NOT RUN | No authorized physical fixture; overhead/ISR/USB/waveform/electrical evidence remains separate |
| Sanitizers | NOT RUN | Previously recorded unavailable host runtimes; no new sanitizer claim |

Actual commands (full host matrix ran after focused corrections):

```powershell
cmake -S . -B build/p25 -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p25 --config Debug
ctest --test-dir build/p25 -C Debug --output-on-failure
cmake --build build/p25 --config Release
ctest --test-dir build/p25 -C Release --output-on-failure
scripts/pio.cmd run -d test/arduino_sdk -e arduino_s2 -e arduino_s3
scripts/pio.cmd run -e esp32s2dev -e esp32s3dev -e esp32s2_owner_service -e esp32s3_owner_service
scripts/pio.cmd run -d test/idf_sdk -e idf_s2 -e idf_s3
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
scripts/pio.cmd pkg pack . --output build/TUSB32x-p25.tar.gz
python build/p25-package.py
git diff --check
```

Local logs use `build/audit2325-*`; session-only documentation/package checkers
are ignored verification helpers, not downstream dependencies. Actual firmware
artifacts are each environment's `.pio/build/<environment>/firmware.elf` and
`firmware.bin` beneath the root or corresponding SDK fixture. Application build
records identify entry commit7f23b90 and a dirty worktree during this review;
they do not pretend to identify a future commit or physical fixture. All corrected
runtime source was included. No remote workflow was dispatched or queried.

The additional enabled owner builds used
`scripts/pio.cmd run -c build/p24-enabled.ini -e esp32s2_owner_service -e esp32s3_owner_service`.
That ignored configuration applies BOARD_READY=1, MAX_OVERHEAD_US=2000,
ACCEPT_SDK_CLEANUP=1, IRQ=6/SECOND_IRQ=7 and RUN_MS=10000 solely to compile/link
the enabled paths. Those synthetic values do not qualify a connected board.


## Prompt 26 native IDF CLI application (2026-10-28)

Started clean and synchronized after fetch at
`4445cdc9a67aceed1347b8685aa2a2a347a79743` on the existing
`promptset/tusb32x-implementation-20261023` branch. Current parent/local rules,
full26, CLI/ownership, maintainability and native adapter contracts were read.
Fresh TMP1x2/SHT3x revisions, clean state, exact native/common/CLI/test/runner/
workflow/AGENTS paths and current official SDK examples/docs are in
[execution26](audit/execution-26-peers.md). No sibling edits/builds, remote CI,
flashing or hardware activity occurred.

The actual application reuses the core, native adapter, shared CLI and startup
owner. It owns UART0/controller0/device resources on the configured core0 main
task. UART has explicit input/output routes; default hardware work is disabled
until explicit board/timing/SDK-cleanup qualification. Startup only identifies
and observes, taking each exact application result before CLI handoff. Shutdown
ends borrowing, preserves/reports outcomes/effects and removes only owned handles.
Independent review found and corrected possible UART error-event loss, repeated
failed teardown and deletion before hardware FIFO completion. Source and tests
were checked by the lead; agent reports were not substituted for integration.

Tools actually used: host GCC15.1.0 MinGW UCRT/MCF, CMake/CTest4.0.1, Ninja1.12.1,
Python3.12.10, Doxygen1.13.2 and installed managed PlatformIO6.1.19/native1.2.1.
Real native builds use IDF5.5.5 at C:/pio/packages/framework-espidf (manifest
3.50505), managed Python3.12.10 at C:/pio/penv/.espidf-5.5.5/Scripts/python.exe,
Xtensa GCC14.2.0 esp-14.2.0_20260121, managed CMake4.0.3 and Ninja1.13.1.
The existing managed environment lacks the standalone-IDF constraints cache;
IDF_PYTHON_CHECK_CONSTRAINTS=no selects installed-requirements checking, not
skipping Python dependency checks. No second Core or new SDK was installed.

| Check | Result | Actual evidence |
| --- | --- | --- |
| Strict Debug/Release host matrix | PASS | 61/61 registered CTests in each; unchanged core/shared CLI/console/transcript/parity, native adapter at 1000/100Hz, Arduino prior regressions and isolated consumer/version/launcher checks |
| Native application behavior | PASS | 28 registered actual-source scenarios: startup/exact takes/failure, blocked and partial output, persistent input suffixes, RX overflow before/after read and event saturation, IRQ absent/configured/setup failure, borrowed-controller denial, creation/deletion failures, uncertain raw error, active/result/startup teardown, runtime timeout and bounded FIFO completion |
| Native configuration/identity fixtures | PASS | Four cases execute production generator and actual host CMake wiring/builds; clean/dirty/untracked/no Git/unrelated parent, target sanitation, renamed directory-derived component, SDKCONFIG/lock isolation, propagated settings and incremental source identity |
| Real SDK application links | PASS | IDF5.5.5 S2/S3 configure/compile/link default and explicitly enabled illustration; saved images below. Actual symbols include app_main, native loop, shared Cli::tick, ExampleOwner::tick, core poll, native adapter/UART calls and enabled I2C creation APIs. Compile databases contain no Arduino/Wire/Serial sources |
| Renamed-root actual SDK image | PASS | Separate copied renamed-lib root builds/links on S2; project metadata identifies component renamed-lib and library0.1.0-dev.1. Its incremental source edit rebuilds identity UTC from 20261028T173222Z to 20261028T173249Z while Git/tree correctly remain unknown |
| Configuration isolation | PASS | Absolute build/p26-idf-s2/sdkconfig and build/p26-idf-s3/sdkconfig; both retain their target across the other's reconfiguration. Per-build dependencies.lock; no project-source sdkconfig/lock. CPU0, primary console NONE, HZ1000 and log maximum0 inspected; S3 secondary NONE, no S2 secondary-console choice |
| SDKCONFIG byte hashes | PASS | S2 SHA256 485063A5070A2AC0BDAC240C3FE8AE9B48F6544A19D959D1FAB65D2A45571B85; S3 66B13892A1DB0068CA4685D917058B273212A3B7D09D81F2BF34A9585E00AA38; unchanged through target/board reconfiguration |
| Source documentation | PASS | Strict Doxygen and XML visibility: 17 core structs/26 owner methods, 11 Cli/12 Console methods and three native lifecycle functions; new Python docstrings/style, local links/anchors, UTF-8/LF/whitespace and all eight vendor reference hashes |
| Version/package/ordinary consumption | PASS | No-op sync/check; 73 source-equal selected archive files, matching IDF inclusion and shipped guide links; renamed tests-OFF core/CLI C++17 consumers beneath unrelated Git compile/link/run with Git/Python/Doxygen/PIO absent from child PATH; ordinary standalone CMake/native PlatformIO consumers also pass |
| Sanitizers/other compilers | BLOCKED | Fresh trivial ASan/UBSan link probes exit1 for absent -lasan/-lubsan. MSVC/Clang/clang-format unavailable |
| Physical qualification | NOT RUN | UART/bridge/serial delivery, native USB operation, optional IRQ levels, CC/VBUS/VCONN/electrical behavior, main stack margin and qualified scheduling/aggregate adapter bounds; no board/port authorization or flashing |

Saved application images under ignored build/p26/artifacts:

| Target | Diagnostic defaults | Enabled compile illustration |
| --- | --- | --- |
| ESP32-S2 | 236768 bytes (0x39ce0) | 243152 bytes (0x3b5d0) |
| ESP32-S3 | 267280 bytes (0x41410) | 273600 bytes (0x42cc0) |

Enabled settings were BOARD_READY=1, MAX_OVERHEAD_US=2000,
ACCEPT_SDK_CLEANUP=1, IRQ=6, ENABLE=7 and RUN_MS=1000. They prove linked paths,
not measured or authorized hardware readiness. The native guide records exact
part supply, ADDR/enable, PORT/DIR, resistor-coupled VBUS_DET and 322I autonomous
VCONN prerequisites, rechecked against archived TI tables. No access rule or
silicon-write capability was expanded.

Actual lead/native integration commands:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Debug -R 'tusb32x_idf_app|tusb32x_idf_example_config|tusb32x_idf_transport|tusb32x_cli|tusb32x_console|cli_transcript|cli_contract' --output-on-failure
python -m py_compile scripts/idf_identity.py tools/test_idf_example.py
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python build/p26-final-check.py
python build/p26-inspect.py
cmake -S examples/host/basic -B build/host-basic -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/host-basic --config Release
& build/host-basic/tusb32x_host_basic.exe
scripts/pio.cmd run -d examples/host/basic -e native -t exec
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-p26.tar.gz
python build/p26-package.py

g++ -std=c++17 -fsanitize=address build/p20-sanitizer.cpp -o build/p26-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p20-sanitizer.cpp -o build/p26-ubsan.exe
git diff --check
```

The inspected session-only native helper sets the exact installed SDK/Python/
CMake/Ninja/toolchain environment, then invokes the actual IDF frontend. For each
target the command is Python C:/pio/packages/framework-espidf/tools/idf.py with
`-C examples/esp_idf/basic -B <absolute-build> -DSDKCONFIG=<absolute-build>/sdkconfig
-DIDF_TARGET=esp32s2` (or esp32s3), plus explicit default/illustrative cache values
and `build`. Exact reusable invocations actually run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File build/p26/run-idf.ps1 -Target esp32s2
powershell -NoProfile -ExecutionPolicy Bypass -File build/p26/run-idf.ps1 -Target esp32s3
powershell -NoProfile -ExecutionPolicy Bypass -File build/p26/run-idf.ps1 -Target esp32s2 -Qualified
powershell -NoProfile -ExecutionPolicy Bypass -File build/p26/run-idf.ps1 -Target esp32s3 -Qualified
powershell -NoProfile -ExecutionPolicy Bypass -File build/p26/run-idf.ps1 -Target esp32s2 -SourceRoot C:/Users/Honza/Documents/Projects/TUSB32x/build/p26-renamed/renamed-lib -BuildDirectory C:/Users/Honza/Documents/Projects/TUSB32x/build/p26-idf-renamed-s2
```

That process-scoped invocation changes no persistent execution policy. Checked
logs are build/p26/logs/idf-{s2,s3}-{default,enabled}-final.log and
idf-renamed-s2-{final,incremental}.log. The helper and XML/package/inspection
scripts remain ignored local evidence, never consumer dependencies. Required
available checks pass; the two sanitizer availability probes fail as recorded.
Native links and recording SDK tests are distinct from hardware/serial proof.
Final Git commit/upstream equality and clean-state evidence belongs in the
handoff and next progress update, avoiding a self-referencing commit-ID loop.
Prompt27 remains unstarted.

## Prompt 27 plain C++ and generic owner integration (2026-10-28)

Started clean after fetch at synchronized
`9e4b08a62303dbb9a6bbbfdbacb6aeb42154533c`, on the existing
`promptset/tusb32x-implementation-20261023` branch. Read full27, current rules,
COMMON, operation/result/time and ownership contracts, actual public declarations
and existing owner/consumer registrations. Fresh FieldCore-node, LDC1614 and
TMP1x2 revisions, clean state, precise source/test/runner/AGENTS paths and adopted
or rejected conventions are in [execution27](audit/execution-27-peers.md).
No sibling edits/builds, remote CI or hardware activity occurred.

Two coding agents owned disjoint example/fixture files; an independent reviewer
checked peer evidence, fairness, effects and lifetime. The lead inspected actual
sources/diffs, integrated registration and documentation and ran final checks.
The fixture's bus/context declaration order was corrected to outlive its two
drivers, with explicit bus-silent end/result-delivery assertions. Existing
fairness/recovery/uncertain-write matrices were reused. No core behavior/API,
public test hook, framework/product dependency or stable version changed.

Tools queried and used: host GCC15.1.0 (MinGW-W64 x86_64 UCRT/MCF), CMake and
CTest4.0.1, Ninja1.12.1, Python3.12.10, Doxygen1.13.2 and the installed managed
PlatformIO Core6.1.19 with native platform1.2.1. No SDK/tool installation or pin
change was needed. Compiler runtime DLLs are ordinary runtime dependencies, not
consumer dependencies on maintainer tools.

| Check | Result | Actual evidence |
| --- | --- | --- |
| Strict full native Debug/Release | PASS | 62/62 registered CTests each; assertions active; all prior suites retained plus the real two-TU shipped host example |
| Generic public owner fixture | PASS | 5,487 checks, zero failures; two targets, finite per-turn budgets, released callback gate/buffers, peer fairness, stale tokens, retained-result fences, cancellation before/after a possible write, external invalidation and caller-selected reconcile/recovery |
| Borrowed lifetime and teardown | PASS | Bus declared before borrowers; explicit end while context remains alive, retained CANCELLED/PARTIAL evidence delivered by exact token, no callback or retained lock during end/take/destruction |
| Runnable ordinary C++ example | PASS | Tests-OFF CMake and managed native PlatformIO build/link/run; simulated initialization, apply, copied port publication, local reset notification, unrelated raw read and separate read-only reconcile/refresh, explicit end; always-active checks and nonzero exit on failure |
| Standalone optional tests | PASS | Final review found duplicate creation of the host target when tests were explicitly ON; the example now reuses that actual target and enables discovery at its project root: 62 discovered, both affected host/operation CTests pass |
| Delivery versus operation outcome | PASS | Host requires delivery OK with CONFIGURATION_MISMATCH payload and NONE effect after an external reset; no desired replay, raw success leaves trust/freshness invalid until separate observations |
| Independent public headers | PASS | Config, Status, TUSB32x, CommandTable and Version each compile as their own translation unit during the strict build; compile contracts are not registered as build-only behavior |
| Package selection and isolation | PASS | 74 files source-equal to the archive, matching IDF include list, no vendor/reference/audit/test/build payload and all shipped guide links resolve; new integration guide is included |
| Extracted ordinary/CLI consumers | PASS | Renamed library beneath unrelated Git parent, tests OFF, all core/consumer TUs compile as C++17 despite enclosing C++11 setting; build/run with Git/Python/Doxygen/PlatformIO absent from child PATH |
| Extracted two-owner fixture | PASS | Same test_owner_scheduling.cpp and scripted support copied beside extracted core, tests OFF; strict build/link/run with 5,487 checks, no private core header path, framework/sibling dependency or maintainer tools on child PATH |
| Documentation/version consistency | PASS | Strict Doxygen and visible public XML, Python docs/style, local links/anchors, stable no-op version sync/check, eight unchanged vendor hashes, UTF-8/LF/whitespace and git diff --check |
| Host sanitizer availability | BLOCKED | Fresh ASan/UBSan link probes fail: installed MinGW toolchain lacks -lasan/-lubsan; cl, clang++ and clang-format unavailable |
| Framework firmware links | NOT RUN | Prior stage evidence is preserved; no framework application/core source changed in this ordinary-consumer stage |
| Physical/serial/Type-C qualification | NOT RUN | Simulation cannot establish SDK aggregate waits/cleanup, real bus timing, supply/reset/IRQ/CC/VBUS, serial behavior or power permission; no identified board/port was authorized |
| Remote CI | NOT RUN | Explicit initial-promptset policy reserves remote CI for prompt34 |

Actual commands run from the repository root:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Debug -R 'tusb32x_host_basic|tusb32x_operations|tusb32x_example_owner|tusb32x_example_integration|tusb32x_portable_consumer' --output-on-failure
cmake -S examples/host/basic -B build/host-basic -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/host-basic --config Release
& build/host-basic/tusb32x_host_basic.exe
scripts/pio.cmd run -d examples/host/basic -e native -t exec
cmake -S examples/host/basic -B build/p27-host-tests -G 'Ninja Multi-Config' -DTUSB32X_BUILD_TESTS=ON
cmake --build build/p27-host-tests --config Release --target tusb32x_host_basic tusb32x_operations
ctest --test-dir build/p27-host-tests -C Release -N
ctest --test-dir build/p27-host-tests -C Release -R '^(tusb32x_host_basic|tusb32x_operations)$' --output-on-failure
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python build/p27-final-check.py
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-p27.tar.gz
python build/p27-package.py
g++ -std=c++17 -fsanitize=address build/p20-sanitizer.cpp -o build/p27-asan.exe
g++ -std=c++17 -fsanitize=undefined build/p20-sanitizer.cpp -o build/p27-ubsan.exe
git diff --check
```

The session-only XML/link/package helpers are ignored local evidence, not core
dependencies or new permanent validators. The package helper verifies the exact
archive, creates an unrelated enclosing Git repository, restricts child PATH and
invokes absolute CMake/Ninja/compiler paths for three separate tests-OFF
applications. The two-owner fixture initially could not start without compiler
runtime DLLs; its final run copies only those DLLs into a separate tool-free
runtime directory, following the existing portable-consumer harness. No delivered
source or contract was weakened. The final isolation artifact is
build/p27-clean-parent-vwvov7qc; build/p27-package.log records all three runs.

The [integration guide](integration.md) gives the concrete owner mapping, aggregate
timeout/SDK disclosures, fresh next owner time after Status-only callbacks,
conservative possible-write acceptance, same-target writer coordination, explicit
power-event invalidation and shutdown. FieldCore's richer completion/effect fields
and millisecond-scaled nowUs hook are documented without implying they survive a
Status-only bridge. Host evidence does not certify source startup or physical
Type-C. Qualified original-device recovery remains a separate application choice
after identity and stable VDD; consumer code copies no silicon sequence.

The final commit and verified push/fetch equality belong in the handoff and next
progress update, avoiding a self-referencing hash update. Prompt28 is unstarted.


## Prompt 28 strict Python sessions and evidence (2026-10-28)

Started clean after fetch at synchronized prompt27 commit
`19fcd2c37837ef823fd852581d4cfe42b6bb8d5c`, on the same execution branch
`promptset/tusb32x-implementation-20261023`. Read the full prompt, current rules,
machine-record/runner contracts and actual CLI schema/output/transcript sources.
Fresh TMP1x2/24Cxx/LDC1614 revisions, clean state, exact code/test/AGENTS/runner
paths and adopted/rejected choices are in [execution28](audit/execution-28-peers.md).
No tracked workflow definitions exist here; remote CI remains prompt34 only.
No sibling edit/build, physical serial open, hardware action or flashing occurred.

The lead integrated three disjoint coding/audit assignments and a strong independent
framing/deadline/ownership review. Findings were checked against actual sources
and live C++ records: per-write lateness, buffered terminal framing, coherent wrong
tokens, missing profile sources, unknown identity, restoration readback and saved
capture/teardown failures now reject. The admission's host deadline and response
allowances persist through later waits/queries; result delivery remains independent
of the original payload. The existing one-request validator moved into the pure
contract module and is shared by Session and the transcript, not duplicated.

Tools used: Python3.12.10, CMake/CTest4.0.1, Ninja1.12.1, GCC15.1.0 MinGW UCRT/MCF,
Doxygen1.13.2 and installed managed PlatformIO6.1.19/native1.2.1. Optional pyserial3.5
is already installed in host Python; the pin was audited against the official
[release](https://pypi.org/project/pyserial/3.5/) and
[API](https://pyserial.readthedocs.io/en/latest/pyserial_api.html). Tests require
no pyserial or wall-clock sleeps. No new Core, package installation or SDK was needed.

| Check | Result | Actual evidence |
| --- | --- | --- |
| Fake serial/session/artifact tests | PASS | 87 unittest cases, including 26 dedicated artifact cases; independently schema-valid reactive peer, injected monotonic time and temporary on-disk reports |
| Protocol/ownership faults | PASS | Fragmented CRLF/ANSI noise, stale/echo-only markers, required/duplicate/unknown fields, all four profile sources, wrong/coherent/reused/foreign tokens, old-origin interleaving and repeated query copies, invalid commands, exhaustion, boot/loss and identity changes |
| Deadline/capture faults | PASS | Late/expired/short writes, late reads/slow fragments, immutable admission deadline across wait/query, response byte/record allowances across interleaved queries, line/noise/read/capture/tail caps and exact boundaries |
| Outcomes/local cleanup | PASS | Failed owned payload remains failed under delivery OK and primary/cleanup PASS; checkpoint before mutations, verified local values, no hardware apply during restoration, lost-session refusal, close/disconnect/capture errors and saved Ctrl-C130 |
| Atomic artifacts | PASS | Exclusive transcript, bounded tail/report/checks, short/failed disk writes, replace/fsync faults, intact previous files, removed temporary files, JSON authoritative if summary replacement fails, missing proof/incomplete/uncertainty prevents PASS |
| Actual compiled C++ session exchange | PASS | Existing cli_transcript registration retains its 248-request transcript/damaged-record checks and adds two Session exchanges: marker/version, local staging, bind/read-only initialize, original completion, repeat query, exact take and verified restoration/end; actual fixture I2C_TIMEOUT preserves signed INT32_MIN under delivery OK |
| Focused registered checks | PASS | hil_runner, cli_transcript, cli_contract and cli_contract_negative all execute with finite timeouts and an explicit built C++ fixture path |
| Full strict native matrix | PASS | 63/63 registered CTests in Debug and Release; five independent public-header TUs compile and assertions remain active in Release; all prior registrations retained |
| Python/documentation | PASS | Six affected Python modules compile, all module/function/class docstrings and 100-column style; strict Doxygen/XML public visibility, local links, UTF-8/LF/whitespace and git diff --check |
| Metadata/package isolation | PASS | Stable version sync/check/no-op and eight unchanged vendor hashes; 75 source-equal exported files with matching IDF selection and resolvable links, new guide included while runner/audit/test machinery stays excluded |
| Tests-OFF consumer linkage | PASS | Renamed extracted host/core, shared CLI and public two-owner fixture build/run beneath unrelated Git parent with Git/Python/Doxygen/PlatformIO absent from child PATH; only compiler runtime DLLs preserved |
| Real serial/hardware/SDK timing | NOT RUN | No port/board selected; opening may reset a board and has no portable deadline. DTR/RTS/open/read/write/close behavior still needs physical backend qualification; no serial/USB/IRQ/CC/VBUS/power claim |
| Sanitizers/other compilers | BLOCKED | Prior fresh MinGW probes lack ASan/UBSan link runtimes; cl/clang++/clang-format unavailable. Core/firmware sources are unchanged and SDK images were not rerun for this Python layer |

Actual commands:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug -R 'hil_runner|cli_transcript|cli_contract' --output-on-failure
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Debug -N
python tools/test_tusb32x_hil_runner.py
python tools/test_cli_transcript.py --host build/native/test/Debug/tusb32x_cli_host.exe
python tools/tusb32x_hil_runner.py --help
python -m py_compile tools/tusb32x_hil_runner.py tools/tusb32x_hil_artifacts.py tools/test_tusb32x_hil_runner.py tools/test_tusb32x_hil_artifacts.py tools/tusb32x_cli_contract.py tools/test_cli_transcript.py
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p10-docs.py
python build/p27-final-check.py
$env:PYTHONIOENCODING = 'utf-8'
scripts/pio.cmd pkg pack . --output build/TUSB32x-p28.tar.gz
python build/p28-package.py
git diff --check
```

An additional session-only AST/style check inspected all six affected Python
modules for complete docstrings and 100-column formatting. XML/link/package/text
helpers remain ignored local evidence, not new permanent guard implementations.
The package fixture is build/p28-clean-parent-unsv2rkj; build/p28-package.log
records its three ordinary applications. Reports in fake tests use temporary
paths and make no invented hardware success claim. The session entry point itself
is currently read-only; complete observation/fixture command-line plans are later
prompts. Main saves primary, cleanup, residual uncertainty and per-check evidence
with exits 0 complete PASS, 1 failure, 2 blocked/incomplete and 130 interruption.

The final commit/push/fetch equality and clean-tree evidence belong in the handoff
and next progress update, avoiding a self-referencing hash loop. Prompt29 remains
unstarted; no remote CI or serial hardware completion is claimed.

## Independent prompt 27/28 audit (2026-10-28)

Entry: clean synchronized `e1e79e6fb440fe2c3eb56d2df215b46722dcded3`, existing
`promptset/tusb32x-implementation-20261023` branch. The
[fresh review](audit/review-27-28.md) records full original-prompt coverage, peer
revisions/dirty state/paths and reproduced gaps. Prompt27 required no correction;
prompt28 required staging/restoration, mandatory proof, parser rejection, early
deadline and late-checkpoint/artifact fixes. Tests use injected monotonic time,
reactive fake serial, real files and the actual persistent C++ fixture. No physical
serial port, sibling build/edit, flashing or remote CI was used.

Tools rechecked: Python3.12.10, CMake/CTest4.0.1, Ninja1.12.1, GCC15.1.0 MinGW
UCRT/MCF, Doxygen1.13.2 and managed PlatformIO6.1.19/native1.2.1. Optional
pyserial3.5 remains pinned; its official release and API were reread, including
finite read/write settings and OS-dependent DTR/RTS pulses on open. No tool or
dependency was installed. Version truth remains 0.1.0-dev.1.

| Check | Result | Executed evidence |
| --- | --- | --- |
| Fresh independent ordinary consumer | PASS | Separate tests-OFF Release configure/build/run in build/re27-host, ordinary callbacks and simulated values |
| Public owner and headers | PASS | Fresh Debug operations/host/header targets in build/re27-owner; operations/host CTest 2/2, 5,487 owner checks, independent public includes |
| Isolated portable consumer | PASS | 369 checks with explicit failure/allocation sentinels; no private headers or maintainer tools on child PATH |
| Session and artifacts | PASS | 104 unittest cases, including 30 artifact cases; original 87 cases remain enabled |
| Actual C++ session exchange | PASS | Existing single cli_transcript runs normal/failed-payload sessions and malformed known-route parser rejections; original 248-request transcript retained |
| Strict complete native regressions | PASS | Debug 63/63 and Release 63/63 CTests, Release assertions active; four affected registrations repeated after the last session fixes |
| Package isolation | PASS | 75 source-equal files; manifest/IDF selection and shipped links match; renamed tests-OFF host, shared CLI and public-owner consumers build/link/run beneath unrelated Git without Git/Python/Doxygen/PlatformIO on child PATH |
| Version and source docs | PASS | Generator sync/check, repeated no-op sync preserving bytes/mtime; strict Doxygen/XML public visibility; Python docstrings and 100-column style |
| Repository consistency | PASS | Local Markdown links/anchors, eight vendor SHA-256 hashes, changed UTF-8/LF/final newline, git diff --check |
| ASan/UBSan and other host compilers | BLOCKED | Fresh trivial link cannot find -lasan/-lubsan; cl, clang++ and clang-format are absent |
| Physical serial/Type-C/IRQ/VBUS | NOT RUN | No identified/authorized port, board, fixture, trace or hardware action; simulations prove software only |

Commands executed from the repository root (Ninja Multi-Config for build/native):

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
python tools/test_tusb32x_hil_runner.py
ctest --test-dir build/native -C Debug -R "^(hil_runner|cli_transcript|cli_contract|cli_contract_negative)$" --output-on-failure
ctest --test-dir build/native -C Release -R "^(hil_runner|cli_transcript|cli_contract|cli_contract_negative)$" --output-on-failure
cmake -S examples/host/basic -B build/re27-host -G Ninja -DTUSB32X_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/re27-host
build/re27-host/tusb32x_host_basic.exe
python scripts/generate_version.py sync
python scripts/generate_version.py check
scripts/pio.cmd pkg pack . --output build/TUSB32x-re27-28.tar.gz
python build/re27-28-package.py
doxygen Doxyfile
python build/p17-doxygen.py
python build/p27-final-check.py
python build/re27-28-checks.py
python build/p10-docs.py
git diff --check
```

The build/ Python files are session-only local verification helpers, not shipped
tools or future guards. Package artifacts/logs are build/TUSB32x-re27-28.tar.gz,
build/re27-28-package.log and build/re27-28-clean-parent-uh2a1ql6. The helper checks
all three extracted consumers with strict warnings and tests OFF; the owner still
runs its real public API's 5,487 checks. Framework images were not rebuilt for
host Python fixes; earlier links are not claimed as new-commit firmware evidence.
Remote CI remains prompt34 only. Commit/push/fetch equality is verified in the
handoff and the next progress update, avoiding a self-referencing hash loop.

## Prompt 29: observation plans and native integration (2026-10-28)

Entry: clean synchronized `aca463158485d13fe8d4437e1962dda4a4c32516`, same
`promptset/tusb32x-implementation-20261023` branch. Fresh sibling revisions, dirty
state, exact sources/AGENTS/workflows and choices are in
[execution29](audit/execution-29-peers.md). Full original29, actual CLI schemas,
core observation/access and retained-result tests were inspected. No remote CI,
sibling edits/builds, hardware open, power control or flashing occurred.

Tools rechecked: Python3.12.10, CMake/CTest4.0.1, Ninja1.12.1, GNU15.1.0 MinGW
UCRT/MCF, Doxygen1.13.2, managed PlatformIO6.1.19/native1.2.1. No tool/dependency
installation or version bump; library remains 0.1.0-dev.1. Actual hardware serial
still needs optional pinned pyserial3.5, but tests/dry-run do not import it.

| Check | Result | Actual evidence |
| --- | --- | --- |
| Planning/invocation | PASS | 26 cases: exact pairs/types/ranges/suites, checked span, finite run/stage/command limits, malformed expected identity/port/numbers, missing port, output failures and interrupted dry-run close |
| Complete runner/session/artifacts | PASS | 161 cases with reactive serial and injected clock/wait; no wall-clock sleeps or hardware dependency; existing session/artifact tests retained |
| Readonly/soak behavior | PASS | Full plans, detached fresh samples, missing desired rejection/all verify aliases with retained desired, zero/one callback budgets, mapped raw counts, negative attribution/counter/effect/freshness tests, lost sessions and local restoration |
| Result and timing boundaries | PASS | Exact owned take, original versus copied result, failed payload, BUSY take no retry, foreign-result denial, run/stage/operation deadlines, clipped explicit firmware timeouts, nonadvancing wait and late final save |
| Actual persistent C++ | PASS | 248 original requests/damaged records, two strict sessions, three full readonly plans (normal/I2C_TIMEOUT/BUSY take) and two controlled owner-turn sessions; zero data writes and preserved core result/state are asserted |
| Native regression/build | PASS | Debug 63/63 and Release 63/63 CTests, strict warnings and active Release assertions; four affected registrations repeated after the final Python fixes |
| Help and real dry-run entry | PASS | --help exit0; explicit readonly LAI/0x47 into build/p29-readonly-final, expected exit2, primary/hardware NOT RUN, finite concrete plan and no firmware identity/port traffic |
| Package boundary | PASS | 76 source-equal files, matching IDF selection, shipped relative links; renamed extracted host/shared-CLI/public-owner consumers build/link/run beneath unrelated Git with tests OFF and Git/Python/Doxygen/PlatformIO absent from child PATH |
| Documentation/metadata | PASS | Generator sync/check and repeated no-op bytes/mtime; Doxygen/XML visibility; eight Python modules' complete docstrings/100-column style; local links, eight vendor hashes, UTF-8/LF and diff whitespace |
| Sanitizers/other compilers | BLOCKED | Installed MinGW lacks ASan/UBSan runtimes as freshly probed during prior27/28 audit; cl/clang++/clang-format remain absent; no unsupported sanitizer claim |
| Physical serial/I2C/CC/IRQ/VBUS/VCONN | NOT RUN | No identified/authorized physical port, target or trace; native modeled counters/attachment do not qualify a connector, waveform, supply or adapter timing |

Commands executed from the repository root:

```text
python tools/test_tusb32x_observation.py
python tools/test_tusb32x_hil_runner.py
python tools/test_cli_transcript.py --host build/native/test/Debug/tusb32x_cli_host.exe
python tools/tusb32x_hil_runner.py --help
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite readonly --out build/p29-readonly-final
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Debug -R "^(hil_runner|cli_transcript|cli_contract|cli_contract_negative)$" --output-on-failure
ctest --test-dir build/native -C Release -R "^(hil_runner|cli_transcript|cli_contract|cli_contract_negative)$" --output-on-failure
python scripts/generate_version.py sync
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p29-final-check.py
python build/p10-docs.py
scripts/pio.cmd pkg pack . --output build/TUSB32x-p29.tar.gz
python build/p29-package.py
git diff --check
```

The build/ scripts are session-only checks, not shipped future validators. Package
isolation logs identify the extracted directory and strict compiler arguments;
the public-owner fixture executes its existing 5,487 checks. Native observation
mode controls simulated owner turns/output drain without changing firmware grammar
or the default fixture mode. Output-full take tests retain actual core result
evidence, not fabricated printed success. Wire-volume review motivated explicit
timeouts and automatic port polling, but does not establish physical serial timing.
Unchanged framework images were not rebuilt for these Python/test/documentation
changes; earlier firmware links are not claimed as final-commit evidence. Commit,
push and final fetched upstream equality are verified in the handoff and recorded
in the next progress update. Prompt30 remains unstarted.

## Prompt 30 fixture plans and bounded physical evidence (2026-10-28)

Entry HEAD `1f0fb7604ad1696aea9f3067c320d3b3c2fca89e` was clean and fetched
equal to upstream on `promptset/tusb32x-implementation-20261023`. Fresh peer
revisions/dirty states, exact inspected files, current workflow/AGENTS conventions
and archived TI tables are in [execution30](audit/execution-30-peers.md).
Only this repository was edited. No serial port, flash, board-power operation,
physical fixture or remote CI was invoked.

Tools: Python3.12.10, CMake/CTest4.0.1, Ninja1.12.1, GNU15.1.0
MinGW-W64 x86_64-ucrt-mcf-seh, Doxygen1.13.2, Git2.52.0.windows.1,
VS-Code-managed PlatformIO Core6.1.19/native1.2.1. No dependencies/pins changed.
The optional serial dependency remains pyserial3.5; tests need no pyserial.

| Layer | Result | Actual evidence/limits |
| --- | --- | --- |
| Fixture/setup/operator/session software | PASS | 225 Python tests in the single hil_runner registration: prior161 plus33 fixture,18 setup/console and13 invocation cases; fake serial/clock, no wall sleeps or hardware |
| Four complete native fixture plans | PASS | Persistent C++ shared CLI/core with launch-only independent physical inputs; actual apply/verify/IRQ W1C effects, exact result takes, conditional cleanup; all modeled physical hardware explicitly NOT RUN |
| Prior native regressions | PASS | Full Debug63/63; full final Release63/63 with strict warnings and active assertions; final affected Debug10/10 after the last corrections |
| CLI entry/help/dry-run | PASS | --help exit0; all four explicit LAI/0x47 fixture suites saved under build/p30-fixtures-verified; actual Python exit2 and primary/hardware NOT RUN, no serial I/O, no raw register list falsely advertised |
| Ordinary consumer | PASS | Fresh separate tests-OFF Release configure/build/run; meaningful simulated owner assertions, no framework/sibling paths or maintainer requirement |
| Package isolation | PASS | 76 source-equal exported files, matched IDF selection; renamed extracted host/shared-CLI/public-owner consumers beneath unrelated Git with Git/Python/Doxygen/PlatformIO absent from child PATH; owner5487 checks/0failures |
| Documentation/metadata | PASS | Version sync/check/no-op stability, strict Doxygen/XML public visibility,13 Python modules' full docstrings/100-column style,133 Markdown files/810 links, vendor hashes, UTF-8/LF and diff whitespace |
| Sanitizers/other compilers | BLOCKED | Fresh address/undefined probe links fail because installed MinGW lacks -lasan/-lubsan; cl/clang++/clang-format absent. No sanitizer claim or weakened checks |
| Physical serial/I2C/CC/INT/current/VBUS/VCONN/supply | NOT RUN | No exact authorized port/target/fixture or physical traces; operator declarations/model bytes are not electrical qualification |

Commands executed from the repository root:

```text
python tools/test_tusb32x_hil_runner.py
python tools/test_tusb32x_fixture_setup.py
python tools/test_tusb32x_fixture_invocation.py
python tools/test_tusb32x_fixtures.py
python tools/test_cli_transcript.py --host build/native/test/Debug/tusb32x_cli_host.exe
python tools/test_cli_transcript.py --host build/native/test/Release/tusb32x_cli_host.exe
python tools/tusb32x_hil_runner.py --help
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite orientation --suite accessory --suite role-current --suite interrupt --run-timeout 1800 --out build/p30-fixtures-verified
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Debug -R "^(tusb32x_protocol|tusb32x_operations|tusb32x_port|tusb32x_diagnostics|tusb32x_cli|tusb32x_console|cli_transcript|hil_runner|cli_contract|cli_contract_negative)$" --output-on-failure
cmake -S examples/host/basic -B build/p30-host-basic -G Ninja -DCMAKE_BUILD_TYPE=Release -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/p30-host-basic
build/p30-host-basic/tusb32x_host_basic.exe
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p30-final-check.py
python build/p10-docs.py
scripts/pio.cmd pkg pack . --output build/TUSB32x-p30.tar.gz
python build/p30-package.py
git diff --check
```

The build/ helpers are session-only checks, not future distributed validators.
hil_runner's finite timeout is90s for expanded atomic-artifact coverage; the
existing cli_transcript remains30s and receives the explicit built fixture path.
No new registration, physical test trigger or firmware command was introduced.

Review found and corrected concrete capability, source-voltage inheritance,
accessory-mode, INT-preflight, raw/profile validity, operator freshness, base
reset-settling and mixed-group evidence defects. Three newly added negative
assertions initially failed because their test hook changed only a copied result;
the strict Session correctly preserved the original token. Fault injection was
moved to original/retained payload construction, preserving the assertions.
Both focused consistency regressions and final225-test registration then passed;
final Release was rerun to63/63 and affected Debug suites to10/10. No contract or
assertion was weakened. Successful mutation needs confirmed effect evidence;
reset also needs COMPLETED. CodeOK cannot conceal uncertain effects.

Device cleanup is success-only and separately evidenced; local restoration cannot
erase blocked hardware cleanup. Setup and every source-capable candidate retain
supply/partner/target assumptions; base current changes explicitly reset and save
the settled post-reset proof. TUSB322I mutation plans remain gated before I2C.
Unchanged framework firmware was not rebuilt for host tooling/test/docs changes;
earlier linked images are not final-commit or hardware proof. The lead commits,
pushes, fetches and verifies equal upstream commits, zero ahead/behind and no
uncommitted task files before handoff. Record that hash in the next progress
update. Prompt31 remains unstarted.

## Independent audit of prompts 29 and 30 (2026-10-28)

Entry HEAD `770e4f6bb892de2df72fb712693d90759ca7c6cd` was clean and fetched
equal to upstream on `promptset/tusb32x-implementation-20261023`. Fresh peer
revisions, exact sources, delegated scopes and confirmed defects are recorded in
the [progress audit](implementation-progress.md#independent-audit-of-prompts-29-and-30-2026-10-28).
The lead reviewed actual returned diffs and behavior. No sibling edit, hardware
open, power control, flashing, framework rebuild or remote CI occurred.

Tools: Python3.12.10, CMake/CTest4.0.1, Ninja1.12.1, GNU15.1.0 MinGW-W64
x86_64-ucrt-mcf-seh, Doxygen1.13.2, Git2.52.0.windows.1 and managed PlatformIO
Core6.1.19/native1.2.1. No package pin, library version or dependency changed.
Optional pyserial remains3.5; deterministic tests need no pyserial.

| Check | Result | Actual evidence |
| --- | --- | --- |
| Deadline regressions | PASS | Delayed Session construction and legal fixture-then-soak duration cases failed before correction; absolute run/transition boundaries now reject expired true/false replies without authorizing later writes |
| Evidence/staging regressions | PASS | Missing/contradictory CONFIG raw/field proof, ignored/invalid staged candidates and unrequested post-apply field changes prevent PASS; exact IRQ staging difference is proved before ACK |
| Extension compatibility | PASS | Changing unknown CONFIG/PROFILE/RESULT extension telemetry is retained without becoming required semantic evidence; known fields still use strict Session comparison |
| Runner/session/artifacts | PASS | 237 deterministic Python cases through the existing hil_runner registration; injected serial/clock, no physical dependency or wall-clock sleeps |
| Native persistent CLI | PASS | Existing248 requests, two strict sessions, three readonly plans, two owner-turn sessions and four full fixture plans retained; two new wrong orientation/current expectations fail against independent literal bytes, save failed evidence and prohibit automatic hardware cleanup |
| Native regression/build | PASS | Final Debug63/63 and Release63/63 CTests, strict warnings and active Release assertions; finite existing runner/transcript timeouts and explicit built fixture paths unchanged |
| CLI/dry-run | PASS | --help exit0; readonly LAI/0x47 and all four fixture suites save fresh reports under build/reaudit2930-readonly and build/reaudit2930-fixtures; actual subprocess exit2 and primary/hardware NOT RUN |
| Ordinary/package consumers | PASS | Fresh tests-OFF Release host configure/build/run;76 source-equal exported files; renamed extracted host/shared-CLI/public-owner consumers beneath unrelated Git build/run without Git/Python/Doxygen/PlatformIO on child PATH; owner5487 checks/0failures |
| Documentation/metadata | PASS | Version check and repeat no-op sync bytes/mtime, strict Doxygen/public XML,13 Python modules' full docstrings/100-column style, local links/vendor hashes, UTF-8/LF and diff whitespace |
| Sanitizers/other compilers | BLOCKED | Fresh minimal address/undefined link probes fail for missing -lasan/-lubsan in installed MinGW; cl/clang++/clang-format absent; no suppressed tests or sanitizer claim |
| Physical serial/I2C/CC/INT/current/supply/VBUS/VCONN | NOT RUN | No identified authorized target/port/fixture or electrical traces; literal model inputs and operator declarations do not qualify hardware or backend aggregate timing |

Commands executed from the repository root:

```text
python tools/test_tusb32x_observation.py
python tools/test_tusb32x_fixture_invocation.py
python tools/test_tusb32x_fixtures.py
python tools/test_tusb32x_hil_runner.py
python tools/test_cli_transcript.py --host build/native/test/Debug/tusb32x_cli_host.exe
python tools/tusb32x_hil_runner.py --help
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite readonly --out build/reaudit2930-readonly
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite orientation --suite accessory --suite role-current --suite interrupt --run-timeout 1800 --out build/reaudit2930-fixtures
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
cmake -S examples/host/basic -B build/reaudit2930-host -G Ninja -DCMAKE_BUILD_TYPE=Release -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/reaudit2930-host
build/reaudit2930-host/tusb32x_host_basic.exe
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p30-final-check.py
python build/p10-docs.py
scripts/pio.cmd pkg pack . --output build/TUSB32x-p30.tar.gz
python build/p30-package.py
git diff --check
```

The build/ helpers are local session checks, not new distributed validators.
Package isolation artifacts are in build/p30-clean-parent-0cmxieba. Fresh minimal
ASan/UBSan probes used g++ with -fsanitize=address/undefined and failed at link
before any instrumented executable existed. The Python registration increased
from225 to237 cases without a second suite or result store. Both native mismatch
cases assert exact PlanFailure, unchanged literal raw evidence, saved FAIL/exit1
and blocked cleanup/residual uncertainty, not a generic process failure. Hardware
and unchanged firmware links remain separate evidence layers. The lead commits,
pushes and fetches to verify upstream equality, zero ahead/behind and a clean task
worktree before handoff. Prompt31 remains unstarted.

## Prompt 31 reset, fault and console suites (2026-10-28)

Started clean at synchronized `b8eb52fa52e800fe3b53d33f4e53384d37f37544` on
`promptset/tusb32x-implementation-20261023`. Fresh peer revisions/dirty state,
exact source/test/CLI/runner/workflow paths and adopted/rejected choices are in
[execution31](audit/execution-31-peers.md). Lead and independent reviewers checked
the actual returned code/diff and archived TI requirements. No sibling edit,
physical serial, flashing, supply/fixture action or remote CI occurred.

Tools: Python3.12.10, CMake/CTest4.0.1, Ninja1.12.1, GNU15.1.0 MinGW-W64
x86_64-ucrt-mcf-seh, Doxygen1.13.2, Git2.52.0.windows.1, managed PlatformIO
Core6.1.19/native1.2.1. The optional pyserial3.5 pin and library0.1.0-dev.1 are
unchanged; fake-serial/native tests do not require pyserial.

Re-read archived SLLSEN9F7.4.3p14 (qualified base recovery) and7.3.1.2p11
(explicit current refresh reset), SLLSEQ8D Tables10/11 pp19-20 (separate controls),
SLLSEO5D6.3.2p9 (reset establishes DRP without all-POR proof),5.3p4/5.5p6/
6.3.6p13 (supply, VCONN min/typ/max and sensing), and SLAAE24p7 (back-powering).
Archived source hashes remain matched by the vendor provenance checks.

| Layer | Result | Actual evidence and bounds |
| --- | --- | --- |
| Complete runner/session/artifacts | PASS | 306 Python cases via the single existing hil_runner;69 recovery cases, fake serial/clock, no physical I/O or wall sleeps; final306-case Debug56.91s/Release56.98s under90s |
| New plans and all-nine audit | PASS | Complete reset LAI/320/320I, fault and console fake plans, all exact prerequisites/action/transition failures; all nine mixed suites through real main preserve one deadline, each check and bounded report metadata |
| Outcome/effect guards | PASS | Expectation fixed before original admission/completion; default/nonmatching failures stay sticky, signed detail/uncertain effect stay raw; host expiry, late success, cap loss, exception/Ctrl-C and missing/failed cleanup cannot be excused |
| Startup/trust/cleanup | PASS | Durable actual pre-recovery baseline, separate termination controls/aggregate proof, unrelated-read fence, historical reset acceptance independent of reconcile, original source power epoch blocks stale readiness and blind recovery/profile replay |
| Console ownership | PASS | Main closes/reopens only explicit fake port with identical DTR/RTS/baud/timeouts; new Session identity/target/nonces, no old token/baseline; stale/malformed/changed identity, reopen error/late return and colored/whitespace boot banners reject |
| Persistent real C++ CLI/core | PASS | Prior248 requests, sessions/readonly/four fixture plans retained; five new complete reset/fault/console plans, literal reset/recovery sequences and31/96ms modeled waits; actual two zero-write returns preserve help,64 force streamed LOSS/FAIL/unrestored evidence; original/322 reset role facts stay separate |
| Native strict regression | PASS | Debug63/63 (88.09s) and Release63/63 (87.06s), then affected runner/transcript/operations/console4/4 in Debug67.91s/Release68.01s after the final fresh-port guard; strict warnings, active Release assertions and existing30s/90s timeouts with explicit built fixture path |
| Entry/help/dry-run | PASS | --help lists all nine; three new suites for LAI/0x47 under build/p31-recovery-plan save concrete actions and actual subprocess exit2, primary/hardware NOT RUN, no port opening |
| Ordinary/package consumers | PASS | Fresh tests-OFF Release host;76 source-equal export files and explicit IDF source selection; renamed extracted host/shared-CLI/public-owner builds/runs beneath unrelated Git without Git/Python/Doxygen/PlatformIO on child PATH; owner5487 checks/0failures |
| Documentation/metadata | PASS | Generator check and repeat no-op sync bytes/mtime, strict Doxygen/public XML,14 Python modules' full docstrings/100-column style, local links/eight vendor hashes, UTF-8/LF and diff whitespace |
| Sanitizers/other compilers | BLOCKED | Fresh address/undefined probe links fail for missing -lasan/-lubsan; cl/clang++/clang-format absent; assertions/checks were preserved |
| Physical serial/USB/I2C/CC/INT/supply/VBUS/VCONN/timing | NOT RUN | No exact authorized target/port/fixture or electrical traces; operator declarations, native faults/process restarts and software clocks are explicitly modeled evidence |

Commands executed from the repository root:

```text
python tools/test_tusb32x_recovery.py
python tools/test_tusb32x_hil_runner.py
python tools/test_tusb32x_fixture_setup.py
python tools/test_cli_transcript.py --host build/native/test/Debug/tusb32x_cli_host.exe
python tools/tusb32x_hil_runner.py --help
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite reset-termination --suite fault --suite console --run-timeout 1800 --out build/p31-recovery-plan
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
ctest --test-dir build/native -C Debug -R "^(hil_runner|cli_transcript|tusb32x_operations|tusb32x_console)$" --output-on-failure
ctest --test-dir build/native -C Release -R "^(hil_runner|cli_transcript|tusb32x_operations|tusb32x_console)$" --output-on-failure
cmake -S examples/host/basic -B build/p31-host-basic -G Ninja -DCMAKE_BUILD_TYPE=Release -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/p31-host-basic
build/p31-host-basic/tusb32x_host_basic.exe
python scripts/generate_version.py check
doxygen Doxyfile
python build/p17-doxygen.py
python build/p30-final-check.py
python build/p10-docs.py
scripts/pio.cmd pkg pack . --output build/TUSB32x-p31.tar.gz
python build/p31-package.py
git diff --check
```

Session-only build/ helpers are not distributed future validators. Package
isolation is saved under build/p31-clean-parent-ty2zdx64. The existing13-module
style/docstring helper plus a separate identical AST/width/UTF-8 check cover the
new recovery module; no production guard stage was added early. ASan/UBSan probes
use g++ -fsanitize=address/undefined on a minimal main and fail before executable
creation. Registered suites are nonempty and no hardware is opened implicitly.

Initial red regressions reproduced missing pre-recovery baseline, invalid reconnect
return masking failure, benign help triggering reboot and ANSI-wrapped boot being
missed. All corrected guards pass runtime negatives. Original source-power epochs
and historical reset uncertainty received independent safety review and targeted
tests. A long native output stall initially caused genuine queue loss; the final
test retains that failed case and separately proves bounded no-loss backpressure.
No loss counter or assertion was weakened. Core/public C++ and production CLI
grammar remain unchanged. Hardware and unchanged firmware images remain separate
evidence layers; earlier links are not final-commit qualification. The lead
commits/pushes/fetches this completed unit and verifies upstream equality, zero
ahead/behind and no task changes before handoff. Prompt32 remains unstarted.

## Prompt 32: archives and clean consumers, 2026-10-28

Starting HEAD458706ae7ebedeb24d43e2e7a7f6a48732865ba5 and upstream equal on
promptset/tusb32x-implementation-20261023 after fetch, clean index/worktree.
Independent current-source audits are [peers](audit/execution-32-peers.md) and
[frameworks](audit/execution-32-frameworks.md); the lead verified their code,
returned diffs, actual image/source paths and hashes. Siblings remain unchanged.
No SDK/tool pin, production core/public API or stable Version.h changed.

| Check | Result | Actual evidence |
| --- | --- | --- |
| Archive rejection | PASS | python tools/test_check_package.py;35runtime tests, hostile names/aliases/types/collisions/PAX/sizes/counts/checksums/terminators, unchanged outside sentinels and no early payload extraction |
| Strict native Debug | PASS | cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON; cmake --build build/native --config Debug; ctest --test-dir build/native -C Debug --output-on-failure;65/65,113.26s |
| Strict native Release | PASS | cmake -S . -B build/release -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release; cmake --build build/release --config Release; ctest --test-dir build/release -C Release --output-on-failure;65/65,113.40s; checks remain active |
| Final affected registration reruns | PASS | ctest --test-dir build/native -C Debug -R tusb32x_package --output-on-failure and matching Release;2/2 each after extraction/tool-error fixes; final35-case additions rechecked below |
| Actual managed-Core archive | PASS | scripts/pio.cmd pkg pack . --output build/TUSB32x-p32-final.tar.gz;294961bytes,76regular files; all bytes match trusted literal export list, IDF list equal |
| Clean copied package consumer | PASS | python tools/check_package.py --archive build/TUSB32x-p32-final.tar.gz --out C:/Temp/tusb32x-p32-delivery; renamed root, all public headers, actual cooperative initialization, token/status/effect/snapshot checks, source-origin report and executable success |
| Child dependency boundary | PASS | Isolated PATH excludes Git/Python/Doxygen/PIO; cache confirms default tests OFF and no maintainer/test dependency discovery; C++17 propagates despite consumer requesting11; compiler inputs originate in clean workspace |
| Parent identity isolation | PASS | Lead-created temporary unrelated parent HEAD73027de1db658cd1e1e05bdd2d7ac4093e3ba2d1; another clean CMake run and all6firmware images retain own provenance boundary; all firmware Git/treeunknown |
| Standalone host example | PASS | cmake -S examples/host/basic -B build/p32-host-basic -G Ninja -DCMAKE_BUILD_TYPE=Release; build and tusb32x_host_basic.exe; simulated operations/invalidation/reconcile checks pass |
| Extracted Arduino links | PASS | Final artifact; basic and two-device environments esp32s2dev/esp32s3dev/esp32s2_owner_service/esp32s3_owner_service;4real ELF/BIN/map images |
| Extracted native-IDF links | PASS | Final artifact; genuine ESP-IDF5.5.5 S2/S3 application images, native APIs/app_main/shared CLI/core, no Arduino/Wire/Serial linkage; separate absolute build/SDKCONFIG/locks |
| Version/docs/source consistency | PASS | sync/check/no-op mtimes stable; doxygen Doxyfile plus public XML visibility; repository136Markdown/826local or own-source links/anchors before final evidence additions; final counts below; archive173local links/anchors |
| Hardware/serial/USB/CC/INT/supply/timing | NOT RUN | No target/port/fixture authorization and no physical action; simulated consumer and firmware image links are software evidence |
| Full supported remote matrix | NOT RUN | Reserved for prompt34; no remote CI dispatched, polled or required |
| Additional host toolchains/sanitizers | NOT RUN | cl/clang++/clang-format unavailable; previously probed MinGW ASan/UBSan runtimes unavailable; no new core code changed |

Final actual distribution SHA-256:
`97549728fd29eae2660630a011a0005991e39bc99ce2ad62aea703821a357316`.
The initial final-image consumer extraction is
C:/Temp/tusb32x-p32-delivery/package-9uqaz8ky/renamed-tusb32x; a separate final
CMake rerun beneath the now-existing unrelated Git parent is
C:/Temp/tusb32x-p32-delivery/package-m_09porw/renamed-tusb32x. Both come from that
same archive and all76files remain source-equal. Each private workspace retains
archive.tar.gz/archive.tar, copied consumer, build/cache/compile database,
package-consumer-origin.txt and package-report.json. No source .git, tests,
framework headers, sibling paths or generation tools are copied into it.

Six-image final evidence is C:/Temp/tusb32x-p32-final2-firmware/image-evidence.json,
with exact commands/environments/exits in arduino.json/idf-s2.json/idf-s3.json,
bounded logs, symbols and artifact/config hashes. The lead independently hashed
all18nonempty ELF/BIN/map artifacts and compared all76extracted bytes with current
source. See the [framework audit](audit/execution-32-frameworks.md) for exact
commands and per-image hashes. Each Arduino row compiled11actual core/shared/app
sources with DWARF compilation directories at the renamed root; each native
compile database has11extracted sources and native-only SDK symbols. Native
project metadata resolves component renamed-tusb32x by actual directory name.
All6identity headers have unknown Git/tree, not the unrelated enclosing HEAD.
S2/S3 configuration, affinity,console NONE,HZ1000 and distinct config hashes were
checked after both builds; no extracted source config/lock/cache was created.

Host tools: Python3.12.10, CMake/CTest4.0.1, Ninja1.12.1, GNU15.1.0 MCF/UCRT,
Doxygen1.13.2, Git2.52.0.windows.1. Existing user-managed launcher Core6.1.19
uses installed C:/pio storage; pioarduino55.03.311, Arduino3.3.11,
Arduino SDK5.5.5+sha.b774170ff46, Xtensa14.2.0 esp-14.2.0_20260121. Native
SDK package3.50505/IDF5.5.5, CMake4.0.3, Ninja1.13.1, managed Python3.12.10,
component manager2.4.11. SDK requirements check passed with the existing documented
IDF_PYTHON_CHECK_CONSTRAINTS=no setting for the managed environment's absent
standalone constraints cache; no dependency check was skipped. No new Core/SDK
was installed and no pin was changed.

Independent negative tests first reproduced concatenated trailing tar acceptance,
recursive extension overflow, semantic PAX-size header concealment and a reopened
input hash. Corrected parsing rejects unsupported semantic metadata before
interpretation, checks physical headers/padding and hashes the immutable snapshot.
Malformed DEFLATE now maps to PackageError/exit1; missing tools are invocation
exit2. Initial CTest isolation correctly exposed a nested checkout workspace;
compilation now uses an external temporary root. Red evidence is retained in logs;
final focused reruns pass without suppressing assertions or weakening contracts.

An earlier76-file artifact230c31...8e08 also completed6links. A documentation-only
candidate3883dc...5d96 was retracted and its partial builds cancelled; it supplies
no final success claim. The final975497...7316 matrix used fresh independent
outputs. Build configuration defaults remain diagnostic-only; neither successful
link nor a compile-only SDK fixture proves live I2C/console/connector operation.
No install/export/find_package infrastructure or release/tag/registry publication
was added. The lead reviews/stages/commits and syncs the checked unit on the same
execution branch and records actual synchronization in the handoff.

Final focused acceptance after the last malformed-input/exit regressions:
Debug package2/2 in2.34s and Release package2/2 in2.36s, each executing the final
35-test safety suite and a fresh archive compile/link/run. Final repository
Markdown check:136files/831local or own-source links/anchors; packaged links173.
Python AST/docstrings and100-column/UTF-8/LF/whitespace checks pass for both new
modules; Doxygen/XML and no-op version sync remain clean. Full Debug/Release
65/65 evidence above precedes only these focused extraction regressions and
excluded validation/progress text; all affected registrations were rerun.

## Independent audit of prompts 31 and 32, 2026-10-28

Re-read both original prompts in full and audited actual commits
b8eb52f..458706a and458706a..c76e101, not the previous handoff. Starting HEAD
c76e101cfc6339a48572573093233efdefb1c55d and upstream were equal after fetch;
index/worktree clean on promptset/tusb32x-implementation-20261023. Fresh strong
reviewers independently examined recovery/effects and archive/provenance, with a
separate requirements/peer/test review. The lead reproduced and reviewed both
confirmed findings. Current peer revisions/paths and adopted/rejected choices
are recorded in [progress](implementation-progress.md). Siblings remain clean.

| Confirmed finding | Correction and independent regression |
| --- | --- |
| Base fault UFP completion could carry old recovery readiness into a later source-capable suite | Checkpoint base_recovery_required before power-cycle intent; preserve it across groups/sessions; block source apply; clear only after newly qualified typed recovery. Both base variants cover separate/combined groups, fresh recovery and absent initial readiness. Native C++ processes prove zero later callbacks/commands; their retained UFP images do not claim POR behavior. |
| Source/include properties and compile_commands missed absolute outside headers | Record compiler/SDK implicit roots before adding the library; validate actual Ninja dependencies or GNU/Clang Make depfiles. Complete copied-checkout consumers reject both original-checkout and arbitrary sibling header imports specifically at that guard. |

Archived TI SLLSEN9F section7.4.3, printed page14, was re-opened: VDD return
requires qualified04/wait30ms/00 before Rp. The runner records this requirement;
it does not operate supplies or add a second operation engine. Only the existing
typed core recovery can establish new proof. Neither software reset nor fresh
identity/reconciliation/reads clear the run fact. Boolean evidence is validated.
The shared fake's absent desired mode now stays independent of mutable actual
mode, matching unavailable desired storage in the real CLI.

| Verification | Result | Actual command/evidence |
| --- | --- | --- |
| Strict Debug build and full registrations | PASS | cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON; cmake --build build/native --config Debug; ctest --test-dir build/native -C Debug --output-on-failure;66/66 in112.52s; build/audit31-32-debug-ctest.log |
| Strict Release build and full registrations | PASS | cmake -S . -B build/release -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release; cmake --build build/release --config Release; ctest --test-dir build/release -C Release --output-on-failure;66/66 in121.57s; build/audit31-32-release-ctest.log; runtime checks active |
| Session/runner/recovery suite | PASS | Existing hil_runner executes311tests in66.449s Debug/68.207s Release, including all74recovery tests and both-variant regressions; no sleeps in fake-clock parsing tests |
| Actual persistent C++ transcript | PASS | Existing cli_transcript receives built host path;12.00s Debug/17.35s Release, retains all earlier plans and adds2base power-epoch guards; modeled hardware NOT RUN |
| Archive/parser unit suite | PASS | python tools/test_check_package.py;39tests, including35archive negatives/positives and4compiler depfile parsing cases |
| Complete header-origin consumers | PASS | tusb32x_package_origins registered120s; clean copy succeeds, checkout/sibling imports fail after compile at actual dependency guard;5.46s Debug/8.69s Release |
| Additional generator coverage | PASS | Same3origin cases with Ninja Release, Ninja Multi-Config Debug and MinGW Makefiles Release; actual Make .d files exercised, not only synthetic text |
| Real managed-Core package | PASS | scripts/pio.cmd pkg pack . --output build/TUSB32x-audit31-32.tar.gz;295925bytes,76regular source-equal files; matching literal IDF/PlatformIO lists |
| Renamed clean package consumption | PASS | python tools/check_package.py --archive build/TUSB32x-audit31-32.tar.gz --out C:/Temp/tusb32x-audit31-32-distribution; actual compile/link/run, all public headers, tests OFF, no Git/Python/Doxygen/PIO on child PATH, actual used-header origins |
| Extracted framework images | PASS | Four Arduino basic/owner S2/S3 plus2genuine native-IDF CLI images from fresh outputs; exact commands/hashes in framework audit |
| Stable metadata/documentation | PASS | Version sync/check and no-op byte/mtime stability; doxygen Doxyfile +17public struct/26owner method XML visibility;136Markdown files/831local or own-source links before final audit links; final count below;173package links |
| Changed Python modules | PASS | All5modules parse, every function/class has docstrings;100columns, UTF-8/noBOM/LF/final newline/no trailing whitespace; scoped Git diff checks |
| Tests-OFF ordinary host example | PASS | cmake --build build/p32-host-basic --config Release and tusb32x_host_basic.exe; simulated owner/invalidation/reconciliation checks pass; initial CMake fixture configured Release/tests OFF |
| Physical hardware | NOT RUN | No port/board/fixture authorization or physical action. Fake/native evidence does not qualify serial/I2C/CC/INT/supply/reset timing. |
| Full supported remote CI matrix | NOT RUN | Prompt34 only; no remote CI dispatch/query or SDK pin change. |
| Other host compilers/sanitizers | NOT RUN | cl/clang++/clang-format unavailable; previously probed MinGW ASan/UBSan runtimes unavailable; no core C++ changed. Unix Makefiles parser supported but no Unix runtime exercised here. |

Final audited archive SHA-256:
`35082d9163f1c0568d267ed057d51d964a3fcdb89143771b601327a4b3099a24`.
Fresh image extraction is
C:/Temp/tusb32x-audit31-32-distribution/package-wwpctqie/renamed-tusb32x;
final clean consumer rerun is package-ee_73pgh/renamed-tusb32x under that same
parent. The parent is an unrelated temporary Git repository with fixture HEAD
e69f18061efe9cd336bd81f960361c537b623629. Every image reports unknown Git/tree;
no enclosing identity is borrowed. The archived example identity scripts remain
present but are never invoked by core consumer configuration/linkage. Stable
Version.h and version truth remain unchanged; no source Git data or generation
step is needed for consuming core.

Six-image output root C:/Temp/tusb32x-audit31-32-firmware retains image-evidence.json,
row command/environment/exit/log JSON, actual dependency reports and compiler
include probes. The lead independently rehashed all18ELF/BIN/map artifacts and
6dependency reports and compared all76extracted bytes with source. Each row has
11actual project-source dependency records: Arduino193/194unique used inputs,
native203. Arduino Make records cover non-system dependencies; native Ninja
records include system headers. System compiler roots are separately detected.
All recorded paths resolve under the exact extraction, selected SDK/toolchain or
that row's generated build outputs. Native app_main, core/shared CLI and UART/I2C
APIs are linked with no Arduino/Wire/Serial symbols; independent absolute S2/S3
SDKCONFIG/lock paths remain isolated. See
[the dated framework evidence](audit/execution-32-frameworks.md#2026-10-28-independent-prompts-3132-audit-verification).

Tools were rechecked unchanged: Python3.12.10, host CMake/CTest4.0.1,
Ninja1.12.1, GNU15.1.0 MCF/UCRT, Doxygen1.13.2, Git2.52.0.windows.1;
managed Core6.1.19, pioarduino55.03.311, Arduino3.3.11 with IDF libraries
5.5.5+sha.b774170ff46. Genuine native SDK5.5.5 uses CMake4.0.3, Ninja1.13.1,
managed Python3.12.10 and Xtensa14.2.0 esp-14.2.0_20260121. The existing native
IDF_PYTHON_CHECK_CONSTRAINTS=no setting handles the absent standalone constraints
cache while installed SDK requirements remain checked. No second Core/SDK was
installed. No source-checkout firmware matrix was duplicated; fresh extracted
links supply the package and firmware evidence together.

Red-before-green proof is retained: prior recovery code admitted later source
work in6subcases; old package code returned PASS with an absolute outside header.
The new positive/negative tests execute actual production helpers and the complete
checker, not source-token searches. Independent final review found no remaining
confirmed defect. No assertion, required suite, output-loss rule or cleanup
contract was weakened. The lead commits/syncs this coherent audited unit on the
existing execution branch and verifies upstream equality before handoff.

Final documentation consistency after the audit evidence was written:136Markdown
files/834local or own-source links/anchors PASS; all173archived guide links still
resolve. Final Git whitespace and changed-module docstring/format checks pass.
No exported payload changed after the audited archive was frozen.

## Local validation entry point

From a source checkout, use:

```text
python tools/validate.py --checks all
python tools/validate.py --checks native,contracts,python --configs Debug
```

The first command requests all declared local non-HIL release checks in Debug
and Release. The second reports focused scope. CMake/CTest 3.21+ (for execution
JUnit), Python 3.10+, a C++17 compiler and the selected generator's build tool
are maintainer requirements. Ordinary tests-OFF consumers still need only CMake
3.16+ and a C++17 compiler. Supported GNU/Clang/MSVC warnings and Release runtime
assertions are preserved.

| --checks name | Owned execution |
| --- | --- |
| native | Runtime core/variant/protocol/owner/port/diagnostics, CLI/console and SDK/application recording fixtures |
| contracts | Compile-contract runtime, isolated allocation-sentinel consumer, existing real CLI parity and its broken fixtures, static core/include/metadata/source-list guard and negatives |
| doxygen | Actual strict Doxygen plus public HTML/XML visibility, generated-file/input boundaries, helper comments and links; real malformed-document fixtures |
| python | Existing version suite, single session/plan suite, single live C++ transcript, example configuration and launcher fixtures, validator and firmware-guard tests |
| package | Existing archive safety, renamed clean tests-OFF consumer and complete outside-header rejection consumers |
| arduino | Four extracted-package basic/owner ESP32-S2/S3 links using the existing managed Core and pinned pioarduino55.03.311/Arduino3.3.11 |
| idf-s2 / idf-s3 | Genuine extracted native CLI links for the respective target, audited ESP-IDF5.5.5, independent absolute build/SDKCONFIG paths |

Selections are comma-separated, deduplicated and explicit. `all` cannot be mixed
with other names. Every CTest has one validation label and a finite timeout;
unknown/missing groups, disabled tests, duplicate names and zero tests fail.
Each group executes once per selected configuration; native plus python never
duplicates a registration. The same fixture path supplied by CMake drives CLI
transcripts and parity. The HIL unit suite loads its existing modules once;
there is no second discovery wrapper. All build configurations compile the
independent-header object target, which is not a behavioral CTest.

Each invocation creates build/validation/run-*/ with command logs, actual CTest
inventory/JUnit and atomically replaced report.json. A completed focused run is
not full release evidence. Failed children and timeouts return 1; unavailable
tools, invalid invocation or unfinished requested scope return 2; interruption
saves completed/failed evidence and pending work, returning 130. Exit 0 requires
all requested checks, including nonzero executed tests. A later successful or
unavailable check cannot erase an earlier failure. Logs preserve the exact child
arguments and signed exit codes. Host commands have a finite --timeout (default
900s); tests also have their own limits. Firmware builds have 1800s per row,
bounded saved output, and a finite aggregate validator bound.

Firmware checks pack once with scripts/pio.cmd, then reuse check_package.py's
hardened extraction and clean consumer. They link only the renamed extraction,
inspect ELF/BIN/map files and required symbols, compiler header dependencies,
unknown archive Git identity and native configuration isolation. The helper
does not install another PlatformIO Core. Set PLATFORMIO_CORE_DIR or --core-dir
to the existing package store. Optional --idf-path and --idf-python identify an
existing audited SDK/environment; SDK Python is a framework tool, while all
maintainer/test Python children use sys.executable. If that SDK has no standalone
constraint cache, the default result is INCOMPLETE. The explicit
--accept-missing-idf-constraints choice permits its documented constraint-cache
waiver while still checking installed SDK Python requirements; the report saves
that choice. No dependency/tool pin changes are implied.

Run malformed-input fixtures before a full validation when changing a guard:

```text
python tools/test_validate.py -v
python tools/test_check_contracts.py -v
python tools/test_check_doxygen.py -v
python tools/test_check_firmware.py -v
```

The guard commands are `python tools/check_contracts.py` and
`python tools/check_doxygen.py`; the latter accepts --doxygen and --out. Their
negative fixtures must fail for their broken inputs, not silently SKIP.
Static boundary checks supplement runtime allocation/portability evidence.
Documentation checks use real parser warnings and independently test visibility;
they do not enable EXTRACT_ALL or suppress missing documentation. The unchanged
version fixture suite owns sync/check/bump/set drift and stable generation.
Archive and linked firmware evidence remain separate from physical qualification.
No selection contacts remote CI, enumerates/opens serial ports, flashes hardware,
publishes a package or runs fixture actions. Prompt34 owns remote matrix proof.

### Prompt33 execution, 2026-10-28

Focused guard fixtures ran before combined validation:37boundary cases,
28real-Doxygen cases,15validator cases and17firmware-inspection cases pass.
The existing18version cases needed no changes. Independent review reproduced
and fixed silent public API hiding through @private, appended EXCLUDE_SYMBOLS
and preprocessing; each now fails its real-parser fixture. Explicit Doxygen
executable propagation, Python interpreter selection, missing registration
families and unusable output paths also have checked corrections. All8C++
file changes preserve executable tokens; only documentation changed.

| Check / actual command | Result / retained evidence |
| --- | --- |
| python tools/validate.py --checks all --accept-missing-idf-constraints | First run-y4c5xv2m correctly FAIL: relative Ninja path broke isolated consumers. Tool paths now resolve before configuration; regression assertion added. This run nevertheless linked and inspected6images. Its failed report remains unchanged. |
| Same full command after correction | run-u1bhwqzn: strict Debug72/72 and Release72/72 PASS; each has53native,6contracts,2Doxygen,8Python and3package registrations. Overall report correctly remains FAIL because the final native-S3 SDK compilation crashed; details below. |
| python tools/validate.py --checks idf-s3 --accept-missing-idf-constraints | run-llfyeh3g: focused PASS/exit0, genuine S3 link and artifact checks. The corrected full run's other5image checks passed. All76payload files are identical between these extractions and current source. No failed report was rewritten into PASS. |
| Latest registration/JUnit verification | Both actual inventories and all10JUnit group files pass current registration/execution checks;72unique, finite, enabled tests per configuration, one owner each. |
| Session and live CLI evidence | The single hil_runner runs311cases (66.088s Debug/65.886s Release); single built-fixture cli_transcript passes in both configurations. No serial port or fixture action is invoked. |
| Version / documentation / formatting | Existing version18cases pass; no-op sync/check preserves bytes/mtimes. Real Doxygen1.13.2 checks36C++files with visible HTML/XML and helper comments.136Markdown files/840local or own-source links/anchors;173package links. All8new Python modules have complete docstrings,100-column lines, UTF-8/LF and clean whitespace. |

Commands and logs are under build/validation/{run-y4c5xv2m,run-u1bhwqzn,
run-llfyeh3g}. Each has its own report.json, command logs and package evidence;
the full host runs retain actual inventory and JUnit files. Framework output
contains exact build/probe argument lists and selected environment in
*.command.json, per-row logs, compiler dependency evidence and hashed artifacts.
The lead independently rehashed the final6image records and verified their11
extracted project sources each, native symbols/no-Arduino, unknown Git identity
and separate absolute SDKCONFIG/lock files. The3package registrations execute
safety, clean tests-OFF linkage and outside-header rejection consumers in both
host configurations. No maintainer tools leak into those consumer PATHs.

The native-S3 failure was Xtensa GCC14.2.0's internal compiler segmentation fault
in unchanged SDK esp_lcd/rgb/esp_lcd_panel_rgb.c:722 during the IRA pass, after
the previous six-image run had passed. The separately requested S3 rerun passed
with the same SDK/compiler/flags and identical input payload. This is retained
as an intermittent toolchain failure, not suppressed or automatically retried.
No single full-validator PASS is claimed for that failed invocation; final
individual host/package/guard and six-image proof is available across the full
and explicitly focused runs. No compiler flags, assertions, pins or gates were
weakened to obtain the later link.

Full-run archive SHA256:
`82a4244ee583571ad81f5a60eb0c9b46d2d7ad19de7f23cbc880ae1251646c4a`;
extraction C:/Users/Honza/AppData/Local/Temp/package-3h5z6s35/renamed-tusb32x.
Focused S3 archive SHA256:
`82fd60471a7028259d545c1fc625b6eb712094f958b9fc51b4619c7520769368`;
extraction C:/Users/Honza/AppData/Local/Temp/package-uy6atcds/renamed-tusb32x.
Each has76source-equal files; differing archive metadata does not change the
stable library header or any payload byte. No source-checkout firmware matrix
was duplicated. The new validator and guard tooling remain outside packages.

Fresh tool probes: Python3.12.10, CMake/CTest4.0.1, Ninja1.12.1,
GNU15.1.0 MinGW UCRT/MCF, Doxygen1.13.2, Git2.52.0.windows.1 and managed
PlatformIO6.1.19. Existing firmware pins remain pioarduino55.03.311,
Arduino3.3.11 and ESP-IDF5.5.5; native tools are CMake4.0.3, Ninja1.13.1,
Python3.12.10 and Xtensa14.2.0 esp-14.2.0_20260121. The explicit constraint-cache
waiver is saved; installed SDK Python requirements still pass their own preflight.
Other host compilers and unavailable MinGW sanitizer runtimes retain earlier
NOT RUN limits. Physical serial/I2C/CC/INT/supply/timing remains NOT RUN; remote
CI belongs to prompt34 and was not contacted. No tags, registry or release was
published. Git commit/sync evidence is reported in the prompt handoff.


## Prompt 34 local preparation, 2026-10-28

Remote checkpoint implementation starts at fc77aae9f81e908ffaa72f2f83781fcb9cba9750
on promptset/tusb32x-implementation-20261023. The working tree was clean and
synchronized before edits. Official pins and read-only peer revisions are in
[audited pins](audit/execution-34-pins.md) and the [SDK audit](audit/execution-34-sdk.md).

| Actual local command | Outcome and evidence |
| --- | --- |
| python tools/check_ci.py --ctest-json build/p34-registration.json | PASS: actual YAML and 72-test Windows inventory, five host/four native matrix rows, immutable action/image pins, finite jobs and single CTest ownership |
| python tools/test_validate.py -v | PASS: 33 cases including 18 CI guard negative cases; missing/disabled jobs, wrong pins, omitted compiler/sanitizers and missing image bindings fail |
| python tools/test_check_firmware.py -v | PASS: 20 cases; official SDK metadata, exact 5.5.5/6.1.0 gate, launcher spelling and generated compiler selection |
| python tools/validate.py --checks native,contracts,python,package,doxygen --configs Debug Release --out build/p34-local | run-zzypjaz6: Debug 72/72 PASS, Release 71/72 with a real package fixture NameError; missing os import corrected. Report retains FAIL. |
| ctest --test-dir build/p34-local/run-zzypjaz6/Release -C Release -R '^(tusb32x_package_safety\|tusb32x_package_consumer\|tusb32x_package_origins\|validate_runner\|firmware_guard)$' --output-on-failure --output-junit build/p34-local/release-corrections.xml | PASS 5/5 after corrections; Debug same focused selection also PASS 5/5 with debug-corrections.xml. All final individual groups pass; no failed report rewritten. |
| python tools/validate.py --checks idf-s2 --accept-missing-idf-constraints --out build/p34-local-firmware | run-ot3on3tb: managed pack and clean consumer PASS; native image linked but inspection failed on an incorrect CMakeCache assumption. Corrected inspector reads unique generated CMake compiler metadata. |
| Explicit reinspection of that existing image with the corrected helper | PASS: firmware-216i669k/reinspection/result.json, all 11 extracted sources, used-header origins, native symbols/no Arduino, ELF/BIN/map and isolated SDKCONFIG/lock. Original failure preserved; no redundant rebuild. |
| python build/p10-docs.py | PASS: 138 Markdown files/851 local or own-source links/anchors, Python syntax/docstrings/style, UTF-8/LF/whitespace, eight vendor hashes and no-op version sync |

Local tools remain Python 3.12.10, CMake/CTest 4.0.1, Ninja 1.12.1, GNU 15.1.0,
Doxygen 1.13.2, Git 2.52.0.windows.1 and managed PlatformIO 6.1.19. Native SDK
5.5.5 uses CMake 4.0.3, Ninja 1.13.1 and Xtensa GCC 14.2.0. The local missing
constraint-cache waiver is explicit and retained. Remote official SDK containers
must pass their dependency checks without this waiver. Linux Clang/ASan/UBSan and
SDK 6.1 are unavailable on this Windows host; the selected remote jobs prove them.

CI declares 71 Linux registrations (53 native, 6 contracts, 7 Python, 2 Doxygen,
3 package). Native/contracts run once per each of five host configurations;
Python/docs/package each run once in Release. Firmware shares one identified
archive for four Arduino images and four genuine native SDK/target images. Source
builds remain diagnostic alternatives, not another final firmware gate. Full
verbose CTest logs and seven-day artifacts preserve actual executed test evidence.
The Windows all selection still uses the managed launcher; Linux CI supplies its
isolated installed Core or official IDF environment to the existing helpers.

Remote verification is PENDING until terminal jobs on the exact implementation
SHA are inspected. No physical serial/I2C/CC/INT/power/timing, HIL, flashing or
publication occurred. A configured workflow, local image and successful push
are distinct from remote PASS.


The first remote run f521f55 retained FAIL: package consumers linked but their
blanket Ninja dependency count included ELF linker records enabled by CMake 4 /
modern GNU ld. Querying only actual compile_commands object outputs preserves
exact nonempty VALID records and all used-header escape checks. The 41-case
package unit suite and three actual Release package registrations pass locally.

All five Linux host rows passed (53 native plus 6 contracts each), including
Clang ASan/UBSan, and the two real Doxygen checks passed. Python's other six
registrations passed; hil_runner alone hit the old 90-second suite ceiling.
An independent timed run completed all 311 cases successfully in 65.922 seconds
locally, with finite multi-suite artifact tests dominating and no isolated hang.
The independent CPU profile identified duplicate complete JSON serialization
at every checkpoint: 20.56 of 28.7 instrumented seconds in the hottest case,
versus 0.525 seconds in fsync. Checkpoints now reuse their already validated
encoding. Registration saves verbose per-case progress; the original 90-second
suite ceiling, serial/operation deadlines and all evidence checks remain intact. The original failed artifacts remain attached to run40775037702.
The grouped corrections will be checked by the next exact-commit push run.


After the encoding correction, all 312 runner cases pass locally in 56.997 seconds
(previous 311-case baseline 65.922 seconds). All 31 focused artifact cases pass,
including fresh-state/exact-byte, once-per-save encoding and invalid-state checks.
No report limit, atomic replacement order, fsync or failure/correction path changes.
The extra case is owned by the existing hil_runner registration. Package41-case
negatives and actual positive/outside-header Release consumers pass. The original
failed CI reports remain intact; these changes are grouped for the next push.


Correction commit aa377ceee86d7c4f23177e987e8769151471df9f was synchronized
and tested by push run [40776243182](https://github.com/janhavelka/TUSB32x/actions/runs/40776243182).
All five host rows, workflow guard, documentation, Python and package jobs passed.
Each host row executes 53 native plus 6 contracts; docs2/package3/Python7 have
matching inventory/JUnit names, no omitted/skipped tests. Actual host compilers
are GCC 14.2.0 and Clang 18.1.3. Python runs all 312 runner cases in 86.915 seconds
(CTest87.094), under the unchanged90-second ceiling; transcript also passes.

The real 76-file archive SHA256 is
57aeb72a99727f013aab98d3828edb311ceba062596326424a3da06bb9d78cfe,
with tests-OFF consumer PASS at /tmp/package-348po76v/renamed-tusb32x.
Both genuine IDF5.5.5 S2/S3 image rows pass. The lead downloaded artifacts
12854373225/12854693178 and rehashed ELF/BIN/map/SDKCONFIG/lock bytes; each
records11extracted sources, native symbols/noArduino and unknown archive Git.

The run correctly remains FAIL: Arduino stopped before build because its
metadata-only toolchain is materialized by the pinned platform during pio run,
after the helper's premature executable check. Both IDF6.1 images linked, then
header-provenance inspection rejected picolibc/no-rtti headers absent from the
bare compiler probe. These are helper defects, not authority to broaden roots,
remove gates or replace real links with SDK-header compilation. The next grouped
correction uses actual configured compiler evidence and defers Arduino inspection
until platform materialization. Additional generated compiler metadata is retained
for diagnosis. Failed/incomplete artifacts remain unchanged. Physical proof is
still NOT RUN.


The next firmware-helper correction passes23focused cases and an actual existing
5.5.5 S2 image reinspection (11sources/5artifacts), saved separately under
reinspection-abi/result.json. IDF6.1 now requires every extracted source command
to name the exact generated compiler response file, then probes with those ABI
flags and the build directory. Missing, empty, relative, variable, mismatched or
duplicate response references fail. Precise returned roots remain existing absolute
directories; there is no wider installation allowlist. Compiler/probe/root/response
hash metadata is saved, together with flags/specs/generated CMake evidence.
Arduino performs its normal pinned platform build before resolving materialized
nm/compiler executables; the mocked sequencing regression cannot yield image PASS.
Existing Windows paths and managed Core remain unchanged. No SDK/tool pin or
firmware source was changed to bypass these failures.


Third implementation commit0ec200d3e1ab65552b81d4272f71568b147431be was
synchronized and tested by push run [40777727121](https://github.com/janhavelka/TUSB32x/actions/runs/40777727121).
Host/docs/package checks again pass. The final helper guard runs23cases.
The runner's prior312-case PASS took86.915s; this run reached278successful cases
before the90.0102s suite deadline. Verbose output identifies continuing bounded
multi-suite artifact tests, not a stalled first case. This demonstrates insufficient
whole-suite CPU margin. The single registration now permits180seconds and retains
every test, assertion, cap and simulated operation deadline. No retry, test omission
or protocol timeout change is introduced. This replaces the measured inadequate
wall-clock ceiling; prior90-second failures and positive results remain recorded.

## Prompt 34 completed remote checkpoint

**PASS:** [run 40778512875](https://github.com/janhavelka/TUSB32x/actions/runs/40778512875),
exact tested implementation SHA `9aa8898c65ca1eebc5f7ea241dfac4bbc815283b`,
branch `promptset/tusb32x-implementation-20261023`. The existing push-triggered run
completed successfully at 2026-10-28 22:20:36 UTC: **14/14 jobs passed**, with no
skipped/allowed-failure matrix rows. No manual dispatch or automatic retry was
needed. Prior failed runs remain FAIL and are documented above.

| Final gate | Actual evidence |
| --- | --- |
| Workflow/metadata | Actual YAML, immutable action/image pins, job ownership/timeouts and stable generated versions PASS |
| Five strict host rows | GCC 14.2.0 Debug/Release, Clang 18.1.3 Debug/Release and Clang 18.1.3 ASan/UBSan Debug; each executes 53 native plus 6 contracts, PASS |
| Registration/execution | 71 unique Linux suites, finite timeouts and exact JUnit/inventory names; no skipped/failed cases; 307 registered executions across owners/configurations |
| Python | 7 registrations PASS: runner 312 cases in 93.180s (CTest 93.380s, 180s bound); real C++ transcript 20.680s; IDF config 4, Arduino config 7, version 18, validator/CI 33 and firmware guard 23 cases |
| Documentation | 2 real-Doxygen/negative registrations PASS with official Doxygen 1.13.2; actual XML/HTML visibility and warning checks retained |
| Package | 3 registrations PASS, then actual PlatformIO archive and clean tests-OFF consumer compile/link/run; 76 source-equal files and 175 package/local links |
| Extracted Arduino | Four basic/owner-service images on S2/S3 PASS; 11 project sources each, 193/194/193/194 approved used dependencies, five extracted public headers each, required symbols and 12 independently rehashed ELF/BIN/map files |
| Extracted native IDF | Both S2/S3 on oldest audited 5.5.5 and current v6.1 (SDK 6.1.0) PASS; 11 project sources each, native app_main/APIs and no Arduino linkage; target/version build and SDKCONFIG/lock isolation |
| Native 6.1 ABI provenance | Actual generated cxxflags/specs and compiler roots retained; 205 used dependencies per target checked, correct picolibc/no-rtti headers, exact response references for all 11 sources |
| Artifacts | All 15 named artifacts exist for the exact tested SHA, seven-day retention; failed-attempt logs, inventories/JUnit, archive provenance, compiler/ABI metadata and actual images/maps retained |

One real archive supplied every firmware job:

```text
SHA256 c492a92e2d1feb65356283329bc09d8a2e6bfda13a428d081e6e41715d287eb7
301887 bytes; 76 files
```

The distribution consumer extracted into `/tmp/package-opg1_28o/renamed-tusb32x`;
Arduino used `/tmp/package-mpobk3ua/renamed-tusb32x`. Native targets have separate
absolute build/SDKCONFIG/lock paths under each job's `build/idf/firmware/` private
work directory. The compiler dependency reports identify all extracted sources,
not this checkout or siblings. Archive firmware correctly reports unknown Git/tree;
external source-sha.log identifies the tested host checkout separately.

Independent reviewers downloaded and checked the exact-SHA artifacts. The lead
also rehashed both 5.5.5 ELF/BIN/map/SDKCONFIG/lock sets and checked native symbols,
source counts, archive identity and target configuration. Retained local evidence:

- `build/p34-ci-final-host9aa`: inventories/JUnit and host/docs/package/Python reports.
- `build/p34-ci-final-arduino-9aa`: common archive and all four Arduino image records.
- `build/p34-ci-final-idf555-9aa/verification.json`: both 5.5.5 rows and five hashes each.
- `build/p34-ci-final-idf61-9aa/verification.json`: both 6.1 rows, ABI roots and hashes.
- `build/p34-final-checkpoint.json`: terminal job/artifact inventory and tested SHA.

CI tools are Python 3.12.16, CMake 4.0.3, Ninja 1.13.0, Doxygen 1.13.2,
PyYAML 6.0.3 and isolated PlatformIO 6.1.19. Hosted Ubuntu 24.04 provides GCC
14.2.0 and Clang 18.1.3; the standalone package consumer uses GNU 13.3.0.
Arduino is the retained pioarduino 55.03.311 / Arduino 3.3.11 / bundled IDF 5.5.5
row, with Xtensa GCC 14.2.0 esp-14.2.0_20260121. The immutable native containers
use Python 3.12.3; 5.5.5 has CMake 3.30.2 and GCC 14.2.0, while 6.1 has
CMake 4.0.3 and GCC 15.2.0 esp-15.2.0_20251204. Native Ninja is the container's
/usr/bin/ninja, recorded through its pinned image rather than a separate version
probe. Official pins, source commits, hashes and Docker digests are in the audit.

Actual commands are the committed workflow's canonical validate.py selections,
PIO pack, check_package.py clean consumer/extraction and check_firmware.py calls.
All subprocess argument lists, finite bounds, selected environment and build paths
are retained in command JSON/logs. Source-build diagnostic commands remain in the
application guides; an identical source-checkout matrix was not added as a gate.

The lead synchronized each implementation unit: f521f55 (workflow/SDK audit),
aa377ce (Ninja provenance/report encoding), 0ec200d (actual ABI/tool materialization)
and 9aa8898 (measured suite margin). The final reporting commit changes documents
only; it is checked locally and synchronized without requiring another remote
checkpoint. Automatic push runs may occur; this record deliberately retains the
actual successful tested SHA rather than assigning its result to a reporting SHA.

**NOT RUN:** physical serial/USB input/output, adapter waveforms/aggregate blocking,
CC attachment/current, INT/supply/enable timing and exact-board qualification.
No serial port, flashing, fixture power control, publication, release or registry
action occurred. Image links and host/fake-serial tests do not establish hardware
qualification. Later numbered prompts use local verification and retain this CI SHA.

## Independent audit of prompts 33 and 34, 2026-10-28

Starting HEAD was `e6d3ba9ab910d672fcf2abda1848ac0838d46912`, clean and equal to
the existing execution branch's upstream after fetch. Re-read both prompts and
reviewed original diffs, current implementation, registered tests, peer sources
and actual checkpoint artifacts. [Progress](implementation-progress.md#independent-audit-of-prompts-33-and-34-2026-10-28)
records exact peer revisions, files and choices. No production C++ behavior,
framework pin, package inventory, version or workflow was changed.

| Confirmed gap | Correction and negative proof |
| --- | --- |
| C++17 digit separators opened a character-literal mask across following code | Recognize preprocessing numbers before character literals. Decimal, binary, hexadecimal, floating/exponent and suffix cases preserve the allocation violation; the original bypass compiles as C++17. |
| Doxygen aliases could hide callable members, and helper declarator spelling could evade comments | Reject aliases and exercise real Doxygen private/protected expansion; detect pointer/reference and trailing returns, including attached names. Positive documented variants still pass. Share the lexical token rules without changing XML line/offset evidence. |
| Final/interrupted report I/O could escape documented exits | Catch final saves and handle interrupted checkpoint failure without recursive saves. Preserve old logs/checkpoints; report incomplete as exit 2 and interruption as 130. |
| A successful short child could leave arbitrarily large validation output | Stop live overflow, fail fast-exit overflow, and retain a maximum 16 MiB prefix. Firmware overshoot is also trimmed after termination. Polling may transiently exceed the retained limit; overflow never becomes successful evidence. |
| Workflow guard accepted absent, inert or failure-suppressed commands and a fixed target | Compare maintained whole scripts, archive upload/download/extraction and matrix binding. Execute negative fixtures for no-ops, heredocs, disabled/reordered/split blocks, ignored exits and duplicate options. No shell interpreter or parallel command framework was added. |
| Ninja Multi-Config queried objects for unbuilt configurations | Select complete unique sources for the requested configuration and its build manifest. Reject missing/ambiguous/outside records; actually build/link/run the clean Multi-Config consumer inside the existing package-origin registration. |

Focused negatives preceded combined verification. Final suites execute boundary
39 cases, Doxygen 33, validator/CI 43 (19 validator, 24 CI), firmware 24 and
package 42, all PASS.
Version fixtures execute 18 cases. No guard was replaced with source-token-only
proof: compiled headers, armed allocation sentinels, real CLI parity/negative
contracts, persistent C++ transcripts and clean package consumers remain active.

Commands and outcomes:

```text
python tools/test_check_contracts.py -v
python tools/test_check_doxygen.py -v
python tools/test_validate.py -v
python tools/test_check_firmware.py -v
python tools/test_check_package.py -v
python tools/check_ci.py
python tools/check_package.py --test-archive --out build/audit-33-34-matrix/package-multiconfig --generator "Ninja Multi-Config"
python tools/test_check_package.py --consumer-origins
python tools/validate.py --checks native,contracts,doxygen,python,package --configs Release --out build/audit-33-34-release
python tools/validate.py --checks native,contracts --configs Debug --out build/audit-33-34-debug
ctest --test-dir build/audit-33-34-release/run-_n_zr31y/Release -C Release -R "^(doxygen|doxygen_negative)$" --output-on-failure
python scripts/generate_version.py sync
python scripts/generate_version.py check
git diff --check
```

**PASS:** Release 72/72 registered suites, Debug 59/59 native/contracts, and final
affected documentation 2/2 after the last declarator fixture. The two combined
validator reports have no pending scope, and exact inventory/JUnit execution
matches ownership: 53 native, six contracts, two Doxygen, eight Python on Windows,
three package. Ninja's actual command database confirms all six strict warnings
and active assertions in 33 runtime-test translation units per configuration.
Compile-only contracts remain separate. Package tests include ordinary and
Multi-Config tests-OFF executions with no Git/Python/Doxygen/PIO on their child
PATH, plus two actual outside-header negative builds. The suite's four bounded
35-second consumers have a 160-second CTest limit; no behavioral contract or
assertion was relaxed. Version sync/check leaves bytes and mtimes unchanged.

Final local evidence:

- `build/audit-33-34-release/run-_n_zr31y`: all five Release groups, inventories,
  JUnit, per-command logs, clean package evidence and fresh generated API pages.
- `build/audit-33-34-debug/run-i707r6_p`: Debug native/contracts and inventory.
- `build/audit-33-34-lead`: final documentation rerun, firmware negatives and
  official action-tag re-resolution. Local Markdown check covers 138 tracked
  Markdown files and 805 local links; packaged links are also checked by the
  real clean consumer.
- `build/audit-33-34-doxygen`: original reproductions, fixed parser omissions
  and real HTML/XML visibility. Current guard checks 36 maintained C++ files.
- `build/audit-33-34-matrix`: original/fixed actual-YAML probes, failed/fixed
  Multi-Config consumer and independently rehashed prior CI artifacts.

Tools: Python 3.12.10, CMake/CTest 4.0.1, Ninja 1.12.1, GNU 15.1.0
MinGW/UCRT/MCF, Doxygen 1.13.2 and existing managed PlatformIO 6.1.19.
PyYAML 6.0.3 is the existing workflow parser; no new Core, SDK or package was
installed. Local Linux sanitizers/SDK 6.1 are not substituted with mocks.

The original checkpoint remains **PASS** at its actual tested SHA
`9aa8898c65ca1eebc5f7ea241dfac4bbc815283b`, [run 40778512875](https://github.com/janhavelka/TUSB32x/actions/runs/40778512875).
Fresh service lookup confirms completed/success. Independent reinspection checks
the stored ZIP digests, 32 ELF/BIN/map/config/lock hashes and all eight image
source sets; no missing image evidence was found. New audit commits do not
inherit that SHA's result. The next entry records synchronization and any
affected push-triggered checks after this tool correction is committed.

Physical serial/I2C/CC/INT/supply/timing remains **NOT RUN**. No board, port,
flash, fixture power, registry or publication operation occurred. Native images
and fake-serial tests establish software handling only.

### Audit push exposed the complete transcript's suite limit

Checked correction `ac211e03a60f4b70a9005d07d6b6dde6f9927c1e` was committed,
pushed and fetched; local/upstream equality and zero ahead/behind were verified
with a clean worktree. Its [push run 40781392477](https://github.com/janhavelka/TUSB32x/actions/runs/40781392477)
passes all five strict host rows (including sanitizers), workflow, Doxygen,
package and four native image rows. The Python job is **FAIL**: actual JUnit and
LastTest.log show cli_transcript killed at 30.0201 seconds. It had emitted no
partial progress. The other six Python registrations pass: runner 312 cases in
138.589s, validator/CI 43, firmware 24, version 18 and both example-config suites.
This host is substantially slower than the earlier runner's 93.180s and complete
transcript's 20.680s. No timeout failure is reclassified as PASS.

Failed artifact 12856604145 (54,409 bytes) has verified ZIP SHA-256
`62168c8293ff54e1028efc1ffe220834228430636aeb456af8b16a580281b91d`;
`build/audit-33-34-new-ci/python-LastTest.log` retains the original failure.
The correction changes only the finite whole-suite CPU/filesystem allowance to
60 seconds and emits four bounded flushed phase messages after their assertions
pass. It preserves every inner deadline, simulated time check, fixture case and
assertion. Local focused reconfiguration/CTest uses the same existing registration
and built fixture; no second transcript suite or remote retry policy is added.

The same run's real package has 76 files, 302,524 bytes, SHA-256
`836669fbaf334ed992710198af00247100f5d2a4fd4a0424e589b4fc7e64623b`.
Its Linux package-origin test executes all four consumers, including the clean
Multi-Config case, in 4.36 seconds. Available native artifacts are independently
rehashed (20 ELF/BIN/map/sdkconfig/lock files), with 11 extracted project sources
and five public headers each, correct target/configuration isolation and native
symbols without Arduino. SDK 5.5.5 checks 203 dependency paths; 6.1 checks 205.
Evidence is retained under `build/audit-33-34-new-ci`; later results stay tied to
their actual commit rather than being assigned to a reporting revision.

All eight image artifacts from that run are now independently verified: 32
ELF/BIN/map/config/lock hashes match and all use the same archive and exact SHA.
The four Arduino rows contain 11 project sources, five public headers and
193/194 approved dependencies each. The run is **FAIL (13/14 jobs passed)**
because of the retained transcript timeout; successful firmware links do not
erase it. The unchanged firmware matrix needs no additional source-build pass.

Focused correction check **PASS**: the actual registered CLI transcript completes
in 10.86s locally, with all 248 persistent requests, session/fixture/failure cases
and four progress checkpoints. Fresh CTest JSON and check_ci.py confirm all 72
registrations still have their exact owners and finite timeouts. Python syntax,
version ownership and diff whitespace checks remain valid. The next affected
push-triggered Python result will be recorded with its own tested commit.

### Completed audit CI

**PASS:** [run 40782369910](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910),
exact tested SHA `c50f8ddda8809425ede2161586466c95fa650ae6`, completed
2026-10-28 22:50:15 UTC. All **14/14 jobs** passed on the same execution branch:
five strict GCC/Clang Debug/Release/sanitizer configurations, workflow, Python,
documentation, package, four Arduino images and four native IDF SDK/target images.
The 15 expected artifacts exist, unexpired and attributed to that exact SHA;
bounded seven-day retention and required artifact paths remain enabled.
No manual dispatch, job retry, disabled case or assertion suppression was used.
The first audit push remains FAIL and its evidence is retained above.

Independent final artifact checks:

| Check | Actual PASS evidence |
| --- | --- |
| Registration | All 71 Linux names match the canonical groups: 53 native, six contracts, seven Python, two Doxygen, three package; finite bounds and no skipped Python JUnit cases |
| Complete transcript | 17.8099s under the 60s suite ceiling; four completed phase messages, all 248 persistent requests and the complete prior failure/fixture checks |
| Runner | 312 cases in 94.474s (CTest 94.7005s); original 180s suite and every inner deadline unchanged |
| Validator/CI, firmware guard, version | 43, 24 and 18 cases respectively, all PASS |
| Package safety/origins | 42 unit cases and real clean consumers, including Ninja Multi-Config and outside-header rejection; origins 3.24851s |
| Archive/consumer | 302,545 bytes, 76 source-equal files, 175 packaged links; renamed tests-OFF consumer compile/link/run; no maintainer tools on its child PATH |
| Final artifacts | Three independently downloaded Python/package ZIP digests match service metadata; exact source SHA, JUnit, logs and consumer archive hash agree |

Final archive SHA-256:

```text
654a5213022bc570eeb346f56d084cdbdaaf75cdbc5928406e25450fa6cf0240
```

All eight final extracted image jobs pass their actual compiler-dependency,
identity, target/configuration, native-symbol and artifact checks. The earlier
independent rehash of 32 image/map/config/lock files remains explicitly attached
to `ac211e0`; it is not transferred to this SHA. No identical extra source
firmware matrix was added. SDK/tool/action pins and production firmware sources
remain unchanged throughout this audit. Actual CI tools are those recorded in
the checkpoint's pinned matrix; no new supported row is claimed.

Evidence: `build/audit-33-34-final-ci/verification.json`, its three artifact ZIPs
and python-LastTest.log; `build/audit-33-34-lead/ci-final.json` records the terminal
run, all jobs and exact-SHA artifact inventory. The local Release 72/72 and
Debug 59/59 evidence precedes only the separately verified transcript CPU-margin
correction; final CI executes all affected checks on that correction.

The final documentation-only report is checked locally and synchronized without
another remote gate. It retains `c50f8dd` as the tested CI revision. Remaining
**NOT RUN** evidence is physical serial/USB, I2C framing/aggregate blocking,
Type-C/current/INT and exact-board supply/timing qualification. No HIL, flashing,
power manipulation, release, registry publication or sibling change occurred.

## Prompt 35: user guides and compiling examples

Date: 2026-10-29, Europe/Prague. Source start:
`45da5296792ee4a38120e9c4a65f23ce6951adce`, clean index/tree, synchronized
same-name upstream after fetch (zero ahead/behind). Commands below ran with the
scoped prompt-35 tree dirty on `promptset/tusb32x-implementation-20261023`.
The [progress entry](implementation-progress.md#prompt-35-user-guides-and-feature-coverage-2026-10-29)
records current peer revisions, exact files, TI/public-source rechecks and
adopted/rejected conventions. Production C++ sources, public signatures,
firmware pins and stable package version `0.1.0-dev.1` are unchanged.

Environment: Windows PowerShell, Python 3.12.10, CMake/CTest 4.0.1, Ninja 1.12.1,
GNU 15.1.0 (MinGW-W64 x86_64 UCRT/MCF), Doxygen 1.13.2, Git 2.52.0.windows.1,
managed PlatformIO Core 6.1.19. Existing supported framework rows and their
actual prior link evidence remain in the checkpoint above; they were not rerun
for prose or the host-only README fixture.

| Check | Actual command/result and evidence |
| --- | --- |
| Literal quick start, Debug | `cmake -S . -B build/p35-docs-debug -G Ninja -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug`; build target `tusb32x_compile_contracts`; `ctest --test-dir build/p35-docs-debug -C Debug -R "^tusb32x_compile_contracts$" --output-on-failure`: **1/1 PASS**. |
| Literal quick start, Release | Equivalent configure in `build/p35-docs-release` with Release; `cmake --build build/p35-docs-release --config Release`: **PASS**, including independent public headers. Registered contract test **PASS**. Both generated snippets equal the literal README fence; strict conversion/sign warnings and `-UNDEBUG` are present. |
| Existing core contracts | `ctest --test-dir build/native -C Debug -R "^(tusb32x_protocol|tusb32x_operations|tusb32x_port|tusb32x_diagnostics|tusb32x_host_basic)$" --output-on-failure`: **5/5 PASS**, 0.19 s. These runtime cases support guide claims; no electrical timing is inferred. |
| Focused registered guards | `ctest --test-dir build/p35-docs-release -C Release -R "^(tusb32x_compile_contracts|tusb32x_package_safety|tusb32x_package_consumer|tusb32x_package_origins|tusb32x_version_sync|core_contracts|doxygen)$" --output-on-failure --output-junit p35-docs.xml`: four PASS and three initial package-link failures. After correction, the three package checks below PASS, giving **7/7 final scoped PASS**; original failed JUnit retained. |
| Package correction | `ctest --test-dir build/p35-docs-release -C Release -R "^tusb32x_package_(safety|consumer|origins)$" --output-on-failure --output-junit p35-package.xml`: **3/3 PASS**, 10.05 s. Includes 42 safety fixtures, real tests-OFF renamed consumption and four origin consumers/negative paths. Newly introduced CLI/examples evidence links pointed outside the export list; corrected to repository URLs. |
| Version truth | `python scripts/generate_version.py sync` and `python scripts/generate_version.py check`: **PASS**, no generated byte change. Registered version suite **18/18 PASS**. No version bump. |
| Source/API documentation | Registered `doxygen`: **PASS**, no warnings/errors, 36 C++ files with HTML/XML visibility, links and helper checks. Generated XML independently contains the facade, Status, DriverConfig, DeviceProfile, OperationResult, PortSnapshot and DriverDiagnostics. Evidence: `build/p35-docs-release/test/doxygen-check/latest.json`. |
| Real command parity | `python tools/check_cli_contract.py --host build/native/test/Debug/tusb32x_cli_host.exe`, followed by `--self-test-negative`: **PASS**, 55 rows/164 executed routes and deliberately broken contract rejection. The actual built fixture is used; no source-token parity claim. |
| Ordinary consumer | `cmake -S examples/host/basic -B build/p35-doc-host-basic -G Ninja -DTUSB32X_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release`; build Release and execute `build/p35-doc-host-basic/tusb32x_host_basic.exe`: **PASS**. Simulated initialization/apply/port/invalidation/reconciliation output agrees with the host README. |
| Runner recipes | Runner `--help` and three documented `--dry-run --variant tusb320lai --address 0x47` selections: readonly; orientation/accessory/role-current/interrupt; reset-termination/fault/console. Saved plans **PASS**, hardware **NOT RUN**, exact exit **2**, TX/RX/capture all zero. Evidence: `build/p35-doc-{readonly,fixture,recovery}-verified/`. |
| Repository navigation | Existing `tools.check_package.markdown_links` applied to all 139 tracked/new Markdown files with the parent rules in scope: **838 local targets/fragments PASS**. No second link-checker implementation. |
| Registration | `ctest --test-dir build/p35-docs-release -C Release --show-only=json-v1`, checked by existing `tools.validate.registration`: **72** unique finite checks, 53 native / six contracts / two Doxygen / eight Python / three package. One contract and one transcript registration; no disabled or omitted required group. |
| Final whitespace/metadata | `git diff --check` and generator `check`: **PASS**. Final staged content reviewed before commit. |

### Current archive and evidence limits

The actual managed-Core commands were:

```text
scripts\pio.cmd pkg pack . --output build/p35-docs-distribution.tar.gz
python tools/check_package.py --archive build/p35-docs-distribution.tar.gz --out build/p35-docs-distribution
```

**PASS:** 77 source-equal files, including `docs/owner-operations.md`, with
**206** valid local links in both the source export set and renamed extraction.
The clean consumer compiles, links and executes public read-only initialization
with tests OFF and maintainer tools absent from its child PATH. Compiler source
and header origins remain inside the approved extracted/fixture/toolchain roots.
There is no install/export claim. Archive SHA-256:

```text
67d3bad27d3945d50faf8c479b751e311a6cc938d865242d414a2e1cbcc6d406
```

Evidence: `build/p35-docs-distribution/package-report.json`, compiler dependency/
origin reports, and extraction
`C:/Users/Honza/AppData/Local/Temp/package-u6fmcz13/renamed-tusb32x`.
`build/p35-docs-release/verification.json` records the initial/final focused
checks and package report without erasing the original failing JUnit.
Earlier trial archives correctly failed source-byte comparison after ongoing
guide edits; only the finished archive above is accepted.

The native-IDF frontend exposes core all/managed totals but optional adapter
callback-kind counts are unknown. Complete readonly/soak plans require those
counts to prove absence of data-write callbacks and are **BLOCKED** there;
individual native CLI observations remain usable. This documented evidence
limit does not turn a runner failure into hardware PASS.

No firmware build matrix or new remote checkpoint was requested or inspected.
The completed prompt-34 audit remains **14/14 PASS only at**
`c50f8ddda8809425ede2161586466c95fa650ae6`,
[run 40782369910](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910).
It does not verify a later guide commit. Physical serial/USB, I2C framing and
aggregate blocking, Type-C/current/INT, supply/startup and exact-board timing
remain **NOT RUN**. No port was opened, fixture manipulated, firmware flashed,
release published or sibling edited. Prompt 36 has not started.

## Prompt 36: source documentation and maintainer guidance

Date: 2026-10-29, Europe/Prague. Started from
`e37f0a35aa0cbad5cf6a4f46d22032e2adce253d`, clean index/tree and equal to
the same-name upstream after fetch, zero ahead/behind. Work remains on
`promptset/tusb32x-implementation-20261023`; checks below used the scoped dirty
documentation tree. The [progress entry](implementation-progress.md#prompt-36-source-documentation-and-maintenance-2026-10-29)
records current peer HEADs/dirty states, exact files, delegation and decisions.
No production tokens, public API, source lists, SDK pins or package version changed.

Environment remains Windows PowerShell, Python 3.12.10, CMake/CTest 4.0.1,
Ninja 1.12.1, GNU 15.1.0 (MinGW-W64 UCRT/MCF), Doxygen 1.13.2 and
Git 2.52.0.windows.1. All required local documentation tools are available.

### Source and generated API audit

Inspected all 36 maintained C++ inputs: five public headers, eight private core
files and 23 example headers/sources, plus every Python module (34 files).
Eleven C++ files changed only comments; all code tokens and string/character
literals independently match HEAD. Twelve Python files changed only docstrings;
ASTs with docstrings removed match HEAD, and every module compiles in memory.
Module/exported helper docs cover meaningful inputs, returns/exits, timeout units,
files and side effects; existing tests/fakes need no boilerplate comments.
No generated Version.h/template change, public alias removal or extraction change
was needed. Private helpers remain private with useful invariant comments.

Reopened archived TI timing/register pages before comment corrections:
SLLSEN9F pp7/14/16/18-19, SLLSEP2D/SLLSEQ8D pp6/14/18-19 and
SLLSEO5D pp6-7/9-11/18-20. Corrected ambiguous fault-bit wording and expanded
mask/wait citations without changing behavior or closing unresolved gates.
Actual runtime cases confirm outputs preserved on failure, bus-silent local
methods, actual-control ACK composition, fresh completion anchors and retained
possible effects. Status-only NACK/BUSY cannot prove a write had no effect.

The generated API was inspected beyond Doxygen's return code: HTML pages and
XML for the facade, Status, DriverConfig/DeviceProfile, token/timing/progress/
result, configuration/port snapshots, transport/managed diagnostics, health and
register decode are visible. All **16 principal types**, **41 public function
entries** and **23 Err values** have prose and applicable parameter/return
contracts. Existing guard checks also cover the remaining public members,
anchors, enum values and local HTML assets. `EXTRACT_ALL=NO`, private extraction
NO and all documentation warnings remain enabled.

### Executed checks

```text
cmake -S . -B build/p36-docs-release -G Ninja -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
ctest --test-dir build/p36-docs-release -C Release -R "^doxygen_negative$" --output-on-failure --output-junit p36-doxygen-negative.xml
cmake --build build/p36-docs-release --config Release --target tusb32x_compile_contracts tusb32x_header_contracts
ctest --test-dir build/p36-docs-release -C Release -R "^(doxygen|tusb32x_version_sync|core_contracts|tusb32x_compile_contracts|tusb32x_portable_consumer|tusb32x_package_consumer)$" --output-on-failure --output-junit p36-final-guards.xml
```

| Check | Actual result |
| --- | --- |
| Existing Doxygen omission suite, run first | **1/1 CTest PASS**, 6.37 s; **33 fixture cases**, including missing public/type/enum comments, wrong parameters, broken reference, hidden/alias API, source/helper omissions, generated boundaries and missing-tool/timeout failure. Uses real Doxygen for documentation defects; no skipped success. |
| Final guard/consumer selection | **6/6 PASS**, 17.49 s. README literal simulation, copied public/allocation-sentinel consumer, renamed extracted tests-OFF consumer, 18 version fixtures, boundary check and real Doxygen. Independent headers also compile under strict warnings. |
| Example/CLI selection | **53/53 Release PASS** with the regex below. Actual adapters/applications, owner/mailbox/host, shared CLI/console, config fixtures, persistent transcript and CLI parity/negative checks; no hardware. |
| Existing core semantics | `ctest --test-dir build/native -C Debug -R "^tusb32x_(protocol|operations|port|diagnostics)$" --output-on-failure`: **4/4 PASS**, existing Debug binaries. Code-token identity verifies these comments introduced no behavior change. |
| Source documentation guard | **PASS**, 36 C++ inputs, no diagnostics. Generated HTML/XML/local links/helpers and independent page/parameter inspection are saved below. |
| Stable metadata | `python scripts/generate_version.py sync` and `check`: **PASS**, unchanged generated bytes. Manifest/component/CMake/Doxygen agree at `0.1.0-dev.1`; MIT/Thymos/maintainer/CODEOWNERS and vendor terms remain unchanged. |
| Maintenance entrypoints | Checker/validator/generator and seven changed CLI `--help` paths: **PASS**. Accepted names/defaults/exits match new instructions. No help invocation starts HIL or firmware work. |
| Registration and Release behavior | Actual CTest JSON checked by existing `tools.validate.registration`: **72** unique finite owned tests (53 native, six contracts, two Doxygen, eight Python, three package). No disabled/missing required group. Strict GNU warning flags and `-DNDEBUG ... -UNDEBUG` remain in Release fixture builds. |
| Repository/package links | Existing Markdown checker passes all 139 repository Markdown files (**844 local links/fragments**) and **207** links in the 77-file export set/extraction. Final counts recorded in `source-review.json` and `build/p36-docs-final-package/package-report.json`; no duplicate checker implementation. |
| Final scoped review | Exact comment/docstring-only checks, independent maintainer/changelog review, generator check and `git diff --check`: **PASS**. No documentation omission is hidden or waived. |

The example selection was:

```text
ctest --test-dir build/p36-docs-release -C Release -R "^(tusb32x_(cli|console|example_owner|example_integration|output_mailbox|host_basic|arduino_wire_[0-9]+|idf_transport_[0-9]+|arduino_app_.+|idf_app_.+|arduino_examples_config|idf_example_config)|cli_transcript|cli_contract|cli_contract_negative)$" --output-on-failure --output-junit build/p36-examples-junit.xml
```

These three Release selections are disjoint: **60/60 selected CTests PASS**.
The complete 72-test matrix was not repeated through another wrapper. Evidence:

- `build/p36-docs-release/{p36-doxygen-negative,p36-final-guards}.xml` and
  `build/p36-docs-release/build/p36-examples-junit.xml`.
- `build/p36-docs-release/test/doxygen-check/latest.json` selects the current
  generated HTML/XML and stdout/stderr under `run-isocazet`.
- `build/p36-docs-release/api-review.json`, `source-review.json`,
  `registrations.json` and `test/package-check/package-report.json` retain
  page/member, source, inventory and extracted-consumer evidence.

After final changelog reconciliation,
`python tools/check_package.py --test-archive --extract-only --out build/p36-docs-final-package`
also passes: 77 source-equal files and 207 packaged links. This final guide-byte
check reports consumer **NOT RUN**; it does not replace the executed clean
consumer above or imply a new firmware link.

AGENTS/CONTRIBUTING now match actual commands, same-branch authorized Git,
delegation/runtime model policy, existing managed Core, exact native SDK rows
and explicit release permission. CHANGELOG's unreleased prompt diary and stale
planned claims were replaced with user-facing features/fixes; the full execution
history remains in progress, validation and Git. No tag, release date or owner
was invented, and no consumer-time generator was introduced.

No firmware matrix or remote CI was rerun solely for comments. The earlier
checkpoint remains tied to `c50f8ddda8809425ede2161586466c95fa650ae6` /
[run 40782369910](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910),
not this later commit. Physical serial/USB, I2C framing and aggregate blocking,
Type-C/current/INT, power/startup and exact-board timing remain **NOT RUN**.
Complete native readonly/soak proof still reports **BLOCKED** without optional
callback-kind counters. No physical port, flash, power action, publication,
sibling modification or next-prompt implementation occurred.

## Independent prompts 35–36 audit (2026-10-29)

Fresh audit of original prompts 35 and 36 and the actual `45da529..HEAD` diff,
starting from `6e96f8efb62b19f78c4e1951b6ed71fa2c574e21`. Initial tree/index
were clean; fetch confirmed same-branch upstream equality and zero ahead/behind.
The [progress record](implementation-progress.md#independent-prompts-3536-audit-2026-10-29)
captures parallel independent reviews, exact peer revisions/paths, TI evidence
and confirmed fixes. Checks below used this audit's scoped dirty tree.

Environment: Windows PowerShell, Python 3.12.10, CMake/CTest 4.0.1, Ninja 1.12.1,
GNU 15.1.0 (MinGW-W64 UCRT/MCF), Doxygen 1.13.2, Git 2.52.0.windows.1 and
the existing managed PlatformIO Core 6.1.19. No new tool installation or pin change.

### Confirmed defects and regression evidence

An initial positive documentation guard did not establish private API exclusion:
direct HTML/XML inspection found five private core structs. A fully documented
private fixture passed the old guard, then failed after the new leak check.
Three fixtures now check published-private rejection, retained file visibility
and missing private-helper comments despite extraction exclusion. All original
public/enum/parameter/reference/hidden-API/generated-boundary cases remain active.
The first positive run after exclusion correctly rejected ten definitions whose
documentation had previously been inherited from headers. Useful adjacent
invariant comments corrected those omissions; the final positive guard passes.

Deterministic child-clock probes also reproduced a zero-exit validator PASS at
1.10 seconds with a 1-second deadline. Both child helpers now clip the final wait
and reject observed completion at or after the deadline. New regression cases
cover 0.999, 1.000 and 1.100 seconds without sleeps, plus retained logs and cleanup.
The existing firmware_guard and validate_runner registrations pass after these
corrections. This closes a real executable defect exposed by the documentation
audit; it does not claim process creation is portably interruptible.

The source/guide review also corrected `irq service` sample-publication wording
and disclosed missing optional callback-kind counters on both Arduino and native
IDF. The actual runner guard and existing negative runtime test block full
readonly/soak plans before initialization. No adapter support was invented.

### Executed local checks

```text
cmake -S . -B build/audit-35-36-release -G Ninja -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/audit-35-36-release --config Release --target tusb32x_compile_contracts tusb32x_header_contracts tusb32x_port tusb32x_cli tusb32x_cli_host tusb32x_console tusb32x_operations tusb32x_protocol tusb32x_diagnostics tusb32x_host_basic
ctest --test-dir build/audit-35-36-release -C Release -R "^(doxygen|doxygen_negative|tusb32x_version_sync|core_contracts)$" --output-on-failure --output-junit audit-docs.xml
ctest --test-dir build/audit-35-36-release -C Release -R "^(tusb32x_compile_contracts|tusb32x_protocol|tusb32x_operations|tusb32x_port|tusb32x_diagnostics|tusb32x_cli|tusb32x_console|tusb32x_host_basic|cli_contract|cli_contract_negative|cli_transcript|tusb32x_package_consumer)$" --output-on-failure --output-junit audit-native.xml
ctest --test-dir build/audit-35-36-release -C Release -R "^firmware_guard$" --output-on-failure
ctest --test-dir build/audit-35-36-release -C Release -R "^validate_runner$" --output-on-failure --output-junit audit-validator.xml
ctest --test-dir build/audit-35-36-release -C Release -R "^(doxygen|doxygen_negative)$" --output-on-failure --output-junit audit-doxygen-fixed.xml
ctest --test-dir build/audit-35-36-release -C Release -R "^doxygen$" --output-on-failure --output-junit audit-doxygen-final.xml
ctest --test-dir build/audit-35-36-release -C Release -R "^tusb32x_portable_consumer$" --output-on-failure --output-junit audit-portable.xml
```

| Evidence | Actual result |
| --- | --- |
| Initial guard selection | **4/4 PASS**, 21.43 s. The later direct private-page finding demonstrates the limitation in that initial positive result. Version fixtures and core boundaries pass. |
| Core/CLI/transcript/package selection | **12/12 Release PASS**, 14.45 s. Includes the actual persistent C++ fixture, CLI parity/negative checks and renamed extracted tests-OFF consumer. |
| Changed child helpers | firmware_guard **1/1 PASS** (25 cases, 1.26 s); validate_runner **1/1 PASS**, 0.90 s. Existing finite registrations reused. |
| Expanded real-Doxygen fixtures | **36 cases PASS**. The paired positive run initially failed on ten missing adjacent helper comments; these were fixed, not suppressed. Final doxygen **1/1 PASS**, 2.78 s. |
| Independent portable consumer | **1/1 PASS**, 1.52 s, actual copied public headers/library and runtime allocation sentinel. |
| README and independent headers | Fresh strict Debug/Release compile/link and README runtime **PASS**. After final comment corrections, rebuilt Release compile/header targets and reran the README contract **1/1 PASS**. |
| Standalone ordinary C++ example | Fresh tests-OFF configure/build/run **PASS**; outputs the documented SIMULATED flow, failed-payload/successful-delivery distinction and invalidation/reconciliation checks. Cache discovers no Python/Git/Doxygen. |
| Generated API inspection | **PASS**, 18 principal type pages/member anchors, all 23 Err values, and inspected parameter/return contracts for startInitialize, poll, takeResult and both raw APIs. No TUSB32x::detail XML compounds or HTML pages remain; public decodeRegister is visible. |
| Source/registration review | **PASS**, five changed C++ files have identical executable tokens/literals; all 34 Python modules compile. Actual CTest JSON retains 72 unique finite owned checks: 53 native, six contracts, two Doxygen, eight Python and three package. Strict flags and Release -UNDEBUG remain active. |
| Stable version | `python scripts/generate_version.py sync` and `check`: **PASS**, no generated byte changes; truth remains 0.1.0-dev.1. |

The Debug README check used a separate `build/audit-35-36-debug` directory,
`CMAKE_BUILD_TYPE=Debug`, target `tusb32x_compile_contracts` and the matching
CTest filter. Standalone commands were:

```text
cmake -S examples/host/basic -B build/audit-35-36-host -G Ninja -DTUSB32X_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/audit-35-36-host --config Release
build/audit-35-36-host/tusb32x_host_basic.exe
```

Evidence is retained under `build/audit-35-36-release`: the named JUnit files,
`audit-api-review.json`, `audit-source-review.json`, `audit-registrations.json`
and package-consumer evidence. The current Doxygen `latest.json` selects
`test/doxygen-check/run-9ypcrg03` with real HTML/XML and empty errors. The scoped
selections were not immediately repeated through a full validation wrapper.
The final link/archive check is recorded below after the guide edits.

No firmware matrix or remote run was repeated for this audit. Earlier firmware
and CI evidence remains tied to `c50f8ddda8809425ede2161586466c95fa650ae6` /
[run 40782369910](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910),
not this later change. Physical serial/USB, aggregate adapter blocking, I2C
framing, CC/current/INT, power/startup and exact-board timing remain **NOT RUN**.
Full readonly/soak evidence on the shipped Arduino/native-IDF examples remains
**BLOCKED** without optional callback-kind counters; manual CLI use is available.
No hardware, sibling, publishing or next-prompt action occurred.

Final repository link check reuses `tools.check_package.markdown_links` across
all **139** tracked Markdown files: **849 local links/fragments PASS**, including
the parent AGENTS reference. The managed Core produced a fresh archive with:

```text
scripts\pio.cmd pkg pack . --output build/audit-35-36-package/TUSB32x.tar.gz
python tools/check_package.py --archive build/audit-35-36-package/TUSB32x.tar.gz --extract-only --out build/audit-35-36-package/check
```

Archive SHA-256:
`284c593213918c24343be45ef3a81dbf8922c488669b261475c1c7de72747014`.
Safe extraction into a newly created temporary root renamed the library to
`renamed-tusb32x`; **77 source-equal members and 210 packaged links PASS**.
Evidence: `build/audit-35-36-package/check/package-report.json`. This final
guide-byte check reports consumer **NOT RUN**; the independently executed
registered consumer above supplies compile/link/runtime proof. No new extracted
firmware link is implied. Version sync/check and the full scoped diff review
pass; final commit/synchronization evidence belongs in the handoff.

## Prompt 37: independent core review

Reviewed clean revision `bdfcc144f292d4e1cdaecea58796aa208a0bec66` on
2026-10-29. The [core audit](final-audit.md) records scope, independent reviewers,
TI/pinned-code evidence, refreshed peer inventory and corrected documentation.
No new executable core defect was reproduced; no runtime or test implementation
changed. Existing fault, timing, encoding and ownership cases were reused.

Actual maintained validation command:

```text
python tools/validate.py --checks native,contracts,doxygen --out build/p37-validation
```

Final report: `build/p37-validation/run-ck1kncpq/report.json`, status PASS with
no pending checks. Both Debug and Release were freshly configured/built using
GNU 15.1.0, CMake 4.0.1, Ninja 1.12.1 and Python 3.12.10. Doxygen 1.13.2 runs through
the maintained guard. Generated Release Ninja flags retain strict warnings and
`-UNDEBUG` for executable checks. CTest inventory still contains 72 finite owned
tests on Windows; this focused selection executes each selected owner once.

| Configuration / check | Result | Evidence |
| --- | --- | --- |
| Debug native | PASS, 53 tests | debug-native.log/xml; core, protocol, operations, port, diagnostics, adapters/apps, CLI/console, host example and live transcript |
| Debug contracts | PASS, 6 tests | debug-contracts.log/xml; compile contracts, isolated public consumer, executable CLI/core contract checks and negatives |
| Debug documentation | PASS, 2 tests | debug-doxygen.log/xml; real-parser visibility/omission/negative checks |
| Release native/contracts/documentation | PASS, 61 tests | release-native/contracts/doxygen.log/xml; assertions/checks active |
| Independent owner reviewer fresh Release | PASS, 7 tests | build/p37-owner-review; protocol/operations/port/diagnostics/compile-contracts/host-basic plus isolated consumer; consumer 369 checks, C/C++ allocation and explicit-failure sentinels |
| Archived TI evidence | PASS | Eight PDF sizes/SHA-256 match provenance; visual B p19, A p19 and V p20 table review, additional reviewer procedure/source checks |
| Physical/firmware/remote checks | NOT RUN in this prompt | No new firmware link, hardware access, flash or HIL claim; remote PASS remains solely run40782369910 at c50f8ddda8809425ede2161586466c95fa650ae6 |

The independent protocol reviewer also ran existing Release protocol/operations
binaries and inspected pinned helper arithmetic; the lead's fresh validation
above supplies final source/build correspondence. Windows sanitizer runtimes
remain unavailable as previously recorded; no new sanitizer run is claimed.
The existing prompt34 Linux sanitizer result is not extended to later commits.

Documentation-only corrections are checked with the maintained Markdown link
parser and Doxygen guard after editing. Prompt38 retains independent integration/
distribution review and final handoff. Missing physical fixtures and unresolved
silicon wording remain qualification limits rather than invented code defects.

Post-edit `python tools/check_doxygen.py` PASS: 36 C++ files, real HTML/XML public
visibility, links and helper comments; evidence at
`build/doxygen-check/run-vt5vgq7m`. Reused `tools.check_package.markdown_links`
over tracked/new Markdown under the parent Projects root: 861 local targets and
fragments PASS, including parent AGENTS. Changed files pass UTF-8/LF/trailing-
whitespace and `git diff --check`. The lead independently matched the four
Linux/regmap/Betrusted cached source hashes and inspected reset-mask and Try.SRC
helper excerpts. No new package or firmware claim is inferred from these checks.

## Prompt 38: final integration and distribution review

Reviewed clean `e12ecb74c2a6870665709233cbfaaa5549a804e5` on 2026-10-29.
[Final audit](final-audit.md) records independent scopes, exact findings, source
checks and dispositions. Application fixes add primary callback-kind evidence
and reject potentially truncated Arduino USB input. Core, adapter timeout/framing
and SDK pins remain unchanged. No remote CI was dispatched or queried.

Actual commands (all from this checkout):

```text
python tools/validate.py --checks python,package,arduino,idf-s2,idf-s3 --accept-missing-idf-constraints --out build/p38-validation
python tools/validate.py --checks native,contracts,doxygen,arduino --out build/p38-final
ctest --test-dir build/p38-final/run-xjsuuhsj/Debug -C Debug -R '^validate_runner$' --output-on-failure
ctest --test-dir build/p38-final/run-xjsuuhsj/Release -C Release -R '^validate_runner$' --output-on-failure
python scripts/generate_version.py check
```

Both validator commands exited 0 with requested scope PASS and no pending checks.
The first run's unchanged Python/package evidence is reused. Its native image
sources include the final callback-counter fix; the later Arduino RX correction
required four new Arduino links. No unchanged native build was duplicated.

| Check | Actual result and retained evidence |
| --- | --- |
| Python/session/transcript and package | PASS 8 + 3 CTests in each Debug/Release, `build/p38-validation/run-rjz5y8or/report.json` and per-group logs/JUnit; hostile session/fixture tests, exact live CLI, Windows aliases and clean consumers |
| Final core/CLI/adapters/apps, contracts and documentation | PASS 55 + 6 + 2 CTests in each Debug/Release, `build/p38-final/run-xjsuuhsj/report.json`; fresh builds, strict warnings and active Release assertions |
| Changed registration guards | Initial focused validator failed because check_ci still expected 53 native tests; corrected to 55 including the two new RX cases. Final validate_runner PASS in both directories above (44 underlying cases). No assertion was weakened. A probe of unused build/release discovered no tests and supplies no evidence; actual Release results are from the explicit final directory |
| Counter regression | Actual application startup/failure/raw-error cases verify available BUS evidence, combined pointer reads, writes, failed/rejected attempts, saturation and retained counts; Arduino excludes preparation and secondary traffic |
| USB loss regression | Pre-fix rx-overflow failed five assertions and rx-overflow-after-read four, including unintended write/reset. Final 12 Arduino application cases PASS in Debug/Release, including active-read completion despite closed input; registered under the existing native owner |
| Final current Arduino links | PASS all four extracted basic/owner S2/S3 images, `build/p38-final/run-xjsuuhsj/firmware/firmware-result.json`; 11 current sources and five public headers per image, 12 ELF/BIN/map hashes independently rechecked in `build/p38-arduino-verification.json` |
| Final current native IDF 5.5.5 links | PASS S2/S3, `build/p38-validation/run-rjz5y8or/firmware/firmware-result.json`; 11 current sources/five headers per image, 10 ELF/BIN/map/SDKCONFIG/lock hashes independently checked in `build/p38-native-verification.json`; native symbols and no Arduino; isolated absolute target configuration paths |
| New native IDF 6.1 links | BLOCKED: no installed local SDK/tool environment, Docker or WSL. Prior CI images remain historical evidence on their tested SHA, not this changed application |
| Physical/flash/serial/HIL | NOT RUN: no identified authorized fixture; host models and linked images do not qualify electrical/USB behavior |

Together the current selections cover all **74 registered Windows tests** in
Debug and Release (73 elsewhere), with the changed registration guard repeated
after its count correction. Scope-specific reports remain separate; an earlier
failed run or probe is not rewritten into PASS. `build/p38-validation-console.log`
and `build/p38-final-console.log` retain command summaries. Ordinary tests never
open a serial port, and fixture absence remains NOT RUN.

Tool inputs: GNU 15.1.0, CMake/CTest 4.0.1, Ninja 1.12.1, Python 3.12.10,
Doxygen 1.13.2, managed PlatformIO 6.1.19, pioarduino 55.03.311 / Arduino 3.3.11 /
bundled IDF 5.5.5; native SDK 5.5.5 uses CMake 4.0.3, Ninja 1.13.1 and Xtensa
GCC 14.2.0. The existing missing standalone IDF constraint-cache waiver is explicit
in the command/report; installed Python requirements still pass their preflight.
No second PlatformIO Core was installed. Windows sanitizer runtimes remain
unavailable; the old Linux sanitizer result is not assigned to this commit.

The first archive (77 files/210 packaged links) has SHA-256
`ca91abd50ff5fba9aa745798d35c22a99bdd64db723850914300c64826be76ae`;
the post-fix Arduino archive has
`b09fc58b3f09094cf332702613b6d52133462d4cdbc7b8014ac073a568a35d28`.
Both clean tests-OFF consumers compile/link/run outside the checkout with used
header provenance checked and maintainer tools absent from their child PATHs.
Firmware identity honestly remains unknown in archives. These exact archives
precede final reporting prose; final package/link checks below cover that prose.

The historical remote checkpoint remains run40782369910 / 14 jobs PASS at
`c50f8ddda8809425ede2161586466c95fa650ae6`. Its eight images, Linux host/sanitizer
and original archive results prove that revision only. No new remote checkpoint
was requested, and missing local 6.1 tools were not masked by that earlier PASS.

Final reporting checks:

```text
python tools/check_doxygen.py
scripts/pio.cmd pkg pack . --output build/p38-handoff/TUSB32x.tar.gz
python tools/check_package.py --archive build/p38-handoff/TUSB32x.tar.gz --out build/p38-handoff/check
```

All PASS. Doxygen checked 36 C++ inputs, real HTML/XML public visibility and
helper comments (`build/doxygen-check/run-fato0fx5`). The final archive has
77 source-equal members and 210 valid package links; the renamed tests-OFF
consumer compiled, linked and ran. Its SHA-256 is
`931c71d7a5cd819d0afa4084854d5a927f342235772cb87c966dc7644d81f2e5`.
`build/p38-handoff/check/package-report.json` and `verification.json` retain
source equality and consumer evidence. All compilation inputs match the selected
current image builds above; only reporting prose differs between these archives.
Reused the maintained Markdown parser for repository targets/fragments and
checked changed-file UTF-8/LF/whitespace plus `git diff --check`. No package,
firmware or physical claim is inferred solely from those text checks.
