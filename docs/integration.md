# Plain C++ and generic owner integration

The [runnable host example](../examples/host/basic/README.md) uses only ordinary
C++17, CMake and public TUSB32x headers. Its small transport and clock are explicitly
simulated; no physical device, framework or product is present. It demonstrates
binding, accepted operation admission, one-callback polls, exact-token delivery,
copied snapshots, external invalidation and caller-selected reconciliation.
The two-device owner fixture in `test/test_owner_scheduling.cpp` executes the
same public boundary with independently scripted bytes, faults and physical-time
observations. Neither consumer adds a scheduler or platform dependency to core.

## Mapping an application owner

| Owner concept | Library boundary | Application responsibility |
| --- | --- | --- |
| I2cOwnerTransport | DriverConfig callbacks/context and terminal Status | Bus lifetime and transaction serialization |
| Request slot | OperationToken, OperationTiming, start methods | Queue capacity and scheduling |
| Service turn | poll with finite callback budget | Fairness and current owner time |
| Completion | takeResult and OperationResult | Routing outcome/effects to requester |
| Diagnostics | TransportStats, DriverDiagnostics and snapshots | Required-device, stale and retry policy |
| Power/reset event | Local invalidation and explicit later operation | Supply sequencing and USB policy |

This mapping uses names from an inspected owner as evidence, not required types.
FieldCore-node's `include/TunnelMonitor/i2c/I2cTask.h`, `I2cOwnerTransport.h`,
`I2cDeviceBinding.h`, `I2cLifetime.h`, `include/TunnelMonitor/contracts/Time.h`
and `src/i2c/I2cTask.cpp` show externally serialized service turns, routed work,
binding lifetimes and monotonic deadlines. They are not include paths, lib_deps
or copied product policy. Current revisions and two generic peer comparisons
are recorded in the source-checkout audit/progress reports. No FieldCore wrapper,
product queue, inheritance hierarchy, singleton or required-device policy is
part of TUSB32x.

## Admit, advance and deliver

Create the application's bus/context before the driver and keep them alive until
every borrowing binding ends. An unchanged variant/address/context does not
establish target association if the callbacks changed; a shared null context
does not identify a bus. Configure exact callbacks, context, variant,
seven-bit strapped address and positive aggregate callback timeout in DriverConfig.
The owner selects the exact part: a shared identity signature is not model or
enable-polarity discovery. `validateDriverConfig` and `bind` do no I2C.

Sample fresh owner time and pass an immutable OperationTiming deadline to a
canonical start method. `startInitialize` is read-only identity/profile
observation; apply, reset, acknowledgement and recovery are separate explicit
operations. An accepted start returns IN_PROGRESS and a nonzero OperationToken.
That is accepted work, not failure or completed success. The application records
which requester owns the token and decides its own bounded queue capacity.
Tokens are instance-local: two drivers can issue equal numeric token values.
Retain the originating instance together with its token when routing work.

For each selected service turn, call `poll(token, freshNowMs, 1)`. At most one
terminal callback executes. A zero budget forbids callbacks while still allowing
local deadline/wait/result progress. An equal integer timestamp may be a fresh
sample after a short callback; repeatedly passing the old pre-callback sample
does not become fresh merely because the function was called again. Poll returns
without sleeping or holding a bus lock across turns. The application fairly
selects the next instance; a hardware wait or full result destination must not
monopolize a shared bus.

One active operation and one retained terminal result belong to each driver.
Replacement starts reject BUSY or RESULT_PENDING without traffic. Use progress
or `resultSnapshot` to inspect work, then `takeResult` with its exact token when
the requester has delivery capacity. Stale tokens preserve current work/results.
`takeResult` returning OK proves delivery only: inspect OperationResult.status,
state, hardwareEffect, configuration, generations and time-valid flags separately.
The host example deliberately receives delivery OK with a
CONFIGURATION_MISMATCH operation payload after external reset. Do not convert
that into successful reconciliation or silently reapply the desired profile.

Local validation, binding, starts, cancellation, cached getters/snapshots,
invalidation, result access and end are bus-silent. Explicit readRegisterSync and
writeRegisterSync diagnostics have their own one-callback bound and owner/result
exclusion. They cannot bypass sequenced silicon operations. TransportStats counts
callback attempts, including failed callbacks and raw access; a combined read can
contain pointer selection and receive as two physical transactions. Managed
health evidence excludes raw callbacks. No counter proves physical byte counts,
wire timing, successful attachment or power permission.

