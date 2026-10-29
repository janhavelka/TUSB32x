# 22 — Complete CLI diagnostics and command parity

Complete the shared CLI against its fixed contract. The result must expose every
implemented applicable feature through one canonical route and tested aliases,
with no advertised placeholders or independent hardware logic in the CLI.

## Inputs and focused audit

Read [COMMON](../../prompts/COMMON.md), [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md),
[CLI_CONTRACT](../../prompts/CLI_CONTRACT.md), and the raw-access/statistics/health sections of
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md). Apply
[MAINTAINABILITY](../../prompts/MAINTAINABILITY.md) and the example/core boundaries in
[ENGINEERING_RULES](../../prompts/ENGINEERING_RULES.md). Prompts 20 and 21 must have a working
bounded console, local commands, operations and retained-result flow.

Re-audit current TMP1x2/24Cxx diagnostic/help tests and contract parity, SHT3x
aliases, LDC1614 command classifications and INA228 exact counter assertions.
Record revisions, dirty state, exact files and choices. Compare the existing
TUSB command rows with public APIs and variant capabilities before adding routes.
Do not redesign the parser or operation engine to close simple dispatch gaps.

## Complete the remaining behavior

1. Add reg/rreg/rawread, pure decode, wreg/rawwrite, cached irq, opt-in intpin,
   explicit invalidate and cached drv/health/state, diag, xfer_stats/counters,
   xfer_reset and xfer_assert. Use the existing core APIs and exact CLI_CONTRACT
   grammar; include any remaining contract route not completed by prompts 20/21.
2. Raw reads/writes use the synchronous diagnostic API, with its one-callback
   bound, access masks, owner exclusion and trust rules. Raw writes cannot bypass
   sequenced role/reset/termination/recovery procedures. Delegate rejection to
   validated core behavior; do not duplicate safe-write composition in the CLI.
   Preserve unknown encodings and original signed backend detail in output.
3. All callbacks count in TransportStats. Raw diagnostics do not update filtered
   managed-health evidence; probe is managed init and does. Counter assertions
   report their source and reject saturated totals. Idle-only xfer_reset cannot
   repair configuration trust, clear effects, consume a result or reset managed
   health evidence. Cached diagnostics and pure decode perform zero callbacks.
4. Invalidate requires an explicit supported reason and preserves historical
   effect evidence. Intpin uses only the injected opt-in application reader;
   common code never sets up GPIO. End is bus-silent and not a hardware reset.
   Test explicit forget-evidence from prompt 20 and reject it while bound, active
   or retaining a result. Target edits never call it implicitly. No address scan,
   hidden target reselection, VBUS policy, bus recovery or sensor command belongs here.
5. Finish the fixed command inventory and `tools/tusb32x_cli_contract.py` rows:
   canonical names/aliases, topic/syntax, execution/effect class, prerequisites,
   capabilities, record keys, callback/byte bounds, fixture and behavioral test.
   All help routes match real dispatch. Unsupported silicon features return
   UNSUPPORTED_FEATURE before traffic and explain their restriction. Do not use
   this status to conceal an unimplemented required route.
6. Finish grouped help, cached/live/staged/write labels, timeout defaults, typed
   statuses, factual profile/port/stats records, verbosity and color-off parity.
   Unknown/not-applicable port values and invalidation must remain distinguishable.
   Test record storage with maximum nonce/integer/enum widths including newline
   and terminator. Extend the existing output code; do not create another renderer.

## Prove full coverage

Extend `test/cli/test_cli.cpp`, `test/console/test_console.cpp` and the persistent
`tusb32x_cli_host` transcript path. Every canonical route, alias and help topic
needs an executed assertion of output, status and effects. Check parser rejection
preserves profile/jobs/caches/results/counters, raw access unsupported/active-owner
denial, all-callback accounting, unsupported variants and staged-versus-applied
IRQ preservation. Repeat only affected earlier tests where fixes warrant it.

Add or complete `tools/check_cli_contract.py` using the built fixture and actual
help/records to compare Python rows with firmware behavior. Static source tokens
are supplementary. Test a deliberately broken alias/record contract to show the
checker fails. Reuse this command in the later validator and CI; do not create
parallel parity implementations. Register any new test with finite timeout and
an explicit built fixture path; retain the single cli_transcript registration.

Run CLI, console and transcript CTests plus the executable parity check. Inspect
registration and nonzero counts. Native fixture success does not prove serial,
adapter framing or connector operation.

## Delegation and completion

Optional: delegate read-only command/help parity or independent fixture cases to
a general coding agent; a faster model suits a fixed inventory/link comparison.
Use stronger reasoning for a disputed write or counter invariant. Give disjoint
write scopes and keep Git with the lead.

Finish `docs/CLI.md` and command coverage, including all syntax/ranges, ownership,
bounds, output loss, source identity and unsupported features. Maintain source
Doxygen, Python docstrings, README, CHANGELOG, validation and progress now.
Review the complete shared CLI before declaring it ready for framework examples.
Commit coherent checked units, commit/sync the completed prompt on the existing
execution branch and verify synchronization. Report remaining hardware evidence
honestly; do not hand off unfinished required CLI behavior.
