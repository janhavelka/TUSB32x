# Prompt 34 SDK compatibility and bounded-work audit

Audited on 2026-10-28. This is source and host-test evidence for the native
adapter's exact ESP-IDF 5.5.5 and 6.1 selections on ESP32-S2/S3. It does not prove
SDK image linkage, physical timing, interrupt scheduling or board qualification.
The prompt 34 validation record owns actual firmware and remote CI results.

## Sources and version identity

Official annotated tags resolve to these source commits:

| Release | Commit | Native I2C source SHA-256 |
| --- | --- | --- |
| 5.5.5 | `b774170ff46c393eeb5e495ea37936038d3f4f4f` | `884ee968512d8e375cd863641a325c4c8d98a21fa302b9cbcc5627f156dda790` |
| 6.1 | `fff9895c82d744c7237be8847347bdd1b07c6643` | `3ebaeee176e2ff731d15c69a7629c32c0339817afcd31e93da167f4f6b9d9ca0` |

The installed `C:/pio/packages/framework-espidf/components/esp_driver_i2c/i2c_master.c`
has the 5.5.5 hash above (69,303 bytes). The official 6.1 file is 69,351 bytes.
Downloaded source copies used for the comparison are under ignored
`build/p34-audit/`; they are audit inputs, not production or packaged files.
The official [6.1 release](https://github.com/espressif/esp-idf/releases/tag/v6.1)
identifies the current minor release. The oldest adapter claim is exactly 5.5.5;
there is no inferred support for intermediate releases or earlier sibling pins.

Version detection must read the three integer settings in
[tools/cmake/version.cmake](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/tools/cmake/version.cmake):
6.1 is the numerical tuple `6.1.0`. The corresponding
[public version header](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_common/include/esp_idf_version.h)
agrees. `version.txt` at the SDK root is optional packaging metadata, not a
required file in an official SDK checkout. IDF's
[build.cmake](https://github.com/espressif/esp-idf/blob/b774170ff46c393eeb5e495ea37936038d3f4f4f/tools/cmake/build.cmake#L95)
uses it when present and otherwise obtains Git/numerical version information.

## Native transfer envelope

The [5.5.5 driver](https://github.com/espressif/esp-idf/blob/b774170ff46c393eeb5e495ea37936038d3f4f4f/components/esp_driver_i2c/i2c_master.c)
and [6.1 driver](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_driver_i2c/i2c_master.c)
retain the same relevant finite wait structure. For q = ceil(1000000 / tickRateHz)
microseconds, positive SDK timeout T milliseconds and qualified whole-callback
CPU/preemption allowance C microseconds:

```text
B(T) = 7 * (1000*T + q) + 2 * (50000 + q)
write        <= C + B(T)
pointer/read <= C + B(T1) + 2 + B(T2)
```

The envelope includes mutually exclusive waits deliberately. In 6.1:

| Exact source location in i2c_master.c | Contribution or lifetime fact |
| --- | --- |
| Lines 1014-1019 | One bounded synchronous bus semaphore wait |
| Lines 529-581 | At most four command semaphore waits for the admitted short operations |
| Lines 585-604 | One final event wait and one NACK-busy wait |
| Lines 682-683, 549-554, 594-599 | Initial stale/busy clear and at most one later error clear |
| Lines 45, 85-101 | Each hardware clear bounded by 50ms plus one tick; comparison is strict `>` |
| Lines 1273-1312, 1343-1365 | Three TX/four RX operations, STOP termination, positive finite timeout forwarded |
| Lines 723-729, 1033-1044 | Synchronous interrupts disabled, failed transaction reset, SDK bus semaphore released |
| Lines 292, 777, 811 | Read descriptor flag still requires controller retirement after failed receive |

The maximum admitted TX is two bytes and RX is eight bytes. Both targets retain
FIFO32. The moved [S2 LL header](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_hal_i2c/esp32s2/include/hal/i2c_ll.h)
and [S3 LL header](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_hal_i2c/esp32s3/include/hal/i2c_ll.h)
retain the previous timing arithmetic. Both use hardware bus clear and the full
reset/mask path rather than the incomplete hardware FSM-only reset. Keep the
default clock and 100000..400000Hz restriction; the 6.1 refactor is not evidence
to widen the admitted clock/rate range.

Keep `trans_queue_depth=0`, no event callbacks, same initialization/owner core,
no owner migration, one nonblocking application gate for every bus user, disabled
PM and SDK logging, and the 100..1000Hz tick guard. Positive per-wait milliseconds
must cover at least one tick because `pdMS_TO_TICKS` truncates. The uint16 cap
keeps conversion finite at these rates. The original aggregate start/deadline
and second-phase recomputation remain unchanged.

The 6.1 implementation replaces conditional clock macros with
`PERIPH_RCC_ATOMIC`. Its
[definition](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_hw_support/include/esp_private/periph_ctrl.h#L45)
and [implementation](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_hw_support/periph_ctrl.c#L20)
use the shared peripheral spinlock. C must cover contention from other peripheral
users as well as scheduling, ISR/flash stalls and all callback CPU work. The bus
gate alone does not serialize this global lock. A previously measured 5.5.5
overhead does not establish a 6.1 bound. Reject physical integration unless this
whole allowance can be qualified; the late-success check cannot establish it.

For arithmetic only, C=2000us and T=10ms give combined bounds 360002us at
1000Hz and 522002us at 100Hz. The 600ms callback example contains both; 400ms
contains only the former. These values are not measured application guarantees.

## Error, buffer and resource semantics

The native bus/device configuration fields and transmit/receive signatures used
by the adapter are unchanged. 6.1 adds an internal-memory check for receive when
`CONFIG_I2C_ISR_IRAM_SAFE` is enabled (i2c_master.c:1347-1349). The adapter's
eight-byte local candidate fits the normal internal task stack. An application
using another stack placement must satisfy this SDK requirement; a rejected call
remains a terminal error and conservatively retires the controller.

6.1 changes the return at i2c_master.c:726 from `ESP_ERR_INVALID_STATE` to
`ESP_ERR_INVALID_RESPONSE` for every non-DONE synchronous status. Despite the
SDK comment/header wording, that condition includes several timeout outcomes.
Map it to I2C_ERROR and retain the full `0x108` detail. Do not infer a NACK phase
or translate it into owner TIMEOUT. ESP_ERR_TIMEOUT alone retains its existing
I2C_TIMEOUT mapping. The regression adds this code to the existing error matrix:
pointer failure issues no receive; receive failure preserves output and retires
all users; real backend detail stays primary even after a late return.

Both versions retain read descriptor state on failed receive. Keep same-core
interrupt exclusion and controller-wide retirement; no new SDK call may awaken
the stale descriptor. The adapter adds no retry or bus recovery.

6.1's `i2c_master_bus_destroy` (lines 893-940) improves interrupt-release failure
handling. It can also return a pin-deinitialization error after freeing the bus.
Therefore a failed teardown means uncertain resource disposition, not proof that
the opaque handle remains usable. The example already makes only one teardown
attempt, stops driver borrowing, retires the bus and retains static context
storage without dereferencing/retrying the failed handle. Preserve that policy.

## Native UART application

Compared the installed 5.5.5 UART source against
[6.1 uart.c](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_driver_uart/src/uart.c),
114,919 bytes, SHA-256
`e8f92b7f2616142cc6b55b42ac8e1b34f94c38618c0c07c8fdae065addaaba8b`.

- `uart_tx_chars` lines 1621-1634 still takes `tx_mux` with portMAX_DELAY and
  writes available FIFO space only. Exclusive application UART0 ownership,
  no VFS/logging/other TX user and same-core application access are essential.
- `uart_wait_tx_done(...,0)` lines 1558-1618 retains zero semaphore waits and
  its explicit 2us setup guard. It does not wait for arbitrary wire completion.
- `uart_read_bytes(...,0)` lines 1738-1777 retains zero semaphore/ring waits;
  copied chunks reduce the finite requested 64 bytes. The same-core setup avoids
  the source's cross-core buffer-full continuation case.
- ISR event enqueue failure remains possible at lines 1397 and 1543. Preserve
  the application's full-queue/drain-budget input-failure rule.
- The timeout parameter spelling becomes uint32_t; literal zero is compatible.
  [uart.h lines 394-447](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_driver_uart/include/driver/uart.h#L394)
  keeps five-argument `uart_set_pin` source compatibility through an inline
  wrapper that supplies unused DTR/DSR pins. Zero-initialized UART configuration
  leaves the new glitch filter disabled. No application API shim is needed.

## Sibling re-audit and adopted boundaries

Both sibling worktrees were clean when inspected; neither was changed.

| Peer HEAD | Exact inspected sources and conclusion |
| --- | --- |
| LDC1614 `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `AGENTS.md`, `.github/workflows/ci.yml`, `platformio.ini`, `examples/esp_idf/basic/main/{CMakeLists.txt,main.cpp}`, `tools/check_idf_example_contract.py`, `test/{test_basic.cpp,test_cli.cpp}`. Native job selects 6.0.2 and both S2/S3, actual app_main and esp_driver_i2c link inputs. Unity suites are executable through `pio test -e native`. |
| INA3221 `41b6184b75f3c152611d4236f7ea41a5ce77bc2c` | `AGENTS.md`, `.github/workflows/ci.yml`, `platformio.ini`, `examples/esp_idf/basic/main/{CMakeLists.txt,main.cpp,Ina3221IdfI2cTransport.cpp}`, `tools/check_idf_example_contract.py`, `test/{test_basic.cpp,test_owner_operations.cpp}`. Native Docker commands select 6.0.1 for S2/S3, actual app_main/native API and component requirements. Unity host test registration is explicit. |

Adopt real native image links and explicit runtime registration. Reject copying
their unpinned action tags, shared SDKCONFIG/build paths or assuming their SDK
success transfers to TUSB32x. Their native helpers using a supplied timeout do
not replace this adapter's aggregate wait/cleanup proof. No sibling remote runs
were queried.

Current local sources re-read include both actual adapter `.h/.cpp` pairs,
`examples/esp_idf/ExampleApp.cpp`, its native main, both SDK fixtures,
`docs/{idf-transport.md,arduino-transport.md}`, `test/CMakeLists.txt` and the native
adapter fake/test sources. The Arduino guard remains exactly Arduino3.3.11 /
IDF5.5.5, with globally disabled HAL locks, prepared handles and controller
retirement as documented. This native audit does not admit Arduino on IDF6.1.

## Focused verification

After the exact native version gate and INVALID_RESPONSE regression change:

```text
cmake --build build/native --config Debug --target tusb32x_idf_transport_1000 tusb32x_idf_transport_100
ctest --test-dir build/native -C Debug -R '^tusb32x_idf_transport_(100|1000)$' --output-on-failure
```

Both registered suites passed. They exercise the actual adapter against the
recording backend at 1000Hz and 100Hz; they do not execute the IDF driver.
The required real 5.5.5/6.1 extracted S2/S3 links belong to the separate prompt34
matrix evidence. Hardware qualification remains NOT RUN.
