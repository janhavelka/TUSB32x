# Prompt 26 peer and console audit

Inspected on 2026-10-28 before native application implementation. TUSB32x was
clean at `4445cdc9a67aceed1347b8685aa2a2a347a79743`. Read parent/local AGENTS,
COMMON, AGENT_WORKFLOW, prompt 26, CLI_CONTRACT, the native transport guide and
the actual adapter, Arduino application, common ExampleOwner and identity code.
Sibling repositories were read-only; no sibling builds, Git mutations, remote CI
queries or hardware operations were performed. Paths below are repository-relative.

| Peer | Current revision and dirty state | Sources re-read |
| --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9`, clean | `AGENTS.md`; `examples/esp_idf/basic/{CMakeLists.txt,sdkconfig.defaults,sdkconfig.defaults.esp32s2,sdkconfig.defaults.esp32s3,main/CMakeLists.txt,main/main.cpp}`; `test/cli/test_cli.cpp`; `tools/{test_idf_component_contract.py,hil_tmp1x2_runner.py}`; `.github/workflows/ci.yml` |
| SHT3x-main | `f65f15573e3c9b1ef04cce72ad08f307d021e563`, clean | `AGENTS.md`; `examples/idf/basic/main/main.cpp`; `examples/common/Sht3xCli.cpp`; `tools/{sht3x_cli_contract.py,check_idf_example_contract.py}`; `.github/workflows/ci.yml` |
| Installed native ESP-IDF | `tools/cmake/version.cmake`: 5.5.5, package `C:/pio/packages/framework-espidf` | `examples/system/console/basic/{main/console_example_main.c,pytest_console_basic.py}`; `components/esp_driver_uart/src/uart.c`; `components/hal/{uart_hal_iram.c,esp32s2/include/hal/uart_ll.h,esp32s3/include/hal/uart_ll.h}`; `components/esp_driver_usb_serial_jtag/src/usb_serial_jtag.c`; `components/esp_system/port/usb_console.c` |

The SDK package has no AGENTS.md and is an installed package, not a sibling
checkout with independently established Git cleanliness. No TUSB32x workflow
directory exists at this stage. Peer workflow source review found TMP1x2 native
S2/S3 matrices on IDF 5.3.2/5.5.1/6.0.1 and SHT3x on IDF 5.4.2. These do not
establish coverage for TUSB32x's pinned IDF 5.5.5 adapter.

## Adopted and rejected peer choices

Adopt TMP1x2's application-owned native handles, explicit main-component source
list for shared CLI, checkout-directory-derived component name, and example-only
identity refreshed on every build. Its contract fixture checks renamed archives,
unrelated enclosing Git repositories and clean/dirty/new-commit changes without
reconfiguration. Those are useful metadata checks, separate from actual SDK links.
Its recording CLI test executes the processor; its runner checks response evidence.
Neither peer runner nor source-text contract is a physical console test.

Adopt SHT3x's separation between shared processor and native application glue.
Reject its direct stdio output, extra input-parser implementation, startup scan,
unbounded draining of the command queue, compatibility APIs and ignored resource
deletion results. TUSB32x already has bounded feed/output and exact-token startup
ownership in ExampleOwner. Reuse them without a second grammar or scheduler.

TMP1x2 selects ROM USB CDC on S2 and USB Serial/JTAG on S3, with a blocking input
worker isolated from I2C. TUSB32x selects application-owned UART0 for both input
and output on both chips: TX43/RX44, 115200 8N1 and no hardware flow control.
`CONFIG_ESP_CONSOLE_NONE=y`, no secondary console and maximum logging level zero
exclude SDK stdio/driver writers from the application UART. This also avoids a
USB wait in the owner: any USB-UART bridge is external wiring, not an SDK USB
console. Native USB input/output is not claimed by these builds.

The SDK console basic example uses esp_console REPL registration, optional
filesystem history and a separate console task. Its own source warns that a
secondary console is output-only. Retain explicit bidirectional route selection;
reject introducing its REPL/history/command registry because the fixed shared CLI
already owns parsing. The included pytest checks startup history messages on
ESP32/C3, not S2/S3 disconnection or bounded owner work.

## Exact console source bounds

The official [IDF 5.5.5 UART guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/api-reference/peripherals/uart.html)
describes `uart_tx_chars` as filling available FIFO capacity. The actual source
adds a required exclusivity condition: `uart.c:1548-1560` takes `tx_mux` with
`portMAX_DELAY` before the FIFO copy. It is bounded here only because one pinned
application task is the sole caller of UART driver transmit/flush operations,
with TX ring size zero and no VFS driver route, SDK console or logging writer.
The mutex is uncontended under that application contract; the API alone does not
promise a finite lock acquisition. Do not replace it with `uart_write_bytes`,
whose implementation waits for buffer/FIFO space.

`uart_enable_tx_write_fifo` at line 1131 enters one critical section and calls
`uart_hal_write_txfifo` once. `uart_hal_iram.c` takes the minimum of requested bytes
and free FIFO space. S2/S3 `uart_ll_write_txfifo` then uses a finite byte-copy loop;
there is no wait for a host to consume bytes. The owner supplies a bounded drain
chunk and retains unsent queue bytes. UART without CTS does not depend on an
attached terminal for transmit progress; successful FIFO acceptance is not proof
that the user received the record.

`uart_read_bytes` at line 1665 takes `rx_mux` and ring-buffer receives using the
supplied tick timeout. Supplying zero prevents RTOS waits. Its loop decreases the
bounded requested length after each copy. The source explicitly states that its
buffer-full retry branch cannot succeed when reader and UART ISR run on the same
core. Create the UART on the pinned owner core and keep reading/teardown there.
Preserve bytes already read into application staging when shared `feed` declines
a byte after its per-turn command/byte budget; do not drop a suffix of a command.
UART hardware/ring overflow is distinct from shared parser overflow. The native
application permanently disables input on FIFO/ring overflow, break, parity or
framing errors, and conservatively treats a saturated event queue or full event
drain budget as possible lost error evidence. IDF `uart.c:1470` otherwise merely
logs failed event enqueues. Check events both before and after bounded reads;
retain output and already-admitted operation progress after input failure.
Host request pacing alone is not proof of lossless arbitrary input.

Shutdown must distinguish an empty application queue from a drained hardware
FIFO: `uart_driver_delete` at line 2053 releases pins and disables the peripheral
without waiting for pending bytes. `uart_wait_tx_done(UART_NUM_0, 0)` permits a
bounded shutdown check; its semaphore waits are zero and its source includes a
fixed 2us FSM update guard. Keep this check inside a finite shutdown-turn cap.

The rejected S3 USB alternative is locally auditable:
`usb_serial_jtag_read_bytes` at line 242 makes one ring-buffer receive;
`usb_serial_jtag_write_bytes` at line 265 uses the caller's timeout for its TX
mutex and ring send. Zero makes both nonwaiting even with an absent host.
S2 ROM CDC uses private `esp_usb_console_*` APIs, software buffering and ROM FIFO
calls. TMP1x2 checks endpoint availability before write/flush; that peer pattern
alone is not a proof of every ROM blocking path and was not needed for this UART
application. Web retrieval of the USB documentation URLs failed; no USB behavior
claim is based on those unavailable pages.

Local SHA-256 provenance for inspected SDK implementations:

| Relative SDK path | SHA-256 |
| --- | --- |
| `components/esp_driver_uart/src/uart.c` | `660092e007c6f9c691e1597baf7012070926e407b1b93daf60398bcb152a88a8` |
| `components/esp_driver_usb_serial_jtag/src/usb_serial_jtag.c` | `fa671482d75c98f8ef694b816e1b5b8719f49d98696aecbcdafae93a859071da` |
| `components/esp_system/port/usb_console.c` | `4c2620a557599f5fc81fed1e6fe3744f44c896cf031a30ba87a12fa92ca21ddd` |

The official [IDF 5.5.5 I2C guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/api-reference/peripherals/i2c.html)
confirms native bus/device ownership and removing devices before deleting a bus.
Aggregate callback timing, synchronous mode, retirement after failed receives,
same-core creation/use/deletion and qualified overhead remain the stricter
existing [native transport contract](../idf-transport.md). A timeout argument or
a successful link does not replace those source and physical timing conditions.

## Review boundaries

Startup must initialize, inspect/take the exact result, then admit read-only port
refresh; output backpressure must not own result delivery. During startup only
ExampleOwner polls; CLI ticks drain only. After handoff only CLI advances the
driver. Stop command admissions before cancelling/ending, retain CLI-owned result
and effect evidence, then remove only application-owned SDK resources. A failed
deletion keeps the corresponding ownership/storage alive and prevents reuse.
Identity generation belongs to the native example build and must not become a
core consumer dependency. Final application/build test evidence is recorded in
the prompt's validation report, separately from this source audit.

Independent application review identified and requested corrections for event
queue saturation hiding a lost error, repeated resource deletion after a setup
cleanup failure, and UART deletion truncating bytes already accepted into the
hardware FIFO. Final source review confirmed the implemented saturation checks,
pre/post-read error checks, one-attempt cleanup flags and zero-wait TX completion
check. Shutdown retains static loss evidence at its 64-turn limit; only a failed
console deletion permits one further 64-turn report-drain phase, without another
deletion attempt. Static contexts survive return from the pinned SDK main task.
No remaining confirmed ownership, console-bound or identity defect was found.

Independent local verification ran
`ctest --test-dir build/native -C Debug --output-on-failure -R idf_app`:
23/23 registered application scenarios passed, including post-read RX loss,
event saturation, combined setup/cleanup failure and pending/stalled TX FIFO.
These recording-backend tests do not establish physical UART or board timing.
