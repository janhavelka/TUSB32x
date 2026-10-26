# Prompt 7 current peer and build audit

Read-only peer review on 2026-10-26 (Europe/Prague). This records current source
inspection for the portable build and pure validators. No sibling was edited,
fetched or built; no remote CI was dispatched or queried. Local Git commands
were limited to reading HEAD, branch and status. All paths below are relative to
`C:/Users/Honza/Documents/Projects/` unless a TUSB32x-relative path is stated.

## Revision and dirty-state snapshot

`git rev-parse HEAD`, `git branch --show-current` and `git status --short` were
read independently in each checkout before writing this report.

| Repository | Full HEAD | Branch | Worktree at inspection |
| --- | --- | --- | --- |
| TUSB32x | `cdbb36cda836ef1cb7c4d66864d93c5ec2b31418` | `promptset/tusb32x-implementation-20261023` | Clean before prompt 7 edits |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `main` | Clean |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | `main` | Clean |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `main` | Clean |
| ADS1115 | `235af3e8c722661a89a44766a4ccd903a654f6fe` | `main` | Clean |
| INA3221 | `41b6184b75f3c152611d4236f7ea41a5ce77bc2c` | `main` | Clean |
| OPT4001 | `dab818e575fc25a0949d962bc0df600eb2ed5200` | `main` | Clean |

These are local source snapshots, not claims about upstream synchronization or
CI results. Earlier reports and the baseline's historical revision table were
pointers; the paths and current implementations below were reopened.

## Local contracts revisited

Read `Projects/AGENTS.md`, `TUSB32x/AGENTS.md`, `docs/prompts/README.md`,
`COMMON.md`, `7-host-build.md`, `MAINTAINABILITY.md`, `AGENT_WORKFLOW.md`, and
the baseline's source layout, test registration, typed settings and validator
contracts. Revisited `docs/api-contracts.md` validator/admission rows,
`docs/register-map.md`, `docs/variant-matrix.md`,
`docs/implementation-progress.md` and `docs/design-decisions.md`.

Current TUSB32x declarations and fixtures inspected:
`include/TUSB32x/{Config,Status,TUSB32x,CommandTable}.h`,
`test/test_compile_contracts.cpp`, `test/consumer/status_other_tu.cpp`,
`test/contract/PlannedApi.h` and `test/README.md`. At the starting snapshot there
was no `src/`, `examples/`, `tools/`, `scripts/` or `.github/` tree and no CMake
build. The existing Status/value check already has multiple translation units
and runtime checks independent of `NDEBUG`; preserve that evidence and register
it rather than inventing a facade stub. Planned owner signatures belong only in
the compile-only fixture until their real implementation stages.

## Current sources and selected conventions

