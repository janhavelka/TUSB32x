# Prompt 8 current metadata and package audit

Read-only source review on 2026-10-26 (Europe/Prague). Siblings were neither
edited, fetched nor built. No remote CI was dispatched, polled or queried.
Paths beginning with a peer name are relative to
`C:/Users/Honza/Documents/Projects/`; other paths are TUSB32x-relative.
This report records inspected sources and recommendations, not build results.

## Current revisions and scope

Read `git rev-parse HEAD`, `git branch --show-current` and
`git status --porcelain=v1` independently before writing this report.

| Repository | Full HEAD | Branch | Initial worktree |
| --- | --- | --- | --- |
| TUSB32x | `93f2d31374dc16bcc95800d0e0c516c4372faa4a` | `promptset/tusb32x-implementation-20261023` | Clean |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `main` | Clean |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | `main` | Clean |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `main` | Clean |
| INA3221 | `41b6184b75f3c152611d4236f7ea41a5ce77bc2c` | `main` | Clean |
| OPT4001 | `dab818e575fc25a0949d962bc0df600eb2ed5200` | `main` | Clean |

These are local snapshots, not upstream or CI claims. Reopened parent and local
`AGENTS.md`, every listed peer's `AGENTS.md`, prompt index, `COMMON.md`,
`8-metadata-versioning.md`, `MAINTAINABILITY.md`, `AGENT_WORKFLOW.md`,
`ENGINEERING_BASELINE.md` layout/version rules and `ENGINEERING_RULES.md`
ownership/distribution rules. Revisited progress and design decisions rather
than treating earlier audit reports as current source evidence.

The starting TUSB32x tree contains four public headers, `src/TUSB32x.cpp`, a
tests-OFF CMake library, registered native tests and `examples/host/basic/`.
There are no starting `scripts/`, `tools/`, `.github/`, package manifests,
generated Version.h or framework examples. Inspected `CMakeLists.txt`,
`test/CMakeLists.txt`, `include/TUSB32x/{TUSB32x,Config,Status,CommandTable}.h`,
`src/TUSB32x.cpp` and the host example's CMake/source files. Only passive values,
Status helpers and pure validators are currently implemented. Package text must
not advertise a device facade, CLI, runner or linked firmware as completed.

## Generator and fixture findings

| Exact source | Current behavior | Adopted or rejected choice |
| --- | --- | --- |
| `TMP1x2/scripts/generate_version.py:254` `_expected_outputs`; `24Cxx/scripts/generate_version.py:241` | Both derive generated Version.h, component version and Doxyfile number from library.json; TMP1x2 additionally updates the CMake project version. `_sync_outputs` avoids writing equal text. | Adopt one manifest truth and deterministic sync/check/bump/set. Require every TUSB32x version field, including CMake, to participate. Test byte stability rather than assuming text equality preserves all input bytes. |
| `TMP1x2/scripts/generate_version.py:86` `_resolve_namespace_dir`; `24Cxx/scripts/generate_version.py:86` | TMP1x2 guesses a namespace from directory/Version.h candidates. 24Cxx uses the manifest's declared public header and checks its existence. | Prefer a fixed TUSB32x target or validated manifest header. Reject stale generated-header discovery as identity. |
| Both scripts: `SEMVER_RE`, `_expected_outputs`, `main` | Numeric regex accepts leading zeroes; missing version silently becomes `0.0.0`; sync/check ignore extra arguments. Invalid data can escape as a traceback. Numeric version encoding is `major*10000+minor*100+patch` without bounded components. | Require a present string version, canonical decimal components, bounded representation if integers are exposed, validated command arity and documented failure exits. Do not copy an ambiguous numeric encoding or unused compatibility aliases. |
| `TMP1x2/scripts/generate_version.py:113` and `24Cxx/scripts/generate_version.py:100` `_get_git_info` | Git is invoked at the source path without proving it is the repository root. Their PlatformIO hooks also inject current build timestamps and Git state. A source archive beneath another repository can inherit that repository's identity. | Reject this identity path. Stable library version output needs no Git, timestamps, build hooks or identity macros. Removing Git discovery entirely makes no-Git and enclosing-Git safety direct and testable. |
| `TMP1x2/include/TMP1x2/Version.h`; `24Cxx/include/EEPROM24Cxx/Version.h` | Checked-in generated headers carry version values plus overridable BUILD/GIT macros and a VERSION_FULL string. | Adopt checked-in generated version documentation and independent inclusion; reject build identity and compatibility aliases in the stable TUSB32x library header. Firmware identity belongs to later example builds. |
| `TMP1x2/tools/test_version_sync.py:14` `VersionSyncTests` | One temporary-copy test changes version to 2.7.13, observes stale check, syncs and checks CMake/component/Doxygen/header. No same-named version fixture exists under current `24Cxx/tools/`. | Adopt isolated fixtures, but cover missing/malformed versions, independent drift, argument exits, repeated sync, no Git, enclosing Git and dirty example source as prompt 8 requires. The peer suite does not already prove those cases. |
| `TMP1x2/CMakeLists.txt:51` | Python 3.10 is found only inside `TMP1X2_BUILD_TESTS`; version fixture is a real CTest test. | Register Python fixtures only in the tests-enabled path; keep ordinary tests-OFF compilation independent of Python. |

