# Passive API contract checks

Prompt 5 provides real value/Status checks and a separate syntax-only sketch of
future methods. There is no library build or behavioral driver suite yet.
Prompt 7 will register these reusable fixtures in CMake/CTest.

Run from the repository root with a C++17 compiler. These PowerShell commands
use the available GCC and put generated files outside the source tree:

```powershell
$p5Output = Join-Path $env:TEMP 'tusb32x-p5-checks'
New-Item -ItemType Directory -Force -Path $p5Output | Out-Null
$p5Flags = @('-std=c++17', '-Wall', '-Wextra', '-Wpedantic',
             '-Wconversion', '-Wsign-conversion', '-Werror', '-I', 'include')
foreach ($header in @('Status.h', 'Config.h', 'TUSB32x.h', 'CommandTable.h')) {
    "#include <TUSB32x/$header>" | & g++ @p5Flags -x c++ -fsyntax-only -
    if ($LASTEXITCODE -ne 0) { throw "Header failed: $header" }
}
& g++ @p5Flags -DNDEBUG test/test_compile_contracts.cpp `
    test/consumer/status_other_tu.cpp -o "$p5Output/passive-contracts.exe"
if ($LASTEXITCODE -ne 0) { throw 'Passive consumer compile/link failed' }
& "$p5Output/passive-contracts.exe"
if ($LASTEXITCODE -ne 0) { throw 'Passive consumer checks failed' }
& g++ @p5Flags -fsyntax-only test/contract/planned_api.cpp
if ($LASTEXITCODE -ne 0) { throw 'Planned signature compilation failed' }
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

For source documentation, prompt 5 also runs Doxygen directly with a temporary
configuration: INPUT=include/TUSB32x, RECURSIVE=YES, EXTRACT_ALL=NO,
WARN_IF_UNDOCUMENTED=YES, WARN_IF_DOC_ERROR=YES, WARN_IF_INCOMPLETE_DOC=YES,
WARN_NO_PARAMDOC=YES, WARN_IF_UNDOC_ENUM_VAL=YES, WARN_AS_ERROR=YES,
GENERATE_XML=YES and GENERATE_LATEX=NO. Inspect generated XML for all 17 public
structs; no warning alone would not prove their visibility. The retained evidence
and exact command are in [validation](../docs/validation.md).

These checks prove the current portable values and planned signature syntax.
Protocol, callbacks, owner behavior, health projection, framework builds,
package consumption and physical hardware tests remain future work.