| Concern | Exact current paths and inspected behavior | Adopt or reject for prompt 7 |
| --- | --- | --- |
| Small host target | `TMP1x2/CMakeLists.txt`; `24Cxx/CMakeLists.txt`: explicit source lists, `target_include_directories(... PUBLIC include)`, PUBLIC `cxx_std_17`, tests default OFF, real CTest registrations inside the test option | Adopt explicit sources, public language/include requirements and opt-in tests. Do not create future source units or use recursive source globs. |
| Framework boundary | Both CMake files use `if(COMMAND idf_component_register)` for their existing IDF support. `LDC1614/CMakeLists.txt`, `INA3221/CMakeLists.txt`, `OPT4001/CMakeLists.txt`, `ADS1115/CMakeLists.txt` are component-oriented; ADS1115 publishes C++11 | TUSB32x needs a real standalone C++17 target now. Existing component examples are evidence for later IDF integration, not a reason to require IDF commands or downgrade C++17. |
| Consumer dependencies | `LDC1614/CMakeLists.txt` unconditionally finds Python and executes `scripts/generate_version.py check` before registering its component | Reject this consumer-time dependency. Ordinary TUSB32x configure/build must work without Python, Git, Doxygen, PlatformIO or firmware SDKs. Metadata checks belong to explicit maintainer checks at their own stage. |
| Test dependencies and assertions | `TMP1x2/CMakeLists.txt` and `24Cxx/CMakeLists.txt` find Python only with tests enabled. `TMP1x2/test/test_core.cpp`, `TMP1x2/test/helpers/test_helpers.cpp`, `24Cxx/test/test_core/test_core.cpp` use explicit CHECK failure paths, not disabled Release assertions | Adopt active Release checks and genuine executable test registration. Current C++ tests do not need Python just because peers later register runner tests. Add only currently implemented suites. |
| Pure value validation | `ADS1115/include/ADS1115/Config.h` pure helper declarations; `ADS1115/src/ADS1115.cpp::validateDeviceProfile/validateComparatorProfile/validateChannelRequest`; `ADS1115/test/test_basic.cpp::test_owner_safe_profile_validation_boundaries` | Adopt explicit invalid-enum rejection before interpretation, separate field/cross-field checks and local Status results. ADC thresholds, modes, address range and comparator rules are device-specific and rejected as TUSB32x rules. |
| Transport versus profile | `TMP1x2/src/TMP1x2Configuration.cpp::validateConfig/validateSettings`; `TMP1x2/test/helpers/test_helpers.cpp::validationAndEncoding`; `ADS1115/include/ADS1115/Config.h::DriverConfig/DeviceProfile`; `INA3221/include/INA3221/Config.h` | Adopt separate binding/profile validation and counting callbacks proving zero transport calls. Do not copy TMP1x2's threshold normalization, GPIO requirements or timeout cap: exact TUSB32x frozen contracts govern. |
| Isolated CMake consumer | `TMP1x2/tools/check_package.py`: writes a renamed extracted library plus a parent CMake consumer, links its public target, builds Release and runs CTest. `24Cxx/tools/check_package_contents.py`; `24Cxx/.github/workflows/ci.yml`: inspect archive content, build extracted package with tests OFF | Adopt a clean ordinary consumer using only the public target and direct headers, and execute its checks. Full archive production/validation remains prompt 32; no package pass claim now. |
| Independent headers and linkage | `LDC1614/tools/check_clean_consumer_compile.py`: compile every public header independently, then compile/link and run a consumer with strict flags and no framework includes. `TMP1x2/test/core_no_arduino/compile_main.cpp`; `24Cxx/test/core_no_arduino/compile_main.cpp`; `OPT4001/test/core_no_arduino/compile_main.cpp` | Adopt isolated includes, constant/type contracts and actual linkage. A single umbrella include can conceal a missing include; several translation units also exercise header ODR/link boundaries. Do not claim a build-only fixture as a behavioral test. |
| Check evidence limits | `INA3221/tools/check_strict_compile.py`: strict GCC/Clang `-fsyntax-only` on one source. `INA3221/.github/workflows/ci.yml`: invokes it separately from native tests and package packing. `OPT4001/tools/check_clean_consumer_package.py`: temporary native PlatformIO package consumer, builds but does not execute it | Retain strict compilation but require link/runtime evidence separately. Do not describe syntax-only compilation, archive creation or a successful PlatformIO build as executed consumer behavior. |

## Workflows, examples and Python boundaries

Reopened `.github/workflows/ci.yml` in all six peers as source; no run status was
read. TMP1x2 and 24Cxx use Ubuntu 24.04 native CMake/CTest, strict GCC warnings
(`-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror`) and
AddressSanitizer/UndefinedBehaviorSanitizer. TMP1x2 has bounded CTest timeouts.
Use supported local equivalents and keep warnings private to owned targets;
compiler-specific GCC switches must not leak into unrelated consumers or MSVC.
Sanitizer evidence must come from an actual supported run, not copied flags.

Their checked-in framework jobs pin PlatformIO 6.1.19; TMP1x2/24Cxx list IDF
5.3.2/5.5.1/6.0.1 and S2/S3. LDC1614 currently lists IDF 6.0.2, ADS1115 uses
5.3.5, and INA3221/OPT4001 use 6.0.1. These are peer workflow inputs, not tested
TUSB32x versions or dependencies to introduce in prompt 7. The TUSB32x initial
promptset rule reserves remote CI work for prompt 34.

Relevant examples/CLI and runner boundaries reopened:

- `TMP1x2/examples/common/Tmp1x2Cli.h` and
  `24Cxx/examples/common/Eeprom24CxxCli.h` explicitly present example-only,
  framework-neutral command processors. Their application-owned platform hooks
  are excluded from the public core target.
