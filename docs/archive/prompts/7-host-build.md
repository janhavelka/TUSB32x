# 7 — Build the portable values and validators

Prerequisites: prompts 1–6 are complete. Read [COMMON](../../prompts/COMMON.md), the baseline's
file/build/type responsibilities and [MAINTAINABILITY](../../prompts/MAINTAINABILITY.md).
Finish a real C++17 library target, implemented local validators and a standalone
host consumer. The host build must pass without any driver or hardware stubs.
Metadata/version tools are the next prompt's separate task.

## Focused re-audit

Recheck TMP1x2/24Cxx layout and optional test builds, LDC1614 component CMake,
ADS1115 variant/profile validators and INA3221/OPT4001 consumer checks. Inspect
current AGENTS, source lists, compiler flags, test registration and workflows.
Record HEAD, dirty state, file paths and choices. Do not inherit unnecessary
consumer dependencies from a stricter peer build.

## Implement one small usable target

1. Add `CMakeLists.txt` target `TUSB32x` with C++17 PUBLIC, correct public include
   directory and explicit source list. No recursive source glob, vendor/reference
   code, sibling paths, Arduino, IDF, FreeRTOS or platform clock/logger dependency.
2. Implement `validateDriverConfig` and `validateProfile` against the frozen
   variant/type rules. Check required callbacks, explicit variant/address pair,
   nonzero timeout, legal enums and capability combinations. Reject unsupported
   valid settings distinctly from invalid configuration. Separate value semantics
   from apply's later changed-field write capability checks; unchanged proved
   322I advertisement must not disable all other supported writes. These
   functions perform no callback, allocation or device mutation.
3. Publish these callable declarations with their real definitions and Doxygen.
   Preserve the already implemented Status/passive types. Other facade methods
   remain planned outside production callable headers until their implementation
   prompt; do not fill missing work with success or UNSUPPORTED_FEATURE stubs.
4. Add only source files with current responsibilities. Adjacent small private
   responsibilities may share a unit; do not create the entire future tree empty.
   Keep helpers private and public headers independently includable.
5. Add an actual minimal consumer under `test/consumer/` or
   `examples/host/basic/`, using values and validators that now exist. Compile and
   link multiple translation units with direct public includes. Explicitly fake
   application callbacks may count attempts but must never imply a real device.
   Full owner examples arrive after the driver exists.
6. Make tests optional with `TUSB32X_BUILD_TESTS` default OFF. Register genuine
   compile-contract and validator tests with CTest, including the baseline
   `tusb32x_compile_contracts` suite. Do not register a build-only executable as
   a passing behavioral test or accept zero discovered tests as success.

## Prove the boundary now

Use supported strict warnings and keep assertions active in Release checks.
Test valid and invalid variant/address pairs, missing callbacks, zero timeout,
invalid enum values and variant-specific profile restrictions. Exercise error
helpers, preserved backend detail, constexpr/header visibility and independent
includes. No validation path may invoke transport; prove this with counting
callbacks rather than a source-text search.

Establish and record the actual commands:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Also configure/build a clean ordinary consumer with tests OFF. Delivered C++
must compile/link without Python, Git, Doxygen, PlatformIO, framework headers or
sibling paths. A source checkout nested beneath another Git repository must not
need its identity. The native consumer is evidence of portable linkage only;
do not claim IDF/Arduino firmware or hardware qualification.

## Documentation, delegation and finish

Add source file briefs and complete public function/type/member Doxygen with
units, output/error semantics, ownership and zero-I2C bounds. Update architecture,
API/feature coverage, README, CHANGELOG Unreleased, CONTRIBUTING and AGENTS with
the commands and implemented subset that now exist. Keep planning claims labeled.
Record exact tool versions, commands/results and any unavailable checks in
progress/validation.

Optional delegation: a general coding agent owns consumer/negative fixtures after
validator signatures are fixed, while the lead owns implementation/build lists.
A read-only reviewer checks downstream dependencies. Give disjoint files and
follow AGENT_WORKFLOW; only the lead handles Git.

Review the full diff and run relevant native checks. Fix local build/test failures
without suppressing assertions or weakening contracts. Commit and sync coherent
checked units and the completed prompt on the existing execution branch. Verify
upstream synchronization, report results/limits, and then proceed to prompt 8.
