# P03 — Build the repository and minimal host consumer

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2/24Cxx
layout and generated metadata, INA3221/OPT4001 package guards, LDC1614 component
build and author/license conventions. Inspect workflows before adding build files.

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

## Deliver and check

- Native configure/build/test succeeds; public includes and names match contract.
- Version sync is idempotent and `check` detects deliberate metadata drift in a
  temporary fixture. Package/build configuration does not glob reference code.
- No Python/Git/platform dependency is needed merely to compile delivered C++.
- Record the actual tool versions and commands; defer firmware support claims
  until the corresponding examples link. Commit/push only under actual repo rules.
