# 32 — Prove safe archives and clean package consumers

Deliver a distributable archive with tested contents and consumers that work
outside this checkout. Source builds and manifest text are not package evidence.
This stage proves package mechanics; prompt 34 runs the full supported CI matrix.

## Read, inspect and re-audit

Read [COMMON](../../prompts/COMMON.md), [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md), packaging/boundary
sections of [ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md) and
[ENGINEERING_RULES](../../prompts/ENGINEERING_RULES.md), plus
[MAINTAINABILITY](../../prompts/MAINTAINABILITY.md). Prompts 1–31 supply versioning, core, shared
CLI, Arduino/native-IDF examples, generic consumer and runner evidence. Inspect
current manifest/CMake/generated-file behavior and example build commands first.

Re-audit TMP1x2 check_package.py and version generation, 24Cxx package links/clean
consumers, OPT4001 archive checks and LDC1614/INA3221 native-IDF image builds.
Record HEAD, dirty state, exact files and choices. Inspect current official tools
when changing package/build pins. Sibling clean-build success proves nothing here.

## Build and verify the package

1. Keep library.json as version truth and committed Version.h stable. Archives
   compile with honest unknown Git identity; downstream core configure/link must
   need neither Git, Python, Doxygen, test frameworks nor platform headers. Tests
   default OFF. Firmware identity generation remains an example-build concern.
2. Define explicit package include/exclude rules covering public/core files,
   metadata/licenses, shared CLI/console/board files, example CMake/sdkconfig
   defaults and useful documentation. Exclude builds/.pio/caches, local sdkconfig
   and locks, binaries/logs/private evidence, research PDFs and vendor source.
   Preserve required licenses and needed example build scripts; blanket exclusion
   of Python can break firmware identity. Core consumers still need no generator.
3. Add tools/check_package.py and tools/test_check_package.py. Reject archive
   traversal, absolute/drive/backslash paths, duplicate names, links and special
   files; also reject case-insensitive collisions, colon/alternate-stream segments,
   Windows device names and trailing-dot/space aliases. Validate portable names
   before extraction; bound member count and individual/total bytes. Extract into a new
   verified temporary root, rename the library directory and validate exact
   required/forbidden members. No move/delete outside that root. Missing shared
   example files must fail; extraction must not follow archive-created links.
4. Add test/package_consumer/CMakeLists.txt and main.cpp. Copy only archive and
   fixture into a clean workspace, with no sibling/check-out include paths, .git,
   build caches or generation tools. Directly include every public header, bind
   fake transport, run a cooperative operation, compile, link and execute against
   the renamed extracted library. Make accidental source-checkout use observable.
5. Required CMake usage is add_subdirectory() with target TUSB32x. Claim install/
   export/find_package only if implemented and tested after moving the install
   prefix. Do not add installation infrastructure merely to increase scope.
6. Build extracted-package Arduino and genuine native-IDF consumers for S2/S3
   with the working configurations established by prompts 23–26 and separate build/
   SDKCONFIG paths. Link real images
   from the extracted shared sources without sibling/source-checkout paths. Check
   native app_main/APIs and absence of Arduino linkage. Record archive hash,
   extracted paths, tool versions and actual commands. Header compilation alone
   is insufficient. Prompt 34 extends this to every declared SDK/toolchain row.
7. Check local Markdown links both in repository and extracted archive. Excluded
   audit/vendor/test files must not leave broken package links; clearly label
   repository-only paths or use verified official URLs. Manifest platform/
   framework claims must match proven portable core and tested adapters.

## Focused acceptance

Test malformed/Windows-aliased paths, links, case collisions, oversized members/counts, missing
members and forbidden content before accepting a valid real archive. Assert safe
failure leaves files outside the extraction root untouched. Test tests-OFF clean
configuration does not discover Python/Doxygen/test dependencies. Register package
tests with finite timeouts and keep their fixtures bounded.

Use the existing Windows scripts/pio.cmd to pack/build; do not install another
PlatformIO Core. Execute the clean CMake consumer and extracted Arduino/native-IDF
links with explicit evidence. A single extracted-package firmware matrix can
satisfy both distribution and firmware proof for that revision; do not require an
identical second source matrix. Missing tools are NOT RUN/BLOCKED for those rows,
not successful checks or authority to weaken declared support.

## Delegation, docs and completion

Optional: delegate portable consumer fixtures or read-only archive/link review to
a general coding agent in disjoint files. Use stronger reasoning for extraction
path safety if uncertain. The lead owns content rules, reviews results and handles
Git. Do not mutate sibling repositories or run destructive cleanup outside the
verified task directory.

Update package/CMake examples, README, CHANGELOG, CONTRIBUTING/release guidance,
supported tool table, source Doxygen/docstrings, coverage, validation and progress.
Preserve actual owner/maintainer metadata; invent no hosting URL. Do not publish
tags/registries/releases without the separate authorization. Finish all local
package defects, commit/sync coherent checked units and the completed prompt on
the existing execution branch, and verify upstream equality. Report clean archive,
portable execution and linked framework evidence separately with remaining limits.
