# Arduino Wire transport

`examples/arduino/Tusb32xWire.h/.cpp` implements terminal callbacks for
pioarduino 55.03.311, Arduino 3.3.11 and ESP-IDF 5.5.5 commit `b774170ff46`, on
ESP32-S2/S3. It is example code, separate from the portable core and shared CLI.
Version/configuration guards reject unsupported builds, including power management
and I2C debug logging. SDK link fixtures are build checks. Complete CLI/owner
applications are described in the [Arduino examples guide](arduino-examples.md);
neither kind of build proves physical qualification.

## Application prerequisites

One `tusb32x_arduino::Context` represents the whole physical controller, shared
by all drivers and every other bus user. The application owns Wire, pins, speed,
power, preparation, teardown and the context's lifetime.

1. Compile **all** Arduino/Wire/HAL sources with `CONFIG_DISABLE_HAL_LOCKS=1`
   and `CORE_DEBUG_LEVEL=0`, as in the fixtures. Do not mix Wire class layouts
   from different flags. Internal `portMAX_DELAY` locks otherwise escape the
   callback deadline. Disabling them requires application serialization for
   every peripheral affected by that global configuration, not just this adapter.
2. Use a task pinned to the core that successfully initializes Wire. Record that
   core in `ownerCore`. Callbacks reject a different core and ISR context;
   the application must also prevent migration after the check. Initialize and
   tear down on the same core. Use synchronous Wire master mode only.
3. Successfully initialize buffers (SDK default 128, minimum 32) and create each
   target's HAL device handle during application setup, before bounded owner work.
   A deliberate pointer-write/read of a documented read-only identity address
   through Wire can prepare a handle; check every return. This is setup traffic,
   outside core initialization and the bounded callbacks. If any setup receive
   fails, stop and tear down the controller before reuse. Set `preparedAddresses`
   only for prepared targets: bit0=0x47, bit1=0x67, bit2=0x60, bit3=0x61.
   Never resize/end/reconfigure Wire while drivers are bound.
4. All users acquire the **same** `Context::gate` with a nonblocking try, check
   `retired`, and release after the complete terminal access. The adapter does
   this itself. No HAL/IDF/Wire traffic may bypass it, including other sensors.
   No outstanding repeated START or asynchronous transfer is allowed. Contention
   returns I2C_BUSY immediately, without a wait or retry.
5. Keep SDK I2C logging disabled. HAL disables `i2c.master` logging during setup;
   do not reenable it. Qualify `maxOverheadUs` for the **whole callback**, including
   CPU work, all scheduler/ISR/flash stalls, counters, clock reads, timeout
   restoration and gate release. Zero rejects setup. A guessed positive value
   is not qualification. Reject integration if these delays cannot be bounded.
   Host tests do not measure ESP32 worst-case scheduling.
6. Set `acceptSdkCleanup` only when SDK controller reset and bus-clear pulses on
   errors are acceptable for all bus devices. The adapter adds no retry, cleanup
   transaction or recovery and never calls Wire.begin/end.

`validateContext` is a bus-silent check before bind. It cannot inspect private
Wire buffers/handles or certify scheduling. Defaults are deliberately invalid.
DriverConfig still supplies the exact variant/address; a prepared address is
not chip identity.

The conservative SDK reserve exceeds the core's default 20ms callback cap.
Choose a larger `DriverConfig::i2cTimeoutMs` and an operation deadline large enough
for its callbacks. For example, with a **separately qualified** 2ms whole-callback
overhead and a 10ms Wire wait cap, 400ms fits both phases. This is arithmetic, not
a measured 2ms board guarantee. Small budgets fail before backend traffic.

Arduino macros DEFAULT and INTERRUPT also name core enum values. Include portable
headers before Arduino headers and locally `#undef` those macros before spelling
these enum values in board code. The SDK fixture needs no Arduino.h. No framework
includes or compatibility aliases were added to the core.

## Aggregate deadline proof

Let q be a FreeRTOS tick in microseconds, rounded up, T the positive Wire wait
setting in milliseconds, and C the qualified whole-callback overhead. Reserve:

```text
B(T) = 7 * (1000*T + q) + 2 * (50000 + q)
write callback        <= C + B(T)
pointer/read callback <= C + B(T1) + 2 + B(T2)
```

