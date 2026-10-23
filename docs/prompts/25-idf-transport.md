# 25 — Implement the native ESP-IDF transport adapter

Read [COMMON.md](COMMON.md). Prerequisites: prompts 1–24 and the existing root
component metadata. Deliver a native IDF adapter with verified timeout/lifetime
semantics and a real linked SDK fixture. Prompt 26 adds the complete native CLI
application. This adapter does not wrap Arduino.

## Audit and delegation

Re-audit LDC1614/MB85RC component and native transport layouts, TMP1x2 native
examples and their AGENTS. Check official documentation for the selected pinned
IDF API. Read ENGINEERING_RULES transport ownership and the baseline callbacks/
clock contract. Record version, source paths and reasons for departures.

Use a coding agent for the isolated component consumer or backend fixtures;
use strong review for deadline rounding and uncertain effects. Freeze callback
interfaces first, assign disjoint files and follow AGENT_WORKFLOW. The lead owns
integration and Git.

## Implement and expose only working behavior

1. Place native sources under the platform-specific examples path in the baseline.
   Use the selected SDK's native master-I2C API. The application creates and owns
   bus/device handles; the adapter borrows them. Do not create/delete buses during
   bind/end, change a consumer's speed or expose SDK types in public core headers.
2. Implement terminal synchronous callbacks with explicit pointer framing and
   serialization across all phases. Pending DMA/asynchronous work must not outlive
   borrowed buffers. The core never imports Arduino, Wire, Serial or FreeRTOS.
3. Use one finite aggregate timeout for lock acquisition, pointer selection and
   data phase. Convert the owner millisecond budget into backend units once per
   remaining phase without resetting it. Check nearly expired budgets before a
   coarser timeout API; zero must not become an infinite SDK wait.
4. Map esp_err_t conservatively into Status, preserving full numerical detail.
   Distinguish operation TIMEOUT from callback I2C_TIMEOUT. Do not invent NACK
   phases, partial byte counts or proof of no write from coarse SDK errors.
5. Convert native monotonic time to the baseline 64-bit millisecond domain at the
   owner boundary. Document rounding/resolution, fresh callback completion handling
   and the one-tick scheduling margin. Keep observable backend counters separate
   from counted core callback attempts.
6. Provide explicit context lifetime and shutdown ordering. Rejection performs no
   backend calls; callbacks never reenter the driver. Bus locks do not survive a
   poll interval. Failure to create resources is handled by the application, with
   no success-shaped half-bound context.
7. Create an independent minimal IDF component/adapter build fixture under the
   test consumer area. It links core plus the real adapter without compiling the
   repository CLI example as library source, without absolute workstation paths,
   and without an Arduino component. Do not duplicate production protocol logic.

## Verify and document

Audit [CHIP_EVIDENCE](CHIP_EVIDENCE.md)'s actual pointer/framing and address
sources against the chosen SDK: LAI `0x47/0x67` are seven-bit device addresses;
wire bytes and register `0x45` are different values. Explicitly select the
register pointer. A combined SDK call does not establish universal
repeated-START support; preserve required serialization if STOP-separated phases
are used. Test emitted frames and one finite terminal timeout across both phases.
Record 400 kHz maximum and application-established supply, pull-up and enable
readiness; SDK success cannot prove these physical conditions.

Run real configure/compile/link for esp32s2 and esp32s3 with isolated build
directories and target-specific configuration. Use the pinned supported SDK
selected from evidence; record exact commands and artifacts. SDK-header shims
may help unit tests but do not prove a linked firmware image.

Exercise argument rejection, backend error mapping, finite lock wait, success
and failure between pointer/read phases, near-expired/coarse timeout behavior,
borrowed context lifetime and cleanup order. Backend fakes must report their
limits; they cannot prove physical transaction timing. Keep core/CLI host tests
passing after any shared correction.

Write file/type/function Doxygen for borrowed handles, units, rounding, framing,
terminal completion and concurrency. Update integration and component-consumer
guides, examples index, CHANGELOG and validation. Document supported SDK choices
without claiming hardware behavior.

## Finish this prompt

Every advertised adapter function must be implemented and linked; required
contract tests and real SDK builds have their actual outcomes recorded. A missing
SDK remains BLOCKED. No CLI application, firmware flash or hardware qualification
is implied by this fixture. Update progress and **commit and sync** this complete
adapter unit on the existing execution branch, verifying upstream equality.
