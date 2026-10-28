# Native core, local CLI, console, transcript and version checks

Prompts 5-8 provide real value/Status, validator and version checks, syntax-only planned
methods/owner examples, compile-time specification arithmetic and a library build.
Prompt 9 adds private terminal transport and exact identity/revision checks.
Prompt 10 adds public supplied-byte decoding, private encoding/comparison and
safe composition tests. Prompt 11 adds the delivered owner facade and read-only
initialization, lifecycle, result ownership and fresh-time behavior. Prompt 12 adds
read-only profile observation/reconciliation and its conservative reset-settling
wait. Prompt 13 adds explicit profile application/verification, literal role
sequences, ambiguous-write effects and independent sub-ms timing fixtures;
software owner-time tests do not prove physical hardware timing.
Prompt 14 adds explicit reset and original TUSB320/TUSB320I recovery, literal
command/procedure scripts, retained sequence history and physical-clock wait checks.
Prompt 15 adds complete port acquisitions, applicability, coherence and historical
freshness through the same public owner engine.
Prompt 16 adds explicit ACK and finite interrupt service, sampled latch evidence,
safe actual-control preservation and retained pre-ACK port observations.
Prompt 17 adds `tusb32x_diagnostics`: bounded raw access, safe-image/sequence
rejection, pre-callback invalidation, all-source/managed accounting, retained
effects and pure health projection. Debug and Release checks stay active.

Prompt 19 adds fixed-capacity external owner scheduling and an isolated linked
public consumer with allocation/failure sentinels. See the
[owner portability matrix](../docs/owner-portability.md) for named cases and limits.

Run from the repository root using CMake 3.16 or newer and a C++17 compiler:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Prompt 20 also registers `tusb32x_cli`, `tusb32x_console` and `cli_transcript`.
The last runs one persistent `tusb32x_cli_host` process through Python; the fixture
executable has no build-only passing CTest. To run the implemented local subset:

```text
ctest --test-dir build/native -C Debug -R "tusb32x_cli|tusb32x_console|cli_transcript" --output-on-failure
```

All 55 canonical rows, aliases and help topics run through
the real shared example sources. Tests cover zero callback deltas, whole-command
rejection, staged/core separation, cross-family target edits, retained effects,
poison/CRLF/backspace, cumulative input 64/one command, queue 8192/record 512/drain 256,
whole-record loss and fairness, maximum-width formatter paths, unsafe injected
messages, request exhaustion and typed schema/closure negatives. The Python rows
are compared with executed C++ HELP records. See the [implemented CLI](../docs/CLI.md).

Prompt 21 extends these same registrations. `cli/test_cli_operations.cpp` invokes
every managed route and inspects literal callback bytes and fixture registers:
admission/desired adoption, manual/automatic zero/one/255 budgets, immutable and
clipped deadlines, late success, invalid owner clocks, reset/recovery waits, IRQ
ACK versus staged/applied values, W1C-preserving apply, ambiguous writes, failed
readback and cancellation. Core-only result retention, stale tokens, repeated
query, one take, conservative output reservation and lost announcements use runtime
behavior. Request 24 may complete after request 25 and a changed marker. The
existing C++ suite additionally exercises maximum-width result formatting and
continued owner progress after request exhaustion. The persistent transcript uses
the actual shared C++ fixture and separate transport/readback fault sessions.

Prompt 22 completes diagnostics in `cli/test_cli_diagnostics.cpp`: exact raw
callback bounds/status normalization and signed detail, reserved/variant decode,
safe-write effects and exclusion, all-source versus managed counters/health,
idle-only clear and saturation rejection, input availability, invalidation/forget
gates, profile mismatch/time evidence and color/verbosity parity. A failed raw
write under full output retains uncertainty and never retries. Console checks
keep full diagnostic records atomic through zero/short writes and loss recovery.

The same `cli_transcript` registration now covers diagnostics and input/raw fault
sessions. `cli_contract` and `cli_contract_negative` execute the sole checker with
`$<TARGET_FILE:tusb32x_cli_host>` and finite 60-second timeouts. Its help comparison
is reused by the transcript; a broken alias or required record field must fail.
Windows has 14 registered CTests, other hosts 13. No build-only passing fixture
or second transcript registration is added.

