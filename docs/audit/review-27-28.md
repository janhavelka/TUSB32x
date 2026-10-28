# Independent review of prompts 27 and 28, 2026-10-28

Entry HEAD `e1e79e6fb440fe2c3eb56d2df215b46722dcded3` was clean and synchronized
on `promptset/tusb32x-implementation-20261023`, as checked by the lead. Reviewers
reread the original prompts, current rules, shared contracts, actual implementation
and executable fixtures. Findings below come from current code and reproductions;
earlier audit reports were revisited as source inventories.

## Prompt 27: ordinary C++ ownership

Independent review found no new defect in the simulated host consumer or generic
owner integration. The review covered callback/context lifetime, two-device
fairness, finite poll budgets, retained-result backpressure, exact-token delivery,
cancellation and uncertain effects, explicit invalidation and caller-selected
reconciliation. Host simulation remains visibly separate from hardware proof.

Current source paths and peer decisions are listed in the
[prompt 27 audit](execution-27-peers.md). The independent reviewer inspected
`9e4b08a..19fcd2c`, `examples/host/basic/main.cpp`, `profile.h`, `profile.cpp`,
its CMake/README files, `test/test_owner_scheduling.cpp`, public declarations,
registrations and the integration/coverage guides. The callback gate and borrowed
buffers are released between polls; declaration order keeps context storage alive
through driver destruction. Copied historical snapshots retain invalidation and
trust distinctions. No core scheduler, product queue or simulation shortcut was
introduced.

Fresh GNU 15.1.0 checks passed: a tests-OFF Release consumer configure/build/run in
`build/re27-host`, Debug operations/host/header-contract builds in
`build/re27-owner`, operations and host CTest 2/2, 5,487 owner scheduling checks
with zero failures, and the isolated portable consumer. The latter executed 369
checks with allocation/failure sentinels; its child PATH excluded Git, Python,
Doxygen and PlatformIO. Public-header compilation is compile-only evidence.

## Prompt 28: confirmed findings and corrections

| Requirement | Confirmed gap | Correction and regression scope |
| --- | --- | --- |
| Complete reusable-session proof | Direct API callers could omit finish and leave no required end-marker check | Every Session starts with end_marker NOT RUN; only a verified final marker can pass it |
| Required operation records | Original terminal closure could be accepted without the command table's JOB record | Require original JOB plus RESULT/RESULT_DETAIL/END and preserve interleaved origin/request/token correlation |
| Firmware parser authority | A known route with malformed arguments can legitimately return command=invalid | Accept only exact INVALID_PARAM, unchanged zero-token ACK and matching zero-callback failed END without payload; do not duplicate argument parsing |
| Restorable local staging | Matching restored values alone could conceal unavailable, invalid or variant-mismatched staging | Capture CAPABILITIES; require matching variant, available staging, exact applicable valid_fields and known values before edits, then prove the complete saved profile after restoration |
| Absent staged field | A coherent partial capability/profile mask could omit the specific edited field | Require that setter's own saved field bit before intent or TX; absent fields cannot escape restoration |
| Cleanup contradiction | Failed cleanup readback left the session usable and a later cleanup could produce PASS | Invalidate and checkpoint cleanup failure; prohibit replay through the lost session |
| Expired public deadlines | Deadline rejection before the I/O try block could leave a verified session and owned token unrecorded | Invalidate and checkpoint expired established-session calls without another write; retain unconsumed-token and unrestored-local facts |
| Authoritative save outcome | Failed summary replacement after final PASS JSON left report.json at exit 0 while the tool returned 2 | Keep bounded sticky save_error; attempt one corrective JSON replacement, preserving failure/interruption precedence and prior bytes if storage also rejects correction |
| Saved interruption | Ctrl-C during the final checkpoint escaped with interrupted=false in the prior report | Mark interruption and attempt one final save without serial retry; return 130 and report inability to persist if that save also fails |
| Late public-API saves | Ctrl-C after final-marker closure or result delivery could escape public methods without invalidation | Use one protected checkpoint helper; save interrupted state and exact already-taken result facts |

The save mismatch was reproduced through the actual main function with the reactive
fake peer and injected clock: failing only the final summary replacement returned
tool exit 2 while the authoritative JSON still said PASS/exit 0. A separate one-shot
KeyboardInterrupt during final checkpointing escaped and left the prior JSON at
NOT RUN/exit 2/interrupted=false. These are host artifact defects; neither
reproduction opened a serial port or invented hardware success.

Artifact regressions exercise successful corrective JSON storage, sticky incomplete
outcome after later successful saves, existing failure/interruption precedence,
failed first JSON replacement, failed corrective replacement and temporary cleanup.
The remaining session regressions belong to the integrated runner suite and live
C++ transcript. No device protocol, public C++ API, firmware scheduler or runner
plan was added.

## Sources and peer decisions

