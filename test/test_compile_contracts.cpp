/// @file test_compile_contracts.cpp
/// @brief Linked passive-value, constexpr Status and independent validity contracts; no driver I/O.
#include <TUSB32x/Config.h>
#include <TUSB32x/CommandTable.h>
#include <TUSB32x/Status.h>
#include <TUSB32x/TUSB32x.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <type_traits>

const char* errorNameFromOtherTranslationUnit(TUSB32x::Err code);
TUSB32x::Status statusFromOtherTranslationUnit();

namespace {

namespace t = TUSB32x;

constexpr bool sameText(const char* left, const char* right) {
  // Every input below is a fixed literal or errorName output; compare content, not pooled addresses.
  for (std::size_t index = 0; index < 64; ++index) {
    if (left[index] != right[index]) return false;
    if (left[index] == '\0') return true;
  }
  return false;
}

constexpr t::Status DEFAULT_STATUS{};
constexpr t::Status SUCCESS = t::Status::Ok();
constexpr t::Status PROGRESS = t::Status::Error(t::Err::IN_PROGRESS, "working");
constexpr t::Status LOW_DETAIL = t::Status::Error(
    t::Err::I2C_TIMEOUT, "low", std::numeric_limits<int32_t>::min());
constexpr t::Status HIGH_DETAIL{
    t::Err::I2C_BUS, std::numeric_limits<int32_t>::max(), "high"};

static_assert(DEFAULT_STATUS.ok() && DEFAULT_STATUS.detail == 0 &&
              sameText(DEFAULT_STATUS.msg, ""), "Default Status is empty successful evidence");
static_assert(SUCCESS.is(t::Err::OK) && !SUCCESS.inProgress() &&
              static_cast<bool>(SUCCESS) && sameText(SUCCESS.msg, "OK"),
              "Canonical success is constant-evaluable");
static_assert(PROGRESS.inProgress() && !PROGRESS.ok() && !static_cast<bool>(PROGRESS),
              "Progress is distinct from success");
static_assert(LOW_DETAIL.detail == std::numeric_limits<int32_t>::min() &&
              HIGH_DETAIL.detail == std::numeric_limits<int32_t>::max(),
              "Status preserves the full signed backend domain");
static_assert(!std::is_convertible<t::Status, bool>::value &&
              std::is_constructible<bool, t::Status>::value,
              "Status success conversion must be explicit");
static_assert(sameText(t::errorName(t::Err::I2C_TIMEOUT), "I2C_TIMEOUT") &&
              sameText(t::errorName(static_cast<t::Err>(255)), "UNKNOWN"),
              "errorName is fully defined for constant evaluation in the public header");
static_assert(std::is_same<std::underlying_type<t::Err>::type, uint8_t>::value,
              "Error representation is one unsigned byte");

using ExpectedWrite = t::Status (*)(uint8_t, const uint8_t*, std::size_t, uint32_t, void*);
using ExpectedWriteRead = t::Status (*)(uint8_t, const uint8_t*, std::size_t,
                                       uint8_t*, std::size_t, uint32_t, void*);
static_assert(std::is_same<t::I2cWriteFn, ExpectedWrite>::value &&
              std::is_same<t::I2cWriteReadFn, ExpectedWriteRead>::value,
              "Borrowed callbacks preserve address, byte count, timeout and context types");
static_assert(std::is_same<decltype(t::Status{}.detail), int32_t>::value &&
              std::is_same<decltype(t::Status{}.msg), const char*>::value,
              "Status retains backend detail and borrowed static text");
static_assert(std::is_same<decltype(t::OperationToken{}.value), uint64_t>::value &&
              std::is_same<decltype(t::OperationTiming{}.nowMs), uint64_t>::value &&
              std::is_same<decltype(t::OperationTiming{}.deadlineMs), uint64_t>::value,
              "Owner time and token correlation use the uint64 domain");
static_assert(!t::OperationToken{}.valid() && t::OperationToken{1}.valid() &&
              t::OperationToken{std::numeric_limits<uint64_t>::max()}.valid(),
              "Token validity means nonzero, without claiming instance currency");
static_assert(t::OWNER_CLOCK_MARGIN_MS == 1, "Owner quantization margin is separate and explicit");

template <typename... Values>
constexpr bool fixedValues() {
  return ((std::is_trivially_copyable<Values>::value &&
           std::is_standard_layout<Values>::value) && ...);
}

static_assert(fixedValues<t::Status, t::DriverConfig, t::DeviceProfile,
                         t::OperationToken, t::OperationTiming, t::PortSnapshot,
                         t::ConfigurationSnapshot, t::IdentitySnapshot,
                         t::InterruptSnapshot, t::OperationResult,
                         t::OperationProgress, t::TransportStats,
                         t::ManagedTransportEvidence, t::HealthPolicy,
                         t::HealthSnapshot, t::DriverDiagnostics, t::RegisterDecode>(),
              "Public passive values remain fixed, standard-layout and trivially copyable");
static_assert(std::extent<decltype(t::IdentitySnapshot{}.rawIdentity)>::value == 8,
              "Identity evidence has fixed eight-byte storage");
static_assert(static_cast<uint8_t>(t::DeviceAddress::ADDR_LOW_0X60) == 0x60 &&
              static_cast<uint8_t>(t::DeviceAddress::ADDR_HIGH_0X61) == 0x61 &&
              static_cast<uint8_t>(t::DeviceAddress::ADDR_LOW_0X47) == 0x47 &&
              static_cast<uint8_t>(t::DeviceAddress::ADDR_HIGH_0X67) == 0x67,
              "Address enum values are exact seven-bit target addresses");

int failures = 0;

void check(bool condition, const char* expression, int line) {
  if (!condition) {
    std::printf("FAIL line %d: %s\n", line, expression);
    ++failures;
  }
}

// These checks stay active when compiled with NDEBUG, including Release consumers.
#define CHECK(expression) check((expression), #expression, __LINE__)

void statusNamesAndLinkage() {
  struct ErrorName {
    t::Err code;
    const char* name;
  };
  constexpr ErrorName EXPECTED[] = {
      {t::Err::OK, "OK"},
      {t::Err::NOT_BOUND, "NOT_BOUND"},
      {t::Err::NOT_INITIALIZED, "NOT_INITIALIZED"},
      {t::Err::INVALID_CONFIG, "INVALID_CONFIG"},
      {t::Err::INVALID_PARAM, "INVALID_PARAM"},
      {t::Err::UNSUPPORTED_FEATURE, "UNSUPPORTED_FEATURE"},
      {t::Err::BUSY, "BUSY"},
      {t::Err::IN_PROGRESS, "IN_PROGRESS"},
      {t::Err::RESULT_PENDING, "RESULT_PENDING"},
      {t::Err::RESULT_NOT_AVAILABLE, "RESULT_NOT_AVAILABLE"},
      {t::Err::TOKEN_MISMATCH, "TOKEN_MISMATCH"},
      {t::Err::TOKEN_EXHAUSTED, "TOKEN_EXHAUSTED"},
      {t::Err::TIMEOUT, "TIMEOUT"},
      {t::Err::CANCELLED, "CANCELLED"},
      {t::Err::CONFIGURATION_UNKNOWN, "CONFIGURATION_UNKNOWN"},
      {t::Err::CONFIGURATION_MISMATCH, "CONFIGURATION_MISMATCH"},
      {t::Err::CHIP_ID_MISMATCH, "CHIP_ID_MISMATCH"},
      {t::Err::I2C_ERROR, "I2C_ERROR"},
      {t::Err::I2C_NACK_ADDR, "I2C_NACK_ADDR"},
      {t::Err::I2C_NACK_DATA, "I2C_NACK_DATA"},
      {t::Err::I2C_TIMEOUT, "I2C_TIMEOUT"},
      {t::Err::I2C_BUS, "I2C_BUS"},
      {t::Err::I2C_BUSY, "I2C_BUSY"},
  };
  static_assert(sizeof(EXPECTED) / sizeof(EXPECTED[0]) == 23, "Complete initial Err vocabulary");
  for (std::size_t index = 0; index < sizeof(EXPECTED) / sizeof(EXPECTED[0]); ++index) {
    CHECK(static_cast<std::size_t>(EXPECTED[index].code) == index);
    CHECK(std::strcmp(t::errorName(EXPECTED[index].code), EXPECTED[index].name) == 0);
    CHECK(std::strcmp(errorNameFromOtherTranslationUnit(EXPECTED[index].code),
                      EXPECTED[index].name) == 0);
    const t::Status value = t::Status::Error(EXPECTED[index].code, "static evidence", -70000);
    CHECK(value.is(EXPECTED[index].code) && value.detail == -70000);
    CHECK(value.ok() == (index == 0));
    CHECK(value.inProgress() == (EXPECTED[index].code == t::Err::IN_PROGRESS));
    CHECK(static_cast<bool>(value) == value.ok());
  }
  constexpr uint8_t INVALID_CODES[] = {23, 127, 255};
  for (const uint8_t invalid : INVALID_CODES) {
    CHECK(std::strcmp(t::errorName(static_cast<t::Err>(invalid)), "UNKNOWN") == 0);
    CHECK(std::strcmp(errorNameFromOtherTranslationUnit(static_cast<t::Err>(invalid)),
                      "UNKNOWN") == 0);
  }
  const t::Status other = statusFromOtherTranslationUnit();
  CHECK(other.is(t::Err::I2C_ERROR) && other.detail == 100000);
  CHECK(std::strcmp(other.msg, "other translation unit") == 0);
  CHECK(LOW_DETAIL.detail == std::numeric_limits<int32_t>::min());
  CHECK(HIGH_DETAIL.detail == std::numeric_limits<int32_t>::max());
}

constexpr bool defaultProfile(const t::DeviceProfile& profile) {
  return profile.portMode == t::PortMode::DRP &&
         profile.sourceCurrent == t::SourceCurrent::DEFAULT &&
         profile.sourcePreference == t::SourcePreference::NORMAL &&
         profile.drpDutyCycle == t::DrpDutyCycle::PERCENT_50 &&
         profile.ccDebounce == t::CcDebounce::MS_168 &&
         !profile.disableUfpAccessory && !profile.disableTerm && !profile.disableRdRp;
}

void profileDefaults() {
  constexpr t::DriverConfig binding{};
  CHECK(binding.i2cWrite == nullptr && binding.i2cWriteRead == nullptr && binding.i2cUser == nullptr);
  CHECK(binding.variant == t::Variant::UNSPECIFIED);
  CHECK(binding.address == t::DeviceAddress::ADDR_LOW_0X47 && binding.i2cTimeoutMs == 20);
  constexpr t::DeviceProfile candidate{};
  CHECK(candidate.portMode == t::PortMode::DRP && candidate.sourceCurrent == t::SourceCurrent::DEFAULT);
  CHECK(candidate.sourcePreference == t::SourcePreference::NORMAL);
  CHECK(candidate.drpDutyCycle == t::DrpDutyCycle::PERCENT_50 && candidate.ccDebounce == t::CcDebounce::MS_168);
  CHECK(!candidate.disableUfpAccessory && !candidate.disableTerm && !candidate.disableRdRp);
}

void portDefaults() {
  constexpr t::PortSnapshot port{};
  CHECK(port.rawStatus08 == 0 && port.rawStatus09 == 0 && port.rawValidMask == 0);
  CHECK(port.validFields == 0 && port.invalidated);
  CHECK(port.attachment == t::Attachment::UNKNOWN);
  CHECK(port.localRole == t::PortRole::UNKNOWN && port.peerRole == t::PortRole::UNKNOWN);
  CHECK(port.orientation == t::Orientation::UNKNOWN && port.detectedCurrent == t::DetectedCurrent::UNKNOWN);
  CHECK(port.advertisedCurrent == t::SourceCurrent::DEFAULT && port.accessory == t::Accessory::UNKNOWN);
  CHECK(!port.activeCable && !port.interruptPending && port.sequence == 0);
  CHECK(port.observedUptimeMs == 0 && !port.observedTimeValid && port.configGeneration == 0);
  CHECK(port.configurationState == t::ConfigurationState::UNCONFIGURED);
  CHECK(port.coherence == t::SnapshotCoherence::SEQUENTIAL);
  CHECK(t::STATUS_08_VALID == 1 && t::STATUS_09_VALID == 2);
  constexpr t::PortField FIELDS[] = {
      t::PortField::ATTACHMENT, t::PortField::LOCAL_ROLE, t::PortField::PEER_ROLE,
      t::PortField::ORIENTATION, t::PortField::DETECTED_CURRENT,
      t::PortField::ADVERTISED_CURRENT, t::PortField::ACCESSORY,
      t::PortField::ACTIVE_CABLE, t::PortField::INTERRUPT_PENDING};
  for (std::size_t index = 0; index < sizeof(FIELDS) / sizeof(FIELDS[0]); ++index) {
    CHECK(static_cast<uint16_t>(FIELDS[index]) == (uint16_t{1} << index));
  }
}

void configurationDefaults() {
  constexpr t::ConfigurationSnapshot image{};
  CHECK(!image.hasDesiredProfile && !image.hasObservedProfile && !image.hasVerifiedProfile);
  CHECK(image.desiredGeneration == 0 && image.verifiedGeneration == 0);
  CHECK(image.state == t::ConfigurationState::UNCONFIGURED);
  CHECK(image.terminationState == t::TerminationState::UNKNOWN && image.resetState == t::ResetState::NONE);
  CHECK(!image.mismatchValid && image.mismatchRegister == 0 && image.mismatchExpected == 0);
  CHECK(image.mismatchObserved == 0 && image.mismatchMask == 0);
  CHECK(image.observedUptimeMs == 0 && !image.observedTimeValid);
  CHECK(image.rawConfig08 == 0 && image.rawConfig09 == 0 && image.rawConfig0A == 0 && image.rawConfig45 == 0);
  CHECK(image.rawConfigValidMask == 0 && image.observedValidFields == 0);
  CHECK(defaultProfile(image.desiredProfile) && defaultProfile(image.observedProfile));
  CHECK(defaultProfile(image.verifiedProfile));
  CHECK(t::CONFIG_08_VALID == 1 && t::CONFIG_09_VALID == 2 && t::CONFIG_0A_VALID == 4 && t::CONFIG_45_VALID == 8);
  constexpr t::ProfileField FIELDS[] = {
      t::ProfileField::PORT_MODE, t::ProfileField::SOURCE_CURRENT,
      t::ProfileField::SOURCE_PREFERENCE, t::ProfileField::DRP_DUTY_CYCLE,
      t::ProfileField::CC_DEBOUNCE, t::ProfileField::DISABLE_UFP_ACCESSORY,
      t::ProfileField::DISABLE_TERM, t::ProfileField::DISABLE_RD_RP};
  for (std::size_t index = 0; index < sizeof(FIELDS) / sizeof(FIELDS[0]); ++index) {
    CHECK(static_cast<uint16_t>(FIELDS[index]) == (uint16_t{1} << index));
  }

  // Public values copy independent evidence; changing bytes never synthesizes decoded validity.
  t::ConfigurationSnapshot rawOnly{};
  rawOnly.rawConfig08 = 0xFF;
  rawOnly.rawConfigValidMask = t::CONFIG_08_VALID;
  const auto copy = rawOnly;
  CHECK(copy.rawConfig08 == 0xFF && copy.rawConfigValidMask == t::CONFIG_08_VALID);
  CHECK(copy.observedValidFields == 0 && !copy.hasObservedProfile && !copy.hasVerifiedProfile);
  CHECK(copy.state == t::ConfigurationState::UNCONFIGURED);
}

void operationAndIdentityDefaults() {
  constexpr t::IdentitySnapshot identity{};
  CHECK(identity.variant == t::Variant::UNSPECIFIED && identity.address == t::DeviceAddress::ADDR_LOW_0X47);
  CHECK(!identity.identityValid && !identity.identityMatches && !identity.revisionValid && !identity.revisionMatches);
  CHECK(identity.rawRevision == 0);
  for (const uint8_t byte : identity.rawIdentity) CHECK(byte == 0);
  constexpr t::InterruptSnapshot interrupt{};
  CHECK(!interrupt.observationValid && !interrupt.pendingBefore && !interrupt.acknowledgeAttempted);
  CHECK(!interrupt.acknowledgeConfirmed && !interrupt.pendingAfter && !interrupt.pendingAfterValid);
  constexpr t::OperationTiming timing{};
  CHECK(timing.nowMs == 0 && timing.deadlineMs == 0);
  constexpr t::OperationResult result{};
  CHECK(!result.token.valid() && result.kind == t::OperationKind::NONE && result.state == t::OperationState::IDLE);
  CHECK(result.status.ok() && result.hardwareEffect == t::HardwareEffect::NONE && result.transfers == 0);
  CHECK(result.startedUptimeMs == 0 && result.completedUptimeMs == 0 && !result.completedTimeValid);
  CHECK(!result.identity.identityValid && !result.configuration.hasVerifiedProfile);
  CHECK(result.port.invalidated && result.port.validFields == 0 && !result.interrupt.observationValid);
  constexpr t::OperationProgress progress{};
  CHECK(!progress.token.valid() && progress.kind == t::OperationKind::NONE && progress.state == t::OperationState::IDLE);
  CHECK(progress.phase == t::OperationPhase::NONE && progress.transfers == 0 && progress.lastPollTransfers == 0);
  CHECK(progress.deadlineMs == 0 && !progress.waiting && progress.readyAtMs == 0);
  CHECK(!progress.readyAtValid && !progress.resultPending);
}

void diagnosticDefaults() {
  constexpr t::TransportStats stats{};
  CHECK(stats.totalAttempts == 0 && stats.totalSuccess == 0 && stats.totalFailures == 0 && stats.lastStatus.ok());
  constexpr t::ManagedTransportEvidence evidence{};
  CHECK(!evidence.initialized && evidence.totalSuccess == 0 && evidence.totalFailures == 0);
  CHECK(evidence.consecutiveFailures == 0 && evidence.lastStatus.ok());
  CHECK(evidence.lastSuccessUptimeMs == 0 && !evidence.lastSuccessTimeValid);
  CHECK(evidence.lastFailureUptimeMs == 0 && !evidence.lastFailureTimeValid);
  constexpr t::HealthPolicy policy{};
  constexpr t::HealthSnapshot health{};
  CHECK(policy.offlineThreshold == 3 && health.offlineThreshold == 3 && health.state == t::DriverState::UNINIT);
  CHECK(!health.evidence.initialized && health.evidence.totalSuccess == 0 && health.evidence.totalFailures == 0);
  constexpr t::DriverDiagnostics diagnostics{};
  CHECK(!diagnostics.bound && !diagnostics.initialized);
  CHECK(diagnostics.configurationState == t::ConfigurationState::UNCONFIGURED);
  CHECK(diagnostics.hardwareEffect == t::HardwareEffect::NONE && diagnostics.terminationState == t::TerminationState::UNKNOWN);
  CHECK(diagnostics.resetState == t::ResetState::NONE && !diagnostics.recoverySequenceUnresolved);
  CHECK(!diagnostics.configurationWriteUnresolved);
  CHECK(!diagnostics.modeTransitionUnresolved && !diagnostics.resetAcceptanceUncertain);
  CHECK(diagnostics.lastOperationKind == t::OperationKind::NONE && diagnostics.lastOperationStatus.ok());
  CHECK(diagnostics.lastFailedTransportStatus.ok() && diagnostics.lastFailedTransportUptimeMs == 0);
  CHECK(!diagnostics.lastFailedTransportTimeValid && !diagnostics.managedTransport.initialized);
  CHECK(!diagnostics.identity.identityValid && !diagnostics.interrupt.observationValid);

  // Ordinary settings uncertainty is independent of the latest read-only outcome.
  // This checks copied value storage, not an unimplemented driver's transitions.
  t::DriverDiagnostics interrupted{};
  interrupted.configurationWriteUnresolved = true;
  interrupted.lastOperationKind = t::OperationKind::READ_PROFILE;
  interrupted.hardwareEffect = t::HardwareEffect::NONE;
  const auto retained = interrupted;
  interrupted.configurationWriteUnresolved = false;
  CHECK(retained.configurationWriteUnresolved);
  CHECK(retained.hardwareEffect == t::HardwareEffect::NONE);
  CHECK(retained.lastOperationKind == t::OperationKind::READ_PROFILE);
}

void registerValueDefaults() {
  constexpr t::RegisterDecode codes{};
  CHECK(codes.variant == t::Variant::UNSPECIFIED && codes.reg == 0 && codes.value == 0);
  CHECK(defaultProfile(codes.profileCodes) && codes.profileValidFields == 0);
  CHECK(codes.attachment == t::Attachment::UNKNOWN && codes.orientation == t::Orientation::UNKNOWN);
  CHECK(codes.detectedCurrent == t::DetectedCurrent::UNKNOWN && codes.accessory == t::Accessory::UNKNOWN);
  CHECK(!codes.activeCable && !codes.interruptPending && codes.statusValidFields == 0);
  CHECK(!codes.resetRequested && !codes.resetValid);
}

#undef CHECK

}  // namespace

int main() {
  statusNamesAndLinkage();
  profileDefaults();
  portDefaults();
  configurationDefaults();
  operationAndIdentityDefaults();
  diagnosticDefaults();
  registerValueDefaults();
  if (failures != 0) return 1;
  std::puts("PASS passive values, 23 error names, constexpr Status and multiple translation units");
  return 0;
}
