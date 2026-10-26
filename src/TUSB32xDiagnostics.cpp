/// @file TUSB32xDiagnostics.cpp
/// @brief One-callback raw diagnostics using shared composition/accounting, and pure passive health.
#include "TUSB32x/TUSB32x.h"

#include "TUSB32xCodecs.h"
#include "TUSB32xProtocol.h"

namespace TUSB32x {
namespace {

Status error(Err code) { return Status::Error(code, errorName(code)); }
constexpr uint16_t field(ProfileField value) { return static_cast<uint16_t>(value); }

// TI SLLSEN9F Table7-7 pp17-19; SLLSEP2D/SLLSEQ8D Tables8-11 pp17-20;
// SLLSEO5D Tables6-8..6-10 pp17-20. This rejects illegal raw images, rather than
// silently clearing RO/reserved bits. It grants no write/sequence permission.
Status validateRawByte(const detail::VariantInfo* info, uint8_t reg, uint8_t value) {
  const bool base = info != nullptr && !info->hasExtendedControls;
  switch (reg) {
    case 0x08:
      if ((value & 0x3FU) != 0 || (value & 0xC0U) == 0xC0) return error(Err::INVALID_PARAM);
      break;
    case 0x09:
      if ((value & 0xE8U) != 0 || (base && (value & 0x01U) != 0))
        return error(Err::INVALID_PARAM);
      break;
    case 0x0A:
      if ((value & 0x08U) != 0) return error(Err::UNSUPPORTED_FEATURE);
      if ((base && (value & 0x07U) != 0) || (value & 0x06U) == 0x04)
        return error(Err::INVALID_PARAM);
      break;
    case 0x45:
      if ((value & 0xF8U) != 0) return error(Err::INVALID_PARAM);
      break;
    default: break;  // Access validation has rejected other write registers.
  }
  return Status::Ok();
}

}  // namespace

Status validateHealthPolicy(const HealthPolicy& policy) {
  return policy.offlineThreshold == 0 ? error(Err::INVALID_CONFIG) : Status::Ok();
}

Status projectHealth(const ManagedTransportEvidence& evidence, const HealthPolicy& policy,
                     HealthSnapshot& out) {
  const Status status = validateHealthPolicy(policy);
  if (!status.ok()) return status;
  HealthSnapshot projected{};
  projected.evidence = evidence;
  projected.offlineThreshold = policy.offlineThreshold;
  if (!evidence.initialized) projected.state = DriverState::UNINIT;
  else if (evidence.consecutiveFailures == 0) projected.state = DriverState::READY;
  else if (evidence.consecutiveFailures < policy.offlineThreshold)
    projected.state = DriverState::DEGRADED;
  else projected.state = DriverState::OFFLINE;
  out = projected;
  return Status::Ok();
}

Status TUSB32x::readRegisterSync(uint8_t reg, uint8_t& out) {
  const Variant variant = _diagnostics.bound ? _config.variant : Variant::UNSPECIFIED;
  const Status access = detail::validateRegisterAccess(variant, reg, false);
  if (!access.ok()) return access;
  if (!_diagnostics.bound) return error(Err::NOT_BOUND);
  if (_progress.state == OperationState::ACTIVE) return error(Err::BUSY);
  if (_progress.resultPending) return error(Err::RESULT_PENDING);
  uint8_t value = 0;
  const detail::ProtocolContext context{_config, _stats, _diagnostics};
  const auto transfer = detail::readRegisters(context, detail::TrafficSource::RAW, reg,
                                              &value, 1, _config.i2cTimeoutMs);
  if (!transfer.status.ok()) return transfer.status;
  if ((reg >= 0x08 && reg <= 0x0A) || reg == 0x45) observeControl(reg, value);
  out = value;
  return transfer.status;
}

Status TUSB32x::writeRegisterSync(uint8_t reg, uint8_t value) {
  const Variant variant = _diagnostics.bound ? _config.variant : Variant::UNSPECIFIED;
  const Status access = detail::validateRegisterAccess(variant, reg, true);
  if (!access.ok()) return access;
  const auto* info = _diagnostics.bound ? detail::findVariant(variant) : nullptr;
  const Status valid = validateRawByte(info, reg, value);
  if (!valid.ok()) return valid;
  if (!_diagnostics.bound) return error(Err::NOT_BOUND);
  if (_progress.state == OperationState::ACTIVE) return error(Err::BUSY);
  if (_progress.resultPending) return error(Err::RESULT_PENDING);
  if (!_diagnostics.initialized) return error(Err::NOT_INITIALIZED);
  // Historical ambiguous reset acceptance may survive settled KNOWN proof. It
  // must not be confused with a currently pending reset or unresolved transition.
  if (_configuration.state != ConfigurationState::KNOWN || !_configuration.hasVerifiedProfile ||
      _diagnostics.configurationWriteUnresolved || _diagnostics.recoverySequenceUnresolved ||
      _diagnostics.modeTransitionUnresolved || _configuration.resetState == ResetState::MAY_BE_PENDING)
    return error(Err::CONFIGURATION_UNKNOWN);

  detail::WriteRequest request{};
  request.variant = variant;
  request.reg = reg;
  request.desired = _configuration.verifiedProfile;  // Actual proof, never staged desired settings.
  uint8_t actual = 0;
  uint8_t requiredRaw = 0;
  RegisterDecode decoded{};
  const Status decode = decodeRegister(variant, reg, value, decoded);
  if (!decode.ok()) return decode;
  switch (reg) {
    case 0x08:
      requiredRaw = CONFIG_08_VALID;
      actual = _configuration.rawConfig08;
      request.fields = field(ProfileField::SOURCE_CURRENT);
      request.desired.sourceCurrent = decoded.profileCodes.sourceCurrent;
      break;
    case 0x09:
      requiredRaw = CONFIG_09_VALID;
      actual = _configuration.rawConfig09;
      request.fields = field(ProfileField::DRP_DUTY_CYCLE);
      request.desired.drpDutyCycle = decoded.profileCodes.drpDutyCycle;
      if (info->hasExtendedControls) {
        request.fields |= field(ProfileField::DISABLE_UFP_ACCESSORY);
        request.desired.disableUfpAccessory = decoded.profileCodes.disableUfpAccessory;
      }
      if ((value & 0x10U) != 0) {
        // IRQ W1C never combines an unapplied settings change with the command.
        const uint8_t mask = info->hasExtendedControls ? 0x07 : 0x06;
        if ((value & mask) != (actual & mask)) return error(Err::UNSUPPORTED_FEATURE);
        request.intent = detail::WriteIntent::ACK_IRQ;
        request.fields = 0;
      }
      break;
    case 0x0A:
      requiredRaw = CONFIG_0A_VALID;
      actual = _configuration.rawConfig0A;
      // Even equivalent322I DRP encodings cannot justify raw mode-bit recoding.
      if (((value ^ actual) & (info->hasExtendedControls ? 0x31U : 0x30U)) != 0)
        return error(Err::UNSUPPORTED_FEATURE);
      request.fields = field(ProfileField::CC_DEBOUNCE) | field(ProfileField::PORT_MODE);
      request.desired.ccDebounce = decoded.profileCodes.ccDebounce;
      request.desired.portMode = decoded.profileCodes.portMode;
      if (info->hasExtendedControls) {
        request.fields |= field(ProfileField::SOURCE_PREFERENCE);
        request.fields |= field(ProfileField::DISABLE_TERM);
        request.desired.sourcePreference = decoded.profileCodes.sourcePreference;
        request.desired.disableTerm = decoded.profileCodes.disableTerm;
      }
      break;
    case 0x45:
      requiredRaw = CONFIG_45_VALID;
      actual = _configuration.rawConfig45;
      if (!_rdRpWriteImageValid) return error(Err::CONFIGURATION_UNKNOWN);
      if (((value ^ actual) & 0x03U) != 0) return error(Err::INVALID_PARAM);
      if (((value ^ actual) & 0x04U) != 0) return error(Err::UNSUPPORTED_FEATURE);
      request.fields = field(ProfileField::DISABLE_RD_RP);
      request.desired.disableRdRp = decoded.profileCodes.disableRdRp;
      break;
    default: return error(Err::UNSUPPORTED_FEATURE);
  }
  if ((_configuration.rawConfigValidMask & requiredRaw) == 0)
    return error(Err::CONFIGURATION_UNKNOWN);
  detail::WriteEvidence evidence{};
  evidence.actual = actual;
  evidence.actualValid = true;
  evidence.effectsResolved = true;
  uint8_t composed = 0;
  const Status composition = detail::composeWrite(request, evidence, composed);
  if (!composition.ok()) return composition;
  if (composed != value) return error(Err::INVALID_PARAM);

  const bool ack = request.intent == detail::WriteIntent::ACK_IRQ;
  if (!ack) {
    invalidateConfiguration();
    _diagnostics.configurationWriteUnresolved = true;
  }
  _diagnostics.hardwareEffect = HardwareEffect::INDETERMINATE;
  const detail::ProtocolContext context{_config, _stats, _diagnostics};
  const auto transfer = detail::writeRegister(context, detail::TrafficSource::RAW, reg, composed,
                                               _config.i2cTimeoutMs);
  if (transfer.status.ok())
    _diagnostics.hardwareEffect = ack ? HardwareEffect::CONFIRMED : HardwareEffect::PARTIAL;
  return transfer.status;
}

}  // namespace TUSB32x
