# P09 — Add Arduino ESP32 examples and transport adapter

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 S2/S3
USB console configuration, INA228/ADS1115 bus adapters, LSM6DS3TR owner-soak example
and TCA9548A transport tests. Inspect official docs for the selected pinned SDK.

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
6. Use bounded input/output buffering and finite loop work; report dropped output
   and keep owner service responsive when USB disconnects or host stops reading.
   Firmware prints actual library/framework/target/build/git/dirty identity.

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
