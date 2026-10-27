# TUSB32x

TUSB32x is a framework-neutral C++17 library in development, starting with
TI TUSB320LAI. It currently provides passive public values, constexpr Status
helpers, pure binding/profile validators, public supplied-byte register decoding
and stable package version constants.
The library also contains [private transport/identity helpers](docs/transport.md)
and the non-copyable `TUSB32x::TUSB32x` owner facade. It provides bus-silent
binding/lifecycle, token-checked polling/cancellation/result delivery, cached
evidence, read-only initialization, profile observation, explicit profile apply,
desired-profile verification, read-only reconciliation, explicit reset and
original TUSB320/I dead-battery recovery, complete read-only port refresh and
explicit interrupt acknowledgement/bounded service, one-callback raw diagnostics
and pure passive health projection. Firmware adapters remain planned.
The development version is defined in library.json; it is not a release or
hardware-support claim.

The [protocol conformance matrix](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md)
maps TI/software requirements to named host tests, including both legal addresses,
ambiguous writes, interrupted recovery and retained observations. These tests
verify software contracts; physical adapters and hardware remain unqualified.

`validateDriverConfig` checks callbacks, the explicit variant/address pair and
positive timeout. `validateProfile` checks value/variant semantics and distinguishes
invalid inputs from unsupported settings. Both are bounded, allocation-free and
perform zero callbacks or device mutation. TUSB322I write gates remain separate
from profile validity. See the [API guide](docs/api-contracts.md) and
[variant boundary](docs/variant-matrix.md).

`decodeRegister` in `<TUSB32x/CommandTable.h>` preserves raw bytes and reports
variant-specific field validity without a binding or device access. Reserved
codes remain raw evidence. Single-byte direction/current codes need complete
port context before use; CC indications are not measured current or permission
to draw power. The private codecs also compare settings and compose qualified
write bytes. The owner engine executes supported profile changes, reset and
base-part recovery and explicit IRQ operations.

## Consume the library

Use CMake 3.16+ and a C++17 compiler:

```cmake
add_subdirectory(path/to/TUSB32x)
target_link_libraries(app PRIVATE TUSB32x)
```

Tests default OFF. The target supplies its public include directory and C++17
requirement; ordinary consumers need no Git, Python, Doxygen, PlatformIO,
framework headers or sibling checkout. For example:

```cpp
#include <TUSB32x/Config.h>
#include <TUSB32x/Version.h>

const auto status = TUSB32x::validateProfile(
    TUSB32x::Variant::TUSB320LAI, TUSB32x::DeviceProfile{});
```

The [real host consumer](examples/host/basic/README.md) builds and runs multiple
translation units with CMake or pinned native PlatformIO. Its callbacks are
explicitly fake and never imply a physical device. If native IDF's
`idf_component_register` exists, the root CMake file registers the same real
sources as a component. Registration and dependency admission are not linked
firmware evidence.

## Develop from a source checkout

