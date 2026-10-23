# 34 — Complete the dedicated CI and extracted firmware checkpoint

Deliver working CI for the proven support matrix and verify this prompt's
completed implementation. This is the only remote CI checkpoint in the initial
promptset. Integrate registered checks and package consumers; do not rewrite them
or claim that workflow files alone establish support. Later prompts use local
validation without repeating this remote matrix merely because their commit changed.

## Starting evidence and audit

Read [COMMON](COMMON.md), [AGENT_WORKFLOW](AGENT_WORKFLOW.md), verification rules
in [ENGINEERING_RULES](ENGINEERING_RULES.md) and the initial-promptset exception
in repository AGENTS. Prompts 23–27 supply adapter/example links, prompt 32
supplies clean archives/consumers and prompt 33 supplies registered guards and
validator. Inspect their actual tested pins and remaining gaps before selecting
CI rows.

Re-audit current TMP1x2 host/docs/Arduino/IDF CI, OPT4001 action-pin checks and
LDC1614/INA3221 genuine native-IDF links/test registration. Record HEAD, dirty
state, exact sources and choices. Historic TMP1x2 revision
bafc34067530eb340e55c42dae3fad4eb795b6e9 used PlatformIO 6.1.19 and IDF
v5.3.2/v5.5.1/v6.0.1 for S2/S3; these are comparison inputs, not validated TUSB
pins. Check official SDK/tool/action sources for every selected or changed pin.

## Implement the matrix

1. Select and document supported Arduino/platform/PlatformIO and native-IDF
   versions from current evidence, including a current supported SDK and oldest
   claimed supported SDK. Cover ESP32-S2 and ESP32-S3. Fix failed rows or explicitly
   revise unsupported claims with evidence; never quietly remove a required gate.
   Do not assume sibling green CI transfers to TUSB32x.
2. Create or complete .github/workflows/ci.yml with minimal contents: read,
   explicit job timeouts and separate host/Python/docs/Arduino/native-IDF jobs.
   Use registered CTest/unittest/guard commands from prompt 33. Assign each suite
   one owner per configuration; do not run it through every nested entry point.
3. Use supported strict host flags: -Wall -Wextra -Wpedantic -Wconversion
   -Wsign-conversion -Werror. Add Linux ASan/UBSan with a supported compiler and
   preserve Release test assertions. Do not apply unsupported sanitizers to SDK
   builds. Inspect expected test registrations and nonzero counts in CI too.
4. Pin actions to verified full SHAs with release-name comments and pin platform/
   SDK/tool versions from authoritative sources. Never copy an unverified SHA
   from a historic report. Existing Windows work uses scripts/pio.cmd and the
   managed PlatformIO Core; pinned isolated Linux CI is a separate environment.
5. Build one identified archive and provide it to firmware jobs where practical.
   Link extracted-package Arduino and genuine native-IDF images on S2/S3 for
   every supported row, with isolated build and SDKCONFIG paths per target/version.
   Preserve archive identity and extracted paths. Confirm app_main/native APIs
   and no Arduino component in IDF. SDK-header compilation is not image proof.
6. Let those extracted-package links satisfy firmware and distribution gates for
   the same revision. Do not require another identical source-checkout matrix.
   Earlier source builds prove only their tested commits. Keep a diagnostic
   source-build path documented without calling it an additional final gate.
7. Save bounded artifacts/retention: failed test logs, CTest summaries, Doxygen
   warnings, package-consumer evidence and useful firmware identity/ELF/map files.
   No secrets, unbounded caches, physical HIL, flashing, publication or board-power
   control belongs in ordinary CI. Verify artifact paths exist on both success
   and failure where evidence is expected.

## Verify this checkpoint's implementation

Run focused local workflow/registration checks and affected builds before push.
The lead commits and syncs the coherent implementation, then inspects terminal
results for CI on that exact commit. Use an existing push-triggered run; dispatch
only when needed. Check actual jobs, tested SHA and matrix rows, not a prior badge
or an unrelated green run. Group local fixes and checks before remote execution.
Fix confirmed failures without disabling assertions, tests, warning checks or
triggers. Repeat affected jobs here only for a confirmed failure or changed
workflow/tool/build inputs, with the resulting commit identified. Do not repeat
unchanged suites after each intermediate commit or documentation-only correction.
Retain the actual tested SHA when a later commit only records documentation or
validation evidence; that reporting commit does not require another remote run.

Missing remote/service/tool access is BLOCKED or NOT RUN for that check, not
permission to invent a remote or claim success. Preserve local work, continue
independent fixes and record the exact limitation. A configured workflow and a
successful push are distinct from passed CI. Follow the existing sync contract:
fetch/integrate safely, push, final fetch and verify local/upstream equality and
zero ahead/behind. Never rewrite shared history to make the workflow appear green.

## Delegation and final evidence

Optional: delegate a read-only pin/matrix review or independent artifact/registration
audit to a general coding agent. A faster model can check an agreed row inventory.
Keep workflow-writing ownership clear and Git with the lead; shared contract
changes need lead review and affected tests.

Update README, CHANGELOG, CONTRIBUTING/release instructions, tool matrix, package
commands, coverage, validation and progress with exact commands and actual
PASS/FAIL/NOT RUN/BLOCKED outcomes. Keep firmware links, clean archive consumption,
host tests and remote CI as distinct evidence. Record tested SHA/run links in the
handoff and next progress update rather than creating a self-referencing commit
hash loop. Commit/sync checked units on the same execution branch throughout.
Finish with this checkpoint's branch/commit, verified sync, tested CI SHA/result
and remaining hardware limitations. Later changes do not retroactively extend
that result to a new SHA or create another CI gate. Publication still needs its
own authorization and required checks.
