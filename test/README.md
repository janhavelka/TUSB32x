# Native contract, validator and version checks

Prompts 5-8 provide real value/Status, validator and version checks, syntax-only planned
methods/owner examples, compile-time specification arithmetic and a library build.
The device facade and behavioral driver suites remain planned.

Run from the repository root using CMake 3.16 or newer and a C++17 compiler:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Set `-DCMAKE_BUILD_TYPE=Debug` at configure time for a single-configuration
generator (for example Ninja). Repeat in `build/release` with Release selected
at configure/build/test time. Tests default OFF. Strict GNU/Clang/MSVC warnings
are private; suite checks stay active with NDEBUG, and test targets also undefine
NDEBUG in Release. Tests ON require Python 3.10+ and CTest must run these three suites:

- `tusb32x_compile_contracts`: executed passive/Status checks across two TUs.
- `tusb32x_validators`: variant/address/callback/timeout checks, exhaustive valid
  profile combinations, invalid enum bytes, input preservation and zero counting
  callbacks; unavailable valid fields return UNSUPPORTED_FEATURE.
- `tusb32x_version_sync`: 17 isolated Python fixture tests for version truth,
  drift/CLI exits, no-op bytes/mtimes, no Git or enclosing identity, and dirty
  example independence. The suite cannot alter checkout metadata.

The build separately compiles each public header as the only include and the
three planned-contract fixtures as objects. They are compile checks, with no
behavioral CTest entry or stub driver definitions. The
[ordinary multi-file consumer](../examples/host/basic/README.md) must also build
and run separately with tests OFF. It links real validators and needs no Python,
Git, Doxygen, PlatformIO, framework headers or sibling include paths.

The older direct compiler commands below remain useful for focused passive-only
checks; CMake/CTest above is the full registered native check.

Run from the repository root with a C++17 compiler. These PowerShell commands
use the available GCC and put generated files outside the source tree:

```powershell
$contractOutput = Join-Path $env:TEMP 'tusb32x-contract-checks'
New-Item -ItemType Directory -Force -Path $contractOutput | Out-Null
$contractFlags = @('-std=c++17', '-Wall', '-Wextra', '-Wpedantic',
             '-Wconversion', '-Wsign-conversion', '-Werror', '-I', 'include')
foreach ($header in @('Status.h', 'Config.h', 'TUSB32x.h', 'CommandTable.h', 'Version.h')) {
    "#include <TUSB32x/$header>" | & g++ @contractFlags -x c++ -fsyntax-only -
    if ($LASTEXITCODE -ne 0) { throw "Header failed: $header" }
}
& g++ @contractFlags -DNDEBUG test/test_compile_contracts.cpp `
    test/consumer/status_other_tu.cpp -o "$contractOutput/passive-contracts.exe"
if ($LASTEXITCODE -ne 0) { throw 'Passive consumer compile/link failed' }
& "$contractOutput/passive-contracts.exe"
if ($LASTEXITCODE -ne 0) { throw 'Passive consumer checks failed' }
& g++ @contractFlags -fsyntax-only test/contract/planned_api.cpp `
    test/contract/owner_examples.cpp test/contract/time_arithmetic.cpp
if ($LASTEXITCODE -ne 0) { throw 'Planned contract compilation failed' }
```

The linked executable checks all 23 error names/order across translation units,
constant evaluation, explicit success conversion, invalid error names, full
int32 backend details, callback shapes, exact addresses and fixed passive values.
Runtime checks remain active with NDEBUG. Defaults preserve raw and interpreted
validity separately. No driver/decoder function is linked or simulated.

The compile-only fixture declares future signatures outside `include/`. It is
never packaged, linked into the core or used to claim operation behavior. As real
methods arrive, migrate their checks to the delivered API and remove corresponding
sketch declarations. Do not create definitions merely to make this sketch link.

`owner_examples.cpp` type-checks passive observation, explicit apply, cancellation
after a possible write and two devices on one external fake owner. Each owner
turn samples time and advances at most once, then exact-takes only a terminal
result. The fake callbacks return I2C_ERROR; nothing pretends to initialize real
hardware. `time_arithmetic.cpp` evaluates 60 independent static assertions for
unsigned half-range comparisons, equality, rollover, deadline/callback caps,
wait margins and checked 32-bit clock extension. These validate specification
arithmetic and example syntax, not the unimplemented engine or adapter timing.
Future transition/fault requirements O01–O25 are in the
[API guide](../docs/api-contracts.md#required-future-behavioral-checks).

Run current version/documentation checks with `python scripts/generate_version.py check`
and `doxygen Doxyfile`; output stays under build/doxygen. Inspect XML/HTML for all
17 public structs, both validators and the four generated version constants.
The complete omission checker and negative fixtures remain prompt 33 work.

For source documentation, prompt 5 also ran Doxygen directly with a temporary
configuration: INPUT=include/TUSB32x, RECURSIVE=YES, EXTRACT_ALL=NO,
WARN_IF_UNDOCUMENTED=YES, WARN_IF_DOC_ERROR=YES, WARN_IF_INCOMPLETE_DOC=YES,
WARN_NO_PARAMDOC=YES, WARN_IF_UNDOC_ENUM_VAL=YES, WARN_AS_ERROR=YES,
GENERATE_XML=YES and GENERATE_LATEX=NO. Inspect generated XML for all 17 public
structs; no warning alone would not prove their visibility. The retained evidence
and exact command are in [validation](../docs/validation.md).

These checks prove current portable values, pure validators, linkage and planned
signature syntax. Protocol, transport execution, owner behavior, health projection, framework builds,
full package qualification and physical hardware tests remain future work.
