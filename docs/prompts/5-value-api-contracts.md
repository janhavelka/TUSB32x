# 5 — Define portable values, errors and the API inventory

Prerequisites: prompts 1–4 are complete. Read [COMMON](COMMON.md),
[ENGINEERING_BASELINE](ENGINEERING_BASELINE.md),
[MAINTAINABILITY](MAINTAINABILITY.md) and the TI contract guides. Finish the
public vocabulary and real passive value types here. Prompt 6 finishes their
owner/state semantics; prompt 7 provides the build and real validators.

## Focused re-audit

Re-read LDC1614 and LSM6DS3TR Status/configuration declarations and tests,
TMP1x2 explicit-bool Status, and ADS1115/INA3221 profile separation. Inspect
matching source, examples, relevant runner record names, AGENTS and registration.
Record HEAD, dirty state, paths and selected differences. Compare at least two
peers for material conventions without reopening settled decisions needlessly.

## Complete the value contract

1. Use namespace/class vocabulary `TUSB32x::TUSB32x`, public include
   `<TUSB32x/TUSB32x.h>`, PascalCase types, camelCase fields/functions,
   `_camelCase` members and CAPS_CASE enum/constants. No legacy aliases.
2. Implement `Status.h` with the exact ordered Err set and
   `Status { Err code; int32_t detail; const char* msg; }` baseline contract.
   Keep `ok`, `is`, `inProgress`, explicit bool, `Ok`, `Error` and one
   `errorName`. Its constexpr definition belongs in the header so all
   translation units and constant expressions work. Invalid enum input returns
   UNKNOWN; messages have static lifetime and backend detail stays intact.
3. Document each error's origin, output preservation, traffic and effect meaning.
   TIMEOUT is owner expiry; I2C_TIMEOUT is a terminal callback failure.
   CONFIGURATION_MISMATCH describes successfully read persistent bits, not a
   bus fault. No cable-detached or OFFLINE transport errors.
4. Implement complete passive callback/configuration/settings types in the
   baseline headers. DriverConfig contains terminal callbacks/context, explicit
   variant/address and callback timeout. DeviceProfile contains only verified
   silicon settings. Pins, bus handles, health thresholds, scheduling and USB
   power/data policy stay separate.
5. Use every exact baseline enum, field, default, unit and validity rule. Validate
   semantic variant/value combinations in the later validator, not by assuming
   enum ordinals are register bits. UNSPECIFIED is invalid binding. Default
   objects do not trigger initialization writes or authorize advertised power.
6. Define the complete operation/result/snapshot/diagnostic value inventory from
   the baseline, with raw versus interpreted evidence, independent validity and
   fixed storage. Retain explicit local/peer role and not-applicable states.
   Prompt 6 freezes publication and lifetime rules before implementation.

## Plan methods without publishing missing implementations

Write the exact baseline signature inventory in `docs/api-contracts.md`: validators,
bind/end/invalidate/forget, every start method, poll/cancel/resultSnapshot/takeResult,
cached snapshots/diagnostics/stat clearing and explicitly synchronous raw access.
No begin/init/unbind aliases or new factory/scheduler/registry/Pimpl hierarchy.
One non-copyable, non-movable facade and engine remain the design.

Close the pure CLI decoder boundary here as well: specify one small namespace
`decodeRegister` function and a fixed value result in the public register-table
contract, using the existing variant/field vocabulary. Define its exact signature,
raw/unknown/reserved validity and output-on-error behavior in the API guide.
Prompt 10 implements and publishes it; prompt 22 formats its result. It accepts
supplied bytes and performs no I2C or state mutation. Do not require private
source headers, duplicate silicon decoding or a simulated driver to decode a byte.

Unimplemented callable methods stay in design documents or an explicitly named
compile-only contract fixture, outside delivered public headers. Do not expose
link promises the library cannot meet, or add success/unsupported stubs to fill
the facade. Public headers may expose complete passive values and implemented
constexpr methods now. Later implementation prompts add callable declarations
with their real code, tests and Doxygen in the same checked unit.

Record callback aggregate timeout, borrowed context, terminal buffer lifetime,
no reentry and no framework dependency in the types' documentation. Owner
time, tokens and snapshots follow the baseline; detailed transition/admission
tables are prompt 6's separate finished artifact.

## Deliver, review and commit

Update `docs/architecture.md` with file responsibilities and external-owner
boundary; start the exact API contract document and feature/API/source matrix.
Write full file/type/member/enum/function Doxygen immediately, including units,
defaults, lifetimes and static-message semantics. Update decisions, progress,
validation and affected README/CHANGELOG; label future calls as planned.

Compile each real public header independently with C++17 and strict available
warnings. Check Status constant evaluation, explicit bool, invalid error names,
wide backend detail and multiple translation units. Use a compile-only fixture
for planned signatures without linking them or claiming protocol validation.
Record actual commands; unavailable tooling is NOT RUN. Prompt 7 registers
these checks in the real build, so keep fixtures reusable.

Optional delegation: a general coding agent owns passive types while a strong
reviewer checks callback/error boundaries read-only. Freeze shared names first;
the lead integrates, reviews all Doxygen/contracts and owns Git. Commit and sync
the completed value/API inventory on the same branch, verify synchronization
and report actual check results before prompt 6.
