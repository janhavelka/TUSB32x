/// @file TUSB32xOperations.cpp
/// @brief Bus-silent lifecycle, shared proof/port publication and explicit bounded write/IRQ operations.
#include "TUSB32x/TUSB32x.h"

#include "TUSB32x/CommandTable.h"
#include "TUSB32xProtocol.h"
#include "TUSB32xCodecs.h"
#include "TUSB32xOwnerTime.h"

#include <cstring>

namespace TUSB32x {
namespace {

Status error(Err code) { return Status::Error(code, errorName(code)); }

// TI SLLSEN9F Table 7-7 p18; SLLSEP2D/SLLSEQ8D Table 9 p18.
// TUSB322I VCONN fault bit 3 is separate (SLLSEO5D Table 6-9 p19).
constexpr uint8_t IRQ_PENDING = 0x10;

// Persistent masks exclude live/RO, IRQ W1C, reset command and internal45 bits.
// TI SLLSEN9F Table 7-7; SLLSEP2D/SLLSEQ8D Tables8-10; SLLSEO5D Tables6-8..6-10.
uint8_t comparisonMask(uint8_t reg, bool extended) {
  switch (reg) {
    case 0x08: return 0xC0;
    case 0x09: return extended ? 0x07 : 0x06;
    case 0x0A: return extended ? 0xF7 : 0xF0;
    case 0x45: return 0x04;
    default: return 0;
  }
}

}  // namespace

bool TUSB32x::unresolved() const {
  return _diagnostics.configurationWriteUnresolved || _diagnostics.recoverySequenceUnresolved ||
      _diagnostics.modeTransitionUnresolved || _diagnostics.resetAcceptanceUncertain ||
      _configuration.resetState == ResetState::MAY_BE_PENDING;
}

// Revoke write images and dependent port usability, retaining raw history and
// desired settings for explicit later reconciliation.
void TUSB32x::invalidateConfiguration() {
  _rdRpWriteImageValid = false;
  _modeWriteImageValid = false;
  _configuration.state = ConfigurationState::UNKNOWN;
  _configuration.terminationState = TerminationState::UNKNOWN;
  _diagnostics.configurationState = ConfigurationState::UNKNOWN;
  _diagnostics.terminationState = TerminationState::UNKNOWN;
  _port.invalidated = true;
}

// End/invalidation also revokes initialization; results and historical effects
// remain available to the serialized owner.
void TUSB32x::invalidateUsability() {
  _rdRpWriteImageValid = false;
  _modeWriteImageValid = false;
  _diagnostics.initialized = false;
  _diagnostics.managedTransport.initialized = false;
  if (_configuration.state != ConfigurationState::UNCONFIGURED)
    _configuration.state = ConfigurationState::UNKNOWN;
  _configuration.terminationState = TerminationState::UNKNOWN;
  _diagnostics.configurationState = _configuration.state;
  _diagnostics.terminationState = TerminationState::UNKNOWN;
  _port.invalidated = true;
}

// Clear the old association after ownership gates, without resetting hardware
// or erasing transport totals.
void TUSB32x::clearTarget() {
  _configuration = {};
  _port = {};
  _diagnostics.identity = {};
  _diagnostics.interrupt = {};
  _diagnostics.configurationWriteUnresolved = false;
  _diagnostics.recoverySequenceUnresolved = false;
  _diagnostics.modeTransitionUnresolved = false;
  _diagnostics.resetAcceptanceUncertain = false;
  _diagnostics.resetState = ResetState::NONE;
  _diagnostics.hardwareEffect = HardwareEffect::NONE;
  invalidateUsability();
}

Status TUSB32x::bind(const DriverConfig& config) {
  const Status valid = validateDriverConfig(config);
  if (!valid.ok()) return valid;
  if (_progress.state == OperationState::ACTIVE) return error(Err::BUSY);
  if (_progress.resultPending) return error(Err::RESULT_PENDING);
  const bool same = _hasAssociation && config.variant == _config.variant &&
      config.address == _config.address && config.i2cUser == _config.i2cUser &&
      config.i2cWrite == _config.i2cWrite && config.i2cWriteRead == _config.i2cWriteRead;
  if (!same && unresolved()) return error(Err::CONFIGURATION_UNKNOWN);
  if (!same) clearTarget();
  else invalidateUsability();
  _config = config;
  _hasAssociation = true;
  _diagnostics.bound = true;
  _diagnostics.identity.variant = config.variant;
  _diagnostics.identity.address = config.address;
  return Status::Ok();
}

// Reject stale/malformed instance-local tokens without consuming any evidence.
Status TUSB32x::checkToken(OperationToken token) const {
  if (!token.valid()) return error(Err::INVALID_PARAM);
  if (!_progress.token.valid()) return error(Err::RESULT_NOT_AVAILABLE);
  if (token.value != _progress.token.value) return error(Err::TOKEN_MISMATCH);
  return Status::Ok();
}

// Publish one retained terminal outcome with transport detail and conservative
// effects. Completion neither rolls back writes nor consumes the result slot.
Status TUSB32x::finish(Status status, bool timeValid, uint64_t nowMs) {
  if (_configuration.state == ConfigurationState::APPLYING) invalidateConfiguration();
  if (status.ok() && (_progress.kind == OperationKind::APPLY_PROFILE ||
      _progress.kind == OperationKind::RESET ||
      _progress.kind == OperationKind::RECOVER_DEAD_BATTERY ||
      _progress.kind == OperationKind::ACKNOWLEDGE_INTERRUPT ||
      _progress.kind == OperationKind::SERVICE_INTERRUPT) &&
      _result.hardwareEffect == HardwareEffect::PARTIAL)
    _result.hardwareEffect = HardwareEffect::CONFIRMED;
  _progress.state = status.ok() ? OperationState::SUCCEEDED :
      status.is(Err::CANCELLED) ? OperationState::CANCELLED :
      status.is(Err::TIMEOUT) ? OperationState::TIMED_OUT : OperationState::FAILED;
  _progress.phase = OperationPhase::NONE;
  _progress.resultPending = true;
  _progress.waiting = false;
  _progress.readyAtValid = false;
  _result.state = _progress.state;
  _result.status = status;
  _result.transfers = _progress.transfers;
  _result.completedTimeValid = timeValid;
  _result.completedUptimeMs = timeValid ? nowMs : 0;
  _result.identity = _diagnostics.identity;
  _result.configuration = _configuration;
  _result.port = _port;
  _diagnostics.lastOperationKind = _result.kind;
  _diagnostics.lastOperationStatus = status;
  _diagnostics.hardwareEffect = _result.hardwareEffect;
  _pending = false;
  return status;
}

Status TUSB32x::cancel(OperationToken token) {
  const Status valid = checkToken(token);
  if (!valid.ok()) return valid;
  if (_progress.state == OperationState::ACTIVE) finish(error(Err::CANCELLED));
  return Status::Ok();
}

Status TUSB32x::end() {
  invalidateUsability();
  _diagnostics.bound = false;
  if (_progress.state == OperationState::ACTIVE) finish(error(Err::CANCELLED));
  return Status::Ok();
}

Status TUSB32x::invalidateDeviceState(const Status& reason) {
  if (reason.code == Err::IN_PROGRESS || static_cast<uint8_t>(reason.code) >
      static_cast<uint8_t>(Err::I2C_BUSY)) return error(Err::INVALID_PARAM);
  const Status cause = reason.ok() ? error(Err::CONFIGURATION_UNKNOWN) : reason;
  invalidateUsability();
  if (_progress.state == OperationState::ACTIVE)
    finish(Status::Error(Err::CANCELLED, cause.msg, cause.detail));
  return Status::Ok();
}

Status TUSB32x::forgetDeviceEvidence() {
  if (_progress.state == OperationState::ACTIVE) return error(Err::BUSY);
  if (_progress.resultPending) return error(Err::RESULT_PENDING);
  if (_diagnostics.bound) return error(Err::INVALID_CONFIG);
  clearTarget();
  _config = {};
  _hasAssociation = false;
  return Status::Ok();
}

Status TUSB32x::resultSnapshot(OperationToken token, OperationResult& out) const {
  const Status valid = checkToken(token);
  if (!valid.ok()) return valid;
  if (!_progress.resultPending) return error(Err::RESULT_NOT_AVAILABLE);
  out = _result;
  return Status::Ok();
}

Status TUSB32x::takeResult(OperationToken token, OperationResult& out) {
  const Status status = resultSnapshot(token, out);
  if (status.ok()) {
    _progress = {};
    _result = {};
  }
  return status;
}

Status TUSB32x::startInitialize(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::INITIALIZE, timing, token);
}

