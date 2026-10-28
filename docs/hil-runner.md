# Python sessions and evidence

Prompt 28 implements reusable framing, provenance, deadlines and saved outcomes
in `tools/tusb32x_hil_runner.py`, using the existing pure
`tools/tusb32x_cli_contract.py`. `tools/tusb32x_hil_artifacts.py` owns streamed
capture and atomic reports. Observation plans and fixture suites are later work;
no unimplemented suite is available as a command. These host tests are separate
from serial delivery, adapter timing and physical Type-C qualification.

## Read-only serial entry point

```text
python tools/tusb32x_hil_runner.py --help
python -m pip install -r tools/requirements-hil.txt
python tools/tusb32x_hil_runner.py --port COM7 --output build/session-001 --timeout 5 --write-timeout 1 --dtr unchanged --rts unchanged
```

COM7 is an illustration, not a discovered or selected device. Supply the actual
authorized port. The entry point performs a fresh marker, version, cached bus,
job-status, console and staged-profile exchange, then a fresh end marker. It does
not initialize the chip, apply settings, acknowledge an interrupt or reset it.
A foreign active operation or retained result blocks the session. Firmware can
advance work it already owns independently while processing a cached query.

Optional hardware I/O pins `pyserial==3.5`, checked against its
[official release](https://pypi.org/project/pyserial/3.5/) and
[API documentation](https://pyserial.readthedocs.io/en/latest/pyserial_api.html).
Parser/artifact tests and the actual C++ fixture need only the Python standard
library. There is no new Python/Git/serial dependency for C++ consumers.

Command/write timeouts must be finite positive seconds, at most 3600, and are
validated before creating or opening the backend. DTR/RTS accept on/off/unchanged;
explicit values are assigned to the closed handle before open. Requested values
are checkpointed before open; successful open also records the observed pre-open
properties. Unchanged makes no setter call, but does not guarantee the OS preserves
an existing electrical level. The driver may pulse or ignore these lines. Port
open has no portable deadline and can reset a board. No scan, enumeration, URL
backend, retry, reconnect, flashing, buffer flush or hidden reset is performed.
Real serial timeout/close behavior still needs backend and board qualification.

## Reusing a session

Session borrows an I/O object providing `read(size, timeout_seconds)` returning
bytes and `write(data, timeout_seconds)` returning the exact written count. Inject
a monotonic clock returning finite non-regressing seconds. The owner closes I/O
after handling the outcome; Session does not retain backend buffers or reconnect.

`begin(expected=...)` establishes fresh framing and firmware identity, rejects
foreign active/retained work and snapshots staging, automatic stepping, color
and verbosity. `command(text)` returns a bounded Exchange with ack/records/end.
An admitted exchange has `admitted=True`, IN_PROGRESS and no terminal END yet.
`wait_operation()` verifies its original terminal RESULT/DETAIL/END without taking
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
Original duplicate terminal pairs, wrong tokens, contradictory closures, LOSS,
SESSION exhaustion, regressed new-input IDs, new boot banners and identity changes
invalidate the session. Already received trailing/partial framing is checked
before another write and before successful finish. Notify reconnect explicitly
with invalidate, then create a fresh Session; no old restoration is replayed.

Host Git/tree metadata is read only for this exact checkout, not an enclosing
repository. It never substitutes for firmware library/version, Git/tree/build,
framework/version or target. All firmware identity fields must be known to meet
an expectation or authorize device mutations. Read-only framing can still record
unknown identity. Optional --expect-library/--expect-git/--expect-tree/
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
Failure to save returns incomplete, never a successful report claim.

Primary outcome, cleanup outcome and residual uncertainty are independent.
Declare required checks NOT RUN before attempting them. Every required check
must PASS; an empty proof set, unknown proof, failed cleanup or residual uncertainty
prevents complete PASS. Intent is checkpointed before local/device mutations and
exceptions/Ctrl-C are saved. Close failures are saved after attempting both serial
and transcript teardown. Failed work plus good cleanup stays failed.

`restore_local()` rechecks identity and restores only saved staging/output/auto
values in the still verified idle session, verifying returned values rather than
trusting OK alone. It never applies a hardware profile. Generations can advance;
restoring values does not roll history back. Session loss preserves unconsumed
tokens, uncertain device intent and unrestored local facts; it does not replay
them against a rebooted/reconnected target. `finish()` requires owned results to
be consumed, restores local values and verifies a fresh end marker.

| Exit | Meaning |
| --- | --- |
| 0 | Complete PASS with all required proof and no residual uncertainty |
| 1 | Test/protocol failure, including failed owned payload or session capture loss |
| 2 | Invocation, missing tool, blocked prerequisite, incomplete capture/report/cleanup |
| 130 | Saved interruption |

## Executed host tests

```text
python tools/test_tusb32x_hil_runner.py
python tools/test_cli_transcript.py --host build/native/test/Debug/tusb32x_cli_host.exe
ctest --test-dir build/native -C Debug -R "hil_runner|cli_transcript|cli_contract" --output-on-failure
```

The single hil_runner registration includes fake serial/clock and artifact fault
tests without pyserial or sleeps. The existing cli_transcript registration also
runs the actual Session against persistent compiled C++ CLI processes: fresh
markers/version, local staging, read-only admission, original completion, repeated
query, exact take and local restoration. A real fixture-injected I2C_TIMEOUT
retains INT32_MIN detail and a failed payload under delivery OK and good cleanup.
This is simulated software evidence, not successful physical hardware evidence.
