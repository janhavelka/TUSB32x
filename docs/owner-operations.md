# Owner operations, results and recovery decisions

One application owner serializes every call to each `TUSB32x::TUSB32x` instance,
including cached getters. It owns the bus, pins, locks, monotonic clock, scheduling
and USB power/data policy. An ISR only signals that owner; it performs no driver
calls. Different devices may share a serialized bus, but each physical target
needs one coordinated configuration owner.

This guide explains the operating workflow. The [API contract](api-contracts.md)
owns exact method admission, output, time and state tables. The
[device procedure guide](device-behavior.md) owns silicon sequences and per-variant
callback ceilings. [Integration](integration.md) maps this workflow to an ordinary
application or a FieldCore-style owner without introducing a product dependency.

## Bind, admit, advance and deliver

1. Create the application transport and an explicit `DriverConfig` with the exact
   part, strap address, callbacks and positive aggregate callback timeout cap.
   `bind(config)` validates and copies it without I2C. Keep the borrowed context
   and callback dependencies alive until `end()` returns.
2. Sample the owner clock and construct `OperationTiming{nowMs, deadlineMs}`.
   Call `startInitialize(timing, token)` and check `status.inProgress()` for
   admission. `Status::ok()` and explicit bool are false for `IN_PROGRESS`.
   Rejected admission preserves the supplied token and performs no callback.
3. On each service turn, sample fresh time and call `poll(token, nowMs, 1)` once.
   It performs at most one callback. Keep the token paired with this driver
   instance. Give other devices service turns while this one is waiting.
4. Inspect `progress()` when deciding whether the result is terminal. A rejected
   poll, such as backward time or a wrong token, is not terminal completion.
   `resultSnapshot(token, out)` can copy the terminal result repeatedly;
   `takeResult(token, out)` copies it and releases its slot exactly once.
5. Check delivery and operation outcome separately. `takeResult` returning `OK`
   means it copied a result; inspect `out.status`, `out.state`, `out.hardwareEffect`
   and the copied evidence to decide what the application does next. Failed
   delivery leaves `out` unchanged. New work is blocked by `BUSY` while active
   and `RESULT_PENDING` until the terminal result is taken.

The [runnable host example](../examples/host/basic/README.md) executes this full
flow against a clearly identified simulation. The [root quick start](../README.md)
shows the public calls. Neither is hardware qualification.

Initialization reads identity and controls without resetting, applying desired
settings, clearing IRQ or authorizing power. Shared signature bytes do not
identify the exact board part. The configured variant must match the BOM and
its [capability limits](variant-matrix.md).

## Choose the operation explicitly

| Application intent | Public operation and consequence |
| --- | --- |
| Inspect profile bytes | `startReadProfile` observes without establishing current trust or adopting desired settings. |
| Establish current identity and controls after invalidation | `startReconcile` performs read-only proof and any required retained-effect wait; it never replays desired settings. |
| Request changed controls | `startApplyProfile` requires proved actual settings, retains validated desired input at admission, writes only differences and verifies. A no-op apply only reads. |
| Compare actual controls with retained desired | `startVerifyProfile` reads and compares; complete differing proof can be `KNOWN` with `CONFIGURATION_MISMATCH`. |
| Obtain port data | `startRefreshPort` reads one fixed 09/08/09 sequence; it does not ACK IRQ or promote configuration trust. |
| Acknowledge IRQ | `startAcknowledgeInterrupt` reads actual controls and explicitly ACKs; `startServiceInterrupt` observes, ACKs and rechecks once. TUSB322I ACK/service remain gated. |
| Reset digital logic | `startReset` sends one qualified command, waits and verifies; it does not restore desired settings or prove attachment readiness. |
| Recover original TUSB320/I dead-battery startup | `startRecoverDeadBattery` requires the owner's stable-VDD decision and executes the qualified 0x45 sequence; later variants reject it. |

Read-only initialization and a clear 0x45 bit do not prove historical base
dead-battery recovery. A new recovery request is an explicit application decision.
Likewise, reset, mode changes and power interruption can affect attachment and
termination. The core cannot decide whether a source role or advertisement is
electrically appropriate for the connected supply and partner.

