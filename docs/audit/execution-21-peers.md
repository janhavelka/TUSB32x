# Prompt 21 current peer audit

Read-only source inspection on 2026-10-28 for managed CLI operations, result
ownership and timing. TUSB32x started at
`072015c5b5d977bfe81d42978bbb587b5b8e2fea`, with empty short status, on the
existing execution branch. The lead owns Git synchronization. Paths below are
relative to `C:/Users/Honza/Documents/Projects/`. No sibling was edited, built
or tested; no remote CI was queried.

Read parent/local AGENTS, prompt index, COMMON, AGENT_WORKFLOW, prompt 21,
CLI_CONTRACT, ENGINEERING_RULES ownership/evidence/silicon sections and
ENGINEERING_BASELINE state/effect/time contracts. Current progress and design
decisions supply prerequisite evidence, not replacement evidence for this audit.

## Current revisions and sources

All four peers had empty `git status --short` output. Each current `AGENTS.md`
and `.github/workflows/ci.yml` was inspected. Their Git/flash policies are not
permissions for this repository.

| Peer | Current HEAD | Exact additional paths inspected |
| --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `include/TMP1x2/TMP1x2.h`; `examples/common/Tmp1x2Cli.cpp`, `Tmp1x2CliDiagnostics.cpp`, `Tmp1x2CliOutput.cpp`; `test/cli/test_cli.cpp`; `tools/hil_tmp1x2_runner.py`, `tools/test_hil_tmp1x2_runner.py`; `CMakeLists.txt` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | `include/EEPROM24Cxx/EEPROM24Cxx.h`; `examples/common/Eeprom24CxxCli.cpp`; `test/test_cli/test_cli.cpp`; `tools/hil_runner.py`, `tools/test_hil_runner.py`, `tools/test_cli_transcript.py`; `CMakeLists.txt` |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `include/LDC1614/LDC1614.h`; `examples/common/Ldc1614Cli.cpp`; `test/test_cli.cpp`; `tools/ldc1614_cli_contract.py`, `tools/ldc1614_hil_runner.py` |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | `include/INA228/INA228.h`; `examples/01_basic_bringup_cli/main.cpp`; `tools/run_i2c_hil.py`, `tools/test_run_i2c_hil_parser.py`, `tools/test_cli_regressions.py` |

LDC1614 remains newer than the historical CLI_CONTRACT source table. Decisions
here use its current `1db38b6` implementation.

TUSB declarations/behavior were checked in `include/TUSB32x/TUSB32x.h`,
`src/TUSB32xOperations.cpp`, `test/test_operations.cpp`, and the existing
`examples/common/Tusb32xCli.h`. `test/CMakeLists.txt` registers the actual shared
CLI sources, console suite and persistent Python transcript against the C++ host.
There is currently no `.github/` directory in TUSB32x; no workflow is implied.

## Concrete choices

- TMP1x2 `finishOperation()` automatically takes the core result and saves
  `_lastOperation`. 24Cxx `complete()` also takes automatically and saves
  `_lastResult`, potentially advancing scratch/repeat work. Reject both designs
  for TUSB32x: one core result remains pending until explicit exact-token take;
  announcements and repeated queries only snapshot it. Keep request/nonce/token
  correlation metadata, not a second result cache.
- TMP1x2 manual tests cover zero, one and larger callback budgets, cancellation
  before/after writes and deadline rollover. Adopt their exact delta assertions
  and failure preservation. Reject their expectation that budget three executes
  more than one callback: TUSB's Status-only callback requires a fresh owner
  timestamp after each invocation. Budget 255 is still a ceiling of one callback
  per poll, and successful terminal publication may require a zero-budget poll.
- 24Cxx scratch tests preserve the primary outcome independently when explicit
  restoration fails. Adopt this evidence distinction: delivery/cancellation or
  later observation cannot rewrite an earlier write outcome. Reject its scratch
  backup, firmware repeat scheduler, implicit child-result consumption and
  restoration commands. TUSB cancellation/end never schedule compensating writes.
- LDC1614 tests prove staged setters have zero transfers and report staged versus
  desired values separately. Adopt these distinctions and transactional candidate
  edits. Reject `profile commit confirm`, init replay and cross-field-invalid
  staging: its `sensorbounds 0 5000000 100000` deliberately stages an invalid
  candidate for later validation. TUSB validates before staging, captures desired
  only on accepted apply admission, and initializes read-only.
- INA228 `V3_TRANSFER_STEPS` checks exact zero/one/N deltas and that BUSY scan,
  selftest and recovery leave the existing owner intact. Adopt exact deltas and
  owner preservation. Its assertion arguments mean read/write/total, and its
  sample/verify engine can consume larger budgets; neither grammar nor numbers
  carry over to TUSB. Its source-extraction tests supplement, not replace,
  execution of TUSB's actual shared CLI and scripted transport.

## Failure and cleanup cases actually read

