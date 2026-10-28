# Library architecture

Prompts 5-8 provide portable values, constexpr Status/token helpers, pure validators,
a C++17 CMake library target and deterministic package metadata/version tooling.
Prompt 9 adds [private register transport and identity checks](transport.md).
Prompt 10 adds public pure register decoding and private field encoding,
semantic comparison and qualified byte composition.
Prompt 11 adds the non-copyable owner facade, bus-silent lifecycle, one active
operation/retained result and read-only initialization. Prompt 12 adds profile
observation and read-only reconciliation through that engine. Prompt 13 adds
explicit differences-only apply and desired-profile verification using the same
proof and write composer. Prompt 14 adds explicit reset and original TUSB320/I
dead-battery recovery without a second engine or scheduler. Prompt 15 adds fixed
read-only port refresh and cached publication through that same engine. Prompt 16
adds explicit IRQ acknowledgement and bounded observe/ACK/recheck service.
Prompt 17 adds safe one-callback raw access and pure passive health. The
[API inventory](api-contracts.md) describes delivered methods and their contracts; the
[engineering baseline](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/prompts/ENGINEERING_BASELINE.md) fixes their names.

## Current files

| File | Responsibility |
| --- | --- |
| include/TUSB32x/Status.h | Exact errors, static-message Status and one constexpr errorName |
| include/TUSB32x/Config.h | Terminal callback types, borrowed binding description, variant/settings, semantic enums and callable pure validators |
| include/TUSB32x/TUSB32x.h | Fixed operation/snapshot/result values and non-copyable owner facade; lifecycle, initialize, profile read/apply/verify/reconcile, reset/base recovery, port refresh, IRQ acknowledgement/service, raw Sync, pure health, token/result and cached methods are callable |
| include/TUSB32x/CommandTable.h | RegisterDecode value and callable pure decodeRegister; raw evidence and independent field validity |
| include/TUSB32x/Version.h | Generated stable constexpr version values; no Git, timestamp or firmware identity |
| src/TUSB32x.cpp | Bounded pure binding/profile validation |
| src/TUSB32xOperations.cpp | Atomic binding/lifecycle/evidence management and one cooperative initialization/profile/apply/verify/reconcile/reset/recovery/port/IRQ engine with private phases, immutable deadlines, tokens and retained results |
| src/TUSB32xDiagnostics.cpp | Safe one-callback raw facade using existing composition/transport, and pure health validation/projection |
| src/TUSB32xOwnerTime.h | Private unsigned token/owner-time arithmetic and saturating publication sequence/generations |
| src/TUSB32xProtocol.h/.cpp | Private mapped terminal callbacks, single accounting boundary, possible-write evidence and pure exact identity/revision comparisons |
| src/TUSB32xCodecs.h/.cpp | Public pure decoder, private complete-port interpretation, typed encoder/comparison and one actual-state-qualified write composer; no transport calls |
| CMakeLists.txt | Explicit source list, PUBLIC C++17/include directory, tests OFF by default; no external maintainer tools for core consumption |
| test/CMakeLists.txt, test/test_validators.cpp, test/test_protocol.cpp, test/test_codecs.cpp, test/test_operations.cpp, test/test_protocol_conformance.cpp, test/test_profile_observation.cpp, test/test_profile_application.cpp, test/test_reset_recovery.cpp, test/test_port.cpp, test/test_interrupt.cpp and test/test_diagnostics.cpp | Registered validator/protocol/codec/owner/port/IRQ/diagnostics suites, plus compile-only standalone-header/signature fixtures |
| test/test_owner_scheduling.cpp | Fixed external two-target scheduler, bounded queues/results and independent late-callback timing; no core scheduler |
| test/consumer/portable/, test/consumer/run_portable.py | Isolated linked public consumer with allocation/failure sentinels; Python only coordinates maintainer CTest |
| examples/host/basic/ | Standalone multi-file consumer of real values, validators, decoder and pure health; explicitly failing fake callbacks |
| test/test_compile_contracts.cpp and test/consumer/status_other_tu.cpp | Real-header default/type/constexpr and multiple-translation-unit link checks |
| test/contract/planned_api.cpp | Compile-only delivered API signature inventory; the former future-declaration sketch is no longer needed |
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

