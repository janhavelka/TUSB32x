# P00 — Re-audit the library ecosystem

Read and apply [COMMON.md](COMMON.md) and root/parent AGENTS.md. This stage is
research and decisions; do not copy a whole sibling repository as the driver.

Before a new implementation execution, follow AGENTS.md to create one new
execution branch and record its name/base commit. If resuming, verify and use the
recorded branch. Keep it through P14; commit and sync after this prompt and each
later prompt, including coherent completed units ready within a prompt.

## Start from the prepared decisions

Read [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md),
[CLI_CONTRACT.md](CLI_CONTRACT.md) and the dated reports in `docs/audit/`.
Your output is an evidence-backed confirmation or correction of those defaults,
not another list saying that an implementation agent should choose them later.

| Concern | Read these actual peer files | Confirm for TUSB32x |
| --- | --- | --- |
| Typed errors | `LDC1614/include/LDC1614/Status.h`, `LSM6DS3TR/include/LSM6DS3TR/Status.h`, `TMP1x2/include/TMP1x2/Status.h` | Structured Status, static messages, distinct operation/transport timeouts; no copied sensor-only errors |
| Owner methods | `LSM6DS3TR/include/LSM6DS3TR/LSM6DS3TR.h`, `ADS1115/include/ADS1115/ADS1115.h` | bind, start verbs, poll, cancel, retained result; exact selected signatures in baseline |
| Diagnostics | `LDC1614/include/LDC1614/LDC1614.h`, `LSM6DS3TR/include/LSM6DS3TR/LSM6DS3TR.h` | Facts without autonomous retry/admission policy |
| Familiar health names | `TMP1x2/include/TMP1x2/TMP1x2.h`, `TMP1x2/src/TMP1x2.cpp` | UNINIT/READY/DEGRADED/OFFLINE as optional projection, distinct from configuration trust |
| CLI rendering and grammar | `TMP1x2/examples/common/Tmp1x2CliOutput.cpp`, `SHT3x-main/examples/common/Sht3xCli.cpp` | Shared neutral processor, grouped help, strict values, cached/live differences |
| Machine command contracts | `LDC1614/tools/ldc1614_cli_contract.py`, `INA228/tools/run_i2c_hil.py` | Explicit effects, bounded commands, correlated evidence and exact traffic units |
| Version/package | `TMP1x2/scripts/generate_version.py`, `TMP1x2/tools/check_package.py`, `24Cxx/tools/check_package_contents.py` | Manifest truth, renamed extracted consumer, no local build prerequisites |
| Documentation | `LDC1614/Doxyfile`, `TMP1x2/tools/check_doxygen.py`, `INA228/.clang-format` | Missing docs detected, generated API visible, direct source style |
| Consumer boundary | `FieldCore-node/docs/guidelines/i2c_peripherals.md`, `FieldCore-node/include/TunnelMonitor/i2c/I2cDeviceBinding.h` | One external owner, borrowed buffers, copied results/effects, no product dependency |

Paths above are relative to the parent Projects directory. Verify existence and
current behavior; record renamed or changed references. Read nearby implementation
and tests rather than treating a declaration or old report as proof.

## Work

1. Enumerate the actual parent Projects directory. Identify independent I2C
   libraries using manifests and source, not names alone. Start with the seventeen
   libraries in COMMON; distinguish empty `EEPROM_24Cxx` from active `24Cxx`, and
   exclude SPI/single-wire projects, archives, dependency copies and worktrees.
2. Record each library's name, version, HEAD, branch, dirty state, author/license,
   include namespace, language standard, source organization and supported builds.
   Inspect AGENTS, manifest, CMake, PlatformIO, IDF metadata, CI and generated files.
   Do not fetch or mutate peers merely to research local conventions.
3. Audit actual callback/status/types/lifecycle implementations and corresponding
   tests. Compare TMP1x2/ADS1115/INA3221 with newer LDC1614/LSM6DS3TR and byte-register
   PCA9555/BME280. Separate common vocabulary from conflicting behavior. Capture
   bus ownership, timeout, partial-write, passive health, configuration trust,
   operation tokens, cancellation and result-retention contracts.
4. Audit common CLI, Arduino and native-IDF dispatch, help rendering, strict
   parsers, native fixtures, serial framing, Python runners, USB console behavior,
   CI registration and package consumers. Read executed code, not just README lists.
5. Inspect FieldCore-node's I2cTask, transport/binding/profile contracts, queue and
   operation progress/results, library-adapter boundaries and device diagnostics.
   Record exactly what a reusable library must allow without importing FieldCore.
   Also describe plain C++/bare-metal, Arduino and independent IDF consumers.
6. Create/update `docs/library-comparison.md` and `docs/design-decisions.md` with
   file/line evidence and reference choices per concern. Preserve dated initial
   audits. Call out stale guidance, unsafe defaults and missing tests explicitly.

## Deliver and check

- Inventory and convention/conflict matrix covering structure, maintenance and
  runtime ownership, API naming, status/health, examples/CLI, runners, tests,
  dependency pins, docs, licenses and release/version workflow.
- Choose one coherent baseline: portable raw diagnostics plus optional passive
  health projection, externally driven operations, shared CLI, clean consumers.
  Do not make OFFLINE an implicit retry or admission policy.
- Record available local Python/CMake/C++ compiler/PlatformIO/IDF tools without
  installing duplicates or reporting build support from PATH checks alone.
- Initialize `docs/implementation-progress.md`. Record which observations were
  source inspection and which checks actually ran. Missing peers do not invalidate
  already verified facts; list limits and continue supported work.

## Concrete report format and finish gate

For each material choice record `concern | observed peer/symbol | source revision |
selected TUSB32x rule | reason | check`. Label newly selected names as TUSB32x
decisions, not names allegedly shared by every peer. Keep one record per decision;
do not duplicate a complete source dump in the report.

Check these known differences deliberately: OPT4001 teardown writes hardware;
some peers gate work on OFFLINE; LDC/LSM diagnostics leave policy outside; peers
use different result-slot, time and counter-lifetime rules; shared directories
can still contain Arduino code; Doxygen EXTRACT_ALL can hide missing comments.
Explain which behavior the new library keeps and which it rejects.

Update the general README's planning/implementation state, the audit comparison,
design decisions and progress entry. Record any user-visible planning changes in
CHANGELOG Unreleased. There is no C++ Doxygen work in a source-free research stage;
state that explicitly, then carry the documentation rule into the next stage.
Use [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md) to split independent library subsets,
with the lead deciding the final common contract.
