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
It also implements safe one-callback raw diagnostics and pure health projection.
The [shared example CLI](docs/CLI.md) implements strict parsing,
bounded output, independent staging and core-owned managed operation/result flow.
All contracted diagnostic routes are also implemented through public core APIs;
Arduino and native-IDF adapters and complete S2/S3 example applications are
implemented; physical board/timing qualification remains separate.
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
active in Release. Seventy-one runtime CTests are registered on all hosts, plus
the Windows launcher suite on Windows; compile-only signature/header fixtures
are build checks. See [test/README.md](test/README.md).

Use `python tools/validate.py --checks all` for the declared local release scope.
Accepted names are native, contracts, doxygen, python, package, arduino, idf-s2
and idf-s3; comma-separated subsets label their narrower scope. CTest ownership
labels are disjoint: adding a suite requires assigning it to one group. Do not
run unittest discovery over the HIL modules: the existing hil_runner registration
already loads those modules once. The validator checks inventory and actual JUnit
execution counts; a missing group, disabled test or zero execution fails.
See [validation prerequisites and exits](docs/validation.md#local-validation-entry-point).
Run focused malformed-input fixtures first, then the combined entry point; avoid
immediately repeating its full suite through a second wrapper. HIL and remote CI
are never selected here. Missing build tools leave the requested scope incomplete.
The simulated ordinary host consumer is also a runtime CTest. Generic owner
cases use public APIs and externally serialized callbacks; keep FieldCore paths
as read-only evidence, never build dependencies. See [integration](docs/integration.md).
Run `python tools/test_tusb32x_hil_runner.py` and the existing live transcript
through CTest when editing sessions or record contracts. These tests need no
pyserial; the pinned optional hardware dependency is tools/requirements-hil.txt.
An actual port must be selected explicitly for the read-only serial entry point;
never use test completion as hardware qualification. See [sessions](docs/hil-runner.md).
Readonly/soak observation and dry-run checks use those same registrations. A
portless `--dry-run --variant tusb320lai --address 0x47 --suite readonly --out
build/readonly-plan` saves the concrete plan and exits 2 with hardware NOT RUN.
Never add physical HIL invocation to ordinary tests or validation commands.
Reset-termination/fault/console plans use the same runner/transcript registrations.
For focused recovery checks run `python tools/test_tusb32x_recovery.py`. Keep exact
negative outcomes declared before admission, actual status/detail/effects retained,
and reboot/reconnect identity/ownership fresh. Physical actions require separate
fixture authorization; tests only model them. See the [runner guide](docs/hil-runner.md).
Native application builds use the pinned IDF5.5.5 environment and independent
absolute build/SDKCONFIG paths, including on incremental builds. See the
[native application guide](examples/esp_idf/basic/README.md) for wiring and commands.
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
The diagnostics suite exercises raw safe-image rejection, pre-callback
invalidation, normalized all-source versus managed counters, retained effects and
pure health thresholds. Source-stat clearing cannot repair trust or reset health.
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
environment is usable; the Arduino/native guides record separate firmware links.
[Development metadata](docs/development.md) defines version exits, pins, package
allowlists, ownership and evidence limits. Extend the existing version fixture
suite as guards mature; do not add a competing version source or fixture suite.

Review the complete scoped diff, run the affected checks and update
[progress](docs/implementation-progress.md). The lead stages owned changes,
reviews the staged diff, commits and synchronizes the established upstream without
rewriting history. Remote CI belongs only to prompt 34 in this initial execution;
preserve required hooks and merge/release protections. Firmware links, package
qualification and physical hardware checks remain separate evidence.

The [declared CI matrix](docs/development.md#declared-ci-matrix) has five Linux
host rows: GCC 14 and Clang 18 in Debug/Release, plus Clang 18 Debug with ASan/UBSan.
Python, documentation and package suites each have one Release owner. Four Arduino
images and native IDF 5.5.5/6.1 images on S2/S3 consume the same identified archive;
do not add another identical source-checkout firmware gate. All workflow jobs use
finite timeouts and retain useful logs, JUnit and package/firmware evidence for
seven days. [Checkpoint 40778512875](https://github.com/janhavelka/TUSB32x/actions/runs/40778512875) passed all 14 jobs at
`9aa8898c65ca1eebc5f7ea241dfac4bbc815283b`. Later edits use their own local checks and
retain this actual CI SHA; documentation commits do not require another remote gate.

Before editing CI, install its source-checkout parser dependency and run:

```text
python -m pip install PyYAML==6.0.3
python tools/check_ci.py
```

The checker reads the actual YAML, verifies action/release pins, matrix rows,
ownership and artifact bounds. Its negative fixtures belong to the registered
`validate_runner` suite. CI pins Python 3.12.16 on Linux; this is not a new
consumer dependency or a Windows binary requirement.

For package changes also run:

```text
python tools/test_check_package.py -v
scripts\pio.cmd pkg pack . --output build/TUSB32x.tar.gz
python tools/check_package.py --archive build/TUSB32x.tar.gz --out build/package-check
```

The three registered package suites run safety fixtures, a real test-archive
consumer and complete injected-header provenance negatives; PlatformIO packing is a separate explicit maintainer command. Keep
exports literal and identical in library.json/idf_component.yml. Preserve both
example identity scripts, licenses and all shared/frontend sources. Never add
local sdkconfig, locks, build outputs, private evidence or vendor artifacts.
Review the clean-consumer origin report and execute it, rather than accepting
source-tree compilation as package proof. When output is inside the checkout,
compilation uses a separate system temporary workspace; console/JSON evidence
reports its absolute path. Artifacts are retained for review.

Accepted input is bounded gzip POSIX tar with regular files and optional per-file
mtime-only PAX metadata, matching the managed packer. Reject unsupported archive
extensions instead of guessing their path/size semantics. This is path/content
validation of trusted local source, not authentication or a CMake sandbox.
Before release, pack and check the exact revision and link extracted Arduino and
genuine native-IDF S2/S3 consumers with independent SDKCONFIG paths. The selected
rows and commands are in [development](docs/development.md#archive-and-clean-consumers)
and [validation](docs/validation.md). Prompt 34 declares the matrix above and owns
its remote checkpoint; configuration alone is not verification. No tag, registry
upload or release publication is implied by package checks.

For recovery plan edits, test base power-cycle readiness across separate session
groups as well as within one combined plan. The saved run fact takes precedence
over the initial fixture declaration; fresh typed recovery must still require its
physical readiness/authorization and verified completion. Do not clear that fact
on raw reads, initialization, reconciliation or reset. Keep positive and negative
native transcript evidence visibly simulated.
