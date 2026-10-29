# Arduino CLI and owner examples

The two applications use the same portable CLI, owner engine and
[qualified Wire adapter](arduino-transport.md). Native USB carries diagnostics;
it does not control the TUSB chip's CC, VBUS, charger or mux. No startup path
applies a profile, resets the chip, acknowledges IRQ or performs dead-battery
recovery.

## Build and connect

Use the installed VS Code-managed PlatformIO Core through the repository wrapper:

```powershell
scripts/pio.cmd run -e esp32s2dev
scripts/pio.cmd run -e esp32s3dev
scripts/pio.cmd run -e esp32s2_owner_service
scripts/pio.cmd run -e esp32s3_owner_service
```

Each environment explicitly selects one main.cpp. The primary environments run
the interactive CLI; owner-service environments add a second device on the same
application-owned controller. The platform is pinned to pioarduino 55.03.311,
Arduino 3.3.11 and IDF 5.5.5. These are linked firmware builds, not board tests.
Use the native USB connector on the identified board, not an unrelated UART bridge.

| Target | Board | USB mode / CDC on boot | Monitor at 115200 |
| --- | --- | --- | --- |
| S2 | esp32-s2-saola-1 | 0 / 1, TinyUSB CDC | RTS 0, DTR 1 |
| S3 | esp32-s3-devkitc-1 | 1 / 1, hardware CDC | RTS 0, DTR 0 |

Opening a serial port may toggle RTS/DTR before the configured values settle.
S2 explicitly disables CDC's optional reboot-on-line-change policy. Its boolean
Serial readiness also checks RTS, although transmit readiness needs DTR; the
worker therefore uses the transmit API rather than that boolean gate on S2.
S3 suppresses writes while disconnected. No example waits in `while (!Serial)`.
SDK acceptance cannot prove receipt by a host, particularly across disconnects.

The pinned SDK creates a 256-byte USB receive queue. The application is its sole
reader and must not resize it. Queue saturation can drop bytes before the shared
parser sees them; the application therefore disables further command input on
suspected loss, reports `APP console=input_failed`, and continues existing owner
work and output. This conservative fence can also reject a full queue that has
not yet lost bytes. A new application session is required to resume input; do not
continue a partial command across reconnect. This does not qualify physical USB
delivery or establish a reliable disconnect-event channel.

