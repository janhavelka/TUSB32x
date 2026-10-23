# 23 — Implement the Arduino transport adapter

Read [COMMON.md](COMMON.md). Prerequisites: prompts 1–22 are complete, including
the linked core and shared CLI. This prompt delivers the terminal Arduino
callbacks and their build/contract tests. Prompt 24 connects them to complete
board examples; do not add a second command processor here.

## Audit and delegation

Re-audit INA228/ADS1115 adapters and TCA9548A transport tests, their AGENTS and
the current pinned Arduino SDK documentation. Read ENGINEERING_RULES transport
ownership and ENGINEERING_BASELINE time/lifetime sections. Record exact sources,
versions and adopted/rejected error and timeout conventions.

A coding agent may own backend fixtures while the lead owns the adapter.
Use a strong reviewer for aggregate deadlines or uncertain error effects.
Follow [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md): disjoint edits, lead integration
and Git ownership. Subagents are optional; they do not change the branch.

## Implement one complete adapter

1. Add the adapter under `examples/arduino/`, with a borrowed context containing
   the application's `TwoWire*`, timeout settings and accurately named backend
   counters. Application code creates/destroys the bus and configures pins/speed.
   No callback calls Wire.begin/end, deletes a bus or repairs a shared bus.
2. Implement the exact DriverConfig callback signatures and framing established
   in prompts 4 and 9. Validate pointers/lengths/address before backend work.
   A combined pointer/read keeps serialization through both phases; pointer
   selection is explicit. Support only framing the selected SDK can actually
   provide and document STOP/repeated-START behavior.
3. Clip lock acquisition and all bus phases to one aggregate finite callback
   timeout. Check remaining time between phases. Round backend timeouts without
   turning an expired or sub-resolution budget into an infinite wait or exceeding
   the promised bound. Never grant the read a second full timeout.
4. Check every Wire write acceptance count, request/read length and terminal
   return code. Preserve full numerical backend detail. Map only backend-confirmed
   address NACKs as such; generic errors do not prove phase or no hardware effect.
   Callback I2C_TIMEOUT differs from owner TIMEOUT. No hidden retry or pending DMA.
5. Supply the example's monotonic 64-bit millisecond clock using a native source
   or correctly extending rollover. Casting raw wrapping millis() is insufficient.
   Record resolution/rounding and respect the core's fresh-time hardware waits.
6. Keep physical transaction/byte counters separate from core callback attempts.
   Do not fabricate partial counts unsupported by Wire. Contexts, buffers and
   locks obey the baseline lifetimes; no reentry or bus lock across owner polls.
7. Add a minimal SDK build fixture that compiles and links the real adapter with
   the core for S2 and S3, using the established `scripts/pio.cmd` and a verified
   pinned platform. The fixture is an adapter build check, not a hardware demo.
   Do not create duplicate placeholder main functions in the production examples.

## Verify and document

Test success, rejected arguments, short write/read, NACK versus generic failure,
timeout after pointer selection, lock contention and remaining-time clipping.
Use backend fakes where they can observe these behaviors; compile the actual
SDK calls in the S2/S3 fixtures. Check nearly expired/coarse timeout cases,
clock rollover and non-owning cleanup. A fake-only compile is not an SDK build.

Add file/context/function Doxygen describing units, framing, lifetime, aggregate
timeout, serialization and uncertain effects. Update the adapter integration
guide, examples index, CHANGELOG and validation evidence with exact versions and
commands. Keep framework types outside public core headers.

## Finish this prompt

The callbacks must be usable, terminal and covered by registered meaningful tests;
the declared adapter subset must link. Run core and shared CLI regressions
affected by the integration. Record each actual SDK build separately and mark a
missing tool BLOCKED, not PASS. No flash or physical USB/role claim is made by a
build. Update progress and **commit and sync** this checked unit on the existing
execution branch; verify upstream equality before reporting completion.
