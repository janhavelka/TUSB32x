# Native ESP-IDF transport

`examples/esp_idf/Tusb32xIdf.h/.cpp` provides the example-only
`tusb32x_idf` callbacks for ESP32-S2/S3 and **ESP-IDF 5.5.5 and 6.1.0**. It directly uses
the native master API. It does not wrap Arduino, create buses, change speed,
retry transfers or implement USB policy. The root component still contains only
the portable core. The independent SDK fixture and host tests are software
evidence. The complete [native CLI application](../examples/esp_idf/basic/README.md)
now uses these callbacks with explicit UART routes, read-only startup and checked
resource teardown. Its target builds and board prerequisites are documented there.

## Application setup and lifetime

Create the bus and devices in application setup, check every creation result,
and populate one `BusContext` per physical controller and one `Context` per
target. `BusContext::config` and `Context::config` must be exact copies of the
settings used to create the borrowed handles. The native handles are opaque:
`validateContext` checks supplied metadata, not the hidden SDK objects. A copied
configuration or a non-null pointer cannot prove the association. Do not change
handles, address or configuration while any affected driver remains bound.

Required configuration:

- Native master with `trans_queue_depth=0` and no event callbacks. Nonzero queue
  depth alone enables asynchronous transfers in this SDK. No transaction may be
  pending when the adapter gains the bus.
- Default I2C clock source (`I2C_CLK_SRC_DEFAULT`), seven-bit addressing, ACK
  checking enabled and SCL in 100000..400000 Hz. Legal
  target addresses are 0x47/0x67 or 0x60/0x61; the core separately validates the
  exact selected variant. Callback address must match the device's recorded
  `device_address`. Register 0x45 and wire bytes such as 0x8E are not target
  addresses.
