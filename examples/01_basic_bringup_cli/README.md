# Interactive Arduino bring-up

Build `scripts/pio.cmd run -e esp32s2dev` or `-e esp32s3dev` from the repository
root. The application uses one shared CLI processor, the tested Wire adapter and
application-owned read-only initialization/status startup. It does not apply
settings, reset, ACK IRQ or run recovery automatically.

Start with the [board, wiring and USB guide](../../docs/arduino-examples.md).
It specifies the exact pull-up supply, seven-bit ADDR and PORT straps, enable
polarity, VBUS_DET network, I2C speed, VDD/current and TUSB322I VCONN constraints.
LAI/HAI VDD is 2.7–5.0V; 322I is 4.5–5.5V. Local LAI 3A advertisement requires
at least 3.5V; remote 3A detection as a sink is a different fact.

Review [BoardConfig](../arduino/BoardConfig.h) overrides before using a fixture.
Default builds keep the diagnostic console available without claiming board or
timing qualification. Missing optional pins remain unconfigured. The board must
establish readiness; unresolved TI startup wording is not replaced by an
invented enable pulse. Use the native USB connector, 115200, with S2 DTR1/RTS0
or S3 DTR0/RTS0. Output congestion never waits in the owner loop.

After startup, `version`, `bus`, `sample`, `cfg` and `diag` inspect evidence.
`read` requests a fresh port observation. Inspect the matching `result`, then
explicitly `result take <token>` before starting another job. Commands that write
the device remain explicit; read the [CLI reference](../../docs/CLI.md).

The shared [application implementation](../arduino/ExampleApp.cpp) owns setup,
finite input/output and orderly bus shutdown. Only one main.cpp is selected by
each environment. A successful firmware build is not flash, serial or HIL evidence.
