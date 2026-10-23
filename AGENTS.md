# TUSB32x working rules

## Scope and evidence

- This checkout starts as an audit and promptset. Implement the library when the
  user asks to execute those prompts; do not interpret planning as a completed
  driver. Preserve the audit as dated evidence.
- Read `../AGENTS.md`, `docs/prompts/README.md`, and
  `docs/prompts/COMMON.md` before implementation. For **every prompt**, inspect the
  current relevant sibling I2C code, examples, tests, runners, and CI; cite files
  and revisions. Historic audit reports are starting points, not substitutes.
- Follow TI's exact variant datasheets for protocol facts. Never copy a peer
  sensor's register behavior into this controller. Record unresolved TI wording
  rather than inventing hardware guarantees.
- Sibling libraries and FieldCore-node are read-only references unless the user
  separately requests changes. Do not import their AGENTS rules as authority to
  push, flash, or modify another repository.

## Library direction

- Public headers: `include/TUSB32x/`; namespace and primary class:
  `TUSB32x::TUSB32x`; C++17 implementation in `src/`.
- Standard C++ core: no Arduino, ESP-IDF, FreeRTOS, platform clocks/delays,
  logging, heap allocation, tasks, GPIO, power switches, or owned I2C bus.
- The application owns transport, serialization, time, pins, recovery and USB
  power/data policy. Callbacks are synchronous, terminal, timeout-bounded, and
  non-reentrant. Cooperative operations keep individual owner steps bounded.
- Match the sibling Status, typed enums, passive diagnostics/health projection, version generation,
  shared CLI and verification conventions as specified in `COMMON.md`.
- Keep transport health, configuration trust, port state, event delivery and
  operation outcome separate. Detach is normal. Acknowledging INT is a write.
- Treat mixed RO/RW/W1C/reset registers deliberately. Cancellation or a transport
  error cannot undo a write. Never replay an uncertain reset or role change.
- The core is useful to FieldCore-like owners and bare-metal/other frameworks
  without a FieldCore dependency. No USB PD, charger, or VBUS policy is implied.

## Work and validation

- Preserve unrelated edits. If Git exists, inspect branch/upstream/status before
  changing it; keep the promptset on one branch unless instructed otherwise.
  A missing Git repository or remote does not prevent local work. Do not invent
  a remote, publish, or derive push authority from a sibling's local rules.
- One lead integrates shared-agent edits and owns Git mutations. Parallel agents
  should have disjoint files and specific review tasks.
- Inspect CI first; run focused checks during work, then full checks for major
  milestones in accordance with `../AGENTS.md`. Report actual commands/results
  and unavailable tooling honestly; never weaken gates to obtain green output.
- Use the existing VS Code-managed PlatformIO via `scripts/pio.cmd` once created.
  Do not install a second local PlatformIO Core.
- `library.json` is the version source. Generate `Version.h` and keep CMake,
  component metadata and Doxygen version fields synchronized.
- Test behavior and fault boundaries, not just source text. Native IDF must link
  without Arduino. Clean consumers must not need sibling paths, Git or generator
  tooling merely to compile the packaged library.
- Keep source URLs, document revisions, retrieval dates and SHA-256 hashes for
  vendor reference artifacts. Preserve vendor licenses and exclude reference
  source and PDFs from production compilation and normal library packages.