```text
ctest --test-dir build/native -C Debug -R "tusb32x_cli|tusb32x_console|cli_transcript|cli_contract" --output-on-failure
python tools/check_cli_contract.py --host build/native/test/Debug/tusb32x_cli_host.exe
python tools/check_cli_contract.py --host build/native/test/Debug/tusb32x_cli_host.exe --self-test-negative
```

Set `-DCMAKE_BUILD_TYPE=Debug` at configure time for a single-configuration
generator (for example Ninja). Repeat in `build/release` with Release selected
at configure/build/test time. Tests default OFF. Strict GNU/Clang/MSVC warnings
are private; suite checks stay active with NDEBUG, and test targets also undefine
NDEBUG in Release. Tests ON require Python 3.10+ and CTest must run these suites:

- `tusb32x_portable_consumer`: copies public headers, the built library and two
  consumer translation units into a fresh tree; builds/runs without repository or
  framework dependencies. Checks all current public methods, allocation traps and
  deliberate failure exits. See [consumer instructions](consumer/portable/README.md).
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
- `tusb32x_operations`: lifecycle/profile/reset/recovery owner methods are compiled,
  linked and called.
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
  `test_reset_recovery.cpp` links both delivered public starts. Reset scripts
  exercise all seven variants with exact E8/EA command composition, a single
  command attempt, 96 elapsed ticks and one complete F. Bounds are B7/L/A8/V7
  callbacks, 8/9/8 TX bytes and 13/14/13 RX bytes. Live prechecks differing
  from cached proof on every variant supply the reset command's actual controls.
  Contradictory post-reset identity or revision immediately invalidates readiness
  and retains partial effects.
  Reset and recovery callbacks also verify deadline-clipped timeout arguments.
  A staged desired profile survives reset with different actual controls and is
  never replayed or used as reset's success comparison. TUSB322I semantic
  DRP00/11 succeeds; UFP/DFP readback gives CONFIGURATION_MISMATCH. An asserted
  reset bit gives bounded CONFIGURATION_UNKNOWN with no second command or read loop.
  Reserved advertisement together with non-DRP mode cannot certify TUSB322I
  reset completion; asserted reset also takes precedence over role mismatch.
  Recovery scripts run only on original TUSB320/TUSB320I and require identity.
  Initial internal45 values01/02/03 reject before writes. Qualified scripts
  execute literal R45/W4504/wait31/W4500/R45 with four callbacks, six TX and
  two RX bytes. Final low-three-bit mismatches fail without successful sequence
  proof. No transient port observation is read or published. Six backend errors
  are injected at each callback; each write runs both possible acceptance cases.
  Cancellation, end, external invalidation and owner expiry cover every callback,
  anchored wait and final fresh-time boundary. Effects, first error/detail,
  callback accounting, retained results and bus-silent rejection are checked.
  Independent microsecond completion at 200.1/200.9 or 300.1/300.9, with delayed
  fresh owner samples, cannot advance the 31/96-tick wait early; stalled ticks,
  zero budget, wait rollover and deadline equality do no early transport work. End/rebind and
  read-only reconciliation retain ambiguous reset acceptance and interrupted
  recovery history. Clear controls alone cannot certify the older recovery;
  a new explicit recovery can resolve it while configuration awaits full proof.
  A newer successful reset preserves that older unresolved recovery history.
  Later reset success also preserves earlier ambiguous command acceptance,
  while its complete waited F can resolve an interrupted mode transition.