TMP1x2 `tools/test_hil_tmp1x2_runner.py` includes
`test_failed_restoration_retains_captured_profile_and_fails_report`: injected
disconnect leaves restoration unverified, reports FAIL and retains the original
profile. `test_failed_owner_suite_restores_automatic_polling_and_baseline` checks
an interrupted manual job against the real CLI fixture. The partial-write
cancellation cleanup test inspects CANCELLED evidence; lost-framing and reboot
tests prohibit further cleanup writes. Preserve failure/session evidence here;
do not import automatic hardware restoration into TUSB's CLI or future runner.

24Cxx `test/test_cli/test_cli.cpp` injects an ambiguous pattern-write failure,
then separately a failed restoration after primary success. It checks retained
backup, original primary status, no automatic retry and explicit restoration.
Cancellation during restoration preserves `primary=OK restore=CANCELLED` and
does no new transfer. `tools/test_hil_runner.py` verifies preflight failure sends
only `job`, never cancels foreign work, and records cleanup NOT_SENT. Its cleanup
error test preserves the original serial-disconnect reason; serial-close failure
does not weaken an already proven FAIL verdict. These are source inspections,
not executed peer test results.

TUSB `test/test_operations.cpp` already executes exact-token rejection,
RESULT_PENDING admission fences, repeated snapshots, one successful take and
unchanged delivery outputs on errors. It asserts one callback for budgets
1/2/255, immutable deadlines, zero-budget expiry, clipping before narrowing,
and final success rejected at/effectively after deadline. CLI tests must retain
these semantics while additionally proving output reservation and framing.

## Integration review priorities

1. Use one poll call site after dispatch; an explicit zero step suppresses that
   turn's automatic budget. Cached/invalid commands remain bus-silent handlers
   while independently admitted automatic work can progress afterward.
2. Reserve the complete take response before consuming, including copied result
   records and END. Output loss cannot remove the only retained result or defer a
   destructive take. Terminal announcement loss does not justify repeated ENDs.
3. Copy each admitted request/nonce/token; later markers and queries cannot
   relabel the original completion. Take delivery OK and payload failure are
   independent. Failed admission has no token or invented terminal result.
4. Obtain time from the owner hook for each actual poll. Do not renew deadlines,
   reuse pre-callback time as completion, or hide original transport failures
   under later TIMEOUT. Keep uncertain effects and current trust separate.
5. Route all silicon operations through existing core APIs. IRQ ACK uses actual
   observed/trusted controls rather than staging. Recovery remains explicit and
   capability gated; this CLI stage introduces no register sequence.

Peer workflow inspection confirms native CLI/runner registration and strict
warnings/sanitizers in TMP1x2/24Cxx, and PlatformIO native/source-contract checks
in LDC1614/INA228. Reuse executable registration and applicable warning policy;
do not import their dependency requirements into tests-OFF C++17 consumers.
Framework links, remote CI and physical qualification were not run by this audit.

Final accounting review recommended removing the synthetic oversized enqueue on
take-reservation refusal. The lead applied the correction and added a near-full
runtime case: complete BUSY rejection with unchanged loss counters, retained core
slot and later successful take. Actual failed record enqueues still retain LOSS.

## Independent integration review

Reviewed the actual changes to `examples/common/Tusb32xCli.h`,
`Tusb32xCli.cpp`, `Tusb32xCliOutput.cpp`, `test/cli/test_cli_operations.cpp`,
`test/cli/cli_host.cpp`, `tools/tusb32x_cli_contract.py` and
`tools/test_cli_transcript.py` against the core declarations/tests above.
The lead and test owner corrected these confirmed findings during integration:

- `tick` discarded a rejected core poll and closed explicit stepping with OK.
  Backward/half-range owner time now remains bus-silent and unchanged while JOB
  reports the rejection and the explicit step END fails. Automatic rejection
  also reports its typed status. Terminal operation failures remain separate
  from successful step delivery. Regression cases exercise both owner modes.
- Successful result delivery validation accepted ACK/END without the copied
  RESULT/RESULT_DETAIL. An executed Python negative reproduced this gap. The
  validator now requires exactly one of each payload, correlates token/origin,
  and rejects contradictory cancelled/timed-out result states. The step-specific
  ACK/END status difference requires a matching rejected active JOB record.
- `job.step` metadata described a cached read although the selected core poll
  can write. It now conservatively declares DEVICE_WRITE and explains that
  advancing an already admitted operation may read or write.

The result remains exclusively in core. Take uses a transient stack copy and
reserves five maximum-sized records, or eight when the original completion has
not yet been announced. No drain or reentrant callback occurs before the
reserved response is queued. Failed reservation retains the core slot and loss
evidence; spontaneous completion is attempted once, even if its records are
dropped. Original request/nonce/token survives later marker/query traffic.
The sole poll call site samples owner time and stops after the core's one
callback ceiling. Automatic work matches the CLI-admitted token; explicit
stepping can intentionally advance a represented application operation.

During review, the focused Debug CLI/console/transcript CTests passed 3/3 and
`git diff --check` passed. Additional fixes and regressions were inspected after
that run; the lead owns the final rebuilt validation matrix and records its
exact results. This review supplies no framework, hardware or remote CI claim.
