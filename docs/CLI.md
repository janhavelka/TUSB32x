# CLI and cooperative operations

The implemented CLI is example code in
[examples/common](../examples/common/Tusb32xCli.h). It accepts local queries,
target/lifecycle edits, independently staged profiles and explicit managed operations.
Command handlers and admission perform **zero I2C callbacks**. Only `tick()` polls
the existing core once per owner turn. Neither setup nor opening a console binds,
initializes, resets, applies settings or acknowledges an interrupt. The application
serializes access and must not separately poll the same driver in that CLI turn.

`Cli` borrows a driver, transport configuration and platform hooks. Call `setup()`
once, then feed input before `tick()` in each serialized owner turn. Setup validates
the supplied hooks and bounded ASCII identity tokens, without output or callbacks.
Output is a borrowed nonblocking writer. The clock supplies the application's
uint64 millisecond domain; platform counters are explicitly reported cached
adapter evidence. No framework header, GPIO setup, platform clock or bus scheduler
is embedded in the common sources.

## Implemented commands

Names and enums are lowercase ASCII. Brackets mark optional arguments.
An omitted control value queries CLI staging. Setters change staging only.

| Topic | Grammar | Aliases | Meaning |
| --- | --- | --- | --- |
| common | `help [topic\|command]` | `?` | Incremental help for implemented rows |
| common | `version` | `ver` | Library version and separately supplied firmware/framework identity |
| common | `color [on\|off\|0\|1]` | — | Query/change human ANSI color |
| common | `verbose [0\|1]` | — | Query/change optional status chatter; errors remain visible |
| common | `hilmark <nonce>` | — | Change correlation nonce; emits MARK and normal END |
| connection | `bus` | — | Cached selected target, adapter framing, pins, timeout and callback counts |
| connection | `console` | — | Fixed console bounds, queue state and cumulative losses |
| connection | `capabilities` | — | Configured capabilities and independent attributed identity evidence |
| connection | `variant [name [address]]` | `model` | Query/edit an exact variant/address pair atomically |
| connection | `addr [address]` | — | Query/edit the strap-selected address; preserve staging |
| lifecycle | `bind` | — | Bind the selected target locally |
| lifecycle | `end` | `unbind` | End locally; preserve retained result and effects |
| lifecycle | `forget-evidence` | — | Explicitly discard target evidence while ended/idle with no result |
| port | `sample` | `snapshot` | Historical cached port observation with validity and provenance |
| port | `freshness` | — | Cached port observation and qualified age |
| profile | `profile [show]` | `cfg`, `settings` | Show staged, desired, observed and historical applied values separately |
| profile | `profile reset` | — | Stage candidate defaults; no silicon reset |
| profile | `profile discard` | — | Copy retained core desired settings to staging |
| profile | `profile validate` | — | Validate staging; establishes no device write permission |
| profile | `mode [port\|dfp\|ufp\|drp]` | — | Local role candidate |
| profile | `current [default\|1a5\|3a0]` | — | CC advertisement category; neither measured load nor PD |
| profile | `duty [30\|40\|50\|60]` | — | DRP source duty percentage |
| profile | `debounce [milliseconds]` | — | Exact variant's nominal debounce |
| profile | `preference [normal\|try-sink\|try-source]` | — | Extended dual-role preference |
| profile | `ufpaccessory [on\|off]` | — | UFP accessory enable, with raw disable polarity reported separately |
| profile | `termination [on\|off]` | — | Extended 0x0A disableTerm control |
| profile | `rd-rp [on\|off]` | — | Separate 0x45 disableRdRp control |

Implemented help topics are `common`, `connection`, `lifecycle`, `port`, `profile`,
`interrupt`, `job` and `all`. Help also accepts a canonical command ID or a listed alias; `profile`
selects its complete topic, and `profile.show` selects that single help row.
Internal IDs such as `profile.show` name machine responses; input uses `profile show`.
Unknown topics and later-stage commands reject as `INVALID_PARAM`.

The accepted variant names are `tusb320`, `tusb320i`, `tusb320li`, `tusb320hi`,
`tusb320lai`, `tusb320hai` and `tusb322i`. Addresses accept complete decimal or
`0x` hexadecimal values, without a sign or suffix. Base TUSB320/I accepts
0x60/0x61; the other supported I2C parts accept 0x47/0x67.

Target edits require an ended, idle driver without a retained result. The optional
address in `variant` validates the pair before changing either part, so
`variant tusb320 0x60` can replace LAI/0x47. An omitted address preserves the current
one and rejects an invalid pair. A successful actual variant change stages the
new part's candidate defaults and reports `staged_reset=1`; an address-only change
preserves staging. Local selection never silently forgets core target evidence.
An application's optional target validator can reject a borrowed adapter handle
that does not match the proposed target, before any selection or bind mutation.

