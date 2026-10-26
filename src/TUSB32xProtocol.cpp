/// @file TUSB32xProtocol.cpp
/// @brief Bounded private register callbacks, transport accounting and exact identity comparisons.
#include "TUSB32xProtocol.h"

#include <cstring>
#include <limits>

namespace TUSB32x {
namespace detail {
namespace {

// TI SLLSEN9F Table 7-7 pp16-19; SLLSEP2D/SLLSEQ8D Tables 7-12 pp16-20;
// SLLSEO5D Tables 6-6..6-11 pp17-20. A shared ID never identifies enable polarity.
constexpr VariantInfo VARIANTS[] = {
    {Variant::TUSB320, false, true, false, 0},
    {Variant::TUSB320I, false, true, false, 0},
    {Variant::TUSB320LI, true, true, true, 0x01},
    {Variant::TUSB320HI, true, true, true, 0x01},
    {Variant::TUSB320LAI, true, true, true, 0x02},
    {Variant::TUSB320HAI, true, true, true, 0x02},
    {Variant::TUSB322I, true, false, true, 0x02},
};
constexpr uint8_t CONTROL_LAST = 0x0A;
constexpr uint8_t RD_RP_REGISTER = 0x45;
constexpr uint8_t REVISION_REGISTER = 0xA0;
constexpr size_t MAX_READ_BYTES = 11;
constexpr uint8_t IDENTITY_320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
constexpr uint8_t IDENTITY_322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};

void increment(uint32_t& value) {
  if (value != std::numeric_limits<uint32_t>::max()) ++value;
}

Status normalize(Status status) {
  switch (status.code) {
    case Err::OK:
    case Err::I2C_ERROR:
    case Err::I2C_NACK_ADDR:
    case Err::I2C_NACK_DATA:
    case Err::I2C_TIMEOUT:
    case Err::I2C_BUS:
    case Err::I2C_BUSY: return status;
    default:
      return Status::Error(Err::I2C_ERROR, "Callback returned a non-transport status",
                           status.detail);
  }
}

Status validateRequest(const ProtocolContext& context, TrafficSource source,
                       uint64_t remainingMs) {
  const Status configStatus = validateDriverConfig(context.config);
  if (!configStatus.ok()) return configStatus;
  if (source != TrafficSource::MANAGED && source != TrafficSource::RAW) {
    return Status::Error(Err::INVALID_PARAM, "Invalid transport accounting source");
  }
  if (remainingMs == 0) {
    return Status::Error(Err::INVALID_PARAM, "Transport budget must be positive milliseconds");
  }
  return Status::Ok();
}

Status validateRange(const VariantInfo& info, uint8_t reg, size_t length) {
  if (length == 0 || length > MAX_READ_BYTES) {
    return Status::Error(Err::INVALID_PARAM, "Read length must be 1..11 bytes");
  }
  // TI maps permit sequential 00..0A and isolated 45/A0, never holes or wrapping.
  // Compare length to remaining range size before addition, including SIZE_MAX input.
  if (reg <= CONTROL_LAST && length <= static_cast<size_t>(CONTROL_LAST - reg) + 1U) {
    return Status::Ok();
  }
  if (reg == RD_RP_REGISTER || reg == REVISION_REGISTER) {
    const bool present = reg == RD_RP_REGISTER ? info.hasRdRp : info.hasRevision;
    if (!present) {
      return Status::Error(Err::UNSUPPORTED_FEATURE, "Register is absent on the selected variant");
    }
    if (length == 1) return Status::Ok();
  }
  return Status::Error(Err::INVALID_PARAM, "Read crosses or selects an unmapped register");
}

Status validateWrite(const VariantInfo& info, uint8_t reg) {
  if ((reg == RD_RP_REGISTER && !info.hasRdRp) ||
      (reg == REVISION_REGISTER && !info.hasRevision)) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "Register is absent on the selected variant");
  }
  // SLLSEO5D Figure 6-3/Table 6-8 disagree on 08 advertisement access; Table 6-9
  // does not define the 09 fault-clear transaction. Gate entire writes, even zero bytes.
  if (info.variant == Variant::TUSB322I && (reg == 0x08 || reg == 0x09)) {
    return Status::Error(Err::UNSUPPORTED_FEATURE,
                         "TUSB322I register write evidence is unresolved");
  }
  if (reg == 0x08 || reg == 0x09 || reg == CONTROL_LAST || reg == RD_RP_REGISTER) {
    return Status::Ok();
  }
  return Status::Error(Err::INVALID_PARAM, "Register is read-only or unmapped");
}