## Fresh time and callback limits

Use one `uint64_t` monotonic millisecond domain for admission, polls and cached
acquisition times, with resolution at most 1 ms. Valid intervals are strictly
between zero and 2^63 ms; natural uint64 rollover is supported. Extend a wrapping
32-bit clock in the application; a cast alone is insufficient. The
[time rules](api-contracts.md#owner-time-and-bounded-work) define exact unsigned
comparisons, invalid-time behavior and completion validity.

Sample again after every callback returns. A newly sampled equal tick is valid;
reusing the old pre-callback value is not fresh. The Status-only callback cannot
report completion time, so an OK callback yields `IN_PROGRESS`. The next valid
poll checks the immutable deadline before committing success or arming a wait.
Failed callbacks freeze their original normalized error immediately with unknown
completion time. A later timestamp does not replace that failure with timeout.

Hardware waits begin at the fresh post-callback boundary. The separate 1 ms
clock margin makes TI intervals of 5, 30 and 95 ms require 6, 31 and 96 elapsed
owner ticks respectively. It never extends the operation deadline. A zero
callback budget still processes bounded time/deadline/wait transitions; a budget
above one still permits at most one callback with the current callback types.
There is no sleep, busy wait or retry inside the driver.

The callback timeout is the smaller of the remaining deadline and configured
cap. It covers lock acquisition, all transfer phases and cleanup together.
Callbacks borrow buffers only until terminal return and must complete the entire
requested transfer before reporting OK. They retain no DMA/buffer work and never
reenter the instance. An aggregate timeout argument alone does not establish a
backend blocking bound; use the [transport contract](transport.md) and the
specific [Arduino](arduino-transport.md) or [native IDF](idf-transport.md) limits.

## Cancellation, invalidation and shutdown

`cancel(token)` performs no I2C. For active work it returns API `OK` and freezes
a `CANCELLED` result; for a matching terminal token it preserves that result.
Cancellation does not roll back writes. Before any mutating callback, the driver
records possible effects; every invoked failed write retains uncertain acceptance,
including `I2C_NACK_ADDR` and `I2C_BUSY`. Take the result and inspect its effect
even when the operation failed, timed out or was cancelled.

Notify external writes, replacement, reset or power loss with
`invalidateDeviceState(reason)`. This retains the binding and historical evidence,
invalidates current initialization/configuration/port usability and cancels
active work locally. `end()` also unbinds. Neither action changes an already
frozen result or undoes hardware effects. End every sharing instance before
destroying the shared borrowed bus context.

Retained write, mode, reset and recovery obligations survive result consumption,
end and same-target rebinding. A different target cannot silently inherit or
discard them. After ending and taking any result, `forgetDeviceEvidence()` is
the explicit unbound, inactive, result-free action to discard target evidence;
it performs no hardware recovery. See [binding and evidence lifetime](api-contracts.md#binding-and-evidence-lifetime)
for callback/context association, token continuity and exact guards.

## Interpret evidence before acting

Desired settings express a request. Observed fields describe acquired bytes;
verified settings describe completed proof and can become historical after
invalidation. A failed apply can therefore leave retained desired input,
`UNKNOWN` configuration and an `INDETERMINATE` effect together. Reconciliation
may later prove a different actual image without claiming that apply succeeded.

Port evidence has separate field validity, coherence, age and invalidation. A
complete refresh can report ordinary detach or changed read bookends; neither
is automatically an I2C failure. Hardware register reads are sequential, not
atomic. Desired role is not actual local role, and a CC current category is not
measured load or a USB PD contract. IRQ latches coalesce; successful ACK does not
promise that the line stayed clear. A complete pre-ACK service snapshot remains
available even if the subsequent ACK or recheck fails.

[Passive health](diagnostics.md#pure-optional-health-projection) describes managed
transport history only. `READY` may coexist with `UNKNOWN` configuration;
`OFFLINE` does not forbid a legal operation. Raw successful reads, result take
and statistics clearing do not repair configuration or port trust. A fresh
complete port publication alone clears port invalidation; age and field validity
still need checking before use.
