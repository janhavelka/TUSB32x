# Library architecture

TUSB32x is a framework-neutral C++17 library with fixed storage, borrowed terminal
transport callbacks and one cooperative operation engine. Ordinary C++, Arduino
and native ESP-IDF owners use the same public facade. FieldCore-style integration
is an application mapping; no sibling project is a dependency.

The [owner guide](owner-operations.md) explains the operating workflow. The
[API contract](api-contracts.md) owns admission, state, lifetime and time tables;
the [device procedure guide](device-behavior.md) owns register sequences and
variant-specific work bounds. [Validation](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md) separates host checks,
linked framework images and exact-board physical qualification.

## Current files

| Boundary | Responsibility |
| --- | --- |
| `include/TUSB32x/` | Public status, binding/profile values, non-copyable facade, snapshots/results, pure decoder and generated package version |
| `src/TUSB32x.cpp` | Pure binding/profile validation |
| `src/TUSB32xOperations.cpp` | Bus-silent lifecycle, one active operation, one retained result and the single initialize/profile/apply/verify/reconcile/reset/recovery/port/IRQ engine |
| `src/TUSB32xProtocol.h/.cpp` | One private callback/accounting path, mapped access and exact identity/revision checks |
| `src/TUSB32xCodecs.h/.cpp` | Pure register/port interpretation, explicit variant encodings and one qualified write composer |
| `src/TUSB32xDiagnostics.cpp` | One-callback raw facade and passive health projection using existing transport/composition |
| `examples/common/` | Portable CLI, output, bounded console and example owner glue |
| `examples/arduino/`, Arduino entry-point examples | Application-owned Wire adapter and complete CLI/two-device owner applications |
| `examples/esp_idf/` | Native handle-based adapter and complete UART0 CLI application |
| `examples/host/basic/` | Runnable ordinary C++ owner with explicitly simulated transport and time |
| `tools/tusb32x_*.py` | Host session/result ownership, bounded runner plans and streamed evidence artifacts |
| `test/` | Host behavior/consumer contracts and independent real-SDK link fixtures |
| `docs/reference/` | Vendor sources and provenance; excluded from production builds and normal packages |

Public headers include standard C++ headers and other public TUSB32x headers
only. No framework types, bus singleton, clock, task, logger, heap allocation or
private source header enters the public interface. The pure decoder in
`CommandTable.h` consumes supplied bytes; it does not require an instance or
perform I2C.

## External owner and operation engine

The application owns bus creation, pins, routing, locks, scheduling, monotonic
time, IRQ handoff, retry/recovery and power/data policy. One owner serializes
every instance call, including cached access; no method is ISR-safe. Different
targets may share a serialized bus, while each physical device has one
coordinated configuration owner. Callback context and buffers are borrowed under
the [transport contract](transport.md).

The non-copyable, non-movable facade contains a copied binding, fixed operation
scratch, one retained terminal result and cached evidence. Admission performs no
I2C. Only `poll` advances managed work, using at most one callback per turn even
with a larger budget. Explicit synchronous raw diagnostics have a separate
one-callback bound and obey the same operation/result exclusion. There is no
second scheduler or result store in the CLI.

Initialization and reconciliation observe without writes. Apply, reset, base
dead-battery recovery and IRQ acknowledgement require explicit requests. All
mutating paths reuse the actual-state-qualified write composer and record
possible effects before callback invocation. An error, cancellation or shutdown
does not undo those effects or start cleanup. The [owner guide](owner-operations.md#cancellation-invalidation-and-shutdown)
explains result consumption and subsequent application decisions.

Desired settings, observed bytes, verified actual settings, port interpretation,
IRQ evidence, operation outcome and passive health are independent values.
Snapshots are copied; a frozen result does not change when live evidence is
invalidated later. Complete port observations publish together in software, but
their hardware reads are sequential. Only a complete fresh port publication
clears port invalidation; a configuration proof or successful unrelated read
does not. See the [API evidence tables](api-contracts.md#independent-state-and-effect-tables)
and [diagnostics guide](diagnostics.md).

## Build and distribution boundary

The root CMake build registers the same core sources as a native IDF component
when `idf_component_register` is available; otherwise it creates the ordinary
`TUSB32x` target. The target exports its include directory and C++17 requirement
without a transitive framework or sibling dependency. Tests default OFF, so
ordinary consumers need only a C++17 toolchain and CMake.

Checked-in version metadata comes from `library.json`; a consumer build does not
run a generator, Git, Python or Doxygen. Tests ON add maintainer fixtures and
Python requirements. Compile-only header/signature checks are separate from
registered runtime suites. The explicit package contents and clean consumption
checks are documented in [development](development.md) and
[validation](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md). CMake install exports are outside this implementation.

## Arduino callback boundary

The [Wire adapter](arduino-transport.md) is example code with a controller context
that borrows `TwoWire`. The application supplies a gate shared by every bus user,
prewarmed handles and the audited SDK/configuration/timing prerequisites.
Controller retirement after a failed receive is distinct from core health,
configuration trust and retained results.

The [Arduino applications](arduino-examples.md) share application code and select
one setup/loop per image. The primary owner performs read-only startup and then
hands control to the shared CLI. The two-device example gives its second target
a separate fixed owner on the same bus context. Console output uses a bounded
mailbox and a USB worker that never calls the core or holds owner state.

## Native IDF callback boundary

The [native adapter](idf-transport.md) borrows application-created synchronous
bus/device handles. One `BusContext` carries the shared gate and controller
retirement evidence; each target `Context` retains its exact device association
and copied creation settings. SDK types stay in example code.

The complete [native CLI application](../examples/esp_idf/basic/README.md) owns
UART0, bus/device setup, read-only startup results and checked teardown. It uses
the same portable CLI and core without an Arduino facade. The independent
`test/idf_sdk` consumer links the adapter/core separately from the CLI application;
neither link result proves a board's physical timing or electrical behavior.

## Host tooling boundary

The [shared CLI](CLI.md) admits single core operations, polls once per turn and
retains only request correlation metadata until explicit result take. Host
Python runners own repeated observations, soak and declared fixture plans;
firmware does not gain another scheduler to execute them.

The [runner guide](hil-runner.md) defines the nine suite families, strict framing,
firmware identity checks, owned tokens/results, deadlines and bounded artifacts.
Physical plans require explicit setup and authorized hardware access. Ordinary
host tests use modeled hardware or the actual shared CLI/core fixture and do not
open a board.
