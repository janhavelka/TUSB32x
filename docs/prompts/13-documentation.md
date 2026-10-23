# P13 — Reconcile public documentation and integration guidance

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 docs index,
variant/integration/validation guides, LDC1614 API/owner docs, INA228 CLI/HIL docs,
and current FieldCore consumer guidance. Describe the implementation that exists.

Read [MAINTAINABILITY.md](MAINTAINABILITY.md) and check the actual Doxygen/code
comments written during P02-P12. This stage reconciles them with the maintained
guides. It is not the first time documentation is written, nor permission to
generate large boilerplate comments from function names.

## Work

1. Replace the planning-only top-level README with accurate library documentation,
   retaining a link to this promptset and dated audit. Lead with supported devices,
   purpose, portable ownership and a short compiling example. Do not claim every
   TUSB32x chip or USB PD capability. Mark TUSB322I extension status per feature.
2. Complete a navigable docs index with architecture, API/status, transport, owner
   operations/time/effects, variant/register behavior, integration, CLI, testing,
   HIL and hardware limits, version/release and validation documents.
3. Public API docs explain ownership/lifetime, blocking bounds, callback timeouts,
   thread/ISR constraints, retained result behavior, bus effects, variant support,
   output-on-failure, clock requirements, trust and recovery. Document raw writes
   and W1C/reset semantics as carefully as typed setters.
4. Publish exact supported-device table, source citations and per-feature coverage
   linking API symbols, CLI routes and tests. Separate original/second/third-gen
   controls, addresses, revisions and enable polarity; distinguish configured
   variant from experimentally verified hardware.
5. Give complete wiring/startup examples: supply/current conditions, ADDR/PORT,
   EN polarity and unresolved timing wording, I2C pullups/back-power, VBUS_DET,
   optional INT. State what the board/application owns. No guessed pin assignment
   or universal low-current/source capability claim.
6. Explain current/orientation/accessory validity, requested vs actual local role,
   ordinary detach, stale observations, coalescing IRQs, non-atomic hardware
   snapshots, explicit acknowledgement and attached-current tracking uncertainty.
7. Include Arduino and native IDF build/run commands, generic host/owner example,
   clean package use, FieldCore-style adaptation and testing entrypoints. All
   snippets must compile or be clearly labeled pseudocode; test important snippets.
8. Reconcile AGENTS, CHANGELOG, CONTRIBUTING, license, component/library metadata,
   generated version, Doxygen, CLI help and runner docs. No copied chip names,
   contradictory grammar, stale path, fictitious remote or unrun PASS badges.
9. Finalize `docs/validation.md`: each check's command, environment, revision/dirty
   state, result, evidence path and limit; hardware per exact part/board/framework.
   Separate implemented, host verified, firmware linked and hardware validated.

## Concrete document ownership and content

| File | Required content and evidence |
| --- | --- |
| `README.md` | What the chip library does, supported/partial/excluded variants, plain-C++ quick start, dependencies, callback ownership, examples, limits and links |
| `CHANGELOG.md` | Unreleased and actual release entries grouped by user-visible additions/changes/fixes; no prompt-by-prompt diary or invented release |
| `CONTRIBUTING.md` | Local tool prerequisites, wrapper usage, focused/full check commands, test registration, docs/version workflow and review expectations |
| `AGENTS.md` | Current layout, selected core/CLI contracts, authorized delegation/model policy, actual commands and evidence requirements |
| `docs/architecture.md` | Core/adapter/CLI/owner responsibility diagram or short table; no separate service-framework architecture |
| `docs/api-contracts.md` | Canonical method names, Status meanings, operation/result delivery, preconditions, ownership, callback budgets and time domain |
| `docs/owner-operations.md` | Admitted/active/terminal-slot lifecycle, tokens, cancel/end, retained effects, delayed callback time and examples |
| `docs/register-map.md` / `docs/variant-matrix.md` | TI revision/page/table, encoding/access semantics, supported masks/registers and all variant limits |
| `docs/integration.md` | Plain C++, Arduino, native IDF and FieldCore-style mapping without product/framework core dependence |
| `docs/CLI.md` | Exact shared grammar/aliases/help topics, stable records, cached/live/mutation distinction, output bounds and examples |
| `examples/README.md` and local READMEs | Build environment, wiring/straps, startup, transport framing, USB console, commands and cleanup |
| `docs/testing.md` | Named native suites, fixtures, registration, fault matrices and concise commands with expected exit behavior |
| `docs/hil-runner.md` / `docs/hardware-validation.md` | Host runner usage, artifacts/provenance/verdicts, DTR/RTS, fixture setup and physical test criteria |
| `docs/feature-coverage.md` | Every supported feature -> API -> CLI -> test -> TI source; explicit gaps |
| `docs/validation.md` | What actually ran for which source/framework/board, outcomes and unavailable checks |
| `docs/design-decisions.md` | Short rationale for meaningful differences from peers; do not duplicate public contracts |

Use the selected baseline's names throughout. In particular, error Status,
operation outcome, HardwareEffect, configuration trust, PortSnapshot and optional
DriverState are different concepts. TIMEOUT and I2C_TIMEOUT mean different layers.
Successful `takeResult` reports delivery success; its copied result may contain a
failed operation. Document these with small concrete examples.

## Source Doxygen audit

Inspect every public header, core source, adapter and shared CLI source. Require
useful file/type/function/member documentation according to MAINTAINABILITY.
Check enum values and units/defaults; ensure callback docs say terminal, borrowed
buffers and aggregate timeout. Compare code with comments on zero-I2C methods,
raw access, staged/apply semantics, uncertain writes, IRQ ACK and cancel/end.

Read generated API pages, not only the Doxygen return code. Confirm the main
class, Status/Err, DriverConfig/DeviceProfile, operations, snapshots and diagnostics
are visible with parameter/return documentation. A public symbol silently hidden
by extraction settings is a failure. Run negative fixtures for a missing comment,
misspelled parameter, broken reference and missing public type. Fix the source;
do not globally enable extraction or disable warnings to mask omissions.

Comments beside W1C masks and mode/reset timing cite the exact TI source. Explain
why the code preserves applied RW fields during ACK and why polling cannot shorten
a delay. Remove generic claims such as safe, robust or thread-safe unless the
actual constraint is stated. State serialized-owner use instead of promising
thread safety that the driver does not implement.

## Example and language checks

Build the important C++ snippets against the real headers. Keep a small example
showing bind -> accepted start -> bounded poll -> takeResult -> inspect status and
effect. Code must not use `if (!startStatus)` as if IN_PROGRESS were rejection.
Use static messages in Status; no returned pointer to a temporary formatted buffer.

Keep one quick-start path for each platform, then link detailed cases. Describe
commands using the current grammar; avoid several conflicting copies of the
complete command list. Document defaults as defaults, hardware observations as
observations, and unverified behavior as unverified. Use simple engineering
English everywhere, including Doxygen warnings and runner report text.

Update README and CHANGELOG with the final user-visible changes and current
implementation state. A documentation-only correction still gets a focused link/
Doxygen check, not a fictitious firmware/HIL validation result.

## Tests and completion

- Doxygen and local links pass; package documentation avoids excluded references.
- CLI help/contract/runner grammar agree; examples and snippets build where claimed.
- Review feature coverage against the full P01 matrix: implement missing applicable
  features or explicitly record an evidence blocker. Do not quietly redefine scope.
- Record remaining uncertainty plainly without burying it in obsolete audit notes.
