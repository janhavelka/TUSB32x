# Contributing

Read [AGENTS.md](AGENTS.md), [COMMON](docs/prompts/COMMON.md) and the requested
[prompt](docs/prompts/README.md) before work. Keep the existing execution branch
for this promptset. Sibling projects are read-only evidence. Use the
[engineering baseline](docs/prompts/ENGINEERING_BASELINE.md) for public names and
[maintainability rules](docs/prompts/MAINTAINABILITY.md) for code and documentation.

The [README](README.md) describes the implemented subset and
[documentation index](docs/README.md) links its contracts. Core is C++17 with
fixed storage and borrowed application callbacks; framework adapters, console
and runner policy stay outside it. Add callable declarations with their real
definitions and link checks. No hardware stubs, empty future files or competing
protocol/operation engines belong here.

## Changes and review

Keep each change small enough to review, with useful source documentation,
behavioral checks for affected contracts and a meaningful Unreleased entry.
The [test guide](test/README.md) maps runtime cases, independent byte expectations,
fault boundaries and compile-only checks. The [owner guide](docs/owner-operations.md)
explains admission, fresh time, delivery versus outcome, retained effects and
shutdown. Refer to these contracts instead of copying diverging state tables.

Use [AGENT_WORKFLOW](docs/prompts/AGENT_WORKFLOW.md) for delegation and model
selection from the current runtime. Assign disjoint write scopes; independent
review may read overlapping files. The lead owns shared decisions, integration
and Git. Inspect status, staged changes and upstream before editing; preserve
concurrent work. Routine coherent commits and pushes are authorized. Synchronize
by fetch, safe integration if needed, push and final fetch, then verify equal
local/upstream commits and zero ahead/behind. Do not rewrite shared history or
create a branch per prompt.

## Local checks

Use CMake 3.16+ and a C++17 compiler. From the repository root:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Repeat affected checks in a separate Release directory, selecting Release at
configure/build/test time. Strict warnings are private to library/test targets;
runtime checks and assertions stay active in Release. All 74 Windows CTests
(73 elsewhere) have finite timeouts and one validation owner. Independent
header/signature compilation is build evidence, not a passing behavioral test.
Tests ON require Python 3.10+; the mailbox fixture also uses host threads.

The combined local release entry point is:

```text
python tools/validate.py --checks all
```