`end` is local cancellation/unbinding, not hardware cleanup. Possible writes,
reset/termination evidence and terminal results survive according to the core
contract. `forget-evidence` is an explicit local loss of that evidence, never bus
recovery or a token/counter reset. Binding and target selection do not invoke it.

## Managed operations and retained results

These routes use the canonical core API shown; none implements register sequencing.
Optional timeouts are complete decimal values from 1 through 2147483647 ms,
default 1000 ms. Read/status and irq read take no timeout argument. Admission
samples owner time once and supplies the immutable uint64 `now + timeout` deadline,
including natural rollover. Core clips each callback to remaining time.

| Grammar | Aliases | Core API / local action |
| --- | --- | --- |
| `init [timeout_ms]` | `initialize`, `probe` | `startInitialize`: read identity and actual profile only |
| `read` | `status` | `startRefreshPort`: read-only complete port acquisition |
| `profile read [timeout_ms]` | — | `startReadProfile`: observe, never adopt desired |
| `apply [timeout_ms]` | — | `startApplyProfile(staged)`: capture desired only on accepted admission |
| `verify [timeout_ms]` | `selfcheck`, `selftest` | `startVerifyProfile`: compare retained desired, not board certification |
| `reconcile [timeout_ms]` | — | `startReconcile`: observe/prove without replay or cleanup |
| `irq read` | — | `startRefreshPort`: sample IRQ with port; no W1C ACK |
| `irq ack [timeout_ms]` | — | `startAcknowledgeInterrupt`: fresh actual controls, explicit ACK |
| `irq service [timeout_ms]` | — | `startServiceInterrupt`: bounded observe/ACK/recheck, coalescing events |
| `reset [timeout_ms]` | — | `startReset`: explicit command/wait/proof, no desired reapply |
| `recover-dead-battery [timeout_ms]` | — | `startRecoverDeadBattery`: original TUSB320/I only |
| `job [status]` | `progress` | Cached core progress and scheduler mode |
| `job auto [0\|1]` | — | Query/set automatic stepping, default on |
| `job step [0..255]` | — | Select this turn's one poll, default budget one |
| `cancel [token]` | `job cancel [token]`, `stop [token]` | `cancel`: local, no rollback |
| `result [token]` | `job result [token]` | `resultSnapshot`: repeat retained result |
| `result take <token>` | — | Exact-token `takeResult`, with complete response reservation |

Tokens are complete nonzero decimal uint64 values; zero, signs, overflow, hex and
junk reject before effects. Omitted query/cancel tokens use represented core work.
Dotted IDs such as `profile.read`, `job.status`, `job.cancel` and `result.take`
are metadata/help identities; command input uses the spaced grammar above.

One active operation and one immutable retained result remain in core. CLI stores
only the admitted request/nonce/token association and whether its spontaneous
completion was announced. A matching terminal snapshot emits one RESULT,
RESULT_DETAIL and END for the original request/nonce, even after newer queries or
markers. Loss drops complete records and marks the announcement attempted once;
the core result remains queryable. Repeated queries have their own request/nonce
and the same origin_request. Externally admitted, unattributed results use origin 0.

Automatic stepping advances only CLI-admitted work. An explicit job step can
advance represented application work by request, and replaces that turn's automatic
budget, including zero. Basic callbacks stop after at most one invocation even
with budget 255: fresh owner time is required afterward. A zero-budget turn can
validate time, expire a deadline, arm/finish a wait or publish pending success.
No callback loop, clock wait, retry, reset/resume policy or extra result slot exists.
Cached or invalid commands never suppress an older automatic job. Output congestion
and request exhaustion also leave polling, deadlines, cancellation and end active.

The step ACK accepts the local budget. Its JOB/END follow the sole poll. JOB
code/detail expose that poll's status; an invalid backward/ambiguous owner clock
causes a failed step END while preserving active work without traffic. An automatic
poll rejection emits a JOB on the original request. A terminal operation failure
still produces its original RESULT; step/take delivery OK does not rewrite it.

Before result take, serialized dispatch reserves five maximum records (2555 bytes):
ACK, optional human status, RESULT, RESULT_DETAIL and END. A still-unannounced
original completion adds three records (4088 bytes total). Insufficient capacity
returns BUSY and retains both core slot and loss evidence; the take is not deferred.
Reservation rejection alone does not increment loss counters. Its actual BUSY
records queue normally; only records that cannot queue count as dropped records.
Wrong/stale tokens never consume a slot. No new operation replaces an untaken result.