Status TUSB32x::startReadProfile(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::READ_PROFILE, timing, token);
}

Status TUSB32x::startReconcile(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::RECONCILE, timing, token);
}

Status TUSB32x::startApplyProfile(const DeviceProfile& profile, const OperationTiming& timing,
                                OperationToken& token) {
  return startOperation(OperationKind::APPLY_PROFILE, timing, token, &profile);
}

Status TUSB32x::startVerifyProfile(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::VERIFY_PROFILE, timing, token);
}

Status TUSB32x::startReset(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::RESET, timing, token);
}

Status TUSB32x::startRefreshPort(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::REFRESH_PORT, timing, token);
}

Status TUSB32x::startAcknowledgeInterrupt(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::ACKNOWLEDGE_INTERRUPT, timing, token);
}

Status TUSB32x::startServiceInterrupt(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::SERVICE_INTERRUPT, timing, token);
}

Status TUSB32x::startRecoverDeadBattery(const OperationTiming& timing, OperationToken& token) {
  return startOperation(OperationKind::RECOVER_DEAD_BATTERY, timing, token);
}

// Admit bus-silent work with an immutable deadline and one instance-local token;
// only the shared poll path advances transport.
Status TUSB32x::startOperation(OperationKind kind, const OperationTiming& timing,
                               OperationToken& token, const DeviceProfile* candidate) {
  const uint64_t interval = timing.deadlineMs - timing.nowMs;
  if (interval == 0 || interval >= detail::HALF_RANGE) return error(Err::INVALID_PARAM);
  const bool interrupt = kind == OperationKind::ACKNOWLEDGE_INTERRUPT ||
      kind == OperationKind::SERVICE_INTERRUPT;
  if (candidate != nullptr) {
    // Before binding only enum shape is meaningful; no variant may be guessed.
    // The complete public validator then checks the bound variant's capabilities.
    if (candidate->portMode > PortMode::DRP || candidate->sourceCurrent > SourceCurrent::MA_3000 ||
        candidate->sourcePreference > SourcePreference::TRY_SRC ||
        candidate->drpDutyCycle > DrpDutyCycle::PERCENT_60 || candidate->ccDebounce > CcDebounce::MS_168)
      return error(Err::INVALID_CONFIG);
    if (_diagnostics.bound) {
      const Status valid = validateProfile(_config.variant, *candidate);
      if (!valid.ok()) return valid;
    }
  }
  if (kind == OperationKind::RECOVER_DEAD_BATTERY && _diagnostics.bound &&
      _config.variant != Variant::TUSB320 && _config.variant != Variant::TUSB320I)
    return error(Err::UNSUPPORTED_FEATURE);
  // SLLSEO5D Table6-9: fault RCU lacks a safe clear transaction. Gate the whole09 write.
  if (interrupt && _diagnostics.bound && _config.variant == Variant::TUSB322I)
    return error(Err::UNSUPPORTED_FEATURE);
  if (!_diagnostics.bound) return error(Err::NOT_BOUND);
  if (_progress.state == OperationState::ACTIVE) return error(Err::BUSY);
  if (_progress.resultPending) return error(Err::RESULT_PENDING);
  if (kind == OperationKind::RESET || kind == OperationKind::RECOVER_DEAD_BATTERY || interrupt) {
    if (!_diagnostics.initialized) return error(Err::NOT_INITIALIZED);
    if (kind == OperationKind::RECOVER_DEAD_BATTERY &&
        (_diagnostics.modeTransitionUnresolved ||
         _configuration.resetState == ResetState::MAY_BE_PENDING))
      return error(Err::CONFIGURATION_UNKNOWN);
  }
  ProfileWrite writes[6]{};
  uint8_t count = 0;
  bool baseMode = false;
  if (kind == OperationKind::VERIFY_PROFILE && !_configuration.hasDesiredProfile)
    return error(Err::CONFIGURATION_UNKNOWN);
  if (candidate != nullptr) {
    if (!_diagnostics.initialized) return error(Err::NOT_INITIALIZED);
    if (_configuration.state != ConfigurationState::KNOWN || !_configuration.hasVerifiedProfile ||
        _diagnostics.configurationWriteUnresolved || _diagnostics.modeTransitionUnresolved ||
        _diagnostics.recoverySequenceUnresolved ||
        _configuration.resetState == ResetState::MAY_BE_PENDING)
      return error(Err::CONFIGURATION_UNKNOWN);
    const Status planned = planProfile(*candidate, writes, count, baseMode);
    if (!planned.ok()) return planned;
  }
  uint64_t next = 0;
  if (!detail::nextToken(_lastToken, next)) return error(Err::TOKEN_EXHAUSTED);
  _lastToken = next;
  if (kind == OperationKind::INITIALIZE) invalidateUsability();
  _progress = {};
  _progress.token = {next};
  _progress.kind = kind;
  _progress.state = OperationState::ACTIVE;
  _progress.phase = OperationPhase::READING;
  _progress.deadlineMs = timing.deadlineMs;
  _result = {};
  _result.token = _progress.token;
  _result.kind = _progress.kind;
  _result.startedUptimeMs = timing.nowMs;
  _lastTime = timing.nowMs;
  const bool proof = kind == OperationKind::RECONCILE || kind == OperationKind::VERIFY_PROFILE;
  _waitReset = proof &&
      _configuration.resetState == ResetState::MAY_BE_PENDING;
  _waitMode = proof && _diagnostics.modeTransitionUnresolved;
  _phase = kind == OperationKind::READ_PROFILE ? Phase::CONFIG08 : Phase::IDENTITY;
  if (_waitReset || _waitMode) {
    _phase = Phase::WAIT;
    _progress.phase = OperationPhase::WAITING;
    _progress.waiting = true;
  }
  _observing = {};
  _observingPort = {};
  _pending = false;
  _pendingStatus = Status::Ok();
  _writeCount = count;
  _writeIndex = 0;
  if (kind == OperationKind::RESET) {
    _phase = Phase::RESET_PRECHECK;
    _waitMode = _diagnostics.modeTransitionUnresolved;
  } else if (kind == OperationKind::RECOVER_DEAD_BATTERY) {
    _phase = Phase::RECOVERY_PRECHECK;
  } else if (kind == OperationKind::REFRESH_PORT || kind == OperationKind::SERVICE_INTERRUPT) {
    _phase = Phase::PORT_FIRST09;
  } else if (kind == OperationKind::ACKNOWLEDGE_INTERRUPT) {
    _phase = Phase::IRQ_PRECHECK;
  }
  if (candidate != nullptr) {
    for (uint8_t i = 0; i < count; ++i) _writes[i] = writes[i];
    _configuration.desiredProfile = *candidate;
    _configuration.hasDesiredProfile = true;
    _configuration.desiredGeneration = detail::nextGeneration(_configuration.desiredGeneration);
    _configuration.mismatchValid = false;
    _phase = baseMode ? Phase::BASE_PRECHECK :
        count != 0 ? Phase::APPLY_WRITE : Phase::IDENTITY;
    _progress.phase = baseMode || count == 0 ? OperationPhase::READING : OperationPhase::WRITING;
  }
  token = _progress.token;
  return error(Err::IN_PROGRESS);
}