// Sole callback invocation/accounting boundary. Callers have already validated arguments/access.
TransferResult invoke(const ProtocolContext& context, TrafficSource source, const uint8_t* tx,
                      size_t txLength, uint8_t* rx, size_t rxLength, uint64_t remainingMs) {
  const auto& config = context.config;
  const uint32_t timeoutMs = remainingMs < config.i2cTimeoutMs
      ? static_cast<uint32_t>(remainingMs) : config.i2cTimeoutMs;
  increment(context.stats.totalAttempts);
  const Status status = normalize(rxLength != 0
      ? config.i2cWriteRead(static_cast<uint8_t>(config.address), tx, txLength, rx, rxLength,
                            timeoutMs, config.i2cUser)
      : config.i2cWrite(static_cast<uint8_t>(config.address), tx, txLength, timeoutMs,
                        config.i2cUser));
  context.stats.lastStatus = status;
  if (status.ok()) {
    increment(context.stats.totalSuccess);
  } else {
    increment(context.stats.totalFailures);
    context.diagnostics.lastFailedTransportStatus = status;
    context.diagnostics.lastFailedTransportTimeValid = false;
  }
  if (source == TrafficSource::MANAGED) {
    auto& managed = context.diagnostics.managedTransport;
    managed.lastStatus = status;
    if (status.ok()) {
      increment(managed.totalSuccess);
      managed.consecutiveFailures = 0;
      managed.lastSuccessTimeValid = false;
    } else {
      increment(managed.totalFailures);
      increment(managed.consecutiveFailures);
      managed.lastFailureTimeValid = false;
    }
  }
  // A Status-only result supplies neither a completion timestamp nor write phase evidence.
  // Leave numeric timestamp storage untouched as unusable history. Never promote trust/identity.
  const WriteAcceptance acceptance = rxLength != 0 ? WriteAcceptance::NOT_ATTEMPTED
      : status.ok() ? WriteAcceptance::CALLBACK_SUCCEEDED : WriteAcceptance::POSSIBLE;
  return {status, true, acceptance};
}

}  // namespace

const VariantInfo* findVariant(Variant variant) {
  for (const auto& info : VARIANTS) {
    if (info.variant == variant) return &info;
  }
  return nullptr;
}

Status validateRegisterAccess(Variant variant, uint8_t reg, bool write) {
  if (reg > CONTROL_LAST && reg != RD_RP_REGISTER && reg != REVISION_REGISTER) {
    return Status::Error(Err::INVALID_PARAM, "Unmapped scalar register");
  }
  const VariantInfo* info = findVariant(variant);
  if (variant != Variant::UNSPECIFIED) {
    if (info == nullptr) return Status::Error(Err::INVALID_CONFIG, "Invalid bound variant");
    const Status status = validateRange(*info, reg, 1);
    if (!status.ok()) return status;
  }
  if (!write) return Status::Ok();
  if (reg < 0x08 || reg == REVISION_REGISTER) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "Raw target is read-only");
  }
  return info == nullptr ? Status::Ok() : validateWrite(*info, reg);
}

TransferResult readRegisters(const ProtocolContext& context, TrafficSource source, uint8_t reg,
                             uint8_t* out, size_t length, uint64_t remainingMs) {
  Status status = validateRequest(context, source, remainingMs);
  if (!status.ok()) return {status, false, WriteAcceptance::NOT_ATTEMPTED};
  if (out == nullptr) {
    return {Status::Error(Err::INVALID_PARAM, "Read output must not be null"), false,
            WriteAcceptance::NOT_ATTEMPTED};
  }
  // Successful binding validation guarantees a table entry; no guessed variant fallback.
  status = validateRange(*findVariant(context.config.variant), reg, length);
  if (!status.ok()) return {status, false, WriteAcceptance::NOT_ATTEMPTED};
  uint8_t scratch[MAX_READ_BYTES]{};
  const TransferResult result = invoke(context, source, &reg, 1, scratch, length, remainingMs);
  if (result.status.ok()) std::memcpy(out, scratch, length);
  return result;
}

TransferResult writeRegister(const ProtocolContext& context, TrafficSource source, uint8_t reg,
                             uint8_t value, uint64_t remainingMs) {
  Status status = validateRequest(context, source, remainingMs);
  if (!status.ok()) return {status, false, WriteAcceptance::NOT_ATTEMPTED};
  status = validateWrite(*findVariant(context.config.variant), reg);
  if (!status.ok()) return {status, false, WriteAcceptance::NOT_ATTEMPTED};
  const uint8_t tx[] = {reg, value};
  return invoke(context, source, tx, sizeof(tx), nullptr, 0, remainingMs);
}

Status checkIdentity(Variant variant, const uint8_t* bytes, size_t length) {
  if (findVariant(variant) == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "Select an explicit supported variant");
  }
  if (bytes == nullptr || length != sizeof(IDENTITY_320)) {
    return Status::Error(Err::INVALID_PARAM, "Identity comparison requires eight complete bytes");
  }
  const uint8_t* expected = variant == Variant::TUSB322I ? IDENTITY_322 : IDENTITY_320;
  if (std::memcmp(bytes, expected, sizeof(IDENTITY_320)) != 0) {
    return Status::Error(Err::CHIP_ID_MISMATCH, "Identity signature differs from selected family");
  }
  return Status::Ok();
}

Status checkRevision(Variant variant, uint8_t revision) {
  const VariantInfo* info = findVariant(variant);
  if (info == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "Select an explicit supported variant");
  }
  if (!info->hasRevision) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "Selected variant has no revision register");
  }
  if (revision != info->revision) {
    return Status::Error(Err::CHIP_ID_MISMATCH, "Revision differs from selected part expectation");
  }
  return Status::Ok();
}

}  // namespace detail
}  // namespace TUSB32x
