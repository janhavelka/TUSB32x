# Portable values and planned API

Prompt 5, 2026-10-23. The four headers under `include/TUSB32x/` now provide
complete passive values and constexpr Status/token helpers. They require C++17
and standard headers only. No device facade, validator, decoder, bus access or
operation engine is implemented yet. This distinction also applies to every
signature below. Prompt 6 completes admission, publication and lifetime tables;
later prompts publish callable declarations with their real implementations.

The [baseline](prompts/ENGINEERING_BASELINE.md), [register map](register-map.md)
and [procedure guide](device-behavior.md) remain the state and silicon contracts.
The [architecture](architecture.md) identifies file responsibilities. The
[compile-only sketch](../test/contract/PlannedApi.h) checks this planned inventory
without adding unresolved link promises to delivered headers.

## Implemented value inventory

All fields are ordinary fixed-storage values. Default construction performs no
I2C, allocation, validation, pin action or power authorization. Semantic enums
are not register encodings; later codecs use explicit variant-specific tables.
The header Doxygen owns the complete field/default inventory.

| Header and types | Contract |
| --- | --- |
| [Status.h](../include/TUSB32x/Status.h): Err, Status, errorName | Ordered uint8 errors; full int32 detail; static-lifetime message; constexpr helpers and explicit bool |
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
Prompt 6 defines exact publication and invalidation boundaries before the engine.

ACK `acknowledgeConfirmed` means a terminal write returned OK. It is never proof
of a sampled-clear latch. `pendingAfterValid` and `pendingAfter` record the
separate recheck. No timestamp is fabricated from a Status-only callback.
Independent `recoverySequenceUnresolved`, `modeTransitionUnresolved` and
`resetAcceptanceUncertain` diagnostics retain obligations after result take.
One latest-operation kind/effect cannot erase an older interrupted recovery.
Reset COMPLETED describes settling/readback, not proof of ambiguous acceptance.

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
| INVALID_PARAM | Bad argument, enum, register, interval or token; zero callbacks, output/state preserved. |
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
Actual operation-status propagation, callback normalization and counters are
future tested code, not behavior claimed by these passive structs.

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

Callbacks may return only OK or I2C_* errors. Future wrappers normalize any other
code to I2C_ERROR, preserve full detail and retain conservative write uncertainty.
Status carries no completion time: the next freshly sampled owner timestamp
anchors hardware waits and time-sensitive success. Owner intervals are strictly
between zero and 2^63 ms; the separate 1 ms hardware margin never extends the
immutable deadline. See the procedure guide for actual transfer ceilings.

## Planned callable inventory

This exact inventory is **not public callable code yet**. The fixture declares
it solely to compile-check names, argument types and non-copyable/non-movable
ownership. Construction/destruction do not perform hardware work; one facade
and one engine are planned. No begin/init/unbind aliases or implementation stubs.

```cpp
namespace TUSB32x {
Status validateDriverConfig(const DriverConfig& config);
Status validateProfile(Variant variant, const DeviceProfile& profile);
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
does not replay, and reset never reapplies a desired profile. Method precedence,
result retention and interruption rows are prompt 6's deliverable. The baseline
already governs these rules; nothing here introduces a competing precedence.

Pure binding/profile validators are implemented in prompt 7. Driver methods are
published with their owning work in prompts 9–17; health projection is prompt 17.
No unavailable function may appear in a runnable quick start before that work.
The health validator requires offlineThreshold >= 1. Invalid policy returns
INVALID_CONFIG, and projectHealth preserves out on that failure; otherwise it
copies the supplied managed evidence and effective threshold into its projection.
It performs no I2C, retry, clock read or evidence repair.

## Pure register decoder boundary

`decodeRegister(Variant, uint8_t reg, uint8_t value, RegisterDecode&)` is planned
for prompt 10; its passive result exists now in CommandTable.h. It interprets one
caller-supplied byte using the exact [field tables](register-map.md#explicit-field-encodings).
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

## Verification boundary

The [fixture instructions](../test/README.md) compile each delivered header in
isolation, check constant evaluation/explicit conversion, and link two real
Status consumers. The planned signature fixture is syntax-checked only. These
checks establish the passive C++ contract, not protocol, owner-state transitions,
framework compatibility or hardware qualification. Prompt 7 registers reusable
checks in the real build. Results and tool versions are in [validation](validation.md).