Relevant TUSB32x sources reviewed were `tools/tusb32x_hil_runner.py`,
`tools/tusb32x_hil_artifacts.py`, `tools/tusb32x_cli_contract.py`, their unit tests,
`tools/test_cli_transcript.py`, `test/cli/cli_host.cpp`, `test/CMakeLists.txt`,
the original prompt 27/28 files and CLI/maintainability contracts. The
`19fcd2c..HEAD` prompt-28 changes and maintained guides were inspected. No tracked
`.github` workflow definitions exist here; finite local CTest registration is the
current check boundary. The existing cli_transcript registration keeps the actual
built fixture path; hil_runner has a finite timeout and includes artifact tests.

Fresh read-only checks found these peer revisions unchanged and clean:

| Peer | Full HEAD |
| --- | --- |
| FieldCore-node, prompt 27 ownership review | `5ad9292dc8481cf08be9a61f662495a6474d1da0` |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` |

Read each peer's AGENTS.md and checked-in workflow definitions. Exact tooling
paths inspected, relative to each peer root:

- FieldCore-node: `include/TunnelMonitor/i2c/I2cTask.h`,
  `I2cOwnerTransport.h`, `I2cDeviceBinding.h`, `I2cLifetime.h`,
  `include/TunnelMonitor/contracts/Time.h`, `src/i2c/I2cTask.cpp`,
  `test/native/test_owner_i2c/test_i2c_task_fake.cpp` and
  `test/python/test_hil_i2c_startup.py`. Adopt serialized ownership, retained
  effects, explicit invalidation and fresh owner time. Its richer transport
  completion/effect fields do not survive TUSB32x's Status-only callbacks;
  nowUsThunk multiplies milliseconds by 1000 without adding clock resolution.
  Reject product queue, mux, required-device health and retry policy.
- TMP1x2: `tools/hil_tmp1x2_runner.py`,
  `tools/test_hil_tmp1x2_runner.py`, `include/TMP1x2/TMP1x2.h`,
  `examples/common/Tmp1x2Cli.cpp`, `examples/common/Tmp1x2CliOutput.cpp`,
  `CMakeLists.txt` and `.github/workflows/ci.yml`. Adopt remaining-deadline I/O,
  fresh exact markers, short-write rejection and restoration refusal after framing
  loss. Reject hardware profile restoration and sensor-specific timing policy.
- 24Cxx: `tools/hil_identity.py`, `tools/hil_artifacts.py`, `tools/hil_guards.py`,
  `tools/test_hil_artifacts.py`, `tools/test_cli_transcript.py`, `CMakeLists.txt`
  and `.github/workflows/ci.yml`. Adopt explicit unknown firmware identity,
  separate host provenance, authoritative JSON and escaped Markdown. Reject
  EEPROM scratch/guard policy and interpreting post-capture size checks as a
  streaming memory bound.
- LDC1614: `tools/ldc1614_hil_runner.py`,
  `tools/test_ldc1614_hil_runner.py` and `.github/workflows/ci.yml`. Revisited
  TranscriptJournal, provenance, overall_status and requested-gate handling.
  Adopt failure-dominant verdicts and refusal to infer success from missing proof.
  Reject the full in-memory transcript mirror and unrelated suite orchestration.

These unchanged peer conventions did not require another abstraction or dependency.
No sibling files, Git state or builds were changed; no remote CI was queried.

## Verification and limits

| Check | Result | Evidence boundary |
| --- | --- | --- |
| Fresh prompt 27 ordinary C++ and owner checks | PASS | Release tests-OFF consumer; Debug operations/host CTest 2/2; 5,487 owner checks; isolated consumer with 369 checks and active sentinels |
| Entry runner/artifact tests | PASS | 87 existing cases before corrections |
| Corrected artifact suite | PASS | 30 cases after the sticky-save correction |
| Runner plus artifact suite at artifact handoff | PASS | 91 cases before the other session corrections were integrated |
| Focused CTest at artifact handoff | PASS | hil_runner, cli_transcript, cli_contract and cli_contract_negative, 4/4 |
| Scoped artifact diff whitespace | PASS | git diff --check for the two artifact files |
| Final integrated runner and live fixture checks | PASS | 104 Python cases; complete Debug and Release CTest 63/63 each; affected four suites repeated after final fixes |
| Final documentation/package checks | PASS | Doxygen/XML, complete Python docstrings/100-column style, version/no-op sync, 130 Markdown files/779 links, eight vendor hashes, 75 source-equal package files and three isolated consumers |
| Host sanitizers/alternative compilers | BLOCKED | Fresh MinGW link lacks -lasan/-lubsan; cl, clang++ and clang-format unavailable |
| Physical serial, board and electrical qualification | NOT RUN | No port, board, flashing or hardware action |

Final integrated commands, outcomes and synchronization belong in
[validation](../validation.md), the implementation progress entry and final
handoff. Passing fake-serial and actual-C++-fixture tests establish software
behavior only. Observation plans and fixture suites remain later prompts.
