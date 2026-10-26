# TUSB32x library development

This repository provides a portable C++17 library target with value types,
Status helpers and pure binding/profile validators. The device driver is still planned.
The requested starting device is **TI TUSB320LAI**. The
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
TI's current LI/HI DFP/DRP caveat remains visible. An independent prompt 2 review
clarified the TUSB322I VCONN supply
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

Prompt 5 implements the [public passive values](include/TUSB32x/TUSB32x.h),
[Status helpers](include/TUSB32x/Status.h) and fully documented settings,
callbacks and decoder-result types. The [API guide](docs/api-contracts.md)
separates these implemented values from future callable methods; the
[architecture](docs/architecture.md) keeps transport and scheduling application-owned.
[Reusable checks](test/README.md) cover independent headers, constexpr behavior
and linked consumers. Decoder logic, driver methods and package/version
metadata remain future work; no runnable device example is available yet.

Prompt 6 finishes the [owner/state contracts](docs/api-contracts.md): exact
admission/results, borrowed lifetimes, fresh owner time, retained write evidence,
snapshot publication and passive health. [Compile-only examples](test/contract/owner_examples.cpp)
show observation, explicit apply, cancellation and two devices on an external
owner. Independent integer-time vectors compile successfully; runtime engine,
adapter and hardware behavior still require their later implementation tests.

Prompt 7 adds the `TUSB32x` CMake target and callable `validateDriverConfig` /
`validateProfile` functions in [Config.h](include/TUSB32x/Config.h). They check
values without invoking transport, allocating or changing hardware. A valid
profile does not authorize gated writes; TUSB322I apply restrictions remain
specified for later implementation in the [API guide](docs/api-contracts.md).

Build and run the native checks with CMake 3.16 or newer and a C++17 compiler:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

For single-configuration generators, select Debug at configure time with
`-DCMAKE_BUILD_TYPE=Debug`. Tests default OFF. Applications can use
`add_subdirectory(path/to/TUSB32x)` and `target_link_libraries(app PRIVATE TUSB32x)`;
the target supplies its public include directory and C++17 requirement. The
[standalone host consumer](examples/host/basic/README.md) builds and runs without
Git, Python, Doxygen, framework headers or sibling include paths. Install/export
packaging and full owner examples remain later work.

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
CI, and physical hardware evidence separate. The current checks establish only
portable values, validators and linkage; they do not qualify a physical device or framework adapter.