Initialization and reconciliation do not reset, acknowledge or replay staging.
IRQ ACK preserves actual observed/trusted duty/control bits even when staging or
desired differ; unrelated apply writes keep W1C zero. Missing/reserved actual
evidence never means equality. TUSB322I fault-ACK/advertisement writes remain gated;
unchanged proved advertisement permits independently supported profile changes.
Recovery is explicit original-part source-startup work: the application establishes
stable VDD and decides it is needed. Later variants reject without traffic. Cancel,
timeout and end retain possible writes, disabled termination and reset/recovery
evidence; they do not undo electrical effects. Autonomous TUSB322I VCONN and board
power policy remain separate from software delivery and communication success.

## Staging and cached evidence

Staging starts at generation one with a validated candidate. Candidate defaults
are DRP, default CC current, 50 percent duty, 168 ms debounce and neutral/enabled
applicable controls. These values are not a silicon reset image. Every successful
staging edit is validated before publication; rejected input changes neither
fields nor generation. Generations saturate at UINT32_MAX.

`profile reset` restores that candidate. `profile discard` requires retained core
desired settings; without them it rejects `CONFIGURATION_UNKNOWN` and leaves
staging unchanged. Staging does not replace core desired, observed or verified
applied settings. A field may be valid to stage even when its hardware write is
separately gated, including TUSB322I advertisement/duty/accessory controls.

Base TUSB320/I uses debounce 116/133/151/168 ms. Extended 320 and TUSB322I use
118/134/152/168 ms. Base parts have no preference, UFP accessory or 0x0A termination
control; only their neutral profile placeholders validate. TUSB322I has no 0x45
Rd/Rp control and does not support `mode port`. `on` means enabled:
`disable_ufp_accessory`, `disable_term` and `disable_rd_rp` show the inverse raw
control polarity, separately from semantic enable values.

CONFIG preserves the exact `raw_valid_mask` and `observed_valid_fields`, plus all
three has-flags and current configuration trust. PROFILE records carry source,
variant attribution, availability, generation and per-field validity. Staged fields
use the CLI-selected part. Desired/applied fields use the retained core association,
even after an ended local target edit; observed fields use the exact core validity
mask, including partially decoded observations where `has_observed=0`.
Absent or invalid fields print `unknown`, not neutral placeholder values.
Applicable masks are base 0x9B, extended 320 0xFF and TUSB322I 0x7F.

An applied record can remain available while current configuration is UNKNOWN.
Its generation identifies historical proof, not current usability. Identity is
attributed to the configured expectation; an ACK/shared signature never discovers
an exact variant. CAPABILITIES distinguishes that retained attributed evidence from
the current CLI-selected target. The configured write-capability flags describe
the part, not current admission prerequisites or guaranteed future write success.

PORT preserves raw/typed validity and invalidation independently. PORT_DETAIL
holds advertisement/accessory/cable/IRQ facts and acquisition configuration/
coherence. Unknown typed port enums use `UNKNOWN`. Numeric unavailable values use
`unknown`. Age is owner `now - observed_ms` only when time is valid and the modulo
elapsed interval is below 2^63; otherwise it is `unknown`. A nonzero age does not
repair invalidation or assert hardware coherence. CC indications do not authorize
VBUS policy, measure load or establish a USB PD contract.

## Bounded console and presentation

Input storage is 192 bytes including its final NUL, with at most 12 tokens and
32 payload bytes per token. Parsing validates the entire command before effects.
Only ASCII printable bytes and tab separators are accepted. CR, LF and CRLF are
line terminators, including split CRLF. Backspace and DEL edit valid bytes.
Overflow, NUL, other controls and non-ASCII poison the entire line through its
terminator; backspace cannot repair malformed input into a command. The next line
starts clean.

Across `feed` calls a turn accepts at most 64 bytes and one completed command;
unconsumed input remains the application's responsibility. A completed pending line
is never overwritten. `tick` starts the next turn. `processCommand` is the bounded
complete-C-string convenience entrypoint, sharing the one-command admission limit;
binary/control/fragmentation owners use `feed`.

Output uses 8192 fixed queue bytes and 512 bytes of formatted-record storage
including the final C NUL. A queued record contains 1..511 bytes including newline.
The NUL is never queued. Oversized, malformed or capacity-rejected records are
dropped whole, with saturating cumulative record and byte counters. Formatting
overflow counts its intended payload length; a formatting error with no meaningful
length records the 512-byte storage bound. No partial success record is queued.

