# P11 - Implement strict Python evidence checks and explicit HIL suites

Read [COMMON.md](COMMON.md), [CLI_CONTRACT.md](CLI_CONTRACT.md),
[ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md),
[MAINTAINABILITY.md](MAINTAINABILITY.md) and [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md).
The shared CLI is the executable protocol. Familiar words in a log do not prove
the selected test completed.

## Fresh audit and concrete files

Re-read current TMP1x2 `tools/hil_tmp1x2_runner.py`/tests for remaining deadlines,
markers, identity and restoration; 24Cxx `hil_runner.py`, `hil_identity.py`,
`hil_artifacts.py`, `hil_guards.py`, `test_cli_transcript.py` and CMake registration;
LDC1614 contract/provenance/verdict logic; INA228 exact callback-budget/owner-
exclusion suites. Compare MB85RC/RV3032 strict parsing where useful. Cite current
revisions/dirty state/files and choices. A bounded transcript retained wholesale
in RAM is not a reason to copy a growing in-memory journal. Inspect the actual
TUSB CLI code and records first.

Create `tools/tusb32x_hil_runner.py`, reusing `tools/tusb32x_cli_contract.py`.
Begin with ordinary functions/small data classes; extract `hil_identity.py` and
`hil_artifacts.py` only when their separate responsibilities improve review.
No generic HIL engine or EEPROM scratch-window policy. Add:

- `tools/test_tusb32x_hil_runner.py`: parser/session/planning/fake-serial tests;
- `tools/test_cli_transcript.py`: persistent real C++ CLI integration;
- `tools/validate.py`, `tools/test_validate.py`: explicit check selection and
  missing-tool/failed-command behavior, invoking the registered CTest checks and
  existing guard commands instead of implementing them again;
- `tools/requirements-hil.txt`: audited pinned optional pyserial dependency.
  Standard-library parser tests work without pyserial or hardware.

Use sibling-style `snake_case`, uppercase constants, pathlib, argparse, unittest
and argument-list subprocess calls with explicit timeouts. Child Python uses
`sys.executable`; never interpolate user values into shell text. Write module/
function/exception docstrings for schemas, deadlines, verdicts and cleanup.

## Serial session and evidence

1. Require explicit `--port` for serial work; provide `--baud`,
   `--command-timeout`, `--write-timeout`, `--variant`, `--address`,
   `--expect-commit`, `--expect-framework`, `--expect-target`, `--suite`, `--out`,
   `--dtr`, `--rts`, `--help` and `--dry-run`. Validate before open. No port
   enumeration/probing, flashing, hidden reset or implicit mutating selection.
   Dry-run saves the plan and reports hardware NOT RUN, never simulated PASS.
2. Configure/report DTR/RTS before open where supported (`on|off|unchanged`), with
   documented defaults/backend limits. Detect reboot/startup/reconnect. Avoiding
   a reset command is not proof that opening serial leaves the board alone.
3. Generate fresh random begin/end nonces. Accept only exact MARK frames with
   expected canonical command/request/token and exactly one original terminal
   RESULT/END pair for admitted work. Query repetition is not a new completion.
   Consume retained results with `result take <token>`. Stream stale input but
   never let it certify a new request. Require increasing new-input ACK/MARK
   request IDs, while accepting an older admitted request's JOB/RESULT/END after
   a newer query response. Correlate by original request/token; never treat that
   legal interleaving as a reboot. Reject session exhaustion without ID reuse.
4. Enforce one monotonic host deadline before writes/reads and after returns.
   Remaining budget covers fragments/markers/completion; no new full timeout per
   read. Late success fails. Bound line/response bytes/count even for endless
   ANSI or boot noise. Reject missing/duplicate/contradictory/truncated records,
   wrong tokens, regressed new-input ACK/MARK, restart, malformed values and
   output loss. Handle the reserved pre-marker nonce and command=invalid exactly.
5. Parse machine schema from CLI_CONTRACT. Match exact typed statuses/states/
   effects; human OK/PASS is supplementary. Expected rejection passes only its
   own command with expected state/callback delta. Do not infer physical bus
   transaction count from terminal callback attempts.
   Exact handler callback-delta assertions require an idle core or automatic
   stepping disabled. Account for independent progress of an older operation
   when testing cached/rejected commands with automatic stepping enabled.
6. Record host commit/dirty state separately from firmware library version,
   commit/tree/build, framework/version and target. Labels do not prove firmware.
   Unknown identity cannot satisfy `--expect-*` or identity-required mutation
   suites. Check variant/address, applied profile/trust/generation and attachment
   before writes; revalidate after reset/reconnect before any cleanup. Port facts
   require the relevant validity bits and `invalidated=0`; a recent timestamp
   alone cannot make an invalidated historical observation usable.
7. Stream timestamped TX/RX to disk, retaining a fixed tail and aggregate stats
   only. Defaults: response 256 KiB, run transcript 32 MiB, retained tail 64 KiB;
   any overrides have finite documented ceilings. Cap hits produce incomplete/
   failed evidence, not silent success. Atomically replace `report.json` and
   `summary.md` via a same-directory temporary file. Checkpoint before mutations
   and on interruption/exception so partial evidence survives.
