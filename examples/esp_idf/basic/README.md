# Native ESP-IDF CLI application

This complete native example links the portable core, synchronous native adapter
and the same 55-row CLI used by Arduino. ESP-IDF **5.5.5 and 6.1.0** and ESP32-S2/S3 are the
implemented SDK/target subset. No Arduino component, Wire, Serial or duplicate
command parser is used. Real linked images are software evidence; no board was
flashed or electrically qualified for this work.

## Build and console

Activate ESP-IDF 5.5.5 first and verify `idf.py --version`. In PowerShell,
from the library root:

```powershell
$s2Build = Join-Path (Get-Location) 'build/idf-5.5.5-s2'
$s3Build = Join-Path (Get-Location) 'build/idf-5.5.5-s3'
idf.py -C examples/esp_idf/basic -B "$s2Build" "-DSDKCONFIG=$s2Build/sdkconfig" -DIDF_TARGET=esp32s2 build
idf.py -C examples/esp_idf/basic -B "$s3Build" "-DSDKCONFIG=$s3Build/sdkconfig" -DIDF_TARGET=esp32s3 build
```

For ESP-IDF 6.1.0, activate that SDK in a separate shell, verify
`idf.py --version`, and use independent absolute paths from the library root:

```powershell
$s2Build = Join-Path (Get-Location) 'build/idf-6.1.0-s2'
$s3Build = Join-Path (Get-Location) 'build/idf-6.1.0-s3'
idf.py -C examples/esp_idf/basic -B "$s2Build" "-DSDKCONFIG=$s2Build/sdkconfig" -DIDF_TARGET=esp32s2 build
idf.py -C examples/esp_idf/basic -B "$s3Build" "-DSDKCONFIG=$s3Build/sdkconfig" -DIDF_TARGET=esp32s3 build
```

These commands build diagnostic-only images with the default readiness inputs.
They neither flash a board nor open a physical console. The exact supported
SDK gate is 5.5.5 or 6.1.0; an arbitrary newer version is not qualified.

Use the same absolute build and SDKCONFIG paths on later build/reconfigure/monitor
invocations. Separate `-B` directories alone do not normally separate a project's
default sdkconfig. This project additionally defaults SDKCONFIG and the component
manager dependency lock to each build directory. Target-specific defaults are
selected by IDF. The main component resolves the core's actual directory name, so
renaming an extracted library does not turn `REQUIRES` into an unavailable target.
The root IDF component continues to compile only the explicit portable core sources.

Both targets use **UART0, 115200 baud, 8N1, TX GPIO43 and RX GPIO44**. Connect a
3.3V USB/UART bridge with common ground, crossing TX/RX. These are explicit
example routes, not detected board wiring. Use the UART bridge connector or
external bridge wired to those pins. The S2/S3 native USB connector is not this
application's console. No USB readiness or connection wait runs in the owner.
Physical input/output and USB bridge operation remain NOT RUN.

The application owns the UART driver exclusively. SDK primary/secondary stdio
console and logging are disabled. Do not add another UART reader/writer, VFS
driver, printf logger or event consumer. RX ring storage is 256 bytes, TX has no
software ring, and the event queue has 16 entries. `uart_read_bytes(...,0)` reads
at most 64 bytes; retained suffixes survive the one-command turn limit.
`uart_tx_chars` offers at most 256 bytes and accepts available FIFO space only.
Its internal TX mutex is proved uncontended by this sole-owner arrangement, not
by a timeout setting. The owner and both SDK interrupts are on core0.

Overflow, framing/parity/break errors or a saturated event queue stop further
input permanently. Events are checked before and after the read; damaged bytes
cannot become a plausible command. Already admitted work and bounded output
continue. Correct the connection and explicitly restart the application; no
automatic UART/bus recovery or new command grammar is added. Wait for startup
reports, then send complete commands and consume each result before another job.

See the pinned [UART documentation](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/api-reference/peripherals/uart.html),
[native adapter contract](../../../docs/idf-transport.md) and
[CLI guide](../../../docs/CLI.md). Source inspection, rather than a zero timeout
alone, establishes the bounded console assumptions.