```text
python scripts/generate_version.py sync
python scripts/generate_version.py check
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Initialization reads identity and persistent settings without writing defaults,
resetting, acknowledging IRQ or replaying a desired profile. The application
serializes calls, supplies a uint64 millisecond clock and keeps each accepted
token with its driver instance. `poll` performs at most one terminal callback;
the next poll supplies fresh time before accepting successful completion.
`startReadProfile` reads persistent controls without adopting desired settings
or promoting configuration trust. `startReconcile` reads identity and all
applicable controls once and can establish initialized, KNOWN actual settings
after external invalidation. It performs no repair or replay writes; unresolved
reset or sequence evidence keeps trust UNKNOWN. Readable TUSB322I controls do
not grant permission to write gated fields.

`startApplyProfile` validates a complete candidate against proved actual controls,
retains desired input at admission and writes only changed fields. A no-op still
runs complete identity/control verification and has no write effect. Base mode
changes require an unattached precheck; current-only changes do not inherit that
restriction. Extended 320 mode changes use separate disable/mode/enable writes
and the TI 5 ms wait plus a separate one-tick owner-clock margin. TUSB322I uses
its own DFP/UFP sequence; advertisement, all 09 writes and direct return to DRP
remain gated.

`startVerifyProfile` reads the full proof and compares retained desired settings.
A complete differing actual image can be KNOWN while the result reports
CONFIGURATION_MISMATCH. Cancellation or failure never rolls back hardware:
desired input, historical values and unresolved effects survive. Reconcile can
observe an interrupted apply, after its required wait, so a later explicit apply
can proceed from proved actual settings. It never replays the old request.

`startReset` reads actual 0A controls, sends the reset command once, waits
96 elapsed owner ticks and performs one identity/control verification. The
95 ms TI reset maximum and separate one-tick clock margin do not prove attachment
readiness or a power-on reset. Reset can resample PORT on the six 320 parts and
returns TUSB322I to DRP. Desired settings remain retained for explicit owner policy.

`startRecoverDeadBattery` is only for original TUSB320/TUSB320I with validated
identity and application-established stable VDD/readiness. It reads 45, requires
internal bits 1:0 clear, writes literal 04, waits 31 elapsed owner ticks, writes
literal 00 and verifies bits 2:0 clear. This is at most four callbacks; the
30 ms TI minimum has a separate one-tick margin. Interrupted sequence evidence
survives result take, end/rebind and read-only reconciliation. Clear controls
alone cannot prove the required sequence ran. Other variants reject recovery
without traffic. Neither operation retries, replays desired settings or publishes
transient recovery port observations. See the [procedure guide](docs/device-behavior.md).

`startRefreshPort` needs a binding and reads 09/08/09 once, within three callbacks,
three requested TX bytes and three RX bytes. `portSnapshot` returns a cached copy
without traffic. Only complete observations publish; failed or cancelled reads
retain the previous snapshot. Matching attachment/direction bookends provide
`BOOKENDS_MATCH`, not atomic sampling; changed bookends keep raw data and invalidate
fields that depend on both registers. Check per-field validity, invalidation,
coherence and age before using a field. Time identifies the owner boundary before
the first read; sequence counts publications and saturates.

Source advertisement and local-sink detected current are separate categories;
neither measures load or establishes USB PD or board power permission. A fresh
register read cannot prove physical current tracking after attachment. Connector
orientation mapping and electrical behavior need separate exact-board evidence.
Refresh observes the IRQ latch without acknowledging it; TUSB322I fault bit 3
remains raw evidence with unresolved access/clear semantics.

`startAcknowledgeInterrupt` reads fresh 09 controls and writes the explicit W1C
command once: at most two callbacks, three requested TX bytes and one RX byte.
`startServiceInterrupt` publishes the complete 09/08/09 port observation, ACKs
once, then reads 09 once: at most five callbacks, six TX bytes and four RX bytes.
Both require current validated identity; UNKNOWN configuration is allowed when
fresh controls establish a safe write image. TUSB322I rejects both before traffic
while its fault/09-write gate remains unresolved. Actual controls are preserved;
staged desired settings never enter the ACK byte.

Service retains the complete pre-ACK snapshot if ACK or recheck later fails.
Sampled pending-before, ACK attempt, callback acceptance and pending-after are
independent facts; callback acceptance does not prove a cleared latch. A pending
recheck is a valid successful bounded service result. The owner decides whether
to request more work. Coalescing and races prevent lossless edge counting. ACK
preserves the historical port snapshot, whose sampled IRQ field can be old.
Polling needs no IRQ pin; GPIO/ISR handling and policy remain application-owned.

`readRegisterSync` and `writeRegisterSync` each perform at most one callback.
Raw writes require proved actual controls and cannot shortcut role, reset,
termination or recovery procedures. `transportStats` counts every callback;
`diagnostics().managedTransport` excludes raw Sync. `projectHealth` displays
UNINIT/READY/DEGRADED/OFFLINE from copied managed evidence without controlling
access, recovery or configuration trust. See [diagnostics](docs/diagnostics.md).

One terminal result blocks replacement until `takeResult` delivers it. Result
delivery returns OK even when the copied operation failed. See the
[implemented owner API](docs/api-contracts.md#implemented-owner-api) for timing,
output preservation and evidence lifetime.

Tests ON require Python 3.10+. For single-config generators also select
`-DCMAKE_BUILD_TYPE=Debug` at configure time. The registered checks cover C++
contracts, validators, transport/codecs, owner operations, port publication,
raw safety/accounting/passive health and isolated generator fixtures. Run `doxygen Doxyfile`
for strict current API documentation. On Windows use the existing VS Code Core
through `scripts/pio.cmd`; do not install another Core.

[Development metadata](docs/development.md) documents generator sync/check/bump/set,
exact exit codes, tool pins, package contents and evidence limits.
[Documentation](docs/README.md) and [examples](examples/README.md) index the
maintained guides. Repository implementation proceeds through the
[38 prompts](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/prompts)
on one execution branch, with source/tests/docs, commits and verified sync for
each completed unit. Remote CI is confined to prompt 34 in this first execution.

Project code is MIT, owned by Thymos Solution s.r.o with Jan Havelka as maintainer.
Archived vendor references retain their original terms and are excluded from
normal library packages. Native tests prove software values and linkage;
Arduino/native-IDF firmware, full distribution qualification and physical
hardware evidence remain separate later work.