## External owner and operation engine

The application owns the physical bus, pins, lock, routing, monotonic clock,
scheduling, VBUS/power policy, IRQ handoff and recovery decisions. It supplies
terminal callbacks and a borrowed context; core values contain no bus handle,
GPIO number, RTOS task or USB stack. Ordinary C++, bare-metal, Arduino, native
IDF and FieldCore-like applications share this boundary. FieldCore is not a
dependency. Different physical targets may share one serialized application bus;
each target has one coordinated configuration owner.

One non-copyable/non-movable facade contains fixed storage for binding,
one operation, one retained terminal result and cached evidence. It uses one
transport path, one deliberate register composer and one owner-driven engine.
There is no factory, generic scheduler, registry, inheritance hierarchy or Pimpl.
Small adjacent private responsibilities may share source units; files are added
when implemented, not as empty placeholders.

The current split keeps pure codecs/composition in `TUSB32xCodecs.cpp` and the
terminal callback/accounting boundary in `TUSB32xProtocol.cpp`, sharing one
closed variant-capability table. Validators remain in `TUSB32x.cpp`;
binding/cache and the single owner engine share `TUSB32xOperations.cpp`.
Port publication also stays adjacent to that engine; no separate port scheduler
is needed. `TUSB32xDiagnostics.cpp` supplies the raw facade and pure health
projection, reusing existing shared helpers. Public declarations arrive with definitions, tests
and Doxygen. The current units implement validators, private transport/identity
and pure codecs/composition, plus lifecycle/token/result handling and read-only
initialization, profile observation, explicit apply, desired-profile verification
and read-only reconciliation, explicit reset, base dead-battery recovery and
complete read-only port refresh and explicit IRQ acknowledgement/service.
Raw diagnostics and passive health are implemented; framework adapters and CLI
remain later work.
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
their build also compiles each public header independently and the API signature
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
implemented by prompt 11 for its delivered subset. Each poll uses a bounded
direct pass and at most one Status-only callback; zero budget still permits
time-only transitions. Current
identity readiness is separate from configuration trust so explicit recovery
remains possible after rebind and read-only initialization.

An initialization reads the exact identity/revision and applicable persistent
controls once. It never writes, adopts a desired profile or establishes that an
interrupted silicon procedure completed. A successful callback yields to the
owner before the next callback or time-sensitive completion. Failure retains
its normalized transport status/detail immediately; fresh late success becomes
owner TIMEOUT. A frozen terminal result survives end/invalidation and repeated
snapshot delivery until exact take. Different-target binding cannot discard
unresolved effects; explicit forgetting requires unbound, inactive, result-free
state. The target association includes both callbacks even with null context.

Profile reads use the fixed P plan, stage complete controls and publish raw and
typed validity separately. They cannot initialize the device or promote trust;
a complete contradiction invalidates current verified trust. Reconciliation uses
the same F identity/control proof as initialization, qualifies retained effects
and can restore initialized, KNOWN actual configuration without writes. A pending
reset first receives its conservative 96-tick wait, then one F pass. A reset first
discovered during that pass remains pending for a later explicit reconciliation.
These configuration-observation and proof paths never replace desired settings
or clear port invalidation. Only a complete fresh port publication clears it.

Apply requires current identity and KNOWN actual settings before admission. It
validates the entire candidate and variant gates before retaining desired input
and incrementing desired generation. Admission and pre-write cancellation retain
proved actual evidence. Before the first possible mutation, the engine invalidates
configuration trust and port usability, marks possible effects and invokes the
terminal transport using the already composed actual-control image. A failed callback may follow an
applied write; no cleanup, rollback or retry obscures the original failure.

