# 33 — Register validation and prove checker failures

Deliver one explicit local validation entry point over existing tests and guards.
Finish missing boundary, version and documentation checks, including evidence
that broken inputs fail. Do not create a second test framework or duplicate the
checks already implemented by earlier prompts.

## Inputs and focused audit

Read [COMMON](../../prompts/COMMON.md), [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md), verification and
packaging sections of [ENGINEERING_RULES](../../prompts/ENGINEERING_RULES.md), and
[MAINTAINABILITY](../../prompts/MAINTAINABILITY.md). Prompts 1–32 supply implemented code,
behavioral tests, version generation, CLI parity, Python runners and packages.
Inspect actual CMake/CTest registrations and guard commands before adding any.

Re-audit TMP1x2 boundary/version/Doxygen checkers and tests, 24Cxx transcript
registration, OPT4001 package/docs guards and applicable sibling validate.py
selection/verdict handling. Record revisions, dirty state, exact sources and
choices. A matching source token or a green script with zero executed tests is
not behavior proof.

## Complete checks without duplicating them

1. Add tools/validate.py and tools/test_validate.py. Provide --checks with explicit
   native, contracts, doxygen, python and package selections, plus names needed
   for the declared firmware matrix. Document the actual accepted names. All means
   all declared local non-HIL release checks; no selection dispatches, polls or
   waits for remote CI. Prompt 34 owns that checkpoint. Narrower selection labels its narrower
   scope. HIL/flashing/publishing never runs implicitly. Missing required tools
   produce INCOMPLETE and nonzero; focused success is not full validation.
2. Reuse registered CTest checks and existing guard commands. Map ownership of each
   suite so native plus python selections do not execute the same tests twice;
   likewise avoid the full suite inside each nested wrapper. Run each requested
   check once per configuration, with finite subprocess timeouts, argument lists
   and sys.executable for Python. Preserve logs and precise failed/missing results.
   Use runner-compatible exit meanings: 0 complete requested PASS, 1 failure,
   2 invocation/missing-tool/incomplete, 130 saved interruption.
3. Add or finish tools/check_contracts.py for core framework/time/allocation
   boundaries, include hygiene, metadata and source registration. Negative fixtures
   demonstrate detection. Keep meaningful compiled/behavioral portability and
   allocation tests; text searches supplement them rather than replacing them.
4. Reuse tools/check_cli_contract.py from prompt 22, executing real help/aliases/
   records/parity through tusb32x_cli_host. Do not replace it with static keyword
   checks or create a second contract-table implementation. Keep its broken-case
   fixtures and exact built executable path.
5. Finish tools/test_version_sync.py for generate_version.py sync/check/bump/set:
   malformed versions, header/CMake/component/Doxygen drift, idempotence and stable
   committed generation. Archives must compile with unknown Git identity without
   requiring generators. Do not bump a release merely to finish this prompt.
6. Add tools/check_doxygen.py and tools/test_check_doxygen.py around actual
   Doxygen execution. Warnings fail. Check missing public API/source helper docs,
   hidden public APIs, broken refs and generated-file boundaries with negative
   fixtures. Fix omissions in source; do not suppress warnings or mark public
   declarations internal to make them disappear. Source documentation is required
   now; prompts 35–36 reconcile it rather than create it from scratch.
7. Reuse hardened package tests and clean consumer from prompt 32; Python parser/
   session/plan tests and actual C++ transcript from prompts 28–31; and linked
   framework consumers from their stages. Record their distinct evidence levels.
   Checkers must fail on their own malformed input or unavailable required tool,
   not print SKIP and turn full validation green.

## Registration and executable acceptance

Keep CMake tests opt-in with TUSB32X_BUILD_TESTS=ON; Python discovery belongs inside
that block. Retain assertions in Release tests. Use standard CMake/CTest/unittest,
finite per-test timeouts and $<TARGET_FILE:tusb32x_cli_host>, never guessed .exe
paths. Register validate_runner and all new guard tests without duplicating
hil_runner or cli_transcript. Inspect ctest --show-only=json-v1 for expected core,
protocol, owner, variants, CLI, console, portability, Python and guard coverage;
zero tests or omitted required groups fails verification.

Test validator invalid selection, missing executable, failed child, timeout,
interruption, partial completion and duplicate-check prevention. Run focused
negative tests first, then python tools/validate.py --checks all when required
tools are available. Avoid rerunning the same full suite immediately through
another entry point. Keep exact PASS/FAIL/NOT RUN/BLOCKED evidence for tool gaps.

## Delegate and finish

Optional: delegate Doxygen/version negative fixtures and independent registration
review to general coding agents with disjoint ownership; a faster model suits
fixed manifest comparisons. The lead owns check selection, verdicts and Git.

Update Python docstrings, source Doxygen fixes, README, CHANGELOG, CONTRIBUTING,
validation commands/tool prerequisites, coverage and implementation progress.
Fix real checker/registration failures without weakening gates. Commit/sync
coherent checked units and the finished prompt on the same branch, verify
upstream equality, and report requested-check scope plus concrete missing evidence.
