# Portable values, decoding and owner API

Prompts 5-18, updated 2026-10-27. The five headers under `include/TUSB32x/` provide
complete passive values, constexpr Status/token helpers, pure validators/health projection,
public pure register decoding and stable package version constants.
They require C++17 and standard headers only. The validators link through target
`TUSB32x`. Prompt 9 implements [private register transport/identity](transport.md),
without publishing bus-access methods. Prompt 10 adds pure codecs/composition.
Prompt 11 implements the non-copyable device facade, lifecycle/token/result
handling, cached evidence and read-only initialization. Prompt 12 adds
`startReadProfile` and `startReconcile`; prompt 13 adds `startApplyProfile` and
`startVerifyProfile`; prompt 14 adds explicit `startReset` and original-part
`startRecoverDeadBattery`; prompt 15 adds complete read-only `startRefreshPort`
and cached port publication; prompt 16 adds `startAcknowledgeInterrupt` and
`startServiceInterrupt`. Prompt 17 adds safe `readRegisterSync`/`writeRegisterSync`,
complete diagnostics views and pure `validateHealthPolicy`/`projectHealth`.
The callable inventory below is delivered; framework adapters and CLI remain
later work.

Prompt 18's [protocol conformance matrix](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md)
connects these owner, interrupt and raw contracts to named runtime tests. The
audit adds coverage without changing admission, effects, timing or silicon gates.

The [baseline](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/prompts/ENGINEERING_BASELINE.md), [register map](register-map.md)
and [procedure guide](device-behavior.md) remain the state and silicon contracts.
The [architecture](architecture.md) identifies file responsibilities. The
[compile-only inventory](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/contract/planned_api.cpp) checks delivered signatures
without claiming runtime behavior. See [diagnostics and health](diagnostics.md)
for safe raw usage and the separate evidence views.

TUSB322I DRP00/11 semantic equality does not prove the old literal mode bits are
still safe to write. An observed recoding revokes that 0A write image while
semantic configuration stays KNOWN. Raw or typed 0A changes then require complete
proof; raw reads cannot renew it. No-op apply, verification and reconciliation
remain read-only ways to establish the current representation.

## Implemented value inventory

All fields are ordinary fixed-storage values. Default construction performs no
I2C, allocation, validation, pin action or power authorization. Semantic enums
are not register encodings; the implemented codecs use explicit variant-specific tables.
The header Doxygen owns the complete field/default inventory.

| Header and types | Contract |
| --- | --- |
| [Status.h](../include/TUSB32x/Status.h): Err, Status, errorName | Ordered uint8 errors; full int32 detail; static-lifetime message; constexpr helpers and explicit bool |
| [Version.h](../include/TUSB32x/Version.h): VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION | Generated inline constexpr uint32 components and full SemVer character array; static lifetime, no I2C/allocation/Git/clock and no flashed-firmware identity claim |
| [Config.h](../include/TUSB32x/Config.h): I2cWriteFn, I2cWriteReadFn, DriverConfig | Borrowed terminal callbacks/context, exact variant/address, nonzero timeout cap; defaults null/UNSPECIFIED/0x47/20 ms are not a valid binding |
| Config.h: Variant, DeviceAddress | Six named 320 parts and separately gated 322I; no GPIO-only 321 target or floating-ADDR I2C option |
| Config.h: PortMode, SourceCurrent, SourcePreference, DrpDutyCycle, CcDebounce, DeviceProfile | Requested silicon settings only; default DRP/default-current/normal/50%/168 ms and false disable controls are a candidate, not reset defaults |
| Config.h: Attachment, PortRole, Orientation, DetectedCurrent, Accessory, SnapshotCoherence | Distinguish unknown, not applicable, local/peer roles and raw CC input names; no voltage/load/USB-PD permission |
| [TUSB32x.h](../include/TUSB32x/TUSB32x.h): OperationToken, OperationTiming | Nonzero instance-local uint64 token; owner uint64 millisecond admission/deadline, no clock inside core |
| TUSB32x.h: OperationKind, OperationState, OperationPhase | Exact baseline kinds/states; phase NONE/READING/WRITING/WAITING/FINISHING is coarse progress, not a private switch offset |
| TUSB32x.h: ConfigurationState, HardwareEffect, TerminationState, ResetState | Independent actual-configuration, write-acceptance, termination and reset-completion facts |
| TUSB32x.h: PortField, ProfileField and raw-valid constants | Named bit masks; decoded validity is independent of byte availability and current usability |
| TUSB32x.h: PortSnapshot | Raw08/final09, per-field interpretation, invalidation, acquisition time, sequence and configuration provenance |
| TUSB32x.h: ConfigurationSnapshot | Desired/observed/verified profiles, generations, raw08/09/0A/45 and validity, mismatch evidence, termination/reset state |
| TUSB32x.h: IdentitySnapshot | Configured variant/address, eight raw bytes with complete-read/match evidence, separate revision validity/match; shared signature never proves exact BOM |
| TUSB32x.h: InterruptSnapshot | Sampled before/after levels and validity, explicit ACK attempt and callback acceptance; no implied clear or lossless event history |
| TUSB32x.h: OperationResult, OperationProgress | Copied fixed result/evidence and bounded progress; callback counts are not wire transactions |
| TUSB32x.h: TransportStats, ManagedTransportEvidence | Saturating all-callback totals versus explicitly filtered managed-operation evidence; independent timestamp validity |
| TUSB32x.h: DriverDiagnostics | Cached identity/IRQ, lifecycle/configuration/effects, last operation and failed transport, independent unresolved obligations, managed evidence |
| TUSB32x.h: DriverState, HealthPolicy, HealthSnapshot | Passive UNINIT/READY/DEGRADED/OFFLINE vocabulary and threshold 3 default; no retry or admission policy |
| [CommandTable.h](../include/TUSB32x/CommandTable.h): RegisterDecode | Supplied-byte field codes and raw evidence only; no complete port observation or writable register mirror |

`ConfigurationSnapshot.observedValidFields` uses ProfileField bits, while
`rawConfigValidMask` records successful complete byte acquisition. Absent
controls retain neutral placeholders without validity or absent-register traffic.
Reserved encodings retain raw evidence and clear the affected decoded flag.
`hasObservedProfile` needs all applicable fields decoded; it does not establish
KNOWN trust. Historical verified values remain distinguishable from current
state. All has-flags/raw masks/timestamps start invalid, despite zero/default
storage values. Generations and sequences start zero and later saturate.

Result evidence is copied at completion and carries each aggregate's own
validity. Configuration and port copies can be retained historical observations;
success of an unrelated operation does not make them fresh. Identity raw validity
means acquisition, not current authorization: `DriverDiagnostics.initialized`
and configuration/effect evidence determine current usability. Result IRQ facts
describe that operation only; an operation that performs no IRQ work leaves its
IRQ member empty. Cached diagnostic IRQ evidence can retain the previous record.
The publication and invalidation tables below govern the delivered owner and
raw diagnostics methods.

ACK `acknowledgeConfirmed` means a terminal write returned OK. It is never proof
of a sampled-clear latch. `pendingAfterValid` and `pendingAfter` record the
separate recheck. No timestamp is fabricated from a Status-only callback.
Independent `recoverySequenceUnresolved`, `modeTransitionUnresolved` and
`resetAcceptanceUncertain` diagnostics retain obligations after result take.
`configurationWriteUnresolved` covers ordinary settings/raw writes whose current
controls still require proof, even after a later read-only result has effect NONE.
One latest-operation kind/effect cannot erase an older interrupted recovery.
Reset COMPLETED describes settling/readback, not proof of ambiguous acceptance.

## Implemented validators

`validateDriverConfig(const DriverConfig&)` and
`validateProfile(Variant, const DeviceProfile&)` are declared in Config.h and
defined in src/TUSB32x.cpp. Both perform bounded value checks with zero callbacks,
allocation, waits, retained references or input/device mutation. Returned status
messages have static lifetime and detail is zero. Success establishes only value
validity, not physical identity, observed state, backend bounds or write permission.

Binding validation requires both callbacks, a selected known variant, positive
uint32 millisecond timeout and a matching seven-bit strap address: 0x60/0x61 for
TUSB320/TUSB320I, 0x47/0x67 for LI/HI/LAI/HAI/322I. A null user context is valid.
Any invalid binding returns INVALID_CONFIG.

Profile validation checks every enum first; invalid/UNSPECIFIED variants and
unknown enum values return INVALID_CONFIG. A valid setting absent on the selected
part returns UNSUPPORTED_FEATURE. Base parts accept neutral NORMAL preference,
false accessory-disable and false disableTerm placeholders; nonneutral values
are unsupported. Base debounce is 116/133/151/168 ms; other parts support
118/134/152/168 ms. TUSB322I rejects FOLLOW_PORT_PIN and true disableRdRp.
All other declared enum values and independent disable combinations are valid
where the controls exist. No new role/preference/duty cross-field policy is added.

TUSB322I advertisement/duty/accessory values remain semantically valid even while
their writes are gated. Apply admission compares proved actual settings
and rejects gated changes; an unchanged gated field permits independent
supported changes. Validation neither observes those fields nor authorizes any
transition, including return to DRP. See the [register map](register-map.md).
The [validator suite](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/test_validators.cpp) exercises capabilities, all
invalid underlying bytes, input preservation and counting callbacks that stay zero.

