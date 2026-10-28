# Prompt 22 fresh peer and public-surface review

Reviewed 2026-10-28 before implementation. TUSB32x was clean at
`e1f5538f669af4f926af355f2bba904da9478049` on
`promptset/tusb32x-implementation-20261023`. This reviewer owns only this report;
Git mutations, integration and verification belong to the lead. No sibling was
modified or built, no remote CI was inspected, and no hardware was exercised.

Read `../AGENTS.md`, local `AGENTS.md`, `docs/prompts/README.md`, `COMMON.md`,
`AGENT_WORKFLOW.md`, prompt 22, `CLI_CONTRACT.md`, relevant raw/statistics/health
sections of `ENGINEERING_BASELINE.md`, `ENGINEERING_RULES.md`,
`MAINTAINABILITY.md`, and current progress/decisions. Sibling instructions inform
conventions only; their permissions and device behavior do not transfer here.

## Current peer evidence

All five `git status --short` results were empty. Paths below are relative to
the named sibling under `../`; every sibling's own `AGENTS.md` was inspected.

| Peer | Current HEAD | Exact relevant source, test, runner and workflow paths |
| --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `examples/common/Tmp1x2CliDiagnostics.cpp`, `examples/common/Tmp1x2Cli.cpp`, `examples/common/Tmp1x2CliOutput.cpp`, `test/cli/test_cli.cpp`, `tools/check_contracts.py`, `tools/hil_tmp1x2_runner.py`, `.github/workflows/ci.yml` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | `examples/common/Eeprom24CxxCli.cpp`, `test/test_cli/test_cli.cpp`, `tools/test_cli_transcript.py`, `tools/check_contracts.py`, `tools/hil_runner.py`, `.github/workflows/ci.yml` |
| SHT3x-main | `f65f15573e3c9b1ef04cce72ad08f307d021e563` | `examples/common/Sht3xCli.cpp`, `tools/sht3x_cli_contract.py`, `tools/check_cli_contract.py`, `tools/test_cli_health_rate.py`, `tools/run_sht3x_hil.py`, `.github/workflows/ci.yml` |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `include/LDC1614/LDC1614.h`, `examples/common/Ldc1614Cli.cpp`, `test/test_cli.cpp`, `tools/ldc1614_cli_contract.py`, `tools/ldc1614_hil_runner.py`, `.github/workflows/ci.yml` |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | `examples/01_basic_bringup_cli/main.cpp`, `examples/esp_idf/basic/main/main.cpp`, `tools/run_i2c_hil.py`, `tools/test_cli_regressions.py`, `docs/CLI.md`, `.github/workflows/ci.yml` |

LDC1614 has advanced from the historical `c0ab4c1...` cited in the initial
contract. The current source and executed-test definitions, rather than that
old report, informed this review. Other four HEADs match the initial evidence.

TMP1x2 tests explicitly check zero transfers for cached diagnostics and invalid
arguments, each grouped help topic, the `?` alias, and absence of ANSI after
color-off. Adopt these observable checks. Its raw/probe health bypass and its
separate firmware diagnostic scheduler are unsuitable here: TUSB probe is
managed initialization, and repeated suites belong to the host runner.

24Cxx runs its actual host CLI persistently and validates emitted evidence
against modeled device state, including before/after contents. Adopt that
independent behavioral oracle and finite subprocess timeout, plus counters reset
without resetting passive health. Do not copy memory scratch restoration,
interface reset, EEPROM geometry, startup writes or its different assertion arity.

SHT3x provides explicit alias metadata and shared command parsing, including
`help/?`, `version/ver`, and `cfg/settings`. Adopt canonical route normalization.
Its checker compares source help strings and required tokens; those checks alone
cannot prove dispatch, callback counts or returned records. TUSB needs the real
built fixture plus intentionally damaged-contract negatives. Do not import its
separate compatibility commands, scan or confirmation grammar.

LDC1614's current command rows separately name execution, safety, fixture and
output evidence. Tests iterate aliases and reject surplus arguments for the
canonical inventory. Adopt explicit classifications and behavioral iteration.
Use TUSB's fixed execution/effect classes, not LDC's `CLI_JOB`, `ONE_TRANSFER`,
`OWNER_BUS` or blanket `confirm` rules. Its GPIO classification as PURE is not a
reason to classify TUSB's injected `intpin` reader as pure: that reader is an
explicit platform action. Its sensor initialization/profile commit semantics do
not describe TUSB read-only init or explicit apply.

