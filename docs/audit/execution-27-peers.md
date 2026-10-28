# Prompt 27 current peer and generic-owner review

Audit date: 2026-10-28. Scope: ordinary C++ consumers, external owner
scheduling, callback/context lifetimes and retained operation effects. This is a
source audit; no sibling build, sibling edit, remote CI or hardware action was
performed. The lead owns implementation checks and Git synchronization.

## Revisions and rules

The execution started at TUSB32x
`9e4b08a62303dbb9a6bbbfdbacb6aeb42154533c`, clean and synchronized as recorded by
the lead. Peer `git status --short --branch` and `git rev-parse HEAD` were read
again for this prompt:

| Read-only peer | Branch | Revision | Worktree |
| --- | --- | --- | --- |
| FieldCore-node | `rodtester` | `5ad9292dc8481cf08be9a61f662495a6474d1da0` | Clean |
| LDC1614 | `main` | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | Clean |
| TMP1x2 | `main` | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | Clean |

Read the parent and local `AGENTS.md`, each selected peer's `AGENTS.md`,
`docs/prompts/README.md`, `COMMON.md`, `AGENT_WORKFLOW.md`, the complete prompt
27, the ownership sections of `ENGINEERING_RULES.md`, and the state/result/effect
and time/work/lifetime sections of `ENGINEERING_BASELINE.md`. TUSB32x's
one-result, exact-token, fresh-owner-time contract remains authoritative.

## Exact peer sources and decisions

All peer paths below are relative to the named sibling repository.

### FieldCore-node

Declarations and implementation inspected:

- `include/TunnelMonitor/i2c/I2cTask.h`: bounded request/result ownership,
  exact retained application-result identity and externally attached clock.
- `include/TunnelMonitor/i2c/I2cOwnerTransport.h`: `I2cBusTransaction`,
  `I2cBusObservation`, `I2cOwnerTransport`, and synchronous mux transport.
- `include/TunnelMonitor/i2c/I2cDeviceBinding.h`: module start/poll/take/cancel,
  invalidation, copied snapshots and bus-silent `waitHint` for parked work.
- `include/TunnelMonitor/i2c/I2cLifetime.h`: checked conversion of modulo-32-bit
  driver due times to owner time; zero extension is explicitly rejected.
- `include/TunnelMonitor/contracts/Time.h`: `Deadline64`, unsigned half-range
  comparisons, monotonic elapsed time and qualified freshness age.
- `src/i2c/I2cTask.cpp`: `processActiveBindingWork`, `collectBindingResult`,
  `physicalTransferOnce`, `nowMsThunk`, `nowUsThunk`, and `ownerNowMs`.

Actual tests inspected in
`test/native/test_owner_i2c/test_i2c_task_fake.cpp` include
`test_i2c_owner_transport_clips_deadlines_counts_bytes_and_never_retries` and
`test_i2c_binding_deadline_consumes_real_completion_at_expiry`. They check clipped
timeout, counted backend calls, late completion evidence and real terminal
delivery around a deadline. `test/python/test_hil_i2c_startup.py` supplies explicit
request correlation and bounded asynchronous observation, with captured CLI
completion wording. These are peer source examples, not TUSB32x test results.

Adopt the ownership boundaries: one serialization owner, bounded callback turns,
request/result correlation, application-owned monotonic time, explicit invalidation
and retained effects while work waits. Do not import product request queues,
device IDs, mux routing, required-device health, retry policy or binding types.

The transport evidence is materially richer than TUSB32x's `Status` callback.
`I2cBusObservation` carries `writtenBytes`, `readBytes`, `writeEffect`,
`durationMs` and `completedUptimeMs`; the owner transport also exposes clock
hooks. `physicalTransferOnce` clips the timeout and combines backend duration
with a subsequent owner-clock sample. Those fields do **not** survive a simple
conversion to TUSB32x `Status`. TUSB32x still requires a fresh timestamp on the
next poll, and a failed write retains conservative acceptance uncertainty.
FieldCore's `nowUsThunk` multiplies the millisecond value by 1000; it is not an
independent source of microsecond resolution. Do not copy its richer completion
claims into the ordinary TUSB32x callback contract.

### LDC1614

Inspected `include/LDC1614/Config.h`, `include/LDC1614/LDC1614.h`, and lifecycle,
poll, cancellation, retained take and invalidation implementation in
`src/LDC1614.cpp`. The peer copies configuration and borrows callback contexts.
`test/test_basic.cpp` includes the generic-owner correlation flow around
operation IDs 91001 through 91006, cancellation, actual possible write acceptance,
dirty configuration and explicit recovery, together with exact-budget and
retained-result tests. `examples/common/Ldc1614Cli.cpp` owns job admission,
one-budget polling and result drainage. `tools/test_ldc1614_hil_runner.py` separates
scheduled session identity, terminal outcome and firmware build identity.

Adopt explicit correlation, bus-silent admission/cancel/invalidation and separate
status/effect/trust evidence. Reject its caller-selected operation IDs, FIFO result
capacity, result discard in `end`, initialization replay and divided timeout
budget as TUSB32x conventions. TUSB32x uses instance-local tokens, one retained
result, retained end evidence, read-only initialization and one callback per
Status-only poll with a fresh later clock sample. Sensor-specific recovery and
acquisition evidence are not USB configuration policy.

### TMP1x2