## Status and error boundaries

`Status{}` has OK/0/empty-string; `Status::Ok()` has OK/0/"OK". `Error` preserves
its supplied code, static message pointer and int32 detail. It is a value factory,
not a validator or callback-status normalizer. `ok()` and explicit bool are true
only for OK; IN_PROGRESS is false and is queried with `inProgress()`.
`errorName` is fully defined constexpr in the header, returns the canonical name
for all 23 errors, and returns `UNKNOWN` for an unrecognized underlying byte.
Invalid underlying codes remain representable evidence, not accepted arguments.

The order below fixes Err values 0 through 22. For all fallible methods with
output arguments, rejected/failed calls preserve those arguments. An accepted
start assigns a token and returns IN_PROGRESS. Successful result delivery returns
OK even when the copied `OperationResult.status` is a failure. These output rules
do not roll back effects of previously attempted hardware writes.

| Err | Origin and output/effect boundary |
| --- | --- |
| OK | API success; cached/value work is bus-silent, explicit I2C methods follow their bounds. Result delivery success is distinct from operation success. |
| NOT_BOUND | Local missing binding; zero callbacks, output preserved. |
| NOT_INITIALIZED | Local missing current validated identity for a mutating action; zero callbacks. Read-only observation remains possible. |
| INVALID_CONFIG | Pure binding/profile/health-policy validation; zero callbacks and no replacement. |
| INVALID_PARAM | Bad method argument, raw/decoder enum or byte, register, interval or token; zero callbacks, output/state preserved. Config/profile validation uses INVALID_CONFIG for invalid field enums. |
| UNSUPPORTED_FEATURE | Valid request outside exact variant capability; reject before callbacks, preserve output. |
| BUSY | Active operation owns the slot; local rejection, not a transport failure. |
| IN_PROGRESS | Admitted or active operation; not terminal and never a legal callback status. |
| RESULT_PENDING | Retained result prevents replacement; zero callbacks, result preserved. |
| RESULT_NOT_AVAILABLE | No matching terminal result to deliver; output unchanged. |
| TOKEN_MISMATCH | Different represented token; zero callbacks and no advancement/cancellation/consumption. |
| TOKEN_EXHAUSTED | Nonzero uint64 token space exhausted; no reuse, admission or callback. |
| TIMEOUT | Owner deadline expired; no new callback after expiry, earlier effects remain. Not callback I2C_TIMEOUT. |
| CANCELLED | Local admitted-operation cancellation; no rollback/callback, retained result records effects. |
| CONFIGURATION_UNKNOWN | Required actual/effect proof absent; can be local rejection or result after successful reads. Never automatically an I2C failure. |
| CONFIGURATION_MISMATCH | Successfully observed persistent controls differ from desired/required postcondition. No transport fault; complete proof may still establish KNOWN actual settings. |
| CHIP_ID_MISMATCH | Complete successful signature/revision acquisition contradicts configured expectation; retain raw evidence, do not enable writes. |
| I2C_ERROR | Terminal callback failure, including normalized illegal callback codes; write acceptance can be uncertain. |
| I2C_NACK_ADDR | Backend positively identifies address NACK; only its phase guarantee can prove no effect for that attempt, never undo earlier writes. |
| I2C_NACK_DATA | Backend identifies data NACK; earlier data can already have affected hardware. |
| I2C_TIMEOUT | Callback cannot complete within its finite aggregate budget; counted callback attempt, not proof of physical bus traffic. |
| I2C_BUS | Backend-reported bus fault; recovery stays outside core, effects may be uncertain. |
| I2C_BUSY | Backend cannot acquire/use the bus within its contract; counted callback attempt, unlike local BUSY. |

Library-generated statuses use detail zero unless a later documented method
assigns another meaning. Backend detail is never narrowed or reinterpreted.
There is no detached or OFFLINE error: detach is normal, health is a projection.
Operation-status propagation, callback normalization and counters are implemented
for initialization, profile observation, apply, verification, reconciliation,
reset/base recovery, port refresh and IRQ acknowledgement/service. Public raw
diagnostic behavior is implemented in prompt 17.

## Borrowed transport and owner boundary

The exact implemented function-pointer types are:

```cpp
using I2cWriteFn = Status (*)(uint8_t address, const uint8_t* data,
                            size_t length, uint32_t timeoutMs, void* user);
using I2cWriteReadFn = Status (*)(uint8_t address, const uint8_t* txData,
                                size_t txLength, uint8_t* rxData,
                                size_t rxLength, uint32_t timeoutMs, void* user);
```

Address is seven-bit. One positive timeout covers lock acquisition and all bus
phases/cleanup together. Context is borrowed until end returns; buffers are
borrowed only until terminal callback return. No pending DMA, asynchronous
completion, reentry, core/adapter retry or bus recovery is permitted. The owner
serializes all calls and supplies clock, scheduler, pins, route, lock and power
policy. Null context is permitted when callbacks need none. The pinned backend's
own cleanup/bounds need adapter evidence; these pointer types cannot prove it.

Callbacks may return only OK or I2C_* errors. The transport wrapper normalizes any other
code to I2C_ERROR, preserve full detail and retain conservative write uncertainty.
Status carries no completion time: the next freshly sampled owner timestamp
anchors hardware waits and time-sensitive success. Owner intervals are strictly
between zero and 2^63 ms; the separate 1 ms hardware margin never extends the
immutable deadline. See the procedure guide for actual transfer ceilings.

## Implemented owner API

`<TUSB32x/TUSB32x.h>` delivers `TUSB32x::TUSB32x`, linked through target
`TUSB32x`. It is non-copyable and non-movable, uses fixed storage and borrows
the callbacks/context from a copied accepted DriverConfig. Construction and
destruction perform no hardware work. All calls require one serialized owner;
none is ISR-safe.

| Delivered methods | Behavior and bound |
| --- | --- |
| bind, end, invalidateDeviceState, forgetDeviceEvidence | Bus-silent atomic binding and evidence lifecycle. Invalid replacement preserves the old binding. End cancels active work into its retained result; it never rolls back hardware effects. Forget requires unbound, inactive, result-free state. |
| startInitialize | Copies timing and admits one nonzero uint64 instance-local token; IN_PROGRESS means admitted. Reads exact identity/revision and persistent controls only, with B5/L/A6/V5 callback ceilings. No profile adoption, reset, ACK or writes. |
| startReadProfile | Binding only; reads P without identity, desired adoption or trust promotion. Complete contradiction invalidates old current trust. B/L/A4 and V3 callback ceilings; partial failed acquisition preserves the prior complete observation. |
| startReconcile | Binding only; read-only F can establish initialization and KNOWN actual controls after invalidation. Retained reset evidence may require a 96-tick wait before the single F pass. B5/L/A6/V5 callback ceilings; no replay or cleanup writes. |
| startApplyProfile | Current identity, KNOWN actual controls and resolved transition proof required. Validate complete input/gates before copying desired and advancing its generation; changed fields only, followed by F. At most B10/L/A12/V8 callbacks; no-op F performs no writes. |
| startVerifyProfile | Retained desired input required; run read-only F and compare without adopting or replaying desired. Complete mismatching proof can establish KNOWN actual controls; B5/L/A6/V5 callback ceilings. |
| startReset | Current validated identity required; UNKNOWN actual configuration allowed when a fresh 0A read supplies a safe command image. Send one command, wait 96 elapsed owner ticks, then run F once; B7/L/A8/V7 callbacks. Never compare or replay desired. |
| startRecoverDeadBattery | Original 320/I only, current validated identity and caller-established stable VDD/readiness. Read 45, literal 04, wait 31 ticks, literal 00, read 45; at most 4 callbacks, 6 TX/2 RX. Own interrupted recovery permits a new explicit attempt; a different unresolved mode/reset transition rejects locally. |
| startRefreshPort | Binding only, including UNKNOWN configuration; read 09/08/09 once, at most 3 callbacks and 3 TX/3 RX bytes. Complete-only cached publication; no identity/configuration promotion, reset, ACK or write. |
| startAcknowledgeInterrupt | Current validated identity; UNKNOWN configuration allowed with a safe fresh actual09 image. B/L/A only; TUSB322I rejects before traffic. R09/W09: at most 2 callbacks, 3 TX/1 RX bytes. Explicit W1C only; no port refresh or recheck. |
| startServiceInterrupt | Same identity/variant gate as ACK-only. R09/R08/R09 publication, W09 ACK and one R09 recheck: at most 5 callbacks, 6 TX/4 RX bytes. Preserve pre-ACK publication if later work fails; no loop until clear. |
| poll | Exact token and owner uint64 time; zero budget means zero callbacks, any larger budget permits at most one terminal callback. Clips timeout to remaining deadline and yields after every callback for fresh owner time. |
| cancel | Exact active token becomes a retained CANCELLED result while delivery returns OK. Matching terminal cancellation returns OK without changing the result. No callbacks. |
| resultSnapshot, takeResult | Exact terminal token copies fixed evidence, returning delivery OK even if result.status failed. Snapshot repeats; take consumes once. Rejected delivery preserves caller output. |
| progress, configurationSnapshot, portSnapshot, transportStats, diagnostics, clearTransportStats | Fixed cached copies or clearing all-callback totals; no bus, clock sampling, retry or trust repair. Port retains historical data with explicit invalidation, age and field validity. Clearing totals does not clear managed evidence or retained results. |