INA228's transfer plan checks exact zero/one/N callback totals and owner-exclusion
rejections, followed by completion of the preserved job. Adopt exact equality
and independent continuation evidence. Reject its unchecked `read + write`
total arithmetic and its active-operation counter-reset pattern: TUSB counts
saturating all-callback totals directly, rejects saturated assertions, and its
CLI reset is idle-only. Do not infer physical transactions from callback counts.

Workflow review: TMP1x2/24Cxx register CMake/CTest strict warning and sanitizer
commands plus Python contract guards; SHT3x registers native PlatformIO and
Python CLI/runner checks; LDC1614 registers CLI/runner/consumer checks with Python
3.11; INA228 pins PlatformIO 6.1.19 for its framework matrix. These are source
observations, not claims that a remote run passed. TUSB has no checked-in
`.github/workflows` directory yet; its current `test/CMakeLists.txt` registers
CLI, console and exactly one persistent `cli_transcript`, with built target path
and finite timeout. Keep that registration and add one executable parity check.

## Existing 44 rows versus public core and variants

Compared `include/TUSB32x/{TUSB32x,Config,CommandTable}.h`,
`src/TUSB32xDiagnostics.cpp`, `src/TUSB32xProtocol.cpp`,
`examples/common/Tusb32xCli.{h,cpp}`, `Tusb32xCliOutput.cpp`,
`tools/tusb32x_cli_contract.py`, `tools/test_cli_transcript.py`,
`test/cli/test_cli.cpp`, `test/cli/test_cli_operations.cpp`,
`test/console/test_console.cpp`, `test/CMakeLists.txt`, and
`docs/variant-matrix.md` against the fixed grammar.

All ten managed `start*` methods already have canonical routes. Bind/end,
forgetting, staging/validation, progress/poll/cancel, result snapshot/take,
configuration and port snapshots already have routes. Complete exactly eleven
remaining canonical rows, yielding 55, with aliases from the fixed contract:

| Missing route | Existing source of behavior | Required boundary |
| --- | --- | --- |
| reg | readRegisterSync | At most one callback; no cached identity promotion |
| decode | decodeRegister | Supplied byte only; independent field-code validity |
| wreg | writeRegisterSync | Core composition and sequence gates; uncertain effects survive failure |
| irq | diagnostics interrupt plus port snapshot | Distinguish retained ACK history from sampled port latch |
| intpin | injected application reader | Opt-in platform action; no GPIO setup or transport accounting |
| invalidate | invalidateDeviceState | Three explicit supported reasons; preserve results/effects |
| drv | diagnostics plus projectHealth | Managed-only passive projection; trust remains separate |
| diag | existing cached snapshots | Zero callbacks; complete usable evidence without invented defaults |
| xfer_stats | transportStats | All callbacks, separately named source and saturation |
| xfer_reset | clearTransportStats | Idle-only CLI gate; no trust/effect/result/managed repair |
| xfer_assert | transportStats equality | Reject any saturated total; report source and expected/actual values |

No additional operation engine, register composer, health controller or generic
registry is required. Validators and enum/status helpers support the existing
routes; they do not each require a redundant command name.

Keep absent registers variant-gated (base A0, 322I 45), and keep 322I 08/09 writes,
IRQ ACK/service and real non-reset return to DRP gated. Raw mode/reset/termination
changes cannot bypass typed sequences. Register decoding can preserve a byte
without granting write permission or proving paired port applicability. No new
silicon claim or mask is needed for this prompt.

## Record and accounting recommendations

Use the existing renderer and split bounded records by evidence responsibility.
At minimum expose supplied register/value and validity, profile codes, status
codes and reset-bit validity; all callback totals/source/saturation; passive
health with effective threshold and managed counts/streak/status; diagnostics
with bound/initialized/trust, effect/termination/reset, all four unresolved or
uncertain flags, managed last-operation kind/status, retained last failed callback
status/detail/time-valid, identity bytes/revision validity, and retained IRQ
before/attempt/acceptance/after validity. Preserve signed int32 detail, raw unknown
encodings and separate historical port invalidation. Default false/zero values
must remain visibly unavailable where the corresponding valid flag is false.

