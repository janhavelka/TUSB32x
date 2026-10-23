/// @file TUSB32x.h
/// @brief Passive operation, observation and diagnostic values for the planned owner-driven driver.
/// @details These fixed aggregates perform no I2C and contain no driver facade or callable driver
/// declarations. Field validity and provenance qualify copied historical evidence independently.
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
struct PortSnapshot {
  uint8_t rawStatus08 = 0; ///< Register 0x08 sample; unavailable until STATUS_08_VALID.
  uint8_t rawStatus09 = 0; ///< Final register 0x09 sample; unavailable until STATUS_09_VALID.
  uint8_t rawValidMask = 0; ///< STATUS_08_VALID/STATUS_09_VALID; zero means no raw samples.
  uint16_t validFields = 0; ///< PortField bits; unsupported/reserved/failed fields stay clear.
  bool invalidated = true; ///< Historical evidence cannot be used as a current port observation.
  Attachment attachment = Attachment::UNKNOWN; ///< Attachment, qualified by its field bit.
  PortRole localRole = PortRole::UNKNOWN; ///< Local role, separate from peer role.
  PortRole peerRole = PortRole::UNKNOWN; ///< Peer role; accessory does not imply a normal role.
  Orientation orientation = Orientation::UNKNOWN; ///< Direction; detached can be NOT_APPLICABLE.
  DetectedCurrent detectedCurrent = DetectedCurrent::UNKNOWN; ///< Remote indication, not load/PD.
  SourceCurrent advertisedCurrent = SourceCurrent::DEFAULT; ///< Invalid placeholder until flagged.
  Accessory accessory = Accessory::UNKNOWN; ///< Paired-observation accessory interpretation.
  bool activeCable = false; ///< Unusable placeholder until ACTIVE_CABLE validity is set.
  bool interruptPending = false; ///< Sampled latch level only; not its value after an ACK.
  uint64_t sequence = 0; ///< Saturating publication count; zero means no published observation.
  uint64_t observedUptimeMs = 0; ///< Owner time before first required read; not publication time.
  bool observedTimeValid = false; ///< Acquisition-boundary timestamp is known in the owner domain.
  uint32_t configGeneration = 0; ///< Relevant verified configuration generation at acquisition.
  ConfigurationState configurationState = ConfigurationState::UNCONFIGURED; ///< Trust provenance.
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
  uint64_t observedUptimeMs = 0; ///< Owner-domain observation timestamp in milliseconds.
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
struct OperationResult {
  OperationToken token{}; ///< Instance-local correlation token; zero means no result.
  OperationKind kind = OperationKind::NONE; ///< Admitted operation kind.
  OperationState state = OperationState::IDLE; ///< Software lifecycle/outcome, independent of effects.
  Status status{}; ///< Original operation outcome, including full terminal backend detail.
  HardwareEffect hardwareEffect = HardwareEffect::NONE; ///< This operation's write-effect evidence.
  uint32_t transfers = 0; ///< Number of terminal callback attempts, not physical transactions.
  uint64_t startedUptimeMs = 0; ///< Accepted admission time in owner-domain milliseconds.
  uint64_t completedUptimeMs = 0; ///< Terminal time only when completedTimeValid is true.
  bool completedTimeValid = false; ///< Fresh terminal owner time is available; cancel/end may lack it.
  IdentitySnapshot identity{}; ///< Copied attributed identity/revision evidence with its validity.
  ConfigurationSnapshot configuration{}; ///< Copied desired/actual/trust and effect evidence.
  PortSnapshot port{}; ///< Copied port observation, independently valid/invalidated/aged.
  InterruptSnapshot interrupt{}; ///< This operation's IRQ evidence; empty when it did no IRQ work.
};

/// @brief Passive copied progress; waiting never implies an internal sleep or a renewed deadline.
struct OperationProgress {
  OperationToken token{}; ///< Represented instance-local token, or zero when no work/result exists.
  OperationKind kind = OperationKind::NONE; ///< Represented operation kind.
  OperationState state = OperationState::IDLE; ///< Active or retained terminal software state.
  OperationPhase phase = OperationPhase::NONE; ///< Coarse phase, not a private engine state.
  uint32_t transfers = 0; ///< Cumulative terminal callback attempts for this operation.
  uint8_t lastPollTransfers = 0; ///< Callback attempts during the last poll; zero for a zero budget.
  uint64_t deadlineMs = 0; ///< Immutable admitted owner-domain deadline in milliseconds.
  bool waiting = false; ///< A required hardware interval is currently pending.
  uint64_t readyAtMs = 0; ///< Owner-domain earliest ready timestamp, only when readyAtValid.
  bool readyAtValid = false; ///< A fresh post-callback boundary has established readyAtMs.
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
struct ManagedTransportEvidence {
  bool initialized = false; ///< Complete read-only initialization proof is current for this target.
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

/// @brief Independent binding, trust, effect and diagnostic history retained by an owner instance.
/// @details Latest-operation fields never erase unresolved recovery, mode-transition or reset
/// acceptance evidence. Taking a result or clearing transport totals cannot repair those facts.
struct DriverDiagnostics {
  bool bound = false; ///< A valid transport/device binding is current.
  bool initialized = false; ///< Current complete read-only initialization proof is available.
  ConfigurationState configurationState = ConfigurationState::UNCONFIGURED; ///< Current trust.
  HardwareEffect hardwareEffect = HardwareEffect::NONE; ///< Latest terminal operation's effects.
  TerminationState terminationState = TerminationState::UNKNOWN; ///< Retained all-control proof.
  ResetState resetState = ResetState::NONE; ///< Retained reset settling/postcondition proof.
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

}  // namespace TUSB32x
