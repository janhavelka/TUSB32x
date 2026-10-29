# 24 — Complete the Arduino CLI and owner examples

Read [COMMON.md](../../prompts/COMMON.md). Prerequisites: prompts 1–23, especially the tested
Arduino adapter and complete shared CLI. Deliver two usable examples on S2/S3:
interactive bring-up and bounded owner service. Reuse the adapter unchanged
unless a real integration defect needs a tested correction.

## Audit and delegation

Re-audit TMP1x2 S2/S3 USB settings, LSM6DS3TR owner-soak scheduling and their
current examples/AGENTS. Check official documentation for the chosen SDK.
Read CLI_CONTRACT console/job rules and the baseline owner/time/effect contract.
A coding agent may own one example after board/adapter interfaces are frozen;
a separate reviewer can check USB stalls and teardown. The lead integrates,
validates and performs Git actions under AGENT_WORKFLOW.

## Build the examples

1. Create `examples/01_basic_bringup_cli/main.cpp` using one shared processor,
   bounded ConsoleBuffer and the Arduino adapter. BoardConfig provides explicit
   model/address, SDA/SCL, optional IRQ/enable, frequency and finite timeouts.
   Support compile-time overrides; identify example pin defaults and leave
   unspecified power/enable pins unconfigured.
2. Application setup owns Wire, console and optional GPIO. Verify documented
   board/supply prerequisites, bind, and explicitly queue read-only identity and
   status. Never hide apply/reset/IRQ ACK in startup. The original 320/I procedure
   after VDD return remains an explicit recover-dead-battery request after
   identity initialization. Reuse the existing engine; do not copy 0x45 writes
   into setup or run the procedure on later variants.
   Setup's queued work has an explicit application owner: advance it through the
   same poll path, inspect and consume its exact terminal result before admitting
   another startup or CLI operation. Failure leaves a usable diagnostic console;
   do not auto-retry or let startup retain an unowned result slot.
3. Loop performs bounded input, one configured owner poll and bounded output.
   Preserve terminal results until the owning command explicitly consumes them.
   USB disconnect or a nonreading host cannot stall owner progress. Bound console
   readiness wait and report dropped output; never spin in while(!Serial).
4. Create `examples/02_owner_service/` demonstrating caller scheduling, a second
   device sharing an externally serialized bus, and optional IRQ flag to owner
   service. Use the platform's synchronization rules; ISR only signals. No I2C,
   logging, core calls, VBUS/charger/mux policy or hidden recovery in an ISR.
5. Stop admitting work before shutdown; cancel or finish locally, retain/report
   results and possible effects, end the binding, then release application-owned
   resources. Never terminate another application's shared Wire bus.
6. Register explicit source-filtered environments for both examples and both
   targets, each with one setup/loop. Primary CLI commands remain
   `scripts/pio.cmd run -e esp32s2dev` and `-e esp32s3dev`; give owner-service
   environments consistent descriptive names and document their exact commands.

The audited TMP1x2 settings are starting evidence, not universal board rules:

| Environment | Board | USB flags | Monitor lines |
| --- | --- | --- | --- |
| esp32s3dev | esp32-s3-devkitc-1 | ARDUINO_USB_MODE=1; ARDUINO_USB_CDC_ON_BOOT=1 | RTS 0, DTR 0 |
| esp32s2dev | esp32-s2-saola-1 | ARDUINO_USB_MODE=0; ARDUINO_USB_CDC_ON_BOOT=1 | RTS 0, DTR 1 |

Both use 115200. Recheck SDK behavior, explain any deliberate changes, document
port-open line toggles and do not copy unrelated sensor bus pins.

## Verify and document

Use [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md)'s exact board constraints in the READMEs:
LAI/HAI VDD is 2.7-5.0 V; 322I is 4.5-5.5 V. The LAI 3.5 V minimum concerns
local 3 A source advertisement, not detecting remote 3 A as a sink. Distinguish
sampled PORT on 320 from DIR on 322, and low-enable from HI/HAI active-high
variants. Never extend a qualified 320 DFP-only VBUS_DET omission to 322. Its
autonomous VCONN needs the specified supply/fixture; the core does not switch it.
Document seven-bit addresses and actual pull-up/strap wiring, not copied board
defaults.

Build all four images and run shared CLI/console tests. Check finite console
work, result ownership, actual library/framework/target/build/Git/dirty records,
missing optional pins and cleanup ordering. Firmware output reports runtime
wiring/framing/timeouts; defaults do not prove a connected fixture.

Write useful source Doxygen, examples index and local wiring/build READMEs,
integration/CLI notes and CHANGELOG. Include exact variant pullup supply, address/
PORT straps, enable polarity, VBUS_DET, I2C speed and supply/current constraints.
Explain unresolved TI startup wording and the chosen board prerequisites without
inventing a universal enable pulse.

## Finish this prompt

Both examples must build for each claimed target with their own source filters.
Report build, flash, serial and HIL evidence separately; only use an identified
authorized board for hardware work. Record unavailable checks honestly.
Update progress and **commit and sync**, including coherent checked units during
work, on the same execution branch. Verify upstream equality.
