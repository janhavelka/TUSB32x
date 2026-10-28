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
