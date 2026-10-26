# Raw diagnostics and passive health

Prompt 17 implements the methods below in `<TUSB32x/TUSB32x.h>`. They use the
same callback boundary and safe composer as managed operations. The application
serializes every call, owns the bus and supplies terminal callbacks; no method
is ISR-safe. These host-tested contracts do not qualify a hardware adapter.

## Bounded raw access

`readRegisterSync(reg, out)` sends one pointer byte and receives one byte in at
most one callback. It accepts mapped 00..0A and present 45/A0. Base 320/I lacks
A0; TUSB322I lacks 45. Unmapped offsets return INVALID_PARAM; absent registers
return UNSUPPORTED_FEATURE. Valid requests then check NOT_BOUND, BUSY and
RESULT_PENDING in that order. Initialization and KNOWN trust are unnecessary.
Every error preserves `out`, including a callback that modifies its scratch
buffer before returning failure. Each request supplies its pointer again.

Raw reads do not publish identity, profile, port or IRQ snapshots, establish
initialization or repair trust. Contrary control evidence can invalidate trust
and dependent port validity. Reading a reset bit of one records MAY_BE_PENDING
without inventing a completion timestamp; subsequent reconciliation establishes
its own fresh 96-tick wait anchor. A later raw zero cannot prove completion.
Protected 45 bits 1:0 are outside profile equality. A contradictory read revokes
their write-image permission without blocking independent supported fields;
only complete managed proof renews that permission.

Status reads are not an explicit IRQ ACK. TI SLLSEN9F Table 7-7 p18 and
SLLSEP2D/SLLSEQ8D Table 9 p18 specify IRQ W1C at 09 bit4. TUSB322I SLLSEO5D
Table 6-9 p19 separately labels the VCONN fault RCU but does not define its
clear transaction. A raw 322I read is therefore not claimed to be a
nondestructive fault observation. No typed fault-ACK API is provided, and all
322I 08/09 writes remain gated. See [register access](register-map.md).

`writeRegisterSync(reg, value)` sends exactly `[register,value]` in at most one
callback, with no preparatory read. It rejects illegal images rather than silently
masking them. Read-only, absent and gated targets reject locally. Intrinsically
forbidden reset commands reject even before binding/slot checks. After those
checks, writes require initialization, KNOWN actual controls, valid relevant raw
evidence and resolved current write/sequence effects. Missing proof returns
CONFIGURATION_UNKNOWN without traffic. Staged desired settings never supply it.

| Target | Permitted raw image |
| --- | --- |
| 08 | Legal advertisement bits C0 only; low RO/reserved bits zero; no advertisement code3 |
| 09 settings | Persistent bits 06 on base, 07 on extended; W1C bit10 zero |
| 09 ACK | Bit10 plus the exact proved actual persistent fields; no combined settings change |
| 0A | Legal persistent fields F0/F7; reset08 zero; actual mode and disableTerm bits unchanged |
| Present 45 | Actual disableRdRp and protected03 unchanged; qualified internal-zero image required |

Actual mode or termination changes and all reset/recovery procedures require
their [typed operations](device-behavior.md); raw access grants no shortcut.
Equivalent 322I DRP encodings do not authorize raw mode-bit recoding. Every
ordinary raw write, including an equal-value write, invalidates configuration
and dependent port trust and records write uncertainty **before** callback
invocation. Success has PARTIAL effect until later complete proof; failure has
INDETERMINATE effect because acceptance may be possible. Desired values and
historical observations remain retained. Pure ACK preserves configuration/port
trust and confirms callback acceptance on success, without claiming the latch
stayed clear or updating the managed IRQ operation record.

Both methods use the configured positive aggregate millisecond timeout and have
no retries, time sampling, retained buffers or allocation. An aggregate timeout
argument does not itself prove backend blocking bounds. See [transport](transport.md).

## Independent evidence views

`transportStats()` copies saturating uint32 all-source `totalAttempts`,
`totalSuccess`, `totalFailures` and latest normalized `lastStatus`. Every actual
callback counts exactly once: raw Sync, initialization, reset, ACK and service
included. A combined callback may contain two physical transactions; these are
callback counts, not physical byte/transaction counters. Saturation means the
three totals need not remain arithmetically equal.

`diagnostics().managedTransport` copies the production subset, excluding raw
Sync. Managed callback success clears its consecutive-failure streak; raw traffic
does not. Callback I2C_BUSY counts as a failure. Local BUSY, invalid arguments,
waits, cancellation, callback-free deadlines, identity/configuration mismatch
and valid detach add no transport failures. Later CLI `probe` will call
initialization and therefore count as managed traffic.

The single transport wrapper accepts only OK and documented I2C_* outcomes.
Other callback codes normalize to I2C_ERROR while retaining full int32 backend
detail. `lastFailedTransportStatus` survives later success. Status-only callbacks
provide no completion time; affected time-validity flags remain false. Raw Sync
has no owner-time argument and cannot reuse a managed operation's old timestamp.

`diagnostics()` also copies binding, initialization, identity, configuration
trust, last managed operation kind/status, latest effect and independent unresolved
write, mode, reset and recovery evidence. A later read-only result may have
NONE effect while an older recovery obligation remains unresolved. Taking results
does not erase that obligation. Diagnostic reads cannot certify a historical
sequence or make partial profile bytes current proof.

`clearTransportStats()` clears only all-source totals and latest `lastStatus`.
It leaves managed history/streak, last failure, port age/validity, trust, effects,
active work and retained results untouched. All views and clearing are bus-silent.

## Pure optional health projection

`validateHealthPolicy(policy)` accepts every positive uint32 threshold; default
is 3. Zero returns INVALID_CONFIG. `projectHealth(evidence, policy, out)` applies
the following table to copied evidence, preserving all of `out` for invalid policy.

| Evidence | State |
| --- | --- |
| Not initialized, regardless of errors | UNINIT |
| Initialized, managed failure streak zero | READY |
| Initialized, positive streak below threshold | DEGRADED |
| Initialized, streak at or above threshold | OFFLINE |

Success copies the supplied evidence and effective threshold. Projection performs
no I2C, mutation, clock access, retries, scheduling or admission decisions.
Initialization/end/external invalidation govern readiness independently of counts.
READY may coexist with UNKNOWN configuration or invalid port fields. OFFLINE
does not block legal raw or managed work; a subsequent managed callback success
restores streak zero. Changing the threshold changes display only. The library
owns no recovery or USB power policy.

The registered `tusb32x_diagnostics` suite exercises raw framing and rejection,
possible failed writes, accounting, retained effects and the complete projection
table. See [test commands](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/README.md)
and [API contracts](api-contracts.md) for exact precedence and lifetime rules.