The fixed write plan changes differing fields only. Base mode changes have a
read-only unattached precheck before any write. Extended 320 mode changes use
separate termination-disable, mode and enable steps, with six elapsed owner ticks
after the fresh mode-write completion boundary. TUSB322I uses its own DFP/UFP
steps without an inherited wait; its unresolved advertisement/09/direct-DRP gates
remain local rejections. Requested final disable controls are preserved.

Apply then runs full F verification; verify runs F without writes and compares
retained desired. A complete differing actual image can become KNOWN with a
CONFIGURATION_MISMATCH result. Reconcile can establish the actual image after
an interrupted apply and any required wait without requiring desired equality.
These paths share proof/publication helpers and do not introduce another engine.

Reset reads safe actual 0A controls, composes command bit 08 once and yields for
fresh owner time before its 96-tick wait. It then runs the same one-pass F proof;
no reset retry, desired replay or whole-register default assumption is added.
The six 320 parts can resample PORT; TUSB322I must prove semantic DRP afterward.
Reset completion, command-acceptance uncertainty and older interrupted recovery
remain independent facts.

Base recovery requires current identity and owner-qualified stable VDD. It reads
45, qualifies internal bits 03 clear, writes literal 04, waits 31 elapsed owner
ticks from fresh completion time, writes literal 00 and verifies once. Its four
callbacks do not publish transient port observations. An interruption retains
the sequence obligation beyond result take/rebind; clear controls and read-only
reconciliation cannot certify the historical wait. A new explicit request can
attempt recovery again. Neither operation adds a power monitor or board policy.

Port refresh stages 09/08/09 in private candidate storage and publishes final09
only after all three reads and the fresh completion/deadline check. It has no
identity/configuration promotion, reset, ACK or retry. It captures owner time,
verified generation and configuration state before its first read. Bookends are
compared under E0; matching is evidence of matching bookends, while changed values
invalidate cross-register fields without discarding raw data. Both remain
sequential hardware observations. Failure preserves the old complete snapshot;
publication increments its saturating sequence and clears invalidated.

IRQ acknowledgement reads fresh actual09 and uses the existing safe composer to
write W1C bit `0x10` while preserving only applicable RW controls. It takes at most
two callbacks, three requested TX bytes and one RX byte. Full service shares
the fixed port observation, publishes it before ACK, then rechecks09 once: five
callbacks, six TX bytes and four RX bytes. Both need current identity but permit
UNKNOWN configuration; TUSB322I rejects before traffic under its unresolved
fault/09-write gate. Desired controls are never write inputs for ACK.

ACK effects do not invalidate the complete port observation or create a settings
proof obligation. Complete pre-ACK data survives a later ACK/recheck error.
Operation IRQ evidence records sampled-before, attempt, callback acceptance and
sampled-after independently; plain refresh updates only the sampled port field.
An OK write can coincide with a reassertion, and a failed write can already have
cleared the latch. One recheck is current sampled evidence, never lossless event
history. A pending recheck does not cause another operation. Polling requires no
IRQ pin; GPIO, ISR handoff and follow-up policy remain in the application.

End, external invalidation and possible port-affecting writes set invalidated
without rewriting the historical fields/time/provenance. Configuration readback
cannot clear it, and uninvalidated data can still be old. Getter/result delivery
is bus-silent; no new GPIO/ISR/event queue or policy layer is introduced. The
application checks field validity, coherence and age, and separately qualifies
connector mapping, attached-current tracking and electrical operation.

Desired input, raw observations, verified actual settings, port interpretation,
IRQ evidence and operation outcome are separate values. Every incomplete or
historical observation carries validity/provenance. Independent interrupted
recovery, mode and reset facts survive result consumption. Optional health only
projects managed transport evidence; it neither repairs trust nor controls work.

Portable CLI code will consume public values and the single pure register
decoder. Framework transport/console adapters stay under examples. Host runners
own repeated fixture sequences rather than adding another core scheduler. No
framework adapter, CLI, runner or hardware test is implemented in this stage.
