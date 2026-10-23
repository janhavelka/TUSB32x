# Selected TUSB32x engineering baseline

This file makes the implementation choices concrete. It is a specification for
future work, not a claim that a driver already exists. All numbered stages read
[COMMON.md](COMMON.md) and the relevant sections here. Use simple engineering
English in code comments, Doxygen, errors, examples, reports and user documents.

## Evidence and selected differences

Read-only source review on 2026-10-23 found these local revisions clean. Recheck
their current state before each implementation stage; this table is dated
evidence, not permission to skip that check. Paths are relative to `Projects/`.

| Peer and revision | Actual declarations and implementation inspected | Selected TUSB32x use |
| --- | --- | --- |
| LDC1614 `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` | `include/LDC1614/Status.h`; `Config.h`; `LDC1614.h` (`TransportStats`, `TerminalOutcome`, `EffectFlag`, `poll(uint64_t,uint8_t)`); `src/LDC1614.cpp` (`bind`, `poll`, `cancelJob`, `takeResult`) | Status layout, 64-bit owner time, bounded callbacks, neutral saturating counters. Do not copy its multi-result ring or caller-assigned operation IDs. |
| LSM6DS3TR `513c5641d9b7dda63fddc27267ea62aa8d03ff93` | `include/LSM6DS3TR/Status.h`, `Config.h`, `LSM6DS3TR.h` (`OperationTiming`, 64-bit `OperationToken`, `ConfigurationState`, `DriverDiagnostics`); `src/LSM6DS3TR.cpp` (`bind`, transport wrappers, `cancelActiveJob`, `takeResult`) | One tokened terminal slot, copied configuration evidence, small passive diagnostics. Omit sensor settling, FIFO, calibration and compatibility facades. |
| TMP1x2 `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `include/TMP1x2/Status.h`, `TMP1x2.h` (`DriverState`, `HealthSnapshot`, token/result names); `src/TMP1x2.cpp` (`bind`, `track`, `end`); `src/TMP1x2Operations.cpp` (`cancel`, `takeResult`) | Explicit bool Status, familiar health names, token-checked take with unchanged output on failure. Its bind clears the binding before validation; deliberately use atomic validation instead. Do not copy measurement errors or managed-only source counters. |
| ADS1115 `381236ebfed813cd7a9aca34520ce031d8a0ff6b` | `include/ADS1115/ADS1115.h` (`DriverConfig`, `DeviceProfile`, `AppliedProfileSnapshot`, `OperationResult`); `src/ADS1115.cpp` (`bind`, profile admission, `takeResult`, configuration invalidation) | Split transport from silicon settings, retain desired/applied evidence and result backpressure. No ADC conversion, mux or legacy API. |
| INA3221 `e45622dbaa5bb0b3151d980f30a1b6ea1cdb1a1b` | `include/INA3221/INA3221.h` (`AppliedConfigState`, `HardwareEffect`, `PollContext`, `JobResult`); `src/INA3221.cpp` (`bind`, `_finishJob`, cancellation and effect classification) | Distinguish partial/uncertain writes from terminal outcome. Its `startReconcile` reapplies a profile; TUSB32x reconciliation is read-only because a role/reset write must not be replayed after uncertainty. |

Behavioral and tooling references also inspected: LDC1614 `test/test_basic.cpp`,
`test/support/FakeLdc1614Device.h`, `examples/common/Ldc1614Cli.*`,
`tools/ldc1614_cli_contract.py`, `tools/ldc1614_hil_runner.py`; LSM6DS3TR
`test/test_basic.cpp`, `test/test_compile_contracts.cpp`,
`examples/02_owner_soak/main.cpp`, `tools/run_owner_soak.py`; TMP1x2
`test/jobs/test_jobs.cpp`; ADS1115 `examples/02_owner_safe_poll/main.cpp` and
`test/test_basic.cpp`; INA3221 `test/test_owner_operations.cpp` and
`test/support/ScriptedTransport.h`; each peer's `.github/workflows/ci.yml`.
Their AGENTS describe conventions, not authority to change or publish a sibling.

The selected names below are TUSB32x decisions. They are not presented as one
existing API shared by all peers. A later evidence-based change updates this
file, decision record, public declarations, implementations, tests, CLI and docs
together. Do not add aliases for names that have never shipped.

## Folder and responsibility map

Use these responsibilities and preferred private source names. Public header/API
names remain fixed. Adjacent small private responsibilities may share a source
unit until separate files improve readability; keep build lists and docs aligned.
Do not add empty classes or unused placeholders merely to match the tree.

```text
include/TUSB32x/
  TUSB32x.h                 Public facade, operation/result/snapshot types
  Config.h                  Callbacks, DriverConfig, DeviceProfile, typed settings
  Status.h                  Err, Status and errorName; no platform errors
  CommandTable.h            Verified registers, access rules and variant capabilities
  Version.h                 Generated package version, no build-time Git dependency
src/
  TUSB32x.cpp               Binding, teardown, validation and cache-only public access
  TUSB32xProtocol.cpp       Exact callback framing, safe byte composition, counters
  TUSB32xOperations.cpp     One owner-driven operation engine and result slot
  TUSB32xPort.cpp           Port decoding, staged observations and IRQ service
  TUSB32xDiagnostics.cpp    Passive diagnostics and optional pure health projection