Each `tick` makes zero or one nonblocking backend write, offering at most 256
contiguous bytes. Short/zero writes retain ordering; invalid acceptance counts
above the offered length are clamped. Output loss retains evidence and produces
a cumulative LOSS marker once space returns. Marker insertion is tried before
and after the one write; failed insertion does not add losses. `loss_pending=0`
means the marker was queued, not that a host received it. A disconnected host
does not cause retries, waits, busy loops or core polling.

Help attempts at most two queue insertions per tick, with a finite walk of fixed
rows; one row's human text and HELP metadata consume both attempts. Loss does not
stall the walk. The title is cyan, section names green and syntax cyan in a
minimum 32-column field before ` - ` and its cached/stage description.
`color off` removes every human ANSI escape. Machine records never contain ANSI.
Human status uses `[OK|W|E] ERR_NAME detail=<signed decimal> <static message>`;
verbose controls optional successful chatter and never hides errors.
Application-supplied status messages are scanned for at most 256 bytes. Only
complete messages of at most 255 printable ASCII bytes are displayed; absent,
unterminated, control-containing or non-ASCII text becomes a fixed omission
message. Typed code and signed detail remain unchanged, including with color off.

## Frozen machine records

[tools/tusb32x_cli_contract.py](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/tools/tusb32x_cli_contract.py) contains the
ordered required keys and implemented command rows. Its parser rejects duplicates,
missing required keys, bad tokens, unsupported schema, invalid enum/numeric values
and contradictions in local END status. Additive unknown fields are retained.
Transcript owners still check request ordering, matching nonce/command, and one
terminal closure; a parsed record alone proves none of those.

Records are newline-delimited ASCII `TUSB <TYPE> key=value ...`. The common prefix
is `schema=1 nonce=<token> request=<nonzero uint64>`, in that order. Nonce defaults
to `none`. `hilmark` accepts 1..32 ASCII letters/digits/underscore/hyphen, with `none`
reserved; it emits MARK rather than ACK and closes with END for command `hilmark`.
Every complete input line, including malformed input, receives an increasing
request. Unknown/malformed/poisoned commands use canonical `command=invalid`.
Aliases resolve to the same canonical command ID.

At request exhaustion, the CLI refuses new commands and preserves existing work,
results and pending help. It emits SESSION without wrapping or resetting IDs. If
output is full, the reason remains pending and is queued once capacity returns; zero-write ticks do
not repeatedly count the same notice as new loss.
SESSION and LOSS are explicit correlation exceptions with no request or nonce.
Any output LOSS invalidates a runner response that depends on the missing output.
VERSION reports package version; FIRMWARE and FRAMEWORK report injected firmware
identity. Host-checkout identity is never proof of flashed firmware.

The following keys follow the common prefix except SESSION/LOSS, whose complete
key lists are shown. ACK/END and helper records fit 512-byte storage with a
32-character nonce and full-width counters; PORT_DETAIL keeps extra facts separate.

| Type | Required keys after the common prefix |
| --- | --- |
| ACK | `command code detail action token staged_generation desired_generation applied_generation configuration` |
| END | `command outcome code detail token callbacks` |
| JOB | `token kind state phase code detail callbacks last_poll_callbacks deadline_ms waiting ready_at_ms ready_at_valid result_pending auto` |
| RESULT | `origin_request token kind state code detail effect callbacks desired_generation applied_generation configuration` |
| RESULT_DETAIL | `origin_request token started_ms completed_ms completed_time_valid termination reset irq_observed irq_before ack_attempted ack_confirmed irq_after irq_after_valid` |
| MARK | none |
| SESSION | `schema code detail reason` |
| LOSS | `schema dropped_records dropped_bytes` |
| VERSION | `library` |
| FIRMWARE | `git tree build` |
| FRAMEWORK | `name version target` |
| BUS | `variant address bound bus framing bus_hz timeout_ms sda scl irq adapter_available adapter_writes adapter_write_reads` |
| CONSOLE | `line_capacity input_budget output_capacity record_capacity drain_budget queued available dropped_records dropped_bytes loss_pending color verbose` |
| CAPABILITIES | `configured_variant observed_variant identity_valid identity_matches revision_valid revision_matches profile_fields advertisement_write duty_write accessory_write` |
| TARGET | `variant address bound staged_reset` |
| CONFIG | `raw_valid_mask observed_valid_fields has_desired has_observed has_applied desired_generation applied_generation configuration raw08 raw09 raw0a raw45` |
| PROFILE | `source variant available generation valid_fields mode current duty debounce preference ufpaccessory termination rd_rp disable_ufp_accessory disable_term disable_rd_rp` |
| PORT | `raw08 raw09 raw_valid_mask valid_fields invalidated attachment local_role peer_role orientation detected_current sequence observed_ms observed_time_valid age_ms` |
| PORT_DETAIL | `advertised_current accessory active_cable interrupt_pending config_generation configuration coherence` |
| HELP | `canonical aliases topic execution effect prerequisites capability records max_callbacks tx_bytes rx_bytes fixture test` |