The front end reports saturating write and write-read callback counts for the
primary CLI target. Setup prewarming and secondary-device traffic are excluded;
pointer selection within a read is not counted as a data-write callback.
The Python runner uses these counts to prove no data-write callbacks during
`readonly` and `soak`, with qualified board/session prerequisites; see the
[runner guide](hil-runner.md#readonly-soak-and-register-observation).

## Board qualification before bus work

The default build provides a diagnostic console. Hardware startup is enabled
only when the compile-time board-readiness and SDK-cleanup choices are supplied
and the whole-callback CPU/scheduling overhead has a positive, qualified bound.
These inputs are documented beside `BoardConfig` in the Arduino example source.
Do not enter a guessed overhead merely to pass validation. Software tests and
SDK builds cannot establish that bound for an unknown fixture.

The application initializes Wire on its pinned owner core, prewarms only the
configured addresses with a checked read-only identity pointer/read, and then
binds the core. This deliberate setup transfer creates the SDK device handle;
it is outside the adapter's bounded steady-state callbacks and is not counted
as core initialization. No bus scan or retry occurs. Failed preparation leaves
the console available and tears down the exclusively owned controller.
The transport guide explains finite phase reserves, controller retirement and
why the 400ms example callback budget is larger than the core default.

Confirm all of the following against the exact part and board before enabling
hardware startup. These are external prerequisites, not measurements performed
by firmware. The [variant guide](variant-matrix.md) retains exact source pages
and unresolved evidence.

Override the following with `-DNAME=value` in an environment's build flags,
retaining its inherited SDK/USB flags. Values are firmware configuration, not
live detection. `TUSB32X_VARIANT` / `TUSB32X_SECOND_VARIANT` accept exact enum
names such as `TUSB320I` or `TUSB322I`, without a namespace prefix.

| Macro(s) | Default / meaning |
| --- | --- |
| TUSB32X_VARIANT, TUSB32X_ADDRESS | TUSB320LAI, 0x47 |
| TUSB32X_SECOND_VARIANT, TUSB32X_SECOND_ADDRESS | TUSB320LAI, 0x67; owner-service only |
| TUSB32X_SDA, TUSB32X_SCL | GPIO4, GPIO5 on both example boards; verify your board routing |
| TUSB32X_IRQ, TUSB32X_SECOND_IRQ | -1 absent; explicit externally pulled-up inputs |
| TUSB32X_ENABLE | -1; optional report-only board-controlled signal, never driven here; must not share an IRQ input |
| TUSB32X_BUS_HZ | 100000; audited SDK range 100000..400000 |
| TUSB32X_CALLBACK_MS, TUSB32X_OPERATION_MS | 400, 5000; finite aggregate callback / owner operation budgets |
| TUSB32X_BOARD_READY | 0; set 1 only after exact-part supply/strap/enable/wiring qualification |
| TUSB32X_MAX_OVERHEAD_US | 0; replace only with the qualified whole-callback CPU/scheduling bound |
| TUSB32X_ACCEPT_SDK_CLEANUP | 0; set 1 only when SDK controller reset/bus clear is acceptable to all users |
| TUSB32X_RUN_MS | 0, continuous operation; positive finite runtime requests orderly shutdown |

The 10ms Wire wait cap remains in the shared adapter Context; it is not the
aggregate callback deadline. A board can also call `requestStop()` from normal
application code. No extra CLI shutdown parser or automatic bus restart is added.

| Connection | Required wiring and limits |
| --- | --- |
| Ground and VDD | Common ground. All six 320 variants operate at 2.7–5.0V; TUSB322I at 4.5–5.5V. LAI/HAI at 3.3V can detect a remote 3A source, but local 3A advertisement requires at least 3.5V. Advertisement never authorizes the board's power path. |
| SDA/SCL | Explicit example GPIOs, not board autodetection. Pull up to a qualified 1.65–3.6V I2C rail; 3.3V I2C requires chip VDD greater than 3.0V throughout use. Do not pull ESP32 pins to 5V. Select resistor values from bus capacitance/rise-time requirements; the tri-level strap's typical 4.7k value is not a universal bus pull-up rule. SCL is at most 400kHz. |
| ADDR | Low/high selects base 0x60/0x61 or extended/322I 0x47/0x67, all seven-bit addresses. Floating selects GPIO mode and disables I2C. High strap references chip VDD, not necessarily the MCU rail. LAI 0x8E/0x8F are wire bytes, never API addresses. |
| PORT / DIR | The six 320 parts have PORT on pin3: low UFP, floating DRP, high DFP, sampled at enable and software reset. TUSB322I instead has VBUS_DET on pin3 and open-drain DIR on pin4; it has no PORT. Parts are not board-interchangeable merely because packages match. |
| Enable | 320/I/LI/LAI/322I enable low; HI/HAI enable high. HI/HAI high range is 1.05–3.65V, not a direct 5V strap. Unspecified optional pins remain unconfigured. Enable/power sequencing remains board-owned; no universal pulse is generated. |
| Startup | Board must already have valid VDD, straps and active enable, and allow the documented enable-to-I2C maximum of 100ms. This does not prove attachment. Exact LI/LAI EN_N text and SLAAE24 disagree about a 50ms startup level; use a qualified board sequence or obtain TI clarification. The example does not resolve the conflict by choosing a pulse. |
| VBUS_DET | Use the exact resistor network: external 855/887/920kΩ min/typ/max (900kΩ prose), with internal nominal 95kΩ. System VBUS up to 28V is not a 28V pad rating; VBUS_DET pad maximum is 4V. The six 320 DFP-only omission exception requires exclusive DFP use forever; it does not apply to this role-changing CLI or to 322I. |
| INT_N | Optional active-low open-drain input with a qualified pull-up and MCU-safe voltage. No implicit internal pull-up or lossless edge counter. The owner-service ISR only signals a coalesced flag; later owner work performs explicit bounded service. |
| Supply-off pins | Respect the exact variant's non-failsafe pins and sequencing. Do not feed unpowered ADDR/PORT/DIR/VBUS_DET or assume unrestricted ID supply-off tolerance. The variant guide preserves the ID rating conflict. |
| TUSB322I VCONN | Autonomous device output, not switched by the core. The source fixture needs the specified 5V VDD/VCONN conditions and 10–200µF bulk capacitance; device operating range alone does not qualify VCONN. No 320 active-cable indication proves powered VCONN. |

For original TUSB320/I, source-capable startup after absent VDD may need the
documented recovery even when no cable was present. After successful identity
initialization and owner-established stable supply, explicitly issue
`recover-dead-battery`; consume its result as for other commands. Later variants
reject that procedure. Setup never duplicates its register writes.

## Ownership, console and shutdown

Setup rejects an already initialized Wire0 controller before calling `begin`:
the SDK otherwise returns success without validating its pins, speed or creating
core. The example never acquires ownership of another application's bus.
SDA/SCL and configured IRQ pins cannot use native USB GPIO19/20; assigning them
would detach the diagnostic console through the SDK peripheral manager.
The optional enable pin remains report-only and externally controlled.

Teardown checks `Wire.end()`. Failure retains ownership, retires the adapter,
reports `bus=release_failed`, and leaves the diagnostic console alive. It does
not retry deletion or claim resources were released. Setup cleanup follows the
same rule. Unowned buses are reported as `bus=not_owned` and left untouched.

Application startup owns read-only initialization followed by port refresh. It
polls through the existing engine, takes each exact terminal result, and queues
an `APP` diagnostic. Output congestion cannot retain startup's result slot.
Failure stops startup without retry and releases the diagnostic console.
After startup, the CLI owns its admitted jobs and never automatically takes
their results. Use `result`, inspect status/effects, then `result take <token>`.
See the [CLI guide](CLI.md) for command grammar and mutation prerequisites.

The owner-service application alternates the two physical targets, with at most
one callback on a selected turn. Both borrow the same controller Context, gate
and retirement flag. The secondary has periodic observations and an optional
explicit deferred IRQ service request; failures stop its automatic scheduling.
The primary CLI cannot retarget to the reserved secondary address. Neither
target can silently replace another configuration owner.

All diagnostic records use one bounded ConsoleBuffer. The owner hands at most
256 bytes to a fixed copied mailbox; a separate static USB worker retains
partial SDK writes. S2 TinyUSB can block on an internal mutex despite a zero TX
timeout, so no SDK transmit call runs in the I2C owner. Input is independently
bounded to 64 bytes and one CLI command per primary turn. Whole-record overflow
is reported by the shared console's LOSS records/counters. A disconnected host
may lose SDK-buffered bytes; the firmware does not claim end-to-end delivery.

Shutdown first stops admissions and IRQ signaling. It cancels/consumes only
application-owned work, preserves externally owned CLI terminal results and
reports available outcomes/effects. Both bindings end before the application
releases its exclusively created Wire bus. An integration borrowing another
application's bus must not call Wire.end. USB resources remain statically alive
after bus shutdown; they are not deleted underneath a blocked SDK write.

`version` reports library, framework, target, build, Git and dirty provenance;
`bus` reports configured address, GPIOs, framing and timeout. Application startup
records add board/readiness/setup evidence. Defaults describe firmware settings,
not a connected or qualified fixture. Exported builds without repository metadata
report unknown Git rather than inheriting a parent repository's identity.

## Evidence boundary

Host tests exercise the same startup owner, shared CLI, bounded console and
mailbox, including a deliberately blocked worker. Four SDK images prove real
framework linkage and source selection. Flash, serial interaction, interrupt
levels, CC/VBUS behavior and timing qualification require a separately identified,
authorized board. None is inferred from those software checks.