## Terminal callbacks and time evidence

Callbacks borrow their TX/RX buffers only until terminal return. They retain no
buffer, invoke no delayed DMA and never re-enter the instance. Serialize every
user of the same physical bus; release that lock before the next poll. The
application owns retry/recovery and teardown. Status messages have static
lifetime; preserve full signed native detail. Return only OK or the documented
I2C_* codes, with complete success for the requested length. The adapter must
reject short transfers before returning OK. Other codes are normalized to
I2C_ERROR; a callback I2C_TIMEOUT is distinct from owner TIMEOUT.

The timeout covers lock acquisition, all phases and cleanup together. A timeout
parameter alone is no blocking bound. Validate the selected backend's locks,
clock quantization, internal waits, synchronous lifetime and cleanup. Disclose
SDK-internal controller reset/bus-clear behavior and obtain application acceptance
where required; the core cannot remove those backend effects. See the
[native](idf-transport.md) and [Wire](arduino-transport.md) adapter contracts for
their narrower supported configurations. The host simulation's fixed local work
does not establish any real SDK/scheduling/physical timing bound.

Use the uint64 millisecond domain with forward/equal samples, unambiguous
intervals below the half range and each operation's bounded interval contract.
Natural rollover is valid; backward or ambiguous samples reject. Never extend an
admitted deadline. Resample after each Status-only callback; it provides no
completion timestamp. Core therefore defers time-sensitive proof to a fresh next
poll and retains its separate 1ms hardware-wait margin. A successful callback
returning late cannot retroactively make the operation on time.

FieldCore's I2cBusObservation can carry writtenBytes/readBytes, writeEffect,
durationMs and completedUptimeMs, plus owner clock hooks. Those richer fields do
not survive an ordinary TUSB32x Status-only bridge. No extra completion/effect
channel is implied here. Its inspected nowUsThunk is nowMs multiplied by 1000,
not independently measured microsecond precision. Do not forward stale owner
time or relabel a generic backend error as a specific NACK phase. Without stronger
acceptance evidence, an error after an attempted write remains conservative.

## External changes and caller-selected recovery

Use one configuration owner per physical target. Bus serialization permits two
different chips to share a bus, but cannot protect a cached write image if another
writer changes the same chip. Coordinate external writers, replacement, power
loss and reset with explicit local `invalidateDeviceState` notification.

Invalidation retains historical port/effect/result evidence with invalidity and
trust qualifications. A later successful raw identity byte or unrelated read
does not restore KNOWN configuration, repair freshness or prove a historical
reset/recovery sequence. The application separately selects `startReconcile`,
`startApplyProfile` or a fresh port observation. Reconcile observes/compares;
it does not replay desired settings. Complete actual settings can be KNOWN while
they differ from desired. Applied historical values, current observation and
desired intent remain separate.

Cancellation, deadlines and end do not undo hardware effects. Preserve the
retained result and diagnostic uncertainty after a possible write. Decide later
work from current evidence and application policy, not an automatic retry loop.
Original TUSB320/I source-capable startup after VDD return requires application
readiness/stable supply and explicit `startRecoverDeadBattery` after identity.
Consumer code never copies that register sequence. Later variants do not inherit
it, and reading ordinary control values does not prove that it ran historically.

## Teardown and executable evidence

Stop admissions, cancel active owned work as needed, inspect/take each owned
terminal result and call end on every binding while callbacks/contexts still
exist. Preserve uncertainty and route the outcome/effect to its requester. Only
then release application-owned devices/bus; never destroy another consumer's bus.
An ended driver can still deliver its retained result without I2C. Its destructor
is bus-silent, but arranging context-before-driver construction and explicit end
makes the borrowed lifetime clear. ISR code only signals the serialized owner;
no core calls, bus access, logging or blocking belong in the ISR.

The standalone commands and simulated output are in the
[host README](../examples/host/basic/README.md). Tests ON additionally registers
that same two-TU example as tusb32x_host_basic. Existing tusb32x_operations owner
cases cover two-target fairness, budgets, stale tokens, result backpressure,
cancellation, uncertain effects, external invalidation and explicit recovery.
The isolated tusb32x_portable_consumer imports only public headers/static library
and runs allocation/failure sentinels. Each public header also compiles as its
own translation unit; signature/header compilation is not a behavioral CTest.
None of these host consumers qualify physical Type-C, serial, IRQ or board timing.
