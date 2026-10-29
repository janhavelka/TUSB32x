# TUSB32x

A framework-neutral C++17 driver for TI Type-C configuration-channel controllers.
The application owns transport, time, scheduling and USB power policy. The library
provides cooperative operations, cached observations, explicit profile changes,
reset/recovery, interrupt service, bounded raw diagnostics and passive health.
It neither measures load current nor negotiates USB Power Delivery.

## Supported devices

| Configured part | Seven-bit addresses | Implemented scope |
| --- | --- | --- |
| TUSB320LAI / TUSB320HAI | `0x47 / 0x67` | Primary extended family; documented profile, port, reset and IRQ operations |
| TUSB320LI / TUSB320HI | `0x47 / 0x67` | Extended family with its own debounce map and silicon recommendations |
| TUSB320 / TUSB320I | `0x60 / 0x61` | Original family, restricted profile controls and explicit dead-battery recovery |
| TUSB322I | `0x47 / 0x67` | Partial support: separate identity and port/profile decoding; qualified DFP/UFP changes and reset. Advertisement writes, all `0x09` writes/ACK/service and direct apply back to DRP remain gated |
| TUSB321 family and unlisted devices | — | Outside this I2C API; no scan, auto-detection or compatibility claim |

Select the exact variant and address explicitly. Identity/revision checks do not
prove enable polarity, board wiring or electrical qualification. TUSB322I has no
`0x45`, and its reset-to-DRP behavior does not prove every register is at its
power-on value. See the [variant matrix](docs/variant-matrix.md) and
[hardware prerequisites](docs/hardware-validation.md).

Original TUSB320/I need an explicit reset to refresh attached-current tracking.
A register read alone cannot establish that refresh. See the guides for exact-part
supply, VBUS_DET and VCONN limits.

## Build and use

Use CMake 3.16+ and a C++17 compiler. Tests default OFF:

```cmake
add_subdirectory(path/to/TUSB32x)
target_link_libraries(app PRIVATE TUSB32x)
```