examples/
  common/Tusb32xCli.*        One portable CLI/parser/output contract; fixed buffers
  common/BoardConfig.h       Shared example settings, no core dependency
  arduino/ArduinoTransport.* Arduino-only application transport
  01_basic_bringup_cli/     Thin Arduino application and board transport
  02_owner_service/         Explicit bounded owner example
  esp_idf/basic/main/       Native ESP-IDF application
  esp_idf/common/IdfTransport.* Native application transport, no Arduino
  host/basic/               Plain C++ consumer with app-owned callback stubs
test/
  support/                  Scripted fake transport and externally timed fake owner
  test_protocol.cpp         Bytes, masks, access semantics, identity and variants
  test_operations.cpp       Admission, timing, cancellation, effects and results
  test_port.cpp             Decode, observation publication and IRQ boundaries
  test_diagnostics.cpp      All-callback counters and pure projection transitions
  cli/test_cli.cpp          Execute portable command processing with fake transport
  cli/cli_host.cpp          Executable CLI fixture; target tusb32x_cli_host
  console/test_console.cpp  Bounded console input/output behavior
  test_compile_contracts.cpp Public headers, types and non-Arduino linkage
  consumer/                 Direct includes and multiple-translation-unit consumers
scripts/                    Version generator and existing-managed-PIO wrapper
tools/                      Guards, CLI contract, runner, runner tests, validation
docs/                       Architecture, API, ownership, variants, CLI and validation
docs/reference/             Vendor artifacts, provenance and licenses; not core input
.github/workflows/          Host, package, framework and documentation gates
```

Keep private helpers private. Add a small `src/` internal header only when two
source files actually need it; do not expose internals through `include/` for
convenience. The facade coordinates the state; there is one transport wrapper,
one register-write composer and one operation engine, not platform-specific
copies. Tests follow responsibility, not one enormous catch-all file.
This layout does not require interfaces, factories, inheritance, allocation,
event buses, workers, scheduler classes, plug-in registries or a public Pimpl.

Register behavioral CTest names `tusb32x_protocol`, `tusb32x_operations`,
`tusb32x_port`, `tusb32x_diagnostics`, `tusb32x_cli`, `tusb32x_console` and
`tusb32x_compile_contracts` as their corresponding real suites are introduced.
The executable CLI fixture target is `tusb32x_cli_host`; it is not an extra
passing test merely because it compiles. Core CMake target remains `TUSB32x`.

## Status and exact error vocabulary

Use `enum class Err : uint8_t` with the following members, in this order. `OK`
has value zero. Keep names stable after the API ships; do not inherit numeric
values from an unrelated device library.

| Err | Meaning and boundary |
| --- | --- |
| `OK` | The requested API action succeeded; for takeResult this means delivery succeeded. Read `OperationResult.status` for operation outcome. |
| `NOT_BOUND` | A required valid transport/device binding is missing. |
| `NOT_INITIALIZED` | This operation needs successful read-only initialization evidence. A read-only probe/observation/reconciliation must still be possible. |
| `INVALID_CONFIG` | Binding or whole silicon profile is internally invalid. |
| `INVALID_PARAM` | Invalid method argument, enum value, register, time interval or budget. |
| `UNSUPPORTED_FEATURE` | A valid request is unavailable on the explicitly selected variant. Reject before traffic. |
| `BUSY` | An active operation owns the instance. |
| `IN_PROGRESS` | An operation was admitted or remains active. It is not terminal transport status. |
| `RESULT_PENDING` | A terminal result occupies the sole slot. Take it before admitting replacement work. |
| `RESULT_NOT_AVAILABLE` | The matching operation has no terminal result yet, or no operation/result exists. |
| `TOKEN_MISMATCH` | A supplied token does not match the represented active operation or result. Preserve both. |
| `TOKEN_EXHAUSTED` | All nonzero token values were used; never wrap into an old token. |
| `TIMEOUT` | The owner operation deadline expired. Distinct from callback `I2C_TIMEOUT`. |
| `CANCELLED` | An admitted operation was cancelled locally. Possible hardware effects remain. |
| `CONFIGURATION_UNKNOWN` | Required verified configuration/effect evidence is unavailable. |
| `CONFIGURATION_MISMATCH` | A successful readback differed in persistent managed bits. It is not a transport error. |
| `CHIP_ID_MISMATCH` | Complete successful identity reads contradicted the chosen identity expectation. ACK alone is insufficient. |
| `I2C_ERROR` | Generic terminal callback failure, including a callback returning illegal IN_PROGRESS. |
| `I2C_NACK_ADDR` | Backend positively reports address NACK. Do not infer it from a generic error. |
| `I2C_NACK_DATA` | Backend positively reports data NACK. Earlier bytes can already have effects. |
| `I2C_TIMEOUT` | One terminal callback exceeded its adapter timeout. |
| `I2C_BUS` | Backend positively reports a bus fault. Recovery remains external. |
| `I2C_BUSY` | Backend reports inability to acquire/use the bus within its bounded contract. It is an attempted callback, unlike local BUSY. |

Do not add `OFFLINE`, cable-detached, sensor-not-ready, conversion, calibration
or FIFO errors. OFFLINE is a projection; detach is a valid observation. A reserved
encoding remains raw evidence with invalid typed fields, rather than becoming a
fake plausible default or an I2C failure.

The public representation is:

```cpp
struct Status {
  Err code = Err::OK;
  int32_t detail = 0;
  const char* msg = ""; // Static lifetime; never a temporary string.
  constexpr Status() = default;
  constexpr Status(Err c, int32_t d, const char* m)
      : code(c), detail(d), msg(m) {}
  constexpr bool ok() const { return code == Err::OK; }
  constexpr bool is(Err value) const { return code == value; }
  constexpr bool inProgress() const { return is(Err::IN_PROGRESS); }
  explicit constexpr operator bool() const { return ok(); }
  static constexpr Status Ok() { return {Err::OK, 0, "OK"}; }
  static constexpr Status Error(Err code, const char* msg, int32_t detail = 0) {
    return {code, detail, msg};
  }
};
constexpr const char* errorName(Err code);
```

The errorName line is a signature sketch: provide its complete constexpr
definition in Status.h so every translation unit can use it, including constant
evaluation. Do not put its only definition in a .cpp file.

`detail` preserves full backend evidence when relevant; document each non-backend
use. Provide one canonical `errorName`, with UNKNOWN for invalid enum input.
Do not add redundant `toString`, `getError`, integer-return or bool-return APIs.
The implementation adds complete Doxygen to every declaration, member and enum
value; these compact sketches do not replace source documentation.

## Transport, types and public method names

Keep the observed callback shapes:

```cpp
using I2cWriteFn = Status (*)(uint8_t address, const uint8_t* data,
                            size_t length, uint32_t timeoutMs, void* user);
