# Prompt 29 current peer audit

Read-only source review on 2026-10-28 for finite observation plans and native
runner integration. This agent owns only this report. No sibling edits, builds,
hardware access, remote CI inspection or Git mutations were performed.

## Entry revisions and rules

Read `git rev-parse HEAD` and `git status --short` independently in each checkout.
All seven worktrees were clean at audit entry; TUSB32x also had no staged diff.
Paths below are relative to the named checkout under
`C:/Users/Honza/Documents/Projects/`.

| Checkout | Full HEAD |
| --- | --- |
| TUSB32x | `aca463158485d13fe8d4437e1962dda4a4c32516` |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` |
| MB85RC | `1bf7c293549b1038b2d8f3b7a8d655fa74201e6e` |
| RV3032-C7 | `aabd8d9204e75072403fefb60cf8d2679a7f56de` |

TUSB32x remains on `promptset/tusb32x-implementation-20261023`. The lead owns
fetch, upstream comparison and synchronization. This report records local source
state, not independent proof of remote equality or peer test success.

Read parent/local `AGENTS.md`, each peer's root `AGENTS.md`, promptset index,
COMMON, prompt 29, AGENT_WORKFLOW, MAINTAINABILITY, current progress/decisions,
and the runner/observation/register sections of CLI_CONTRACT and
ENGINEERING_BASELINE. The TUSB32x prompt-34-only remote CI exception applies.
The current TUSB32x checkout has no `.github/workflows/` directory; peer workflow
definitions were inspected as source only.

## TMP1x2: finite plans, fresh samples and bounded reporting

Inspected `include/TMP1x2/TMP1x2.h`, `src/TMP1x2Operations.cpp`,
`examples/common/Tmp1x2Cli.cpp`, `examples/common/Tmp1x2CliOutput.cpp`,
`tools/hil_tmp1x2_runner.py`, `tools/test_hil_tmp1x2_runner.py` and
`.github/workflows/ci.yml`.

- Adopt pre-open argument validation, explicit target/model identity, finite
  positive time windows, bounded suite/count selection and dry-run hardware
  NOT_RUN. Its parser limits stress count to 1..100000, soak to 604800 seconds
  and suites to 32. TUSB32x must use its own documented interval/count/whole-run
  limits and checked arithmetic rather than inherit those numbers accidentally.
- Adopt the soak summary's aggregate counters, bounded retained history and
  streamed transcript. `test_long_soak_aggregates_without_per_pass_result_growth`
  models 12000 batches without one retained result per passing command.
- Adopt stopping on new console loss, failed checks or incomplete batches.
  Tests named `test_soak_stops_on_failure_or_output_loss_without_recovery`,
  `test_soak_rejects_hidden_temperature_excursions_and_stale_batch_evidence`,
  `test_interrupted_soak_does_not_pass_a_partial_duration_or_batch` and
  `test_last_soak_evidence_write_failure_cannot_leave_a_pass_summary` exercise
  these failures with deterministic injected sessions.
- Adopt checking advancing acquisition evidence independently of acceptable
  decoded values. Reject copying temperature ranges, conversion rules or the
  assumption that an acquisition must change attachment/current values.
- Reject firmware `stress_mix` scheduling and restoration through recover/typed
  hardware settings. TUSB32x observation loops belong on the host; cleanup restores
  captured local staging/presentation/auto values only. Also strengthen duration
  handling: the peer checks its duration between complete batches; TUSB32x needs
  an explicit finite whole-run deadline that bounds command work too.
- Its workflow runs CMake/CTest with strict warnings and ASan/UBSan, plus separate
  documentation, Arduino and native IDF jobs. Reuse local test registration and
  evidence separation; do not import its SDK matrix or claim it ran here.

## 24Cxx: persistent actual-CLI integration

Inspected `include/EEPROM24Cxx/EEPROM24Cxx.h`,
`examples/common/Eeprom24CxxCli.cpp`, `test/cli_host/main.cpp`,
`tools/test_cli_transcript.py`, relevant runner/runner-test paths in
`tools/hil_runner.py` and `tools/test_hil_runner.py`, `CMakeLists.txt` and
`.github/workflows/ci.yml`.

- Adopt executing real shared C++ CLI code through a host fixture and feeding its
  output to production runner validators. `capture()` executes a bounded process
  with nonce-like case markers; the readonly and scratch plans use actual emitted
  records rather than hand-written successful text.
- TUSB32x should strengthen that pattern by retaining one persistent process
  through its full readonly plan, including changed local state, owned results,
  failed callback scripts and restoration. Its existing `PersistentHost`/Session
  integration is the starting point; no second transcript registration is needed.
- Adopt passing the built executable path from CTest and finite process/test
  bounds. Peer CMake registers `hil_runner` and `cli_transcript`; its workflow
  runs those through CTest under strict native compiler flags.
- Reject EEPROM scratch preimages, CRC restoration, write-cycle waits, current
  address reads, firmware watch/stress schedulers and physical-attempt vocabulary
  as TUSB32x rules. A 24Cxx ACK is not identity; TUSB32x must check selected family
  identity while keeping exact-variant/board assumptions explicit.

## LDC1614: requested gates and truthful verdicts

Inspected `include/LDC1614/LDC1614.h`, `src/LDC1614.cpp`,
`examples/common/Ldc1614Cli.cpp`, `tools/ldc1614_hil_runner.py`,
`tools/test_ldc1614_hil_runner.py` and `.github/workflows/ci.yml`.

- Adopt explicit distinction between absent evidence, observed failure and a
  completed pass. `overall_status()` rejects empty commands/transcript;
  `combine_requested_gate_status()` prevents an explicitly requested NOT_RUN gate
  from leaving a PASS. Its tests cover requested optional gate failure, bounded
  repeats/dry-run, missing base expectations and partial soak cycles.
- Adopt separate host and firmware identities, retaining partial evidence through
  disconnect/context-exit errors, and refusing to count partial soak cycles.
  Tests include `test_requested_values_cannot_be_satisfied_by_stale_echoes`,
  `test_mid_soak_exception_counts_no_partial_cycle` and
  `test_serial_context_exit_after_soak_keeps_complete_evidence_and_fails`.
- Reject literal peer status names and exit numbers as a replacement for the
  implemented TUSB32x artifact/session classifications. Also reject permissive
  expected-token additions as proof of typed TUSB32x success. Every expected
  machine record must satisfy the existing exact schema and correlation rules.
- Reject LDC's result queue and device-side run/session machinery: TUSB32x owns
  one active operation and one retained result. The peer workflow's Python runner
  tests are useful host evidence; its linked targets and physical channel
  acquisition assumptions do not qualify TUSB32x hardware.

## INA228: exact callback budgets and owner exclusion

Inspected `include/INA228/INA228.h`, `src/INA228.cpp`,
`examples/01_basic_bringup_cli/main.cpp`, `test/test_basic.cpp` and
`.github/workflows/ci.yml`.

- Adopt counter deltas around an isolated zero/one-budget poll and explicit
  rejection checks. `test_cooperative_initialize_budget_order_forwarding_and_alert_determinism`
  checks exactly one callback per poll and the full ordered register history.
- Adopt separate cache access from hardware ownership. The test
  `test_active_job_excludes_other_hardware_apis_but_allows_cache_access` checks
  BUSY with unchanged counters, cached snapshots during work, and rejection while
  a terminal result remains unconsumed.
- Adopt foreign-owner exclusion even for token zero. The test
  `test_legacy_sample_step_does_not_adopt_owner_job` checks active and retained
  foreign work, exactly-once take and token-zero ownership.
- TUSB CLI auto polling creates an additional owner-turn effect: exact cache or
  parser-rejection deltas require idle state or `job auto 0`. Test an already
  admitted operation advancing once per turn under cached queries separately.
  Do not conflate handler callbacks, core callbacks and backend transactions.
- Reject INA initialization's configuration writes and alert semantics for
  readonly TUSB initialization. The peer workflow includes native tests and
  static owner/CLI/parser checks; source registration alone is not passing evidence.

## Optional strict-parsing comparison

Read MB85RC `tools/hil_runner.py`, `tools/test_hil_runner.py`,
`examples/common/CliShell.h` and `.github/workflows/ci.yml`; read RV3032-C7
`tools/hil_cli_runner.py`, `tools/test_hil_cli_runner.py`,
`examples/common/CommandHandler.h` and `.github/workflows/ci.yml`.
Adopt duplicate-free complete health records, full-match payload parsing and
refusal to pass expected text without completion. Reject substring-only positive
tokens, `allow_unknown` as successful evidence and peer reconnect/retry policy.
Keep TUSB32x's typed records and failed-session/no-replay rules.

## TUSB32x constraints checked against current code

Revisited `include/TUSB32x/TUSB32x.h`, `src/TUSB32xProtocol.cpp`,
`src/TUSB32xDiagnostics.cpp`, `src/TUSB32xOperations.cpp`,
`examples/common/Tusb32xCli.cpp`, `examples/common/Tusb32xCliOutput.cpp`,
`tools/tusb32x_hil_runner.py`, `tools/test_cli_transcript.py`,
`test/CMakeLists.txt`, `docs/register-map.md` and `docs/variant-matrix.md`.

- Port freshness needs matching completed refresh evidence, an advancing
  nonsaturated sequence, a valid acquisition time, `invalidated=0` and requested
  field-validity bits. `observedUptimeMs` is acquisition-boundary owner time, not
  publication time. A failed refresh preserves old data/time. Repeated detached
  observations are legal; no USB PD contract, VBUS policy or measured-load claim
  follows from a CC indication.
- Readonly staging must never apply settings to satisfy verify/selfcheck aliases.
  Those routes use `startVerifyProfile` and require retained desired state;
  preserve or test the missing-precondition rejection explicitly.
- Finite raw observation uses ordinary scalar `reg` commands. Current access
  validation permits 00..0A for every bound variant; isolated 45 is absent on
  TUSB322I; isolated A0 is absent on original TUSB320/I. Respect the existing
  capability table, not a copied universal register dump.
- A raw read may call `observeControl` for 08..0A/45 and change observed trust.
  It does not publish the fixed 09/08/09 port snapshot. Document possible silicon
  read effects, especially unresolved 322I latch wording, using the maintained
  [register guide](../register-map.md). No readonly route may ACK/reset/recover or
  use a write to establish read safety.
- Retain Session's one admitted token, original request and immutable deadline.
  Querying a result is not a second completion. Take each matching terminal
  result before the next operation; never cancel/take another owner's work.
  Lost framing/restart retains unrestored-state evidence without reconnect replay.
- Keep the single `hil_runner` and `cli_transcript` registrations with explicit
  built host path and 30-second CTest timeouts. Pure parser/fake-clock tests,
  actual C++ CLI integration, firmware links and physical serial/I2C/CC
  qualification remain separate evidence layers.

## Audit validation and limits

Report validation checks UTF-8 without BOM, LF/final newline, trailing whitespace,
relative Markdown links and exact existence of cited source paths. These are
documentation/source checks only. No peer test or build was executed. Prompt29
implementation, validation, commits and synchronization remain the lead's work;
this audit does not declare them complete. Physical hardware remains NOT RUN.
