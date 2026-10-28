# Python observation/fixture plans, sessions and evidence

The executable `tools/tusb32x_hil_runner.py` runs finite `readonly` and `soak`
plans from `tools/tusb32x_hil_plans.py`, using the existing Session and pure CLI
contract. `tools/tusb32x_hil_artifacts.py` owns streamed capture and atomic reports.
Explicit `orientation`, `accessory`, `role-current` and `interrupt` suites use
`tools/tusb32x_hil_fixtures.py` and its bounded setup/operator helper. Native/fake-serial tests
prove software behavior; physical serial, I2C timing, CC, VBUS and IRQ
qualification remain separate and NOT RUN. Read the
[hardware prerequisites](hardware-validation.md) before opening a physical port.
The runner is source-checkout tooling; normal library packages exclude tools and
tests. Their shipped documentation does not add Python to C++ consumption.

## Read-only serial entry point

```powershell
python tools/tusb32x_hil_runner.py --help
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite readonly --out build/readonly-plan
```

A dry-run needs no port or pyserial. It saves the selected target, normalized
suites, mapped register list, actions, count and timing in `metadata.plan`, plus
`report.json`, `summary.md` and an empty transcript. Plan validation is PASS;
primary and hardware remain NOT RUN, so the exit code is 2. A plan alone cannot
prove firmware identity or physical behavior. Use a fresh output directory.

The following is an operator-edited recipe, not an executed hardware result.
Install the optional serial dependency only for physical serial work, and enter
the authorized port and expected flashed-firmware identity yourself:

```powershell
python -m pip install -r tools/requirements-hil.txt
$observationPort = Read-Host 'Explicit authorized physical serial port'
$observationCommit = Read-Host 'Expected flashed firmware Git commit (40 hex characters)'
$observationFramework = Read-Host 'Expected firmware framework name'
$observationTarget = Read-Host 'Expected firmware target'
python tools/tusb32x_hil_runner.py --port "$observationPort" --variant tusb320lai --address 0x47 --expect-commit "$observationCommit" --expect-framework "$observationFramework" --expect-target "$observationTarget" --suite readonly --out build/readonly-hardware-001 --command-timeout 5 --write-timeout 1 --dtr unchanged --rts unchanged
```

The runner compares the selected variant/address with firmware records. It
does not change the target to make a plan fit; a mismatch blocks the run. It may
bind an ended, idle owner to its already selected target, then explicitly run
read-only initialization. A foreign active operation or retained result blocks
work. Readonly/soak never apply settings, reset, acknowledge IRQ or perform dead-battery
recovery. Firmware can advance already admitted work while handling cached input.