- `tusb32x_port`: linked public `startRefreshPort` and cached `portSnapshot`,
  with literal 09/08/09 scripts on all seven variants. Independent expected
  tables cover 14,336 complete acquisitions across all 256 register08 values,
  all four attachment states and both directions, including reserved
  advertisement/accessory codes. Another 1,792 observations cover every final09
  byte, IRQ levels, raw fault/reserved/control preservation and low-bit changes
  excluded from E0 coherence comparison. Every unequal pair of attachment/direction
  bookends clears cross-register validity while preserving complete raw evidence
  and independently valid final09 fields. No production decoder supplies expected
  values. Current indication, local/peer role, detached inapplicability,
  charge-through and variant-specific debug meanings remain separate.
  An explicit CC/VBUS fixture varies both independently: only the combined
  condition reports sink attachment. Current/cable bits alone imply no attach;
  this fixture assumption does not qualify physical Type-C behavior.
  Each required read injects all six terminal backend errors. Every callback and
  final fresh-time boundary exercises cancellation, end, external invalidation
  and owner expiry, preserving the old complete snapshot, acquisition time,
  sequence, configuration provenance and prior invalidated state. No failed
  partial receive fabricates a detach. Delayed publication retains the first
  read's owner timestamp; rollover, backward time, clipped timeout, budgets0/1/
  larger and retained result ownership are checked. A private arithmetic fixture
  tests sequence saturation without a public state seed.
  Public earlier operations prove that read-only configuration verification,
  reconciliation and no-op apply never refresh port data. Possible apply/reset/
  recovery writes invalidate before callback entry, including ambiguous errors;
  reset/recovery transient reads preserve historical port data. Admission and
  pre-write cancellation retain valid port evidence. Complete port refresh can
  clear port invalidation while configuration uncertainty remains. Successful
  contrary08/09 evidence downgrades configuration immediately without changing
  an older port image when a later read fails, is cancelled or expires; a complete
  observation retains configuration provenance from before its first read.
  A contradiction first seen in final09 revokes trust before fresh publication,
  cancellation or expiry; rejected backward-time polling preserves that evidence.
  `test_interrupt.cpp` links both public interrupt starts. Six supported variants
  execute exact ACK R09/W09 and service R09/R08/R09/W09/R09 plans: respectively
  two/five callbacks, three/six TX bytes and one/four RX bytes. Every poll invokes
  at most one callback, including larger budgets; zero budget permits fresh
  pending-data processing but no callback. TUSB322I rejects both starts locally
  because its separate raw fault bit3 has unresolved write access semantics.
  Independent literal vectors cover B4->14, B5->15 despite staged02, actual50
  versus staged30, 06->16, all256 fresh09 values and already-UNKNOWN trust.
  Service bookends with differing RW controls require final06/07->ACK16/17,
  including independent IRQ changes in both directions. Changed attachment/
  direction bookends retain final raw/role evidence and withhold dependent
  current/accessory/cable validity through successful service or ACK/recheck errors.
  Complete pre-ACK data retains acquisition time, sequence and configuration
  provenance through ACK/recheck failure; a final64 recheck cannot replace the
  complete prior sink observation. A contradiction first seen only in recheck
  immediately revokes configuration trust while preserving the complete port
  publication through success, cancellation, expiry and rejected backward time.
  Every callback injects six terminal transport errors; failed ACKs run both
  before and after simulated latch clearing. Truncated failed reads cannot
  publish complete observations. Cancellation, end, external invalidation and
  deadline equality cover every callback/final-fresh-time boundary. Tests keep
  NONE/PARTIAL/INDETERMINATE/CONFIRMED effects, original errors/details, retained
  results, diagnostic IRQ history and per-operation empty/partial IRQ records
  separate. Public profile changes for every individual field, including all
  role-stage writes, preserve a pending latch rather than accidentally ACK it.
  An explicit coalescing fake schedules reassertion between observation and ACK,
  during ACK, before recheck and after its sample, plus stuck-latch and no-pin
  polling cases. It assumes non-consuming reads and explicitly scheduled events;
  the TI RU-change/CSR wording is not treated as every-write IRQ behavior or as
  evidence for fault clearing, physical GPIO timing or lossless edge counting.
