# Preparation validation

Date: **2026-10-23**. These results validate the audit/promptset artifacts only.
There is no driver implementation in this checkout to compile or hardware-test.

## Initial preparation checks

| Check | Result | Scope |
| --- | --- | --- |
| Prompt sequence | PASS | Fifteen files, numbered 00 through 14, all listed in the promptset index |
| Shared contract | PASS | Every numbered prompt imports COMMON.md and requires renewed sibling auditing |
| Local Markdown links | PASS | All local Markdown targets resolve in this checkout |
| Text integrity | PASS | UTF-8 readable; no replacement characters; fenced blocks balanced |
| Vendor archive | PASS | Seven PDFs parse, 195 pages total; each byte count and SHA-256 matches provenance.json |
| Repository snapshots | PASS | Seventeen I2C libraries and FieldCore-node; sampled HEAD/dirty state unchanged at final comparison |
| Independent convention review | COMPLETE | Public layout, ownership, API/status, health, examples/CLI, tests, runners, tooling and package coverage |
| Independent consumer/tooling review | COMPLETE | FieldCore independence, generic consumers, callback budgets/timeouts, diagnostics and evidence verdicts |
| Independent TI review | COMPLETE | Family boundaries, addresses/identity, masks, W1C/reset, timing and documented ambiguities |

Mechanical checks used Python's pathlib/re/json/hashlib/subprocess, installed
PyMuPDF for PDF parsing, and read-only `git rev-parse` / `git status --porcelain`
against the sibling snapshots. No persistent test framework or driver test was
added for these documentation-only checks.

## Review corrections incorporated

- Defined a poll transfer budget as terminal callback attempts; a STOP-separated
  register access may contain two bus transactions. Adapter transaction counts
  and callback/managed-health counts are separate, with one aggregate timeout.
- Health is neutral diagnostics plus optional passive projection; an OFFLINE label
  cannot suppress an explicitly requested legal operation or schedule retries.
- Hardware waits start from completion/fresh time evidence; callbacks cannot
  consume a deadline unnoticed or shorten a required delay.
- Interrupt acknowledgement preserves observed/trusted applied RW settings,
  never unapplied staged settings. Explicit regression case required.
- Python serial runners specify DTR/RTS/open/reconnect behavior and invalidate
  evidence after a detected reboot; avoiding a reset command is not a guarantee.
- TUSB322I power effects are documented separately from application-owned external
  power policy; no false guarantee that role operations cannot affect VCONN.
- Repeated START and interrupt-clear timing are not invented TI requirements.
  Startup/current-tracking/access-wording conflicts remain visible.

## Not performed or claimed

- TUSB32x implementation, native tests, Arduino/native-IDF firmware builds,
  package compilation, CI or physical hardware tests.
- Sibling builds/tests, modifications, upstream synchronization, serial probing,
  flashing, publication, commits or pushes.

Those checks/actions belong to execution of the implementation promptset and its
actual environment/authorization. The local folder initially had no Git metadata;
the deliverable is the files in this checkout.

## Expanded promptset validation — 2026-10-23

The follow-up request expanded all fifteen stages and added ENGINEERING_BASELINE,
CLI_CONTRACT, MAINTAINABILITY and AGENT_WORKFLOW. This checkout now has Git
metadata; the expansion was prepared against `7c1e21b` without committing or
publishing. The earlier tables describe the initial preparation, not an assertion
that every neighboring checkout stayed unchanged during this later work.