Inspected `include/TMP1x2/TMP1x2.h`, `include/TMP1x2/BusOperations.h`, and
`src/TMP1x2.cpp` lifecycle/dirty-state implementation. Actual tests in
`test/jobs/test_jobs.cpp` (`admissionAndResults`, `budgetsAndHealth`,
`cancellationAndTeardown`, `abandonedConversions`) check unchanged output on a
wrong token, retained terminal backpressure, no-I2C cancel/end and dirty evidence
after an actual write. `examples/common/Tmp1x2Cli.cpp` shows explicit invalidation
and operation-aware CLI access. `tools/test_hil_tmp1x2_runner.py` checks token/kind
matching, repeated token rejection and incomplete terminal evidence.

Adopt those observable lifecycle and negative-test patterns. Reject the peer's
32-bit operation clock, sensor conversion timing, `end` versus `unbind` semantics,
blocking convenience paths and broad recovery replay as TUSB32x APIs. The current
TUSB32x engine already distinguishes local teardown, uncertain hardware effects,
read-only reconciliation and separately selected desired-profile application.

### Workflow source review

Read FieldCore-node `.github/workflows/verify.yml`, which delegates checks to
`python scripts/verify.py` and pins Python 3.12, Node 22.22.0 and PlatformIO 6.1.19;
LDC1614 `.github/workflows/ci.yml`, which registers native tests, timing/CLI/clean
consumer guards and separate firmware/documentation jobs; and TMP1x2
`.github/workflows/ci.yml`, which registers strict/sanitized CMake/CTest, contract,
documentation, package and framework jobs. None of these workflow definitions
proves a successful run or authorizes importing a sibling dependency. The current
TUSB32x checkout has no `.github/workflows` directory; prompt 34 owns its remote
checkpoint. No remote status was queried.

## Existing TUSB32x evidence and integration gaps

Read public `include/TUSB32x/TUSB32x.h` and `Config.h`,
`test/test_owner_scheduling.cpp`, `test/test_example_owner.cpp`,
`test/consumer/portable/{main.cpp,secondary.cpp,CMakeLists.txt}` and
`test/consumer/run_portable.py`.

| Existing evidence | What the current fixture actually covers |
| --- | --- |
| `waitingOwnerDoesNotHoldBus` | Two distinct targets share callbacks; base recovery waits while the peer completes two reads; callback buffers/lock are released between turns; physical scripted timestamps check the minimum wait. |
| `queueAndResultBackpressure` | Bounded application queues, result storage pressure, core `RESULT_PENDING`, exact delivery association and no bus work during retention. |
| `failedOrRepeatedIrqOwnerDoesNotStarvePeer` | One target's failure or repeated bounded IRQ service does not stop peer progress. |
| `literalLateWriteKeepsOutcomeAndEffect` | The test explicitly applies a write even on callback failure, advances simulated physical time past deadline and checks original transport status versus owner timeout, effect uncertainty and peer trust. |
| `test_example_owner.cpp` | Read-only startup, handoff to another operation owner, independent targets, coalesced service requests, failure isolation and bus-silent stop with retained cancellation. |
| Isolated portable consumer | Public-only multiple-translation-unit linkage, operation traversal and failure cases, allocator/failure sentinels, and copied library/header consumption without downstream Git/Python/Doxygen/PlatformIO on Windows. |

The early review requested four additional integration paths, using the existing
public-only scheduler rather than another owner abstraction:

1. Stale poll/cancel/take tokens preserve active or retained work, output values
   and peer state.
2. Cancellation before and after a possible write retains appropriate effects;
   separately selected reconciliation observes without replaying that write.
3. External reset/power-loss invalidation keeps old copied observations historical;
   a successful unrelated raw read repairs neither configuration trust nor port
   freshness; later reconcile/apply is a separate owner decision.
4. Both owners end and handle their retained results while borrowed context remains
   alive; later local inspection and destruction perform no callbacks.

The initial scheduler declared its bus after its driver lanes, causing reverse
member destruction to destroy the context first. The current default driver
destructor has no callback, but that layout does not demonstrate the documented
borrow-until-end lifecycle. The fixture should declare context before borrowers
and explicitly end owners before releasing it. This is consumer lifetime guidance,
not a request for destructor I/O or a core scheduler.

## Independent implementation review

Reviewed the resulting host `main.cpp`, README and CMake target,
`docs/integration.md`, the added CTest registration and all four owner fixture
additions. The host keeps simulation explicit, samples fresh synthetic time,
bounds each turn, takes the exact token, and demonstrates delivery OK with a
CONFIGURATION_MISMATCH payload. Its successful raw read preserves UNKNOWN trust
and invalidated port evidence. Every host exit calls `end` while context remains
alive. The integration mapping accurately explains the lost FieldCore completion
and effect metadata.

The owner fixture now declares the bus before its borrowing drivers and ends
them explicitly. The new cases assert stale-token preservation, pre/post-write
cancellation, retained-result pressure, separately selected read-only reconcile,
external invalidation with independent peer progress, renewed port publication,
and retained cancellation/effect delivery on shutdown. Existing literal late-write
tests continue to supply the distinct INDETERMINATE acceptance case. The base-part
power-return example remains observation evidence, not source-startup readiness;
qualified explicit recovery is a separate application decision and is exercised
by the existing recovery/fairness case.

No core defect or need for a new owner wrapper was found. The reviewer reran the
already built `build/p27-example-agent/tusb32x_host_basic.exe` and
`build/p27-owner-agent/test/tusb32x_operations.exe`: both returned zero, the host
output matched its README, and the operation executable reported 5,487 scheduling
checks with zero failures plus successful remaining operation suites.
`git diff --check` also passed. These reruns are not a substitute for the lead's
final rebuild and registered matrix; the prompt validation record owns that
evidence. No sibling build, remote CI or physical qualification is claimed.
