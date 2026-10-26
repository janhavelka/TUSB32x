/// @file TUSB32xOperations.cpp
/// @brief Bus-silent lifecycle and bounded identity/profile observation engine.
#include "TUSB32x/TUSB32x.h"

#include "TUSB32x/CommandTable.h"
#include "TUSB32xProtocol.h"
#include "TUSB32xCodecs.h"
#include "TUSB32xOwnerTime.h"

#include <cstring>
#include <limits>

namespace TUSB32x {
namespace {

Status error(Err code) { return Status::Error(code, errorName(code)); }

void increment(uint32_t& value) {
  if (value != std::numeric_limits<uint32_t>::max()) ++value;
}

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

void TUSB32x::invalidateConfiguration() {
  _configuration.state = ConfigurationState::UNKNOWN;
  _configuration.terminationState = TerminationState::UNKNOWN;
  _diagnostics.configurationState = ConfigurationState::UNKNOWN;
  _diagnostics.terminationState = TerminationState::UNKNOWN;
  _port.invalidated = true;
}

void TUSB32x::invalidateUsability() {
  _diagnostics.initialized = false;
  _diagnostics.managedTransport.initialized = false;
  if (_configuration.state != ConfigurationState::UNCONFIGURED)
    _configuration.state = ConfigurationState::UNKNOWN;
  _configuration.terminationState = TerminationState::UNKNOWN;
  _diagnostics.configurationState = _configuration.state;
  _diagnostics.terminationState = TerminationState::UNKNOWN;
  _port.invalidated = true;
}

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

Status TUSB32x::checkToken(OperationToken token) const {
  if (!token.valid()) return error(Err::INVALID_PARAM);
  if (!_progress.token.valid()) return error(Err::RESULT_NOT_AVAILABLE);
  if (token.value != _progress.token.value) return error(Err::TOKEN_MISMATCH);
  return Status::Ok();
}

Status TUSB32x::finish(Status status, bool timeValid, uint64_t nowMs) {
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
  return startObservation(OperationKind::INITIALIZE, timing, token);
}

Status TUSB32x::startReadProfile(const OperationTiming& timing, OperationToken& token) {
  return startObservation(OperationKind::READ_PROFILE, timing, token);
}

Status TUSB32x::startReconcile(const OperationTiming& timing, OperationToken& token) {
  return startObservation(OperationKind::RECONCILE, timing, token);
}

Status TUSB32x::startObservation(OperationKind kind, const OperationTiming& timing,
                               OperationToken& token) {
  const uint64_t interval = timing.deadlineMs - timing.nowMs;
  if (interval == 0 || interval >= detail::HALF_RANGE) return error(Err::INVALID_PARAM);
  if (!_diagnostics.bound) return error(Err::NOT_BOUND);
  if (_progress.state == OperationState::ACTIVE) return error(Err::BUSY);
  if (_progress.resultPending) return error(Err::RESULT_PENDING);
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
  _waitReset = kind == OperationKind::RECONCILE &&
      _configuration.resetState == ResetState::MAY_BE_PENDING;
  _waitMode = kind == OperationKind::RECONCILE && _diagnostics.modeTransitionUnresolved;
  _phase = kind == OperationKind::READ_PROFILE ? ReadPhase::CONFIG08 : ReadPhase::IDENTITY;
  if (_waitReset || _waitMode) {
    _phase = ReadPhase::WAIT;
    _progress.phase = OperationPhase::WAITING;
    _progress.waiting = true;
  }
  _observing = {};
  _pending = false;
  _pendingStatus = Status::Ok();
  token = _progress.token;
  return error(Err::IN_PROGRESS);
}

void TUSB32x::publishIdentityRead() {
  auto& identity = _diagnostics.identity;
  if (_phase == ReadPhase::IDENTITY) {
    std::memcpy(identity.rawIdentity, _received, sizeof(identity.rawIdentity));
    // A matching signature during reconciliation does not erase an older complete
    // revision if the following read fails. Contradiction still invalidates it now.
    if (_progress.kind != OperationKind::RECONCILE || !identity.identityMatches ||
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

void TUSB32x::acceptRead() {
  // Fixed F plan: TI SLLSEN9F Table 7-7 pp16-19; SLLSEP2D/SLLSEQ8D
  // Tables 7-12 pp16-20; SLLSEO5D Tables 6-6..6-11 pp17-20.
  // Read identity then each applicable persistent-control byte once; no writes,
  // reset-bit completion assumption, IRQ consumption or dead-battery recovery.
  const auto& info = *detail::findVariant(_config.variant);
  if (_phase == ReadPhase::IDENTITY) {
    publishIdentityRead();
    _phase = info.hasRevision ? ReadPhase::REVISION : ReadPhase::CONFIG08;
    return;
  }
  if (_phase == ReadPhase::REVISION) {
    publishIdentityRead();
    _phase = ReadPhase::CONFIG08;
    return;
  }
  const uint8_t reg = _phase == ReadPhase::CONFIG08 ? 8 : _phase == ReadPhase::CONFIG09 ? 9 :
      _phase == ReadPhase::CONFIG0A ? 10 : 0x45;
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
    _phase = ReadPhase::CONFIG09;
  } else if (reg == 9) {
    _observing.rawConfig09 = _received[0];
    _observing.rawConfigValidMask |= CONFIG_09_VALID;
    profile.drpDutyCycle = decoded.profileCodes.drpDutyCycle;
    profile.disableUfpAccessory = decoded.profileCodes.disableUfpAccessory;
    _phase = ReadPhase::CONFIG0A;
  } else if (reg == 10) {
    _observing.rawConfig0A = _received[0];
    _observing.rawConfigValidMask |= CONFIG_0A_VALID;
    profile.portMode = decoded.profileCodes.portMode;
    profile.sourcePreference = decoded.profileCodes.sourcePreference;
    profile.ccDebounce = decoded.profileCodes.ccDebounce;
    profile.disableTerm = decoded.profileCodes.disableTerm;
    _phase = info.hasRdRp ? ReadPhase::CONFIG45 : ReadPhase::DONE;
  } else {
    _observing.rawConfig45 = _received[0];
    _observing.rawConfigValidMask |= CONFIG_45_VALID;
    profile.disableRdRp = decoded.profileCodes.disableRdRp;
    _phase = ReadPhase::DONE;
  }
}

void TUSB32x::observeControl(uint8_t reg, uint8_t value) {
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
  const bool fullProof = _progress.kind != OperationKind::READ_PROFILE;
  if (fullProof) {
    _diagnostics.initialized = true;
    _diagnostics.managedTransport.initialized = true;
    // A prior wait and one complete clear-reset F prove reset settling. Unrelated
    // reserved fields/recovery history may still prevent full configuration trust.
    if (_waitReset && (_observing.rawConfig0A & 0x08U) == 0) {
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
    increment(_configuration.verifiedGeneration);
    _diagnostics.configurationWriteUnresolved = false;
  }
  _diagnostics.configurationState = _configuration.state;
  _diagnostics.terminationState = _configuration.terminationState;
  if (_configuration.state != ConfigurationState::KNOWN) return error(Err::CONFIGURATION_UNKNOWN);
  if (_progress.kind == OperationKind::RECONCILE && _configuration.hasDesiredProfile)
    return compareObservedProfile(_configuration.desiredProfile);
  return Status::Ok();
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
  // pre-callback timestamp. TI reset max95 ms / extended mode min5 ms, plus
  // OWNER_CLOCK_MARGIN_MS: SLLSEN9F 6.7; SLLSEP2D/SLLSEQ8D 6.7,7.3.2;
  // SLLSEO5D 5.7. No extra 5 ms wait is specified for 322I mode changes.
  if (_phase == ReadPhase::WAIT) {
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
    _phase = ReadPhase::IDENTITY;
  }
  // Direct acyclic pass: validate, wait, consume pending bytes, finish or invoke one callback.
  if (_pending) {
    _pending = false;
    if (!_pendingStatus.ok()) return finish(_pendingStatus, true, nowMs);
    acceptRead();
    if (!_pendingStatus.ok()) return finish(_pendingStatus, true, nowMs);
    if (_phase == ReadPhase::DONE) return finish(publishConfiguration(), true, nowMs);
  }
  if (maxTransfers == 0) return error(Err::IN_PROGRESS);
  const uint8_t reg = _phase == ReadPhase::IDENTITY ? 0 : _phase == ReadPhase::REVISION ? 0xA0 :
      _phase == ReadPhase::CONFIG08 ? 8 : _phase == ReadPhase::CONFIG09 ? 9 :
      _phase == ReadPhase::CONFIG0A ? 10 : 0x45;
  if (_phase == ReadPhase::CONFIG08) _observing.observedUptimeMs = nowMs;
  const detail::ProtocolContext context{_config, _stats, _diagnostics};
  const auto transfer = detail::readRegisters(context, detail::TrafficSource::MANAGED, reg,
      _received, _phase == ReadPhase::IDENTITY ? 8U : 1U, _progress.deadlineMs - nowMs);
  if (transfer.callbackInvoked) {
    ++_progress.transfers;
    _progress.lastPollTransfers = 1;
  }
  if (!transfer.status.ok()) return finish(transfer.status);
  _pending = true;
  if (_phase == ReadPhase::IDENTITY) {
    _pendingStatus = detail::checkIdentity(_config.variant, _received, 8);
  } else if (_phase == ReadPhase::REVISION) {
    _pendingStatus = detail::checkRevision(_config.variant, _received[0]);
  } else {
    observeControl(reg, _received[0]);
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
