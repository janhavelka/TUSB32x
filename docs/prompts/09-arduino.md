# P09 — Add Arduino ESP32 examples and transport adapter

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 S2/S3
USB console configuration, INA228/ADS1115 bus adapters, LSM6DS3TR owner-soak example
and TCA9548A transport tests. Inspect official docs for the selected pinned SDK.

Use [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md) for core symbols and
[CLI_CONTRACT.md](CLI_CONTRACT.md) for commands. Arduino-specific work belongs in
the adapter/example files; the shared processor and core retain the same API.

## Work

1. Add `examples/01_basic_bringup_cli/main.cpp` with the shared processor and a
   thin explicit Arduino adapter. BoardConfig holds overridable SDA/SCL/IRQ/enable
   pin assumptions, exact model/address, frequency and finite timeouts. Document
   which pins are unconfigured until supplied; never guess a connected power pin.
2. The example application initializes/owns Wire, console, clocks and GPIO; core
   borrows callbacks/context. Preserve exact transfer framing from P01/P04.
   Check Wire buffer/write acceptance, short read and terminal error codes; do
   not claim a NACK phase/effect the backend cannot distinguish. Timeout units,
   lock wait and completion semantics must meet the core contract.
3. Add a concise `examples/02_owner_service/` demonstrating caller-owned polling,
   shared-bus fairness and optional IRQ flag -> owner service. No ISR I2C. Use
   bounded processing; expose intended policy hooks without implementing VBUS,
   charger/USB-stack role switching or reinitializing an application's bus.
4. Support ESP32-S2 and ESP32-S3 environments with the audited, pinned platform.
   Configure USB mode/CDC/console for each target deliberately. Review DTR/RTS
   differences rather than copying S3 defaults to S2. No infinite `while(!Serial)`.
5. Start with passive bind and safe read-only identification/status. Settings
   application is explicit. EN and supply readiness are board prerequisites;
   documentation must include TI's startup wording conflict and the chosen board
   sequence. Do not fabricate a universal GPIO enable pulse.
   For original TUSB320/I source-capable startup after VDD returns, document the
   separate explicit recover-dead-battery operation after identity initialization.
   Use the existing engine and its 30 ms wait; do not duplicate register writes
   in Arduino setup or run the workaround on LAI/HAI or other later variants.
6. Use bounded input/output buffering and finite loop work; report dropped output
   and keep owner service responsive when USB disconnects or host stops reading.
   Firmware prints actual library/framework/target/build/git/dirty identity.

## Concrete adapter and loop responsibilities

- A non-owning adapter context holds the application's `TwoWire*`, selected address
  if required by the backend, finite timeout settings and adapter counters. It
  never deletes the bus or calls `Wire.begin()` from a driver callback.
- `setup()` configures the example's own bus/console, verifies board prerequisites,
  constructs DriverConfig and calls `bind`. Read-only initialization is queued
  explicitly; no apply/reset/IRQ acknowledgement is hidden in startup.
- `loop()` processes bounded input, advances at most the configured callback
  budget, copies completed facts, and flushes bounded output. A retained terminal
  result is consumed once by the command/operation owner. It is not overwritten
  by a background watch or a new user command.
- The callback validates arguments, sets/clips the backend timeout, checks every
  write/read length, and returns a terminal Status with original backend detail.
  A combined pointer/read uses one aggregate timeout and preserves serialization
  across its phases. Do not grant a full new timeout to the second phase.
- Error mapping explicitly distinguishes a backend-confirmed address NACK from
  generic bus failure; TIMEOUT is the operation layer, I2C_TIMEOUT the callback
  layer. Do not claim no hardware effect merely because the return code failed.
- Where Arduino supplies only a wrapping millisecond clock, extend it correctly
  in the application or use a native monotonic 64-bit source available to that
  platform. Do not repeatedly cast raw `millis()` to uint64_t and call it monotonic.
- The optional IRQ handler only records a flag/notification using the platform's
  synchronization rules. The owner reads that flag and admits `startServiceInterrupt`.
  No core calls, Serial printing or register reads in the ISR.
- Stop accepting new commands before adapter/driver teardown. Finish or cancel
  locally, retain/report effects and result, release the binding, then destroy
  example-owned resources. Never end another application's shared Wire instance.

Use compile-time board overrides for model/address/pins. SDA/SCL defaults must
be documented as example board choices. An absent optional IRQ/enable pin is
represented explicitly, not guessed from another sensor example. CLI displays
actual runtime wiring/timeout/framing; header defaults are not physical evidence.

Establish named S2/S3 environments for CLI and owner_service, with explicit source
filters so each image has one setup/loop and includes needed common sources only.
`scripts/pio.cmd run -e esp32s2dev` and `-e esp32s3dev` remain the primary bring-up
commands; name additional owner-example environments consistently in platformio.ini.

The inspected TMP1x2 platformio.ini provides these concrete starting points:

| Environment | Board | USB flags | Monitor control lines |
| --- | --- | --- | --- |
| `esp32s3dev` | `esp32-s3-devkitc-1` | `ARDUINO_USB_MODE=1`, `ARDUINO_USB_CDC_ON_BOOT=1` | RTS 0; DTR 0 in this example |
| `esp32s2dev` | `esp32-s2-saola-1` | `ARDUINO_USB_MODE=0`, `ARDUINO_USB_CDC_ON_BOOT=1` | RTS 0; DTR 1 for its TinyUSB CDC transmit requirement |

Both use 115200 monitor speed. These are audited example settings, not universal
USB rules or proof of board wiring. Recheck the pinned SDK/backend before adopting
them, include a clear deliberate difference if changed, and document that opening
a serial port can toggle control lines. Do not copy the bus pins from TMP1x2.

## Source and example documentation

Add Doxygen `@file`, context lifetime, callback `@param/@return`, total timeout,
framing and effect comments in adapter headers/source. Explain USB/control-line
choices and the owner-loop bounds where implemented. Update examples/README,
the example's wiring/build README, docs/integration, CLI diagnostics docs and
CHANGELOG. Record S2 and S3 builds separately in validation. An SDK or pin change
must update the comments and guide in the same edit.

## Tests and completion

- Build both S2/S3 environments using `scripts/pio.cmd`; compile every example
  via a documented environment/filter. Test adapter behavior with meaningful
  fake/backend contract cases where possible; verify core remains framework-free.
- Run shared CLI/console tests after wiring the adapter. Record flash/serial/HIL
  separately and only run against an identified authorized board/port.
- Document wiring, pullup supply, address/PORT straps, enable polarity, maximum
  I2C speed, VBUS_DET requirements and supply/current constraints per exact variant.
- An Arduino build does not establish native IDF compatibility or physical USB
  role/current/interrupt behavior. Update evidence with actual results only.