// Plan at most six writes from proved actual settings, changing requested fields
// only; sequence permission remains with the owner engine.
Status TUSB32x::planProfile(const DeviceProfile& candidate, ProfileWrite (&writes)[6],
                          uint8_t& count, bool& baseMode) const {
  const auto& info = *detail::findVariant(_config.variant);
  const uint8_t required = info.hasRdRp ? 0x0F : 0x07;
  if ((_configuration.rawConfigValidMask & required) != required)
    return error(Err::CONFIGURATION_UNKNOWN);
  const auto& actual = _configuration.verifiedProfile;
  const auto field = [](ProfileField value) { return static_cast<uint16_t>(value); };
  const uint16_t fields08 = candidate.sourceCurrent != actual.sourceCurrent ?
      field(ProfileField::SOURCE_CURRENT) : 0;
  const uint16_t fields09 = static_cast<uint16_t>(
      (candidate.drpDutyCycle != actual.drpDutyCycle ? field(ProfileField::DRP_DUTY_CYCLE) : 0U) |
      (candidate.disableUfpAccessory != actual.disableUfpAccessory ?
          field(ProfileField::DISABLE_UFP_ACCESSORY) : 0U));
  const uint16_t fields0A = static_cast<uint16_t>(
      (candidate.portMode != actual.portMode ? field(ProfileField::PORT_MODE) : 0U) |
      (candidate.sourcePreference != actual.sourcePreference ? field(ProfileField::SOURCE_PREFERENCE) : 0U) |
      (candidate.ccDebounce != actual.ccDebounce ? field(ProfileField::CC_DEBOUNCE) : 0U) |
      (candidate.disableTerm != actual.disableTerm ? field(ProfileField::DISABLE_TERM) : 0U));
  const bool modeChanges = candidate.portMode != actual.portMode;
  const bool rdRpChanges = candidate.disableRdRp != actual.disableRdRp;
  if (_config.variant == Variant::TUSB322I &&
      (fields08 != 0 || fields09 != 0 || (modeChanges && candidate.portMode == PortMode::DRP)))
    return error(Err::UNSUPPORTED_FEATURE);
  if (rdRpChanges && (!_rdRpWriteImageValid || (_configuration.rawConfig45 & 0x03U) != 0))
    return error(Err::CONFIGURATION_UNKNOWN);
  if (fields0A != 0 && !_modeWriteImageValid) return error(Err::CONFIGURATION_UNKNOWN);

  uint8_t image08 = _configuration.rawConfig08;
  uint8_t image09 = _configuration.rawConfig09;
  uint8_t image0A = _configuration.rawConfig0A;
  uint8_t image45 = _configuration.rawConfig45;
  // Validate the entire finite plan before admission. Stage flags here describe
  // preceding planned steps, not observed completion. poll must execute them in
  // order, with fresh time and the base detached precheck before any write.
  const auto append = [&](uint8_t reg, detail::WriteIntent intent, uint16_t fields,
                          uint8_t& image, bool modeStep = false, bool waitAfter = false) {
    detail::WriteRequest request{};
    request.variant = _config.variant;
    request.reg = reg;
    request.intent = intent;
    request.desired = candidate;
    request.fields = fields;
    detail::WriteEvidence evidence{};
    evidence.actual = image;
    evidence.actualValid = true;
    evidence.effectsResolved = true;
    evidence.detached = true;
    evidence.disableWritten = intent == detail::WriteIntent::MODE_CHANGE ||
        intent == detail::WriteIntent::MODE_ENABLE;
    evidence.modeWritten = intent == detail::WriteIntent::MODE_ENABLE;
    evidence.minimumWaitElapsed = intent == detail::WriteIntent::MODE_ENABLE;
    uint8_t value = 0;
    const Status status = detail::composeWrite(request, evidence, value);
    if (!status.ok()) return status;
    if (count >= 6) return error(Err::INVALID_CONFIG);
    writes[count++] = {reg, value, modeStep, waitAfter};
    image = value;
    return Status::Ok();
  };
  baseMode = modeChanges && !info.hasExtendedControls;
  Status status{};
  if (baseMode) {
    status = append(0x0A, detail::WriteIntent::BASE_MODE_CHANGE, fields0A, image0A, true);
    if (!status.ok()) return status;
  }
  if (rdRpChanges && candidate.disableRdRp) {
    status = append(0x45, detail::WriteIntent::SETTINGS, field(ProfileField::DISABLE_RD_RP), image45);
    if (!status.ok()) return status;
  }
  if (fields08 != 0) {
    status = append(0x08, detail::WriteIntent::SETTINGS, fields08, image08);
    if (!status.ok()) return status;
  }
  if (fields09 != 0) {
    status = append(0x09, detail::WriteIntent::SETTINGS, fields09, image09);
    if (!status.ok()) return status;
  }
  if (modeChanges && info.hasExtendedControls) {
    // TI SLLSEP2D/SLLSEQ8D 7.3.2 p14; SLLSEO5D 6.3.2.1-2 pp10-11.
    // Even initially disabled/unattached parts require the separate stage writes.
    status = append(0x0A, detail::WriteIntent::MODE_DISABLE, 0, image0A, true);
    if (!status.ok()) return status;
    status = append(0x0A, detail::WriteIntent::MODE_CHANGE, fields0A, image0A,
                    true, info.hasRdRp);
    if (!status.ok()) return status;
    if (!candidate.disableTerm) {
      status = append(0x0A, detail::WriteIntent::MODE_ENABLE, 0, image0A, true);
      if (!status.ok()) return status;
    }
  } else if (!baseMode && fields0A != 0) {
    status = append(0x0A, detail::WriteIntent::SETTINGS, fields0A, image0A);
    if (!status.ok()) return status;
  }
  if (rdRpChanges && !candidate.disableRdRp) {
    status = append(0x45, detail::WriteIntent::SETTINGS, field(ProfileField::DISABLE_RD_RP), image45);
    if (!status.ok()) return status;
  }
  return Status::Ok();
}

