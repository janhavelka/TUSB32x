# Arduino example audit, 2026-10-28

Entry revision: `eace1c3a33961bfad83798af4c15ae1ecb9b6d68`, clean and synchronized
on `promptset/tusb32x-implementation-20261023`. Prompt 23's adapter and actual
S2/S3 link checks are the prerequisite. Siblings were read-only; remote CI and
hardware were not invoked.

| Evidence | Revision / paths | Adopted / rejected |
| --- | --- | --- |
| TMP1x2 | Clean `bafc34067530eb340e55c42dae3fad4eb795b6e9`; AGENTS.md, platformio.ini, examples/01_basic_bringup_cli/main.cpp, test/console/test_console.cpp, tools/hil_tmp1x2_runner.py, .github/workflows/ci.yml | Adopt explicit S2/S3 USB settings, finite input, whole-record loss reporting. A zero Serial TX timeout alone does not prove TinyUSB cannot block. |
| LSM6DS3TR | Clean `362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc`; AGENTS.md, examples/02_owner_soak/main.cpp, examples/common/BoardConfig.h, test/test_basic.cpp, tools/run_owner_soak.py, platformio.ini, .github/workflows/ci.yml | Adopt exact-token result take, one poll per owner turn and separate fixed scheduling. Reject steady-path Serial.flush, sensor configuration/maintenance, copied sensor pins and callback-as-transaction wording. |
| Existing TUSB32x CLI | examples/common/Tusb32xCli.cpp: Cli::tick and deliverResult; test/cli/ | Automatic polling belongs only to CLI-admitted jobs. Application startup must own its own token. Routing startup through CLI result take can strand startup behind output reservation, so consume application-owned results independently. |
| Existing TUSB32x adapter | examples/arduino/Tusb32xWire.cpp and docs/arduino-transport.md | Both devices share one Context, gate and retirement state. Prepared handles, pinned owner core, explicit cleanup acceptance and a qualified CPU overhead remain prerequisites. |

## Pinned SDK and USB isolation

The selected platform remains pioarduino 55.03.311 / Arduino 3.3.11 /
ESP-IDF 5.5.5 `b774170ff46`. Current official
[CDC API documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/usb_cdc.html)
was checked against the pinned implementation, not treated as a stronger timing
guarantee. Local installed SDK sources were inspected alongside:

- [USBCDC.cpp](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/USBCDC.cpp):
  S2 transmit enters TinyUSB; its bool conversion requires DTR and RTS, while
  actual transmit readiness requires DTR. Disable the optional reboot-on-line
  changes policy in this diagnostic example.
- [HWCDC.cpp](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/HWCDC.cpp):
  S3 zero-timeout ring operations are finite, but disconnect races can replace
  SDK-buffered bytes. SDK acceptance is never host delivery evidence.
- Bundled TinyUSB `53f8c53c2` [stream code](https://github.com/espressif/tinyusb/blob/53f8c53c2/src/tusb.c)
  and [FIFO code](https://github.com/espressif/tinyusb/blob/53f8c53c2/src/common/tusb_fifo.c):
  stream/FIFO/endpoint mutex paths can use `OSAL_TIMEOUT_WAIT_FOREVER`, mapped to
  `portMAX_DELAY`. Direct tud_cdc calls do not remove this problem.

Use one static USB worker and a single copied 256-byte mailbox. The owner only
tries a nonblocking handoff. The worker retains partial output and never touches
the core, Wire, CLI or ConsoleBuffer. A stuck SDK transmit cannot hold an owner
lock or borrowed owner buffer. Keep USB resources alive after bus shutdown;
never delete the worker or Serial resources while the SDK might still use them.
Serial input uses zero-wait queue operations and has a separate 64-byte bound.

The actual S2 compiler does not promise always-lock-free atomic<bool>. The fixed
handoff therefore uses two standard atomic_flag operations with no retry loop.
Flags occupy separate aligned words. The pinned IDF
[atomic implementation](https://github.com/espressif/esp-idf/blob/b774170ff46/components/newlib/src/stdatomic.c)
and [platform helpers](https://github.com/espressif/esp-idf/blob/b774170ff46/components/newlib/priv_include/esp_stdatomic.h)
provide the S2 byte exchange inside a small interrupt-masked section; S3 uses
word exchange. No USB or user callback runs inside that section. Both actual
cross-compilers and linked images were checked, alongside a concurrent host test.

## Ownership choices

Application startup performs read-only initialization and port refresh through
the core, consumes its exact terminal results and then releases the primary
device to the shared CLI. Terminal failure stops startup without retry and
leaves local diagnostics usable. CLI operations retain their own results until
explicit command take. One selected owner advances per loop turn in the
two-device example. Optional IRQ signals coalesce; no lossless edge count is
claimed. The secondary address is reserved against primary CLI retargeting.

Shutdown stops admission and IRQ signaling, cancels/retains or consumes only
application-owned results, ends both bindings, then releases the application's
exclusive Wire controller. USB drainage is not a shutdown prerequisite.

Exact electrical prerequisites remain in the variant guide: no universal
enable pulse resolves TI's conflicting EN_N startup wording. Default builds
must remain diagnostic-only until the application's board readiness, SDK
cleanup acceptance and timing qualification are explicitly supplied.
