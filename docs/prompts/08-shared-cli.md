# P08 — Build the complete shared diagnostic CLI

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 and
SHT3x shared processors, LDC1614 command contract/help, INA228/INA3221 diagnostic
output and 24Cxx console/runner fixtures. Compare executed Arduino/IDF commands.

## Work

1. Implement a framework-neutral command processor under `examples/common/`.
   Inject text output, time, named callback/transaction counters and explicit
   owner/platform actions.
   Keep Serial, Wire, FreeRTOS and IDF out of this processor and the device core.
2. Define a command contract with canonical name, aliases, syntax, effect class,
   prerequisites, variants, output keys, expected transfer budget and test fixture.
   Cover each applicable feature from the feature matrix; no advertised but
   unimplemented routes. Help and dispatch must agree in both front ends.
3. Match grouped sibling help, aligned command columns, cyan commands/green
   sections, readable typed statuses, optional colors and topic help. Preserve
   recognizable names where semantics match; label cached vs live commands clearly.

| Command group | Required behavior; final grammar belongs in CLI.md |
| --- | --- |
| Common | `help/?`, topics, `version/ver`, color, quiet/verbose, bounded `hilmark` nonce |
| Context | Cached `bus`, console counters, exact model/address/capabilities and firmware provenance |
| Connection | Passive bind/end/unbind, explicit read-only identify/probe/discover; address/model changes validated before effects |
| Port | Live status/read, cached snapshot/age, requested vs actual role, orientation/current/accessories/raw evidence |
| Profile | Cached desired/observed settings; stage/validate/apply/verify/reconcile with generation/trust |
| Controls | Supported mode, advertisement, DRP duty/debounce/preference, UFP accessories, termination controls |
| Interrupt | Cached pending/evidence, live inspection, explicit acknowledge/service; opt-in application pin observation |
| Lifecycle | Explicit reset/reconcile/invalidate; no hidden reset or role change inside ordinary diagnostics |
| Jobs | Start/status/auto/manual step with 0/1/N budgets, cancel/stop, retained result and token/effect display |
| Registers | Allowlisted read/dump, explicit typed/raw writes with W1C/reset warnings and trust effects |
| Diagnostics | Raw transport statistics, optional health projection, config trust, counter reset/assert with idle preconditions |
| Runs | Finite watch/stress/selfcheck, separate fixture-selected mutation tests, bounded aggregate summaries |

4. Family discovery uses legal target addresses and read-only identity checks;
   never auto-select enable polarity or grant capabilities from ACK. A broad
   all-address scan, if offered, is an explicit application action with probe
   availability and traffic documented, not startup behavior.
5. Parse exact arity, integer bases/ranges, signedness, overflow, trailing junk,
   booleans/enums and token lengths. On rejection, preserve staged profile, jobs,
   caches and traffic counters. Handle CRLF, backspace if supported, empty input,
   unknown commands, overlong lines, embedded control characters and overflow.
6. Commands submit work and return; a bounded loop services jobs/console. Finite
   runs have explicit target/deadline and cancel; retained output/latency statistics
   are bounded. Print failures immediately, never silently reset/retry and resume.
7. Emit stable machine-readable terminal/job/result records with correlation,
   status/detail/effects and firmware identity. Color does not alter meaning.
   Distinguish observations, read-only selfcheck, and fixture-required changes.
   Startup and `diag` are non-mutating. No automatic power/data-role decisions.
8. Mutation selftests capture a trustworthy baseline and only restore when the
   target/attachment assumptions still hold. Report cleanup uncertainty distinctly.
   Cancellation does not promise rollback. Do not restore an old source role
   blindly after a cable or power-state change.

## Tests and completion

- Execute the actual shared command processor in a persistent native fixture;
  assert outputs, statuses, state and exact transport deltas for valid/invalid inputs.
- Cover every command/alias/help route and unsupported variant behavior. Compare
  both front ends to the same behavioral contract, not only source-token searches.
- Test long/fragmented console input, output backpressure, stop responsiveness,
  counters, nonce validation, job results and interruption during diagnostics.
- Add `docs/CLI.md` and update feature coverage. Hardware front ends come next;
  native fixture success is not serial or connector validation.
