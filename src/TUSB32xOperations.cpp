/// @file TUSB32xOperations.cpp
/// @brief Bus-silent lifecycle and one bounded read-only initialization engine.
#include "TUSB32x/TUSB32x.h"

#include "TUSB32x/CommandTable.h"
#include "TUSB32xProtocol.h"
#include "TUSB32xOwnerTime.h"

#include <cstring>
#include <limits>

namespace TUSB32x {
namespace {

Status error(Err code) { return Status::Error(code, errorName(code)); }

void increment(uint32_t& value) {
  if (value != std::numeric_limits<uint32_t>::max()) ++value;
}

}  // namespace

bool TUSB32x::unresolved() const {
  return _diagnostics.configurationWriteUnresolved || _diagnostics.recoverySequenceUnresolved ||
      _diagnostics.modeTransitionUnresolved || _diagnostics.resetAcceptanceUncertain ||
      _configuration.resetState == ResetState::MAY_BE_PENDING;
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
  const uint64_t interval = timing.deadlineMs - timing.nowMs;
  if (interval == 0 || interval >= detail::HALF_RANGE) return error(Err::INVALID_PARAM);
  if (!_diagnostics.bound) return error(Err::NOT_BOUND);
  if (_progress.state == OperationState::ACTIVE) return error(Err::BUSY);
  if (_progress.resultPending) return error(Err::RESULT_PENDING);
  uint64_t next = 0;
  if (!detail::nextToken(_lastToken, next)) return error(Err::TOKEN_EXHAUSTED);
  _lastToken = next;
  invalidateUsability();
  _progress = {};
  _progress.token = {next};
  _progress.kind = OperationKind::INITIALIZE;
  _progress.state = OperationState::ACTIVE;
  _progress.phase = OperationPhase::READING;
  _progress.deadlineMs = timing.deadlineMs;
  _result = {};
  _result.token = _progress.token;
  _result.kind = _progress.kind;
  _result.startedUptimeMs = timing.nowMs;
  _lastTime = timing.nowMs;
  _phase = InitPhase::IDENTITY;
  _observing = {};
  _pending = false;
  _pendingStatus = Status::Ok();
  token = _progress.token;
  return error(Err::IN_PROGRESS);
}

void TUSB32x::publishIdentityRead() {
  auto& identity = _diagnostics.identity;
  if (_phase == InitPhase::IDENTITY) {
    std::memcpy(identity.rawIdentity, _received, sizeof(identity.rawIdentity));
    identity.identityValid = true;
    identity.identityMatches = _pendingStatus.ok();
    identity.revisionValid = false;
    identity.revisionMatches = false;
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
  if (_phase == InitPhase::IDENTITY) {
    publishIdentityRead();
    _phase = info.hasRevision ? InitPhase::REVISION : InitPhase::CONFIG08;
    return;
  }
  if (_phase == InitPhase::REVISION) {
    publishIdentityRead();
    _phase = InitPhase::CONFIG08;
    return;
  }
  const uint8_t reg = _phase == InitPhase::CONFIG08 ? 8 : _phase == InitPhase::CONFIG09 ? 9 :
      _phase == InitPhase::CONFIG0A ? 10 : 0x45;
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
    _phase = InitPhase::CONFIG09;
  } else if (reg == 9) {
    _observing.rawConfig09 = _received[0];
    _observing.rawConfigValidMask |= CONFIG_09_VALID;
    profile.drpDutyCycle = decoded.profileCodes.drpDutyCycle;
    profile.disableUfpAccessory = decoded.profileCodes.disableUfpAccessory;
    _phase = InitPhase::CONFIG0A;
  } else if (reg == 10) {
    _observing.rawConfig0A = _received[0];
    _observing.rawConfigValidMask |= CONFIG_0A_VALID;
    profile.portMode = decoded.profileCodes.portMode;
    profile.sourcePreference = decoded.profileCodes.sourcePreference;
    profile.ccDebounce = decoded.profileCodes.ccDebounce;
    profile.disableTerm = decoded.profileCodes.disableTerm;
    _phase = info.hasRdRp ? InitPhase::CONFIG45 : InitPhase::DONE;
  } else {
    _observing.rawConfig45 = _received[0];
    _observing.rawConfigValidMask |= CONFIG_45_VALID;
    profile.disableRdRp = decoded.profileCodes.disableRdRp;
    _phase = InitPhase::DONE;
  }
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
  _diagnostics.initialized = true;
  _diagnostics.managedTransport.initialized = true;
  // F is one identity/profile pass (device-behavior.md). A clear reset byte alone cannot
  // discharge an earlier pending reset or a historical recovery sequence.
  const bool settled = !_diagnostics.recoverySequenceUnresolved &&
      !_diagnostics.modeTransitionUnresolved && _configuration.resetState != ResetState::MAY_BE_PENDING;
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
    _configuration.terminationState = disabled ? TerminationState::DISABLED : TerminationState::ENABLED;
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
  return _configuration.state == ConfigurationState::KNOWN ? Status::Ok() : error(Err::CONFIGURATION_UNKNOWN);
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
  // Direct acyclic pass: validate, consume pending bytes, finish or invoke one callback.
  if (_pending) {
    _pending = false;
    if (!_pendingStatus.ok()) return finish(_pendingStatus, true, nowMs);
    acceptRead();
    if (!_pendingStatus.ok()) return finish(_pendingStatus, true, nowMs);
    if (_phase == InitPhase::DONE) return finish(publishConfiguration(), true, nowMs);
  }
  if (maxTransfers == 0) return error(Err::IN_PROGRESS);
  const uint8_t reg = _phase == InitPhase::IDENTITY ? 0 : _phase == InitPhase::REVISION ? 0xA0 :
      _phase == InitPhase::CONFIG08 ? 8 : _phase == InitPhase::CONFIG09 ? 9 :
      _phase == InitPhase::CONFIG0A ? 10 : 0x45;
  if (_phase == InitPhase::CONFIG08) _observing.observedUptimeMs = nowMs;
  const detail::ProtocolContext context{_config, _stats, _diagnostics};
  const auto transfer = detail::readRegisters(context, detail::TrafficSource::MANAGED, reg,
      _received, _phase == InitPhase::IDENTITY ? 8U : 1U, _progress.deadlineMs - nowMs);
  if (transfer.callbackInvoked) {
    ++_progress.transfers;
    _progress.lastPollTransfers = 1;
  }
  if (!transfer.status.ok()) return finish(transfer.status);
  _pending = true;
  if (_phase == InitPhase::IDENTITY) {
    _pendingStatus = detail::checkIdentity(_config.variant, _received, 8);
  } else if (_phase == InitPhase::REVISION) {
    _pendingStatus = detail::checkRevision(_config.variant, _received[0]);
  } else if (_phase == InitPhase::CONFIG0A && (_received[0] & 8U) != 0) {
    _configuration.resetState = ResetState::MAY_BE_PENDING;
    _diagnostics.resetState = ResetState::MAY_BE_PENDING;
    _configuration.state = ConfigurationState::UNKNOWN;
    _diagnostics.configurationState = ConfigurationState::UNKNOWN;
    _port.invalidated = true;
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
