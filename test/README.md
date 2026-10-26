# Native contract, validator, transport, codec, owner and version checks

Prompts 5-8 provide real value/Status, validator and version checks, syntax-only planned
methods/owner examples, compile-time specification arithmetic and a library build.
Prompt 9 adds private terminal transport and exact identity/revision checks.
Prompt 10 adds public supplied-byte decoding, private encoding/comparison and
safe composition tests. Prompt 11 adds the delivered owner facade and read-only
initialization, lifecycle, result ownership and fresh-time behavior. Port
acquisition, other mutating families and health projection remain planned. Prompt 12 adds
read-only profile observation/reconciliation and its conservative reset-settling
wait. Prompt 13 adds explicit profile application/verification, literal role
sequences, ambiguous-write effects and independent sub-ms timing fixtures;
software owner-time tests do not prove physical hardware timing.

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
NDEBUG in Release. Tests ON require Python 3.10+ and CTest must run these suites:

- `tusb32x_compile_contracts`: executed passive/Status checks across two TUs.
- `tusb32x_validators`: variant/address/callback/timeout checks, exhaustive valid
  profile combinations, invalid enum bytes, input preservation and zero counting
  callbacks; unavailable valid fields return UNSUPPORTED_FEATURE.
- `tusb32x_protocol`: fixed scripted exact address/kind/bytes/length/timeout and
  extra-call expectations; variant/register/range gates, pointer every read,
  scratch publication, legal/illegal callback status/detail, write acceptance,
  saturating all/managed accounting, independent instances and TI-derived
  identity/revision cases. Independent codec tables cover every byte and variant,
  all small typed domains, reserved/absent validity, complete error-output
  preservation, settings comparison and actual-versus-staged write composition.
  Literal frames verify ACK/settings/reset at the transport boundary. Mode and
  recovery checks prove composition prerequisites, not executed/timed procedures.
  The public decoder is also linked by the standalone consumer without private includes.
- `tusb32x_operations`: every public owner method is compiled, linked and called.
  Exact read-only initialization scripts cover all seven variants, 5/6-callback
  ceilings and absence of writes. Each callback boundary exercises backend
  failures, cancellation, end, invalidation, owner expiry and failed
  reinitialization. Tests cover atomic binding, copied inputs, token precedence,
  unchanged outputs, retained results, exact take, association changes including
  null contexts/both callbacks, independent instances on one serialized fake bus,
  separate configuration/termination/reset facts and transport-statistics clearing.
  A complete clear-register reinitialization after observed reset, end and same-target
  bind must retain pending reset evidence on all seven variants.
  Budgets 0/1/larger, fresh pending completion, late success versus original backend
  failure, timestamp validity, rollover, backward/half-range rejection and clipped
  callback timeouts are executed. A bounded private token-arithmetic fixture
  reaches exhaustion without a public seed API or an exhaustion loop.
  `test_profile_observation.cpp` extends the same executable with exact P/F plans
  for all seven variants: scalar 08/09/0A and applicable 45, plus identity/revision
  only for reconciliation. P uses 3/4 callbacks and 3/4 received bytes; F uses
  5/6 callbacks and 12/13 received bytes. Both permit at most one callback per
  poll, including with larger budgets, and perform no writes. Tests exercise
  reconciliation without/after failed initialization, external invalidation,
  observation without desired adoption/trust promotion, repeated verified
  generation increments, persistent contradiction, historical-image preservation,
  reserved typed fields with complete raw evidence, both independent termination
  controls, reset evidence and semantic TUSB322I DRP00/11 equivalence. RO/W1C bits
  do not create persistent mismatches. Matching signature followed by failed
  revision acquisition preserves earlier valid identity/configuration evidence.
  Four literal full-profile vectors per variant and operation check publication
  of every valid current, duty, mode, preference, debounce and disable value,
  complete raw-byte preservation and absent-control validity. Profile observation
  creates no desired image, verified generation or configuration trust.
  Every required read exercises all six terminal transport errors; cancellation,
  end and expiry cover every read and the final fresh-time boundary. Partial
  acquisitions preserve the complete old observation, and immediate adverse
  evidence survives cancellation. First error/detail, retained result delivery,
  rollover, invalid owner time and clipped callback timeout are checked.
  Separately established termination evidence from P is invalidated immediately
  when a later partial P contradicts either disable control, including when an
  unrelated reserved field prevented a complete typed profile or trust.
  Reset first discovered during F ends UNKNOWN with no extra F; a later explicit
  reconciliation waits 96 ticks from its first fresh sample and performs one F.
  Wait rollover, cancellation/expiry and each incomplete post-wait F boundary
  cannot claim reset completion. Read-only results retain
  HardwareEffect::NONE and port invalidation.
  `test_profile_application.cpp` adds linked public apply/verify coverage for all
  seven variants. Literal scripts check no-op F without writes, whole-candidate
  rejection and desired-copy/generation preservation, nonzero actual fields,
  base detached precheck before any write, attached current-only updates,
  independent early/late 45 controls and separate disable/mode/enable writes.
  Initial and requested-final disabled states are covered. Literal UFP role
  writes execute on all seven variants. Ordinary 0A debounce and
  termination-only changes execute one write plus F, finishing at equal fresh
  owner ticks without a role sequence or wait; both termination directions are
  checked where supported. TUSB322I changes to
  advertisement/duty/accessory and direct return to DRP reject locally; DRP00
  equivalence and independent raw fault evidence remain observable.
  Six terminal backend errors are injected at every write and following F read
  of a complete six-write plan, both before acceptance and after possible
  acceptance. Every callback/final-time boundary exercises cancel, end, external
  invalidation and expiry, retaining the first error/detail, desired input,
  hardware effects, old complete observation and exact callback accounting.
  Callback entry checks configuration/port invalidation before mutation.
  An independent microsecond clock floors owner samples: mode completion at
  108.1/108.9 cannot enable at 113, a fresh108 sample permits114, and a fresh110
  sample permits116. Requested-disabled L/A also waits before F; TUSB322I adds
  no 5ms wait. Exact deadline equality prevents enable, and a late final read
  cannot publish success. Budgets0/1/larger retain the one-callback ceiling.
  Complete mismatching F publishes KNOWN actual settings independently of desired
  and returns CONFIGURATION_MISMATCH. Read-only verify/reconcile never write;
  reconciliation after an ambiguous mode write can establish different actual
  controls and a later explicit apply makes progress. No desired setter, private
  state seed, automatic rollback, cleanup or replay is used.
  Interrupted original-part mode writes retain transition uncertainty after both
  successful and failed callbacks. Complete read-only F clears it, with no added
  mode wait; differing actual controls can still be KNOWN with mismatch.
  Active apply retains the previous terminal diagnostic effect/kind/status while
  separate flags expose live configuration uncertainty; termination updates the
  diagnostic history with the new result.