## Package and component choices

`TMP1x2/tools/check_package.py:62` checks real archive members before writing a
renamed extraction. It rejects POSIX/Windows absolute paths, traversal,
backslashes, colons, duplicate names, links and other non-file members, then
builds and runs a separate CMake consumer. Adopt the real-artifact and renamed
consumer principle; full extraction/consumer hardening remains prompt 32.

`24Cxx/tools/check_package_contents.py:15` lists real consumer requirements and
checks forbidden research/build material and packaged Markdown links. Its
dictionary construction loses duplicate-member evidence and it does not perform
the same Windows-path checks. Do not copy that as a complete extraction guard.
Its workflow builds the extraction, whereas TMP1x2's checker also executes a
consumer. `OPT4001/tools/check_clean_consumer_package.py` packs and builds a
temporary native PlatformIO consumer but does not execute it. Preserve those
evidence distinctions.

`TMP1x2/library.json` and `24Cxx/library.json` use explicit export inclusions
with generated-example exclusions; their broad `docs/*.md` and `examples/**`
patterns are inappropriate as an unreviewed initial TUSB32x package.
`INA3221/library.json` has no explicit export selection;
`OPT4001/library.json` uses exclusions only. Reject both as the initial package
policy. `LDC1614/library.json` includes a vendor datasheet and version script;
reject those inclusions for the TUSB32x normal package.

`LDC1614/CMakeLists.txt:1` unconditionally finds Python, executes the version
checker and then calls `idf_component_register`. Reject its consumer-time
maintainer dependency and component-only entry. Adopt explicit real source
registration and PUBLIC C++17 from that file and
`INA3221/CMakeLists.txt` / `OPT4001/CMakeLists.txt`. Adopt the conditional
`if(COMMAND idf_component_register)` branch from `TMP1x2/CMakeLists.txt` and
`24Cxx/CMakeLists.txt`, preserving a normal host target and tests OFF.
The current core requires no Arduino, I2C driver, FreeRTOS or consumer-product
component dependency. Component registration cannot prove a native firmware link.

Recommended initial version: `0.1.0`, explicitly an unreleased development
package containing passive contracts and validators. Use name `TUSB32x`, public
header `TUSB32x/TUSB32x.h`, `includeDir: include`, `srcDir: src`, no framework
dependencies and no board/architecture support whitelist copied from a sensor.
The repository's actual origin is `https://github.com/janhavelka/TUSB32x.git`,
confirmed by `git remote -v`; its URL need not be guessed from another package.

For PlatformIO, prefer an explicit list of currently maintained consumer files:
the five public headers including generated Version.h, `src/TUSB32x.cpp`,
`CMakeLists.txt`, `library.json`, `idf_component.yml`, `LICENSE`, README and
CHANGELOG, plus the existing host example and reviewed maintained integration
guides needed by packaged links. Exclude all vendor/reference payloads, audits,
prompts, test/tool/script machinery and generated build outputs. New future
source/header/example files must join this list when implemented. The lead must
reconcile README/example links with the actual package; a working checkout link
alone does not establish a self-contained archive.