The extra 2us explicitly guards TI's STOP-to-START minimum. The pinned packages
use q=1000us. The adapter reserves both phases before traffic, then clips phase
two from the original start and remaining budget. Each setting is capped by
`wireTimeoutMs`, fits uint16 and covers at least one whole tick. No phase receives
a renewed whole-callback budget. TX=2 writes and TX=1/RX=1..8 reads fit FIFO32.

The [exact IDF driver](https://github.com/espressif/esp-idf/blob/b774170ff46/components/esp_driver_i2c/i2c_master.c)
has up to four command semaphore waits, one mutex wait, one final event wait and
one NACK-busy wait. Short commands advance the finite command list without FIFO
refill. One initial stale-status/busy clear and one later failure clear each
take at most 50ms plus a tick. Extra tick allowances cover strict greater-than
comparisons. This deliberately overestimates mutually exclusive/uncontended waits.
Prewarmed handles avoid allocation in `i2cAddDeviceIfNeeded`; exclusive ownership
excludes indefinite contention in IDF device lists/locks.

[Wire](https://github.com/espressif/arduino-esp32/blob/3.3.11/libraries/Wire/src/Wire.cpp)
and [HAL](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/esp32-hal-i2c-ng.c)
pass the setting into these paths. HAL uses `trans_queue_depth=0`. IDF masks
interrupts before synchronous return and resets its FSM on error. Same-core
execution excludes a concurrently executing ISR surviving that return; interrupt
allocation follows the [initializing core](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/api-reference/system/intr_alloc.html).
SDK transfers use Wire-owned buffers. Caller RX is copied only after complete
success; no pending transfer retains caller buffers. A late-return check catches
qualification violations but does not prove or repair a bound already exceeded.

## Failures and ownership

Wire 0 means success; 5 maps to I2C_TIMEOUT. Other Wire codes map to I2C_ERROR with
the numeric code intact. Code 2 conflates ESP_FAIL and ESP_ERR_NOT_FOUND; it proves
neither address NACK nor absence of device effects. `requestFrom` hides ESP errors.
A short receive reports only its returned length; a failed `read` preserves its
integer return. No unavailable error or physical byte count is invented.

A short buffered write only calls `flush()`. With HAL locks disabled this clears
local buffers without retaining a lock or emitting partial writes/empty probes.
Every exit restores a changed Wire timeout before releasing the gate. An actual
backend error remains primary even if a later time observation would be late.

The SDK can retain `contains_read` after failed receive. Every incomplete
`requestFrom` or invalid `read` therefore retires the **shared controller**.
All later callbacks, including other prepared addresses, fail I2C_BUS without
backend work. Core end/bind/forget and result take cannot clear retirement.
The application may end all bound drivers, tear down/reinitialize on the original
core, repeat preparation and requalify before creating fresh usable context.
Merely clearing `retired` is unsafe. This is an SDK restriction, not core health
policy or automatic recovery.

WireStats counts backend method calls, buffered TX bytes and returned RX bytes;
all saturate. These differ from core callback attempts and physical bus activity.
SDK cleanup can add invisible pulses. A read normally uses two backend calls.
`ownerNowMs` floors native esp_timer microseconds to 64-bit milliseconds, spanning
32-bit millis rollover with 1ms resolution. Sample it afresh after each callback.

## Electrical and verification boundary

TI SLLSEN9F section7.5, SLLSEP2D/SLLSEQ8D section7.4 and SLLSEO5D section6.5
describe explicit pointer selection with STOP. The adapter uses pointer STOP then
read STOP, not an inferred repeated START. Seven-bit addresses are base 0x60/61
and later 0x47/67; LAI 0x8E/8F are wire bytes. SCL must not exceed 400kHz. Verify
pull-ups, exact supply/pad limits, ADDR straps, enable polarity/readiness and board
CC/VBUS wiring against the [variant guide](variant-matrix.md).

Host tests compile the real adapter against recording SDK fakes. Separate real
SDK fixtures compile/link core, Wire/HAL and adapter:

```text
scripts/pio.cmd run -d test/arduino_sdk -e arduino_s2
scripts/pio.cmd run -d test/arduino_sdk -e arduino_s3
```

No flashing or physical role, clock, cleanup, CPU bound or electrical qualification
is claimed. The [board/CLI examples](arduino-examples.md) apply these prerequisites
and isolate USB transmission from owner progress.