using I2cWriteReadFn = Status (*)(uint8_t address, const uint8_t* txData,
                                size_t txLength, uint8_t* rxData,
                                size_t rxLength, uint32_t timeoutMs, void* user);
struct OperationToken {
  uint64_t value = 0;
  constexpr bool valid() const { return value != 0; }
};
struct OperationTiming { uint64_t nowMs = 0; uint64_t deadlineMs = 0; };
```

`DriverConfig` contains `i2cWrite`, `i2cWriteRead`, `i2cUser`, `variant`, `address`
and nonzero `i2cTimeoutMs`. `Variant` names explicitly verified parts;
`DeviceAddress` names verified strap/address choices. Validate their combination.
The application selects the exact variant; identification does not guess it.
`DeviceProfile` contains only P01-verified silicon settings, such as `portMode`,
`sourceCurrent`, `sourcePreference`, `drpDutyCycle` and variant-specific controls.
Do not put desired initialization writes or health thresholds in DriverConfig.
Do not let a default DeviceProfile silently advertise power on binding.

### Concrete settings and snapshot vocabulary

These are chosen semantic names, not direct copies of sibling sensor enums.
The register meanings come from the exact TI variants in P01 and
`docs/audit/ti-family.md`; keep the P01 source revision/page/mask beside the
encoder table. Enum ordinal is not a register encoding unless explicitly stated.
Do not cast an enum straight into a register to avoid writing the table.

| Type | Initial exact members and meaning |
| --- | --- |
| Variant | UNSPECIFIED, TUSB320, TUSB320I, TUSB320LI, TUSB320HI, TUSB320LAI, TUSB320HAI, TUSB322I. UNSPECIFIED is invalid binding. TUSB322I operations remain capability-gated by resolved source evidence. |
| DeviceAddress | ADDR_LOW_0X60, ADDR_HIGH_0X61, ADDR_LOW_0X47, ADDR_HIGH_0X67. Explicit values are their seven-bit addresses; validate against Variant. No floating-ADDR I2C choice. |
| PortMode | FOLLOW_PORT_PIN, UFP, DFP, DRP. Local requested mode. FOLLOW_PORT_PIN is unsupported on 322I; its register mode00 means DRP, never pin-following. |
| SourceCurrent | DEFAULT, MA_1500, MA_3000. CC advertisement categories, not measured/authorized board load. |
| SourcePreference | NORMAL, TRY_SNK, TRY_SRC. Gate Try modes by variant. |
| DrpDutyCycle | PERCENT_30, PERCENT_40, PERCENT_50, PERCENT_60. Semantic duty; encode by explicit table. |
| CcDebounce | MS_116, MS_118, MS_133, MS_134, MS_151, MS_152, MS_168. Support each time only on variants whose P01 map provides it; invalid pair fails locally. |
| Attachment | UNKNOWN, DETACHED, LOCAL_SOURCE, LOCAL_SINK, ACCESSORY. UNKNOWN means undecoded/unavailable, not cable detached. |
| PortRole | UNKNOWN, NOT_APPLICABLE, SOURCE, SINK. Snapshot has separate localRole and peerRole; do not derive a normal peer power role from an accessory encoding. |
| Orientation | UNKNOWN, NOT_APPLICABLE, CC1, CC2. Detached has NOT_APPLICABLE; failed/unread field UNKNOWN. |
| DetectedCurrent | UNKNOWN, NOT_APPLICABLE, DEFAULT, MA_1500, CHARGE_THROUGH_500_MA, MA_3000. Only valid in documented local-sink/accessory contexts. |
| Accessory | UNKNOWN, NOT_APPLICABLE, NONE, AUDIO, CHARGE_THROUGH, DEBUG, DEBUG_LOCAL_DFP, DEBUG_LOCAL_UFP. Gate raw-code interpretation by selected variant. |
| SnapshotCoherence | SEQUENTIAL, BOOKENDS_MATCH, CHANGED_DURING_READ. None means hardware-atomic. Matching bookends only prove the observations they contain. |

`DriverConfig` has exactly the following initial field/default contract:

```cpp
I2cWriteFn i2cWrite = nullptr;
I2cWriteReadFn i2cWriteRead = nullptr;
void* i2cUser = nullptr; // Null is allowed when callbacks need no context.
Variant variant = Variant::UNSPECIFIED; // Must be selected explicitly.
DeviceAddress address = DeviceAddress::ADDR_LOW_0X47;
uint32_t i2cTimeoutMs = 20; // Selected adapter cap, not a TI timing value.
```

Default DeviceProfile is a concrete candidate: `portMode = PortMode::DRP`,
`sourceCurrent = SourceCurrent::DEFAULT`,
`sourcePreference = SourcePreference::NORMAL`,
`drpDutyCycle = DrpDutyCycle::PERCENT_50`, `ccDebounce = CcDebounce::MS_168`,
`disableUfpAccessory = false`, `disableTerm = false`, `disableRdRp = false`.
These are selected requested settings, not a claim of reset defaults. Only an
explicit accepted apply may write them. `disableTerm` names 0A state-machine/
termination disable; `disableRdRp` names the separate 45 control. Never substitute
one for the other. Unsupported true/feature values fail before traffic; false
does not authorize touching a register absent on that variant. Apply follows the
verified safe mode sequence, then establishes the requested final disable state
through separately reviewed steps; it must not accidentally enable a profile
whose requested final state is disabled. P01 must establish these sequences.
322I advertisement/debounce/control requests require its independently closed
capability evidence; shared member names do not establish support.

The initial PortSnapshot aggregate uses these exact public fields:

```cpp
uint8_t rawStatus08 = 0;
uint8_t rawStatus09 = 0;
uint8_t rawValidMask = 0; // STATUS_08_VALID and STATUS_09_VALID constants.
uint16_t validFields = 0; // Named PortField mask bits below.
bool invalidated = true; // Historical fields are not current usable evidence.
Attachment attachment = Attachment::UNKNOWN;
PortRole localRole = PortRole::UNKNOWN;
PortRole peerRole = PortRole::UNKNOWN;
Orientation orientation = Orientation::UNKNOWN;
DetectedCurrent detectedCurrent = DetectedCurrent::UNKNOWN;
SourceCurrent advertisedCurrent = SourceCurrent::DEFAULT; // Invalid until flagged.
Accessory accessory = Accessory::UNKNOWN;
bool activeCable = false; // Meaningful only with ACTIVE_CABLE validity.
bool interruptPending = false; // A sampled level, not ACK proof.
uint64_t sequence = 0;
uint64_t observedUptimeMs = 0;
bool observedTimeValid = false;
uint32_t configGeneration = 0;
ConfigurationState configurationState = ConfigurationState::UNCONFIGURED;
SnapshotCoherence coherence = SnapshotCoherence::SEQUENTIAL;
```

Define `enum class PortField : uint16_t` flags ATTACHMENT, LOCAL_ROLE, PEER_ROLE,
ORIENTATION, DETECTED_CURRENT, ADVERTISED_CURRENT, ACCESSORY, ACTIVE_CABLE and
INTERRUPT_PENDING as named one-bit values. `validFields` records successful and
documented interpretation; NOT_APPLICABLE is a valid interpretation of an observed
state. An unsupported/unknown raw code leaves its field flag clear. A failed
transfer cannot populate flags from zero-filled buffers. No public `valid=true`
shortcut may imply all fields share applicability or time.

`invalidated` is separate from historical field validity. Set it false only when
a complete fresh observation is published. Set it true on end, external device
invalidation, and before a write that may change port configuration, termination
or reset state, including an uncertain write. Preserve the old raw/typed fields,
sequence, acquisition time and configuration provenance. A failed refresh does
not change those facts or clear invalidated. ACK-only traffic preserves the
observation; sampled interruptPending is not a claim about the latch after ACK.
An uninvalidated snapshot can still be old: the application checks its age.
For PortSnapshot, observedUptimeMs records the supplied owner timestamp before
the first required read of the published observation. With observedTimeValid it
identifies that acquisition boundary in the owner clock domain, not the instant
of hardware sampling. An age derived from it conservatively includes acquisition
time; sequential registers still have different sampling times. Do not replace
it with a later post-callback poll/publication timestamp and imply younger data.

The flag values in that listed order are `1U << 0` through `1U << 8`.
STATUS_08_VALID is `1U << 0`; STATUS_09_VALID is `1U << 1` in rawValidMask.

`ConfigurationSnapshot` has `hasDesiredProfile`, `desiredProfile`,
`desiredGeneration`, `hasObservedProfile`, `observedProfile`,
`hasVerifiedProfile`, `verifiedProfile`, `verifiedGeneration`, `state`,
`terminationState`, `resetState`, `mismatchValid`, `mismatchRegister`,
`mismatchExpected`, `mismatchObserved`, `mismatchMask`, `observedUptimeMs` and
`observedTimeValid`. Profiles begin default-initialized with the has-flags false;
generations begin zero; state UNCONFIGURED; termination UNKNOWN; reset NONE;
mismatch validity false. Increment desiredGeneration on accepted desired profile
replacement; increment verifiedGeneration on each complete verified commit.
Use saturating uint32 values, with zero reserved for no record; at saturation the
value is a diagnostic counter, not a unique identity. Snapshots/results copy the
relevant generation; OperationToken remains the correlation identity.

KNOWN means the actual persistent configuration and required effect evidence are
established; it does not mean the actual profile equals the desired profile.
Read-only verification or reconciliation may publish the proved actual image as verifiedProfile
and advance verifiedGeneration without replacing desiredProfile. If it compares
against a retained desired profile and finds a difference, report
CONFIGURATION_MISMATCH while retaining that known actual image. Unresolved
transient completion remains UNKNOWN. A later explicit apply can use established
actual controls even when they differ from desired; do not require the interrupted
desired profile to match before allowing recovery. Reconciliation never proves
that an earlier operation succeeded or that an unobserved sequence occurred.

Decoded profile comparison is semantic where different register encodings mean
the same mode, especially 322I DRP00/11. Preserve raw observed bits and compare
using the exact P01 equivalence rule. Never call a valid reset DRP mode a mismatch
solely because its bit pattern differs from the selected canonical encoder.

Use these public signature names. Fill the data structures from the P01 register
matrix in P02 without renaming the chosen contract. Additional device features need a concrete TI requirement, not a
second vocabulary for the same operation.

```cpp
Status validateDriverConfig(const DriverConfig& config);
Status validateProfile(Variant variant, const DeviceProfile& profile);

