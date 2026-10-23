# Concrete CLI and runner contract

This is an implementation specification, not a claim that these commands exist
yet. P08 implements it; P09 and P10 compile the same processor; P11 tests and uses
it; P12 packages and registers it. Read [COMMON.md](COMMON.md),
[ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md) and
[MAINTAINABILITY.md](MAINTAINABILITY.md). Keep the resulting `docs/CLI.md` beside
the implementation. Changes to this contract require matching code, behavioral
tests, runner parsing, help and documentation in the same phase.

## Source choices and limits

Fresh read-only inspection on 2026-10-23 found the following clean checkouts.
These are starting revisions; each implementation stage must inspect them again.

| Peer revision | Actual files inspected and decision |
| --- | --- |
| TMP1x2 `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `examples/common/Tmp1x2Cli.h`, `Tmp1x2Cli.cpp`, `Tmp1x2CliOutput.cpp`, `ConsoleBuffer.h`, `test/cli/test_cli.cpp`, `tools/hil_tmp1x2_runner.py`, `tools/check_package.py`, `CMakeLists.txt`, `.github/workflows/ci.yml`: adopt neutral processor, strict parsing, grouped output, complete-record dropping, retained jobs, executable fixture and provenance checks. Do not inherit temperature commands or general-call reset. |
| 24Cxx `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | `examples/common/Eeprom24CxxCli.cpp`, `tools/hil_runner.py`, `tools/test_cli_transcript.py`, `tools/check_package_contents.py`, `CMakeLists.txt`, `.github/workflows/ci.yml`: adopt persistent real-CLI transcript tests, exact terminal evidence, separate host/firmware identity and package consumers. Do not inherit scratch-window, erase or write-cycle policy. |
| SHT3x-main `f65f15573e3c9b1ef04cce72ad08f307d021e563` | `examples/common/Sht3xCli.cpp`, `tools/sht3x_cli_contract.py`: adopt shared dispatch and small host command rows. Avoid its compatibility shims and unrelated measurement commands. |
| LDC1614 `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` | `examples/common/Ldc1614Cli.cpp`, `tools/ldc1614_cli_contract.py`, `tools/ldc1614_hil_runner.py`: adopt execution/effect/fixture classifications and staged settings. Its init replays settings and its profile commit is bus-silent; neither establishes TUSB initialize/apply semantics. Do not copy its sensor-specific commands or blanket `confirm` convention. |
| INA228 `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | `tools/run_i2c_hil.py`, `examples/common/CliStyle.h`, `docs/CLI.md`: adopt exact transfer-delta and owner-exclusion checks, recognizable status vocabulary and terminal style. Do not copy clear-on-read or measurement behavior. |

Sibling AGENTS files were inspected only as evidence about their conventions.
Their push/flash permission does not apply here. No sibling was built or changed
for this specification. TI controls hardware semantics, including the separate
TUSB322I unresolved VCONN-fault RCU semantics; a command classified as a read does
not prove it has no chip side effects.

## Files and ownership

Use `examples/common/Tusb32xCli.h/.cpp`, `Tusb32xCliOutput.cpp`,
`ConsoleBuffer.h` and `BoardConfig.h`. Keep parsing and dispatch in the first
source and formatting/help in the second. Add another source only for a current,
clear responsibility. Use one
example-only `tusb32x_cli::Cli` with `setup()`, `feed()`, `processCommand()`,
`tick()`, `printHelp()` and injected platform functions. Do not introduce a
reusable command framework, registry, plug-in architecture or inheritance tree.

The core remains `TUSB32x::TUSB32x`. Platform actions, serial output, firmware
identity, clocks, pins and backend counters belong to the examples. Both front
ends compile these sources; they do not maintain separate dispatchers. Command
rows are a fixed C++ array. `tools/tusb32x_cli_contract.py` holds small host-side
rows and expected schemas; compare them by executing the real CLI, not by
pretending the Python rows are firmware code generation.

Each row records canonical name/aliases, topic, syntax, execution class,
hardware-effect description, prerequisites, capability, output record keys,
maximum callbacks per owner step, aggregate byte bounds, fixture and behavioral
test. Use only `CACHE_ONLY`, `PURE`, `CORE_JOB`, `RAW_I2C` and
`PLATFORM_ACTION`. Effect classes are `NONE`, `LOCAL_STATE`, `READ_OBSERVATION`,
`DEVICE_WRITE` and `PLATFORM_EFFECT`. A live read may consume evidence on a
particular variant; record that separately. Capability denial precedes traffic.

## Grammar and routing

Names are lowercase ASCII. Accept only the aliases below unless a later audited
change updates the contract. `[]` means optional, `|` means alternatives, and
`<...>` means one token. Do not silently interpret `cfg` as a live read or `init`
as permission to apply settings. No command runs automatically merely because
the console opens. Preserve the agreed example startup behavior and document it
in P09/P10; this contract does not add a new startup sequence.

| Topic | Canonical grammar and aliases | Meaning and routing |
| --- | --- | --- |
| common | `help [topic|command]`, alias `?`; `version`, alias `ver` | Cache only; topics: common, connection, port, profile, interrupt, job, registers, diagnostics, all. Include firmware/library/framework/target identity. |
| common | `color [on|off|0|1]`; `verbose [0|1]`; `hilmark <nonce>` | Local only. Verbose controls progress chatter; never hide errors or terminal records. Nonce is 1..32 ASCII letters/digits/underscore/hyphen; `none` is reserved. |
| connection | `bus`, `console`, `capabilities` | Cache only; configured variant/address, strap assumptions, adapter framing, finite timeouts, pins, build identity and counters. Capability output distinguishes configured and observed identity. |
| connection | `variant [name]`, alias `model`; `addr [address]`; `bind`; `end`, alias `unbind` | Queries cache only; target edits require ended, idle instance with no retained result and pass full validation before replacing anything. Bind/end are bus-silent. End is no hardware reset. |
| connection | `init [timeout_ms]`, aliases `initialize`, `probe` | `startInitialize`: read-only identification and observation of the selected target; never reset/apply, scan addresses or infer exact variant from identity bytes. |
| port | `read`, alias `status`; `sample`, alias `snapshot`; `freshness` | Read starts `startRefreshPort`; sample/freshness print cached validity, observation sequence/time/age and provenance. Live read does not acknowledge the TUSB320 interrupt latch. |
| profile | `cfg`, alias `settings`; `profile [show]`; `profile reset`; `profile discard`; `profile validate` | Cache/pure. Show CLI staged, core desired, observed and trusted applied fields separately. Reset stages the documented DeviceProfile candidate defaults for the variant; these are not a chip reset image. Discard copies core retained desired to staging; if none exists, reject CONFIGURATION_UNKNOWN without changing staging and direct the user to profile reset. Neither writes. Validation is not apply. |
| profile | `profile read [timeout_ms]`; `apply [timeout_ms]`; `verify [timeout_ms]`, aliases `selfcheck`, `selftest`; `reconcile [timeout_ms]` | Profile read starts `startReadProfile`. Apply starts `startApplyProfile(staged,...)`; only accepted admission updates core desired. It is the explicit hardware mutation. Verify and its aliases compare reads with core retained desired and require that desired profile; they are not a general hardware test. Reconcile observes/computes trust without replaying writes. |
| profile | `mode [port|dfp|ufp|drp]`; `current [default|1a5|3a0]`; `duty [30|40|50|60]`; `debounce [milliseconds]`; `preference [normal|try-sink|try-source]`; `ufpaccessory [on|off]`; `termination [on|off]`; `rd-rp [on|off]` | Getters show staging; setters only stage. `port` selects PORT-pin-determined MODE00 on verified TUSB320 variants; TUSB322I MODE00 means DRP, so port is unsupported there. Termination means the extended 0x0A DISABLE_TERM control. Rd-rp means the separate capability-gated 0x45 Rd/Rp control; neither substitutes for the other. On/off means enabled/disabled, with raw disable polarity shown separately. Advertisement is a CC category, not measured load or PD. Print exact variant debounce values from P01; unsupported controls/values reject before staging. |
| interrupt | `irq`; `irq read`; `irq ack [timeout_ms]`; `irq service [timeout_ms]`; `intpin` | Bare irq is cached retained evidence; read inspects live without TUSB320 W1C acknowledgement. Ack is explicit write; service is bounded read/publish/ack/recheck with documented coalescing limits. Intpin calls an opt-in application input reader; no GPIO setup in common code. |
| lifecycle | `reset [timeout_ms]`; `invalidate <external-reset|power-loss|target-change>` | Reset starts `startReset`, never automatic reapply. Invalidate passes an explicit CONFIGURATION_UNKNOWN reason to local invalidateDeviceState and preserves effect evidence. Explicit role/reset actions may affect autonomous VCONN on TUSB322I. |
| lifecycle | `recover-dead-battery [timeout_ms]` | `startRecoverDeadBattery`, kind `RECOVER_DEAD_BATTERY`: explicit source-capable startup recovery for original TUSB320/TUSB320I only, using the P01/baseline sequence and prerequisites. Never an init side effect; unsupported variants reject before traffic. |
| job | `job [status]`, alias `progress`; `job auto [0|1]`; `job step [0..255]` | Status/cache and local scheduler mode; explicit step selects this turn's single `poll(token,nowMs,budget)`, default budget 1. It replaces automatic stepping for that turn, including budget zero; never poll again afterward. |
| job | `cancel [token]`, aliases `job cancel`, `stop` | Cancel/stop is local, preserves effects and produces a terminal result; cancellation does not imply rollback. Start operations through their named commands above. |
| job | `result [token]`, alias `job result`; `result take <token>` | Non-consuming `resultSnapshot` versus exact-token `takeResult`. Untaken terminal result fences further jobs. No implicit discard when a new command arrives. |
| registers | `reg <address>`, aliases `rreg`, `rawread`; `decode <address> <byte>` | Reg aliases all call the same readRegisterSync diagnostic API; no invented managed/raw API pair. Output raw/reserved/unknown encodings; decode is pure. All callbacks count in transportStats; raw diagnostics do not update managed health evidence. |
| registers | `wreg <address> <byte>`, alias `rawwrite` | Same writeRegisterSync diagnostic API. Enforce access/masks and trust invalidation. Reject changes requiring sequenced role/reset/termination procedures; direct users to typed operations. Mixed W1C/reset composition is deliberate, not generic RMW or unchecked arbitrary bus writes. |
| diagnostics | `drv`, aliases `health`, `state`; `diag`; `xfer_stats`, alias `counters`; `xfer_reset`; `xfer_assert <attempts> [successes failures]` | Cached projection/trust/transport summaries. Diag has zero traffic. Idle-only xfer_reset calls clearTransportStats and never repairs trust, consumes a result or resets managed health evidence. Assertions reject saturated totals and report counter source. |

P01/P02 must close the exact variant capability and control-value table before
those rows are implemented. TUSB322I advertisement writes remain unavailable
until the TI wording is resolved with evidence. Address discovery and bus scans
are outside this CLI. Do not import general-call reset, sensor calibration or
USB PD rows.

Repeated watch/stress sampling, register dumps and fixture suites belong to the
host runner. It issues bounded sequences of the ordinary commands above, consumes
each matching terminal result explicitly, and records run statistics on the host.
For dumps it selects a finite variant-allowlisted register list and documents
read effects. Firmware has no bulk diagnostic scheduler, second run ID, aggregate
result slot or hidden child-result consumption. This keeps every device feature
available through the same one-operation routes used by ordinary C++ owners.

Hardware-changing command names already express the requested action. Do not add
repeated `confirm`, arm/confirm cycles or conversational permission gates to normal
`apply`, `reset`, `irq ack` or register commands. Help identifies their effects;
automated mutation suites require explicit fixture/suite selection. Board-power
and bus-recovery actions need their own explicit application contract and are not
implied by these commands.

## Staging, applied state and IRQ acknowledgement

Maintain separate CLI staging, core retained desired and driver-observed/applied
evidence. Name their generations in output. There is no second CLI committed
profile and no profile commit command. Successful staging means local acceptance
only; accepted startApplyProfile captures the candidate as core desired. An
admitted job means ACTIVE, not hardware completion. Report hardware success only
from the matching terminal result and verification evidence.

Example: applied duty is 50%, staging changes it to 30%, and an IRQ is pending.
`irq ack` must preserve the observed/trusted applied RW fields in register 0x09;
it must not copy 30% out of staging or desired settings into the ACK write. An
unrelated settings write must compose W1C=0 and must not clear the latch. If a
safe applied/observed value is unavailable, perform the documented bounded read
or reject the write; never guess. Test desired!=applied, trust unknown, an IRQ
arriving between read/write, and failure after the ACK write. TUSB322I VCONN-fault
RCU means Read/Clear/Update; it does not establish read-to-clear behavior. Verify
clear/update semantics independently and gate unsupported actions while wording
is unresolved. Preserve raw fault evidence without promising a read clears it.

Invalid syntax, unsupported feature, busy owner or stale token leaves staging,
desired/applied generations, caches, retained result and callback counters unchanged
by that command handler. Already-admitted work may independently advance through
the turn's automatic poll. Cached/pure handlers have the same no-I2C scope; they
do not suspend existing work. Zero-delta handler tests use an idle driver or
disable automatic stepping, and separate tests prove automatic work still
progresses under cached/rejected command traffic. A raw
write can invalidate trust even when its callback reports failure. Subsequent
successful unrelated reads do not restore profile trust.

## Bounded parser, console and appearance

Use fixed storage: 192-byte line including terminator, at most 12 tokens, maximum
32 payload bytes per token plus storage for its terminator; help output is
incremental. Timeout arguments are
decimal 1..2147483647 ms, default 1000 ms, translated once into the chosen
uint64_t owner deadline using its validated half-range/modulo arithmetic,
including natural rollover. Callback timeout remains clipped
to remaining time. Host-runner counts are 1..100000 and intervals 1..3600000 ms;
validate a finite whole-run host deadline with checked arithmetic. Byte/address
values accept decimal or `0x` hex; other numbers have documented bases. Reject signs on unsigned values,
overflow, octal assumptions, NaN/infinity, junk suffixes, extra tokens, empty
required values and nonexact booleans/enums.

CRLF is one terminator. Backspace edits valid bytes only. Overflow, NUL or illegal
control bytes poison the entire line until newline: deleting the bad byte must
not turn malformed input into a mutation. Feed at most 64 input bytes and execute
at most one complete command per owner turn; stop remains responsive. Keep no
unbounded line queue. Report rejected complete commands rather than silently
overwriting a pending command.
Use one owner-poll call site per turn. Dispatch selects an explicit job-step
budget or the automatic budget, then that call site polls at most once. An
explicit zero budget suppresses automatic callbacks for the same turn.
Local-command callback counts describe the handler; core JOB/RESULT counts
describe its operation. Do not attribute an older automatic job's transfer to
the cached or rejected command merely because they share a loop turn.

Begin with an 8192-byte fixed output queue and 512-byte maximum formatted record.
Drain at most one nonblocking backend write of at most 256 bytes per loop turn.
Prove these values fit both supported board examples; an evidence-based smaller
queue is allowed when tests/docs agree. Drop whole records on overflow, increment
saturating dropped-byte/record counters, and emit an output-loss marker when
space returns. A machine record is never truncated into plausible success.
Terminal results remain queryable even when their announcement is dropped.
Before `result take`, reserve space for its complete bounded response, including
the copied RESULT and END, before releasing the core slot. If that space is not
available, leave the slot intact and report output loss/rejection when space
returns; do not defer a destructive take or add a second retained result cache.
Output loss inside a runner response invalidates that response. A disconnected
USB host must not stall polling, cancellation, transport deadlines or teardown.

Render `=== TUSB32x CLI Help ===` in cyan, `[Common]` and other section names in
green, command syntax in cyan in a minimum 32-character field, normal descriptions
after ` - `. Use ANSI reset, red errors, yellow warnings and optional gray detail;
`color off` removes all escape sequences. Include `[cached]`, `[live read]`,
`[stage]` or `[device write]` in descriptions. Typed status display is
`[OK|W|E] <ERR_NAME> detail=<signed decimal> <static message>`; text/color never
changes status meaning. Keep terminal output and docs in simple English.

## Machine records and result ownership

Use newline-delimited ASCII `TUSB <TYPE> key=value ...` records, schema version 1,
fixed keys per type, no duplicate keys, no ANSI within records. Symbolic error
names match `Err` exactly; signed detail is preserved. Avoid arbitrary strings in
values: use bounded fixed tokens or separate human messages. The host strips ANSI
only around human output. Unknown fields may be recorded; missing required keys,
duplicate keys and contradictory terminal records fail schema validation.

Every complete input line, including rejected commands, receives an increasing
nonzero uint64 `request` within the example session. Unknown commands, malformed
commands and poisoned lines use `command=invalid`; never echo arbitrary input
as a canonical command ID. Use the reserved `nonce=none` before the first marker.
At request-counter exhaustion, stop accepting commands, preserve current work
and results, and report session exhaustion; never wrap or silently reset IDs.
`hilmark <nonce>` echoes exactly the validated nonce and associates it with later requests until
the next mark; it does no I2C. An admitted job retains its original request/nonce
even when later input changes the marker. A runner uses
fresh begin/end markers and verifies the expected request, canonical command and
operation token within that frame. Cache queries receive their own request IDs;
result payloads also carry `origin_request` to identify the operation being
reported. Check request monotonicity on ACK/MARK records that acknowledge new
input lines. JOB/RESULT/END for an older admitted request may follow a newer
request's response: for example, apply request 24 may finish after job status
request 25. Correlate those records with the saved origin/token instead of
calling them request regression. A regressed new-input ACK/MARK, new boot banner,
identity change, session exhaustion or framing loss invalidates the runner
session. Do not claim the host revision proves firmware.

Illustrative record shapes below are future protocol examples, not test results:

```text
TUSB MARK schema=1 nonce=case_7_begin request=20
TUSB ACK schema=1 nonce=case_7_begin request=21 command=mode code=OK detail=0 action=STAGED staged_generation=4 desired_generation=2 applied_generation=2 configuration=KNOWN
TUSB END schema=1 nonce=case_7_begin request=21 command=mode outcome=SUCCEEDED code=OK detail=0 callbacks=0
TUSB ACK schema=1 nonce=case_8_begin request=24 command=apply code=IN_PROGRESS detail=0 action=ADMITTED token=17 staged_generation=4 desired_generation=3 applied_generation=2 configuration=KNOWN
TUSB JOB schema=1 nonce=case_8_begin request=24 token=17 state=ACTIVE callbacks=1 deadline_ms=12000
TUSB RESULT schema=1 nonce=case_8_begin request=24 origin_request=24 token=17 kind=APPLY_PROFILE state=FAILED code=I2C_TIMEOUT detail=-7 effect=INDETERMINATE callbacks=2 desired_generation=3 applied_generation=2 configuration=UNKNOWN
TUSB END schema=1 nonce=case_8_begin request=24 command=apply outcome=FAILED code=I2C_TIMEOUT detail=-7 token=17
TUSB PORT schema=1 nonce=case_9_begin request=30 raw_valid_mask=3 valid_fields=31 invalidated=0 attachment=DETACHED local_role=NOT_APPLICABLE peer_role=NOT_APPLICABLE orientation=NOT_APPLICABLE detected_current=NOT_APPLICABLE sequence=9 observed_ms=12050 observed_time_valid=1 age_ms=12
TUSB FIRMWARE schema=1 nonce=case_10_begin request=31 git=unknown tree=unknown build=unknown
TUSB FRAMEWORK schema=1 nonce=case_10_begin request=31 name=esp-idf version=unknown target=esp32s3
```

Finalize field ordering and maximum lengths in `tools/tusb32x_cli_contract.py` and
`docs/CLI.md` before parser implementation. Test each record's maximum with
32-character nonces, full-width integers and longest legal enum names, including
the newline and C terminator in its storage bound. Split an oversized schema
into explicit record types instead of truncating values. The ACK example is sent
before owner stepping; later JOB/RESULT describe the evolving configuration. Preserve the
start API's `IN_PROGRESS` on admission: it is expected progress, not a runner
failure. An `OK` staging ACK means local acceptance only. `HardwareEffect`
NONE/CONFIRMED/PARTIAL/INDETERMINATE, configuration
state and terminal operation state remain separate. A cancelled result may still
have CONFIRMED or INDETERMINATE effects. Callback failures and whole-job deadline
expiry preserve their distinct `I2C_TIMEOUT` versus `TIMEOUT` statuses.

PORT uses the baseline's raw/typed validity masks and exact enum names; there is
no whole-snapshot `valid` shortcut. NOT_APPLICABLE is a valid interpretation of a
detached observation. Use `observed_time_valid=0 age_ms=unknown` when no supported
observation time exists; do not calculate age from a default zero timestamp.
Include the snapshot's `invalidated` flag. Historical valid fields/time may
remain visible after invalidation; consumers require `invalidated=0` plus each
field's validity bit before using them as current port evidence.

ACK is an input-action response; END closes that request. Each input line has
one ACK, except hilmark uses MARK as its acknowledgement; do not emit both for
the same marker request. A local staged command has ACK/END immediately.
Admitted work has an IN_PROGRESS ACK and its END only
at terminal completion, after RESULT. A failed admission has a typed END and no
token or terminal operation result. A later cancel/status/result request has its
own END and cannot close another request by accident. All aliases normalize to
the same command ID; use fixed dotted IDs for subcommands where needed, such as
`job.step` and `result.take`. `result.take` END with OK reports successful result
delivery; its copied RESULT may still contain FAILED/I2C_TIMEOUT. Runner verdicts
read the payload outcome rather than treating delivery success as hardware success.

One admitted operation produces exactly one spontaneous terminal RESULT and one
END for its original request. A `result` query may repeat the retained payload,
with its own request and the same origin/token, and is not another completion.
`result take <token>` emits the copied payload and releases that exact slot;
wrong/stale tokens do neither. The runner consumes results explicitly before
the next operation; firmware never clears a result to make room. Record no bus
transaction/byte count unless the backend supplies it; tracked callback attempts
always include diagnostic and initialization traffic.

## Required execution tests

Use `test/cli/test_cli.cpp`, `test/console/test_console.cpp` and a persistent
`test/cli/cli_host.cpp` fixture linked to the actual shared sources and scripted
transport. Test each canonical/alias/help route against state, output and exact
callback deltas; execute Arduino/IDF adapters against that same contract. Add
`tools/test_cli_transcript.py` for the Python-to-live-C++ path. Validate positive
and deliberately broken records, malformed lines, output backpressure, token
reuse, interleaved old-job completion/new-query responses, request exhaustion,
pre-marker/invalid-command records, zero/one/N callback budgets, retained results
and failed take-response reservation, manual step with auto enabled without a
second poll, staged-versus-applied IRQ
ACK, unsupported variants and cancellation with uncertain effects. Static guards
supplement these tests; a matching command string is not proof of behavior.

Document every C++ CLI/console/helper public contract with Doxygen in the source,
including bounds, output callbacks, cached/live effects, lifetime and thread/owner
requirements. Add Python module/function docstrings for grammar and evidence
validation. Each stage updates README links, CHANGELOG, `docs/CLI.md`, command
coverage and validation evidence; P13 reconciles them rather than writing them for
the first time.