// After writes/waits, require a complete fresh observation before trusting apply.
void TUSB32x::beginApplyProof() {
  _phase = Phase::IDENTITY;
  _progress.phase = OperationPhase::READING;
  // All separate mode steps and any required wait have completed. The following
  // F still must prove current controls before this obligation can be discharged.
  _waitMode = _diagnostics.modeTransitionUnresolved;
}

// Publish completed signature/revision evidence without inventing polarity or
// losing an earlier complete revision when a later proof phase fails.
void TUSB32x::publishIdentityRead() {
  auto& identity = _diagnostics.identity;
  if (_phase == Phase::IDENTITY) {
    std::memcpy(identity.rawIdentity, _received, sizeof(identity.rawIdentity));
    // A matching signature during non-initializing proof does not erase an older complete
    // revision if the following read fails. Contradiction still invalidates it now.
    if (_progress.kind == OperationKind::INITIALIZE || !identity.identityMatches ||
        !_pendingStatus.ok()) {
      identity.revisionValid = false;
      identity.revisionMatches = false;
    }
    identity.identityValid = true;
    identity.identityMatches = _pendingStatus.ok();
  } else {
    identity.rawRevision = _received[0];
    identity.revisionValid = true;
    identity.revisionMatches = _pendingStatus.ok();
  }
}

// Advance the fixed profile-observation plan from one completed scratch read.
void TUSB32x::acceptRead() {
  // Fixed F plan: TI SLLSEN9F Table 7-7 pp16-19; SLLSEP2D/SLLSEQ8D
  // Tables 7-12 pp16-20; SLLSEO5D Tables 6-6..6-11 pp17-20.
  // Read identity then each applicable persistent-control byte once; no writes,
  // reset-bit completion assumption, IRQ consumption or dead-battery recovery.
  const auto& info = *detail::findVariant(_config.variant);
  if (_phase == Phase::IDENTITY) {
    publishIdentityRead();
    _phase = info.hasRevision ? Phase::REVISION : Phase::CONFIG08;
    return;
  }
  if (_phase == Phase::REVISION) {
    publishIdentityRead();
    _phase = Phase::CONFIG08;
    return;
  }
  const uint8_t reg = _phase == Phase::CONFIG08 ? 8 : _phase == Phase::CONFIG09 ? 9 :
      _phase == Phase::CONFIG0A ? 10 : 0x45;
  RegisterDecode decoded{};
  // Exact variant/map already validated; decoding retains reserved raw evidence.
  const Status status = decodeRegister(_config.variant, reg, _received[0], decoded);
  if (!status.ok()) { _pendingStatus = status; return; }
  _observing.observedValidFields |= decoded.profileValidFields;
  auto& profile = _observing.observedProfile;
  if (reg == 8) {
    _observing.rawConfig08 = _received[0];
    _observing.rawConfigValidMask |= CONFIG_08_VALID;
    profile.sourceCurrent = decoded.profileCodes.sourceCurrent;
    _phase = Phase::CONFIG09;
  } else if (reg == 9) {
    _observing.rawConfig09 = _received[0];
    _observing.rawConfigValidMask |= CONFIG_09_VALID;
    profile.drpDutyCycle = decoded.profileCodes.drpDutyCycle;
    profile.disableUfpAccessory = decoded.profileCodes.disableUfpAccessory;
    _phase = Phase::CONFIG0A;
  } else if (reg == 10) {
    _observing.rawConfig0A = _received[0];
    _observing.rawConfigValidMask |= CONFIG_0A_VALID;
    profile.portMode = decoded.profileCodes.portMode;
    profile.sourcePreference = decoded.profileCodes.sourcePreference;
    profile.ccDebounce = decoded.profileCodes.ccDebounce;
    profile.disableTerm = decoded.profileCodes.disableTerm;
    _phase = info.hasRdRp ? Phase::CONFIG45 : Phase::DONE;
  } else {
    _observing.rawConfig45 = _received[0];
    _observing.rawConfigValidMask |= CONFIG_45_VALID;
    profile.disableRdRp = decoded.profileCodes.disableRdRp;
    _phase = Phase::DONE;
  }
}

