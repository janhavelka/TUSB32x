# 38 — Review integration and finish the promptset

Read [COMMON.md](COMMON.md). Prerequisites: prompts 1–37, including the completed
core review and fixes. Deliver an independently reviewed, validated library with
an honest final report. This prompt closes integration/distribution gaps; it does
not repeat unchanged checks simply to produce more logs.

## Audit and delegation

Re-audit current peer CLI, adapters, Python runners, clean consumers, CI and
documentation practices. Use the refreshed inventory from prompt 37 and inspect
changes relevant to this review. Read CLI_CONTRACT, distribution rules and
actual final code, tests, metadata and artifacts.

Use a coding reviewer for CLI/adapters/runners and a separate distribution/docs
reviewer when available. Give read-only scopes initially, select complexity-
appropriate supported models under AGENT_WORKFLOW, and assign fixes explicitly.
The lead integrates, validates and owns Git on the same execution branch.

## Review and close gaps

1. Compare prompts 20–36 and the full requirement/feature coverage ledger with
   actual outputs. Check framework neutrality, naming/ownership/status consistency,
   examples, CLI parity, Python suites, docs and package contents. Confirm core
   findings from prompt 37 remain fixed. Missing implementation is work to finish.
2. Execute hostile CLI and fake-serial cases: strict grammar, cached/staged/live
   semantics, no unintended writes, stale/duplicate/late/rebooted records, token
   ownership, partial output and full queues. A delayed old operation completing
   after a newer query is valid, not a reboot. Result take reserves its full
   response before consumption; manual step replaces automatic polling.
3. Verify Arduino/native-IDF frontend parity, borrowed resources, one owner poll
   path, finite timeouts and cleanup. USB stalls/reconnects cannot block owner
   progress or permit stale success. Nearly expired/coarse timeout handling
   remains correct in the real selected SDKs.
4. Check host versus flashed-firmware identity, DTR/RTS session changes, suite
   completeness, bounded logs/soak memory, unknown/incomplete verdicts and exit
   codes. Cleanup cannot blindly restore against changed identity, session,
   attachment or uncertain state. Missing fixtures stay NOT RUN.
5. Inspect hardened archive extraction, clean CMake/PIO/IDF consumers, manifests,
   generated versions and package links. Ordinary core consumers require no
   sibling, Git, Python or Arduino. An extracted firmware build may prove both
   package and firmware requirements when its evidence explicitly covers both.
6. Check registered suites actually run, Release assertions remain active,
   documentation guards detect hidden APIs and missing comments, and generated
   workflow files are not reported as executed CI. Verify every claimed
   framework/target against linked image evidence.
7. Remove duplicate state/protocol logic, needless abstractions and stale docs
   found during review. Each correction includes its meaningful regression,
   Doxygen/docstring and affected guides/CHANGELOG in the same unit.

## Final verification and evidence

Re-open [CHIP_EVIDENCE](CHIP_EVIDENCE.md)'s sources for every claimed board or
adapter behavior. Check actual seven-bit addresses, explicit pointer framing and
aggregate terminal timeouts; repeated-START code elsewhere is not qualification.
Challenge supply/strap/VBUS_DET guidance, especially the separate 322 requirements,
local 3 A advertisement versus remote detection, and typical versus maximum
CC/VCONN values. A current register read is not proof of updated physical
detection; base explicit-reset requirements must not become hidden runner policy.
Fix copied errors in examples, tests and guides together. Keep code review,
linked firmware and exact-board traces separate in the final support matrix.

Extend `docs/final-audit.md` using the existing finding format and record reviewed
revision/dirty state, reviewers, fixes, exact evidence and remaining limits.
Reconcile the progress ledger, source/API/CLI/test coverage and validation report.
No locally fixable implementation defect or failed mandatory check is silently
carried into a COMPLETE status.

Run the required final matrix after fixes: registered host/CLI/Python/guard tests,
Doxygen, clean package consumers and the supported Arduino/native-IDF builds.
Reuse current evidence when source/tool inputs are unchanged and the requirement
allows it; repeat affected local checks after fixes. Do not dispatch, poll or wait
for remote CI in this prompt. Report prompt 34's actual tested SHA/result
separately from this final commit's local validation; the earlier run does not
verify later changes. Unrequested remote CI is not a completion gate. Report
unavailable required tools and physical fixtures accurately.

## Finish this promptset

Review and **commit and sync** coherent final fixes on the recorded execution
branch. Fetch/push and verify equal commits, zero ahead/behind and no uncommitted
task changes. Report concrete failures without claiming sync; never force-push,
create a new branch for this final step or publish a release/tag without authority.

Final handoff names the implemented family/capability limits, portable ownership,
examples/CLI/runners, actual host/build/package/CI results, hardware evidence and
remaining blockers. Link final audit and validation. Do not call all variants or
boards hardware-qualified because only LAI or one board was exercised. Record
completed commit IDs without creating a self-referencing hash-update loop.
