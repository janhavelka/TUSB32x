# Independent review of prompts 23–25, 2026-10-28

Entry HEAD `7f23b901b97325fa3c49f3a1e4f26bd05d4231f0` was clean and synchronized
on `promptset/tusb32x-implementation-20261023`. The lead reread all three prompts,
contracts, implementation and tests; parallel reviewers independently examined
coverage, actual application paths and SDK timeout/lifetime semantics. Findings
were checked against code and diffs, not accepted from previous summaries.

## Confirmed findings and corrections

| Requirement | Actual evidence | Correction / regression |
| --- | --- | --- |
| Callback error vocabulary | Wire transfer treated timeout zero as invalid arguments | I2C_TIMEOUT before any backend work; localRejections tests both callbacks |
| Application owns only its bus | Wire.cpp 3.3.11 begin returns true for an initialized controller without checking configuration | ExampleApp checks i2cIsInit(0) first; existing-bus scenario proves no begin/transfer/end and reports not_owned |
| Honest teardown | Wire.end returns false after failed native deletion, but application discarded ownership | releaseWire attempts once, checks result, retains ownership/retired context; end-failure and cleanup-failure prove no retry and live diagnostics |
| Console survives rejected setup | SDK pin peripheral manager can detach native USB on GPIO19/20 reassignment | Reject SDA/SCL or configured IRQ conflicts; two separately compiled pin scenarios |
| Supported SDK timing subset | IDF5.5.5 S2 REF_TICK/400kHz arithmetic underflows; extremely low rates exceed S2/S3 timing fields | Native default clock only, both adapters 100000..400000Hz; invalid clock/rates and accepted boundaries tested; Wire records application busHz |
| Coarse tick deadline example | q=10000us yields 522002us combined envelope for C=2000us/T=10ms | Native guide uses 600ms; independent fake worst-wait test rejects 400ms at100Hz and accepts600ms at both rates |
| Direct application evidence | Portable helpers alone did not execute real setup/cleanup/USB boundary | Ten registered actual ExampleApp + adapter scenarios, including stalled sink and active/terminal CLI result ownership |

The SDK findings use installed Arduino3.3.11 Wire.cpp/esp32-hal-i2c-ng.c and
IDF5.5.5 i2c_master.c plus target HAL i2c_ll.h and soc/usb_pins.h. Exact official
links and aggregate wait proof remain in [Wire](../arduino-transport.md) and
[native](../idf-transport.md) guides. Failed native receive retirement, explicit
STOP-separated pointer framing, conservative error mapping and same-core terminal
lifetime rules were rechecked; no further defect was confirmed under the stated
application prerequisites. No custom driver, retry, core framework dependency,
second CLI or prompt26 application was added.

## Peer/source boundaries

Read-only peer revisions and clean state were rechecked; all match the cited
implementation audits: INA228 b28c5ea, ADS1115 235af3e, TCA9548A ce425e1,
TMP1x2 bafc340, LSM6DS3TR 362b140, LDC1614 1db38b6 and MB85RC 1bf7c29.
Exact paths/full revisions and adopted/rejected patterns are in the
[23 audit](execution-23-arduino.md), [24 audit](execution-24-arduino-examples.md)
and [25 audit](execution-25-peers.md). Timeout parameter names, peer short-write
cleanup and copied USB defaults are not stronger evidence than actual SDK code.
Sibling files and Git state were not changed; remote CI was not queried.

## Verification boundary

The lead reviewed integrated fixes and the independent reviewer rechecked final
ownership/lifetime/clock changes. [Validation](../validation.md) records final
host suites, all eight actual SDK images and documentation/package checks.
Thin host SDK fakes prove application decisions; they do not model physical
interrupt concurrency or qualify a CPU/preemption envelope. No board was flashed,
and no serial, HIL, waveform, supply or electrical qualification is claimed.
