# P12 — Complete clean packaging, CI and release tooling

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 clean
package consumer and version guard, 24Cxx package documentation links, OPT4001
clean-consumer/action-pin checks, LDC1614/INA3221 native-IDF/native test registration.
Inspect current official tool documentation when selecting or changing pins.

## Work

1. Make manifest claims match actual framework-independent core and tested adapters.
   Define package includes/excludes explicitly: public headers, core, needed example
   common files, metadata, license and useful docs; exclude builds, logs, local
   paths, .pio, sdkconfig artifacts, research PDFs/vendor code and private evidence.
2. Pack then safely extract into a renamed clean temporary directory. Compile/link/
   run a downstream CMake consumer without sibling paths, .git or a source-version
   generator prerequisite. Build an extracted-package framework consumer too.
   Check public direct includes and docs links; excluded repository-only references
   must be labeled or use official web links rather than broken package links.
3. Distinguish proven CMake add_subdirectory use from install/find_package support.
   Implement/test install exports only if claimed; do not advertise a nonexistent
   target/config package. Tests and Doxygen remain optional for library consumers.
4. Complete version/metadata consistency, framework/timing boundary, strict compile,
   CLI parity, runner tests, package-content and Doxygen guards. Test guard failure
   paths with fixtures. Avoid brittle text checks as substitutes for executable tests.
5. Configure CI with actual registered native behavioral suites, CLI/console tests,
   Python parser/live-fixture tests, strict warnings/sanitizers, version/metadata
   checks, docs, clean packages, Arduino S2/S3 and native IDF S2/S3 supported versions.
   Verify generated artifacts and test names; a file in test/ is not registration.
6. Pin actions/toolchains appropriately, set least required workflow permissions,
   job timeouts and bounded artifacts. Use the existing Windows wrapper locally;
   an isolated pinned CI install is a different environment, not a second local Core.
7. Add concise CONTRIBUTING/release guidance and valid owner metadata. Preserve
   copied MIT notices. Use real established repository URLs/owners only. Do not
   automatically publish a registry package/tag/release from a planning instruction.

## Tests and completion

- Run the full applicable local validation and actual package consumers. Test
  generated metadata drift, missing shared-example files, unexpected build files
  and absent sibling sources. Builds from an empty native consumer must work.
- For a configured remote, follow parent policy and verify CI for the exact final
  relevant commit after meaningful changes. If unavailable, record the blocker;
  do not claim configured workflows passed or weaken gates.
- Document version matrix, compiler/SDK/tool pins, commands, artifacts and actual
  results. Ensure validate.py rejects missing mandatory checks and wrong return codes.