// Partial observations may revoke existing proof, but cannot repair whole
// configuration trust or certify historical mode/reset/recovery sequences.
void TUSB32x::observeControl(uint8_t reg, uint8_t value) {
  // SLLSEO5D Table6-10 p20: DRP00/11 are semantically equivalent, but composing
  // from the older byte would silently recode MODE_SELECT. Keep profile trust
  // separate from literal write-image permission; only complete F renews it.
  if (reg == 0x0A && ((value ^ _configuration.rawConfig0A) & 0x30U) != 0)
    _modeWriteImageValid = false;
  // Protected45 controls do not participate in user-profile equality. A raw/partial
  // observation can revoke that write image without replacing the historical P image
  // or blocking independently proved user settings. Only a complete F renews it.
  if (reg == 0x45 && ((value ^ _configuration.rawConfig45) & 0x07U) != 0) {
    _rdRpWriteImageValid = false;
  }
  if (reg == 0x0A && (value & 0x08U) != 0) {
    _configuration.resetState = ResetState::MAY_BE_PENDING;
    _diagnostics.resetState = ResetState::MAY_BE_PENDING;
    invalidateConfiguration();
  }
  // Even a complete single byte can contradict old current proof. Invalidate
  // immediately, but never publish this scratch byte as a whole observation.
  bool equal = false;
  Status status = detail::comparePersistentSettings(_config.variant, reg, value, value, equal);
  if (!status.ok()) {
    invalidateConfiguration();
    return;
  }
  // A complete P can establish termination without full configuration trust.
  // A changed disable bit invalidates that independent proof immediately, even
  // if a later read fails before this new P can replace the old raw observation.
  const auto& info = *detail::findVariant(_config.variant);
  if (_configuration.terminationState != TerminationState::UNKNOWN) {
    const bool termChanged = reg == 0x0A && info.hasExtendedControls &&
        ((value ^ _configuration.rawConfig0A) & 0x01U) != 0;
    const bool rdRpChanged = reg == 0x45 && info.hasRdRp &&
        ((value ^ _configuration.rawConfig45) & 0x04U) != 0;
    if (termChanged || rdRpChanged) invalidateConfiguration();
  }
  if (_configuration.state == ConfigurationState::KNOWN && _configuration.hasVerifiedProfile) {
    uint8_t expected = 0;
    status = detail::encodeProfileRegister(_config.variant, reg,
                                           _configuration.verifiedProfile, expected);
    if (status.ok())
      status = detail::comparePersistentSettings(_config.variant, reg, value, expected, equal);
    if (!status.ok() || !equal) invalidateConfiguration();
  }
}

// Compare semantic persistent fields, retaining the first raw mismatch; live
// status and IRQ bits do not participate in profile equality.
Status TUSB32x::compareObservedProfile(const DeviceProfile& expectedProfile) {
  const auto& info = *detail::findVariant(_config.variant);
  const uint8_t registers[] = {0x08, 0x09, 0x0A, 0x45};
  const uint8_t actual[] = {_configuration.rawConfig08, _configuration.rawConfig09,
                           _configuration.rawConfig0A, _configuration.rawConfig45};
  const unsigned count = info.hasRdRp ? 4U : 3U;
  for (unsigned i = 0; i < count; ++i) {
    uint8_t expected = 0;
    Status status = detail::encodeProfileRegister(_config.variant, registers[i], expectedProfile,
                                                   expected);
    if (!status.ok()) return status;
    bool equal = false;
    status = detail::comparePersistentSettings(_config.variant, registers[i], actual[i], expected,
                                                equal);
    if (!status.ok()) return status;
    if (!equal) {
      _configuration.mismatchValid = true;
      _configuration.mismatchRegister = registers[i];
      _configuration.mismatchExpected = expected;
      _configuration.mismatchObserved = actual[i];
      _configuration.mismatchMask = comparisonMask(registers[i], info.hasExtendedControls);
      return error(Err::CONFIGURATION_MISMATCH);
    }
  }
  _configuration.mismatchValid = false;
  return Status::Ok();
}