- `tusb32x_diagnostics`: public raw safety, all-source/managed accounting and pure
  health projection. See the [diagnostics cases](#raw-diagnostics-and-passive-health)
  below for retained effects, safe images, saturation and projection boundaries.
- `tusb32x_version_sync`: 18 isolated Python fixture tests for version truth,
  drift/CLI exits, alternate-form duplicate rejection, no-op bytes/mtimes,
  no Git or enclosing identity, and dirty example independence. The suite cannot
  alter checkout metadata.
- On Windows, `tusb32x_pio_launcher`: two temporary CMD fixtures check child
  exit codes with inherited ERRORLEVEL shadows and missing-install diagnostics.
  No PlatformIO installation is needed for these fixtures.

The build separately compiles each public header as the only include and the
three contract fixtures as objects. Their delivered signatures and owner examples
are compile checks, with no behavioral CTest entry or stub driver definitions. The
[ordinary multi-file consumer](../examples/host/basic/README.md) must also build
and run separately with tests OFF. It links real validators/decoder/health and needs no Python,
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
if ($LASTEXITCODE -ne 0) { throw 'API contract compilation failed' }
```

The linked executable checks all 23 error names/order across translation units,
constant evaluation, explicit success conversion, invalid error names, full
int32 backend details, callback shapes, exact addresses and fixed passive values.
Runtime checks remain active with NDEBUG. Defaults preserve raw and interpreted
validity separately. No driver/decoder function is linked or simulated.

The historical `planned_api.cpp` filename now checks only delivered public
signatures. The former future-declaration header and derived fixture facade were
removed when the final raw/health methods gained definitions. These compile-only
checks are never registered as behavioral CTests or packaged with the library.

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
17 public structs, the owner class and methods, binding/profile/health validators, projectHealth, decodeRegister and
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
transport/identity/codecs, owner lifecycle/profile/port/IRQ/raw behavior, passive
health, linkage and signature syntax. Framework builds, complete
package qualification and physical hardware tests remain separate work.

## Raw diagnostics and passive health

Run the focused registered suite with:

```text
ctest --test-dir build/native -C Debug -R tusb32x_diagnostics --output-on-failure
```

`test_diagnostics.cpp` links the real public raw and health functions. It checks
all variant/map cases, exact one-callback frames, unchanged failed-read output,
active/result backpressure, complete safe-image requirements, same-value writes,
possible applied-then-error effects, staged/actual disagreement and unsafe-sequence
rejection. Protected45 observations revoke raw and typed45 writes independently
of current-only changes; only complete proof renews permission. Reset observed
through raw reads still requires reconciliation's fresh96-tick anchor.

Both raw directions exercise every uint8 callback code with full backend detail.
Raw callbacks contribute once to all-source totals and never managed health.
Private protocol-context fixtures exercise saturation and unusable timestamp
storage at arithmetic limits without exposing a public state seed. Projection
tests cover thresholds1/3/UINT32_MAX, invalid0 unchanged output, all state rows,
independent instances and initialized READY with UNKNOWN settings. Runtime cases
prove OFFLINE allows explicit work and managed success clears the streak while
raw success/failure, detach and source-stat clearing do not. Result delivery and
clearing preserve unresolved recovery, trust, port age and active/retained slots.
The suite is behavioral CTest evidence; it does not qualify physical adapters.

## Protocol conformance

The [requirement matrix](../docs/protocol-conformance.md) maps TI/software rules
to real runtime functions and registered suites. `test_protocol_conformance.cpp`
is part of `tusb32x_operations`; it checks public initialization, refresh, ACK
(or the 322I gate) and raw reads at both legal addresses for all seven variants.
Expected addresses, identity bytes and frames are independent literals.

The scripted transport rejects short receives falsely marked OK; a Status-only
callback cannot tell the core how many bytes arrived. Existing failed-receive
tests separately check unchanged outputs and complete observation retention.
Diagnostics tests cover both hardware outcomes of every supported failed raw
write and ACK. A later successful identity-byte read cannot repair write trust
or erase the original transport failure. These cases reuse the existing fixed
fake and counters without another engine or simulation framework.

The independent prompts 16-18 audit adds
`equivalentModeObservationsRevokeStaleWriteImages` for 322I raw/partial profile
observations in both DRP encoding directions. It preserves semantic KNOWN state
while rejecting stale raw/typed 0A writes until complete proof. The operations
suite also runs `tusb322ModeWriteFailures` and
`tusb322ModeInterruptionBoundaries`: literal DFP/UFP sequences, both failed-write
acceptance outcomes, cancel/deadline after every callback and final fresh time,
retained original failure/effects and no hidden retry or borrowed L/A delay.

## CLI audit regressions

`setupAndAppearanceContracts` checks one heading per topic and contiguous actual
HELP records. `unannouncedCompletionTakeReservation` covers both capacity outcomes
when a core result is ready before the CLI's next completion announcement. The
persistent transcript injects schema-valid duplicate/contradictory terminal and
late payload records captured from the real fixture; request closure is separate
from operation-token completion order. Existing alias/schema negatives remain in
the same registered checker. See `docs/validation.md` for current results.

## Arduino adapter checks

The registered tusb32x_arduino_wire_1000 and tusb32x_arduino_wire_100 suites compile
the actual example adapter against recording SDK headers at two tick rates.
See [fixture details](arduino_wire/README.md). The Windows host total is now 16
registered suites; Release checks remain active. Real framework links are separate:

    scripts/pio.cmd run -d test/arduino_sdk -e arduino_s2 -e arduino_s3

The pinned fixtures link all adapter entry points with the core. They do not begin
a bus, run a device demo or flash hardware. Application prerequisites and deadline
qualification are in [the integration guide](../docs/arduino-transport.md).