`configurationWriteUnresolved` is independently necessary even when a later
read-only terminal operation reports effect NONE. Clearing transport totals
cannot erase that obligation or the managed failure streak. Health threshold
three is the existing policy default, not a silicon limit; no new setter grammar
is necessary. An opt-in GPIO reading is not an I2C callback.

For `xfer_reset`, reject ACTIVE with BUSY and retained-result ownership with
RESULT_PENDING. This gives the literal idle-only CLI contract while leaving the
core's broader clear API intact. Prove both rejection preservation and successful
idle clearing with unchanged trust, effects and managed evidence.

Raw handlers can report callback count from the returned terminal status domain
even when counters are saturated: OK or documented I2C_* means one callback;
other statuses mean zero. The current protocol validates before invocation and
normalizes every callback to that domain. Do not infer a saturated delta by
subtracting totals. Assert rejects saturation of attempts, successes or failures,
including apparent numeric equality; use a typed existing error and explicit
assertion verdict rather than inventing a new core Err.

Exercise every record at maximum nonce, request/token, uint32/uint64, signed
detail and enum widths, including final newline and NUL storage. Exercise actual
runtime records and help against Python rows, not only constructed schema text.
Any new record must fit the existing whole-record loss/reservation guarantees.

This is an audit and implementation-review checklist. Test execution, completed
implementation, commit and sync evidence are recorded by the lead in progress
and validation; this report does not claim those checks ran.

## Independent implementation review

Read the production changes in all three common CLI files after the eleven
routes were implemented. The raw handlers call only public Sync APIs; no private
core header, register composer or alternate transport path was added. The
normalized raw terminal domain preserves an exact callback count at saturated
totals. `ATTEMPTED` distinguishes a failed invoked write from local `UNCHANGED`
rejection. Public decode validity masks qualify every printed profile/status
field, including unknown/reserved codes; supplied-byte interpretation does not
claim paired applicability. Cached handlers and counter operations remain
bus-silent; automatic work remains in the one existing tick poll site.

The strict idle counter-reset gate preserves active or retained ownership by
rejection. The successful path invokes only `clearTransportStats`, which cannot
erase managed evidence, effects or trust. Assertions test all three counters for
saturation before equality. Pin output uses available/unknown and does not count
the opt-in application hook as transport.

Conservative manual format calculations using maximum nonce/request/integer and
enum spellings, including newline and NUL, gave DIAG 384, MANAGED 361, DECODE 378,
DECODE_PROFILE 357 and XFER 334 bytes, below the existing 512-byte storage bound.
These calculations supplement the lead's executable storage tests.

One completeness finding was sent to the lead: the preexisting profile records
did not expose `ConfigurationSnapshot` mismatch validity/register/expected/
observed/mask or acquisition timestamp/time-valid. A separate bounded
configuration-detail record can close that public-evidence gap without changing
the parser or core. No raw-safety, callback-accounting or ownership defect was
found in the reviewed production diff. Final executable tests and subsequent
changes remain the lead's integration responsibility.

Final review confirmed this finding is closed by `CONFIG_DETAIL`; unavailable
tuple bytes print `unknown`. `configurationMismatchDetails` obtains an actual
managed mismatch and checks distinct register 8, expected 64, observed 128 and
mask 192, acquisition time 200, and retained result preservation through all
four profile query spellings. `DRV` now explicitly names `managed_callbacks`.

The executable checker starts the supplied built fixture, executes each
canonical route and every alias, checks typed closure and actual adapter callback
deltas, and compares grouped/per-command help metadata and human syntax. Broken
alias and required-record-field dependencies are injected into the checker and
must fail against actual fixture output. `compare_help` is reused by the
persistent transcript; there is no second source-token parity implementation.
Both checker modes have finite CTest timeouts and explicit target paths, and the
original single transcript registration remains. Suggested additionally checking
the independent adapter write count stays unchanged for read-observation routes;
a total delta and reported effect alone cannot identify a mistaken write.

Output loss cannot rerun a raw access or undo its evidence: dispatch occurs once
before formatting, and enqueue failures only update loss accounting. Every
normalized raw callback status is exercised for read/write, preserving original
signed detail and independent managed history. Failed raw writes keep UNKNOWN
trust and INDETERMINATE effect. Console saturation is independently tested;
a combined raw-error-with-output-congestion regression was suggested to strengthen
that interaction. These source-review conclusions do not substitute for the
lead's recorded execution results.