- This clock/rate subset is an adapter qualification restriction, not a TI
  minimum frequency. The pinned S2 HAL's REF_TICK/400kHz timing arithmetic
  underflows, and very low rates exceed hardware timing fields on S2/S3. See
  [S2 timing calculation](https://github.com/espressif/esp-idf/blob/v5.5.5/components/hal/esp32s2/include/hal/i2c_ll.h)
  and [S3 timing calculation](https://github.com/espressif/esp-idf/blob/v5.5.5/components/hal/esp32s3/include/hal/i2c_ll.h).
  Validation rejects other clocks/rates before SDK work.
- Create, use and delete the bus/interrupt on the same core. Set `ownerCore`
  to that core and pin the owner task there so it cannot migrate after the
  callback's check. No ISR calls or reentry. Both tested targets allocate the
  interrupt on the creating core; this excludes a concurrently executing read
  ISR when the synchronous task resumes.
- All bus users, including other libraries and device-handle management, share
  the single `BusContext::gate` and obey `retired`. The adapter tries the gate
  once. Contention returns I2C_BUSY with no SDK call, finite zero lock wait and
  no retry. The gate covers both STOP-separated phases and is released on every
  return. Never hold it across an owner poll interval.
- Compile with power management and I2C debug logging disabled and
  `CONFIG_LOG_MAXIMUM_LEVEL=0`. Version, target, logging and tick-rate guards
  reject builds outside the audited configuration. The tested rates are 100 and
  1000 Hz; the bound supports integer rates between them.
- Set `maxOverheadUs` only to a qualified whole-callback CPU/preemption bound:
  include scheduling, ISR/flash stalls, all clock reads, checks, copies, counters
  and gate release. Zero rejects setup. Tasks/interrupts/ticks must continue to
  run; a positive number alone is not measurement or qualification.
- Explicitly accept SDK-internal controller reset and bus clear with
  `acceptSdkCleanup`. Those effects may occur on errors and affect all devices.
  The adapter adds no recovery transaction or custom peripheral driver.

Call `validateContext` before binding on the serialized owner, without concurrent
callbacks or resource changes. It performs no SDK operation. Missing
resources, async settings, invalid address/length/speed, disabled ACK checking or
unqualified setup return I2C_ERROR. Defaults cannot become a successful half-bound
context. SDK creation/cleanup are application work outside the bounded callbacks.

Stop new admissions before shutdown, cancel/end every binding locally, inspect
and retain/take each exact result as its owner requires, then remove the
application-owned devices and delete the application-owned bus on its creating
core. Check cleanup results before releasing context storage. Never delete a
shared bus owned by another application. A failed cleanup requires keeping
remaining live resources/storage, not returning success and destroying context.
IDF 6.1 can return an error after freeing a bus; an error therefore leaves resource
ownership uncertain. The example attempts teardown once and retains static context
storage. Do not retry deletion using an uncertain handle.

## Aggregate timeout and buffer proof

The exact driver is
[IDF b774170ff46 i2c_master.c](https://github.com/espressif/esp-idf/blob/b774170ff46/components/esp_driver_i2c/i2c_master.c).
The installed file is byte-identical to official v5.5.5: 69,303 bytes, SHA-256
`884ee968512d8e375cd863641a325c4c8d98a21fa302b9cbcc5627f156dda790`.
The [SDK I2C guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/api-reference/peripherals/i2c.html)
describes the public calls; the bound below comes from the actual synchronous
implementation, not the timeout parameter name.

The second audited implementation is official
[IDF 6.1 i2c_master.c](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_driver_i2c/i2c_master.c).
Its synchronous short-transfer waits, cleanup and retained-read restriction preserve
the conservative bound below. CPU overhead includes its RCC critical-section
contention. IRAM-safe receive requires internal RAM; keep the callback stack in
internal memory. The exact-version gate accepts 5.5.5 and 6.1.0 only. The
repository-only [SDK audit](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-34-sdk.md)
records the source comparison; linked image evidence is recorded separately.

Let q = ceil(1000000 / tickRateHz) microseconds, T the positive SDK wait timeout
in milliseconds and C the qualified whole-callback overhead:

```text
B(T) = 7 * (1000*T + q) + 2 * (50000 + q)
write        <= C + B(T)
pointer/read <= C + B(T1) + 2 + B(T2)
```

| Source location | Bound contribution |
| --- | --- |
| `s_i2c_synchronous_transaction`, lines 1011-1012 | One bus semaphore wait |
| `s_i2c_send_commands`, lines 542-581 | At most four command waits for these short transfers |
| Lines 589 and 596-603 | Final event wait and additional NACK-busy wait |
| Lines 686-687, 553-559, 602 | Initial stale/busy reset plus at most one later bus clear |
| Lines 51, 95-104 | Each clear at most 50ms plus tick; strict greater-than comparisons |
| Transmit/receive operation lists, lines 1283-1296 and 1346-1350 | Three/four operations; TX <=2 and RX <=8 fit S2/S3 FIFO32, no refill loop |
| Lines 727-731, 1034-1037 | Interrupt mask/error reset and SDK bus-lock release before return |

The formula deliberately includes mutually exclusive waits. SDK reset without
bus clear and other CPU work belong to C. The adapter reserves both phases before
starting, then recomputes the second from the original callback start and remaining
budget. It never renews the deadline. Each wait is capped by `sdkTimeoutMs`, fits
uint16, and covers at least a whole tick so IDF truncation cannot turn it into a
zero wait. Zero/insufficient callback budgets return I2C_TIMEOUT before SDK work.
An error between phases issues no receive. A late success is rejected, but that
check only detects violation of the qualified overhead; it cannot bound earlier
blocking. An actual backend error keeps its original code/detail even when late.

The default core callback cap of 20ms cannot contain this conservative bound.
Configure a larger cap and an owner deadline that can contain all required
callbacks. For arithmetic illustration, C=2000us and a 10ms SDK wait cap fit a
600ms combined callback at either tested tick rate: the conservative bounds are
360002us at 1000Hz and 522002us at 100Hz. A 400ms budget fits only the former.
This is not a measured 2ms
application guarantee. Clipped nearly expired owner deadlines may legitimately
reject a transfer before traffic.

SDK operation descriptors retain buffer addresses internally even after terminal
return. The supported synchronous same-core configuration masks interrupts and
finishes/reset-stops transfer execution before returning. Caller RX is copied
only after complete success from an eight-byte local candidate. **A failed
receive retires the entire controller**: `contains_read` is set at line 296 and
cleared only by successful receive handling at line 777. No later bus operation
may revive a stale descriptor. All contexts sharing that bus return I2C_BUS
until the application ends bindings and tears down/recreates all SDK resources.
Never clear `retired` on a live bus. Result take, core end/bind or health do not
clear it. This conservative SDK restriction is not automatic core retry policy.

## Framing, status and time

`write` sends exactly pointer/data once. `writeRead` calls
`i2c_master_transmit` with an explicit one-byte pointer, waits a bounded 2us
STOP-to-START guard, then calls `i2c_master_receive` for 1..8 bytes. Both SDK calls
emit STOP; the application gate spans the gap. The guard covers TI's 1.3us
minimum. `i2c_master_transmit_receive` uses repeated START and is deliberately
not used. This follows SLLSEN9F section 7.5 p15, SLLSEP2D/SLLSEQ8D section 7.4 p15
and SLLSEO5D section 6.5 pp15-16, as recorded in the [transport guide](transport.md).
It does not claim an atomic multi-register observation or physical waveform proof.

ESP_OK maps to OK and returned ESP_ERR_TIMEOUT to I2C_TIMEOUT. All other errors
map to I2C_ERROR with full signed esp_err_t detail. In IDF 5.5.5,
`s_i2c_transaction_start` returns ESP_ERR_INVALID_STATE for several non-DONE
outcomes, including errors that lose more specific NACK/timeout information.
IDF 6.1 uses ESP_ERR_INVALID_RESPONSE for non-DONE outcomes instead. It remains
I2C_ERROR, with its original detail; it does not establish a NACK phase.
Do not infer an address NACK, partial byte count or no device effect. No owner
TIMEOUT is returned by this adapter. IDF has no short-count success interface:
its synchronous ESP_OK promises completion; fake tests can model partial data
only by also returning a failure. Caller output stays unchanged on every failure.

`BackendStats` saturates observable transmit/receive call counts and requested
payload bytes for SDK-successful calls. These are neither core callback attempts
nor physical transaction/byte counts: failure and SDK cleanup can add unreported
traffic. A combined callback normally makes two native calls. Statistics may
record an SDK success even when a later aggregate-time check rejects publication.

`ownerNowMs` floors native esp_timer microseconds to uint64 milliseconds without
a wrapping millis intermediate. The owner samples again after every callback;
these Status-only callbacks provide no completion timestamp. Core hardware waits
retain their separate 1ms software margin (6/31/96 ticks for 5/30/95ms policies),
and never extend the immutable operation deadline.

## Consumer and qualification

The root component has no SDK peripheral dependencies. An application component
adds the adapter source, its include directory and `REQUIRES esp_driver_i2c
esp_timer` alongside the root core component. See the relocatable
[component fixture](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/test/idf_sdk)
for exact CMake, target defaults and commands:

```text
scripts/pio.cmd run -d test/idf_sdk -e idf_s2
scripts/pio.cmd run -d test/idf_sdk -e idf_s3
```

Pinned pioarduino 55.03.311 selects native `framework-espidf` 3.50505.0 (IDF5.5.5).
Each target has its own generated sdkconfig and build directory. The fixture
links every advertised adapter function and the core without Arduino or the CLI.
Its main exercises local rejection only; it is not a board application.

Before physical use, qualify supply, external I2C pull-ups and pad levels, exact
ADDR/PORT straps, enable polarity/readiness and CC/VBUS wiring from the
[variant guide](variant-matrix.md). LAI/HAI VDD is 2.7-5.0V, 322I 4.5-5.5V;
LAI local 3A advertisement requires at least 3.5V and says nothing about sink
detection permission. Respect the guide's non-failsafe pin constraints and
unresolved startup wording; do not invent a universal enable pulse.
Host fakes prove adapter decisions, not interrupt scheduling, electrical timing,
SDK cleanup waveforms or physical terminal completion. Real SDK links prove
compilation/linkage, not flash, serial/HIL results or board qualification.
