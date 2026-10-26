# Contributing

Read [AGENTS.md](AGENTS.md), the [prompt index](docs/prompts/README.md) and
[COMMON](docs/prompts/COMMON.md) before implementation. Keep the existing
execution branch for this promptset. Sibling repositories are read-only evidence.
Use the [engineering baseline](docs/prompts/ENGINEERING_BASELINE.md) for public
names and [maintainability rules](docs/prompts/MAINTAINABILITY.md) for code/docs.

The implemented library contains portable values, Status helpers and pure
validators only. Add callable declarations with real definitions and link checks;
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
active in Release. Two runtime CTest suites are currently registered; compile-only
signature/header fixtures are build checks. See [test/README.md](test/README.md).
Also configure, build and run the [ordinary consumer](examples/host/basic/README.md)
with tests OFF after public/build changes. Core consumers need no Python, Git,
Doxygen, PlatformIO, framework headers or sibling paths. Use available host
sanitizers and record unavailable checks accurately in [validation](docs/validation.md).

Review the complete scoped diff, run the affected checks and update
[progress](docs/implementation-progress.md). The lead stages owned changes,
reviews the staged diff, commits and synchronizes the established upstream without
rewriting history. Remote CI belongs only to prompt 34 in this initial execution;
preserve required hooks and merge/release protections. Firmware links, package
qualification and physical hardware checks remain separate evidence.