- `LDC1614/examples/common/Ldc1614Cli.h` and
  `LDC1614/include/LDC1614/Config.h` retain the public/example separation.
- `ADS1115/examples/02_owner_safe_poll/main.cpp` places Arduino, Wire, bus mutex
  and scheduling in the application. Its one-turn callback/result design does
  not justify implementing a TUSB32x owner facade during this passive stage.
- `INA3221/examples/common/I2cTransport.h` is explicitly Wire example glue;
  `OPT4001/examples/common/CliShell.h` includes Arduino. Reject either directory
  as a transitive TUSB32x include/source dependency.
- `INA3221/examples/esp_idf/basic/components/INA3221/CMakeLists.txt` resolves its
  own repository root; do not add sibling checkout paths to TUSB32x consumers.
- Module entry/docs and relevant imports were reopened in
  `TMP1x2/tools/hil_tmp1x2_runner.py`, `24Cxx/tools/hil_runner.py`,
  `LDC1614/tools/ldc1614_hil_runner.py`, `ADS1115/tools/run_i2c_hil.py`,
  `INA3221/tools/hil_cli_runner.py`, `OPT4001/tools/hil_opt4001_runner.py`.
  These host serial tools and their parser tests remain separate from ordinary
  library compilation. No runner or hardware suite was executed for this audit.

Each peer's current `AGENTS.md` was reopened for layout, framework boundary,
callback ownership, tooling and verification rules. Those policies reinforce
source/example separation; sibling lifecycle/recovery/health details are not
TUSB32x contract authority. `LDC1614/Doxyfile` was also revisited for its
`EXTRACT_ALL=NO` and undocumented/parameter/error warnings. Useful documentation
checks remain explicit maintainer validation, with no Doxygen configure-time
dependency for a plain consumer.

## Validator and build recommendations

Keep the existing `Config.h` value vocabulary unchanged. Publish only the two
implemented namespace functions, `validateDriverConfig` and `validateProfile`,
with real definitions and complete no-callback documentation. Reject unknown
variant/address/enums and invalid binding as `INVALID_CONFIG`; check all defined
field values before reporting a valid but unsupported combination as
`UNSUPPORTED_FEATURE`. A valid null context is not a missing callback.

Profile validation checks semantic variant support without actual-device proof.
The frozen baseline explicitly separates it from apply-time changed-field gates:
TUSB322I's unresolved advertisement write must not reject every otherwise valid
profile or prevent unrelated supported changes when advertisement is unchanged.
Use the maintained variant/register tables for debounce, preferences, disable
controls and neutral placeholders; no sensor peer supplies those silicon facts.

Create one small explicit C++17 target and register the real existing
`tusb32x_compile_contracts` suite plus validator behavior. Build the existing
planned-contract examples only as compile fixtures, without publishing or
linking unimplemented facade calls. Exercise a separate tests-OFF consumer in a
source copy without its own Git metadata, including a copy under another Git
repository, to catch accidental parent identity/developer-tool discovery.

This audit proves inspected source choices only. Prompt 7 implementation,
configure/build/test commands, consumer execution and tool versions are recorded
by the lead in progress/validation; this report claims none of those outcomes.

## Independent prompt 7 review, 2026-10-26

Rechecked all six local revisions, branches and dirty states, then reopened the
listed current CMake, AGENTS, workflows, declarations, validators, tests,
examples/CLI, runners and consumer checkers. All six remain clean on main at the
full revisions in the table above. No source changed since the prompt 7 audit.
Historic reports supplied paths, not an exemption from this source inspection.

The existing choices remain appropriate: explicit PUBLIC C++17/includes and
tests OFF; strict private warnings; independent header and actual multi-file
link/run checks; no configure-time Python/Git/Doxygen dependency or copied sensor
restrictions. The current INA3221 syntax-only and OPT4001 build-only checks still
cannot establish runtime consumer behavior. The lead verified relevant source
bodies in TMP1x2, 24Cxx, LDC1614, ADS1115, INA3221 and OPT4001.

The peer/test reviewer also independently rebuilt and ran TUSB32x checks and a
clean tests-OFF consumer; those results are recorded in
[validation](../validation.md#independent-prompt-7-review-2026-10-26).
No sibling fetch, build or mutation, and no remote CI action, accompanied this
re-audit. This source review closes no firmware or physical qualification gap.
