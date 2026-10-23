# P08 - Implement the shared diagnostic CLI and execute its contract

Read [COMMON.md](COMMON.md), [CLI_CONTRACT.md](CLI_CONTRACT.md),
[ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md),
[MAINTAINABILITY.md](MAINTAINABILITY.md) and [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md).
This phase creates working example code and tests. Another command wishlist does
not complete it.

## Re-audit before editing

Record HEAD, dirty state and exact files re-read for TMP1x2 and 24Cxx shared CLI,
console, native fixture, Python transcript test, CMake and CI. Compare SHT3x
dispatch, LDC1614 command rows/staging and INA228 transfer assertions. Read an
actual parser rejection test and a retained-result/failed-cleanup test. Start
with revisions/files in CLI_CONTRACT, but cite current source. Inspect Arduino
and IDF entry points: some sibling common directories contain framework APIs;
some `init` commands replay profiles. Do not import those behaviors or change
the agreed startup sequence in this phase.

## Implementation order

1. Create `examples/common/Tusb32xCli.h/.cpp`, `Tusb32xCliOutput.cpp`
   and `ConsoleBuffer.h`. Inject monotonic time, nonblocking output, opt-in pin
   observation, adapter counters and firmware
   identity. Compile on the host without Arduino/IDF headers. Use direct
   functions, fixed arrays and switches; no generic command engine or framework
   compatibility facade.
2. Add a fixed command inventory with aliases, syntax/topic, execution/effect
   class, prerequisites/capabilities, output keys, callback/byte bounds and test
   fixture. Add `tools/tusb32x_cli_contract.py` with testable host rows. Implement
   the CLI_CONTRACT grammar for all supported features. Unsupported routes return
   typed `UNSUPPORTED_FEATURE` before traffic and explain the restriction in help.
3. Implement input/token conversion before device routes. Parse and validate the
   whole command into locals before changing state. Enforce exact arity, complete
   integer conversion, checked ranges and whole-line poisoning after overflow or
   control bytes. No partial setter execution or deleting malformed bytes into a
   valid write. Follow the concrete line/token/output/turn bounds in CLI_CONTRACT.
4. Implement cache-only routes/help, then admission/stepping/result rendering,
   register diagnostics and explicit writes. Handlers use the same driver API as
   non-CLI owners; do not duplicate protocol/register logic. Both hardware front
   ends compile these same sources and do not keep separate dispatchers.
5. Keep CLI staging, core retained desired `DeviceProfile`, observed settings and
   trusted applied generation separate. Controls only stage; apply passes that
   candidate and only accepted admission updates core desired. No separate CLI
   committed profile or profile commit command. ACK means acceptance,
   not hardware completion. `init` identifies/observes and `reconcile` observes/
   compares; neither resets, acknowledges IRQ nor replays settings.
6. Route `irq ack` through `startAcknowledgeInterrupt`, distinct from bounded
   read/publish/ack/recheck `startServiceInterrupt`. ACK preserves actual observed/
   trusted applied 0x09 RW fields even when staged/desired fields differ.
   Unrelated settings writes compose W1C=0. Keep TUSB322I VCONN-fault RCU clear
   semantics unresolved until independently verified; RCU is not proof of
   read-to-clear. Gate unresolved fault-clear and advertisement-write actions.
   Route `recover-dead-battery` through `startRecoverDeadBattery` only for
   original TUSB320/TUSB320I, following the baseline's source-capable startup
   prerequisites and sequence. Initialization never performs that recovery.
7. Use one pending operation/result slot. Admission performs no I2C; `job step 0`
   makes no callbacks; normal auto/step budget is one terminal callback.
   Keep one poll call site per owner turn: an explicit step replaces that turn's
   automatic budget, even when zero, rather than adding another poll.
   `result` uses `resultSnapshot`; `result take <token>` uses `takeResult`.
   Wrong tokens and RESULT_PENDING preserve evidence. Reserve the complete
   result-take response before consuming its slot; full output queues preserve
   the result. No command clears a result to make room.
8. Emit versioned MARK/ACK/JOB/RESULT/END and factual profile/port/stats records.
   Preserve signed transport detail, raw unknown encodings and independent
   operation state/effect/configuration trust. Verbose/color only changes human
   presentation. Match grouped cyan commands/green sections, minimum 32-column
   command field and exact color-off behavior.
