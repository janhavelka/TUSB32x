# Library architecture

Prompts 5-8 provide portable values, constexpr Status/token helpers, pure validators,
a C++17 CMake library target and deterministic package metadata/version tooling.
Prompt 9 adds [private register transport and identity checks](transport.md).
There is no device facade or operation engine yet. The
[API inventory](api-contracts.md) labels all future calls explicitly; the
[engineering baseline](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/prompts/ENGINEERING_BASELINE.md) fixes their names.

## Current files

| File | Responsibility |
| --- | --- |
| include/TUSB32x/Status.h | Exact errors, static-message Status and one constexpr errorName |
| include/TUSB32x/Config.h | Terminal callback types, borrowed binding description, variant/settings, semantic enums and callable pure validators |
| include/TUSB32x/TUSB32x.h | Fixed operation, snapshot, result, diagnostic and health values; no callable facade yet |
| include/TUSB32x/CommandTable.h | Passive RegisterDecode value; decoder and register tables arrive with implementation in prompt 10 |
| include/TUSB32x/Version.h | Generated stable constexpr version values; no Git, timestamp or firmware identity |
| src/TUSB32x.cpp | Bounded pure binding/profile validation; no driver instance |
| src/TUSB32xProtocol.h/.cpp | Private mapped terminal callbacks, single accounting boundary, possible-write evidence and pure exact identity/revision comparisons |
| CMakeLists.txt | Explicit source list, PUBLIC C++17/include directory, tests OFF by default; no external maintainer tools for core consumption |
| test/CMakeLists.txt, test/test_validators.cpp and test/test_protocol.cpp | Registered active C++ suites, exhaustive validator/protocol checks and compile-only standalone-header/signature fixtures |
| examples/host/basic/ | Standalone multi-file consumer of the real values and validators; explicitly failing fake callbacks |
| test/test_compile_contracts.cpp and test/consumer/status_other_tu.cpp | Real-header default/type/constexpr and multiple-translation-unit link checks |
| test/contract/PlannedApi.h and planned_api.cpp | Explicitly compile-only future signatures, never linked or delivered |
| test/contract/owner_examples.cpp | Compile-only passive observation, explicit apply/cancel and two devices sharing an external fake owner; no driver definitions |
| test/contract/time_arithmetic.cpp | Independent compile-time integer time vectors; not engine behavior or hardware timing proof |
| docs/reference/ | Research sources and provenance, excluded from production compilation and future ordinary packages |
| .gitignore | Excludes local build/documentation output under build/ from commits |
| library.json, idf_component.yml and Doxyfile | Version truth and synchronized component/documentation metadata, with explicit normal-package boundaries |
| scripts/generate_version.py and tools/test_version_sync.py | Explicit maintainer sync/check/bump/set and isolated registered version fixtures |
| scripts/pio.cmd, tools/test_pio_launcher.py and examples/host/basic/platformio.ini | Existing managed Windows Core launcher, isolated Windows exit/error fixtures and pinned real native consumer |

Status depends only on standard integer types. Config includes Status and
standard size/integer types. TUSB32x includes Config for copied values.
CommandTable includes the public value inventory for the shared decoder result;
TUSB32x does not include CommandTable. No private source header, sibling include,
framework header, global bus or hidden service is required.

## External owner and later implementation

The application owns the physical bus, pins, lock, routing, monotonic clock,
scheduling, VBUS/power policy, IRQ handoff and recovery decisions. It supplies
terminal callbacks and a borrowed context; core values contain no bus handle,
GPIO number, RTOS task or USB stack. Ordinary C++, bare-metal, Arduino, native
IDF and FieldCore-like applications share this boundary. FieldCore is not a
dependency. Different physical targets may share one serialized application bus;
each target has one coordinated configuration owner.

One future non-copyable/non-movable facade contains fixed storage for binding,
one operation, one retained terminal result and cached evidence. It uses one
transport path, one deliberate register composer and one owner-driven engine.
There is no factory, generic scheduler, registry, inheritance hierarchy or Pimpl.
Small adjacent private responsibilities may share source units; files are added
when implemented, not as empty placeholders.

The future source split is binding/validation/cache (`TUSB32x.cpp`), transport
and composition (`TUSB32xProtocol.cpp`), operations (`TUSB32xOperations.cpp`),
port observation (`TUSB32xPort.cpp`) and passive diagnostics
(`TUSB32xDiagnostics.cpp`). Public declarations arrive with definitions, tests
and Doxygen. The current units implement the validators and private transport/
identity layer; the safe composer and owner engine remain later work.
Prompt 8 implements package/version metadata and checks a real PlatformIO archive;
full archive/firmware consumption qualification remains prompt 32. CMake install
exports remain outside the current implementation.

When idf_component_register exists, the root build registers the same explicit
sources as an IDF component; otherwise it creates the ordinary host target. The
component branch calls no Python/Git and owns no bus. Its registration fixture
is a host check, not linked IDF firmware. Version metadata is checked in and
updated only by explicit maintainer commands. Tests ON add Python fixtures;
tests-OFF consumers have no new tool dependency. See [development](development.md).

`TUSB32x` has no transitive link dependency. Core warnings are private and its
public include directory/C++17 feature propagate to consumers. Tests are optional;
their build also compiles each public header independently and the planned API
fixtures without registering those build-only objects as passing runtime tests.
`validateProfile` accepts readable TUSB322I settings independently of later
changed-field write gates; it cannot prove observed configuration or a legal
transition. The standalone consumer uses only actual callable API.

Managed I2C advances only in poll. Admission, cancellation, teardown and cached
copies remain bus-silent. Synchronous raw diagnostics have an explicit separate
one-callback bound. Buffers never survive callback return; no DMA, retries,
reentry or platform delays are hidden in core. The owner supplies a fresh time
after each Status-only callback. Silicon waits and uncertainty follow the
[device procedure guide](device-behavior.md). Prompt 6 completes the precise
[admission, state and lifetime tables](api-contracts.md#method-admission-and-output-preservation)
before the engine exists. Each poll uses a bounded direct pass and at most one
Status-only callback; zero budget still permits time-only transitions. Current
identity readiness is separate from configuration trust so explicit recovery
remains possible after rebind and read-only initialization.

Desired input, raw observations, verified actual settings, port interpretation,
IRQ evidence and operation outcome are separate values. Every incomplete or
historical observation carries validity/provenance. Independent interrupted
recovery, mode and reset facts survive result consumption. Optional health only
projects managed transport evidence; it neither repairs trust nor controls work.

Portable CLI code will consume public values and the single pure register
decoder. Framework transport/console adapters stay under examples. Host runners
own repeated fixture sequences rather than adding another core scheduler. No
framework adapter, CLI, runner or hardware test is implemented in this stage.
