# 1 — Audit the current library ecosystem

Read root and parent AGENTS.md, [COMMON](../../prompts/COMMON.md), the promptset index,
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md) and
[AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md). This is the first execution prompt. Its
complete result is a current inventory and a chosen engineering baseline; it
does not implement the driver or copy a sibling repository.

## Start and scope

Before a new execution, synchronize the intended base, create the execution
branch under AGENTS.md and record its name and base commit. If resuming this
execution, verify the recorded branch instead. Keep that branch through prompt
38 and every resumed session. Only the lead changes Git state.

Use the dated `docs/audit/` reports as starting evidence. Confirm or correct their
decisions now; do not leave another list of choices for an implementation agent
to invent. Subsequent prompts perform focused updates, not this full inventory
again. Research is read-only in sibling projects; do not fetch or modify them.

## Current source audit

Enumerate the parent Projects directory and identify independent I2C libraries
from manifests and code. Start with TMP1x2, 24Cxx, INA228, INA3221, ADS1115,
OPT4001, LDC1614, BME280, SHT3x-main, LSM6DS3TR, MB85RC, MCP45HVX1, PCA9555,
RV3032-C7, SCD41, SSD1315 and TCA9548A. Distinguish empty EEPROM_24Cxx from active
24Cxx. Exclude archives, dependencies, worktrees and non-I2C projects.

For each library record version, HEAD, branch, dirty state, author/license,
namespace, language level, source layout and supported builds. Inspect current
AGENTS, manifest, CMake, PlatformIO, IDF metadata, CI and generated-file rules.
Read actual implementations and tests beside declarations. Compare at least two
useful peers for each material choice; record changed or missing paths.

| Concern | Concrete starting files beneath Projects | Decision to confirm |
| --- | --- | --- |
| Status and errors | LDC1614, LSM6DS3TR and TMP1x2 `include/<name>/Status.h` | Structured Status, static messages, distinct owner/transport timeouts |
| Owner API | LSM6DS3TR and ADS1115 main headers, matching implementation/tests | bind/start/poll/cancel, tokens, retained result, bus-silent teardown |
| Health and trust | TMP1x2 main header/source; LDC1614 and LSM6DS3TR diagnostics; INA3221 owner tests | UNINIT/READY/DEGRADED/OFFLINE as passive projection; independent configuration/effect facts |
| CLI | TMP1x2 `examples/common/Tmp1x2CliOutput.cpp`; SHT3x-main shared CLI | Portable dispatch, grouped help, strict input, cached/live distinctions |
| Runners | LDC1614 `tools/ldc1614_cli_contract.py`; INA228 `tools/run_i2c_hil.py` | Fresh correlation, identity, bounded output and honest traffic units |
| Version/package | TMP1x2 version generator/package guard; 24Cxx package guard | Manifest truth, extracted consumer, no downstream tool dependency |
| Documentation/style | LDC1614 Doxyfile; TMP1x2 documentation checker; INA228 formatting | Missing comments detected, visible public API, simple source style |
| Consumer | FieldCore-node I2cTask, I2cOwnerTransport, I2cDeviceBinding, I2cLifetime and I2C guidelines | External ownership without a FieldCore dependency |

## Finish the engineering choices

1. Compare lifecycle, callback lifetime, aggregate timeout, partial writes,
   cancellation, token/result retention and counter lifetimes. Separate common
   vocabulary from differing behavior. Inspect byte-register peers PCA9555 and
   BME280 without copying their unrelated silicon behavior.
2. Inspect shared CLI dispatch/help/parsing, Arduino and native-IDF adapters,
   console bounds, Python runner framing, tests and actual CI registration.
   A shared folder can still contain framework code; verify its dependencies.
3. Confirm plain C++, bare-metal, Arduino, native IDF and FieldCore-like owners
   can use the chosen callback API. The core owns no bus, scheduler or USB policy.
4. Explicitly reject unsuitable peer behavior: OPT4001 teardown hardware writes,
   OFFLINE admission/retry policy, incompatible result/time rules, copied sensor
   features, and Doxygen EXTRACT_ALL masking missing comments.
5. Record available compiler, CMake, Python, PlatformIO and IDF tools. A PATH
   check proves availability only; it does not prove firmware builds or hardware.

## Deliver, review and commit

Update `docs/library-comparison.md` and `docs/design-decisions.md`. For each
material decision use `concern | peer file/symbol | revision | selected rule |
reason | check`. Preserve dated initial reports and avoid complete source dumps.
Cover ownership, status/health, structure, examples/CLI, runners, tests, pins,
dependencies, documentation, license and release/version workflow.

Initialize `docs/implementation-progress.md` with branch/base, completed scope,
audit sources, decisions, checks and evidence gaps. Review root README and update
its planning state where needed; record user-visible changes in CHANGELOG
Unreleased. There is no source Doxygen to add in this source-free research step.

Optional delegation: use general coding agents for disjoint library subsets and
a strong reasoning reviewer for ownership conflicts. A fast agent may collect
metadata. Give exact read-only peer scopes; the lead selects the common contract,
reviews evidence and edits shared decisions. Follow AGENT_WORKFLOW.

Check local links, evidence paths, inventory exclusions and consistency with the
baseline. Resolve local contradictions and label missing evidence explicitly.
Commit and sync this complete audit, including coherent checked units during the
work when useful. Verify upstream equality and report branch/commit/sync evidence
before moving to prompt 2; follow AGENTS.md for real synchronization limitations.
