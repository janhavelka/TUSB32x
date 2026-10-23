# Library architecture

Prompt 5 provides portable values and implemented constexpr Status/token helpers.
There is no device facade or operation engine yet. The
[API inventory](api-contracts.md) labels all future calls explicitly; the
[engineering baseline](prompts/ENGINEERING_BASELINE.md) fixes their names.

## Current files

| File | Responsibility |
| --- | --- |
| include/TUSB32x/Status.h | Exact errors, static-message Status and one constexpr errorName |
| include/TUSB32x/Config.h | Terminal callback types, borrowed binding description, variant/settings and semantic enums |
| include/TUSB32x/TUSB32x.h | Fixed operation, snapshot, result, diagnostic and health values; no callable facade yet |
| include/TUSB32x/CommandTable.h | Passive RegisterDecode value; decoder and register tables arrive with implementation in prompt 10 |
| test/test_compile_contracts.cpp and test/consumer/status_other_tu.cpp | Real-header default/type/constexpr and multiple-translation-unit link checks |
| test/contract/PlannedApi.h and planned_api.cpp | Explicitly compile-only future signatures, never linked or delivered |
| docs/reference/ | Research sources and provenance, excluded from production compilation and future ordinary packages |
| .gitignore | Excludes local build/documentation output under build/ from commits |

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
and Doxygen. Build metadata/validators belong to prompt 7; package/version
metadata belongs to prompt 8. This stage does not claim a consumable built package.

Managed I2C advances only in poll. Admission, cancellation, teardown and cached
copies remain bus-silent. Synchronous raw diagnostics have an explicit separate
one-callback bound. Buffers never survive callback return; no DMA, retries,
reentry or platform delays are hidden in core. The owner supplies a fresh time
after each Status-only callback. Silicon waits and uncertainty follow the
[device procedure guide](device-behavior.md); prompt 6 completes precise state
and lifetime tables before the engine exists.

Desired input, raw observations, verified actual settings, port interpretation,
IRQ evidence and operation outcome are separate values. Every incomplete or
historical observation carries validity/provenance. Independent interrupted
recovery, mode and reset facts survive result consumption. Optional health only
projects managed transport evidence; it neither repairs trust nor controls work.

Portable CLI code will consume public values and the single pure register
decoder. Framework transport/console adapters stay under examples. Host runners
own repeated fixture sequences rather than adding another core scheduler. No
framework adapter, CLI, runner or hardware test is implemented in this stage.