The next successful admission cannot replace an active operation or retained
terminal result. Tokens never reset through end/bind/forget and never wrap.
Accepted reinitialization clears current initialized/identity usability before
reads; failure cannot retain the earlier initialized state. Historical complete
configuration remains distinguishable from current trust. A new target never
inherits old identity/configuration/port evidence. Retained association includes
variant, address, context and both callbacks, even with a null context.

After an OK callback, the next valid poll supplies fresh time before another
callback or successful completion. Equal ticks are valid and establish no
elapsed wait. Backward or half-range-ambiguous time is rejected unchanged.
A late pending success becomes owner TIMEOUT; an actual callback failure and
its full detail remain primary even if a later timestamp is late. Starts,
lifecycle/cache/result methods and cancellation are bus-silent. See the exact
precedence/lifetime/time tables below and the repository-only
[validation record](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md).

Profile observation uses P = R(08,1), R(09,1), R(0A,1), then R(45,1) on
B/L/A only. Its callback/TX/RX ceilings are 4/4/4 for B/L/A and 3/3/3 for V.
F reads identity first: R(00,8), R(A0,1) on L/A/V only, then P. Reconciliation
therefore has ceilings 5/5/12 for B, 6/6/13 for L/A and 5/5/12 for V.
Both operations have `HardwareEffect::NONE`. A complete profile read returns OK
even when its valid controls contradict old verified settings; it invalidates
that old trust and never promotes the replacement. Complete reserved encodings
retain raw validity, clear the affected typed flags and return
CONFIGURATION_UNKNOWN. A failed or cancelled partial acquisition does not publish
a mixed observation.

Reconciliation establishes KNOWN only with matching identity, every applicable
persistent field and required termination/reset/sequence proof. KNOWN describes
actual controls independently of desired settings. It increments the saturating
verified generation on each complete proof commit and never adopts desired.
Complete raw acquisition may establish initialized even when reserved controls
or unresolved history prevent KNOWN. A clear reset bit cannot replace a required
settling interval or prove a historical recovery sequence. If reset is already
pending without an owner-time anchor, the first fresh poll starts a 96-tick wait
before F; reset first discovered in F finishes CONFIGURATION_UNKNOWN without
a second pass. Neither operation repairs port invalidation.

`startApplyProfile` retains its whole validated candidate and advances the
saturating desired generation once at admission. Rejected input, gated changes
and failed admission preserve desired and outputs. Admission, cancellation or
expiry before the first mutating callback preserves still-proved actual and port
evidence. Unknown actual controls require read-only observation/reconciliation
before applying; a valid candidate alone is insufficient proof for safe writes.
The entire bounded plan is composed and checked before desired input changes.

Only differing persistent fields are written. Base MODE_SELECT changes read
attachment before any writes and reject attached state; unchanged mode with a
current-only change is allowed. Cable attachment can race that precheck, so the
application still owns electrical/power policy. Extended 320 mode changes use
separate disable/mode/enable steps while preserving non-mode controls. The TI
5 ms wait becomes six elapsed owner ticks after fresh completion time; a mode
write completion sample of 108 enables no earlier than 114, or 116 when the
first fresh sample is 110. The separate one-tick margin never extends deadline.
Requested final disableTerm/disableRdRp settings remain effective and distinct.

TUSB322I DFP/UFP uses its own disable/mode/enable procedure without adding a
5 ms wait. Actual advertisement changes, every 09 write and a direct mode
change to DRP are rejected before traffic. Unchanged proved gated controls and
semantic DRP00/11 equivalence permit independently supported changes. Role changes
can have autonomous VCONN effects; board power and USB policy stay with the owner.

Before any possible port-affecting write, current configuration and port usability
are invalidated, retaining historical values. Effect evidence is set before the
callback because error return does not prove that the data was rejected. The
first backend error/detail survives; no retry, rollback or cleanup follows
failure, cancellation, expiry or end. No-op apply runs full F with effect NONE;
a completed changed-image procedure needs final proof for CONFIRMED.

The maximum apply callback/TX/RX buffer-byte ceilings are 10/14/13 for B,
12/18/13 for L/A and 8/11/12 for V. Requested final disableTerm reduces a mode
change ceiling to 11/16/13 on L/A or 7/9/12 on V by omitting the final enable.
These are worst-case changed-field plans, bounded by six writes; unchanged
fields reduce work. F-only no-op/verify costs 5/5/12 on B, 6/6/13 on L/A and
5/5/12 on V. Waits add no callbacks. Byte counts are requested buffer lengths,
including pointers, excluding address/ACK framing; they are not physical traffic.