Physical serial uses the optional pinned dependency `pyserial==3.5`, checked against its
[official release](https://pypi.org/project/pyserial/3.5/) and
[API documentation](https://pyserial.readthedocs.io/en/latest/pyserial_api.html).
Parser/artifact tests and the actual C++ fixture need only the Python standard
library. There is no new Python/Git/serial dependency for C++ consumers.

All arguments, expectations, variant/address pairs, arithmetic and capture limits
are validated before serial open. Decimal unsigned fields reject signs, spaces
and suffixes; address additionally accepts a complete `0x` hexadecimal token.
`--out`/`--output`, `--command-timeout`/`--timeout` and
`--expect-commit`/`--expect-git` share their respective meanings. Command/write
timeouts must be finite positive seconds, at most 3600. DTR/RTS accept on/off/unchanged;
explicit values are assigned to the closed handle before open. Requested values
are checkpointed before open; successful open also records the observed pre-open
properties. Unchanged makes no setter call, but does not guarantee the OS preserves
an existing electrical level. The driver may pulse or ignore these lines. Port
open has no portable deadline and can reset a board. No scan, enumeration, URL
backend, retry, reconnect, flashing, buffer flush or hidden reset is performed.
Real serial timeout/close behavior still needs backend and board qualification.

## Readonly, soak and register observation

With no `--suite`, the runner selects `readonly`. Repeat `--suite readonly` and
`--suite soak` to select both; duplicates are removed in first-selection order.
Only selected suites execute. Both require initialization, exact owned results
and finite register observation. When both are selected, readonly runs before soak.
The readonly suite additionally checks cached
queries and malformed-input fences with automatic stepping disabled, changes
only CLI staging, validates it without apply, and checks verify/selfcheck/selftest
against an existing core desired profile or their CONFIGURATION_UNKNOWN rejection.
It reads the profile and complete port, checks zero/one-callback poll budgets,
queries retained results and explicitly takes each matching token. Separately,
already admitted read work progresses under cached queries with auto enabled.

Callback deltas use all-source counters with `job auto 0` for exact comparisons.
Initialization/profile observation/verification pass the existing optional
firmware timeout in milliseconds, clipped to the remaining command/run deadline;
the host still enforces its original deadline. Port read/status has a fixed
1000 ms firmware timeout and accepts no override. Slow serial/adapter/owner turns
can exhaust that bound; this is a failure with retained evidence, not a retry.
Port/soak reads use automatic finite polling so their owner deadline is not spent
on manual serial exchanges; initialization separately proves explicit zero/one
budgets. Scheduler settings are still captured and restored locally.
Handler callbacks, cooperative operation callbacks, adapter data-write callbacks
and backend physical transactions are separate quantities. A combined pointer/read
callback is one callback, even though it transfers a register pointer. The plan
requires adapter counter availability, checks that no data-write callback occurs,
and blocks rather than inventing that proof when the adapter cannot supply it.
Saturated adapter-write or all-source counters cannot prove exact deltas; regressed
or inconsistent totals fail. Exact staging/validation and rejected verify aliases
also require zero callback deltas.

Soak performs repeated complete port observations; count includes the immediate
first sample. Detached is a valid result. Each publication requires advancing
sequence, non-regressing acquisition time, valid raw and relevant interpreted
fields, and `invalidated=0`. A recent historical timestamp alone does not qualify.
Acquisition milliseconds use the firmware's uint64 forward/equal half-range rule,
including natural rollover; backwards or ambiguous half-range differences fail.
The host retains bounded counts and the last complete sample, with streamed
transcript evidence. No firmware bulk scheduler or second run token is added.
Useful attachment/local/peer fields must be known and mutually consistent with
their validity bits; optional fields may remain unknown. Fixed-read coherence
must be BOOKENDS_MATCH or CHANGED_DURING_READ, and raw bytes must be present.
Sequence saturation or non-advancement cannot establish this plan's freshness proof.
The hardware label in metadata.plan describes the saved input plan, not an
execution verdict; named checks and the primary/cleanup outcomes describe execution.

| Option | Default | Valid bound and meaning |
| --- | --- | --- |
| `--count` | 10 | 1..100000 soak observations |
| `--interval-ms` | 1000 | 1..3600000 ms; minimum gap after the preceding observation completes |
| `--duration-ms` | Unset | Optional soak-stage ceiling, 1..86400000 ms |
| `--run-timeout` | 60 | Finite positive seconds, at most 86400; includes handshake, operations, waits, result delivery and local cleanup |
| `--command-timeout` | 5 | Finite positive seconds, at most 3600 and no larger than the run deadline |
| `--write-timeout` | 1 | Finite positive seconds, at most 3600; additionally clipped to remaining exchange time |

For selected soak, `(count - 1) * interval_ms` must be strictly below both the
whole-run milliseconds and any explicit duration. Duration must not exceed the
whole-run ceiling. This leaves room for observation but does not guarantee the
chosen timeout can accommodate the work: actual command time and setup also
consume it. Unselected soak values still receive type/range checks. No interval
catch-up burst or renewed operation deadline is used. Waits use at most 50 ms per
sleep, clipped to the immutable stage/run deadline. Port opening itself has the
separate portability limit described above.

For example, after choosing the same explicit operator values above:

```powershell
python tools/tusb32x_hil_runner.py --port "$observationPort" --variant tusb320lai --address 0x47 --expect-commit "$observationCommit" --expect-framework "$observationFramework" --expect-target "$observationTarget" --suite readonly --suite soak --count 20 --interval-ms 1000 --duration-ms 30000 --run-timeout 90 --out build/observation-hardware-002
```

Register observation issues individual ordinary `reg` commands over this finite
variant allowlist. Each successful read must count exactly one callback.

| Selected variant | Ordinary register offsets |
| --- | --- |
| TUSB320, TUSB320I | 00..0A, 45 |
| TUSB320LI, TUSB320HI, TUSB320LAI, TUSB320HAI | 00..0A, 45, A0 |
| TUSB322I | 00..0A, A0; never 45 |

The [register contract](register-map.md) describes access effects. Status reads
are not an implied IRQ ACK on the six 320 parts. TUSB322I 09 includes an RCU fault
field whose clearing transaction remains unresolved; reading it is not claimed
non-destructive. Initialization and port observation also read 09. Ordinary raw
reads do not publish new profile/port snapshots. Contrary control evidence can
invalidate configuration trust and dependent port validity; see
[raw diagnostics](diagnostics.md). Read-only means no requested register-data
writes, reset, ACK or apply; it does not promise that reads leave silicon or
cached evidence unchanged.

Every admitted operation must reach its original matching terminal records and
have its result taken before the next admission. Repeated queries are copies,
not additional completions. Failure stops the run and retains the transcript,
unexpected effects, incomplete results and restoration uncertainty. Only saved
local staging, auto stepping, color and verbosity may be restored in a verified
idle session. Lost identity/session evidence prevents replay; no later apply or
hardware rollback is scheduled.

## Reusing a session

Session borrows an I/O object providing `read(size, timeout_seconds)` returning
bytes and `write(data, timeout_seconds)` returning the exact written count. Inject
a monotonic clock returning finite non-regressing seconds. The owner closes I/O
after handling the outcome; Session does not retain backend buffers or reconnect.

`begin(expected=...)` establishes fresh framing and firmware identity, rejects
foreign active/retained work and snapshots staging, automatic stepping, color
and verbosity, together with the target's reported profile capability mask.
Every Session registers required end-marker proof as NOT RUN, including reusable
API sessions. A successful begin alone cannot produce a complete PASS.
`command(text)` returns a bounded Exchange with ack/records/end.
An admitted exchange has `admitted=True`, IN_PROGRESS and no terminal END yet.
`wait_operation()` requires original JOB evidence and verifies terminal
RESULT/RESULT_DETAIL/END without taking
the result. `take_result()` sends the exact owned token, verifies the copied
payload and consumes the slot only on delivery OK. Inspect payload code/state,
signed detail, effect and configuration independently of delivery. Failed owned
payloads set a sticky failed check even when result delivery and cleanup succeed.

Only this run's admitted token can be queried, stepped, cancelled or consumed.
No replacement admission occurs before explicit consumption. Generic cancel,
job step and result aliases use the same ownership fences. Scheduler edits and
other local lifecycle commands refresh the idle prerequisite when no owned token
exists. Target queries remain available; target edits require a separately
established session rather than a hidden variant/address change during a run.
No device operation or safe-write composition is implemented again in Python.

Profile staging edits require an available saved baseline for the selected
variant. Its valid_fields must match CAPABILITIES.profile_fields, and every
applicable value must be known. Unavailable or contradictory staging still permits
read-only inspection; it cannot support an edit that would need later restoration.

Each top-level exchange has one absolute deadline covering logging, write,
fragments and closure. Per-write time is additionally clipped to its configured
ceiling and checked after return. An admitted operation retains its original
host deadline through later wait/query/marker calls; they cannot renew it. This
host deadline is distinct from firmware owner deadline_ms. Explicit later result
delivery may use a new deadline once the original completion was proved on time.
Pass the same explicit deadline to several calls when the application wants one
larger frame bound. Every I/O is checked before/after; received late bytes remain
in capture but cannot establish success. Serial reads use at most a 50ms backend
wait clipped to remaining time, with no new command timeout for each fragment.
An expired public deadline in an established session invalidates and checkpoints
the session before another write. Unconsumed tokens and unrestored local settings
remain recorded; a later call cannot renew the lost operation's evidence.

## Framing, schemas and provenance

The [CLI contract](CLI.md) and `SCHEMAS` in the pure CLI contract module define
required keys and typed values. Records are newline-delimited ASCII, schema=1;
duplicate/missing keys, invalid enums, oversized values and ANSI inside records
reject. CRLF is accepted. Bounded human/ANSI noise and echoes, including OK/PASS,
are never evidence. Unknown extension fields are retained without replacing
required fields or controlling singleton detection. Profile responses require
all four explicit staged/desired/observed/applied sources, including unavailable
values; unknown values do not become verified defaults.

Fresh begin/end nonces use 128 random bits, encoded as 32 ASCII hex characters.
MARK replaces ACK and still requires its matching END. Before the begin marker,
bounded stale records/boot noise are discarded; they cannot establish this run.
New-input ACK/MARK request IDs must increase without reuse. An older admitted
request's JOB/RESULT/END may follow a newer query or marker: original request,
nonce, command, kind and token stay associated. Query copies are compared with
the original payload and never counted as another completion. Nonce=none,
command=invalid and counter exhaustion keep their explicit protocol meanings.
Malformed arguments to a known route may receive command=invalid. The session
accepts only the exact bus-silent INVALID_PARAM rejection: an unchanged zero-token
ACK and matching failed END with zero callbacks, without payload records. It uses
the firmware rejection rather than duplicating the firmware's argument parser.
Original duplicate terminal pairs, wrong tokens, contradictory closures, LOSS,
SESSION exhaustion, regressed new-input IDs, new boot banners and identity changes
invalidate the session. Already received trailing/partial framing is checked
before another write and before successful finish. Notify reconnect explicitly
with invalidate, then create a fresh Session; no old restoration is replayed.

Host Git/tree metadata is read only for this exact checkout, not an enclosing
repository. It never substitutes for firmware library/version, Git/tree/build,
framework/version or target. All firmware identity fields must be known to meet
an expectation or authorize device mutations. Read-only framing can still record
unknown identity. Optional --expect-library/--expect-commit/--expect-tree/
--expect-build/--expect-framework/--expect-framework-version/--expect-target compare
explicit expectations. Variant/address, trust/generations and port masks,
invalidation, applicability and acquisition time remain separate record facts.

## Limits and artifacts

| Limit | Default | Finite ceiling |
| --- | --- | --- |
| Response payload bytes per exchange | 256KiB | 4MiB |
| Response machine records | 4096 | 65536 |
| Human/boot/ANSI noise bytes | 8192 | 65536 |
| Assembled line bytes | 1024 | 4096; machine schema remains at most 511 normalized bytes including newline |
| Read calls per exchange | 16384 | 262144 |
| Encoded transcript bytes | 32MiB | 128MiB |
| Retained transcript tail | 64KiB | 1MiB, no larger than transcript |

The original operation's byte/record allowance also persists across admission,
waits and interleaved queries; a new query does not restart that allowance.

CLI overrides exist for response/transcript/tail bytes; Limits and CaptureLimits
validate the other explicit API limits. Reaching a cap fails/incompletes evidence.
The transcript contains timestamped escaped TX/RX bytes streamed directly to disk.
RAM retains only the fixed tail, counters, current bounded response, one owned
operation and local snapshots; it contains no full transcript journal or growing
token/request history. TX/RX counters measure captured payload bytes, not I2C
bytes/physical transactions. Capture records count logging calls, not commands.

Use a fresh output directory: transcript.log is created exclusively. Each
checkpoint validates bounded metadata/checks and atomically replaces report.json
then summary.md using flushed same-directory temporary files. Each file is atomic;
JSON is authoritative if replacing the summary fails. Temporary files are removed
on failure. At most 1024 required checks, 64 residual facts and a 512KiB JSON report
are accepted. Reports include at most 64KiB of the retained tail and count omitted
bytes. Capture loss is sticky; later checkpoints can still save that failure.
The bounded save_error field preserves the first checkpoint I/O failure and
prevents exit zero. If summary replacement fails after JSON was published, one
best-effort corrective JSON replacement records the save error and nonzero outcome.
The summary is not retried. Failed first or corrective JSON replacement preserves
the existing bytes and raises the save failure; unavailable storage may prevent
persisting the newest failure. The tool reports that limitation and returns a
nonzero exit instead of claiming a successful final save.

Primary outcome, cleanup outcome and residual uncertainty are independent.
Declare required checks NOT RUN before attempting them. Every required check
must PASS; an empty proof set, unknown proof, failed cleanup or residual uncertainty
prevents complete PASS. Intent is checkpointed before local/device mutations and
exceptions/Ctrl-C are saved. Close failures are saved after attempting both serial
and transcript teardown. Failed work plus good cleanup stays failed.
Ctrl-C during the final checkpoint marks interruption and makes one further save
attempt, returning 130. A second interruption or storage failure can prevent that
interruption from being saved and is reported explicitly.

`restore_local()` rechecks identity and restores only saved staging/output/auto
values in the still verified idle session, verifying returned values rather than
trusting OK alone. Profile restoration verifies availability, variant, the complete
applicable validity mask and all saved applicable values in a final profile query.
An individual setter requires its own saved field bit. A contradictory cleanup
response invalidates the session; a later attempt cannot turn that loss into PASS.
Public API checkpoints also save late interruptions, including after result take
or final-marker closure. It never applies a hardware profile. Generations can advance;
restoring values does not roll history back. Session loss preserves unconsumed
tokens, uncertain device intent and unrestored local facts; it does not replay
them against a rebooted/reconnected target. `finish()` requires owned results to
be consumed, restores local values and verifies a fresh end marker.

| Exit | Meaning |
| --- | --- |
| 0 | Complete PASS with all required proof and no residual uncertainty |
| 1 | Test/protocol failure, including failed owned payload or session capture loss |
| 2 | Dry-run without hardware proof, invocation, missing tool, blocked prerequisite, incomplete capture/report/cleanup |
| 130 | Saved interruption |

## Executed host tests

```text
python tools/test_tusb32x_hil_runner.py
python tools/test_tusb32x_observation.py
python tools/test_cli_transcript.py --host build/native/test/Debug/tusb32x_cli_host.exe
ctest --test-dir build/native -C Debug -R "hil_runner|cli_transcript|cli_contract" --output-on-failure
```

The single hil_runner registration includes planning/pre-open/dry-run guards,
fake serial/clock and artifact faults without pyserial or real sleeps. The
existing cli_transcript registration runs complete plans against persistent
compiled C++ CLI processes with scripted hardware, covering local restoration,
retained-result query/take, callback budgets and observation validity. Malformed
requests, failed callbacks, output-full result take and a legal original
completion following a newer query retain executable protocol coverage. A
fixture-injected I2C_TIMEOUT retains INT32_MIN detail and a failed payload under
delivery OK and good cleanup. This is simulated software evidence; physical
serial/I2C/CC/IRQ/VBUS qualification remains NOT RUN.

## Explicit physical fixture suites

Repeat `--suite orientation`, `--suite accessory`, `--suite role-current` and
`--suite interrupt` to select this work. There is no `--all` or implicit mutation.
Selection authorizes the listed ordinary CLI mutations, not power control,
flashing, a VBUS switch, autonomous retries or a change to board USB policy.
Readonly remains the default. Consecutive observation and fixture groups retain
selection order, one open transport and one immutable whole-run deadline. Each
group finishes and restores local values before establishing the next session.

Every fixture run requires `--fixture <id> --fixture-config <path>` and expected
flashed `--expect-commit`, `--expect-framework`, `--expect-target`. Unknown firmware
identity blocks physical transitions and writes. Missing equipment or operator
input reported as skip/EOF is NOT RUN; unsupported variant/setup is BLOCKED.
Contradictory observations, late input or failed operation payloads FAIL. These
outcomes cannot become PASS through cleanup.

Setup is strict UTF-8 JSON, at most 16KiB, with no duplicate/extra/missing keys.
The example below is an operator-edited declaration, **not executed evidence**.
Replace each identity, wiring/strap, measured supply and capability with the
actual authorized setup; save it outside shipped core as a local JSON file.

```json
{
  "board": "operator-supplied exact board and revision",
  "part": "operator-supplied full assembled part number",
  "variant": "tusb320lai",
  "adapter": "operator-supplied adapter and firmware identity",
  "wiring": "exact SDA/SCL/INT/CC and resistor-coupled VBUS_DET connections",
  "addr_strap": "actual ADDR strap selecting 0x47",
  "enable": "actual EN_N active-low wiring and asserted level",
  "supply_mv": 5000,
  "partner": "identified controlled reversible dual-role partner/fixture kit",
  "operator": "responsible operator and declared power/fixture assumptions",
  "capabilities": ["orientation", "audio", "debug", "role-current", "interrupt"],
  "startup_ready": true,
  "base_recovery_complete": false
}
```

Strings are printable ASCII, at most 256 characters (wiring 1024). Fixture IDs
use 1..64 letters/digits/`_.-`. Capability names are exactly `orientation`, `audio`,
`debug`, `charge-through`, `debug-local-dfp`, `debug-local-ufp`, `role-current`,
`interrupt`; no duplicate names. The two readiness booleans are optional in JSON
but startup_ready must be true to run. Original TUSB320/I also requires the
application's base_recovery_complete declaration after stable VDD and identity;
the runner does not perform dead-battery recovery or infer it from a register.
Wiring must explicitly describe VBUS_DET; parsing that text cannot prove safety.
See [electrical prerequisites](hardware-validation.md) for the actual pad limits.

All TUSB320 VDD is 2700..5000mV; HI/HAI's 1050..3650mV range describes EN,
not VDD. TUSB322I VDD is 4500..5500mV. Role-current setup requires at least3500mV
because it includes local 3 A advertisement. Every source-capable apply,
including cleanup, separately checks the candidate current and measured VDD;
an existing sink profile with 3 A cannot bypass this check by changing mode.
Remote current detection has no local 3 A advertisement prerequisite.

This saves a concrete plan without opening a port; exit2 and hardware NOT RUN are
expected, including when equipment/setup is missing:

```text
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite orientation --suite accessory --suite role-current --suite interrupt --out build/fixture-plan --run-timeout 1800
```

For real work add the operator-selected port, actual setup/fixture ID and expected
firmware values using the variables in the earlier recipe. Use a fresh output
directory. Set --run-timeout for the entire finite plan, including physical
transitions and cleanup (maximum86400s). Each operator prompt has at most300s,
clipped to that immutable deadline. On a local TTY, type exactly `ready` only
after performing and externally observing the displayed transition; it attests
the same declared target, partner and measured supply. Type `skip` to leave proof
NOT RUN, or Ctrl-C to save interruption130. There is no unbounded input/readline
thread: Windows key polling and POSIX ready reads accept at most64 characters,
with at most50ms waits and checked monotonic time. Local prompt output assumes
the host output sink returns; OS console/serial-open blocking still needs host
qualification. A host or firmware report cannot replace physical CC/INT evidence.

| Suite | Required fixture and finite work |
| --- | --- |
| orientation | Identified reversible source partner; detached baseline, UFP apply/verify, both CC1 and CC2 with valid local-sink/peer-source fields, then detach |
| accessory | Each explicitly declared exact accessory type/wiring; detached DFP apply (UFP for debug-local-ufp), valid typed accessory/raw bytes, detach after each. Base audio/debug/charge-through supported; base debug-local variants denied before traffic |
| role-current | Controlled same dual-role peer and selected safe power capability; sink detects externally varied default/1.5A/3A while attached; detach, source advertises each category through stage/apply/verify and external CC observation; detach, DRP resolves as sink; detach |
| interrupt | Wired, opt-in application INT input checked before I2C; applied50/staged30 ACK preserves50; explicit reassertion; unrelated duty30 apply leaves latch pending; multiple physical events show coalescing; one service proves ACK/recheck and INT deassertion; detach |

Original TUSB320/I uses **explicit reset operations** after each attached peer
advertisement change (SLLSEN9F7.3.1.2p11), followed by identity, reconciliation,
profile and port evidence. A read alone cannot prove that current update.
Later variants do not inherit these resets. Their successful register observation
still does not resolve all TI wording about physical attached-current tracking.
CC categories are neither measured load nor PD contract nor authorization to draw
power. TUSB322I fixture plans are BLOCKED before initialization/I2C: advertisement,
mixed09/fault-clear and return-to-DRP gates prevent this complete mutation/cleanup
plan. Its read-only observation remains separately available with access context.

Each operation admits once, uses at most512 explicit job-step1 exchanges with
at most5ms sleeps, and takes the exact owned original result once. The command
deadline includes admission, waits and take. Rejected admission, late callback,
cancellation, failed payload or BUSY take stops the plan; no uncertain write,
ACK, reset, cancel or take is automatically retried. Port reads retain the CLI's
fixed1000ms owner timeout. Up to64 physical steps save bounded attestation plus
raw/typed/validity/sequence/time evidence; no growing transcript journal is added.

Before writes, capture firmware, configured target, observed profile/trust and
generations, current coherent attachment and supply assumptions. Stage from the
proved actual profile, not CLI defaults. Generations remain history. On successful
plans only, cleanup requires a new original-partner/supply attestation, fresh
identity/target, detached port and known profile; restore the original actual
profile through apply/verify. This restores settings, not past USB events or the
original desired-profile generation. The original desired snapshot is preserved
in the report; cleanup never blindly reapplies it. Refuse cleanup if the partner,
target or power assumptions changed. Never restore an old source role on another
partner. Device failure/uncertainty stops without automatic hardware restoration.

`metadata.hardware_cleanup`, primary outcome and local restoration are separate.
`fixture_progress` counts confirmed transitions, consumed operations and
mutation_requests (including no-op/rejected requests), not writes or bus transactions.
Successful Session.finish restores staging/auto/color/verbose and cannot overwrite
BLOCKED device cleanup. Residual effects, owned tokens and historical observations
remain saved; an explicit later owner decision must resolve them. Missing cleanup
proof prevents overall PASS. Transcript fixture reports label modeled observations
and physical hardware NOT RUN; real connector/INT/electrical evidence is separate.