## Board choices and wiring

Defaults are diagnostic-only: UART works, but no I2C controller/device is created
until board readiness, a qualified positive CPU-overhead bound and acceptance of
SDK-internal cleanup are supplied. Native SDK cleanup may reset the controller
or clear the physical shared bus on errors. A number such as 2000 microseconds
in a build test is not measured qualification of scheduling, ISR or flash stalls.

Override the following CMake cache settings with `-DTUSB32X_...=...` on the same
build/configuration paths. BoardConfig validates values before narrowing.

| Setting | Default / meaning |
| --- | --- |
| TUSB32X_VARIANT, TUSB32X_ADDRESS | TUSB320LAI, 0x47; exact part and seven-bit ADDR strap |
| TUSB32X_SDA, TUSB32X_SCL | GPIO4, GPIO5; qualified external pull-ups required |
| TUSB32X_IRQ | -1 absent; optional externally pulled-up input only, no implicit ACK/service |
| TUSB32X_ENABLE | -1 unspecified; external enable is reported, never driven or configured |
| TUSB32X_UART_TX, TUSB32X_UART_RX | GPIO43, GPIO44; must not conflict with I2C/input/enable wiring |
| TUSB32X_BUS_HZ | 100000; native adapter accepts 100000..400000Hz |
| TUSB32X_CALLBACK_MS | 600; whole callback cap including both pointer/read phases and cleanup |
| TUSB32X_OPERATION_MS | 10000; immutable interval for each application startup operation |
| TUSB32X_BOARD_READY | 0; explicit supply/strap/enable/wiring readiness opt-in |
| TUSB32X_MAX_OVERHEAD_US | 0; qualified whole-callback CPU/preemption bound, microseconds |
| TUSB32X_ACCEPT_SDK_CLEANUP | 0; explicit acceptance by every physical bus user |
| TUSB32X_RUN_MS | 0; positive milliseconds request finite-runtime shutdown |

The adapter's 10ms per-SDK-wait cap is separate from the aggregate callback cap.
The CLI's operation default remains 1000ms; use an explicit longer timeout when
the conservative adapter bounds require it. This does not extend an admitted
deadline or prove a hardware timing bound.

| Connection | Exact-part prerequisites |
| --- | --- |
| Ground/VDD | Common ground. LAI/HAI operates at 2.7-5.0V; 322I at 4.5-5.5V. LAI local 3A advertisement needs VDD at least 3.5V; detecting a remote 3A source does not. Never infer a power-path authorization from CC indication. |
| SDA/SCL | Qualified 1.65-3.6V I2C pull-up rail, never 5V at ESP32 pins. A 3.3V I2C rail requires chip VDD greater than 3.0V. Choose pull-ups from capacitance/rise-time constraints. |
| ADDR | Low/high selects original 320/I 0x60/0x61 or LI/HI/LAI/HAI/322I 0x47/0x67. High references chip VDD. Floating selects GPIO mode and disables I2C; wire address bytes such as 0x8E/0x8F are not seven-bit API addresses. |
| Enable | Original 320/I, LI, LAI and 322I use active-low EN_N; HI/HAI use active-high EN. The application reports this from the exact variant. The industrial I suffix does not invert base polarity. Board readiness includes stable VDD and the exact startup timing/polarity; the L/A datasheet/application-note EN_N wording conflict remains unresolved. |
| PORT/DIR | 320 pin3 PORT is sampled at startup/reset; pin4 is VBUS_DET. 322 pin3 is VBUS_DET and pin4 is DIR, with no PORT pin. It is not a pin-for-pin replacement. |
| VBUS_DET | Use the exact TI resistor-coupled sensing network: 855/887/920kohm min/typ/max versus common 900kohm prose. A 28V system-rail limit is not the pad rating; do not wire that rail directly to the pad. The 320 DFP-only omission exception does not apply to 322I, which requires its resistor connection. |
| 322I VCONN | Automatic on the unused CC line after source attachment with Ra, removed on detach. No qualified software VCONN-enable API exists. The board supplies 5V VDD after Attached.SRC; the VCONN range is 4.75-5.5V with VDD=5V. Current limit is 225/300/375mA min/typ/max, FET resistance at most 1.25ohm, bulk capacitance 10-200uF. The typical 300mA is not guaranteed output capability; generic 4.5-5.5V device operation does not establish the VCONN condition. |

