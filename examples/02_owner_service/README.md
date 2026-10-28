# Two-device Arduino owner service

Build `scripts/pio.cmd run -e esp32s2_owner_service` or
`scripts/pio.cmd run -e esp32s3_owner_service`. This example adds a second
physical device to the same externally serialized Wire controller and retains
the same primary CLI. Targets must have distinct strapped addresses.

The default intended pair is LAI at 0x47 and 0x67. Use ADDR low and high
respectively; a floating ADDR disables I2C. Check
[BoardConfig](../arduino/BoardConfig.h) for compile-time overrides. The CLI cannot
retarget to the secondary's reserved address or an unprepared handle.

Follow the [complete wiring and USB guide](../../docs/arduino-examples.md),
including 1.65–3.6V I2C pull-ups, at most 400kHz, exact PORT versus DIR wiring,
enable polarity and VBUS_DET resistor/pad limits. LAI/HAI VDD is 2.7–5.0V;
322I requires 4.5–5.5V and its separate autonomous VCONN supply/fixture rules.
LAI local 3A advertisement requires at least 3.5V, unlike remote 3A detection.
No missing power/enable pin is configured or pulsed. Establish a qualified
startup sequence before explicitly enabling bus work.

The fixed owner alternates targets. Secondary startup initializes and refreshes
read-only, then schedules bounded observations. An optional IRQ input only sets
a synchronized coalesced flag in the ISR; the owner later requests one bounded
service operation. There is no I2C, logging or core call in the ISR. IRQ service
explicitly writes W1C; periodic status alone never ACKs. Stuck IRQ does not cause
an unbounded ACK loop. Automatic scheduling stops on failure without retry.

Each application-owned token is consumed exactly; primary CLI results require
explicit command take. Shutdown stops new work, cancels/retains effects, ends
both bindings and only then releases this application's exclusive Wire bus.
The static USB worker remains alive, so a stalled host cannot delay bus teardown.
See the shared [implementation](../arduino/ExampleApp.cpp) and
[portable owner helper](../common/ExampleOwner.h).

Build, flash, serial and physical IRQ/CC/VBUS evidence are separate. These
environments do not assert that either device is connected or qualified.
