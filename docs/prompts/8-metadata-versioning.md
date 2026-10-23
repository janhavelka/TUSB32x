# 8 — Add package metadata and deterministic version tools

Prerequisites: prompts 1–7 are complete and the native build passes. Read
[COMMON](COMMON.md), the baseline's package/layout rules and
[MAINTAINABILITY](MAINTAINABILITY.md). Finish version truth, development metadata,
native-IDF component registration and the managed PlatformIO launcher. The full
Doxygen omission checker is prompt 33; source documentation remains required now.

## Focused re-audit

Recheck TMP1x2/24Cxx version generators and package guards, LDC1614 component
CMake, INA3221/OPT4001 packaging and current author/license conventions. Inspect
AGENTS, manifests, generated files, workflows and actual scripts/tests. Record
revisions, dirty state, sources and choices. Verify proposed dependency/tool
pins against current working peers and official support; do not copy repository
URLs or claim support merely from a manifest entry.

## Complete the metadata and generator

1. Add `library.json` as version truth, with name TUSB32x, accurate public headers,
   source/include directories, evidenced maintainer/license and explicit package
   content. Choose an honest development version. Exclude reference/vendor
   artifacts, audit machinery and unrelated build outputs from normal packages.
2. Add `idf_component.yml` with consistent version/dependencies and no Arduino or
   consumer-product requirement. Extend CMake component registration only when
   `idf_component_register` exists; default host configuration remains ordinary
   CMake. Use explicit real sources in both modes. Component registration alone
   is not proof of linked native-IDF firmware.
3. Implement `scripts/generate_version.py` with sync/check/bump/set, deterministic
   output and explicit error/exit semantics. Derive generated Version.h and all
   required CMake/component/Doxygen version fields from manifest truth. No-op
   sync changes no bytes. Do not hand-edit generated version headers.
4. Make generator behavior safe outside Git and beneath an unrelated enclosing
   repository. Do not borrow host/parent identity. Stable library version headers
   contain no volatile build timestamp/commit; later example firmware supplies
   its own build identity. No Git/Python requirement is added to consumer linkage.
5. Add `.editorconfig`, `.clang-format`, license/ownership files and a small
   Doxyfile matching the maintained C++ API. Prefer evidenced simple formatting;
   no framework-first include preference in core. Doxygen uses visible public
   inputs and warnings for missing documentation, not suppression/EXTRACT_ALL
   to conceal omissions. The complete negative-fixture guard belongs to prompt 33.

## Launcher and fixture tests

Create `scripts/pio.cmd` around the existing
`%USERPROFILE%\.platformio\penv\Scripts\pio.exe`; pass arguments faithfully,
preserve its exit code and report a missing installation clearly. Do not install
a second PlatformIO Core. Keep board pins and USB flags out of core and in the
later real example build environments. Record selected tool/dependency pins and
place usable PlatformIO configuration where current implemented sources permit;
do not invent successful firmware stubs to fill future environments.

Create `tools/test_version_sync.py` with temporary generator fixtures; prompt 33
extends this same file rather than creating a second suite. Cover missing version, malformed semver,
CMake/component/Doxygen drift, no Git, extracted tree nested under another Git
repository, dirty example source and repeated sync. Test bump/set argument
validation and exact failure exit codes. The dirty source must not alter stable
library version text or be mistaken for flashed firmware identity. Keep fixture
mutations outside repository metadata and register real Python tests where the
current test build can run them without requiring Python for tests-OFF consumers.

Establish these commands without treating their existence as prior success:

```text
python scripts/generate_version.py sync
python scripts/generate_version.py check
```

Run generator tests, the existing native CMake/CTest checks affected by metadata
and a plain tests-OFF consumer. Verify manifest content rules exclude reference
code and that source lists remain explicit. If Doxygen is installed, run the
current configuration and inspect visible public documentation; report unavailable
tools honestly. Do not claim the omission guard or later firmware matrix exists.

## Documentation, delegation and finish

Generated Version.h includes accurate generated Doxygen. Give Python modules and
exported helpers docstrings for inputs, outputs, temporary files and exits. Update
README, CHANGELOG Unreleased, CONTRIBUTING, docs/examples indexes and AGENTS with
actual version/build commands. Preserve license terms and evidenced ownership;
do not invent release dates, contributors or tested support. Review affected
API/architecture/coverage and record progress, decisions and validation.

Optional delegation: a general coding agent owns generator fixtures after its
format is fixed; another reviews packaging/component registration read-only.
The lead owns shared metadata/version changes, integration and Git. Keep scopes
disjoint. Commit and sync completed tested units, then the complete prompt, on
the same execution branch. Verify upstream equality and report actual results
and remaining evidence limits before the next numbered prompt.
