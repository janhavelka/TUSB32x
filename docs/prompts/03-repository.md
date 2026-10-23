# P03 — Build the repository and minimal host consumer

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2/24Cxx
layout and generated metadata, INA3221/OPT4001 package guards, LDC1614 component
build and author/license conventions. Inspect workflows before adding build files.

Use the concrete file responsibilities in
[ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md) and the source/documentation
rules in [MAINTAINABILITY.md](MAINTAINABILITY.md). Start small: compile the passive
types and real validators before introducing the protocol or operation engine.

## Work

1. Create the agreed public headers and small coherent implementation units.
   Implement passive types/validators and build a minimal portable core; do not
   add success-returning hardware stubs masquerading as completed features.
2. Add `library.json`, CMake host/IDF component modes, `idf_component.yml`,
   PlatformIO configuration, license/ownership, formatting, Doxyfile and changelog.
   Reuse evidenced names/maintainer identity, not peer repository URLs. Set an
   honest development version; support claims must track implemented features.
3. Create `scripts/generate_version.py` with sync/check/bump/set; update generated
   Version.h, component/CMake/Doxygen versions from manifest truth. Make generation
   stable and safe outside Git; never borrow enclosing repository identity.
4. Create `scripts/pio.cmd` using the installed VS Code-managed PlatformIO. Pin
   dependencies based on current audited working references and official support.
   Keep board pins and USB flags in examples/build environments, out of core.
5. Add a minimal CMake consumer that compiles/links core without Arduino/IDF or
   sibling include paths. Tests optional/off for downstream users. Add real tests
   for current validators and error types, and a strict compile target.
6. Update root AGENTS with actual commands and architecture. Keep planning/audit
   documents clearly distinguished from implemented behavior.

## Concrete build and repository contract

| File or area | Implement now |
| --- | --- |
| `include/TUSB32x/Status.h` | Chosen Err/Status/errorName contract with meaningful Doxygen for codes and methods |
| `include/TUSB32x/Config.h` | Exact callback typedefs, DriverConfig and ownership/timeout documentation; no board pins |
| Main public header and baseline type headers | Real implemented passive API only; explicit units/defaults and no successful hardware stubs |
| `src/` | Focused implementation files, explicit CMake source list and no reference/vendor source glob |
| `CMakeLists.txt` | Target `TUSB32x`, C++17 PUBLIC requirement, correct PUBLIC include directory; native tests behind `TUSB32X_BUILD_TESTS` default OFF; IDF mode only when `idf_component_register` exists |
| `library.json` | Name/headers/source/include dirs, evidenced owners/license, honest description and explicit package content |
| `idf_component.yml` | Name/version/dependencies consistent with portable core; no Arduino or consumer-product requirement |
| `scripts/generate_version.py` | sync/check/bump/set, deterministic generated text, no-op sync leaves files unchanged, version propagation to all required metadata |
| `scripts/pio.cmd` | Existing `%USERPROFILE%\.platformio\penv\Scripts\pio.exe`; preserve exit code; report missing installation |
| `Doxyfile` and `tools/check_doxygen.py` | Public docs visible, undocumented parameters/types detected, bounded execution, negative fixtures per maintainability contract |
| `.editorconfig` / `.clang-format` | Agreed simple formatting; no framework include preference in core |
| `test/consumer/` | Standalone native direct-include/multiple-translation-unit consumer; no sibling paths |

Do not create every future source file as an empty placeholder. Add it when it
has an implemented responsibility and register it in both applicable build modes.
Examples, scripts and test requirements are not transitive core dependencies.

Proposed commands to establish during this stage (not prior test results):

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
python scripts/generate_version.py sync
python scripts/generate_version.py check
python tools/check_doxygen.py
```

On a native consumer with tests disabled, CMake must not require Python, IDF,
PlatformIO or Doxygen. Treat the IDF branch as component registration, not the
default host build. Inspect the current peer CMake before copying; some peer
requirements are stricter than this new portable package needs.

Version tests use temporary fixtures: missing version, malformed semver, version
drift in CMake/component/Doxygen, no Git, extracted tree nested beneath an unrelated
Git repository, dirty example source and repeated sync. Keep library version
headers stable; volatile build timestamp/commit belongs to example firmware.

## Documentation in this stage

Document each created header/source's responsibility and every public type/value,
field and function in Doxygen. The generated Version.h template produces its own
comments. Give scripts module docstrings and precise CLI exit behavior. Create
or update README, CHANGELOG Unreleased, CONTRIBUTING, examples/docs indexes and
AGENTS with the commands that now exist. Mark future features as planned. Do not
put a README claim ahead of its implementation or silence Doxygen warnings.

## Deliver and check

- Native configure/build/test succeeds; public includes and names match contract.
- Version sync is idempotent and `check` detects deliberate metadata drift in a
  temporary fixture. Package/build configuration does not glob reference code.
- No Python/Git/platform dependency is needed merely to compile delivered C++.
- Record the actual tool versions and commands; defer firmware support claims
  until the corresponding examples link. Commit and sync completed units under
  AGENTS.md and COMMON, keeping the execution branch created at the start.