9. Stream help over bounded turns. Drop complete records with loss counters;
   keep terminal results queryable after announcement loss. USB backpressure must
   not block deadlines, cancellation or teardown. Host-runner sequences implement
   watch/stress, dumps and fixture suites using ordinary commands plus explicit
   result take. Add no firmware bulk scheduler or separate run result. No hidden
   reset/retry/resume, unbounded Serial-ready wait or background owner task.
10. Make selfcheck/selftest aliases of verify with the same retained-desired
    precondition, probe an alias of init, and aliases share canonical records.
    Keep one verbosity switch and one named command per start operation; no
    duplicate job-start grammar. Explicit mutation commands require no repeated
    confirmation ritual. The host runner owns fixture baseline/cleanup planning;
    cancellation is not rollback.

## Executable acceptance

Distinguish command-handler work from the turn's independent automatic poll.
For exact zero-callback/cache-preservation assertions, use an idle core or set
job auto 0 first. Also test active auto mode: cached/rejected commands perform
no handler I2C or mutation while already-admitted work still progresses once
per turn. Do not accidentally starve the owner by suppressing every query turn.

Add `test/cli/test_cli.cpp`, `test/console/test_console.cpp` and persistent
`test/cli/cli_host.cpp`. Link the actual shared sources to a scripted device and
transport with state preserved between commands. Register CTest `tusb32x_cli`/`tusb32x_console`
and fixture target `tusb32x_cli_host`. Assert state and callback deltas:

| Case | Required proof |
| --- | --- |
| Every canonical/alias/topic route | Correct output/status/effects, no advertised unimplemented command; grouped style and color-off output |
| Cache/staging | Zero callbacks for help/version/bus/health/progress/result/cfg/validate; staging cannot change desired/applied generation |
| Invalid input | Empty/unknown/extra/overflow/sign/enum/control/oversize rejection preserves profile, jobs, caches, results and counters |
| Owner jobs | Zero/one/N callback bounds, explicit step replaces auto for its turn including zero, wait-only turns, immutable deadline, timeout clipping, retained results, stale tokens, take once, late success and cancellation effects |
| Profile/IRQ | Stage 30% while 50% applied, then ACK exact write retains 50%; apply leaves unrelated IRQ pending; fail after write/readback and retain trust/effects |
| Selected-target/raw | Probe shares init semantics, no address scan/reselection, unsupported access silent, raw reads included in all-callback statistics; recovery only on supported original variants |
| Console | Fragmented CRLF/backspace, size boundary, short/zero output, full queue, complete-record loss, take-response reservation, lost-host fairness and stop responsiveness |
| Records | Correlated nonce/request/token, pre-marker nonce, invalid command IDs, request exhaustion, delayed original completion after newer query, repeated result query distinguished, invalidated historical port observations |

Add `tools/test_cli_transcript.py` now with a minimal real-fixture path; P11
expands it into runner execution. Register CTest `cli_transcript` now, passing
`$<TARGET_FILE:tusb32x_cli_host>` and a finite timeout. Source-token guards
supplement executable tests; they do not prove parity or effects. Native success is not serial, CC/INT
or connector qualification.

Proposed commands after these files/targets exist, not prior audit results:

```powershell
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug --parallel
ctest --test-dir build/native -C Debug -R '^(tusb32x_cli|tusb32x_console|cli_transcript)$' --output-on-failure
python tools/check_cli_contract.py
python tools/check_doxygen.py
```

## Documentation and handoff

Write `docs/CLI.md`: every syntax/value/range, cached/live/staged/write route,
callback/time/output bound, result ownership, timeout/verbose/color defaults and
record examples. Add source Doxygen to CLI/platform/console/helper contracts,
including lifetime, owner/threading assumptions and loss behavior; add Python
module/function docstrings. Update README, CHANGELOG, feature-to-API-to-CLI-to-test
matrix, validation and phase progress now. Record exact commands and PASS/FAIL/
NOT RUN. P09/P10 integrate this tested implementation; P13 reconciles existing
documentation rather than writing it for the first time.
