# P00 — Re-audit the library ecosystem

Read and apply [COMMON.md](COMMON.md) and root/parent AGENTS.md. This stage is
research and decisions; do not copy a whole sibling repository as the driver.

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
