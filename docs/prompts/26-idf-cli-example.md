# 26 — Complete the native ESP-IDF CLI application

Read [COMMON.md](COMMON.md). Prerequisites: prompts 1–25, including the native
adapter and shared CLI. Deliver the actual native application and its S2/S3
builds. Reuse existing protocol, operation and CLI code.

## Audit and delegation

Re-audit TMP1x2 native CLI, SHT3x shared processor integration and current IDF
console examples/AGENTS. Read CLI_CONTRACT, the established adapter contract and
official documentation for the selected console/I2C SDK. Record current pins,
versions and meaningful differences.

A coding agent can own target configuration/build checks while the lead owns
application lifecycle. A second reviewer can inspect console blocking and
identity generation. Follow AGENT_WORKFLOW, disjoint ownership and lead-only Git.

## Implement the application

1. Add `examples/esp_idf/basic/CMakeLists.txt`, `sdkconfig.defaults`, target
   defaults as needed and `main/CMakeLists.txt` / `main/main.cpp` with
   `extern "C" void app_main()`. Root component registration provides only core;
   the application explicitly adds the native adapter and shared CLI.
2. The application owns native bus/device handles, tasks, locks, clock, console
   and optional GPIO ISR notification. No Arduino-as-component, Wire, Serial,
   Arduino clock or facade. Create resources before binding and release them only
   after work ends and the driver no longer borrows their context.
3. Set actual input and output console routes for S2/S3 USB or UART deliberately.
   Bound reads, writes and loop work so absent/disconnected USB cannot stall the
   owner. ISR only flags/notifies; owner serializes core access and invokes the
   shared CLI's single poll path. Do not introduce a native-only command grammar.
4. Supply explicit board model/address, wiring overrides and absent optional pins.
   Start with board readiness, bind and explicit read-only identification/status.
   Do not apply profiles, ACK or reset automatically. Original TUSB320/I recovery
   after VDD return is an owner-selected call after identity, using the existing
   engine; later variants do not inherit the procedure.
   Assign startup requests to an explicit application owner using the same poll
   path. Inspect and take each exact terminal result before another startup/CLI
   operation; failures leave diagnostics available without automatic retry or an
   unowned retained result blocking the console.
5. Generate example-only firmware identity on clean and incremental builds:
   library, framework, target, build/Git/dirty and protocol identity agree with
   Arduino machine records. Core consumers still require neither Git nor Python.
   Changed source metadata must not leave stale firmware identity after rebuild.
6. Handle creation failures and shutdown explicitly. Stop new commands, finish or
   cancel locally, retain/report outcome and effect, release binding, then delete
   owned device/bus/console resources. Never destroy a shared consumer-owned bus.

## Verify and document

Recheck [CHIP_EVIDENCE](CHIP_EVIDENCE.md)'s board prerequisites against actual
wiring. LAI/HAI operates at 2.7-5.0 V; 322I at 4.5-5.5 V. Local 3 A source
advertisement requires LAI VDD >=3.5 V; remote 3 A detection does not. Record
ADDR straps, exact enable polarity, 320 PORT versus 322 DIR and resistor-coupled
VBUS_DET. A 28 V system-rail limit is not the pad rating. Do not copy a 320
DFP-only sensing omission into 322. Explain autonomous 322 VCONN and its
min/typ/max limits without presenting typical current as a guaranteed capability
or software-controlled output.

Establish and run real IDF set-target/build commands for esp32s2 and esp32s3
using separate build directories and isolated target configuration. For example,
use `idf.py -C examples/esp_idf/basic -B <build-dir> set-target esp32s2`
then build with that same directory; document the corresponding S3 commands.
Verify the linked images contain shared CLI and core and require no Arduino.

Run the unchanged shared CLI behavioral suite and appropriate native adapter
tests. Check missing IRQ/console, resource creation failure, command/result
ownership, output backpressure, error/time mapping and teardown. Console route
compilation is not proof of physical input/output; record serial checks separately
and use hardware only when target/port are identified and authorized.

Write Doxygen for application/context functions and source-list choices that
need explanation. Update native README, examples index, integration/owner guide,
CLI provenance notes, CHANGELOG and validation with actual versions, commands and
results. Include wiring, straps, enable/supply prerequisites and console selection.

## Finish this prompt

The complete example must configure, compile and link for every claimed target;
no SDK shim or workflow file substitutes for those builds. Record unavailable
checks as BLOCKED/NOT RUN, and preserve hardware gaps separately. Update progress,
review the complete change, **commit and sync** on the same execution branch and
verify upstream equality.
