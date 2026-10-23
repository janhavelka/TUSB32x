# Primary I2C library audit

Audit date: 2026-10-23. Scope: actual local source, examples, tests, runners,
manifests and CI in TMP1x2, 24Cxx, INA228, INA3221, ADS1115 and OPT4001.
These repositories were inspected read-only. No fetch, build, test, flash or
hardware action was performed. A configured CI job is not a passing CI result.
Paths and line numbers below refer to these local snapshots and may move.

## Observed local snapshots

All six were clean on `main` at inspection. This establishes local source state,
not remote synchronization or the identity of flashed firmware.

| Repository | HEAD |
| --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` |
| INA3221 | `e45622dbaa5bb0b3151d980f30a1b6ea1cdb1a1b` |
| ADS1115 | `381236ebfed813cd7a9aca34520ce031d8a0ff6b` |
| OPT4001 | `dab818e575fc25a0949d962bc0df600eb2ed5200` |

The shared `Projects/AGENTS.md` requires inspecting CI before implementation,
focused local checks, and actual final-commit remote CI evidence for major
changes. Each library's own AGENTS.md also describes a single existing branch,
preserving concurrent changes, and lead-owned Git mutations. Those sibling
files govern those repositories; they are useful conventions, not automatic
instructions to alter a new repository or push research documentation.

## What is consistent

| Aspect | Evidence | Recommendation for TUSB32x |
| --- | --- | --- |
| Public layout | `TMP1x2/AGENTS.md:42`, `INA228/AGENTS.md:71`, `INA3221/AGENTS.md:54`, actual `include/<Library>/` trees | `include/TUSB32x/{TUSB32x,Config,Status,CommandTable,Version}.h`, `src/`, `examples/`, `test/`, `tools/`, `scripts/`, `docs/`; add further files only for a concrete responsibility |
| Framework boundary | `TMP1x2/include/TMP1x2/TMP1x2.h:122`; `INA3221/include/INA3221/INA3221.h:283`; `ADS1115/include/ADS1115/Config.h:166` | C++17 core without Arduino/IDF/FreeRTOS, logging, bus handles, task creation, pin initialization or heap allocation in normal paths |
| Status | `TMP1x2/include/TMP1x2/Status.h:40`; every peer has `Status.h` | `Status { Err code; int32_t detail; const char* msg; }`, static strings, `ok()`, `is()`, `inProgress()`, explicit bool, `Ok()`/`Error()`; no exception or bool-only loss of transport evidence |
| Naming | Actual peer headers; `ADS1115/AGENTS.md` naming section | Namespace/library family name `TUSB32x`, class `TUSB32x`; camelCase methods/fields, `_camelCase` members, uppercase enum values and public register constants. Local constants may use the chosen peer style consistently |
| Application ownership | `ADS1115/include/ADS1115/Config.h:13`, `INA3221/include/INA3221/Config.h:13`, `24Cxx/include/EEPROM24Cxx/Config.h:6` | One owner controls bus setup, handles, pins, speed, serialization, scheduling, timeouts and recovery; callback contexts outlive binding; no reentry or ISR calls |
| Physical transfer contract | `ADS1115/include/ADS1115/Config.h:13`, `TMP1x2/include/TMP1x2/Config.h:8` | Each callback is one synchronous terminal physical attempt with exact lengths and a timeout including lock acquisition; no secret retry or recovery; borrowed buffers cannot escape |
| Cooperative ownership | `TMP1x2/include/TMP1x2/TMP1x2.h:182`, `ADS1115/include/ADS1115/ADS1115.h:226`, `INA3221/include/INA3221/INA3221.h:300` | Bus-silent bind/admission/cancel/result consumption/teardown; only owner-authorized polls run production I2C; one active operation and retained terminal result per instance |
| Health | `TMP1x2/include/TMP1x2/TMP1x2.h:10`, `TMP1x2/src/TMP1x2.cpp:120`, `ADS1115/include/ADS1115/ADS1115.h:16` | Four passive transport-health states: UNINIT, READY, DEGRADED, OFFLINE; count physical tracked attempts exactly once; logical validation, not-ready, cancellation and deadline expiry are separate |
| Trust versus transport | `ADS1115/include/ADS1115/ADS1115.h:24`, `INA3221/include/INA3221/INA3221.h:229`, `TMP1x2/src/TMP1x2.cpp:163` | Keep configuration certainty, last observation validity, interrupt-event retention and operation state separate from transport health |
| Version source | `TMP1x2/AGENTS.md:55`, `ADS1115/AGENTS.md` version section; `scripts/generate_version.py` in all peers | `library.json` is authoritative; generate and track `Version.h`, synchronize IDF and Doxygen metadata, check drift in CI; Git identity belongs in example build metadata |
| Packaging | `TMP1x2/library.json:9`, `TMP1x2/tools/check_package.py:54`, `24Cxx/.github/workflows/ci.yml` package job | Build from an extracted package in a clean consumer; no sibling, `.git`, generated local build or test dependency in ordinary library use |

Use these shared contracts while deriving all register/timing/electrical facts
from the actual TI source. A common family name must not imply every TUSB32x
part has the same capabilities or register behavior.

## Owner API details worth preserving

TMP1x2 offers the clearest small-family reference. Its public owner API at
`include/TMP1x2/TMP1x2.h:182` specifies admission without I2C, a nonzero token,
rejection without changing the caller token, cancellation without hardware
access, retained results and bounded `poll(nowMs, maxTransfers)`. Lines 198-204
specify zero budget, no sleep/yield/retry, remaining-deadline timeout clamping,
and the difficult hookless case: start a physical settling wait from a later
poll because callback completion time was not observable.

ADS1115 has stronger separation of transport binding from the desired hardware
profile: `Config.h:166` (`DriverConfig`) and `Config.h:187` (`DeviceProfile`).
Its `ADS1115.h:54` token contract, `:107` applied-profile snapshot and `:127`
terminal result are good references for explicit identity and certainty.
Its `:326` poll documentation also accounts for an execution budget and
uncertain conversion starts. Borrow the ownership properties, not its ADC
channel, gain, conversion freshness or reconciliation states by rote.

INA3221 makes partial/uncertain effects particularly visible:
`include/INA3221/INA3221.h:200` distinguishes successful, failed, cancelled,
timed-out, partial and indeterminate terminal states; `:229` gives a separate
`HardwareEffect`; `:238` supplies caller-owned absolute deadline, callback
timeout and transfer count; `:246` retains request ID, transfers, profile
generation and mismatch register/expected/actual/mask. TUSB32x needs the
effect-evidence principle for reset, mode writes, interrupt acknowledgement
and failures after partial configuration, without inheriting measurement-only
types or unnecessarily exposing every private state-machine stage.

For new code, specify exactly which time model is chosen. Siblings differ:
TMP1x2 uses unsigned 32-bit elapsed durations and optional authoritative clock;
ADS1115 owner calls use caller time; INA3221 uses 64-bit absolute uptime.
Document rollover, maximum interval, backward time, deadline clamping before
and after callbacks, and physical waits after ambiguous writes. Do not mix
these models implicitly or refresh whole-job deadlines on each poll.

The owner engine should publish one terminal result for every admitted job,
reject a second job until the result is consumed, keep outputs unchanged on
rejection/failure unless explicitly documented, and retain the actual transport
error and possible hardware effects even if a logical cancellation/timeout also
occurs. An operation being finished does not establish successful hardware work.

INA228 is especially relevant to interrupt evidence:
`include/INA228/INA228.h:132` preserves clearable diagnostic evidence from
earlier destructive reads; `:219` keeps a fixed latest/sticky event record;
`:408` provides a cache-only event getter and `:413` a bus-silent software
acknowledgement. `src/INA228.cpp:988` implements those without I2C. Its
`:1187` helper contract writes cached alert configuration without pre-reading
live flags. Adapt this separation of observed live state, retained software
events and explicit hardware acknowledgement to the TUSB datasheet's actual
semantics; do not transplant INA228 clear-on-read behavior into a W1C register.
`INA228/tools/run_i2c_hil.py:455` checks exact zero/one/multi-transfer counts,
and `:474` checks that scan/selftest/recover cannot steal an active owner's job.

## Conflicts and legacy patterns to avoid

| Conflict | Actual evidence | Required decision |
| --- | --- | --- |
| `end()` is not universally silent | `OPT4001/include/OPT4001/OPT4001.h:240` attempts a raw power-down write and ignores status; TMP1x2 `TMP1x2.h:167` is local-only; 24Cxx `EEPROM24Cxx.h:63` is silent and preserves physical busy evidence | Make TUSB32x `end()`/unbind local and silent. Device reset/mode changes belong in explicit fallible operations |
| OFFLINE sometimes suppresses I2C | OPT4001 `OPT4001.h:224` documents tick suppression while OFFLINE; TMP1x2 `TMP1x2.h:133` and ADS1115 `ADS1115.h:20` explicitly make it diagnostic | Choose passive health. Successful owner I2C may restore READY; recovery/retry policy stays outside the core |
| Bind validation failure differs | OPT4001 `OPT4001.h:207` clears prior binding even on invalid input; 24Cxx `Config.h:29` preserves the old binding after failed validation | Prefer validation/admission failure without changing working state; document any intentional lifecycle reset |
| AGENTS can lag implementation | OPT4001 AGENTS describes a managed synchronous architecture; its actual `OPT4001.h:246` and `:332` expose poll-chunked work | Audit actual declarations, source, tests and dispatch; write new AGENTS to match the selected implementation |
| Transport return types differ | Sensor peers return `Status`; 24Cxx `Types.h:30` and `:69` have typed terminal results, byte counts and write-effect evidence | Do not impose EEPROM memory semantics. Either use well-documented Status callbacks with conservative ambiguous-write handling, or justify a small typed transport result needed for this chip |
| Register-read framing differs | INA3221 `Config.h:33` allows noninterleaved STOP/START where device supports it; ADS1115 `Config.h:32` and TMP1x2 `Config.h:12` require repeated START | Establish the TUSB protocol from TI and test the exact transaction; generic callback spelling cannot establish protocol validity |
| Configuration-health states differ | ADS1115 has `UNCONFIGURED/APPLYING/VERIFIED/UNKNOWN`; TMP1x2 uses a dirty latch; INA3221 separates measurement and alert certainty | Choose only the distinctions needed for TUSB32x, including configuration and interrupt evidence; do not add health enum values for attach/detach or configuration status |
| Constant spelling differs | ADS1115 AGENTS uses `kCamelCase` for local constants; INA3221/OPT4001 specify CAPS_CASE | Keep public register vocabulary uppercase and choose/document one internal convention |
| `examples/common` may contain Arduino APIs | `OPT4001/examples/common/CliStyle.h:28` calls Serial; INA3221 AGENTS explicitly calls that directory Arduino glue; TMP1x2 and 24Cxx have framework-neutral shared command processors | Copy terminal appearance, not Arduino dependencies; share the actual neutral parser/dispatcher between both frameworks |
| Framework parity checkers vary in strength | `OPT4001/tools/check_cli_contract.py:138` compares dispatcher text; `ADS1115/tools/check_cli_contract.py` primarily inspects Arduino structure; TMP1x2/24Cxx execute shared CLI fixtures | Static checks supplement executable parser and workflow tests; presence of command text does not prove parity or correct effects |
| Review ownership metadata differs | INA228 CODEOWNERS uses `info@thymos.cz`; ADS1115 and OPT4001 use `@janhavelka`, OPT explicitly documents email-resolution limitations | Inspect established maintainer metadata; use a real resolvable owner and avoid invented organizations/repository URLs |

TMP1x2 and 24Cxx's historical `docs/library-comparison.md` contain broader
all-16-library inventories and useful findings, but their earlier commit tables
are historical. Do not report them as fresh audit evidence. They explicitly
warn about outdated runners and differences in actual Arduino/IDF command
coverage. Recheck the current sources when implementing the new library.

## Examples, CLI and host runner

The preferred example structure is a neutral `examples/common/Tusb32xCli.*`,
an Arduino `examples/01_basic_bringup_cli/main.cpp`, a genuine native
`examples/esp_idf/basic/main/main.cpp`, and a small owner-loop example for
shared-bus application integration. Platform entry points own controller,
GPIO, console and scheduling resources. IDF must use native `app_main`,
`driver/i2c_master.h`, timer/task/GPIO APIs; no Arduino compatibility facade.
Examples and runners remain outside the packaged core API.

Useful verified terminal conventions:

- `OPT4001/examples/common/CliStyle.h:17`: 32-character command column;
  `:28`, `:32`, `:36`: cyan title/commands and green bracketed sections.
- `TMP1x2/examples/common/Tmp1x2CliOutput.cpp:10`: ANSI 0/31/32/33/36/90,
  color off; `:29` topical help; `:45` aligned descriptions; `:50` `hilmark`.
- `24Cxx/examples/common/Eeprom24CxxCli.cpp:145`: topic-based grouped command
  inventory with syntax/effect descriptions; `:242` renderer; `:91` status
  severity, symbolic error and detail.
- `TMP1x2/examples/common/Tmp1x2Cli.cpp:13`: strict numeric parsing with full
  token consumption/range checks; `:80`: CR/LF, backspace, overflow/control
  rejection; `:100`: do not delete malformed bytes into a valid mutation.
- `TMP1x2/examples/common/ConsoleBuffer.h:38`: drop complete oversized records
  and count losses rather than emitting misleading truncated diagnostics.

Common command families worth adapting: help/topic help, version/build identity,
color, settings, health/state, bus/transport counters, bind/init/end, probe,
detect/scan where explicitly selected, recover, staged profile validation/apply,
job start/auto/step/status/cancel/result, `xfer_reset`/`xfer_assert`, read-only
selftest, explicitly mutating tests, bounded stress/soak and raw register
diagnostics. Add TUSB-specific role, attach/orientation/current/accessory,
interrupt-event, acknowledgement, reset and feature commands only when they
match documented capabilities. Display configured model versus observed ID and
hardware straps separately. Do not reuse sensor conversion/FIFO/calibration
vocabulary or imply software controls board VBUS hardware.

Health, cached settings, version, help and operation snapshots must be
bus-silent. A diagnostic read can still consume/affect chip evidence: document
that effect separately and avoid making passive status commands acknowledge
interrupts. Raw writes must invalidate managed certainty as appropriate.
Mutation commands must not run at startup or during ordinary read-only checks.

The strongest runner reference in this group is current
`TMP1x2/tools/hil_tmp1x2_runner.py`: lines 5-7 define pass/fail/inconclusive and
explicit probe scope; `:140` saves valid inconclusive evidence before the run
finishes; `:160` atomically replaces reports; `:598` computes remaining serial
budget; `:639` rejects expired writes; `:669` rejects late responses; `:676`
validates exact nonce markers; `:690` generates a unique marker; `:828`
validates firmware identity independently of host checkout. Its `:271` and
`:358` require verified restoration where mutation occurred.

24Cxx adds `tools/hil_identity.py`, `hil_guards.py`, `hil_artifacts.py` and
`test_cli_transcript.py`; its CMake registers the actual shared native C++ CLI
fixture with the Python parser. This is a strong pattern for testing the real
command/response contract. Do not copy EEPROM write-cycle or scratch-window
policy into a Type-C controller runner.

Runner requirements should include exact command correlation, structured
terminal evidence, rejection of duplicate/contradictory/stale/truncated records,
deadline enforcement, finite memory/rotating or streamed transcripts, complete
selected-suite accounting, stop-on-unexplained-failure, host and firmware
provenance, separate cleanup outcomes and no overall PASS for skipped required
checks. Firmware reset or framing loss must invalidate a session. TUSB mode or
role tests require an appropriate physical fixture and explicit selection;
capturing/reapplying registers cannot undo a real USB attach/detach/power event.

## Tests, build and release evidence

`TMP1x2/test/jobs/test_jobs.cpp:137` tests compatibility API exclusion while a
job/result owns the instance; `:171` checks bind/admission silence, rejected
tokens, zero budgets, retained results, wrong tokens and take-once consumption;
`:210` tests physical transfer budgets and timeout grants; `:231` tests whole
deadline expiry and late-success health accounting. These are behavioral
contracts rather than tests that merely repeat implementation structure.

`TMP1x2/test/cli/test_cli.cpp:215` executes the input path, `:224` validates
marker grammar, `:657` tests real baseline restoration and failed restoration,
and `:878` tests wrapped deadlines. Use the same level of transport/state
assertions around invalid TUSB CLI commands. Add a scripted transport capable
of failures at every callback, successful writes followed by failed readback,
ambiguous writes, concurrent hardware status changes and reasserted events.

`TMP1x2/CMakeLists.txt:9` and `:16` support both an IDF component and standalone
CMake library. `:18` keeps native tests opt-in; `:23` through `:59` register
core/CLI/console/jobs/variants/helpers/bus/no-framework and Python suites.
24Cxx `CMakeLists.txt:38` and `:43` registers runner tests and executable CLI
transcripts. Register all tests in the actual local/CI entry point; a test file
existing in the tree does not establish that it runs.

TMP1x2's current `.github/workflows/ci.yml` configures host CMake/CTest with
strict warnings, conversion/sign warnings, ASan/UBSan, contracts, Doxygen,
Arduino S2/S3 builds, package validation and genuine native IDF S2/S3 builds
across its pinned IDF matrix. 24Cxx additionally extracts the archive and builds
a clean consumer. This is a better new-repository baseline than copying older
IDF-header-only compilation or a single native PlatformIO environment.

Windows should use the existing VS Code-managed PlatformIO via a project
`scripts/pio.cmd`. TMP1x2's wrapper uses `%USERPROFILE%\.platformio\penv`.
ADS1115's wrapper additionally isolates short package/cache paths to prevent
Windows extraction limits and cross-project pinned-package collisions. Audit
the available installation and chosen pins; do not install a second Core or
claim other repositories' pins have been validated for TUSB32x.

Keep a feature-to-API-to-CLI-to-test matrix and separate evidence categories:
source inspected, native tests passed, package consumed, framework compiled,
native IDF linked, physical I2C captured, actual CC/INT/role/accessory behavior
tested. Native fake transport results cannot establish USB-C electrical
behavior, connector orientation, bus recovery or board power safety. Preserve
dates, exact commands, source revision, fixture/model/board details and result
logs for any claimed hardware validation.

## Recommended reference priority

1. TMP1x2: small family naming, shared CLI, practical owner engine, native
   CMake/package checks, comprehensive runner and evidence discipline.
2. ADS1115 and INA3221: transport/profile separation, strong owner contracts,
   retained results, configuration certainty and effect evidence.
3. 24Cxx: shared example/console architecture, strict parsing, executable
   Python-to-C++ transcript tests and robust artifacts; exclude memory protocol.
4. INA228: mixed live/configuration status register discipline, preserved
   diagnostic events, owner-job transfer counters and CLI/HIL correlation.
5. OPT4001: terminal appearance and broad diagnostic coverage; reconcile its
   legacy teardown, health gating and independently implemented front ends
   before transferring any behavioral pattern.

The future implementation agent must repeat this audit against current sibling
source at the start and recheck relevant references before each major phase.
It must record conflicts and justify one coherent TUSB32x contract rather than
claiming a nonexistent single identical API across all siblings.
