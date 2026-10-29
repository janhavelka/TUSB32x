# Current library comparison

Prompt 1 execution audit, 2026-10-23; inventory refreshed by prompt 37 on
2026-10-29. The parent Projects directory was inspected
read-only. The 17 independent I2C libraries and FieldCore-node were compared by
actual declarations, implementations, tests, build definitions and guidance.
No sibling fetch, modification, build, firmware run or remote CI check was made.

## Current evidence

| Report | Evidence |
| --- | --- |
| [Execution inventory](audit/execution-1-inventory.md) | Included/excluded projects, source layouts, metadata, language and declared build differences |
| [Machine-readable inventory](audit/execution-1-inventory.json) | Full revisions, branches, dirty state, manifests, ownership and inspected paths |
| [Owner and consumer audit](audit/execution-1-owner.md) | Actual lifecycle, callback, status, health, trust, timing and FieldCore contracts |
| [CLI and tooling audit](audit/execution-1-tooling.md) | CLI/adapters, runner framing, CTest/workflows, documentation, packaging and generated versions |
| [Engineering decisions](design-decisions.md) | Concern, peer symbol/revision, selected rule, reason and required check |

The original [preparation inventory](audit/repository-snapshot.json) and reports
remain dated evidence. Prompt 37 still finds 17 independent I2C libraries among
47 immediate project directories, all clean on main. Six library HEADs changed
since prompt 1, although their inspected production/test/build scopes did not.
FieldCore-node is clean on rodtester at
`5ad9292dc8481cf08be9a61f662495a6474d1da0`. The
[final core review](final-audit.md) records current revisions, changed exclusions
and freshly inspected behavior. Never interpret local snapshots as confirmation
that sibling remotes are synchronized. TUSB32x is excluded as the repository
being reviewed, not as a still-empty planning project.

## Selected baseline

Use the exact [engineering baseline](prompts/ENGINEERING_BASELINE.md),
[CLI contract](prompts/CLI_CONTRACT.md) and
[maintainability rules](prompts/MAINTAINABILITY.md). The current source evidence
supports their direction. The decision table records explicit exclusions instead
of asking later agents to invent a fresh architecture.

- Core: standard C++17, TUSB32x namespace/class, structured Status, static error
  strings, borrowed callbacks, fixed storage and no platform/service ownership.
- Operations: bus-silent binding/admission/cancel/end, external owner time,
  token-correlated retained results, bounded polling and conservative effects.
- Evidence: desired/observed/verified configuration, port validity, transport
  history and passive health stay separate. Reconciliation never replays writes.
- Examples: shared neutral CLI and application adapters; native IDF remains
  native. Board pins, addresses, straps and resource ownership are explicit.
- Tooling: behavioral tests registered when introduced, Python sessions tied to
  actual firmware evidence, renamed extracted consumers and generated versions.
- Maintenance: simple source with useful Doxygen and general docs updated with
  behavior; one execution branch and checked commits/sync after each prompt.

This is a synthesis of peers, not an assertion that they implement one uniform
API. For example, ADS1115 native/IDF uses C++11 while TUSB32x selects C++17;
most peer root CMake files are component-only rather than standalone consumers.
TMP1x2's public-API visibility checks are useful, but its EXTRACT_ALL setting is
not proof of complete comments. LDC1614/INA228 supply stricter documentation
settings. TUSB32x now has real-parser missing-comment negative fixtures from
prompt 33; the initial audit did not find those fixtures in either peer.
No library is copied whole.

## Prompt 1 tool availability (historical)

Checked in the active Windows PowerShell environment with executable resolution
and version commands. These observations do not prove a compiler/SDK can build
the future library or a board image.

| Tool | Observed result | Selected use or limitation |
| --- | --- | --- |
| Python | 3.12.10, user Python312 installation | Local audit/check scripts |
| CMake / CTest | 4.0.1, WinGet WinLibs mingw64/bin | Host configuration/test tooling when code exists |
| g++ | MinGW-W64 GCC 15.1.0, same WinLibs bin | Available host compiler; no compilation claimed here |
| Ninja | 1.12.1, same WinLibs bin | Available build backend |
| Doxygen | 1.13.2, same WinLibs bin | Future source-documentation checks |
| VS Code-managed PlatformIO | 6.1.19, C:/Users/Honza/.platformio/penv/Scripts/pio.exe | Use scripts/pio.cmd once introduced; do not install another Core |
| PATH pio | Resolves to Python312/Scripts/pio.exe | Different executable; do not use it for the managed-PlatformIO requirement |
| Native IDF | idf.py absent from PATH; no IDF environment variables; C:/Espressif, ~/.espressif and ~/esp absent | Not located by this audit; later native-IDF builds require an actual configured SDK |
| clang++ / MSVC cl | Not on this shell's PATH | No claim they are absent from the entire machine |

The PlatformIO package directory contains Arduino-ESP32 and toolchains but no
framework-espidf directory. Package presence is not a native-IDF build result.
No new tools were installed, and no SDK version was selected as a support promise.

## Prompt 1 evidence limits (historical)

Local links, inventory paths/exclusions, source claims and baseline consistency
are this prompt's acceptance checks; see [validation](validation.md). There is no
source Doxygen to add and no driver suite to run in this research step. Prompt 2
owns the next family-capability verification. It has not been executed here.
