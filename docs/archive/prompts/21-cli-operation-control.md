# 21 — Connect CLI operations and retained results

Deliver all managed operation routes and their bounded scheduling/result flow.
Use the tested core engine; this prompt creates no second scheduler, result cache
or register protocol implementation.

## Read and re-audit

Read [COMMON](../../prompts/COMMON.md), [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md), and
[CLI_CONTRACT](../../prompts/CLI_CONTRACT.md), concentrating on grammar, staging/applied state,
machine records and result ownership. Read the cooperative operations, timing,
trust and silicon sections of [ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md) and
[ENGINEERING_RULES](../../prompts/ENGINEERING_RULES.md). Prompt 20 must pass its console, parser
and live-fixture tests; prompts 1–19 supply implemented core operations.

Re-audit TMP1x2/24Cxx retained-result dispatch and failed-cleanup tests, LDC1614
staging and INA228 transfer-budget assertions. Record current revisions, dirty
state, exact sources and adopted/rejected choices. Read actual TUSB operation
declarations and tests. Sibling init/profile semantics are not this core's rules.

## Implement operation routing

1. Add init/initialize/probe, read/status, profile read, apply, verify/selfcheck/
   selftest, reconcile, irq read/ack/service, reset and recover-dead-battery using
   their one canonical core start API. Add job status/progress, job auto, job step,
   cancel/job cancel/stop and result/job result/result take. Update fixed command
   rows, host schemas and help as each route becomes executable.
2. Admission is bus-silent and returns IN_PROGRESS. Initialize identifies and
   observes only; reconcile observes/compares. Neither resets, clears IRQ or
   reapplies desired settings. Verify aliases require retained desired settings;
   they are not general board certification. Only accepted apply admission copies
   CLI staging into core desired. Never add profile commit or duplicate job-start
   grammar. Supported explicit mutations need no repeated confirmation ritual.
3. Use one poll call site per owner turn. Explicit job step replaces automatic
   budget for that turn, including zero; default budget is one terminal callback.
   Cached/rejected handlers do no I2C, while already-admitted work may advance
   independently once afterward. Do not suppress every query turn or double-poll.
4. Preserve one active operation and one core retained result. Result queries use
   resultSnapshot; exact-token take uses takeResult. Wrong tokens and pending
   results preserve evidence. Before consuming a result, reserve its entire
   bounded response, including copied RESULT and END. Queue-full leaves the core
   slot intact; do not defer destructive take or introduce another result store.
5. Retain original request/nonce/token for admitted work. Emit one spontaneous
   RESULT/END pair per operation; later queries have their own request and the
   same origin_request. An older operation may finish after a newer status ACK.
   Delivery OK is separate from payload failure. Preserve signed transport detail,
   effects, configuration trust, operation outcome and generations independently.
6. Route IRQ acknowledgement separately from bounded service. ACK preserves
   actual observed/trusted 0x09 controls even when staging or desired differ.
   Unrelated writes use W1C=0. Keep unresolved TUSB322I fault-clear/advertisement
   writes unavailable; unchanged proved advertisement must not block other allowed
   profile changes. Missing/reserved actual evidence is not equality. Original
   TUSB320/I recovery uses startRecoverDeadBattery
   and its stable-VDD/source-startup prerequisites; later variants reject before
   traffic. Do not hide recovery inside initialization or copy its register writes.
7. Output backpressure must not block polling, deadlines, cancellation or end.
   Drop complete records with loss evidence, leaving results queryable. Host
   runners will own bulk sampling/dumps/fixture sequences. Add no firmware watch
   loop, retry/reset/resume policy, background task or second run-token scheme.

## Behavioral completion gate

Extend the actual shared-source CLI tests and persistent transcript fixture from
prompt 20. Test admission rejection, every new alias, zero/one/N budgets, explicit
step with auto enabled, zero step, wait-only turns, timeout clipping and late
success. Check immutable deadlines, cancellation after possible writes, pending
result fences, stale tokens, repeated query, take once and failed reservation.

With auto off or idle, assert exact zero handler callbacks and full state
preservation for invalid/cached commands. Separately prove active auto work still
progresses under those commands. Test apply request 24 completing after status
request 25, new marker while work is active and lost terminal announcements.
Test staging 30% while applied 50%, then IRQ ACK preserving 50%; apply leaving an
unrelated latch pending; failure after write/readback; recovery capability denial;
and invalidated historical port output. Check hardware effects in core/fixture
state, not just printed words. Run existing CLI/console/transcript CTests.

## Delegate, document and sync

Optional: assign transcript fault cases to a general coding agent in disjoint
tests; use an independent stronger-reasoning reviewer for result ownership,
timing and uncertain effects. Freeze shared records before parallel edits. The
lead reviews changes and owns staging/commits/pushes.

Update source Doxygen, Python docstrings, `docs/CLI.md`, README, CHANGELOG,
feature-to-API-to-CLI-to-test coverage, validation and progress with implemented
routes and exact commands/results. Fix this stage's failures before handoff.
Commit and sync coherent checked units and the finished prompt on the same
execution branch; verify upstream equality. Report native proof separately from
serial/CC/INT qualification. Prompt 22 closes remaining command coverage, not
unfinished operation-control defects.
