# 20 — Build the bounded console and local CLI commands

Deliver a working host-tested CLI for the local commands listed below. Finish
that subset completely; prompt 21 adds operation control and prompt 22 closes
device-command coverage. Do not advertise routes that are not implemented yet.

## Read and check the starting point

Read [COMMON](COMMON.md), [AGENT_WORKFLOW](AGENT_WORKFLOW.md),
[MAINTAINABILITY](MAINTAINABILITY.md), and the files/ownership, grammar, parser,
appearance and machine-record sections of [CLI_CONTRACT](CLI_CONTRACT.md).
Use [ENGINEERING_BASELINE](ENGINEERING_BASELINE.md) for existing core names and
[ENGINEERING_RULES](ENGINEERING_RULES.md) for ownership and example boundaries.
Prompts 1–19 must have delivered the core and conformance evidence. Inspect its
actual declarations and registered tests before writing example code.

Re-audit current TMP1x2 and 24Cxx console/parser/CLI tests, SHT3x dispatch and
LDC1614 command rows/staging. Record HEAD, dirty state, exact files and decisions.
Read at least one malformed-input test and one output-backpressure test. Do not
copy framework APIs from sibling common directories or their startup mutations.

## Implement this complete subset

1. Create `examples/common/Tusb32xCli.h/.cpp`, `Tusb32xCliOutput.cpp` and
   `ConsoleBuffer.h`. Use one example-only `tusb32x_cli::Cli`, fixed arrays and
   direct dispatch. Inject time, nonblocking output, optional input-pin reading,
   adapter counters and firmware identity. Host compilation needs no Arduino or
   IDF headers. No generic command framework or new core scheduler.
2. Implement common help/version/color/verbose/hilmark; cached bus/console/
   capabilities; variant/address queries and validated target edits; bind/end;
   cached sample/freshness, cfg/profile show/reset/discard/validate; and the
   profile control getters/setters in the contract. Include their specified
   aliases. Target edits require an ended, idle instance with no retained result.
   All these commands are bus-silent. No startup command runs automatically.
3. Keep CLI staging separate from core retained desired, observed and applied
   settings. Reset stages variant candidate defaults, not a silicon reset image.
   Discard without retained desired rejects CONFIGURATION_UNKNOWN. Validate all
   values and capabilities before changing any field or generation. Staging does
   not apply, acknowledge an IRQ or change core desired state.
4. Implement strict whole-command parsing before effects: exact arity, full
   integer conversion, bounds, exact enums/booleans and no unsigned signs or junk
   suffixes. Enforce the contract's 192-byte line, 12 tokens, 32-byte token
   payload, CRLF and backspace rules. Overflow, NUL or illegal controls poison the
   entire line until newline. Do not edit malformed input into a valid command.
5. Enforce 64 input bytes and one completed command per turn; bounded incremental
   help; 8192-byte output queue; 512-byte formatted-record storage; one nonblocking
   backend write of at most 256 bytes per turn. Count whole-record losses, retain
   loss evidence and never truncate a record into plausible success. Document any
   justified smaller queue. Lost output must not create an unbounded loop.
6. Implement grouped cyan/green help, minimum 32-column command syntax, explicit
   cached/stage labels and exact color-off behavior. Freeze implemented record
   schemas in `tools/tusb32x_cli_contract.py` and `docs/CLI.md`: typed statuses,
   signed detail, request IDs, nonce=none, command=invalid, MARK instead of ACK
   for hilmark, normal END closure and exhaustion without counter wrap. List later
   routes only as planned documentation; executable rows/help/parity cover the
   implemented subset, with no placeholder handlers.

## Prove it now

Add `test/cli/test_cli.cpp`, `test/console/test_console.cpp` and persistent
`test/cli/cli_host.cpp`, using the actual shared sources and a scripted transport.
Register `tusb32x_cli`, `tusb32x_console` and fixture target `tusb32x_cli_host`.
Add minimal `tools/test_cli_transcript.py` and register `cli_transcript` now with
`$<TARGET_FILE:tusb32x_cli_host>` and a finite timeout.

Test every implemented canonical/alias/topic route; zero callback deltas; full
state preservation on invalid input; staging/desired separation; poisoned input,
fragmentation and boundaries; short/zero output writes, queue loss and fairness;
maximum-width records including newline/NUL; nonce/ID validation and exhaustion.
Exercise one persistent C++ process from Python. Static command strings are not
behavioral proof. Run the registered CLI/console/transcript tests, with nonzero
test counts; later stages extend these same registrations.

## Delegate and finish

Optional: a general coding subagent can implement console tests in disjoint files
after the parser/output contract is fixed. A separate reviewer can inspect bounds
and record framing; use stronger reasoning if effects or ownership are unclear.
The lead integrates, reviews evidence and owns Git.

Write Doxygen for CLI/console/helper contracts and Python docstrings. Update
`docs/CLI.md`, README, CHANGELOG, coverage, validation and implementation progress
with this implemented subset and exact PASS/FAIL/NOT RUN evidence. Finish all
subset behavior and tests; do not defer defects to prompt 22. Commit coherent
checked work, sync and verify upstream on the existing execution branch before
handoff. Report implementation scope, checks, limitations and commit/sync result.