Accepted selections are `native`, `contracts`, `doxygen`, `python`, `package`,
`arduino`, `idf-s2` and `idf-s3`; comma-separated subsets report their narrower
scope. Debug and Release are the default host configurations. The validator
reuses disjoint CTest groups, verifies inventory and executed JUnit cases, and
saves bounded logs and partial reports. Missing required tools mean INCOMPLETE,
not a skipped PASS. See [prerequisites, exits and evidence](docs/validation.md#local-validation-entry-point).

Run focused negative fixtures before a combined check; do not immediately repeat
its complete suite through another wrapper. In particular, `hil_runner` already
loads the session/fixture modules once. Do not add broad unittest discovery over
those modules. Ordinary tests and validation never open a hardware port, flash,
publish or request remote CI. Physical plans require separately authorized,
identified fixtures; their instructions and cleanup limits are in the
[runner guide](docs/hil-runner.md).

After public/build changes, also build and run the
[ordinary host consumer](examples/host/basic/README.md) with tests OFF. Consuming
core requires no Python, Git, Doxygen, PlatformIO, framework or sibling paths.
Use supported host sanitizers for relevant changes; they do not apply to SDK
builds or establish physical timing.

## Source and generated API documentation

Document public files, types, enum values, members and functions when changing
them. State units, valid ranges/defaults, borrowed lifetimes, preconditions,
output-on-failure, status meaning and callback/effect bounds. Explain non-obvious
private invariants concisely. Cite exact TI revisions/sections/tables beside
mixed W1C/reset masks and silicon waits. Python modules and exported helpers use
ordinary docstrings for inputs, returns/exits, deadlines, files and side effects.

For a focused documentation check use:

```text
python tools/check_doxygen.py --out build/doxygen-check
ctest --test-dir build/native -C Debug -R "^doxygen_negative$" --output-on-failure
```

The guard invokes real Doxygen and checks HTML/XML visibility, local links,
generated boundaries and source/example helpers. Inspect generated API pages,
including parameters and returns; a quiet run with hidden APIs is a failure.
`doxygen Doxyfile` is the direct renderer, not the complete guard. Keep
`EXTRACT_ALL=NO` and warnings enabled. Fix omissions rather than hiding public
symbols, suppressing diagnostics or publishing private implementation helpers.
The existing negative fixtures must reject missing public/enum comments, wrong
parameters, broken references, hidden public APIs and published private core types.
Private extraction exclusions must still leave file and helper comments checked.

## Version, packages and firmware

`library.json` is version truth. Use:

```text
python scripts/generate_version.py sync
python scripts/generate_version.py check
```

For an intentional version change, use `bump patch|minor|major` or
`set <semver>`, then check. The generator owns Version.h and the managed
CMake/component/Doxygen fields. Never hand-edit generated headers or bump a
version merely to finish a prompt. Stable library text has no volatile firmware
identity; example builds generate that identity separately. Generator fixtures
remain in the existing `tusb32x_version_sync` suite.

On Windows use `scripts/pio.cmd` with the installed VS Code-managed Core.
Do not install another PlatformIO Core. The
[development guide](docs/development.md) owns supported tool/SDK pins, version
exits and package commands. The [Arduino guide](docs/arduino-examples.md) and
[native application guide](examples/esp_idf/basic/README.md) own wiring/readiness
and diagnostic build commands. Native adapter audit rows are exactly IDF 5.5.5
and 6.1.0, with separate absolute build and SDKCONFIG paths per target/version.

For package changes run the registered package suites, then pack and check a
real managed-Core archive:

```text
scripts\pio.cmd pkg pack . --output build/TUSB32x.tar.gz
python tools/check_package.py --archive build/TUSB32x.tar.gz --out build/package-check
```

Keep PlatformIO/IDF export lists literal and identical. Preserve licenses,
shared/frontend sources and both example identity scripts; exclude vendor
artifacts, local sdkconfig/locks, caches, logs and private evidence. Accept only
trusted locally packed sources: the checker validates bounded portable archive
paths/content, then builds a renamed external consumer; it is not a CMake sandbox.
Inspect compiler source/header origins and actual execution, not source lists
alone. Core consumption uses `add_subdirectory()` and target `TUSB32x`; no
install/export interface is claimed.

## Checkpoint and release evidence

The [declared matrix](docs/development.md#declared-ci-matrix) records five Linux
host configurations, separate Python/docs/package ownership and eight extracted
Arduino/native-IDF image links. The completed prompt-34 audit passed 14 jobs at
`c50f8ddda8809425ede2161586466c95fa650ae6`,
[run 40782369910](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910).
Later changes use local evidence and do not inherit that exact-SHA result.
The initial promptset's remote checkpoint belongs only to prompt 34; preserve
existing hooks and merge/release protections.

Before editing CI run `python tools/check_ci.py`; it requires the pinned
source-checkout parser `PyYAML==6.0.3`. Its malformed-workflow fixtures already
belong to `validate_runner`. Workflow files alone establish no successful link.
Extracted firmware links may satisfy package and firmware gates for the same
revision; do not add an identical source-checkout matrix merely to repeat proof.

Before an authorized release, select its version intentionally, run the declared
local release checks and validate the exact archive and supported extracted
image rows. Preserve actual commands, tool versions, source revision/dirty state,
artifact identity and limits in [validation](docs/validation.md), and update
[progress](docs/implementation-progress.md) with coherent commit/sync evidence.
Host tests, linked images, remote CI and exact-board physical qualification
remain separate. Project ownership/license metadata and vendor terms must stay
accurate. Package checks do not authorize tags, registry uploads, releases,
flashing or board-power actions.