| Check | Result | Current scope |
| --- | --- | --- |
| Stage sequence and recurring requirements | PASS | All 15 stages are indexed, import COMMON, require renewed audit and cover documentation |
| Local links and text | PASS | 30 Markdown files, 117 local links; UTF-8 readable and fences balanced |
| CLI record examples | PASS | 11 records across prompts; ASCII, unique keys, longest 273 bytes including newline against the selected 384-byte limit |
| Archived TI references | PASS | Seven PDFs, 195 pages; byte counts and SHA-256 still match provenance.json |
| API/CLI/state consistency review | COMPLETE | Status/admission/result separation, enum/field names, health, profile staging, CTest names and output validity aligned |
| Protocol/owner review | COMPLETE | W1C composition, uncertain effects, termination controls, fresh completion time and millisecond quantization reviewed independently |
| Sibling device libraries | PASS | All 17 I2C libraries remain at their recorded HEAD and clean at final comparison |
| FieldCore consumer | CHANGED EXTERNALLY | Final HEAD `154a4dcf62d5d439954ae20e6ffff3f402433db1`, clean; see scope below |
| Whitespace | PASS | `git -c core.autocrlf=false diff --check`; changed Markdown normalized to UTF-8 LF |

FieldCore-node advanced from the recorded `7185a6e` revision during this work.
The observed diff concerns calibration workflows, calibration/serial runners and
their documentation/tests. Its cited I2C owner transport, binding, lifetime,
device wrapper and ownership/health files did not change. This task did not write
to any sibling. The original repository snapshot remains dated evidence; every
implementation stage must still inspect current sources.

Three bounded subagent tasks covered API/health, CLI/tooling, and TI/protocol/tests.
The first two used `gpt-6.1-sol` with high reasoning; the third used `gpt-6-astra`
with high reasoning for variant and timing analysis plus an independent final
review. File ownership was disjoint. The lead integrated returned changes,
checked the resulting contracts and ran the mechanical checks above.

Corrections from this review are explicit requirements and future regression
cases, not claims of passing driver tests:

- Admission reports IN_PROGRESS; successful result delivery reports OK while
  the copied operation result may still contain a failure.
- PORT output uses the exact typed enum vocabulary and validity masks. Detached
  is an attachment state, not an invented role or blanket invalid observation.
- CLI staging passes directly to an accepted apply; no redundant commit layer.
  Raw aliases share one documented diagnostic route and count in source totals.
- Termination evidence accounts for independent 0A and 45 disable controls.
- A one-millisecond software margin prevents floor-rounded clock values from
  shortening required hardware waits; it never extends the owner deadline.
- A late callback success becomes owner TIMEOUT; an actual callback failure
  keeps its original I2C error and backend detail.
- TUSB322I RCU wording does not prove read-to-clear behavior. Unresolved actions
  remain gated while their documented evidence is retained.
- Source Doxygen and general documentation accompany each implementation stage;
  missing-comment negative fixtures prevent a hidden-API build from passing.

No driver code, native/firmware build, physical test or remote CI was run for
this documentation expansion. Those remain implementation-stage work.

## AGENTS and Git workflow follow-up — 2026-10-23

Compared current AGENTS.md guidance in TMP1x2, 24Cxx, LDC1614, LSM6DS3TR,
INA3221 and ADS1115. All six remain clean at the revisions recorded in the
repository snapshot. Selected rules and deliberate exclusions are recorded in
the audit index. Siblings were read only.

Expanded TUSB32x AGENTS.md and aligned COMMON, the master prompt, delegation,
repository setup and final-audit instructions. Routine commits/pushes are now
explicitly authorized after every prompt and at complete checked units during
prompts. The whole promptset stays on its existing branch. Sync includes upstream
integration where safe, push and verification; local commits alone do not qualify.
Concurrent edits, failed synchronization and separate release actions have
explicit handling. Parent CI policy remains unchanged.

Independent read-only review found no copied sensor/API/health conflicts. Two
wording fixes preserve legitimate cooperative owner loops and permit independent
local work during an actual synchronization failure without hiding that failure.

Focused checks passed: 30 Markdown files; 127 local links including the new Git
workflow anchor; balanced fenced blocks; UTF-8 text; fifteen indexed prompts with
COMMON imports; changed Markdown in LF; `git -c core.autocrlf=false diff --check`.
The existing `main` branch and `origin/main` upstream were fetched and compared
before editing. The final handoff supplies the resulting commit and verified sync
state, avoiding a self-referencing commit hash in this file.

This remains documentation work. No driver tests, firmware builds, remote CI,
hardware operations or release publication are claimed for this follow-up.