8. Keep primary outcome, cleanup outcome and residual hardware uncertainty
   independent. Failed test plus good cleanup stays failed. Success plus missing
   cleanup proof stays incomplete. Per-check PASS/FAIL/NOT RUN/BLOCKED is explicit;
   required NOT RUN prevents overall PASS. Exit 0 only for complete PASS, 1 for
   test/protocol failure, 2 for invalid invocation/missing tool/incomplete run,
   and 130 for interruption after evidence is saved.

## Explicit suites

Implement repeated sampling, stress, register dumps and fixture procedures here
as finite sequences of ordinary CLI commands. Complete and explicitly take each
operation's result before starting the next. A register dump uses a bounded,
variant-allowlisted sequence of reg commands with documented read effects.
Keep run counts, intervals, whole-run deadlines and summaries on the host; no
firmware bulk commands, child-result consumption or second result/token scheme
is required. Stop on unexplained failure. Read-only verification aliases require
an existing retained desired profile; a readonly suite must report that
precondition or test its rejection without applying a profile to satisfy it.

| Suite | Required evidence |
| --- | --- |
| `readonly` (default) | Identity, cache-only routes, live identify/status, zero/one-callback budgets, malformed-input fences, staging/validation without apply, retained-result handling and provenance |
| `soak` | Explicit finite duration/count, read-only polling, owner/console fairness and monotonic observations. Unchanging detached state is valid; observation sequence proves freshness |
| `orientation` | Reversible connector fixture, both orientations, local role/orientation validity and board/fixture identity |
| `role-current` | Fixture/power capability, local sink/source/DRP, supported CC advertisement categories, stage/apply/verify evidence; no inferred PD/current measurement |
| `interrupt` | Wired INT, assertion/ack/reassertion, retained events, staged!=applied 0x09 preservation and independently verified clear semantics; RCU alone is not read-to-clear evidence |
| `reset-termination` | Explicit sequence/reset waits, timeout/cancel/effect uncertainty and legal reconciliation without blind replay; original TUSB320/TUSB320I dead-battery recovery through recover-dead-battery under its source-capable fixture prerequisites, with denial on other variants |
| `fault` | Operator/fixture-induced NACK/disconnect/power cycle, trust invalidation, owner responsiveness and new identity checks; no automatic supply/VBUS manipulation |
| `console` | Authorized physical reconnect/backpressure, queue/loss reporting, reboot detection and framing re-establishment; native zero-write model is separate proof |
| `accessory` | Exact supported fixtures plus capability denial on other variants; absent fixture is NOT RUN |

Accept repeated `--suite`; hardware mutation suites require `--fixture <id>` and
their declared setup prerequisites. No generic `--all` silently includes writes.
Capture identity/profile/attachment baseline. Restore only while target,
attachment and supply assumptions permit it; otherwise report blocked/uncertain
cleanup and manual reconciliation facts. Never restore an old source role,
replay uncertain role/reset/apply or imply readback undoes a real USB event.

## Tests and acceptance commands

Inject fake serial and monotonic clock. Cover fragmented ANSI/CRLF, stale marks,
echo-only OK/PASS, missing/duplicate/out-of-order records, identity change, boots,
token reuse, expired writes/late replies, short writes, exceptions/missing ports,
pre-marker nonce, invalid commands, request exhaustion, older completion after
newer query, output-full result take, invalidated historical port data, capture
caps, Ctrl-C, atomic reports and cleanup eligibility. No wall-clock sleeps
are needed for parser tests. Test every guard's negative path.

Run one persistent `tusb32x_cli_host` with real shared CLI and scripted hardware,
including failed writes, staged!=applied IRQ ACK and full readonly plan execution.
Good hand-written transcripts alone are insufficient. Register CTest `hil_runner`
and `validate_runner`; extend the existing P08 `cli_transcript` registration
rather than registering a duplicate. Use finite timeouts and the exact built
fixture path supplied by CMake. These are future acceptance commands:

```powershell
python tools/tusb32x_hil_runner.py --help
python -m unittest discover -s tools -p test_tusb32x_hil_runner.py
ctest --test-dir build/native -C Debug -R '^(hil_runner|cli_transcript|validate_runner)$' --output-on-failure
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite readonly --out build/hil-plan
python tools/validate.py --checks native,contracts,doxygen,python,package
```

Document operator-edited hardware commands with real target/port/expected commit;
fabricated COM ports/SHAs are not run evidence. HIL never runs implicitly through
validate.py or a normal test invocation.

## Documentation and handoff

Write `docs/hil-runner.md` for flags/schema/verdicts/artifacts, and
`docs/hardware-validation.md` for per-variant wiring, straps, fixture/power
conditions, command plans, cleanup and NOT RUN criteria. Update README, CHANGELOG,
CLI links, coverage, validation and phase progress now; maintain Python docs and
source Doxygen when helpers change. Report parser/fake-serial/native-fixture PASS
separately from serial, physical I2C framing and CC/INT/connector validation.
Missing hardware stays visible through final audit.
