# Contributing

Read [AGENTS.md](AGENTS.md), the [prompt index](docs/prompts/README.md) and
[COMMON](docs/prompts/COMMON.md) before implementation. Keep the existing
execution branch for this promptset. Sibling repositories are read-only evidence.
Use the [engineering baseline](docs/prompts/ENGINEERING_BASELINE.md) for public
names and [maintainability rules](docs/prompts/MAINTAINABILITY.md) for code/docs.

The implemented library contains portable values, Status helpers and pure
validators and public pure register decoding, plus private transport/identity,
field encoders, settings comparison and qualified write composition. The owner
facade implements lifecycle, token/result ownership, read-only initialization,
profile observation, explicit apply, desired-profile verification, read-only
reconciliation, explicit reset, original TUSB320/I dead-battery recovery and
complete read-only port refresh and explicit IRQ acknowledgement/bounded service.
Later operation families remain planned.
Add public callable
declarations with real definitions and link checks;
keep future facade signatures in documentation/compile-only fixtures. Do not add
hardware stubs or empty future source files. Document behavior and errors with
the change, update Unreleased and distinguish software checks from hardware proof.

Use CMake 3.16 or newer and a C++17 compiler:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Single-configuration generators also need `-DCMAKE_BUILD_TYPE=Debug` at configure
time. Repeat in a separate Release directory for relevant changes. Supported GNU,
Clang and MSVC builds use strict private warnings; runtime suite checks remain
active in Release. Six runtime CTest suites are registered on all hosts, plus
the Windows launcher suite on Windows; compile-only signature/header fixtures
are build checks. See [test/README.md](test/README.md).
The protocol suite includes independent field tables, reserved-code validity,
actual-versus-staged controls and procedure-proof rejections. Keep expected
bytes independent of production codecs. Pure composition proves no timed job.
The operations suite tests initialization, profile observation, explicit apply,
desired verification, read-only reconciliation, reset and base recovery through
the real engine.
Keep literal per-variant write traces, preserved control fields, no-op behavior,
fresh wait anchors, cancellation/deadline boundaries and applied-then-error
faults independent of production arithmetic and codecs. Retain result and shared
owner coverage. A complete mismatching image may prove actual KNOWN state;
reconciliation never replays desired settings.
Reset tests must assert the actual 08 command bit, one attempt, fresh 96-tick
wait and variant postconditions. Base recovery tests use literal read/04/wait/00/read
traces, a fresh 31-tick wait and retained interrupted history. Clear controls
alone do not establish a completed recovery sequence.
The port suite tests public refresh/cached access with independent status
expectations. Preserve exact 09/08/09 traces, complete-only publication, original
acquisition time, saturating publication sequence and captured configuration
provenance. Exercise failures/cancellation/deadlines at each read and changed
bookends without retries. Configuration verification cannot freshen an old port
snapshot; raw completeness does not establish field applicability or atomicity.
IRQ tests keep independent literal actual-control/W1C expectations and bounded
R09/W09 or R09/R08/R09/W09/R09 traces. Check reassertion/coalescing, failed ACK
before and after simulated clearing, stuck IRQ and polling without a pin.
Preserve complete pre-ACK publication after later failure/cancellation/deadline,
and keep sampled-before, attempted/accepted write and sampled-after facts separate.
TUSB322I ACK/service must reject before traffic; raw bit3 is not an IRQ-ACK bit.
Also configure, build and run the [ordinary consumer](examples/host/basic/README.md)
with tests OFF after public/build changes. Core consumers need no Python, Git,
Doxygen, PlatformIO, framework headers or sibling paths. Use available host
sanitizers and record unavailable checks accurately in [validation](docs/validation.md).

Tests ON additionally require Python 3.10+. Maintain version truth with:

```text
python scripts/generate_version.py sync
python scripts/generate_version.py check
python tools/test_version_sync.py -v
doxygen Doxyfile
```

Use generator bump/set only for intentional version changes; never hand-edit
Version.h. No generator or Git discovery runs during consumer builds. On Windows
use `scripts/pio.cmd` for the existing managed Core. The pinned native consumer
environment is currently usable; later firmware environments remain planned.
[Development metadata](docs/development.md) defines version exits, pins, package
allowlists, ownership and evidence limits. Extend the existing version fixture
suite as guards mature; do not add a competing version source or fixture suite.

Review the complete scoped diff, run the affected checks and update
[progress](docs/implementation-progress.md). The lead stages owned changes,
reviews the staged diff, commits and synchronizes the established upstream without
rewriting history. Remote CI belongs only to prompt 34 in this initial execution;
preserve required hooks and merge/release protections. Firmware links, package
qualification and physical hardware checks remain separate evidence.