Apply and `startVerifyProfile` share F identity/control/effect proof. Verify needs
retained desired input and has no writes. Complete proof of a different actual
image publishes KNOWN actual settings and CONFIGURATION_MISMATCH while keeping
desired unchanged. Unresolved transition/reset history keeps trust UNKNOWN even
when some bytes match. After an interrupted apply, read-only reconciliation can
wait and prove a different actual image, allowing a new explicit apply. It never
claims the earlier operation succeeded or discards its retained effect evidence.
The [procedure guide](device-behavior.md#fixed-observations-and-shared-proof) documents
the proof and independent termination controls.

`startReset` qualifies actual 0A persistent controls with one fresh read and
sets command bit 08 in their safe composed image. It never sends literal 01
under mask 08, substitutes reset defaults for actual controls or resends a
possibly accepted command. Successful callback acceptance starts no clock inside
core: the next fresh poll anchors 96 elapsed ticks for TI's 95 ms maximum plus
the separate one-tick clock margin. Then one complete F pass proves clear reset
and applicable postconditions. Callback/TX/RX ceilings are B7/8/13, L/A8/9/14,
V7/8/13, including the pre-read, command and F. No wait turn invokes transport.

Reset affects digital logic, can resample PORT on the six 320 parts and returns
TUSB322I to DRP. It is not POR or attachment-ready proof, and does not promise a
whole-byte reset image or restore desired settings. TUSB322I accepts either
documented semantic DRP code in final proof; a complete non-DRP image returns
CONFIGURATION_MISMATCH, retains raw observation and leaves reset MAY_BE_PENDING
with configuration/termination UNKNOWN. VCONN can change with source-role/attachment behavior, subject to
the board's supply prerequisite. Historical desired/observed/verified values
remain separate from present trust. An older unresolved recovery can keep
configuration UNKNOWN even when this explicit reset succeeds.

`startRecoverDeadBattery` requires original TUSB320/TUSB320I identity and the
owner's decision that stable-VDD startup needs Rp recovery. Missing VDD with no
cable is also a documented dead-battery case; the core has no battery monitor.
It reads 45 and fails CONFIGURATION_UNKNOWN before writing when internal bits 03
are nonzero, then writes literal 04, waits 31 elapsed owner ticks from fresh
post-write time, writes literal 00 and reads 45 once. Final `(value & 0x07)==0` is
required; mismatch returns CONFIGURATION_MISMATCH. Four callbacks, 6 TX/2 RX is the
whole procedure ceiling. A final 04 can establish termination DISABLED while
the failed sequence remains unresolved; nonzero internal bits keep termination
UNKNOWN. The internal-zero qualification and final readback are
library policy beyond TI's recipe. Other variants reject before transport.

Recovery invalidates current configuration/port usability before a possible
write and publishes no transient port observation between writes. Its own
successful sequence can have CONFIRMED effect while configuration remains UNKNOWN
until full F. Cancellation, expiry, end and failed callbacks never undo effects
or trigger cleanup. The first error/detail, reset acceptance uncertainty and
independent recovery obligation survive result take and same-target rebind.
Later clear 45 can describe present controls but cannot prove the historical
writes/wait; reconciliation observes once and never replays them. Only a new
explicit recovery request can attempt that sequence again. Exact reset/retention
and recovery citations are in the [procedure guide](device-behavior.md).

## Complete design inventory

This inventory summarizes the delivered API. The compile-only fixture checks
these real public declarations without defining a second facade. Profile observation and
read-only reconciliation are delivered in prompt 12; application and desired-profile
verification are delivered in prompt 13; reset and recovery in 14; port refresh
and publication in 15; explicit IRQ acknowledgement/service in 16. Raw access
and health helpers are delivered in 17.
No begin/init/unbind aliases or successful placeholder methods are provided.

```cpp
namespace TUSB32x {
Status validateHealthPolicy(const HealthPolicy& policy);
Status projectHealth(const ManagedTransportEvidence& evidence,
                     const HealthPolicy& policy, HealthSnapshot& out);
Status decodeRegister(Variant variant, uint8_t reg, uint8_t value, RegisterDecode& out);

class TUSB32x {
 public:
  TUSB32x();
  ~TUSB32x();
  TUSB32x(const TUSB32x&) = delete;
  TUSB32x& operator=(const TUSB32x&) = delete;
  TUSB32x(TUSB32x&&) = delete;
  TUSB32x& operator=(TUSB32x&&) = delete;
  Status bind(const DriverConfig& config);
  Status end();
  Status invalidateDeviceState(const Status& reason);
  Status forgetDeviceEvidence();
  Status startInitialize(const OperationTiming& timing, OperationToken& token);
  Status startReadProfile(const OperationTiming& timing, OperationToken& token);
  Status startApplyProfile(const DeviceProfile& profile,
                          const OperationTiming& timing, OperationToken& token);
  Status startVerifyProfile(const OperationTiming& timing, OperationToken& token);
  Status startReconcile(const OperationTiming& timing, OperationToken& token);
  Status startRefreshPort(const OperationTiming& timing, OperationToken& token);
  Status startAcknowledgeInterrupt(const OperationTiming& timing, OperationToken& token);
  Status startServiceInterrupt(const OperationTiming& timing, OperationToken& token);
  Status startReset(const OperationTiming& timing, OperationToken& token);
  Status startRecoverDeadBattery(const OperationTiming& timing, OperationToken& token);
  Status poll(OperationToken token, uint64_t nowMs, uint8_t maxTransfers = 1);
  Status cancel(OperationToken token);
  Status resultSnapshot(OperationToken token, OperationResult& out) const;
  Status takeResult(OperationToken token, OperationResult& out);
  OperationProgress progress() const;
  PortSnapshot portSnapshot() const;
  ConfigurationSnapshot configurationSnapshot() const;
  TransportStats transportStats() const;
  DriverDiagnostics diagnostics() const;
  void clearTransportStats();
  Status readRegisterSync(uint8_t reg, uint8_t& out);
  Status writeRegisterSync(uint8_t reg, uint8_t value);
};
}  // namespace TUSB32x
```

Validation, binding, starts, cancellation, end/invalidation/forgetting and cached
delivery are bus-silent. Only poll advances managed I2C; the two explicit Sync
methods have a one-callback limit and cannot bypass silicon procedures. Admission
retains desired input separately; initialization observes only, reconciliation
does not replay, and reset never reapplies a desired profile. The tables below
finish the baseline's precedence, result retention and interruption rules.

Pure binding/profile validators are implemented in prompt 7, decoding in 10,
and the delivered owner subset, including IRQ acknowledgement/service, in 11–16.
Raw access and health helpers remain prompt 17 work.
No unavailable function may appear in a runnable quick start before that work.
The health validator requires offlineThreshold >= 1. Invalid policy returns
INVALID_CONFIG, and projectHealth preserves out on that failure; otherwise it
copies the supplied managed evidence and effective threshold into its projection.
It performs no I2C, retry, clock read or evidence repair.

## Pure register decoder boundary

`decodeRegister(Variant, uint8_t reg, uint8_t value, RegisterDecode&)` is implemented
and linked through target `TUSB32x`; its result and declaration are in CommandTable.h.
It interprets one caller-supplied byte using the exact
[field tables](register-map.md#explicit-field-encodings).
It performs zero callbacks, changes no instance/counter/cache and requires no
binding or constructed driver. Work is a fixed register/variant branch, no loops
over hardware, allocation or retry. CLI prompt 22 formats this same result.

Validation order: invalid/UNSPECIFIED Variant returns INVALID_PARAM, then an
unmapped register returns INVALID_PARAM, then an absent documented family
register (base A0 or 322I 45) returns UNSUPPORTED_FEATURE. Every error preserves
the complete output. Successful decode replaces the output with a default-empty
result populated only by this byte, preserves variant/reg/value and returns OK.
Reserved field codes are successful raw evidence with that field's validity clear;
they do not cause an I2C error or a plausible valid default.

| Input register | Populated interpretation and validity |
| --- | --- |
| 00..07, present A0 | Raw byte only; one signature byte cannot prove identity and a revision byte cannot establish model support |
| 08 | profileCodes.sourceCurrent with SOURCE_CURRENT validity if legal; detectedCurrent, accessory and activeCable with corresponding status-field code validity, variant-aware reserved codes clear flags |
| 09 | ATTACHMENT, ORIENTATION and INTERRUPT_PENDING field codes; duty/UFP-accessory profile fields where documented; 322I fault bit stays raw, no invented consuming/clear interpretation |
| 0A | Debounce/mode/preference/disableTerm profile field codes where present; resetRequested and resetValid report command bit only, never completion |
| Present 45 | disableRdRp profile field code; TI-internal/reserved bits remain raw and never enter DeviceProfile |

`profileValidFields` uses ProfileField bits. `statusValidFields` uses PortField
bits but never LOCAL_ROLE, PEER_ROLE or ADVERTISED_CURRENT: the first two need
port context and advertisement is already represented by SOURCE_CURRENT.
0x09 direction decodes as CC1/CC2 even when its attachment code is detached:
this is a register field code, not usable cable orientation. Likewise decoded
current/accessory/active-cable codes do not establish applicability. No fabricated
0x09 byte, assumed detach or single-byte PortSnapshot enters this API.

The complete port decoder separately applies attachment context and can publish
valid NOT_APPLICABLE interpretations. Unknown/reserved profile values retain
default storage with a clear flag; raw value remains authoritative evidence.
Readable 322I fields may decode while their writes remain gated. Decode never
authorizes a write or removes an unresolved silicon/board prerequisite.

## Method admission and output preservation

These tables are implemented for the delivered owner and raw methods and are
exercised by the registered suites. The
[current peer review](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-11-peers.md) records the implementations
and tests compared. The baseline's precedence is unchanged. Evaluate the
following rows in order; stop at the first applicable rejection. Rejections
preserve all caller outputs and instance state, including token allocation,
desired settings, progress and counters. No rejected admission invokes a callback.

| Entry group | Ordered checks before work |
| --- | --- |
| Pure validators | DriverConfig: callback presence, exact variant, matching address, positive timeout; each bad binding returns INVALID_CONFIG. Profile: exact non-UNSPECIFIED variant and defined field enums first (INVALID_CONFIG), then unsupported semantic combinations (UNSUPPORTED_FEATURE). Health: threshold zero is INVALID_CONFIG. |
| All starts | Argument validity first: timing interval then profile enum validity if supplied; pure variant capabilities are checked once a valid binding supplies the variant. Then NOT_BOUND, BUSY, RESULT_PENDING. After these: missing identity where required (NOT_INITIALIZED), missing desired/actual/effect proof (CONFIGURATION_UNKNOWN), changes to gated fields (UNSUPPORTED_FEATURE), exhausted token space (TOKEN_EXHAUSTED). |
| bind | Validate the entire proposed config first; BUSY, RESULT_PENDING; then different target with unresolved effects gives CONFIGURATION_UNKNOWN. No NOT_BOUND requirement. Commit replacement only after every check passes. |
| Raw Sync | Validate register existence/access and supplied byte encoding first; variant-specific absent register or gated write is UNSUPPORTED_FEATURE when a binding supplies the variant. Then NOT_BOUND, BUSY, RESULT_PENDING; writes then require identity and safe actual controls. Unsafe sequence shortcut gives UNSUPPORTED_FEATURE; missing actual proof gives CONFIGURATION_UNKNOWN. |
| Tokened methods | Zero token: INVALID_PARAM; no represented work/result: RESULT_NOT_AVAILABLE; different represented token: TOKEN_MISMATCH. Only then apply the active/terminal table below. Do not require a binding to inspect/cancel/take represented work. |
| invalidateDeviceState | Unrecognized reason code or IN_PROGRESS: INVALID_PARAM without changes. OK is accepted and normalized to CONFIGURATION_UNKNOWN with static message and detail zero. Other defined terminal codes are accepted notifications. No binding/slot prerequisite. |
| forgetDeviceEvidence | BUSY if active, RESULT_PENDING if terminal, INVALID_CONFIG if still bound; otherwise clear target evidence locally. These are method-specific guards, not the start admission order. |

Raw unmapped registers and illegal/reserved byte encodings return INVALID_PARAM;
read-only-register writes and intrinsically forbidden commands/procedures return
UNSUPPORTED_FEATURE. These intrinsic checks precede binding/slot/identity checks
where their inputs suffice. Variant-specific meanings need the bound variant.
A change that is forbidden only in comparison with actual controls is checked
after current identity and actual-image proof. Apply's invalid profile enum
returns INVALID_CONFIG, as validateProfile does; invalid timing is INVALID_PARAM
and is checked first. These distinctions define multi-error cases unambiguously.

Before binding, variant-dependent checks cannot guess a variant. Check only
variant-independent argument errors, then NOT_BOUND. With a binding, pure
capability checks precede slot checks; comparisons requiring a known actual image
follow the slot and identity/proof checks. For Apply, validate the profile's
fields in declaration order. A valid unchanged gated field is not a requested
write: e.g. proved unchanged 322I advertisement does not reject other supported
changes. No missing field is treated as a default or as unchanged. Known but
unsupported profile combinations are separate from invalid enum values.

An accepted start copies its inputs, allocates the next nonzero uint64 token and
returns IN_PROGRESS. UINT64_MAX may be issued once; all later admissions fail
TOKEN_EXHAUSTED. Never wrap, reset or recycle the counter through end, bind or
forget. Other failures do not consume a token. Tokens belong to an instance,
not a bus or process; owners pair them with that instance.

| Represented work and matching token | poll | cancel | resultSnapshot / takeResult |
| --- | --- | --- | --- |
| Active | Validate time, then bounded advancement; IN_PROGRESS or terminal status | Return OK; freeze CANCELLED result with retained effects and unknown completion time | RESULT_NOT_AVAILABLE; out unchanged |
| Terminal | Return frozen result.status; ignore nowMs/budget, no state change or traffic | OK; preserve the entire frozen result | OK and copy, even if result.status is failure; only take consumes |
| After exact take | RESULT_NOT_AVAILABLE | RESULT_NOT_AVAILABLE | RESULT_NOT_AVAILABLE; out unchanged |

After consumption progress is default IDLE with no represented token; diagnostic
history, desired/observed/verified records and unresolved effects remain. A stale
token encountering new represented work gives TOKEN_MISMATCH even if the new
operation has the same kind. A result is immutable after publication, including
its copied snapshots, times and effect. Later end/invalidation changes live
evidence, not the old result.

### Complete method responsibilities

All validators, lifecycle methods, starts and cached/result methods are
bus-silent. Only poll runs managed callbacks. The numbers below bound an entire
operation, not admission. B means 320/I; L/A means LI/HI/LAI/HAI; V means 322I.
The [procedure ceilings](device-behavior.md#callback-and-requested-byte-ceilings)
also fix TX/RX bytes and variant-specific paths; never add retries to these limits.
I, P, F and S name the identity, profile, full proof and port plans in that guide.

| Method | Prerequisites beyond the common checks; UNKNOWN allowed | Result/effect and maximum work |
| --- | --- | --- |
| validateDriverConfig | No instance | Pure fixed-field validation; OK or INVALID_CONFIG, no replacement |
| validateProfile | Explicit exact variant; no actual image needed | Pure field/capability validation; does not authorize a change or board power |
| validateHealthPolicy / projectHealth | Positive threshold; arbitrary copied evidence allowed | Pure validation/projection; invalid policy preserves out; success replaces out |
| decodeRegister | Exact variant and mapped byte register | Pure one-byte decode as specified above; never changes instance evidence |
| Constructor / destructor | Owner serializes lifetime | Fixed empty values / local teardown, zero callbacks; take evidence before destruction if needed |
| bind | Valid borrowed callbacks and owner context | Replace binding atomically; current initialization/usability reset, retained association rules below |
| end | Any lifecycle, including unbound/terminal | Local cancellation if active, unbind, invalidate current usability; OK, no hardware cleanup |
| invalidateDeviceState | Valid reason, any lifecycle | Local cancellation if active; binding retained, identity/config/port invalidated; OK |
| forgetDeviceEvidence | Unbound, inactive, result-free | Discard target records/obligations; OK, no hardware recovery or token reset |
| startInitialize | Binding only; UNKNOWN permitted | F, B5/L/A6/V5 callbacks, no writes; clears current initialization on admission; never adopts desired |
| startReadProfile | Binding only; UNKNOWN permitted | P, B/L/A4/V3 reads; publish observed settings, never promote trust or adopt desired |
| startApplyProfile | Current identity, KNOWN actual controls, resolved transition/recovery proof; no UNKNOWN | Retain desired at admission; changed-field plan and F; at most B10/L/A12/V8 callbacks, no-op F has no writes |
| startVerifyProfile | Retained desired; UNKNOWN allowed, no current identity prerequisite | F and semantic desired comparison; B5/L/A6/V5, no writes; proved actual may be KNOWN on mismatch |
| startReconcile | Binding only; UNKNOWN permitted | Bounded optional prior-effect wait then F once; B5/L/A6/V5, no writes/replay; unresolved proof returns CONFIGURATION_UNKNOWN |
| startRefreshPort | Binding only; UNKNOWN permitted | S once, 3 reads; no identity/config promotion, ACK or implicit reset |
| startAcknowledgeInterrupt | Current identity; UNKNOWN permitted if fresh R09 yields safe controls; B/L/A only | R09 + explicit ACK, 2 callbacks; no port refresh or recheck; preserve actual controls |
| startServiceInterrupt | Same identity/variant gate; UNKNOWN permitted | S publication + ACK + one recheck, 5 callbacks; no loop until IRQ clears |
| startReset | Current identity; UNKNOWN permitted if pre-read supplies safe 0A | One command, wait, F; B7/L/A8/V7; no desired comparison/reapply, no repeat reset |
| startRecoverDeadBattery | Base only, current identity, caller stable VDD/readiness; own old recovery uncertainty allowed, different pending mode/reset rejected | R45,04,wait,00,R45: 4 callbacks, 6 TX/2 RX; no intermediate port publication or desired replay |
| poll | Exact active token and valid owner time; UNKNOWN per admitted operation | Zero or one callback with basic transport; bounded local stages below; no new operation/deadline |
| cancel | Exact represented token | Local OK; active result becomes CANCELLED, with independent write effects; no rollback |
| resultSnapshot / takeResult | Exact terminal token | Fixed copy, then optional consume; failed delivery preserves out; no evidence repair |
| progress | Any lifecycle | Fixed current operation/terminal-slot copy; no time sampling |
| portSnapshot / configurationSnapshot | Any lifecycle | Fixed cached copy; field validity, current trust and age remain distinct |
| transportStats / diagnostics | Any lifecycle | Fixed history/live-evidence copy; no poll or policy action |
| clearTransportStats | Any lifecycle, including active | Clear only all-callback totals and lastStatus; no operation or managed-evidence change |
| readRegisterSync | Valid binding; UNKNOWN allowed; documented mapped scalar | At most one callback; successful byte delivery replaces out, failure preserves out; no profile/identity/port publication or trust promotion |
| writeRegisterSync | Current identity and proved safe actual image; UNKNOWN cannot supply it | At most one callback, no hidden read; deliberate mixed-access composition, invalidate affected trust before possible write; no multi-step bypass |

Raw writes use the [qualified access rules](register-map.md) and procedure guide.
They cannot reset, change a sequenced role, perform recovery or invent termination
proof. No generic raw RMW exists. A successful raw read that reveals reset bit 1
still sets MAY_BE_PENDING and invalidates affected configuration/port evidence;
delivery returns OK but does not establish a wait anchor. Merely observing that
command does not revoke already validated identity. Raw writes are never a
substitute for typed apply or a way to promote KNOWN. Scalar raw identity/revision
reads, including A0, are diagnostic bytes, not the managed I proof; they neither
publish identity nor change initialized. An owner interpreting unexpected raw
bytes as replacement/external state change must explicitly invalidateDeviceState.
Managed complete I contradictions do revoke readiness. Thus raw traffic remains
excluded from managed health, while raw reset/control safety evidence can still
invalidate configuration/port trust independently.

## Binding and evidence lifetime

One application owner serializes all calls, including cached access and external
invalidation. Methods are not ISR-safe and callbacks cannot reenter. Assign one
configuration owner per physical target. Different devices may share the bus;
serialization alone does not make two independent writers to one device safe.

| Object or event | Ownership and retained state |
| --- | --- |
| DriverConfig / DeviceProfile / OperationTiming | Copy on accepted bind/start; caller storage may change afterward; deadline is immutable |
| Context and callbacks | Borrowed, valid through end return; retain only their association afterward, never dereference/invoke the old bus |
| TX/RX buffers | Fixed driver storage borrowed only until terminal callback returns; no pending DMA or deferred use |
| Token/result/snapshot | Owner pairs token with instance; result stays in one slot until exact take; returned copies never change underneath the caller |
| Status.msg | Static lifetime, including externally supplied reasons; no temporary formatting buffer |
| First binding | Empty target evidence, UNCONFIGURED, initialized=false; no traffic |
| Same association rebind | Clear current initialized/usability, set existing configuration proof UNKNOWN, invalidate port; retain desired/history and every unresolved obligation |
| Different association, no unresolved evidence | Clear all target records, including desired/observed/verified/raw identity/IRQ/config/port; initialize a new empty target, retain instance history |
| Different association with unresolved evidence | CONFIGURATION_UNKNOWN and leave old binding/evidence unchanged; caller must end, take any result, then explicitly forget before replacement |
| end | Active work freezes CANCELLED; terminal result unchanged; unbind, initialized=false, port.invalidated=true, established configuration becomes UNKNOWN; retain effects/history |
| invalidateDeviceState | Keep binding; clear current initialization, invalidate configuration/port; preserve historical bytes/generations and unresolved effects |
| forgetDeviceEvidence | Clear target records and obligations only after its guards; retain token sequence, transport counters and instance diagnostic history |

Association is exactly `(variant, address, i2cUser, i2cWrite, i2cWriteRead)`;
timeout changes alone do not change target. Context reuse, null context and equal
callback pointers do not prove physical identity. The owner reports replacement
or external writes explicitly. The different-target gate checks the four named
unresolved diagnostic flags and resetState==MAY_BE_PENDING; it does not infer
history from the latest result's effect or UNKNOWN alone. An empty UNCONFIGURED
object alone is not an interrupted effect. Historical ambiguity
about reset acceptance can remain after settled current proof and is retained
until explicit forgetting; it must never be silently assigned to another target.
Historical uncertainty about a pure ACK is excluded from this target-change
guard: it changes no persistent controls and leaves no mode/reset/recovery
obligation. Keep that uncertainty in its IRQ/result record, without inventing
an outstanding latch-clear task or preventing a later target binding.

External invalidation is a notification, not a transport callback. For active
work it freezes state/status CANCELLED, using the normalized reason's static
message/detail as explanation. Return API OK. The reason code remains the
caller's cause record, not a new transport failure. Without active work, retain
the previous last-operation record; do not invent an operation to store a reason.
An existing terminal result is unchanged. Neither invalidation nor end invents a
completion time from the last poll. Both are idempotent for already invalid
live evidence; only explicit forget discards the retained target record.
For active end/invalidation, invalidate live evidence before freezing the new
CANCELLED result so its copies show that invalidation. Plain cancel does not
invalidate otherwise proved actual state. An already frozen result is never
rewritten by any of these actions.

## Independent state and effect tables

### Lifecycle and operation

| Event | Binding / initialized | Operation/result |
| --- | --- | --- |
| Construct | false / false | IDLE, no token/result |
| Accepted bind | true / false | IDLE (slot guards already passed) |
| Accepted start | Unchanged except initialize clears initialized | ACTIVE; new token, kind, immutable timing, phase, zero transfers |
| Callback OK | Transport success; no automatic initialization | Stage completion for a fresh poll; ACTIVE |
| Callback failure | Keep identity unless contradicted; retain effects | Immediate FAILED with original normalized transport error; time invalid |
| Complete required read-only initialization evidence | true / true, independent of config trust | May still end FAILED/CONFIGURATION_UNKNOWN for unresolved recovery or reserved controls |
| Terminal OK / protocol failure / expiry | Independent of operation status | SUCCEEDED / FAILED / TIMED_OUT; retain one result |
| Local cancel / end / invalidation | Lifecycle as above | Active becomes CANCELLED; terminal remains frozen |
| Exact take | No identity/trust changes | IDLE, slot released; tokens never reused |

Initialization requires complete matching I and complete successful raw P for
the current association. A reserved control or unresolved effect can prevent
KNOWN without preventing this identity readiness. Thus end/rebind, initialize
returning CONFIGURATION_UNKNOWN, take, explicit base recovery is possible; there
is no identity/recovery deadlock. A failed/incomplete I/P or identity mismatch
cannot establish initialization. Full F in verify/reconcile/reset/apply uses the
same identity readiness helper. Initialize admission first clears current
initialized and any existing KNOWN proof, retaining history. Ordinary callback
failures do not erase a previously validated identity; external invalidation or
a complete contradictory identity does. `managedTransport.initialized` always
matches `diagnostics.initialized`, not `lastOperationStatus.ok()`.

### Configuration and publication

| Event | Desired | Observed / verified / current state |
| --- | --- | --- |
| Empty target | Absent, generation0 | Absent, UNCONFIGURED |
| Accepted Apply | Copy complete candidate; saturating desiredGeneration increment | No actual change yet; old proof/port preserved |
| Pre-write cancel/deadline/rejection | Accepted desired remains | Keep old proved actual state; rejected admission changes nothing |
| First possible Apply write | Unchanged | APPLYING; port invalidated before invocation; interrupted work leaves UNKNOWN |
| Other possible configuration/reset/recovery write | Unchanged | Invalidate affected proof, UNKNOWN; old raw/verified records retained |
| Complete P | Unchanged | Replace raw/decoded observed image and acquisition time; hasObservedProfile only if all applicable fields decode; no trust promotion |
| Partial/failed P | Unchanged | Previous complete image/time unchanged; independently discovered adverse evidence still invalidates trust |
| Complete P contradicts verified controls or contains applicable reserved value | Unchanged | UNKNOWN, affected port invalidated, verifiedProfile remains historical |
| Complete F, all required effects settled and applicable controls valid | Unchanged except earlier Apply admission | KNOWN; commit proved actual verifiedProfile, saturating verifiedGeneration increment, even when desired comparison mismatches |
| F incomplete or transient proof unresolved | Unchanged | No promotion; previously UNKNOWN stays UNKNOWN. Preserve old proof when no initialize invalidation, possible write, contradiction or unresolved effect has invalidated it; failed acquisition alone is not adverse evidence |
| End/reinitialize/external invalidation | Retained | Existing target proof UNKNOWN; empty target may remain UNCONFIGURED |

F is the shared [reconciliation proof](device-behavior.md#fixed-observations-and-shared-proof),
never a replay. Compare only persistent writable semantics, including 322I
DRP00/DRP11 equivalence; exclude live status, W1C, reset commands and internal 45
bits. Verify and reconcile compare retained desired when present; initialize and
reset do not treat old desired as their success postcondition. Verify/reconcile
with complete proof and a desired difference return CONFIGURATION_MISMATCH but publish
KNOWN actual; later apply uses it without requiring the interrupted desired to
match. Missing effect proof returns CONFIGURATION_UNKNOWN before any desired
mismatch claim. Recovery and reset retain their separate operation postconditions.

Observed raw bytes and their decoded flags publish together only after the whole
P succeeds. Use owner time immediately before P's first required read for
observedUptimeMs, not the end of F. Mismatch records come from a complete valid
comparison; choose the first differing register in ascending address order,
with its persistent comparison mask and observed/expected bytes. A completed
matching comparison clears mismatchValid; new desired admission also clears it
so an old comparison cannot appear to describe the new candidate. No comparison
means no new mismatch record. Absent/reserved fields never acquire validity
from placeholder values. There is no invented observed-generation counter:
observed bytes/time, desiredGeneration and verifiedGeneration serve different
purposes. Both generations saturate at UINT32_MAX; zero means no publication,
and saturated values cannot uniquely correlate work.

### Termination, reset and retained obligations

For explicit 322I reset, an asserted reset bit or incomplete profile interpretation
returns CONFIGURATION_UNKNOWN before a role-mismatch status. Independently valid
non-DRP mode evidence still prevents COMPLETED even if another field is reserved.
Complete valid non-DRP proof with a clear reset bit returns CONFIGURATION_MISMATCH.

| Evidence/event | Required fact |
| --- | --- |
| All applicable disable controls clear, settled reset/mode proof | Termination ENABLED; not an electrical measurement |
| A controlling disable proved effective, no contradictory transient uncertainty | DISABLED; clear0A alone cannot override set45 |
| Missing/ambiguous control or transient evidence | UNKNOWN; retain individual raw controls |
| Possible settings write, including raw diagnostic write | Set configurationWriteUnresolved before invocation; retain through later read-only results/end/rebind. Complete current-control/effect proof clears it, including KNOWN with desired mismatch |
| Asserted reset bit from any successful read or attempted reset command | MAY_BE_PENDING, config/port invalidated immediately |
| Required reset settling plus complete clear-bit/postcondition proof | COMPLETED; does not prove ambiguous command acceptance |
| Mode sequence interrupted after possible write | modeTransitionUnresolved=true until bounded observation/settling proves current controls; not proof old request succeeded |
| Base recovery interrupted after possible04 | recoverySequenceUnresolved=true; zero45, reset, take or arbitrary elapsed time cannot clear historical sequence obligation |
| Explicit complete base recovery | Clear its recovery obligation; OK/CONFIRMED for this sequence, configuration still UNKNOWN until F |
| Reset possibly accepted with callback failure | resetAcceptanceUncertain=true; later settled proof can establish current state without claiming the old command ran |

Keep these facts independent. A new successful reset can have CONFIRMED effect
while an older recovery obligation keeps configuration UNKNOWN. Read-only
reconcile can prove KNOWN actual without changing the old failed result or
erasing historical reset-acceptance uncertainty. No result take, statistics
clear, successful unrelated read, end or same-target bind erases obligations.

| This operation's write evidence | HardwareEffect |
| --- | --- |
| No data-changing write could have reached target | NONE, including all read-only operations and pre-write cancellation |
| At least one confirmed write but final requested postconditions incomplete | PARTIAL |
| Any attempted write has unresolved acceptance | INDETERMINATE, even with earlier confirmed writes |
| All intended writes and required final postconditions proved | CONFIRMED |

Mark possible effect before invoking a write callback, after local budget guards.
Only an actual backend phase guarantee can prove an address-NACK attempt had no
effect; no generic error guesses that phase. ACK acceptance is the ACK operation's
postcondition, not proof the latch remained clear. Software CANCELLED with
INDETERMINATE, and READY health with UNKNOWN configuration, are valid pairs.
If an authoritative backend guarantee proves a settings-write attempt had no
effect, preserve the earlier configurationWriteUnresolved value; do not clear
an older obligation or introduce one for that disproved attempt. Pure ACK that
preserves actual controls does not set this settings flag. Reset/mode/recovery
retain their own proof flags; any settings they may change also needs complete
current proof. Explicit forgetting clears all target obligations without proving
hardware recovery. ConfigurationWrite uncertainty never changes a frozen result.

`diagnostics.hardwareEffect` copies the latest managed terminal result or later
raw write's effect. A successful ordinary raw write is PARTIAL until its separate
postconditions are proved; an ambiguously failed raw write is INDETERMINATE.
A qualified raw ACK can confirm command acceptance, never latch-clear state.
Raw reads leave this field unchanged. Raw writes do not invent lastOperationKind
or lastOperationStatus: those still describe managed terminal history. Complete
F may clear the ordinary settings obligation without rewriting historical
write acceptance. The named flags, not this latest-effect summary, control
unresolved-evidence admission.

## Owner time and bounded work

Let `H = uint64_t{1} << 63`; all subtraction/addition below is unsigned modulo
2^64. A valid admission interval is `0 < deadline - now < H`. For an active
operation, `now - lastAccepted < H` admits forward time, equal ticks and natural
rollover; `>= H` returns INVALID_PARAM with no state change. Terminal token access
does not validate or reinterpret its old time. Use one domain across operations
and bindings, and sample often enough to avoid half-range ambiguity.

| Time boundary | Rule |
| --- | --- |
| Active valid poll before deadline | `now - deadline >= H`; positive remaining is `deadline - now` |
| At/after deadline | `now - deadline < H`; finish TIMEOUT without a new callback |
| Equal tick | Valid fresh sample if sampled again after callback; elapsed waits do not grow |
| Backward step / exactly half-range delta | INVALID_PARAM; no counters, lastPollTransfers, result or evidence changes |
| Stalled clock | No fabricated elapsed time or busy loop; owner must advance its clock or explicitly cancel |
| Callback cap | Compare uint64 remaining to configured uint32 cap, then narrow the minimum; never call with zero |
| Required wait | Arm at next fresh post-callback poll; readyAt is wrapping addition of 6/31/96 ticks for 5/30/95 ms respectively |
| Zero maxTransfers | Validate time, enforce deadline, process pending completion, arm/expire waits; no callback |
| maxTransfers greater than one | Still at most one callback with these Status-only types; stop immediately after it |

After an OK callback, retain its bytes/acceptance and return IN_PROGRESS. On the
next valid fresh poll, check deadline first, then process successful completion,
publish complete evidence and advance the plan. Even an otherwise successful
last callback cannot produce late success. Protocol failures discovered from
those successful bytes are decided there; owner expiry has priority over pending
success/protocol interpretation. Adverse evidence is the exception to deferred
publication: a complete identity contradiction, asserted reset or observed
control contradiction invalidates affected usability immediately, even if the
owner cancels or times out before another poll. Do not publish partial scratch
buffers as a complete snapshot.
Retain a newly acquired complete signature or revision segment with its raw
validity when it contradicts identity, even before that next poll; a failed
partial receive never supplies such evidence. This does not mark initialization
complete. ACK acceptance flags/effects also record callback OK immediately,
independently of deferred operation completion.

An actual failed callback instead freezes FAILED immediately with its normalized
I2C_* error/detail, including illegal-code normalization to I2C_ERROR. Its
completion time is unknown. A later terminal poll returns that same error even
after deadline; it cannot revise its time or status. Cancel/end/invalidate after
an OK callback also cannot invent a terminal timestamp. Success, protocol failure
or timeout committed on a fresh active poll has completedTimeValid=true and uses
that poll time. This records a software boundary, not exact electrical completion.

Fresh sampling may have the same integer value. The owner must actually sample
after callback return; the core cannot detect reused timestamps. At most 1 ms
resolution plus OWNER_CLOCK_MARGIN_MS=1 prevents early waits after floored
samples. The [physical-time vectors](device-behavior.md#independent-physical-time-vectors)
cover sub-ms completion. The margin never extends the immutable deadline. If a
wait cannot finish before deadline, no late write or automatic new operation is
allowed. A raw reset observation has no time argument: a later explicit reconcile
must establish its conservative anchor. Never use an old last-poll time for it.

The compile-only [arithmetic fixture](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/contract/time_arithmetic.cpp) checks
the unsigned rules independently of any future engine. The external owner
example extends a uint32 source using checked modulo deltas below 2^31, sampling
more often than that horizon; reject ambiguous/backward input without changing
the stored extension. Simply widening a wrapping raw counter is incorrect.

Keep the engine a direct bounded pass: validate time, settle at most one pending
callback, process at most one wait boundary, evaluate completion, select one
next callback, and invoke it at most once. Each of these six local stages runs
at most once per poll; no loop to drain a plan or stabilize silicon is needed.
If implemented as a transition loop, use a named private limit of six local
stages and prove the mapping in engine tests. Identity checks inspect at most
eight bytes; profile checks at most eight fields; fixed register plans never
exceed the procedure guide's twelve callbacks. No allocation, unbounded scan,
sleep, retry, log, scheduler or physical bus recovery belongs in this work.

One callback's aggregate positive timeout covers lock acquisition, pointer and
data phases and terminal cleanup once. Buffers expire at return. No pending DMA,
deferred completion, callback reentry or adapter-added retries are allowed. The
owner must establish that its selected backend actually respects that bound;
the function-pointer type cannot prove it. A callback attempt is not a wire
transaction or evidence of any number of completed physical bytes.

## Port publication and IRQ evidence

`startRefreshPort(const OperationTiming&, OperationToken&)` and
`portSnapshot() const` are implemented. Admission needs a binding only and follows
the existing active/result/token/deadline rules. The refresh reads 09/08/09 with
one callback per nonzero-budget poll, at most three callbacks, three requested TX
bytes and three RX bytes. No refresh path writes, resets or acknowledges IRQ.
The getter returns a fixed cached copy even while unbound, with no callback or
clock sampling. `startAcknowledgeInterrupt(const OperationTiming&, OperationToken&)`
and `startServiceInterrupt(const OperationTiming&, OperationToken&)` are also
implemented. Both need current validated identity; UNKNOWN configuration does
not block a fresh safe actual-control image. TUSB322I returns UNSUPPORTED_FEATURE
before any traffic while V_FAULT_CLEAR gates all09 writes.

Attachment is relative to the local port and does not equal requested mode.
Normal source/sink observations have separate opposite local/peer roles;
accessory status does not invent a normal peer role. Detached orientation and
current can be valid NOT_APPLICABLE values. Reserved accessory/advertisement
encodings retain raw bytes with invalid affected fields. The paired status and
selected variant establish applicability; a single decoded byte cannot.
The [register guide](register-map.md#complete-port-interpretation-implemented-prompt-15) defines the complete
applicability matrix. Ordinary sink detection accepts codes 0/1/3 only with
accessory code 0; charge-through accepts detected-current code 2. Extended
local-UFP debug current remains UNKNOWN under conservative library policy.
Accessory attachment does not establish orientation or active-cable usability.
A contrary persistent 08/09 field can immediately invalidate current configuration
trust, even when a later read fails and the cached port remains unchanged.
Published port provenance still describes configuration before its first read.
Advertisement remains a configuration category, separate from local-sink current
detection. Neither is measured load, a PD contract or board power permission;
DEFAULT is not unconditional 500 mA. Refresh cannot resolve TI's attached-current
tracking uncertainty and never introduces the base-part reset recommendation as
an implicit read side effect. CC and VBUS remain separate hardware facts;
connector orientation and electrical behavior need exact-part/board qualification.
Raw active-cable evidence does not establish usable current or powered VCONN.

| Observation/event | Publication and retained evidence |
| --- | --- |
| Default | invalidated=true, no raw or decoded validity, sequence/time absent |
| S begins | Capture owner time, verifiedGeneration and configurationState immediately before first R09 |
| All R09/R08/R09 succeed and fresh completion poll passes deadline | Publish raw08 and final09, valid fields, captured acquisition provenance; increment saturating sequence; invalidated=false |
| Bookends equal under0xE0 | BOOKENDS_MATCH; apply documented field/context validity, never claim atomicity |
| Bookends differ under0xE0 | CHANGED_DURING_READ; clear detected-current/accessory/active-cable validity, current/accessory UNKNOWN; independent final09 and08 advertisement fields can remain valid |
| Required read fails, cancellation or expiry before complete publication | Preserve all old raw/typed data, sequence, acquisition time and provenance; no partial replacement |
| End, external invalidation, possible port-affecting write | Set invalidated=true; preserve historical fields/time; ACK alone does not invalidate the port observation |
| Detached complete sample | NOT_APPLICABLE interpretations can be valid; not an error or OFFLINE signal |

`observedUptimeMs` with `observedTimeValid` identifies the owner boundary before
the first required read, conservatively including acquisition duration in age;
it is never replaced by a later publication timestamp. Captured configuration
provenance does not promote trust. Sequence counts complete publications from
zero and saturates at UINT64_MAX; it does not count electrical edges.

A complete raw publication can clear invalidated even with changed bookends.
Consumers still check each field, coherence and age. Only a later complete
publication can clear invalidated; unrelated successful I2C cannot. At sequence
saturation the token, not sequence, correlates operation delivery.

The following `InterruptSnapshot` contract belongs to the explicit ACK/service
operations. Plain refresh records sampled IRQ only in
`PortSnapshot.interruptPending`; it leaves its `OperationResult.interrupt` empty
and does not publish diagnostic IRQ history.

| ACK/service step | InterruptSnapshot facts |
| --- | --- |
| Fresh pre-ACK R09 or completed S, processed after the fresh-poll deadline check | observationValid=true and sampled pendingBefore; no implied acknowledgement |
| About to invoke ACK write | acknowledgeAttempted=true before callback |
| ACK callback OK | Immediately acknowledgeConfirmed=true; sampled-clear claim still absent, terminal success still deferred |
| ACK callback failure | Original error/effect retained; acknowledgeConfirmed=false even if physical clear may have occurred |
| Service recheck succeeds and next fresh poll passes deadline | pendingAfterValid=true with sampled pendingAfter; pending=true still allows successful bounded service |
| Recheck failure or interruption | No pendingAfter validity from guessed data; preserve complete published port S and earlier ACK facts |

Refresh produces only the sampled port field described above. ACK-only has no recheck. Service
publishes S before attempting ACK, then rechecks exactly once. Never restart S,
automatically service reassertion or model this as a lossless event queue.
OperationResult.interrupt starts empty for each operation; diagnostic IRQ history
changes only when an IRQ-observing operation publishes some evidence, including
partial ACK/service evidence. Work that obtains no IRQ evidence leaves that
diagnostic history unchanged. Raw reads do not create a managed IRQ record.
ACK-only is R09/W09 with ceilings of two callbacks, three TX bytes and one RX byte.
Full service is R09/R08/R09/W09/R09 with ceilings of five callbacks, six TX bytes
and four RX bytes. Counts include failed attempts and requested bytes, not proved wire
transactions. Each callback yields for fresh owner time; zero budget sends none,
and larger budgets still permit at most one callback per poll.

ACK composes from fresh actual controls, never desired staging or live RO bits.
Extended actual `0xB5` composes `0x15`, actual `0x06` composes `0x16` and base
actual `0xB4` composes `0x14`; staged `0x02` cannot replace actual `0x05`, and
desired 30% cannot replace actual 50%.
If a safe actual image cannot be proved, fail without guessing a write image.
ACK alone preserves port invalidation/sequence/time and configuration trust;
contrary fresh controls still invalidate inconsistent prior configuration proof.
Service's final recheck updates IRQ evidence only, never the complete port sample.
ACK-only has no pending-after evidence. A pending recheck permits successful
bounded service and exposes remaining work for explicit owner policy. Cancellation,
expiry and failure retain all earlier observations and possible write effects.
No repeated ACK, guessed ACK delay, GPIO/ISR service or event queue is added.
TUSB322I keeps raw fault bit3 validity separate; all09 writes and ACK/service remain
gated, and IRQ W1C is never treated as a fault acknowledgement. Read-only does
not establish non-destructive fault access from TI's unresolved RCU wording.

## Transport accounting and passive health

Before each callback increment saturating all-callback totalAttempts; afterward
normalize status, increment exactly one success/failure and replace lastStatus.
Managed poll callbacks also update managed totals/lastStatus: success clears
consecutiveFailures, failure increments it with saturation. Raw Sync updates
only source totals and last-failed transport diagnostics, never the managed
subset. A local I2C_* invalidation reason is not a callback. CLI probe is managed
initialize, regardless of its command alias. Saturated counts are diagnostic
totals; their sum need not remain arithmetically equal.

With basic Status-only callbacks, completion timestamps are unavailable at
return: update the affected last-success/failure validity to false, retaining
its old numeric storage as unusable history. Do not relabel a pre-call sample
as completion or annotate these counters later from an unrelated operation.
The first implementation leaves these callback-time flags false; operation
completion and acquisition timestamps have the separate fresh-poll rules above.
Last failed transport status/detail remains after later success, raw or managed.
No callback means no transport accounting, including owner timeout, cancellation,
configuration mismatch, detached status, invalid args or a wait-only poll.

| Evidence supplied to projectHealth | Result |
| --- | --- |
| offlineThreshold=0 | INVALID_CONFIG; entire out unchanged |
| !initialized | OK, UNINIT regardless of streak/totals |
| initialized, consecutiveFailures=0 | OK, READY |
| initialized, 0 < consecutiveFailures < threshold | OK, DEGRADED |
| initialized, consecutiveFailures >= threshold | OK, OFFLINE |

Copy supplied evidence and effective threshold on successful projection. The
helper never samples time, schedules, rejects a request or repairs trust. READY
with UNKNOWN configuration is valid; OFFLINE never blocks a legal owner request.
Every managed successful callback resets the streak, including during a later
failed protocol comparison. Initialize/end/bind/invalidate control current
initialized independently of counters; raw diagnostics do not change managed
health. `clearTransportStats` clears only all-callback totals/lastStatus. It cannot
reset the managed streak/totals, last failure, trust, token, result or effects.
New targets reset current target readiness, but instance transport history remains.

<a id="required-future-behavioral-checks"></a>

## Behavioral acceptance vectors

These acceptance vectors cover the delivered engine and diagnostics. Prompt 11's
`tusb32x_operations` suite exercises initialization, lifecycle, token/result and
time behavior. Prompts 12-13 add profile observation, read-only proof, apply,
desired verification and their write/effect boundaries. Prompt 14 adds reset and
base-recovery sequence, timing and retained-effect cases. Prompts 15-16 add the
registered port suite for publication and explicit IRQ boundaries. Raw/health
cases are exercised by the registered `tusb32x_diagnostics` suite in prompt 17.
Actual commands/results are recorded
in validation; the complete table also guides later conformance work.
Combine the rejection ordering above with every write/failure boundary in the
procedure guide; test callback counts, outputs, state and original status together.

| ID | Input/event | Required assertion |
| --- | --- | --- |
| O01 | Invalid timing while unbound/busy/result pending | INVALID_PARAM before slot error; token/output/state untouched |
| O02 | Valid start while active / terminal | BUSY / RESULT_PENDING, zero callbacks, no consumed token |
| O03 | Zero / stale / no represented token | INVALID_PARAM / TOKEN_MISMATCH / RESULT_NOT_AVAILABLE; no work advanced |
| O04 | Active snapshot/take; failed terminal exact take twice | Unchanged sentinel; then delivery OK with failed result.status; repeat RESULT_NOT_AVAILABLE |
| O05 | Exhaust UINT64_MAX, end/bind/forget, start | No reuse; TOKEN_EXHAUSTED, output preserved |
| O06 | Invalid bind or different unresolved target | Entire old binding/evidence preserved; both callback pointers participate in association |
| O07 | Cancel before any apply write | Desired changed once, proved actual and port preserved, effect NONE |
| O08 | Failed/cancelled write at every sequence boundary | Original status/detail, PARTIAL/INDETERMINATE as evidenced, no cleanup/retry; result take preserves obligations |
| O09 | Recovery interrupted, end/take/rebind/init | Full matching I/P can set initialized despite CONFIGURATION_UNKNOWN; explicit recovery admitted; zero45 alone cannot repair sequence |
| O10 | Complete F mismatch then explicit new apply | KNOWN actual committed, desired unchanged, mismatch valid; new apply can use actual image |
| O11 | ReadProfile match / contradiction / reserved field / failed read | No promotion; contradiction/reserved invalidates current proof; failed image preserves previous complete data/time |
| O12 | Invalid/backward/half-range poll; equal tick; uint64 wrap | Invalid preserves every field; equal valid but no elapsed wait; wrapping deadline/waits work |
| O13 | Zero budget; larger budget; failed callback | Time transitions allowed without I2C; at most one callback per poll; failure counts once |
| O14 | Callback OK just before deadline, next fresh poll at deadline | TIMEOUT, no late success or next write; possible effects retained |
| O15 | Callback I2C_TIMEOUT then terminal poll after deadline | Same I2C_TIMEOUT/detail, completedTimeValid=false; never owner TIMEOUT substitution |
| O16 | Cancel/end/invalidate after successful callback | No invented terminal time, no further callback; reason I2C_TIMEOUT still local CANCELLED with no transport failure increment |
| O17 | Invalid invalidation reason IN_PROGRESS or unknown enum | INVALID_PARAM; binding/result/evidence untouched |
| O18 | Reset bit observed in P/precheck/raw then zero | MAY_BE_PENDING retained until required wait/proof; raw read OK but no timestamp or KNOWN promotion |
| O19 | Successful reset with older recovery obligation | Own OK/CONFIRMED, separate recovery uncertainty and UNKNOWN config retained |
| O20 | Changed S bookends, partial read, detach | Complete changed raw publication clears dependent flags; partial preserves old observation/time; detached N/A valid |
| O21 | Complete S then ACK/recheck failure, or IRQ reassertion | S/time retained; separate ACK uncertainty; five-callback ceiling, no lossless queue claim |
| O22 | Raw success/failure, clear totals while OFFLINE | Source totals count; managed health/streak unchanged; legal managed action still admitted |
| O23 | Invalid health policy with sentinel out; threshold1 and UINT32_MAX | Out preserved for invalid; exact boundary projection, no overflow or policy action |
| O24 | Two driver instances on one external bus, stale token on one | Serialized independent work; no token, target evidence or callback-lifetime cross-contamination |
| O25 | Ambiguous raw settings write, then read-only operation with effect NONE | configurationWriteUnresolved remains true; different-target bind rejected; complete F can resolve actual controls even on desired mismatch without claiming the old write succeeded |

## Verification boundary

The [fixture instructions](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/README.md) compile each delivered header in
isolation, check constant evaluation/explicit conversion, and link two real
Status consumers. Public signatures and the
[owner examples](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/contract/owner_examples.cpp) are syntax-checked only;
the arithmetic fixture evaluates specification expressions at compile time. These
checks establish their passive/compile-only contracts. Separate runtime suites
exercise validators, private transport/codecs, initialization and profile
observation/apply/verify/reconciliation, and
public link tests call every delivered method. None proves
linked framework firmware or physical qualification. Results and tool versions
are in [validation](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md).