// Publish completed observation, validity, trust and unresolved sequence facts
// separately. A readback cannot erase historical recovery uncertainty.
Status TUSB32x::publishConfiguration() {
  const auto& info = *detail::findVariant(_config.variant);
  const uint16_t applicable = static_cast<uint16_t>(info.hasExtendedControls ?
      (info.hasRdRp ? 0xFFU : 0x7FU) : 0x9BU);
  _configuration.rawConfig08 = _observing.rawConfig08;
  _configuration.rawConfig09 = _observing.rawConfig09;
  _configuration.rawConfig0A = _observing.rawConfig0A;
  _configuration.rawConfig45 = _observing.rawConfig45;
  _configuration.rawConfigValidMask = _observing.rawConfigValidMask;
  _configuration.observedProfile = _observing.observedProfile;
  _configuration.observedValidFields = _observing.observedValidFields;
  _configuration.observedUptimeMs = _observing.observedUptimeMs;
  _configuration.observedTimeValid = true;
  _configuration.hasObservedProfile = (_observing.observedValidFields & applicable) == applicable;
  // SLLSEO5D 6.3.2 p9 promises reset to DRP, not an exact 0x0A reset image.
  const bool resetRoleMismatch = _progress.kind == OperationKind::RESET &&
      _config.variant == Variant::TUSB322I &&
      (_configuration.observedValidFields & static_cast<uint16_t>(ProfileField::PORT_MODE)) != 0 &&
      _configuration.observedProfile.portMode != PortMode::DRP;
  const bool fullProof = _progress.kind != OperationKind::READ_PROFILE;
  if (fullProof) {
    _diagnostics.initialized = true;
    _diagnostics.managedTransport.initialized = true;
    // A prior wait and one complete clear-reset F prove reset settling. Unrelated
    // reserved fields/recovery history may still prevent full configuration trust.
    if (_waitReset && !resetRoleMismatch && (_observing.rawConfig0A & 0x08U) == 0) {
      _configuration.resetState = ResetState::COMPLETED;
      _diagnostics.resetState = ResetState::COMPLETED;
    }
    if (_waitMode && _configuration.hasObservedProfile &&
        _configuration.resetState != ResetState::MAY_BE_PENDING) {
      _diagnostics.modeTransitionUnresolved = false;
    }
  }
  // F is one identity/profile pass (device-behavior.md). A clear reset byte alone cannot
  // discharge an earlier pending reset or a historical recovery sequence.
  const bool settled = !_diagnostics.recoverySequenceUnresolved &&
      !_diagnostics.modeTransitionUnresolved &&
      _configuration.resetState != ResetState::MAY_BE_PENDING;
  // Termination evidence depends on its own disable controls, not an unrelated
  // reserved advertisement or preference code. Reset/mode uncertainty still wins.
  const uint16_t terminationFields = static_cast<uint16_t>(
      (info.hasExtendedControls ? static_cast<uint16_t>(ProfileField::DISABLE_TERM) : 0U) |
      (info.hasRdRp ? static_cast<uint16_t>(ProfileField::DISABLE_RD_RP) : 0U));
  const bool terminationKnown = settled &&
      (_observing.observedValidFields & terminationFields) == terminationFields;
  _configuration.terminationState = TerminationState::UNKNOWN;
  if (terminationKnown) {
    const bool disabled = _configuration.observedProfile.disableTerm ||
        _configuration.observedProfile.disableRdRp;
    _configuration.terminationState =
        disabled ? TerminationState::DISABLED : TerminationState::ENABLED;
  }
  if (_progress.kind == OperationKind::READ_PROFILE) {
    // Per-byte adverse evidence already invalidated any contradicted old proof.
    // P may observe termination independently, but never changes initialization
    // or creates a new verified generation, even when every setting is readable.
    _diagnostics.terminationState = _configuration.terminationState;
    return _configuration.hasObservedProfile ? Status::Ok() : error(Err::CONFIGURATION_UNKNOWN);
  }
  if (!_configuration.hasObservedProfile || !settled) {
    _configuration.state = ConfigurationState::UNKNOWN;
  } else {
    _configuration.state = ConfigurationState::KNOWN;
    _configuration.verifiedProfile = _configuration.observedProfile;
    _configuration.hasVerifiedProfile = true;
    _configuration.verifiedGeneration = detail::nextGeneration(_configuration.verifiedGeneration);
    _diagnostics.configurationWriteUnresolved = false;
    _rdRpWriteImageValid = info.hasRdRp && (_observing.rawConfig45 & 0x03U) == 0;
    _modeWriteImageValid = true;
  }
  _diagnostics.configurationState = _configuration.state;
  _diagnostics.terminationState = _configuration.terminationState;
  if (_progress.kind == OperationKind::RESET) {
    if ((_observing.rawConfig0A & 0x08U) != 0) return error(Err::CONFIGURATION_UNKNOWN);
    if (resetRoleMismatch) return error(_configuration.hasObservedProfile ?
        Err::CONFIGURATION_MISMATCH : Err::CONFIGURATION_UNKNOWN);
    // Prove this reset independently of an older interrupted recovery recipe.
    return _configuration.hasObservedProfile &&
        _configuration.resetState == ResetState::COMPLETED &&
        !_diagnostics.modeTransitionUnresolved ? Status::Ok() : error(Err::CONFIGURATION_UNKNOWN);
  }
  if (_configuration.state != ConfigurationState::KNOWN) return error(Err::CONFIGURATION_UNKNOWN);
  if ((_progress.kind == OperationKind::RECONCILE ||
       _progress.kind == OperationKind::VERIFY_PROFILE ||
       _progress.kind == OperationKind::APPLY_PROFILE) && _configuration.hasDesiredProfile)
    return compareObservedProfile(_configuration.desiredProfile);
  return Status::Ok();
}

Status TUSB32x::prepareInterruptAck(uint8_t actual09) {
  _result.interrupt.observationValid = true;
  _result.interrupt.pendingBefore = (actual09 & IRQ_PENDING) != 0;
  _diagnostics.interrupt = _result.interrupt;
  detail::WriteRequest request{};
  request.variant = _config.variant;
  request.reg = 0x09;
  request.intent = detail::WriteIntent::ACK_IRQ;
  detail::WriteEvidence evidence{};
  evidence.actual = actual09;
  evidence.actualValid = true;
  const Status status = detail::composeWrite(request, evidence, _commandByte);
  if (status.ok()) {
    _phase = Phase::IRQ_ACK;
    _progress.phase = OperationPhase::WRITING;
  }
  return status;
}

