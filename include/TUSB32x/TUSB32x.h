/// @file TUSB32x.h
/// @brief Cooperative owner driver and fixed operation, observation and diagnostic values.
/// @details The facade borrows terminal application transport; passive aggregates perform no I2C. Field validity and provenance qualify copied historical evidence independently.
#pragma once

#include <cstdint>

#include "TUSB32x/Config.h"
#include "TUSB32x/Status.h"

namespace TUSB32x {

/// @brief Extra elapsed owner-clock millisecond tick used for TI minimum-wait quantization.
/// @details Separate library margin, not a TI duration or an extension to an operation deadline.
inline constexpr uint64_t OWNER_CLOCK_MARGIN_MS = 1;

/// @brief Instance-local operation correlation value; zero is invalid and issued values never repeat.
/// @details Nonzero alone does not prove currency or identity across different instances.
struct OperationToken {
  uint64_t value = 0; ///< Issued nonzero token; default represents no operation.

  /// @brief Inspect the stored token without callbacks.
  /// @return True exactly when value is nonzero; represented-work matching is a separate check.
  constexpr bool valid() const { return value != 0; }
};

/// @brief Admission timing in one application-owned uint64 monotonic millisecond domain.
/// @details Valid unsigned deadline-minus-now is in (0, 2^63); resolution is at most 1 ms.
/// Natural rollover is allowed. Callbacks provide no completion time; the owner must sample freshly
/// after a callback. Defaults do not describe a valid nonzero interval. No clock is owned here.
/// Active poll samples must advance by less than 2^63, including equal ticks and natural wrap.
/// A zero callback budget still permits deadline/wait transitions; it never invokes transport.
struct OperationTiming {
  uint64_t nowMs = 0;      ///< Owner admission timestamp in milliseconds.
  uint64_t deadlineMs = 0; ///< Immutable exclusive completion deadline in the same clock domain.
};

/// @brief Kind of one admitted operation; no implicit initialization, reset or replay exists.
enum class OperationKind : uint8_t {
  NONE,                   ///< No represented operation.
  INITIALIZE,             ///< Read-only identity and current configuration initialization.
  READ_PROFILE,           ///< Read actual profile without promoting trust from transport alone.
  APPLY_PROFILE,          ///< Explicitly apply changed requested settings and verify postconditions.
  VERIFY_PROFILE,         ///< Read-only proof against retained desired settings.
  RECONCILE,              ///< Read-only identity/configuration/effect observation; never replay writes.
  REFRESH_PORT,           ///< Fixed bounded port observation; no implied IRQ acknowledgement.
  ACKNOWLEDGE_INTERRUPT,  ///< Explicit variant-qualified IRQ acknowledgement.
  SERVICE_INTERRUPT,      ///< Bounded observation/acknowledgement/recheck operation.
  RESET,                  ///< Explicit reset command, wait and read-only completion proof.
  RECOVER_DEAD_BATTERY     ///< Explicit base-320/I qualified 0x45 recovery sequence and wait.
};

/// @brief Software lifecycle of one operation; hardware effects are independent.
enum class OperationState : uint8_t {
  IDLE,      ///< No active or retained terminal operation.
  ACTIVE,    ///< Admitted work requires owner advancement.
  SUCCEEDED, ///< Requested operation completed successfully.
  FAILED,    ///< Terminal transport, protocol or evidence failure.
  CANCELLED, ///< Terminal local cancellation; hardware effects may remain.
  TIMED_OUT  ///< Terminal owner deadline expiry; hardware effects may remain.
};

/// @brief Coarse public progress only; does not expose private engine offsets or switch states.
enum class OperationPhase : uint8_t {
  NONE,      ///< No current phase.
  READING,   ///< Obtaining identity, controls, status or verification bytes.
  WRITING,   ///< Preparing or performing an explicitly requested write.
  WAITING,   ///< Awaiting a required owner-clock interval without callbacks or blocking.
  FINISHING  ///< Completing status/evidence publication after bounded work.
};

/// @brief Current configuration trust, independent of desired settings and software completion.
enum class ConfigurationState : uint8_t {
  UNCONFIGURED, ///< No established target configuration record.
  APPLYING,     ///< A possible mutating write has begun for the active apply.
  KNOWN,        ///< Actual persistent controls and required identity/effect proof are established.
  UNKNOWN       ///< Current proof is missing or invalidated; historical records may remain.
};

/// @brief Effect classification of one operation; does not replace its original failure status.
enum class HardwareEffect : uint8_t {
  NONE,          ///< No data-changing write could have reached the target; includes read-only success.
  CONFIRMED,     ///< Every intended write and required final postcondition was verified.
  PARTIAL,       ///< At least one confirmed write, without a fully verified requested final state.
  INDETERMINATE  ///< At least one attempted write has unresolved acceptance; prior effects remain.
};

/// @brief Configured CC termination availability, not an electrical measurement or attachment proof.
enum class TerminationState : uint8_t {
  UNKNOWN, ///< Applicable controls or transition/reset evidence do not establish availability.
  ENABLED, ///< All applicable disable controls proved clear and no unresolved transition/reset.
  DISABLED ///< A documented controlling disable is proved effective.
};

/// @brief Retained reset-completion evidence; acceptance uncertainty is recorded independently.
enum class ResetState : uint8_t {
  NONE,           ///< No retained pending reset evidence.
  MAY_BE_PENDING, ///< Asserted command or uncertain completion requires retained settling proof.
  COMPLETED       ///< Settling interval and observed postconditions proved, not command acceptance.
};

/// @brief PortSnapshot raw-status validity bit for register 0x08.
inline constexpr uint8_t STATUS_08_VALID = 1U << 0;
/// @brief PortSnapshot raw-status validity bit for register 0x09.
inline constexpr uint8_t STATUS_09_VALID = 1U << 1;

/// @brief Independent decoded PortSnapshot validity bits; inapplicable is a valid interpretation.
/// @details RegisterDecode reuses only code-validity bits: single-byte decoding establishes
/// documented raw field meanings, without the paired context required by PortSnapshot.
enum class PortField : uint16_t {
  ATTACHMENT = 1U << 0,         ///< Attachment decoded from a documented successful observation.
  LOCAL_ROLE = 1U << 1,         ///< Local power role, including NOT_APPLICABLE, is established.
  PEER_ROLE = 1U << 2,          ///< Peer power role, including NOT_APPLICABLE, is established.
  ORIENTATION = 1U << 3,        ///< Direction, including NOT_APPLICABLE, is established.
  DETECTED_CURRENT = 1U << 4,   ///< Remote current interpretation and its applicability established.
  ADVERTISED_CURRENT = 1U << 5, ///< Local advertisement category decoded; not a power authorization.
  ACCESSORY = 1U << 6,          ///< Accessory interpretation and its applicability established.
  ACTIVE_CABLE = 1U << 7,       ///< Active-cable indication has an applicable documented context.
  INTERRUPT_PENDING = 1U << 8   ///< Sampled IRQ-latch level decoded; not acknowledgement proof.
};

/// @brief Copied port observation with separate raw, interpretation, age and trust evidence.
/// @details Fields are historical when invalidated; even current copies require per-field validity
/// and age/coherence checks. Publication follows 0x09, 0x08, 0x09 once, with no retry-until-stable.
/// Changed 0xE0 bookends clear dependent current/accessory/cable validity. No hardware atomicity.
/// Failed acquisition preserves the previous complete data/time. End, external invalidation and
/// possible port-affecting writes set invalidated; only complete fresh publication clears it.
/// Normal source/sink roles are opposite; detached/accessory roles are NOT_APPLICABLE.
/// Accessory direction and unresolved UFP-debug current stay UNKNOWN. Advertisement is a
/// local category, never measured load, a PD contract or permission to supply board power.
struct PortSnapshot {
  uint8_t rawStatus08 = 0; ///< Register 0x08 sample; unavailable until STATUS_08_VALID.
  uint8_t rawStatus09 = 0; ///< Final register 0x09 sample; unavailable until STATUS_09_VALID.
  uint8_t rawValidMask = 0; ///< STATUS_08_VALID/STATUS_09_VALID; zero means no raw samples.
  uint16_t validFields = 0; ///< PortField bits; unsupported/reserved/failed fields stay clear.
  bool invalidated = true; ///< Historical evidence cannot be used as a current port observation.
  Attachment attachment = Attachment::UNKNOWN; ///< Attachment, qualified by its field bit.
  PortRole localRole = PortRole::UNKNOWN; ///< Local role, separate from peer role.
  PortRole peerRole = PortRole::UNKNOWN; ///< Peer role; accessory does not imply a normal role.
  Orientation orientation = Orientation::UNKNOWN; ///< Chip CC1/CC2; detached N/A, accessory unknown.
  DetectedCurrent detectedCurrent = DetectedCurrent::UNKNOWN; ///< Remote indication, not load/PD.
  SourceCurrent advertisedCurrent = SourceCurrent::DEFAULT; ///< Invalid placeholder until flagged.
  Accessory accessory = Accessory::UNKNOWN; ///< Paired-observation accessory interpretation.
  bool activeCable = false; ///< Raw 08 bit0; usable only with ACTIVE_CABLE validity, not VCONN proof.
  bool interruptPending = false; ///< Sampled latch level only; not its value after an ACK.
  uint64_t sequence = 0; ///< Saturating publication count, initially zero; saturated counts repeat.
  uint64_t observedUptimeMs = 0; ///< Owner time before first required read; not publication time.
  bool observedTimeValid = false; ///< Acquisition-boundary timestamp is known in the owner domain.
  uint32_t configGeneration = 0; ///< Relevant verified configuration generation at acquisition.
  ConfigurationState configurationState = ConfigurationState::UNCONFIGURED; ///< Trust before first read; not later current trust.
  SnapshotCoherence coherence = SnapshotCoherence::SEQUENTIAL; ///< Bounded acquisition evidence.
};

/// @brief ConfigurationSnapshot raw-control validity bit for register 0x08.
inline constexpr uint8_t CONFIG_08_VALID = 1U << 0;
/// @brief ConfigurationSnapshot raw-control validity bit for register 0x09.
inline constexpr uint8_t CONFIG_09_VALID = 1U << 1;
/// @brief ConfigurationSnapshot raw-control validity bit for register 0x0A.
inline constexpr uint8_t CONFIG_0A_VALID = 1U << 2;
/// @brief ConfigurationSnapshot raw-control validity bit for register 0x45, where present.
inline constexpr uint8_t CONFIG_45_VALID = 1U << 3;

/// @brief Independent observed profile validity bits; capability determines applicable fields.
enum class ProfileField : uint16_t {
  PORT_MODE = 1U << 0,             ///< Documented semantic role mode decoded.
  SOURCE_CURRENT = 1U << 1,        ///< Documented CC advertisement code decoded.
  SOURCE_PREFERENCE = 1U << 2,     ///< Documented Try preference decoded.
  DRP_DUTY_CYCLE = 1U << 3,        ///< Documented DRP duty code decoded.
  CC_DEBOUNCE = 1U << 4,           ///< Exact-variant debounce setting decoded.
  DISABLE_UFP_ACCESSORY = 1U << 5, ///< Applicable UFP accessory-disable control decoded.
  DISABLE_TERM = 1U << 6,          ///< Applicable 0x0A termination/state-machine disable decoded.
  DISABLE_RD_RP = 1U << 7          ///< Applicable independent 0x45 Rd/Rp disable decoded.
};

/// @brief Desired, observed and historically verified profiles with independent validity and trust.
/// @details Default profiles are placeholders until their has-flags are set. Absent controls have no
/// observedValidFields bit and use the DeviceProfile neutral placeholder without absent-register I2C.
/// Reserved encodings retain raw bytes but invalidate the affected field. A complete raw transfer
/// alone cannot establish KNOWN. Failed/partial acquisitions preserve the previous complete image.
/// Desired and verified generations saturate; zero means no record and tokens identify operations.
/// Apply admission changes desired only. Full identity/control/effect proof can publish KNOWN actual
/// settings even on CONFIGURATION_MISMATCH; read-profile alone cannot promote trust. Result take
/// never adopts desired or erases interrupted effects. Historical verified values can remain valid
/// records while state is UNKNOWN; their generation does not assert current usability.
/// Termination can be proved independently of full profile trust. A newly observed contrary
/// disable bit invalidates that proof even if a later read fails before replacing this image.
struct ConfigurationSnapshot {
  bool hasDesiredProfile = false; ///< An accepted desired candidate exists; not actual device proof.
  DeviceProfile desiredProfile{}; ///< Last accepted desired settings; never an implied write.
  uint32_t desiredGeneration = 0; ///< Saturating accepted-desired-replacement count.
  bool hasObservedProfile = false; ///< Every applicable field of the complete observation decoded.
  DeviceProfile observedProfile{}; ///< Last complete decoded actual image; consult validity/trust.
  bool hasVerifiedProfile = false; ///< A verified historical profile exists; state qualifies currency.
  DeviceProfile verifiedProfile{}; ///< Proved actual image, which may differ from desired settings.
  uint32_t verifiedGeneration = 0; ///< Saturating complete-verified-commit count.
  ConfigurationState state = ConfigurationState::UNCONFIGURED; ///< Current proof state.
  TerminationState terminationState = TerminationState::UNKNOWN; ///< All-control availability proof.
  ResetState resetState = ResetState::NONE; ///< Completion evidence, not write-acceptance proof.
  bool mismatchValid = false; ///< The following persistent-bit mismatch record is meaningful.
  uint8_t mismatchRegister = 0; ///< Register containing a successfully read persistent mismatch.
  uint8_t mismatchExpected = 0; ///< Expected comparison image; meaningful under mismatchMask only.
  uint8_t mismatchObserved = 0; ///< Successful readback byte; meaningful under mismatchMask only.
  uint8_t mismatchMask = 0; ///< Persistent bits compared; status/commands do not imply mismatch.
  uint64_t observedUptimeMs = 0; ///< Owner time before the first required profile read, not publication.
  bool observedTimeValid = false; ///< observedUptimeMs is supported by owner time evidence.
  uint8_t rawConfig08 = 0; ///< Complete raw 0x08 sample, qualified by CONFIG_08_VALID.
  uint8_t rawConfig09 = 0; ///< Complete raw 0x09 sample, qualified by CONFIG_09_VALID.
  uint8_t rawConfig0A = 0; ///< Complete raw 0x0A sample, qualified by CONFIG_0A_VALID.
  uint8_t rawConfig45 = 0; ///< Complete raw 0x45 sample where present; CONFIG_45_VALID qualifies it.
  uint8_t rawConfigValidMask = 0; ///< CONFIG_*_VALID bits; absent/failed reads never supply validity.
  uint16_t observedValidFields = 0; ///< ProfileField bits; applicable reserved encodings stay invalid.
};

/// @brief Complete signature and separate revision evidence attributed to an explicit target choice.
/// @details A shared signature/revision never proves the exact board, enable polarity or selected
/// variant. Bytes are in ascending register order 0x00..0x07; partial failed reads are not valid.
struct IdentitySnapshot {
  Variant variant = Variant::UNSPECIFIED; ///< Configured exact-part expectation, not auto-detection.
  DeviceAddress address = DeviceAddress::ADDR_LOW_0X47; ///< Configured strap address attribution.
  uint8_t rawIdentity[8]{}; ///< Fixed complete signature bytes; unavailable until identityValid.
  bool identityValid = false; ///< All eight signature bytes were read successfully.
  bool identityMatches = false; ///< Complete valid signature matches the selected variant family.
  uint8_t rawRevision = 0; ///< Raw 0xA0 revision; absent on base 320/I and invalid until flagged.
  bool revisionValid = false; ///< Documented revision register was read successfully.
  bool revisionMatches = false; ///< Valid revision equals the exact selected variant expectation.
};

/// @brief Sampled IRQ and write-acceptance evidence, independent of attachment and event counting.
/// @details Coalescing latches do not guarantee lossless capture. A successful ACK callback does not
/// prove the latch stayed clear; only a valid post-write sample describes that later sampled level.
/// ACK-only publishes its fresh pre-write 09 sample; service publishes after complete 09/08/09.
/// Attempt/acceptance are recorded at the callback, while read publication requires fresh owner time.
/// Ordinary refresh reports only PortSnapshot.interruptPending, leaves its operation IRQ record
/// empty and preserves prior diagnostic IRQ history.
struct InterruptSnapshot {
  bool observationValid = false; ///< pendingBefore is a successfully decoded pre-ACK sample.
  bool pendingBefore = false; ///< Pre-ACK sampled latch level; invalid until observationValid.
  bool acknowledgeAttempted = false; ///< Terminal ACK write callback was invoked; effects may vary.
  bool acknowledgeConfirmed = false; ///< ACK write callback returned success; not latch-clear proof.
  bool pendingAfter = false; ///< Post-ACK sampled latch level; invalid until pendingAfterValid.
  bool pendingAfterValid = false; ///< Post-ACK recheck completed and decoded successfully.
};

/// @brief Copied terminal operation outcome and fixed evidence; no borrowed pointers into a driver.
/// @details Identity/configuration/port copies can include retained historical observations.
/// Their own validity/provenance applies. The interrupt member starts empty for each operation and
/// records only that operation's partial IRQ work, never an earlier operation's ACK acceptance.
/// Taking a result delivers this value; delivery does not change status or erase hardware effects.
/// A frozen result never gains a later timestamp. Callback failures and local cancellation/end lack
/// a completion sample; success/protocol failure/expiry on a fresh poll records that software time.
struct OperationResult {
  OperationToken token{}; ///< Instance-local correlation token; zero means no result.
  OperationKind kind = OperationKind::NONE; ///< Admitted operation kind.
  OperationState state = OperationState::IDLE; ///< Software lifecycle/outcome, independent of effects.
  Status status{}; ///< Original operation outcome, including full terminal backend detail.
  HardwareEffect hardwareEffect = HardwareEffect::NONE; ///< This operation's write-effect evidence.
  uint32_t transfers = 0; ///< Number of terminal callback attempts, not physical transactions.
  uint64_t startedUptimeMs = 0; ///< Accepted admission time in owner-domain milliseconds.
  uint64_t completedUptimeMs = 0; ///< Terminal time only when completedTimeValid is true.
  bool completedTimeValid = false; ///< Fresh terminal owner time exists; callback failure/cancel lacks it.
  IdentitySnapshot identity{}; ///< Copied attributed identity/revision evidence with its validity.
  ConfigurationSnapshot configuration{}; ///< Copied desired/actual/trust and effect evidence.
  PortSnapshot port{}; ///< Copied port observation, independently valid/invalidated/aged.
  InterruptSnapshot interrupt{}; ///< This operation's IRQ evidence; empty when it did no IRQ work.
};

/// @brief Passive copied progress; waiting never implies an internal sleep or a renewed deadline.
/// @details Terminal progress remains until exact take; then it is IDLE with no token. Rejected polls
/// preserve this value. waiting may be true before a fresh sample establishes readyAtValid.
struct OperationProgress {
  OperationToken token{}; ///< Represented instance-local token, or zero when no work/result exists.
  OperationKind kind = OperationKind::NONE; ///< Represented operation kind.
  OperationState state = OperationState::IDLE; ///< Active or retained terminal software state.
  OperationPhase phase = OperationPhase::NONE; ///< Coarse phase, not a private engine state.
  uint32_t transfers = 0; ///< Cumulative terminal callback attempts for this operation.
  uint8_t lastPollTransfers = 0; ///< Attempts in last accepted active poll; zero for its zero budget.
  uint64_t deadlineMs = 0; ///< Immutable admitted owner-domain deadline in milliseconds.
  bool waiting = false; ///< A required hardware interval is currently pending.
  uint64_t readyAtMs = 0; ///< Owner-domain earliest ready timestamp, only when readyAtValid.
  bool readyAtValid = false; ///< Fresh owner sample established readyAtMs; after the write, if any.
  bool resultPending = false; ///< Sole terminal slot is occupied and requires exact-token take.
};

/// @brief Neutral all-callback diagnostics; local rejections, waits and deadline expiry do not count.
/// @details Counters saturate at UINT32_MAX. At saturation attempts need not equal success+failures.
/// Explicit synchronous diagnostics count here as well as managed production callbacks.
struct TransportStats {
  uint32_t totalAttempts = 0; ///< Invoked callback count, incremented before each call.
  uint32_t totalSuccess = 0; ///< Complete normalized OK callback count.
  uint32_t totalFailures = 0; ///< Normalized terminal I2C_* callback failure count.
  Status lastStatus{}; ///< Last normalized callback outcome; default OK means no retained attempt.
};

/// @brief Filtered production-operation transport history for optional passive health projection.
/// @details Explicit raw diagnostics are excluded here but counted in TransportStats. Counters and
/// failure streak saturate. Successful transport resets the streak without repairing configuration.
/// Basic Status-only callbacks supply no completion timestamp, so callback-time validity remains
/// false. Clearing TransportStats does not clear this history, live streak or initialized readiness.
struct ManagedTransportEvidence {
  bool initialized = false; ///< Matching identity plus complete raw profile acquired; not KNOWN trust.
  uint32_t totalSuccess = 0; ///< Saturating managed successful-callback count.
  uint32_t totalFailures = 0; ///< Saturating managed failed-callback count.
  uint32_t consecutiveFailures = 0; ///< Saturating failures since the last managed successful callback.
  Status lastStatus{}; ///< Last normalized managed callback status; no operation-status substitution.
  uint64_t lastSuccessUptimeMs = 0; ///< Owner-domain time of last managed success, when valid.
  bool lastSuccessTimeValid = false; ///< Fresh completion time supports lastSuccessUptimeMs.
  uint64_t lastFailureUptimeMs = 0; ///< Owner-domain time of last managed failure, when valid.
  bool lastFailureTimeValid = false; ///< Fresh completion time supports lastFailureUptimeMs.
};

/// @brief Passive health labels; no value blocks legal operations or starts recovery.
enum class DriverState : uint8_t {
  UNINIT,   ///< Current initialization proof is absent, regardless of historical counters.
  READY,    ///< Initialized and no current managed failure streak.
  DEGRADED, ///< Initialized with a positive managed failure streak below the chosen threshold.
  OFFLINE   ///< Initialized with a managed failure streak at or above the chosen threshold.
};

/// @brief Application-selected diagnostic threshold; separate from silicon settings and scheduling.
struct HealthPolicy {
  uint32_t offlineThreshold = 3; ///< Must be at least one; selected default is not a TI requirement.
};

/// @brief Copied optional health projection; owns no clock, counters, retries or recovery policy.
struct HealthSnapshot {
  DriverState state = DriverState::UNINIT; ///< Passive projected state; not a transport error.
  ManagedTransportEvidence evidence{}; ///< Copied managed evidence used for this projection.
  uint32_t offlineThreshold = 3; ///< Effective positive threshold used for this projection.
};

/// @brief Validate an application-selected passive display threshold without I2C or allocation.
/// @param[in] policy Borrowed value; offlineThreshold must be at least one, including UINT32_MAX.
/// @return OK for a positive threshold, INVALID_CONFIG for zero; no mutation or retained storage.
Status validateHealthPolicy(const HealthPolicy& policy);

/// @brief Project copied managed transport evidence without I2C, clock access or policy actions.
/// @param[in] evidence Borrowed history; initialization and the managed failure streak determine state.
/// @param[in] policy Positive offlineThreshold; changing it only changes this display projection.
/// @param[out] out Replaced with state, evidence and threshold on OK; unchanged on INVALID_CONFIG.
/// @return INVALID_CONFIG for threshold zero, otherwise OK. UNINIT overrides any failure history;
/// initialized streak zero is READY, below threshold DEGRADED, otherwise OFFLINE. Fixed work,
/// no allocation, retries, recovery or admission decisions; READY does not establish configuration
/// trust or port validity. All returned messages have static lifetime and detail zero.
Status projectHealth(const ManagedTransportEvidence& evidence, const HealthPolicy& policy,
                     HealthSnapshot& out);

/// @brief Independent binding, trust, effect and diagnostic history retained by an owner instance.
/// @details Latest-operation fields never erase unresolved recovery, mode-transition or reset
/// acceptance evidence. Taking a result or clearing transport totals cannot repair those facts.
/// configurationWriteUnresolved also retains ordinary settings/raw-write uncertainty across later
/// read-only results. Only complete current proof or explicit local forgetting clears that obligation.
/// hardwareEffect may describe a later raw write; lastOperationKind/status remain managed history.
struct DriverDiagnostics {
  bool bound = false; ///< A valid transport/device binding is current.
  bool initialized = false; ///< Current matching identity and complete raw profile, independent of trust.
  ConfigurationState configurationState = ConfigurationState::UNCONFIGURED; ///< Current trust.
  HardwareEffect hardwareEffect = HardwareEffect::NONE; ///< Latest managed terminal or raw-write effect.
  TerminationState terminationState = TerminationState::UNKNOWN; ///< Retained all-control proof.
  ResetState resetState = ResetState::NONE; ///< Retained reset settling/postcondition proof.
  bool configurationWriteUnresolved = false; ///< Possible settings change lacks complete current proof.
  bool recoverySequenceUnresolved = false; ///< Base dead-battery sequence completion remains unproved.
  bool modeTransitionUnresolved = false; ///< Required mode settling/current-control proof is unresolved.
  bool resetAcceptanceUncertain = false; ///< Reset-command acceptance remains ambiguous, even if settled.
  OperationKind lastOperationKind = OperationKind::NONE; ///< Latest terminal operation's kind.
  Status lastOperationStatus{}; ///< Latest terminal operation's original status and backend detail.
  Status lastFailedTransportStatus{}; ///< Last normalized failed callback; OK before any retained failure.
  uint64_t lastFailedTransportUptimeMs = 0; ///< Owner-domain failure completion time, if valid.
  bool lastFailedTransportTimeValid = false; ///< Fresh owner time supports the last failed callback.
  ManagedTransportEvidence managedTransport{}; ///< Explicitly filtered production callback history.
  IdentitySnapshot identity{}; ///< Copied identity/revision evidence, available after result take.
  InterruptSnapshot interrupt{}; ///< Last IRQ operation's historical evidence, not current latch state.
};


/// @brief Serialized application-owned cooperative driver; no allocation or implicit I2C.
/// @details Poll advances managed transport; explicit Sync diagnostics invoke at most one callback.
/// Borrowed context must live until end returns. No ISR use,
/// concurrent access or callback reentry. One active token becomes one immutable retained result.
/// Apply changes requested settings; reset, base recovery and IRQ ACK/service are explicit writes.
/// Initialization, profile/port observation, verification and reconciliation perform reads only.
/// Cached copies remain historical unless their validity and current trust allow use.
/// This object cannot be copied/moved.
class TUSB32x {
 public:
  /// @brief Construct empty unbound evidence without callbacks.
  TUSB32x() = default;
  /// @brief Release locally without callbacks; consume results before destruction if needed.
  ~TUSB32x() = default;
  /// @brief Copying is forbidden: tokens, borrowed binding and evidence have one owner.
  TUSB32x(const TUSB32x&) = delete;
  /// @brief Copy assignment is forbidden to preserve instance-local operation ownership.
  TUSB32x& operator=(const TUSB32x&) = delete;
  /// @brief Moving is forbidden: active work and retained tokens stay with this instance.
  TUSB32x(TUSB32x&&) = delete;
  /// @brief Move assignment is forbidden to preserve binding and result lifetimes.
  TUSB32x& operator=(TUSB32x&&) = delete;
  /// @brief Validate then atomically copy a binding; zero callbacks and fixed work.
  /// @param[in] config Borrowed callbacks/context and copied exact variant/address/timeout.
  /// @return INVALID_CONFIG from validation, then BUSY, RESULT_PENDING or
  /// CONFIGURATION_UNKNOWN for unresolved target replacement; OK otherwise. Errors change nothing.
  Status bind(const DriverConfig& config);
  /// @brief Cancel locally, unbind and invalidate usability; retain result/effects, zero callbacks.
  /// @return OK even when already unbound; active result becomes CANCELLED with unknown time.
  Status end();
  /// @brief Notify external change, preserving history and binding; no callbacks.
  /// @param[in] reason Static-message cause; OK normalizes to CONFIGURATION_UNKNOWN. Active result
  /// has CANCELLED with this message/detail; the cause code remains caller-owned.
  /// @return INVALID_PARAM for IN_PROGRESS/unknown code without changes; otherwise OK.
  Status invalidateDeviceState(const Status& reason);
  /// @brief Discard target evidence locally; tokens and instance transport history remain.
  /// @return BUSY, RESULT_PENDING, INVALID_CONFIG if bound, or OK, in that order. Zero callbacks.
  Status forgetDeviceEvidence();
  /// @brief Admit read-only identity/configuration observation, without applying defaults.
  /// @param[in] timing Owner milliseconds with unsigned deadline-now strictly in (0,2^63).
  /// @param[out] token New nonzero instance token on IN_PROGRESS; unchanged on rejection.
  /// @return INVALID_PARAM, NOT_BOUND, BUSY, RESULT_PENDING, TOKEN_EXHAUSTED in that order,
  /// or IN_PROGRESS. Admission clears initialization usability; no callbacks or writes.
  /// @details Poll reads identity, revision where present and each applicable profile register
  /// once: five callbacks on TUSB320/TUSB320I/TUSB322I, six on LI/HI/LAI/HAI. Complete
  /// matching identity and raw profile establish initialization; reserved fields or pending
  /// reset/mode/recovery proof return CONFIGURATION_UNKNOWN and prevent KNOWN configuration.
  /// Successful observations publish only after fresh owner time. No desired profile is created
  /// or applied.
  Status startInitialize(const OperationTiming& timing, OperationToken& token);
  /// @brief Admit one persistent-profile observation; no desired adoption or trust promotion.
  /// @param[in] timing Owner milliseconds with unsigned deadline-now strictly in (0,2^63).
  /// @param[out] token New token on IN_PROGRESS; unchanged on rejection.
  /// @return INVALID_PARAM, NOT_BOUND, BUSY, RESULT_PENDING, TOKEN_EXHAUSTED in order,
  /// or IN_PROGRESS. Admission is bus-silent and preserves current evidence.
  /// @note Uses four scalar reads on 320 variants, three on 322I. Complete raw/decoded data
  /// publishes after fresh time; partial failure preserves the previous image. Contradictory
  /// controls invalidate trust immediately, without erasing identity or historical evidence.
  /// Complete reserved controls return CONFIGURATION_UNKNOWN; otherwise observation returns OK.
  Status startReadProfile(const OperationTiming& timing, OperationToken& token);
  /// @brief Admit an explicit changed-field apply followed by complete read-only proof.
  /// @param[in] profile Copied validated desired settings; board power policy remains external.
  /// @param[in] timing Immutable owner milliseconds; deadline-now must be in (0,2^63).
  /// @param[out] token New nonzero token on IN_PROGRESS; unchanged on rejection.
  /// @return Timing/profile errors, NOT_BOUND, BUSY, RESULT_PENDING, NOT_INITIALIZED,
  /// CONFIGURATION_UNKNOWN, changed-field UNSUPPORTED_FEATURE, TOKEN_EXHAUSTED, or IN_PROGRESS.
  /// @details Zero callbacks at admission. Requires known actual controls and resolved current
  /// effects. Accepted desired generation advances even if cancelled before writing; old actual
  /// proof survives until a possible write. Base mode needs a later detached precheck and an
  /// external owner/fixture preventing the attachment race. LI/HI/LAI/HAI mode waits six ticks
  /// from a fresh sample after the mode-write callback; 322I mode has no added delay.
  /// At most 10 base, 12 LI/HI/LAI/HAI or 8 322I callbacks including proof.
  /// Failed writes retain uncertain effects and original errors; no rollback, retry or replay.
  /// An observed equivalent 322I DRP recoding keeps semantic trust but revokes the old 0A write
  /// image. Changed 0A fields require complete proof again; a no-op apply can obtain that proof.
  Status startApplyProfile(const DeviceProfile& profile, const OperationTiming& timing,
                           OperationToken& token);
  /// @brief Admit complete read-only proof against retained desired settings.
  /// @param[in] timing Immutable owner milliseconds with interval in (0,2^63).
  /// @param[out] token New nonzero token on IN_PROGRESS; unchanged on rejection.
  /// @return INVALID_PARAM, NOT_BOUND, BUSY, RESULT_PENDING, CONFIGURATION_UNKNOWN when no
  /// desired exists, TOKEN_EXHAUSTED, or IN_PROGRESS, in that order. Admission is bus-silent.
  /// @details No current identity/trust prerequisite. Shares reconciliation's bounded wait and
  /// identity/profile proof: five base/322I or six LI/HI/LAI/HAI reads, no writes. Complete
  /// differing actual controls become KNOWN with CONFIGURATION_MISMATCH; desired and earlier
  /// results remain unchanged.
  Status startVerifyProfile(const OperationTiming& timing, OperationToken& token);
  /// @brief Admit read-only identity/control/effect proof, available without initialization.
  /// @param[in] timing Immutable owner millisecond interval strictly in (0,2^63).
  /// @param[out] token New token on IN_PROGRESS; unchanged on rejection.
  /// @return INVALID_PARAM, NOT_BOUND, BUSY, RESULT_PENDING, TOKEN_EXHAUSTED in order,
  /// or IN_PROGRESS. No callbacks at admission and no desired-profile replacement.
  /// @note At most 5 base/322 or 6 extended callbacks. Retained pending reset first requires
  /// 96 ticks from the first fresh poll; interrupted LI/HI/LAI/HAI mode requires 6 ticks. The
  /// deadline is never extended. Newly discovered reset remains unknown without another pass.
  /// Full proof may establish KNOWN actual with CONFIGURATION_MISMATCH against retained desired;
  /// unresolved proof returns CONFIGURATION_UNKNOWN. Old failed results are never rewritten.
  Status startReconcile(const OperationTiming& timing, OperationToken& token);
  /// @brief Admit one read-only port observation; admission performs no callbacks.
  /// @param[in] timing Owner millisecond admission time and immutable deadline.
  /// @param[out] token New token on IN_PROGRESS; unchanged on rejection.
  /// @return IN_PROGRESS or normal timing/binding/slot errors; initialization is not required.
  /// @details Poll reads 09/08/09 once (three callbacks, three TX and three RX bytes).
  /// Publish only after the final fresh owner-time check. Failed/incomplete work preserves
  /// the old snapshot. Matching E0 bookends do not prove atomicity; changed bookends clear
  /// dependent field validity. Acquisition time/configuration provenance precede the first read.
  /// No ACK/reset, GPIO or power policy. Contradictory controls can invalidate current
  /// configuration trust without replacing the historical port observation.
  Status startRefreshPort(const OperationTiming& timing, OperationToken& token);
  /// @brief Admit one explicit IRQ acknowledgement using fresh actual controls; no admission I2C.
  /// @param[in] timing Owner admission time and immutable deadline in milliseconds.
  /// @param[out] token New token on IN_PROGRESS; unchanged on rejection.
  /// @return IN_PROGRESS or normal admission errors; requires current validated identity.
  /// TUSB322I returns UNSUPPORTED_FEATURE before traffic while fault/write semantics are gated.
  /// @details UNKNOWN configuration is allowed. Poll reads 09 and writes a safely composed
  /// W1C byte once: at most two callbacks, three TX bytes and one RX byte. Desired settings
  /// are ignored. No port refresh, recheck or automatic retry; the historical port remains.
  /// Callback acceptance is distinct from sampled latch state. Failed writes may have cleared
  /// IRQ; cancellation/end never undo effects. Contradictory reads can revoke configuration trust.
  Status startAcknowledgeInterrupt(const OperationTiming& timing, OperationToken& token);
  /// @brief Admit one bounded port observation, IRQ ACK and sampled recheck; no admission I2C.
  /// @param[in] timing Owner admission time and immutable deadline in milliseconds.
  /// @param[out] token New token on IN_PROGRESS; unchanged on rejection.
  /// @return IN_PROGRESS or the same identity/variant/admission errors as ACK-only.
  /// @details Poll uses 09/08/09, publishes the complete port, writes ACK, then reads 09 once:
  /// at most five callbacks, six TX bytes and four RX bytes. Fresh owner time qualifies
  /// each pending completion. Complete pre-ACK data survives a later failure/cancel/expiry.
  /// Recheck updates only IRQ evidence; pendingAfter=true can accompany successful service.
  /// No GPIO/ISR, event count, lossless queue or automatic follow-up operation is provided.
  Status startServiceInterrupt(const OperationTiming& timing, OperationToken& token);
  /// @brief Admit one explicit reset; admission copies timing and performs no I2C.
  /// @param timing Immutable owner deadline and admission time in milliseconds.
  /// @param token Receives a new token only on IN_PROGRESS; preserved on rejection.
  /// @return Normal admission errors, including NOT_INITIALIZED without current identity.
  /// @details UNKNOWN configuration is allowed: poll first observes actual 0x0A controls.
  /// One composed reset write is followed by 96 elapsed owner ticks and full identity/control
  /// proof, bounded to 7/8/7 callbacks for base/extended-320/322I. Desired settings are never
  /// replayed. Reset may resample PORT; 322I must return to DRP. Completion does not prove
  /// attachment readiness or clear historical ambiguous acceptance/recovery evidence.
  Status startReset(const OperationTiming& timing, OperationToken& token);
  /// @brief Admit the original TUSB320/TUSB320I dead-battery recovery only; no admission I2C.
  /// @param timing Immutable owner deadline and admission time in milliseconds.
  /// @param token Receives a new token only on IN_PROGRESS; preserved on rejection.
  /// @return UNSUPPORTED_FEATURE for other bound variants, NOT_INITIALIZED without identity,
  /// or CONFIGURATION_UNKNOWN for a pending mode/reset transition; normal slot errors apply.
  /// @details Caller establishes stable VDD/I2C readiness and decides source startup needs
  /// recovery, including VDD loss without a cable. Poll checks internal 0x45 bits are zero,
  /// writes literal 0x04, waits 31 elapsed ticks from fresh completion evidence, writes 0x00
  /// and verifies low three bits. Four callbacks, six TX/two RX bytes excluding address phases.
  /// No cleanup/replay or transient port publication occurs. Success proves this sequence and
  /// termination, not a full profile; interrupted history survives result take/end/rebind.
  Status startRecoverDeadBattery(const OperationTiming& timing, OperationToken& token);
  /// @brief Advance one bounded owner turn; no sleep, cleanup or retry.
  /// @param[in] token Exact represented token; zero is INVALID_PARAM, absent is RESULT_NOT_AVAILABLE,
  /// different is TOKEN_MISMATCH, checked before time. Terminal calls return frozen status unchanged.
  /// @param[in] nowMs Fresh owner sample after preceding callback, same uint64 millisecond domain.
  /// Backward/half-range deltas reject without changes; equal fresh ticks are valid.
  /// @param[in] maxTransfers Callback ceiling; zero permits local transitions only. Basic callbacks
  /// always stop the turn after one invocation, even with a larger ceiling. Failed attempts count.
  /// @return IN_PROGRESS or terminal outcome. Deadline expires before new traffic/pending success;
  /// callback failure stays primary with unknown time. Initialize/reconcile use at most 5 base/322
  /// or 6 extended callbacks; profile reads use 4 base/extended or 3 on 322. Apply's bounds
  /// include its explicit writes; successful writes remain PARTIAL until final proof succeeds.
  /// Timeout includes lock/all phases and clips to remaining time.
  Status poll(OperationToken token, uint64_t nowMs, uint8_t maxTransfers = 1);
  /// @brief Cancel matching active work locally; no rollback, I2C or fabricated completion time.
  /// @param[in] token Exact token with the same zero/absent/mismatch precedence as poll.
  /// @return OK for matching active/terminal work; active result has CANCELLED, terminal unchanged.
  Status cancel(OperationToken token);
  /// @brief Copy the immutable retained result without consuming it; zero callbacks.
  /// @param[in] token Exact token; zero/absent/mismatch precedence as poll; active is unavailable.
  /// @param[out] out Copy on OK, otherwise unchanged.
  /// @return OK for delivery even when out.status failed; otherwise token/unavailable error.
  Status resultSnapshot(OperationToken token, OperationResult& out) const;
  /// @brief Deliver and consume the exact retained result; no callbacks or evidence repair.
  /// @param[in] token Exact token with the same precedence as resultSnapshot.
  /// @param[out] out Copy on OK, otherwise unchanged; repeated take is RESULT_NOT_AVAILABLE.
  /// @return OK on delivery regardless of operation outcome, otherwise token/unavailable error.
  Status takeResult(OperationToken token, OperationResult& out);
  /// @brief Read one mapped byte using one terminal callback, without managed publication.
  /// @param[in] reg Scalar 00..0A, present45 or presentA0; no holes, ranges or pointer reuse.
  /// @param[out] out Replaced only on callback OK; unchanged on every local/transport error.
  /// @return INVALID_PARAM for unmapped offsets, UNSUPPORTED_FEATURE for absent registers, then
  /// NOT_BOUND, BUSY or RESULT_PENDING. UNKNOWN configuration is allowed. Callback errors retain
  /// normalized I2C_* and full detail. One TX pointer/one RX byte, configured aggregate timeout in
  /// milliseconds, no retry or time sampling. Raw counts exclude managed health history.
  /// @note Adverse control evidence can invalidate trust/port; reset1 records MAY_BE_PENDING with
  /// no completion anchor. Raw reads never publish identity/profile/port/IRQ or repair trust.
  /// TUSB322I09 fault-consuming semantics are unresolved; raw access is not declared nondestructive.
  Status readRegisterSync(uint8_t reg, uint8_t& out);
  /// @brief Write one qualified raw image with one terminal callback and no preparatory read.
  /// @param[in] reg Writable 08/09/0A/present45; RO/unmapped/absent/gated targets reject locally.
  /// @param[in] value Complete legal byte, already preserving proved controls; never silently masked.
  /// @return INVALID_PARAM for illegal images/unmapped offsets, UNSUPPORTED_FEATURE for RO/absent/
  /// gated access or sequence shortcuts, then NOT_BOUND/BUSY/RESULT_PENDING/NOT_INITIALIZED or
  /// CONFIGURATION_UNKNOWN for missing current safe proof. A changed mode, reset, termination or
  /// recovery requires its typed operation. TUSB322I08/09 writes remain gated even when unchanged.
  /// @details Requires KNOWN actual controls, current identity and relevant raw-image proof. Uses
  /// two TX bytes and zero RX with the configured aggregate millisecond timeout. No retries or
  /// clock access. Ordinary writes, including same-value writes, invalidate trust/port and retain
  /// configuration-write uncertainty before invocation; success is PARTIAL, errors INDETERMINATE.
  /// Pure IRQ ACK preserves actual09 controls and trust/port; success confirms acceptance only.
  /// Equivalent 322I DRP00/11 observations cannot authorize the older literal 0A image;
  /// complete managed proof must renew it before a raw 0A write.
  /// Raw callbacks count in source totals, never managed history. Managed last-operation and IRQ
  /// records are unchanged. Callback failure detail is preserved; failure time remains unknown.
  Status writeRegisterSync(uint8_t reg, uint8_t value);
  /// @brief Copy cached progress with fixed work and zero callbacks.
  /// @return Current active/terminal slot, or IDLE after take.
  OperationProgress progress() const { return _progress; }
  /// @brief Copy historical port evidence without callbacks or age sampling.
  /// @return Last complete observation; inspect validity, coherence, invalidation and age.
  PortSnapshot portSnapshot() const { return _port; }
  /// @brief Copy desired/observed/verified configuration without callbacks.
  /// @return Historical values qualified by field flags and current trust.
  ConfigurationSnapshot configurationSnapshot() const { return _configuration; }
  /// @brief Copy saturating all-callback totals without callbacks.
  /// @return Instance transport history; not physical transaction counts.
  TransportStats transportStats() const { return _stats; }
  /// @brief Copy cached lifecycle, identity and retained effect evidence without callbacks.
  /// @return Independent target facts and instance transport/operation history.
  DriverDiagnostics diagnostics() const { return _diagnostics; }
  /// @brief Clear only all-callback totals/lastStatus; no callbacks or managed-streak/effect changes.
  void clearTransportStats() { _stats = {}; }
 private:
  enum class Phase : uint8_t {
    WAIT, BASE_PRECHECK, APPLY_WRITE, APPLY_WAIT,
    RESET_PRECHECK, RESET_WRITE, RECOVERY_PRECHECK, RECOVERY_DISABLE,
    RECOVERY_ENABLE, RECOVERY_VERIFY, PROCEDURE_WAIT,
    PORT_FIRST09, PORT08, PORT_LAST09,
    IRQ_PRECHECK, IRQ_ACK, IRQ_RECHECK,
    IDENTITY, REVISION, CONFIG08, CONFIG09, CONFIG0A, CONFIG45, DONE
  };
  /// @brief One composed write in a fixed profile plan of at most six entries.
  struct ProfileWrite {
    uint8_t reg = 0; ///< Exact scalar register pointer.
    uint8_t value = 0; ///< Qualified byte preserving preceding actual/planned controls.
    bool modeStep = false; ///< Attempt starts or continues a mode-proof obligation.
    bool waitAfter = false; ///< Completed mode write requires the extended 320 minimum wait.
  };
  ProfileWrite _writes[6]{};
  uint8_t _writeCount = 0;
  uint8_t _writeIndex = 0;
  uint8_t _commandByte = 0;
  DriverConfig _config{};
  bool _hasAssociation = false;
  uint64_t _lastToken = 0;
  uint64_t _lastTime = 0;
  OperationProgress _progress{};
  OperationResult _result{};
  ConfigurationSnapshot _configuration{};
  ConfigurationSnapshot _observing{};
  PortSnapshot _port{};
  PortSnapshot _observingPort{}; ///< Private first09/08 staging and pre-read acquisition provenance.
  TransportStats _stats{};
  DriverDiagnostics _diagnostics{};
  /// @brief Protected45 bits are outside profile equality; only full proof renews its write image.
  bool _rdRpWriteImageValid = false;
  /// @brief Literal mode bits need full proof even when 322I DRP00/11 compare equivalent.
  bool _modeWriteImageValid = false;
  Phase _phase = Phase::IDENTITY;
  bool _waitReset = false;
  bool _waitMode = false;
  bool _pending = false;
  uint8_t _received[8]{};
  Status _pendingStatus{};
  Status checkToken(OperationToken token) const;
  Status startOperation(OperationKind kind, const OperationTiming& timing, OperationToken& token,
                          const DeviceProfile* candidate = nullptr);
  Status planProfile(const DeviceProfile& candidate, ProfileWrite (&writes)[6],
                     uint8_t& count, bool& baseMode) const;
  void beginApplyProof();
  Status finish(Status status, bool timeValid = false, uint64_t nowMs = 0);
  void invalidateConfiguration();
  void invalidateUsability();
  void clearTarget();
  bool unresolved() const;
  void publishIdentityRead();
  void acceptRead();
  Status publishConfiguration();
  Status compareObservedProfile(const DeviceProfile& expected);
  void observeControl(uint8_t reg, uint8_t value);
  /// @brief Publish fresh pre-ACK evidence and compose 09 from actual controls, never desired.
  /// @param[in] actual09 Complete current pre-ACK sample; caller has passed the fresh-time check.
  /// @return Composer status; successful composition selects the single ACK write phase.
  Status prepareInterruptAck(uint8_t actual09);
};

}  // namespace TUSB32x
