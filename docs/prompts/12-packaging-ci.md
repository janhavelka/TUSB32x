# P12 - Prove clean packages, register checks and run CI

Read [COMMON.md](COMMON.md), [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md),
[CLI_CONTRACT.md](CLI_CONTRACT.md), [MAINTAINABILITY.md](MAINTAINABILITY.md) and
[AGENT_WORKFLOW.md](AGENT_WORKFLOW.md). A manifest/workflow is not a passed build.
Prove the archive works without this checkout, siblings, Git or source generation.

## Fresh audit and pins

Inspect TMP1x2 `tools/check_package.py`, version tests/generator, CMake and full
CI; 24Cxx package/docs links, clean consumers and transcript registration;
OPT4001 clean-consumer/action-pin checks; LDC1614/INA3221 native-IDF image builds
and test registration. Cite current HEAD/dirty state/files and adopted/rejected
choices. Read official tool/SDK docs when selecting/changing pins.

TMP1x2 `bafc34067530eb340e55c42dae3fad4eb795b6e9` currently separates native,
documentation, Arduino and native-IDF jobs, uses strict ASan/UBSan host checks,
PlatformIO 6.1.19 and IDF v5.3.2/v5.5.1/v6.0.1 for S2/S3. These are comparison
inputs, not already validated TUSB pins. Select the supported matrix from P09/P10
evidence, covering a current supported SDK and oldest claimed supported SDK. Fix
failures or explicitly revise support claims with evidence; never quietly remove
a gate. Sibling green CI does not verify this library.

## Packaging and clean consumers

1. `library.json` remains version truth. `scripts/generate_version.py`
   `sync/check/bump/set` generates stable committed `Version.h` and synchronizes
   CMake/component/Doxygen fields. Archives compile with honest unknown Git
   identity and no Python/Git generator required at consumer configure/link time.
   Firmware identity belongs to example builds.
2. Explicit package includes/excludes cover public/core files, metadata/license,
   shared CLI/console/board files, example CMake/sdkconfig defaults and useful
   docs. Exclude builds, `.pio`, caches, local sdkconfig/locks, binaries/logs,
   private evidence, research PDFs/vendor source. Preserve required licenses.
   Keep needed example build scripts: blindly excluding every Python file can
   break firmware-identity generation. Core consumers still need no generator.
3. Add `tools/check_package.py` and `tools/test_check_package.py`. Reject archive
   traversal/absolute/drive/backslash paths, duplicate members, links and special
   files; bound member/count/byte size. Extract into a new verified temporary
   root and rename the library directory. Never move/delete outside that root.
   Validate exact required/forbidden contents and fail on missing shared files.
4. Add `test/package_consumer/CMakeLists.txt`/`main.cpp`. Directly include each
   public header, bind fake transport, run a cooperative operation, compile/link/
   execute against the extracted archive. Copy only archive and fixture to the
   workspace: no siblings, `.git`, build caches or generator tools. Tests OFF
   must not discover Python, Doxygen or test frameworks or require Arduino/IDF.
5. Required CMake interface is `add_subdirectory()` with target `TUSB32x`.
   Claim install/export/`find_package()` only if implemented with relocatable
   exports and tested after moving the install prefix. Source build success is
   not installed-package proof.
6. Build extracted-package Arduino and genuine native-IDF consumers on S2/S3 for
   the supported matrix; link images, without sibling or source-checkout paths.
   Verify `app_main`/native APIs and absence of Arduino component linkage. SDK
   header-only compilation is insufficient.
7. Check local Markdown links in repository and archive. Excluded audit/vendor/
   test files must not leave broken package links; label repository-only paths
   or use verified official source URLs. Manifest framework/platform claims match
   proven portable core and tested adapters.

## Checks and registration

Use one `tools/validate.py --checks ...` interface with P11 verdict/exit rules.
`--checks all` means all declared non-HIL release checks. Required missing tools
produce INCOMPLETE/nonzero; focused selection names its narrower scope and never
claims complete validation. Locally use existing `scripts/pio.cmd`; no second
PlatformIO Core install. Pinned isolated Linux CI is a separate environment.