Status TUSB32x::poll(OperationToken token, uint64_t nowMs, uint8_t maxTransfers) {
  Status status = checkToken(token);
  if (!status.ok()) return status;
  if (_progress.resultPending) return _result.status;
  if (nowMs - _lastTime >= detail::HALF_RANGE) return error(Err::INVALID_PARAM);
  _lastTime = nowMs;
  _progress.lastPollTransfers = 0;
  if (nowMs - _progress.deadlineMs < detail::HALF_RANGE)
    return finish(error(Err::TIMEOUT), true, nowMs);
  // A conservative anchor is the first fresh poll, never admission or a stale
  // pre-callback timestamp. TI reset max 95 ms: SLLSEN9F section 6.7 p7,
  // SLLSEP2D/SLLSEQ8D section 6.7 p6 and SLLSEO5D section 5.7 p7.
  // Extended 320 mode wait 5 ms: SLLSEP2D/SLLSEQ8D section 7.3.2 p14.
  // OWNER_CLOCK_MARGIN_MS is separate library quantization allowance. No extra
  // 5 ms wait is specified for 322I mode changes (SLLSEO5D sections 6.3.2.1-2 pp10-11).
  if (_phase == Phase::WAIT) {
    if (!_progress.readyAtValid) {
      const auto& info = *detail::findVariant(_config.variant);
      const uint64_t ticks = _waitReset ? 95 + OWNER_CLOCK_MARGIN_MS :
          info.hasExtendedControls && info.hasRdRp ? 5 + OWNER_CLOCK_MARGIN_MS : 0;
      _progress.readyAtMs = nowMs + ticks;
      _progress.readyAtValid = true;
    }
    if (nowMs - _progress.readyAtMs >= detail::HALF_RANGE) return error(Err::IN_PROGRESS);
    _progress.waiting = false;
    _progress.readyAtValid = false;
    _progress.phase = OperationPhase::READING;
    _phase = Phase::IDENTITY;
  }
  // Direct acyclic pass: validate, wait, consume pending bytes, finish or invoke one callback.
  if (_pending) {
    _pending = false;
    if (!_pendingStatus.ok()) return finish(_pendingStatus, true, nowMs);
    if (_phase == Phase::PORT_FIRST09) {
      _observingPort.rawStatus09 = _received[0];
      _phase = Phase::PORT08;
    } else if (_phase == Phase::PORT08) {
      _observingPort.rawStatus08 = _received[0];
      _phase = Phase::PORT_LAST09;
    } else if (_phase == Phase::PORT_LAST09) {
      PortSnapshot complete{};
      const Status decoded = detail::decodePort(_config.variant, _observingPort.rawStatus09,
          _observingPort.rawStatus08, _received[0], complete);
      if (!decoded.ok()) return finish(decoded, true, nowMs);
      complete.sequence = detail::nextSequence(_port.sequence);
      complete.observedUptimeMs = _observingPort.observedUptimeMs;
      complete.observedTimeValid = true;
      complete.configGeneration = _observingPort.configGeneration;
      complete.configurationState = _observingPort.configurationState;
      complete.invalidated = false;
      _port = complete;
      if (_progress.kind == OperationKind::REFRESH_PORT) return finish(Status::Ok(), true, nowMs);
      const Status prepared = prepareInterruptAck(_received[0]);
      if (!prepared.ok()) return finish(prepared, true, nowMs);
    } else if (_phase == Phase::IRQ_PRECHECK) {
      const Status prepared = prepareInterruptAck(_received[0]);
      if (!prepared.ok()) return finish(prepared, true, nowMs);
    } else if (_phase == Phase::IRQ_ACK) {
      if (_progress.kind == OperationKind::ACKNOWLEDGE_INTERRUPT)
        return finish(Status::Ok(), true, nowMs);
      _phase = Phase::IRQ_RECHECK;
      _progress.phase = OperationPhase::READING;
    } else if (_phase == Phase::IRQ_RECHECK) {
      _result.interrupt.pendingAfter = (_received[0] & IRQ_PENDING) != 0;
      _result.interrupt.pendingAfterValid = true;
      _diagnostics.interrupt = _result.interrupt;
      return finish(Status::Ok(), true, nowMs);
    } else if (_phase == Phase::BASE_PRECHECK) {
      // TI SLLSEN9F 7.4.1-2 p14: detached before any base mode write.
      // The owner/fixture must prevent attachment racing this advisory sample.
      if ((_received[0] & 0xC0U) != 0 || _configuration.state != ConfigurationState::KNOWN)
        return finish(error(Err::CONFIGURATION_UNKNOWN), true, nowMs);
      _phase = Phase::APPLY_WRITE;
      _progress.phase = OperationPhase::WRITING;
    } else if (_phase == Phase::RESET_PRECHECK) {
      // TI reset command bit 3: SLLSEN9F Table 7-7 p19,
      // SLLSEP2D/SLLSEQ8D Table 10 p19 and SLLSEO5D Table 6-10 p20.
      // Compose from fresh actual controls, never desired.
      detail::WriteRequest request{};
      request.variant = _config.variant;
      request.reg = 0x0A;
      request.intent = detail::WriteIntent::RESET;
      detail::WriteEvidence evidence{};
      evidence.actual = _received[0];
      evidence.actualValid = true;
      const Status composed = detail::composeWrite(request, evidence, _commandByte);
      if (!composed.ok()) return finish(composed, true, nowMs);
      _phase = Phase::RESET_WRITE;
      _progress.phase = OperationPhase::WRITING;
    } else if (_phase == Phase::RECOVERY_PRECHECK) {
      // SLLSEN9F 7.4.3 p14 + Table7-7 p19: zero-internal precheck is library
      // policy qualifying TI's literal 04/00 writes; ordinary writes preserve bits.
      if ((_received[0] & 0x03U) != 0)
        return finish(error(Err::CONFIGURATION_UNKNOWN), true, nowMs);
      _phase = Phase::RECOVERY_DISABLE;
      _progress.phase = OperationPhase::WRITING;
    } else if (_phase == Phase::RESET_WRITE || _phase == Phase::RECOVERY_DISABLE) {
      // This is the first fresh sample after the successful callback. The
      // earlier pre-write nowMs cannot anchor either interval; late polls add
      // conservatism without moving the immutable operation deadline.
      // Recovery 30 ms: SLLSEN9F section 7.4.3 p14; reset duration cited above.
      const uint64_t ticks = _phase == Phase::RESET_WRITE ? 95 : 30;
      _phase = Phase::PROCEDURE_WAIT;
      _progress.phase = OperationPhase::WAITING;
      _progress.waiting = true;
      _progress.readyAtValid = true;
      _progress.readyAtMs = nowMs + ticks + OWNER_CLOCK_MARGIN_MS;
    } else if (_phase == Phase::RECOVERY_ENABLE) {
      _phase = Phase::RECOVERY_VERIFY;
      _progress.phase = OperationPhase::READING;
    } else if (_phase == Phase::RECOVERY_VERIFY) {
      const bool complete = (_received[0] & 0x07U) == 0;
      if (complete) _diagnostics.recoverySequenceUnresolved = false;
      // Only the base part has this procedure and its sole termination control
      // is 45[2]. This read proves no complete profile or historical failed job.
      _configuration.terminationState = (_received[0] & 0x03U) != 0 ?
          TerminationState::UNKNOWN : complete ? TerminationState::ENABLED : TerminationState::DISABLED;
      _diagnostics.terminationState = _configuration.terminationState;
      return finish(complete ? Status::Ok() : error(Err::CONFIGURATION_MISMATCH), true, nowMs);
    } else if (_phase == Phase::APPLY_WRITE) {
      const bool wait = _writes[_writeIndex++].waitAfter;
      if (wait) {
        _phase = Phase::APPLY_WAIT;
        _progress.phase = OperationPhase::WAITING;
        _progress.waiting = true;
        _progress.readyAtValid = true;
        _progress.readyAtMs = nowMs + 5 + OWNER_CLOCK_MARGIN_MS;
      } else if (_writeIndex == _writeCount) {
        beginApplyProof();
      }
    } else {
      acceptRead();
      if (!_pendingStatus.ok()) return finish(_pendingStatus, true, nowMs);
      if (_phase == Phase::DONE) return finish(publishConfiguration(), true, nowMs);
    }
  }
  if (_phase == Phase::APPLY_WAIT) {
    if (nowMs - _progress.readyAtMs >= detail::HALF_RANGE) return error(Err::IN_PROGRESS);
    _progress.waiting = false;
    _progress.readyAtValid = false;
    if (_writeIndex == _writeCount) beginApplyProof();
    else {
      _phase = Phase::APPLY_WRITE;
      _progress.phase = OperationPhase::WRITING;
    }
  }
  if (_phase == Phase::PROCEDURE_WAIT) {
    if (nowMs - _progress.readyAtMs >= detail::HALF_RANGE) return error(Err::IN_PROGRESS);
    _progress.waiting = false;
    _progress.readyAtValid = false;
    _phase = _progress.kind == OperationKind::RESET ? Phase::IDENTITY : Phase::RECOVERY_ENABLE;
    _progress.phase = _progress.kind == OperationKind::RESET ?
        OperationPhase::READING : OperationPhase::WRITING;
  }
  if (maxTransfers == 0) return error(Err::IN_PROGRESS);
  if (_phase == Phase::IRQ_ACK) {
    // A pure W1C ACK preserves persistent controls and historical port evidence.
    // TI SLLSEN9F Table 7-6 p16 and Table 7-7 p18;
    // SLLSEP2D/SLLSEQ8D Table 9 p18. No IRQ inter-clear wait is specified.
    _result.interrupt.acknowledgeAttempted = true;
    _diagnostics.interrupt = _result.interrupt;
    _result.hardwareEffect = HardwareEffect::INDETERMINATE;
    const detail::ProtocolContext context{_config, _stats, _diagnostics};
    const auto transfer = detail::writeRegister(context, detail::TrafficSource::MANAGED,
        0x09, _commandByte, _progress.deadlineMs - nowMs);
    if (transfer.callbackInvoked) {
      ++_progress.transfers;
      _progress.lastPollTransfers = 1;
    }
    if (!transfer.status.ok()) return finish(transfer.status);
    _result.interrupt.acknowledgeConfirmed = true;
    _diagnostics.interrupt = _result.interrupt;
    _result.hardwareEffect = HardwareEffect::PARTIAL;
    _pending = true;
    return error(Err::IN_PROGRESS);
  }
  if (_phase == Phase::APPLY_WRITE || _phase == Phase::RESET_WRITE ||
      _phase == Phase::RECOVERY_DISABLE || _phase == Phase::RECOVERY_ENABLE) {
    const bool apply = _phase == Phase::APPLY_WRITE;
    const bool reset = _phase == Phase::RESET_WRITE;
    const uint8_t reg = apply ? _writes[_writeIndex].reg : reset ? 0x0A : 0x45;
    const uint8_t value = apply ? _writes[_writeIndex].value : reset ? _commandByte :
        _phase == Phase::RECOVERY_DISABLE ? 0x04 : 0x00;
    // A terminal callback may apply data before returning an error. Publish the
    // possibility before invoking it; callback success alone is not final proof.
    invalidateConfiguration();
    if (apply) {
      _configuration.state = ConfigurationState::APPLYING;
      _diagnostics.configurationState = ConfigurationState::APPLYING;
      if (_writes[_writeIndex].modeStep) _diagnostics.modeTransitionUnresolved = true;
    } else if (reset) {
      _configuration.resetState = ResetState::MAY_BE_PENDING;
      _diagnostics.resetState = ResetState::MAY_BE_PENDING;
      _waitReset = true;
    } else {
      _diagnostics.recoverySequenceUnresolved = true;
    }
    _diagnostics.configurationWriteUnresolved = true;
    _result.hardwareEffect = HardwareEffect::INDETERMINATE;
    const detail::ProtocolContext context{_config, _stats, _diagnostics};
    const auto transfer = detail::writeRegister(context, detail::TrafficSource::MANAGED,
        reg, value, _progress.deadlineMs - nowMs);
    if (transfer.callbackInvoked) {
      ++_progress.transfers;
      _progress.lastPollTransfers = 1;
    }
    if (!transfer.status.ok()) {
      if (reset) _diagnostics.resetAcceptanceUncertain = true;
      return finish(transfer.status);
    }
    _result.hardwareEffect = HardwareEffect::PARTIAL;
    if (_phase == Phase::RESET_WRITE || _phase == Phase::RECOVERY_DISABLE) {
      _progress.phase = OperationPhase::WAITING;
      _progress.waiting = true;
      // The status-only callback supplied no completion timestamp.
      _progress.readyAtValid = false;
    }
    _pending = true;
    return error(Err::IN_PROGRESS);
  }
  const uint8_t reg = _phase == Phase::IDENTITY ? 0 : _phase == Phase::REVISION ? 0xA0 :
      (_phase == Phase::CONFIG08 || _phase == Phase::PORT08) ? 8 :
      (_phase == Phase::CONFIG09 || _phase == Phase::BASE_PRECHECK ||
       _phase == Phase::PORT_FIRST09 || _phase == Phase::PORT_LAST09 ||
       _phase == Phase::IRQ_PRECHECK || _phase == Phase::IRQ_RECHECK) ? 9 :
      (_phase == Phase::CONFIG0A || _phase == Phase::RESET_PRECHECK) ? 10 : 0x45;
  if (_phase == Phase::CONFIG08) _observing.observedUptimeMs = nowMs;
  if (_phase == Phase::PORT_FIRST09) {
    // Conservative acquisition boundary before the first required callback.
    _observingPort.observedUptimeMs = nowMs;
    _observingPort.configGeneration = _configuration.verifiedGeneration;
    _observingPort.configurationState = _configuration.state;
  }
  const detail::ProtocolContext context{_config, _stats, _diagnostics};
  const auto transfer = detail::readRegisters(context, detail::TrafficSource::MANAGED, reg,
      _received, _phase == Phase::IDENTITY ? 8U : 1U, _progress.deadlineMs - nowMs);
  if (transfer.callbackInvoked) {
    ++_progress.transfers;
    _progress.lastPollTransfers = 1;
  }
  if (!transfer.status.ok()) return finish(transfer.status);
  _pending = true;
  if (_phase == Phase::IDENTITY) {
    _pendingStatus = detail::checkIdentity(_config.variant, _received, 8);
  } else if (_phase == Phase::REVISION) {
    _pendingStatus = detail::checkRevision(_config.variant, _received[0]);
  } else {
    const bool invalidated = _port.invalidated;
    observeControl(reg, _received[0]);
    // Negative control evidence changes current configuration trust immediately.
    // Port/IRQ observations preserve historical port data until a complete S is
    // published. ACK/recheck do not invalidate that complete pre-ACK publication.
    if (_progress.kind == OperationKind::REFRESH_PORT ||
        _progress.kind == OperationKind::ACKNOWLEDGE_INTERRUPT ||
        _progress.kind == OperationKind::SERVICE_INTERRUPT) _port.invalidated = invalidated;
  }
  if (!_pendingStatus.ok()) {
    // Complete adverse identity evidence must survive cancel/expiry before the
    // fresh poll. Matching acquisition is published only by acceptRead instead.
    publishIdentityRead();
    invalidateUsability();
  }
  return error(Err::IN_PROGRESS);
}
}  // namespace TUSB32x
