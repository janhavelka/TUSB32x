# Prompt 5 focused peer re-audit

Reviewed 2026-10-23 for passive values, Status, configuration separation and the
planned API inventory. Read repository/parent AGENTS.md, prompt index, COMMON,
ENGINEERING_BASELINE, MAINTAINABILITY, AGENT_WORKFLOW and prompt 5; revisited
execution-4-peers as dated comparison evidence. Paths below are relative to
`Projects/`. Siblings were read-only: no fetches, edits, builds, remote CI queries
or hardware actions. Referenced peer tests were inspected, not executed.

## Captured state

All five peers had empty `git status --short`, branch `main`, upstream
`origin/main`, and local `HEAD...@{upstream}` ahead/behind `0/0`. No fetch was
performed, so this does not establish current server state.
Their HEADs and clean worktrees were unchanged at the final recheck.

| Peer | Full HEAD |
| --- | --- |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` |
| LSM6DS3TR | `e262bf22f3d0cf035fbbca10f45b3e0752bf2bac` |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| ADS1115 | `235af3e8c722661a89a44766a4ccd903a654f6fe` |
| INA3221 | `40838b56e41c25183478b086e9e214ceffe3290c` |

TUSB32x was clean at audit start on
`promptset/tusb32x-implementation-20261023`, HEAD
`d29cb36b3e0f5ed57dc5512964ce40e2b5326ae2`. The lead owns synchronization and
all shared-contract edits. These peer HEADs supersede the baseline's historical
revision table for this review; the historical table is not a current-state claim.

Read all five root AGENTS.md files for their relevant framework, ownership,
Status, effect, naming, validation and example boundaries. They agree on borrowed
application transport, fixed storage and separation of desired and verified
hardware state. Their compatibility surfaces, sensor state names, callback
"physical attempt" wording and recurring remote CI policy do not override the
TUSB32x callback-count contract or prompt-34-only remote CI exception.

## Status and passive public values

Re-read `LDC1614/include/LDC1614/Status.h` and
`LSM6DS3TR/include/LSM6DS3TR/Status.h`: both expose typed `Err : uint8_t`, public
`Status { code, int32_t detail, const char* msg }`, constexpr construction,
`ok/is/inProgress` and `Ok/Error`. Messages are explicitly static-lifetime.
Neither header provides explicit bool or a public `errorName`. LDC's
`test/test_basic.cpp::test_status_and_public_type_contracts` checks constexpr
value helpers, trivial-copy/fixed-memory properties and invalid channel input;
its forced transport statuses use full details `-1234567` and `-7654321`.
LSM's `test_status_contract` checks exact `I2C_TIMEOUT`, detail `-42` and message
presence. Both are present in their files' actual `RUN_TEST` lists.

`TMP1x2/include/TMP1x2/Status.h` supplies the additional
`explicit constexpr operator bool()` and a complete header-defined constexpr
`errorName` with `UNKNOWN` fallback. Adopt those mechanisms alongside the
LDC/LSM representation. Reject TMP's `toString`, compatibility enum alias,
`OFFLINE`, sensor-ready errors and separate `OPERATION_TIMEOUT`; TUSB32x owns its
exact ordered error vocabulary and owner `TIMEOUT` versus callback `I2C_TIMEOUT`.
No inspected TMP test explicitly proves forbidden implicit Status conversion or
constant evaluation of every error name. TUSB32x therefore needs its own compile
checks, invalid-name case and multiple-translation-unit check; peer code is not
verification of those requirements.

Re-read `LSM6DS3TR/test/test_compile_contracts.cpp`: it preserves an application
macro while including Config.h, checks trivial-copy public payloads and uint64
token identity, and rejects copy/move of the facade. Adopt self-contained header,
fixed-memory and ownership checks. Its hard-coded size bounds describe that
peer's payload and target ABI; do not copy them as portable TUSB32x limits.
LDC's fixed-array/mask helpers are useful examples of total constexpr functions,
but its channel, clock, conversion, two-result FIFO and caller-ID vocabulary do
not belong in a Type-C controller API.

## Configuration separation and validity

Read `LDC1614/include/LDC1614/Config.h`, `LDC1614.h` and the matching
`src/LDC1614.cpp` validation/binding helpers. Its single `Config` combines
callbacks/address, electrical facts, channels and replayable silicon settings;
default selection sentinels are deliberately invalid. The registered
`test_public_config_validation_is_pure_and_matches_bind_contract` checks zero
I2C/INTB calls and channel-index detail. The registered
`test_bind_is_zero_i2c_and_validates_complete_explicit_profile` exercises 24
invalid candidates, including null callbacks, zero timeout, unspecified variant,
unknown enum values and cross-field clock/channel limits. Adopt explicit
invalid selection and semantic/cross-field validation. Reject the combined
binding/profile value and initialization replay as TUSB32x contracts.

Read `LSM6DS3TR/include/LSM6DS3TR/Config.h`, `LSM6DS3TR.h` and
`src/LSM6DS3TR.cpp::validateDriverConfig/validateProfile/bind`. Its DriverConfig
contains transport, address and timeout; DeviceProfile contains typed silicon
settings. Validation precedes state mutation. The registered
`test_validate_driver_config_and_address` checks callback/timeout/address
rejection; `test_validate_profile_rejects_all_unsupported_or_unsafe_values`
checks invalid enum values and unsupported combinations. Adopt split values,
documented defaults and validation before mutation. Reject assuming enum ordinal
is register encoding, unsupported future profile fields, sensor settling and its
`unbind` result-clearing policy. TUSB32x prompt 5 exposes passive types only;
validators and binding remain planned until their implementation stages.

Read `ADS1115/include/ADS1115/Config.h::DriverConfig/DeviceProfile`,
`ADS1115.h::AppliedProfileSnapshot/OperationResult` and
`src/ADS1115.cpp::bind/takeResult`. Its non-owning transport is separate from
desired/applied profiles and generation; bind validates before replacing state.
Registered `test_owner_safe_profile_validation_boundaries`,
`test_owner_rebind_preserves_address_on_invalid_or_busy_candidate` and
`test_owner_safe_apply_profile_success_commits_candidate_snapshot_and_generation`
cover those boundaries. Its DeviceProfile contains the target address, and a
typed read can update the committed mux/gain profile. TUSB32x instead places
variant/address in DriverConfig and preserves desired settings separately from
raw observation and verified applied evidence.

Read `INA3221/include/INA3221/Config.h::TransportConfig/DeviceProfile` and
`INA3221.h::AppliedConfigState/AlertSnapshot/JobResult`, plus
`src/INA3221.cpp::bind/startReconcile/_markRegisterUnknown/_markRegisterDirty`.
Separate profile certainty and explicit raw/effect validity are useful. Reject
putting host calibration, clock/yield hooks or health thresholds into TUSB32x's
silicon profile/binding. INA's read/compare/write/verify reconciliation replays
settings; TUSB32x reconciliation remains read-only. Its registered
`test_owner_bind_unbind_and_invalid_rebind_are_bus_silent`,
`test_owner_bind_requires_explicit_calibration_only_for_enabled_channels`,
`test_owner_result_identity_pending_take_once_and_stale_prevention` and
`test_owner_profile_reads_disprove_retained_certainty_before_writes` are useful
future failure-boundary references, without importing their sensor policy.

TMP's `src/TMP1x2Operations.cpp::takeResult` checks availability/token before
copying output, then consumes only the matching result and returns delivery OK.
`test/jobs/test_jobs.cpp::admissionAndResults` checks unchanged sentinel output
when unavailable or mismatched, pending backpressure, zero-budget silence and
exactly-once take. Adopt those result checks. Its `src/TMP1x2.cpp::bind` clears
the binding before validation and its normalization rejects only callback
IN_PROGRESS; reject both as TUSB32x rules. TUSB32x preserves an existing binding
on rejected candidates and accepts only OK/documented I2C_* callback statuses.

## Material choices compared against two current peers

| Choice | Compared evidence | Selected TUSB32x rule |
| --- | --- | --- |
| Status representation | LDC and LSM Status.h; TMP Status.h | Preserve int32 detail/static message and constexpr helpers; add explicit bool and one inline constexpr errorName; no compatibility aliases. |
| Binding versus settings | LSM split Config.h; ADS split Config.h versus LDC combined Config | DriverConfig owns borrowed transport, explicit variant/address and timeout; DeviceProfile owns verified silicon settings only. |
| Defaults and enums | LDC invalid selectors/tests; LSM enum/cross-field validation | UNSPECIFIED variant fails binding; semantic enum values require explicit codec tables and variant-aware validation. A candidate profile authorizes no writes or board power. |
| Configuration evidence | ADS AppliedProfileSnapshot; INA certainty/AlertSnapshot | Desired, observed raw bytes, independent validity and verified configuration stay separate; a default typed value is not an observation. |
| Token/result boundary | TMP admissionAndResults/takeResult; LSM token tests and fixed payloads | One nonzero instance token and retained terminal slot; failed take preserves output/evidence, delivery status differs from operation status. |
| Public inventory | LSM compile contracts; TMP core_no_arduino consumer | Publish implemented passive values/helpers; document later callable signatures without linking promises or fake stubs. |

No inspected current peer evidence requires changing the settled baseline's
passive API contract. Differences above are deliberate TUSB32x choices, not
claims that all peers implement the same API.

## Examples, record names and registration

Re-read the relevant command/output producers and Python consumers:

- LDC `examples/common/Ldc1614Cli.cpp` declares `cfg/settings` as cache-only
  desired/staged output. `tools/ldc1614_hil_runner.py` distinguishes
  `CLI result: command=... session=... outcome=... code=...` from
  `Operation result: operation=... kind=... outcome=... effects=... code=...
  detail=... msg=...`, and requires `firmware_git`/`firmware_status=clean`.
- LSM `examples/01_basic_bringup_cli/main.cpp::printProfiles/printTerminalResult`
  emits separate `profile staged`, `profile desired`, `profile verified` and
  `result token=... kind=... state=...` records. `tools/run_hil.py` checks
  firmware/library/platform version, WHO_AM_I and verified-profile output.
- TMP `examples/common/Tmp1x2Cli.cpp` uses cached settings and owned operation
  result commands. `tools/hil_tmp1x2_runner.py::operation_admission/
  operation_status` requires matching kind/token and terminal completion;
  settings begin with `Desired: model=... address=...`.
- ADS `examples/02_owner_safe_poll/main.cpp::consumeTerminalResult` takes the
  active token before replacement work. `tools/run_i2c_hil.py` uses
  `firmware_clean_commit` on the firmware's `ADS1115 library commit:` record
  and preserves `NOT_RUN` rows.
- INA `examples/01_basic_bringup_cli/main.cpp::printOwnerResult` emits
  `=== Owner Job Result ===` and separate status/effect fields.
  `tools/hil_cli_runner.py::extract_hil_frame` matches both
  `HIL_BEGIN token=... seq=...` and `HIL_END token=... seq=... status=...`.

Adopt explicit settings provenance, matching results and separate firmware
identity evidence. Reject copying any peer's second firmware session scheduler,
record grammar or conversion fields into the TUSB32x shared CLI contract.

Read all five `platformio.ini`, root `CMakeLists.txt` and
`.github/workflows/ci.yml` registration points. LDC/LSM/ADS/INA use Unity with
`test_build_src=true` and workflow native test commands. LDC/LSM/INA native use
C++17; ADS native still uses C++11, which is not TUSB32x's selected standard.
Their root CMake files register IDF components rather than standalone CTest.
INA's `test/test_basic.cpp` actually calls `runOwnerOperationTests()` from
`test/test_owner_operations.cpp`; the latter calls its named `RUN_TEST` entries.
LSM's compile-contract translation unit is included by native test compilation;
it contains static assertions rather than an extra runtime success test.

TMP supplies plain CMake with tests OFF by default and CTest names `core`, `cli`,
`console`, `jobs`, `variants`, `helpers`, `bus`, `core_no_arduino` plus Python
guards. Its workflow runs configure/build/CTest with strict conversion warnings
and ASan/UBSan. Its PlatformIO custom runner is
`test/test_custom_runner.py`, recognizing `[PASS]`/`[FAIL]` lines. Adopt ordinary
C++ consumption, opt-in tests and explicit suite registration without inheriting
generic CTest names. TUSB32x's baseline owns its `tusb32x_*` names.

The workflows separately register CLI/IDF/timing guards and runner checks:
LDC `tools/test_ldc1614_hil_runner.py`; LSM `tools/test_run_hil.py`; TMP CTest
`hil_runner` using `tools/test_hil_tmp1x2_runner.py`; ADS
`tools/run_i2c_hil.py --parser-test`; INA
`tools/hil_cli_runner.py --parser-self-test`. This is source registration evidence
only. No peer test, firmware build or remote workflow was executed by this audit.

## Validation boundary

PASS: current local revision/branch/dirty-state capture and direct source/test/
registration inspection; report whitespace check and existence checks for 17
key source paths. This documentation audit establishes convention choices,
not TUSB32x runtime, protocol, SDK or hardware correctness. The lead runs and
records the prompt's real public-header, constexpr, explicit-bool, wide-detail,
multiple-TU and compile-only inventory checks with the implementation.