ACK `action` is QUERY, LOCAL, UNCHANGED, STAGED or ADMITTED. An admitted operation
uses IN_PROGRESS and a nonzero token; failed admission uses token 0, UNCHANGED
and immediate END with no terminal operation result. Local failures are UNCHANGED.
Local END has SUCCEEDED for OK and FAILED otherwise, with callbacks 0; operation
END carries exact SUCCEEDED/FAILED/CANCELLED/TIMED_OUT and its callback count.
Result query/take END reports delivery independently of the copied payload failure.
Token 0 denotes a local response without an associated operation. `code` is the
exact public Err spelling and `detail` retains signed int32. Configuration is UNCONFIGURED, APPLYING,
KNOWN or UNKNOWN. Profile `source` is staged/desired/observed/applied; generation
zero means no attributed generation, including observed records. RESULT state is
the exact terminal core state; effect NONE/CONFIRMED/PARTIAL/INDETERMINATE is
independent. RESULT_DETAIL time values require completed_time_valid; zero or a
previous poll timestamp never substitutes for unknown completion. Its IRQ flags
belong to that operation, not a prior ACK. JOB phase is coarse public progress,
and ready_at_ms is meaningful only with ready_at_valid. No physical transaction
or byte count is inferred from callback counts.

HELP is executable metadata for the advertised subset: canonical ID/aliases,
topic, execution/effect, prerequisites/capability, record types, callback/TX/RX
bounds and host fixture/test. Rows use CACHE_ONLY, PURE or CORE_JOB, with explicit
effect classes. Managed rows
and job step advertise one callback per owner poll, at most two TX/eight RX bytes
per callback. These are ceilings, not physical transaction counts or whole-job
bounds. Job step conservatively advertises DEVICE_WRITE because already-admitted
work may write. All admission and other local handler callback bounds remain zero.
Core operation totals and timing remain in the procedure/API guides. Human
descriptions distinguish cached queries, staging, live reads and device writes.

Example correlation (illustrative, not hardware evidence):

```text
TUSB MARK schema=1 nonce=case_a request=1
TUSB END schema=1 nonce=case_a request=1 command=hilmark outcome=SUCCEEDED code=OK detail=0 token=0 callbacks=0
TUSB ACK schema=1 nonce=case_a request=2 command=mode code=OK detail=0 action=STAGED token=0 staged_generation=2 desired_generation=0 applied_generation=0 configuration=UNCONFIGURED
TUSB END schema=1 nonce=case_a request=2 command=mode outcome=SUCCEEDED code=OK detail=0 token=0 callbacks=0
TUSB LOSS schema=1 dropped_records=2 dropped_bytes=534
TUSB SESSION schema=1 code=TOKEN_EXHAUSTED detail=0 reason=request_exhausted
```

## Verification and later scope

From the source checkout:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug -R "tusb32x_cli|tusb32x_console|cli_transcript" --output-on-failure
```

The test-only `tusb32x_cli_support` target compiles the two common CPP files.
Applications compile those same files with `examples/common` on their include
path and link `TUSB32x`; core never links the CLI. The normal package includes
the four common files and this guide, while Python contracts/tests remain
source-checkout tools. No consumer linkage invokes Python or Git.

The current x86-64 host ABI measures `sizeof(Cli)=9416` bytes, including the
8192-byte output queue; this is not firmware RAM/link qualification. Board-specific
placement, front-end stack budgets and linked S2/S3 memory evidence arrive with
the actual firmware examples. No smaller unproved queue is substituted here.

The registered `tusb32x_cli` and `tusb32x_console` suites exercise the real shared
sources. `tusb32x_cli_host` is a persistent, explicitly fake-bus fixture used by
`cli_transcript`; it is not hardware evidence. A friend `CliTestAccess` exists only
for bounded host fixtures at uint64 request/uint32 generation limits. There is
no public seed setter or production boundary-manipulation command.

Prompt 21 completes operation admission/control/result routes. Prompt 22 adds
remaining cached IRQ/input-pin, invalidation, register/decode and diagnostic routes.
These remaining routes are not advertised or handled yet. Framework front ends,
serial/CC/INT fixtures and board memory/electrical qualification remain later work.