- `tusb32x_version_sync`: 18 isolated Python fixture tests for version truth,
  drift/CLI exits, alternate-form duplicate rejection, no-op bytes/mtimes,
  no Git or enclosing identity, and dirty example independence. The suite cannot
  alter checkout metadata.
- On Windows, `tusb32x_pio_launcher`: two temporary CMD fixtures check child
  exit codes with inherited ERRORLEVEL shadows and missing-install diagnostics.
  No PlatformIO installation is needed for these fixtures.

The build separately compiles each public header as the only include and the
three contract fixtures as objects. Their future signatures and owner examples
are compile checks, with no behavioral CTest entry or stub driver definitions. The
[ordinary multi-file consumer](../examples/host/basic/README.md) must also build
and run separately with tests OFF. It links real validators/decoder and needs no Python,
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

The compile-only fixture declares future signatures outside `include/` in
`TUSB32x::planned::Driver`, which inherits the delivered public facade. It neither
redefines that facade nor redeclares implemented methods. Future declarations are
never packaged, linked into the core or used to claim operation behavior. As real
methods arrive, migrate their checks to the delivered API and remove the sketch
declarations. Do not create definitions merely to make this sketch link.

`owner_examples.cpp` type-checks passive observation, explicit apply, cancellation
after a possible write and two devices on one external fake owner. Each owner
turn samples time and advances at most once, then exact-takes only a terminal
result. The fake callbacks return I2C_ERROR; nothing pretends to initialize real
hardware. `time_arithmetic.cpp` evaluates 60 independent static assertions for
unsigned half-range comparisons, equality, rollover, deadline/callback caps,
wait margins and checked 32-bit clock extension. These validate specification
arithmetic and example syntax; runtime owner behavior is exercised separately by
`tusb32x_operations`. Neither proves adapter timing or hardware waits.
Future transition/fault requirements O01–O25 are in the
[API guide](../docs/api-contracts.md#required-future-behavioral-checks).

Run current version/documentation checks with `python scripts/generate_version.py check`
and `doxygen Doxyfile`; output stays under build/doxygen. Inspect XML/HTML for all
17 public structs, the owner class and methods, both validators, decodeRegister and
four generated version constants.
The complete omission checker and negative fixtures remain prompt 33 work.

For source documentation, prompt 5 also ran Doxygen directly with a temporary
configuration: INPUT=include/TUSB32x, RECURSIVE=YES, EXTRACT_ALL=NO,
WARN_IF_UNDOCUMENTED=YES, WARN_IF_DOC_ERROR=YES, WARN_IF_INCOMPLETE_DOC=YES,
WARN_NO_PARAMDOC=YES, WARN_IF_UNDOC_ENUM_VAL=YES, WARN_AS_ERROR=YES,
GENERATE_XML=YES and GENERATE_LATEX=NO. Inspect generated XML for all 17 public
structs; no warning alone would not prove their visibility. The retained evidence
and exact command are in [validation](../docs/validation.md).

These checks prove current portable values, pure validators, private register
transport/identity/codecs, owner lifecycle/profile behavior, linkage and planned
signature syntax. Later operation families, health projection, framework builds, complete
package qualification and physical hardware tests remain separate work.
