# Contributing

Read [AGENTS.md](AGENTS.md) and the relevant [engineering contracts](docs/prompts/README.md)
before work. The original [implementation prompts](docs/archive/prompts/README.md)
are completed history. Use the [engineering baseline](docs/prompts/ENGINEERING_BASELINE.md)
for public names and [maintainability rules](docs/prompts/MAINTAINABILITY.md) for
code and documentation. Sibling projects are read-only references.

The [README](README.md) describes current support and the
[documentation index](docs/README.md) links the maintained guides. The core uses
C++17, fixed storage and borrowed application callbacks. Keep framework adapters,
console and runner policy outside it.

## Changes and review

Keep each change small enough to review, with useful source documentation,
behavioral checks for affected contracts and a meaningful Unreleased entry.
The [test guide](test/README.md) maps runtime cases, independent byte expectations,
fault boundaries and compile-only checks. The [owner guide](docs/owner-operations.md)
explains admission, fresh time, delivery versus outcome, retained effects and
shutdown. Refer to these contracts instead of copying diverging state tables.

Inspect status, staged changes and upstream before editing; preserve concurrent
work. Routine commits and pushes are authorized. Fetch, integrate safely, push,
then fetch again and verify equal local/upstream commits with zero ahead/behind.
Do not rewrite shared history. For delegated work, follow
[AGENT_WORKFLOW](docs/prompts/AGENT_WORKFLOW.md): assign separate write scopes and
leave integration and Git to the lead.

## Local checks

Follow [software validation setup](docs/software-validation-setup.md) for tool
installation and checks that need no board. The basic host commands are:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Repeat affected checks in a separate Release directory. Runtime assertions stay
active in Release. Compile-only checks do not establish runtime behavior.

The combined local release entry point is:

```text
python tools/validate.py --checks all
```

Use `--checks` with `native`, `contracts`, `doxygen`, `python`, `package`,
`arduino`, `idf-s2` or `idf-s3` for focused work. Debug and Release are the default
host configurations. Missing required tools mean INCOMPLETE, with a nonzero exit.
See [validation](docs/validation.md#local-validation-entry-point) for logs and exits.

Run focused checks after changes and avoid repeating the same suite through
multiple wrappers. `hil_runner` already loads the session/fixture tests; do not
add broad unittest discovery over them. Ordinary tests never open a hardware
port, flash, publish or request remote CI. Physical work needs separate
authorization and the setup described in the [runner guide](docs/hil-runner.md).

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

The guard runs Doxygen and checks public visibility, links, generated files and
helper comments. Inspect the generated API pages as well. `doxygen Doxyfile`
renders documentation without the additional guard checks. Keep `EXTRACT_ALL=NO`
and warnings enabled; fix documentation instead of hiding symbols or warnings.

## Version, packages and firmware

`library.json` is version truth. Use:

```text
python scripts/generate_version.py sync
python scripts/generate_version.py check
```

For an intentional version change, use `bump patch|minor|major` or
`set <semver>`, then check. The generator owns Version.h and the managed
CMake/component/Doxygen fields. Never hand-edit generated headers or bump a
version for routine maintenance. Stable library text has no volatile firmware
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

## CI and release evidence

The [declared matrix](docs/development.md#declared-ci-matrix) records five Linux
host configurations, separate Python/docs/package ownership and eight extracted
Arduino/native-IDF image links. The recorded CI checkpoint passed 14 jobs at
`c50f8ddda8809425ede2161586466c95fa650ae6`,
[run 40782369910](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910).
That result applies only to the recorded commit. Follow the current CI policy
in [AGENTS.md](AGENTS.md) and preserve hooks and merge/release protections.

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