class TUSB32x {
 public:
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
```

Token-checked poll is a selected extension to the generally implicit-active-token
peer poll APIs. It prevents a delayed owner command from advancing replacement
work. `valid()` only means nonzero; equality against this instance's represented
token establishes currency. Tokens are instance-local, never reused through
end/bind/forget. The owner must keep tokens paired with their instance.
Make the driver non-copyable and non-movable; no borrowed pointer into its state.

For local validation, validate method arguments before binding/slot requirements;
then NOT_BOUND, BUSY for an active slot, RESULT_PENDING for terminal backpressure.
Tokened access first rejects token zero as INVALID_PARAM; without represented
work/result return RESULT_NOT_AVAILABLE; a different represented token gives
TOKEN_MISMATCH. Matching active take/snapshot returns RESULT_NOT_AVAILABLE.
Matching terminal poll returns the retained terminal status without traffic;
matching terminal cancel returns OK without changing the result. After take,
the token no longer identifies represented work. Document these cases in P02.

| Method group | Work and mutation contract |
| --- | --- |
| validate/bind/start | Zero callbacks. Validate and stage copies. An admitted start returns IN_PROGRESS and one new token. Failed calls leave output token and prior evidence unchanged. |
| startInitialize | Identity and initial register observation only. Never apply defaults, reset, ACK IRQ or change role. Successful evidence enables managed initialized state. |
| startReadProfile | Read current persistent settings; no desired-profile replacement and no trust promotion by itself. |
| startApplyProfile | Explicit complete candidate application and persistent-field verification. Retain the desired candidate at admission; keep it separate from observed/applied values even after cancellation. |
| startVerifyProfile | Read actual persistent settings and compare with retained desired. Complete identity/control/effect proof establishes KNOWN actual state even when the comparison returns CONFIGURATION_MISMATCH. Keep desired unchanged; incomplete proof remains UNKNOWN. |
| startReconcile | Read identity/configuration/termination/reset evidence needed to resolve a previously interrupted sequence. No role/reset replay and no hidden cleanup write. May finish with CONFIGURATION_UNKNOWN when observation cannot prove completion. Share verification helpers rather than duplicate them. |
| startRefreshPort | Live read-only port refresh; preserve old successful snapshot on failure. |
| startAcknowledgeInterrupt | Explicit ACK-only write using a fresh safe image of actual control bits, never staged desired settings. No promise of full service/recheck. |
| startServiceInterrupt | Observe/publish, explicit W1C ACK, bounded recheck. It is a write operation even if caller thinks of it as reading events. |
| startReset | Explicit one-shot reset command and bounded observation. No automatic desired-profile reapply. A failed/uncertain reset is never resent by the operation. |
| startRecoverDeadBattery | Explicit base TUSB320/TUSB320I post-VDD-return sequence from P01, using the same operation engine. Requires valid identity; the caller owns the VDD-return prerequisite and decision that recovery is needed. Other variants return UNSUPPORTED_FEATURE without traffic. No automatic initialization write or desired-profile replay. |
| poll | Only managed-operation path that invokes callbacks; explicitly named Sync diagnostics are separate. Zero budget invokes none; default at most one. Time-only transitions are bounded too. |
| cancel | Local cancellation succeeds with OK; retained operation result has CANCELLED. No callback/rollback. Terminal result with matching token remains unchanged. |
| resultSnapshot/takeResult | Copy terminal result without/with consumption. Successful delivery returns OK even if the result contains I2C_ERROR. Failure leaves `out` unchanged. |
| progress/snapshot/stats/diagnostics | Copy cached facts, no callback, no hidden age poll, no logging. |
| readRegisterSync/writeRegisterSync | Explicit bounded diagnostic escape: at most one terminal callback; return exact raw data on success. Validate mapped register/access/variant first, obey mixed-register rules, reject while operation/result owns slot. Raw write invalidates affected trust before invocation and keeps possible effects after failure. Raw reads with variant-specific consuming effects are documented. |
| end | No callback. Cancel active work into the retained terminal slot, unbind transport and invalidate current usability; keep possible effects and result readable. Idempotent with a retained result. It does not reset the chip. |
| invalidateDeviceState | Bus-silent notification of external reset/power loss/change. Cancel active work, retain result/effects, invalidate identity/config/port freshness. Normalize an OK reason to CONFIGURATION_UNKNOWN. |
| forgetDeviceEvidence | Explicit local evidence loss, only while unbound, inactive and with no retained result. Never presented as cleanup or hardware recovery. Does not reset tokens or transport totals. |

`end` is the sole public unbinding method; do not add a second unbind alias.
CLI setters may edit their own candidate DeviceProfile without calling the core.
Only explicit apply starts device changes. ACK composition uses actual observed
control fields, never the candidate or desired image. A one-callback raw write
cannot enforce a multi-step role/reset/termination procedure: reject changes that
require that procedure and direct the caller to the typed owner operation.
Do not expose a raw shortcut that bypasses the sequenced silicon preconditions.
If safe composition needs a read before a write, a one-callback Sync write cannot
perform both: use a current proved safe image or reject without traffic. Never
hide two callbacks in a function documented as one.

For startRecoverDeadBattery, first read 0x45 and require its TI-internal bits
1:0 to be zero; otherwise fail CONFIGURATION_UNKNOWN before any write. Write
literal 0x04, wait the TI 30 ms interval plus OWNER_CLOCK_MARGIN_MS from a fresh
completion boundary, write literal 0x00, then verify `(reg45 & 0x07) == 0`. This is at
most four callbacks (two one-byte reads and two two-byte writes), six TX bytes
and two RX bytes, excluding bus address phases. Apply normal deadline/result/
effect rules and publish no transient port snapshot. Preserve interrupted
sequence and termination evidence through cancel/end/rebind; a later read of
0x00 alone cannot prove that the required sequence and wait occurred. An explicit
owner-selected new attempt is not an automatic replay. No startup state machine
or core VDD/power-policy field is required.
Retain that unresolved sequence in the existing effect record, and expose its
kind/uncertainty through diagnostics as well as the terminal result, including
after result consumption. No admission restriction may depend on hidden recovery
history. Current termination-control evidence and sequence completion are separate.

## State, result and effect separation

Use compact typed enums with exactly these initial members:

```cpp
enum class OperationKind : uint8_t {
  NONE, INITIALIZE, READ_PROFILE, APPLY_PROFILE, VERIFY_PROFILE,
  RECONCILE, REFRESH_PORT, ACKNOWLEDGE_INTERRUPT, SERVICE_INTERRUPT, RESET,
  RECOVER_DEAD_BATTERY
};
enum class OperationState : uint8_t {
  IDLE, ACTIVE, SUCCEEDED, FAILED, CANCELLED, TIMED_OUT
};
enum class ConfigurationState : uint8_t {
  UNCONFIGURED, APPLYING, KNOWN, UNKNOWN
};
enum class HardwareEffect : uint8_t {
  NONE, CONFIRMED, PARTIAL, INDETERMINATE
};
enum class TerminationState : uint8_t { UNKNOWN, ENABLED, DISABLED };
enum class ResetState : uint8_t { NONE, MAY_BE_PENDING, COMPLETED };
```

These enums are chosen vocabulary, informed by the peers above. There is no
sensor-style SETTLING configuration state. Required silicon waits are operation
phases. `OperationState` describes software completion; `HardwareEffect`
describes this operation's writes; configuration and termination/reset evidence
describe retained device facts. Never use one enum to encode their cartesian
product. INDETERMINATE is an effect, not an excuse to discard the original status.

`TerminationState` reports configured CC termination availability from every
supported disable control, including independent 0A disableTerm and 45 disableRdRp
where present. ENABLED requires complete verified evidence that all applicable
disable controls are clear and no reset/transition remains unresolved. DISABLED
requires evidence that a documented controlling disable remains effective;
otherwise use UNKNOWN. Clearing 0A alone while 45 remains disabled never means
ENABLED. This describes configured availability, not electrical measurement or
proof of attachment. Keep the individual control values in profile evidence.

For `HardwareEffect`: NONE means no data-changing write could have reached the
target; CONFIRMED means every intended write and required postcondition was
verified; PARTIAL means at least one confirmed write occurred but the requested
final state was not fully verified; INDETERMINATE means any attempted write has
unresolved acceptance. An address NACK can prove no effect for that attempt only
when the backend's phase guarantee says so; it cannot undo an earlier write.
Read-only success has NONE. A successful reset command write alone does not
establish reset completion. Read-only reconciliation does not rewrite the old
operation's result or claim the old operation succeeded.

`OperationResult` contains `token`, `kind`, `state`, `status`, `hardwareEffect`,
`transfers`, `startedUptimeMs`, `completedUptimeMs`, `completedTimeValid`, and
copied relevant identity/configuration/port/IRQ evidence with explicit validity.
Use a small fixed aggregate. Do not introduce a public polymorphic result tree.
`OperationProgress` contains token/kind/state, a coarse named phase, cumulative
`transfers`, `lastPollTransfers`, `deadlineMs`, `waiting`, `readyAtMs`,
`readyAtValid` and `resultPending`. Public phase labels describe reading, writing,
waiting or finishing; they do not freeze private switch phases or offsets.

| Example | API Status | Terminal outcome/effect | Other independent evidence |
| --- | --- | --- | --- |
| Admission while active | BUSY | Existing operation unchanged | Zero callbacks and counters unchanged |
| Take cancelled result | OK | CANCELLED / retained prior effect | Caller reads result.status == CANCELLED |
| Failure after termination-disable verified | I2C_ERROR from poll | FAILED / PARTIAL or INDETERMINATE according to failed phase | Termination DISABLED if still proved, otherwise UNKNOWN; configuration UNKNOWN |
| Backend timeout on role write | I2C_TIMEOUT | FAILED / INDETERMINATE | No reset/retry; later explicit reconciliation needed |
| Owner deadline after successful write | TIMEOUT | TIMED_OUT / PARTIAL | Callback transport success still counted; no late commit as success |
| Complete readback mismatch | CONFIGURATION_MISMATCH | FAILED / NONE for read-only verification | Transport successes increase; KNOWN actual image with complete required proof, otherwise UNKNOWN |
| Valid cable detach | OK | SUCCEEDED / NONE for port refresh | Snapshot valid; attachment detached; health unaffected by detach |
| Projected OFFLINE then explicit diagnostic read | Actual callback outcome | No health-based rejection | All-callback counters include it; raw traffic does not change managed projection |

## Time, work bounds and lifetimes

Use one owner-supplied `uint64_t` monotonic millisecond domain for admission,
poll, deadlines and snapshot times. Accept intervals strictly in `(0, 2^63)`;
use tested unsigned half-range comparisons, including natural uint64 rollover.
The owner's clock resolution is at most 1 ms; integer times may be floored.
Use a named `OWNER_CLOCK_MARGIN_MS = 1` for hardware delay scheduling, separate
from TI's specified interval and from owner deadline semantics. A TI 5 ms minimum
wait needs 6 elapsed integer ticks; a 95 ms reset-completion scheduling delay
needs 96; the base dead-battery recovery 30 ms wait needs 31. These are software
margins, not new TI timing facts. Test sub-ms
callback completion to prove the selected clock cannot shorten those intervals.
Owners extending a 32-bit hardware counter do so correctly in their adapter;
simply casting a wrapping timestamp is insufficient. Never renew the deadline
inside poll. Validate invalid time input without traffic or evidence loss.

The Status-only callback cannot prove completion time. Therefore after any
callback, stop that owner turn unless authoritative fresh completion time is
available through a separately justified contract. On the next fresh poll,
check deadline before committing a time-sensitive terminal success; arm a required
hardware wait from that fresh boundary. Thus `maxTransfers > 1` is a ceiling,
not a promise to consume it. With the basic callbacks the conservative engine
still uses at most one per call; do not invent elapsed time by dividing a budget.
No early wait expiry based on pre-callback nowMs. No sleep or busy wait.

A callback failure keeps its original typed error/detail as the primary terminal
outcome even if the next owner timestamp also shows an expired deadline. Deadline
expiry replaces a pending success with TIMEOUT; it must not erase an actual
transport failure. Preserve time/effect evidence independently of that outcome.

P04/P05 publish the exact maximum callback count and bytes per operation for
each variant from the verified sequence. Keep all state-transition loops bounded
by a small named limit. A complete logical write/read callback can contain a
pointer-write STOP and a read transaction; callback count is not physical bus
transaction count. The callback timeout is `min(i2cTimeoutMs, remainingMs)` with
safe narrowing and no zero-timeout attempt; it covers lock acquisition and every
phase once. Failed callbacks consume one. No retries inside core or adapter.

| Object/data | Owner and lifetime |
| --- | --- |
| DriverConfig/DeviceProfile arguments | Copied on accepted bind/start; no retained reference into caller storage. |
| i2cUser and physical bus | Borrowed application context remains valid until end returns; callback never owns it. A retained identity tag is never dereferenced after end. |
| Callback TX/RX arrays | Driver-owned fixed storage borrowed only until terminal callback return; no DMA/pending transfer may outlive that call. |
| OperationToken | Owner retains with its driver instance until result taken; it is not proof of identity across different instances. |
| Terminal result | Driver owns one retained copy until exact take; resultSnapshot copies it without release. end/invalidate preserve it. |
| Cached snapshots | Returned by value; old caller copies do not change as driver advances. Validity/provenance qualify every field. |
| Status.msg | Static lifetime, no formatter-owned buffer or heap string. |

Binding validates completely before commit. Active operation returns BUSY;
retained result returns RESULT_PENDING. Same-target rebinding preserves unresolved
effects. Define target association as the retained variant/address/context tuple;
the owner must notify physical replacement or context reuse. A different tuple
cannot silently discard unresolved effects: end, consume the result, and invoke
explicit forgetDeviceEvidence before changing target. This is a local evidence
contract, not hardware identity proof. No operation can act on the old bus after end.

## Neutral transport diagnostics and optional health projection

`TransportStats` uses the observed LDC1614 field names `totalAttempts`,
`totalSuccess`, `totalFailures`, `lastStatus`; counters are saturating uint32_t.
Count every invoked callback once, including raw diagnostics and initialization.
Increment attempt immediately before callback; increment exactly one success or
failure after normalizing terminal status. No local rejection, wait, detach,
configuration mismatch, cancellation or deadline-only expiry counts as I2C failure.
At saturation do not assume `attempts == successes + failures` still holds.

`DriverDiagnostics` provides bound/initialized flags, configuration/effect state,
last terminal operation status, last failed transport status with timestamp and
timestamp validity, and explicitly named `managedTransport` evidence. The latter
contains `initialized`, `totalSuccess`, `totalFailures`, `consecutiveFailures`,
`lastStatus` and last-success/failure time validity. Its callbacks are the filtered
production-operation subset. Explicit Sync diagnostics are in TransportStats but
excluded from managed projection; never hide them from the source totals.
The CLI probe alias calls startInitialize and follows managed initialization
accounting; a command alias never changes the underlying callback classification.
Do not add backend bytes/transactions when the callback cannot report them;
example adapters may expose their own separate, accurately named counters.

The optional helper is a pure projection:

```cpp
enum class DriverState : uint8_t { UNINIT, READY, DEGRADED, OFFLINE };
struct HealthPolicy { uint32_t offlineThreshold = 3; };
Status validateHealthPolicy(const HealthPolicy& policy); // Threshold >= 1.
Status projectHealth(const ManagedTransportEvidence& evidence,
                     const HealthPolicy& policy, HealthSnapshot& out);
```

Threshold three is the selected diagnostic default, not a TI requirement.
`HealthSnapshot` contains the projected `state`, copied managed evidence and the
effective threshold. The helper owns no clock, counters, retries or policy
actions. Invalid policy returns INVALID_CONFIG and leaves out unchanged.
Do not put health thresholds in silicon tables. A user may ignore this helper.

| Input/event | Evidence update | Projection |
| --- | --- | --- |
| Constructed, end, or bound without completed initialization | initialized false | UNINIT regardless of old failure totals |
| Failed initialization | Failed callback recorded; initialized stays false | UNINIT; diagnostics still show error |
| Successful read-only initialization | initialized true; last managed success sets streak zero | READY |
| Managed callback succeeds after initialization | Increment success; streak becomes zero | READY, including after OFFLINE |
| Managed callback fails, initialized true, streak below threshold | Increment failure and saturating streak | DEGRADED |
| Managed callback fails and streak reaches threshold | Increment failure and saturating streak | OFFLINE |
| Raw diagnostic callback | All-callback totals only | Unchanged |
| No callback; validation/cancel/wait/deadline expiry/detach | No transport evidence update | Unchanged |
| Successful transport but mismatch/unknown port encoding | Count transport success; preserve separate protocol/evidence status | READY when initialized, even though trust/data can be unknown |
| External device-state invalidation | initialized false; retain transport history and uncertainty | UNINIT |
| clearTransportStats | Clear all-callback cumulative totals and lastStatus only | Unchanged; never reset managed streak/trust/effects/results |
| HealthPolicy threshold changes | No device or callback action | Recompute from same evidence |

The streak is since the last managed successful callback, not since the last
successful whole operation. Last failed transport evidence remains available
after later success. Any explicit clearing of additional diagnostic history must
be separately named and must not change live evidence. OFFLINE never blocks
legal operations, starts recovery, changes role or invalidates a valid port merely
because of its name. Application scheduling uses its own decisions.

## Documentation and completion in every stage

For each changed public header, update file/type/member/enum Doxygen and each
function's parameters, units, return/status meanings, prerequisites, effects,
callback/CPU bounds, ownership and lifetime. Mention thread/ISR/reentry limits
once at class/transport scope and repeat exceptions where needed. For every
source file, keep a useful file brief and explain register masks, uncertain-write
handling, wait anchors and state invariants near the code that implements them.
Do not narrate obvious assignments or copy public comments onto every definition.

Update the relevant architecture/API/ownership/state/CLI/example guide with the
same change. Record tests and actual results in `docs/validation.md` and decisions
in `docs/design-decisions.md`. Run applicable documentation checks at the stage
where declarations/behavior appear. Stage 13 reconciles complete documentation;
it does not permit earlier stages to ship undocumented behavior.
