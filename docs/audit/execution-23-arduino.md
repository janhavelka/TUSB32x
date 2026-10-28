# Prompt 23 audit, 2026-10-28

Entry `eba12f2`, clean and synchronized on the existing execution branch.
Three inherited-model agents handled peer/SDK fixtures, independent deadline
review, and disjoint host tests. Lead selected the supported contract and reviewed
source, tests, SDK build logs and compilation databases. No sibling changes.

## Current peers

All clean; AGENTS, actual code, matching tests, owner examples, runner semantics,
registration and checked-in workflows were inspected read-only.

| Peer HEAD | Paths and adopted/rejected rules |
| --- | --- |
| INA228 `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | `examples/common/I2cTransport.h`, examples and `tools/run_i2c_hil.py`: borrow the bus and label callback counts honestly. Reject code2 address-NACK inference, renewed phase budgets and short-write cleanup that emits an empty probe. |
| ADS1115 `235af3e8c722661a89a44766a4ccd903a654f6fe` | `examples/common/I2cTransport.h`, owner-safe example and tests: external ownership/remaining budget is useful. Reject rounding an insufficient remainder up, abandoning Wire's held lock after short write, callback validation errors outside I2C_* and raw millis casting. |
| TCA9548A `ce425e1984ac269e7934dd811a06ec08b91e307c` | `test/test_wire_transport/test_wire_transport.cpp`, fake `Wire.h`, `examples/common/I2cTransport.h`, `examples/01_basic_bringup_cli/main.cpp`: adopt recording fixtures and generic code2 mapping. Verify receive retirement independently against SDK. Reject pointerless mux framing and sending a partial buffer to release a lock. |

All peers pin pioarduino 55.03.311. Their workflows register separate host and
S2/S3 builds; none proves this adapter's aggregate deadline. No remote runs queried.

## Exact SDK evidence

Platform release 55.03.311 selects Arduino 3.3.11 and libraries
5.5.5+sha.b774170ff46. Installed managed Core 6.1.19 uses effective core directory
`C:/pio`. The inspected user-profile packages and effective build packages agree.
Official raw source retrieved on 2026-10-28 UTC:

| Source | Bytes | SHA-256 |
| --- | --- | --- |
| [Wire.cpp 3.3.11](https://github.com/espressif/arduino-esp32/blob/3.3.11/libraries/Wire/src/Wire.cpp) | 16658 | `6018a5e4c8228cacdf7596a82679a1340a0f178f50f3f2a332dcee7553a09fce` |
| [esp32-hal-i2c-ng.c 3.3.11](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/esp32-hal-i2c-ng.c) | 13692 | `eb815402f5bef114f04ebb34b1ce5c6437fa45251cbd8eaa9504115d1756cae4` |
| [i2c_master.c b774170ff46](https://github.com/espressif/esp-idf/blob/b774170ff46/components/esp_driver_i2c/i2c_master.c) | 69303 | `884ee968512d8e375cd863641a325c4c8d98a21fa302b9cbcc5627f156dda790` |

The prompt's Arduino3.3.0 seed was reopened; 3.3.11 still has indefinite Wire/HAL
locks. The selected profile compiles these out globally and requires an external
nonblocking gate for every user. Actual compilation databases confirm the flags
on Wire.cpp, HAL C and adapter, not just application code. Target SDK configs
have 1000Hz ticks, no power management and no I2C debug logging.

The [integration guide](../arduino-transport.md) gives the exact bound and required
application qualifications. Relevant IDF symbols are `s_i2c_send_commands`,
`s_i2c_transaction_start`, `s_i2c_synchronous_transaction`,
`s_i2c_master_clear_bus`, `s_i2c_hw_fsm_reset`, `i2c_isr_receive_handler` and
`i2c_new_master_bus`. Short TX/RX fit FIFO32. The finite command list gives at
most seven timeout-bearing waits plus two 50ms clear allowances per phase.
Same-core execution excludes an ISR concurrently surviving terminal mask/reset.
Persistent `contains_read` on some failure paths requires shared-controller
retirement before any further transaction. It is not enough to retire one device.

Wire code2 loses NACK phase, and requestFrom discards ESP status; report only
observable detail. No implementation promises unavailable native detail. HAL
already suppresses i2c.master logs; application must not reenable them. Setup
prewarming avoids allocation/device-list changes during bounded callbacks.

TI framing/address/electrical evidence was rechecked in CHIP_EVIDENCE and the
completed device/variant guides: explicit pointer STOP/read STOP, seven-bit
addresses, 400kHz maximum and separate enable/power prerequisites. No new silicon
encoding or procedure was inferred from these SDK sources.

## Review and validation

Independent review found a read-output publication edge: a second elapsed check
after memcpy could fail after changing caller output. The final code has no
fallible step after publication. The new regression fails twice against the
old path and passes against the corrected source. Firmware include-order review
also exposed Arduino DEFAULT/INTERRUPT macro collisions; the build fixture avoids
unneeded Arduino.h and the application guide explains the boundary.

Host tests compile actual adapter code at 1000Hz and a deliberately coarse 100Hz
fake tick. They prove local bounds/framing/error behavior, not the physical SDK's
timing. Real S2/S3 builds separately link every adapter entry point and core.
No bus is initialized or flashed by the fixture. Detailed commands and outcomes
are recorded in [validation](../validation.md).