The IDF manifest's `files.include` is not a restrictive allowlist. Official
[component manifest documentation](https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html#files)
states that files start included, exclusions run first, and include patterns
re-include matches. To express the same minimal consumer set, exclude `**/*`
then re-include the explicit consumer paths, and verify actual manager behavior.
Do not assume PlatformIO and IDF selection fields have identical semantics.
Official [PlatformIO export documentation](https://docs.platformio.org/en/latest/manifests/library-json/fields/export/index.html)
also directs maintainers to inspect the `pio pkg pack` result.

## Ownership, format and documentation evidence

All five current `library.json` files identify Thymos Solution s.r.o,
`info@thymos.cz`, `https://www.thymos.cz`, and Jan Havelka as maintainer.
All five LICENSE files contain MIT terms and copyright 2026 Thymos Solution
s.r.o. This agrees with TUSB32x `ENGINEERING_RULES.md:32` and the existing
maintenance/license decision; preserve that project ownership and the full MIT
terms. It does not relicense any vendor archive. TUSB32x's recent Git authors
are Jan Havelka, independently confirming the maintainer name.

`INA3221/CODEOWNERS` uses `info@thymos.cz`; `OPT4001/CODEOWNERS` uses
`@janhavelka` and explains GitHub email resolution. Prefer that evidenced username
for review ownership; no invented team or second reviewer. The actual repository
remote corroborates the account name, without proving GitHub access permissions.

`LDC1614/.clang-format`, `INA3221/.clang-format` and `OPT4001/.clang-format`
use two-space indentation, attached braces and 100 columns. Adopt the simple
style, reject framework-first include categories in core. `LDC1614/.editorconfig`
uses UTF-8/LF/final newline; add four-space Python indentation under the local
maintainability rule instead of inheriting its two-space default for Python.

`LDC1614/Doxyfile` enables `EXTRACT_ALL=NO`, static constants, public inputs,
undocumented/parameter/document-error warnings and failure on warnings.
Adopt these omission-sensitive settings with output under `build/`.
`TMP1x2/Doxyfile` uses `EXTRACT_ALL=YES` and hides undocumented classes; reject
that combination as proof comments exist. Its
`tools/check_doxygen.py` provides useful bounded invocation, public HTML/XML
visibility checks and internal link checks, but the complete TUSB32x checker and
negative fixtures remain prompt 33. Basic Doxygen execution now must inspect
visible current public types rather than claiming the future omission guard.

## Tools, dependency ranges and example identity

Reopened all five `.github/workflows/ci.yml` files as source. TMP1x2/24Cxx
pin PlatformIO 6.1.19 and IDF containers 5.3.2/5.5.1/6.0.1 on S2/S3;
their native jobs use Ubuntu 24.04, strict GCC warnings and ASan/UBSan.
LDC1614 uses IDF 6.0.2; INA3221/OPT4001 use 6.0.1. LDC1614's
`requirements-dev.txt` pins PlatformIO 6.1.19 and pyserial 3.5. Other named peers
have no requirements-dev file. OPT4001's workflow selects Python 3.12;
LDC1614/INA3221 select 3.11. These are source-declared inputs, not observed green
runs or installed local versions. Do not copy moving action majors or add a
workflow before its assigned stage.

All five root `platformio.ini` files select pioarduino archive 55.03.311 for
Arduino; TMP1x2/24Cxx native-IDF example configs use that archive as well.
The [upstream release](https://github.com/pioarduino/platform-espressif32/releases/tag/55.03.311)
identifies Arduino 3.3.11 based on IDF 5.5.5. This verifies the pin's identity,
not TUSB32x adapter compatibility. Use a real native-only config now if desired:
TMP1x2/24Cxx/OPT4001 pin `platformio/native@1.2.1`; the
[official native platform guide](https://docs.platformio.org/en/latest/platforms/native.html)
requires an existing host GCC toolchain and distinguishes building from
`pio run -t exec`. Do not manufacture Arduino/IDF firmware stubs to fill a matrix.
The [published PlatformIO 6.1.19 package](https://pypi.org/project/platformio/6.1.19/)
confirms that peer tool version exists; a newer release does not require changing
this reviewed pin. Windows must use the existing managed installation.

`TMP1x2/scripts/pio.cmd` and `LDC1614/scripts/pio.cmd` both quote
`%USERPROFILE%\.platformio\penv\Scripts\pio.exe`, forward `%*` and preserve
`%ERRORLEVEL%`; missing executable fails explicitly. Adopt these mechanics and
the local no-second-install rule. No wrapper invocation occurred in this audit.

TMP1x2/24Cxx declare IDF `>=5.3.0`; LDC1614/INA3221/OPT4001 declare `>=6.0.0`.
For the current framework-neutral subset, `>=5.3.0,<7.0.0` is a conservative
proposed admission range grounded in two current peers and their 5/6-series
source matrix; it is a project policy, not a tested TUSB32x support claim.
The [IDF 5.3.2 build-system guide](https://docs.espressif.com/projects/esp-idf/en/v5.3.2/esp32/api-guides/build-system.html)
documents explicit component SRCS/INCLUDE_DIRS registration. There is no current
TUSB32x backend requiring IDF 6 or any target restriction. Document the selected
range as unqualified pending real links and revisit it at adapter/package stages.
No core build needs the SDK solely because this metadata exists.

Reopened `TMP1x2/examples/common/Tmp1x2CliOutput.cpp:24`,
`TMP1x2/tools/hil_tmp1x2_runner.py:438,828`,
`24Cxx/examples/common/Eeprom24CxxCli.cpp:141` and
`24Cxx/tools/hil_identity.py`. They show that firmware-reported identity is
separate evidence from the host checkout; TMP1x2's runner checks requested
commit/framework/target/clean fields. Preserve that separation in later work.
`TMP1x2/examples/esp_idf/basic/main/BuildIdentity.cmake` verifies a source-local
`.git`, resolves the Git top level and refreshes example identity at build time;
`generate_identity.py` invokes it with a timeout. This is a better later-example
reference than the library generator's unqualified Git calls. No TUSB32x CLI,
runner, flashed identity or HIL support is introduced by prompt 8.

## Evidence limits and lead follow-up

PASS: current peer revisions/status, source inspection, ownership comparison and
primary-source checks for manifest semantics and pin identities. No peer build,
generator execution, package extraction, SDK link or physical test was run.
The audit recommends decisions; it does not establish prompt 8 completion.

The lead owns final metadata/range selection, fixture implementation, Doxygen
visibility, real package-content inspection, tests-OFF consumer evidence,
documentation, commit and synchronization. IDF archive-tool behavior and actual
firmware links remain distinct checks. No hardware or release publication is
authorized or claimed here.
