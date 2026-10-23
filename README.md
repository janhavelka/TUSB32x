# TUSB32x library planning

This repository currently contains an audit and implementation promptset, not a
finished device driver. The requested starting device is **TI TUSB320LAI**. The
planned library covers the verified I2C-capable TUSB320 variants through explicit
capabilities; the name is not a claim of compatibility with every TUSB32x product.

Start with [the promptset](docs/prompts/README.md). It contains a master execution
prompt, a short common contract, and 38 implementation prompts numbered 1–38. Every
stage requires a fresh audit of relevant sibling I2C libraries before editing.

Each prompt has a bounded scope, prerequisites, delegation guidance and its own
tests, documentation and commit/sync gate. The [scope audit](docs/audit/promptset-scope-review.md)
maps the original stages to the new sequence. Promptset execution has started:
[prompt 1's current comparison](docs/library-comparison.md) and
[engineering decisions](docs/design-decisions.md) establish the implementation
baseline. Prompt 2 adds the [variant matrix](docs/variant-matrix.md),
[feature coverage](docs/feature-coverage.md) and verified source provenance.
The matrix separates exact silicon facts, library policy and unresolved gates;
TI's current LI/HI DFP/DRP caveat remains visible. Driver implementation has not
started. An independent prompt 2 review clarified the TUSB322I VCONN supply
prerequisite and the unresolved write boundary for its shared fault/control
register. See
[execution progress](docs/implementation-progress.md) and [validation](docs/validation.md).

Prompt 3 adds the [register contract](docs/register-map.md) and
[independent encoding vectors](docs/register-vectors.md). They specify exact
access masks, field encodings, safe write images and partial base-part defaults.
Prompt 4 adds the [device procedure guide](docs/device-behavior.md): selected
transport framing, exact role/reset/recovery/IRQ steps, timing anchors, transfer
ceilings and failure vectors. Driver implementation and hardware qualification
remain later work; unresolved TUSB322I writes and board prerequisites stay explicit.

[Chip evidence](docs/prompts/CHIP_EVIDENCE.md) supplies concrete register, address,
timing, electrical and failure-case findings from TI documents and public code,
starting with TI's GitHub organization. The [source manifest](docs/reference/public-implementations.json)
pins 13 files from six repositories. The coder must verify each adopted claim
against actual sources, correct inherited mistakes and retain unresolved gates.

The prompts now include a [concrete engineering baseline](docs/prompts/ENGINEERING_BASELINE.md)
for API names, errors, health states, lifetimes and folder responsibilities, plus
a [CLI contract](docs/prompts/CLI_CONTRACT.md) with command and runner conventions.
[Maintainability rules](docs/prompts/MAINTAINABILITY.md) require simple code,
engineering English and documentation with every change, including source Doxygen.
[Agent guidance](docs/prompts/AGENT_WORKFLOW.md) permits delegation and model
selection according to task complexity and the active runtime's capabilities.

[AGENTS.md](AGENTS.md) defines the engineering and Git workflow. Start each new
promptset execution on a new branch and keep it throughout that execution;
commit and verify upstream sync after each prompt and
at coherent, checked units during a prompt. The lead handles Git for shared work.

The first implementation promptset uses focused local checks throughout. Remote
CI execution and status checking are confined to prompt 34; other prompts and
final-review fixes do not wait for another remote run. Existing merge/release
requirements remain separate from completing this initial implementation.

The supporting [audit](docs/audit/README.md) records source observations from
this repository's parent directory, `Projects/`.
The latest [correctness and completeness review](docs/audit/promptset-completeness-review.md)
records fixes to chip procedures, evidence handling, adapters, CLI and tooling.
`FieldCore-node` is the consumer inspected for the user's `fieldcore-noce`
reference. Its architecture informs integration requirements; it will not be a
library dependency.

The audit and promptset were prepared on **2026-10-23**. Sibling repositories were
inspected without changing them. Audit observations are not claims that their
builds, CI, or hardware tests passed during this task.

The implementation agent must keep native host, Arduino, native ESP-IDF, package,
CI, and physical hardware evidence separate. No implementation or hardware
validation is claimed by the existence of this promptset.