| Check / proposed files | Required evidence |
| --- | --- |
| CMake/CTest `TUSB32X_BUILD_TESTS=ON` | Registered core/jobs/variants/protocol/owner/CLI/console/no-framework executables; Release retains test assertions |
| `tools/check_contracts.py` | Core framework/time/allocation boundaries, include hygiene, metadata/source registration; negative fixtures demonstrate detection |
| `tools/check_cli_contract.py` | Executed help/aliases/records/parity through real CLI; static source tokens supplementary |
| `scripts/generate_version.py check`, `tools/test_version_sync.py` | Header/CMake/component/Doxygen drift, idempotent sync, malformed version and archive unknown Git identity |
| `tools/check_doxygen.py`, `tools/test_check_doxygen.py` | Actual Doxygen warnings fail; public API and relevant CLI/helper contracts documented in source; broken refs/missing-doc negative cases |
| Python unittest + runner/fixture CTest | Fault parsing, provenance, cleanup/artifacts/verdicts and real C++ transcript path actually execute |
| `tools/check_package.py`, `tools/test_check_package.py` | Whitelist/malicious/missing-member fixtures, clean headers/linked consumer, shared example files and docs links |
| Arduino + native-IDF matrix | Real S2/S3 links with documented pinned tools, source/package results labeled separately |

Use standard CMake targets, CTest and unittest; no custom test framework. Python
discovery belongs inside the opt-in test block. Pass
`$<TARGET_FILE:tusb32x_cli_host>` to fixture tests instead of guessed `.exe`
paths. Set per-test timeouts and inspect `ctest --show-only=json-v1` for expected
suite registration. Zero tests or omitted required groups fail verification.
Test guard failure/exit paths; printed SKIP must not become green full validation.

## CI jobs and artifacts

Create `.github/workflows/ci.yml` with minimal `contents: read`, explicit timeouts,
separate host/Python/docs/Arduino/native-IDF jobs and bounded artifacts/retention.
Host strict flags include `-Wall -Wextra -Wpedantic -Wconversion
-Wsign-conversion -Werror`; Linux ASan/UBSan uses a supported compiler. Do not
apply unsupported sanitizers to embedded SDKs. Pin actions to verified full SHAs
with release-name comments and pin PlatformIO/platform/SDK versions from official
sources. Never copy an unverified action SHA from an old audit.

Keep failed-test logs, CTest summaries, Doxygen warnings and package-consumer
evidence; retain firmware identity/ELF/map where useful. No secrets or unbounded
caches. Ordinary CI never flashes, runs HIL, publishes or changes board power.
For a configured remote, follow parent policy and verify exact-final-commit CI at
this milestone, dispatching only if needed. Inspect actual jobs and fix failures.
Missing remote/service access is BLOCKED/NOT RUN for that check, not authority to
invent a remote or claim configured CI passed. Routine commits and pushes are
already authorized under AGENTS.md; use the configured upstream and verify sync.

These are proposed acceptance commands after implementation, not prior results:

```powershell
python scripts/generate_version.py check
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release --parallel
ctest --test-dir build/native -C Release --show-only=json-v1
ctest --test-dir build/native -C Release --output-on-failure
python tools/check_contracts.py
python tools/check_doxygen.py
.\scripts\pio.cmd run -e esp32s2dev -e esp32s3dev
.\scripts\pio.cmd pkg pack -o build/TUSB32x.tar.gz .
python tools/check_package.py build/TUSB32x.tar.gz
python tools/validate.py --checks all
```

P09 creates consistent proposed `esp32s2dev`/`esp32s3dev` environment names.
Native IDF is separate `idf.py -C examples/esp_idf/basic set-target <target> build`
with isolated directories per target/version; record actual S2/S3 commands and
results. Expected output pasted from docs is not run evidence.

## Documentation and completion

Update CONTRIBUTING/release guidance, README, CHANGELOG, package/CMake examples,
supported tool matrix, source Doxygen/docstrings, coverage and validation now.
Document commands, files, artifacts and limits in plain English. Preserve actual
maintainer/owner metadata; invent no hosting URL. Publishing tags/registry/releases
requires actual user authorization, not a sibling rule. Report source tests,
clean archive consumption, linked framework images and exact-commit remote CI
separately. Missing required evidence leaves that item incomplete; never weaken
a check to finish.