HI/HAI EN high is limited to 1.05..3.65 V, with a 4 V absolute maximum;
5 V VDD does not permit a direct EN-to-VDD strap. Optional INT_N is active-low
open-drain and needs a qualified external pull-up to an MCU-safe rail. An ISR
only signals the owner. Respect supply-off and non-failsafe pin limits to avoid
back-powering through ADDR, PORT/DIR, VBUS_DET or other external connections.
The [hardware guide](../../../docs/hardware-validation.md) retains exact-part
startup conflicts and sensing limits, including the 4 V VBUS_DET pad maximum.

These values were rechecked against archived TI SLLSEQ8D recommended operating/
electrical tables and SLLSEO5D sections 5.3, 6.3.6 and 6.3.8. The
[variant guide](../../../docs/variant-matrix.md) retains capability/access limits
and unresolved advertisement/fault-clear wording. Board readiness is not inferred
from an ACK, matching signature, successful host fixture or compiler result.

## Lifecycle, identity and shutdown

`app_main` uses IDF's explicitly core0-pinned main task, with an 8192-byte stack.
Static application state includes the CLI queue and survives failed cleanup.
The application creates UART, then an exclusively owned controller0 and a
synchronous seven-bit device. Copied SDK configuration must match those opaque
handles. Existing UART/controller ownership is rejected; borrowed resources are
never deleted. Handle/address replacement requires explicit application code
while ended/idle. This example restricts CLI edits/bind to the prepared target.

After readiness and binding, the application startup owner initializes read-only,
takes the exact terminal result, then refreshes port/status and takes that result.
The CLI tick drains output during startup but never polls that owner's token.
Input starts after completion/failure, so startup cannot leave an unowned pending
result blocking diagnostics. Each turn uses one existing poll path, never two.
Initialization does not apply a profile, reset, ACK or recover automatically.
Original 320/I recovery is a separate explicit typed operation after identity;
the owner establishes stable VDD and source-startup need. Later parts reject it.

`version` emits the same library/FIRMWARE/FRAMEWORK and schema-1 protocol records
as Arduino. An example-only CMake target regenerates Git/dirty/UTC/target identity
on every build, including an incremental rebuild after source edits. Archives
outside their own Git root report unknown; an unrelated parent repository is
never borrowed. Saturating primary-target callback-kind counters count write
and write-read invocations, including rejected and failed callbacks. A pointer
phase within write-read is not a data-write callback; SDK transmit/receive totals
remain separate. The Python readonly/soak plans use these counts to prove no
data-write callbacks, subject to board and session prerequisites. Core all-callback
and managed statistics remain available through existing commands under
the [single CLI reference](../../../docs/CLI.md).

Application `requestStop()` or TUSB32X_RUN_MS stops command admission, cancels and
consumes only startup-owned work, ends the binding, and snapshots/reports retained
CLI results and hardware effects. It removes its device before its bus, resets
only its optional configured input, then drains UART output. Failed deletion
retains static handles/ownership and is never retried. A no-wait FIFO-completion
check prevents normal shutdown from truncating accepted UART bytes.

Each output-drain phase is capped at 64 owner turns; failed console deletion
allows one additional phase for its failure record. A stalled terminal cannot
hold the task forever. Forced output abandonment is retained in static diagnostic
state; delivery is not promised when the receiver is absent. No callback occurs
after the binding ends, and the SDK deletes the main task when app_main returns.
Host fakes verify these policies; stack margin, native serial operation, physical
IRQ/CC/VBUS and aggregate timing still require an identified authorized fixture.