Target `TUSB32x` supplies the public include directory and C++17 requirement.
Building the core needs no Python, Git, Doxygen, PlatformIO or framework headers.
The [package guide](docs/development.md#archive-and-clean-consumers) covers archive
consumption. There is no install/export/`find_package()` interface.

This complete function uses application callbacks and a monotonic millisecond
clock. `binding` must select the exact target and provide terminal callbacks whose
aggregate bound includes bus locking, all phases and SDK cleanup. The callbacks
and their borrowed context stay alive through `end()`. `serviceOthers` performs
bounded unrelated owner work without re-entering this driver. The local 128-turn
limit is example policy; a real owner normally returns to its event loop each turn.

```cpp
#include <TUSB32x/TUSB32x.h>
#include <cstdint>

bool observeOnce(const TUSB32x::DriverConfig& binding,
                 std::uint64_t (*nowMs)(), void (*serviceOthers)()) {
  if (nowMs == nullptr || serviceOthers == nullptr) return false;
  namespace t = TUSB32x;
  t::TUSB32x driver;
  bool passed = false;
  if (driver.bind(binding).ok()) {
    t::OperationToken token{};
    const auto start = nowMs();
    auto status = driver.startInitialize({start, start + 1000}, token);
    if (status.inProgress()) {  // IN_PROGRESS means accepted work.
      for (unsigned turn = 0; turn < 128 && status.inProgress(); ++turn) {
        serviceOthers();
        status = driver.poll(token, nowMs(), 1);  // At most one callback.
      }
      if (driver.progress().state == t::OperationState::ACTIVE)
        status = driver.cancel(token);
      t::OperationResult result{};
      const auto delivery = driver.takeResult(token, result);
      const auto observed = driver.configurationSnapshot();  // No I2C.
      passed = status.ok() && delivery.ok() && result.status.ok() &&
               result.hardwareEffect == t::HardwareEffect::NONE &&
               observed.hasObservedProfile;
    }
  }
  const auto ended = driver.end();  // Local unbind; no hardware reset.
  return passed && ended.ok();
}
```

Initialization identifies and observes only. It does not apply defaults, ACK,
reset or establish base-part recovery history. Result delivery may return OK
while `result.status` reports failure; inspect outcome and effect independently.
The README function is compiled and executed from this exact code block by the
registered contract suite, using explicitly simulated transport and time.

The runnable [plain C++ example](examples/host/basic/README.md) uses simulated
transport and shows result handling. Read the [owner guide](docs/owner-operations.md)
for timing, cancellation and shutdown, and the [integration guide](docs/integration.md)
for shared-bus ownership. Call all APIs from one serialized owner; an ISR only
signals that owner.

## Examples, console and runners

- [Arduino](docs/arduino-examples.md): shared CLI and two-device owner applications
  for the pinned S2/S3 configuration, with explicit board/timing readiness.
- [Native ESP-IDF](examples/esp_idf/basic/README.md): S2/S3 application,
  audited SDK 5.5.5/6.1.0, bounded UART console and application-owned resources.
- [CLI reference](docs/CLI.md): the authoritative 55-row grammar, aliases, staging,
  operation/result ownership, diagnostics and machine records.
- [Python runner](docs/hil-runner.md): finite readonly/soak and seven explicitly
  selected fixture suites, strict identities/deadlines and streamed artifacts.
  Dry-run performs no hardware I/O and exits 2 with hardware NOT RUN.
- [Hardware validation](docs/hardware-validation.md): exact wiring, fixture,
  supply/attachment prerequisites, conditional cleanup and qualification limits.

CLI staging, desired settings, observed registers and verified settings are
separate. Raw access cannot bypass required device procedures. Host tests and
firmware builds do not qualify physical serial, I2C, CC, INT or supply timing.

## Development and recorded checks

```text
python scripts/generate_version.py sync
python scripts/generate_version.py check
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Tests ON require Python 3.10+. Runtime assertions remain active in Release.
Registered [fuzz tests with fault injection](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/fuzz-testing.md)
exercise CLI input, uncertain transfers, output stalls, serial records and archive
rejection with repeatable seeds. They require no hardware or extra local tools.
For prerequisites and a complete software-only check, use the source-checkout
[validation setup](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/software-validation-setup.md).
`python tools/validate.py --checks all` runs the declared local checks without
hardware I/O or flashing. Missing required tools produce INCOMPLETE and a nonzero
exit. On Windows, use the installed managed Core through `scripts/pio.cmd`.
The [development guide](docs/development.md) owns tool versions and package rules.

The recorded [CI checkpoint](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910)
passed 14 jobs at `c50f8ddda8809425ede2161586466c95fa650ae6`: five strict
Linux host configurations, Python/docs/package checks, four extracted Arduino
images and four native-IDF images. This result applies only to that commit.
No physical board is qualified by that run.

The [final review](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/final-audit.md)
and [validation record](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md)
record completed software reviews and checks. A newer
[CI run](https://github.com/janhavelka/TUSB32x/actions/runs/40864851161) passed all
14 jobs at `ff26de9592d9cfbd5565f62e25e126cf44d06d90`, including Linux sanitizers
and IDF 6.1 for S2/S3. These checks needed no changes to the local Windows tools.
Physical board and timing qualification remain NOT RUN. Later reporting edits
do not change the tested commit recorded with either run.

Start with the [documentation index](docs/README.md) and
[examples index](examples/README.md). The repository retains the
[completed implementation plan](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/archive/prompts)
and [dated 2026-10-23 audit](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/library-comparison.md)
as source-only history. Current work follows
[contribution guidance](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/CONTRIBUTING.md).

Development version `0.1.0-dev.1` is not a release claim. Project code is MIT,
owned by Thymos Solution s.r.o with Jan Havelka as maintainer. Archived vendor
references retain their own terms and are excluded from normal packages.
