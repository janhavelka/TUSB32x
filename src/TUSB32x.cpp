/// @file TUSB32x.cpp
/// @brief Pure binding/profile validation; no transport access or retained state.
#include "TUSB32x/Config.h"

namespace TUSB32x {
namespace {

bool isBase(Variant variant) {
  return variant == Variant::TUSB320 || variant == Variant::TUSB320I;
}

// Reject unknown enum casts before selecting an address or capability rule.
bool isValidVariant(Variant variant) {
  switch (variant) {
    case Variant::TUSB320:
    case Variant::TUSB320I:
    case Variant::TUSB320LI:
    case Variant::TUSB320HI:
    case Variant::TUSB320LAI:
    case Variant::TUSB320HAI:
    case Variant::TUSB322I: return true;
    default: return false;
  }
}

}  // namespace

Status validateDriverConfig(const DriverConfig& config) {
  if (!isValidVariant(config.variant)) {
    return Status::Error(Err::INVALID_CONFIG, "Select an explicit supported variant");
  }
  if (config.i2cWrite == nullptr || config.i2cWriteRead == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "Both terminal I2C callbacks are required");
  }
  if (config.i2cTimeoutMs == 0) {
    return Status::Error(Err::INVALID_CONFIG, "I2C timeout must be positive milliseconds");
  }
  // TI SLLSEN9F section 7.5 p15; SLLSEP2D/SLLSEQ8D section 7.4 p15;
  // SLLSEO5D section 6.5 p15. Addresses are seven-bit strap choices, not registers.
  const bool addressValid = isBase(config.variant)
      ? config.address == DeviceAddress::ADDR_LOW_0X60 ||
        config.address == DeviceAddress::ADDR_HIGH_0X61
      : config.address == DeviceAddress::ADDR_LOW_0X47 ||
        config.address == DeviceAddress::ADDR_HIGH_0X67;
  if (!addressValid) {
    return Status::Error(Err::INVALID_CONFIG, "Address does not match the selected variant");
  }
  return Status::Ok();
}

Status validateProfile(Variant variant, const DeviceProfile& profile) {
  if (!isValidVariant(variant)) {
    return Status::Error(Err::INVALID_CONFIG, "Select an explicit supported variant");
  }
  // These uint8_t semantic enums are contiguous from zero, not register codes.
  // Check every enum before capability tests so malformed profiles stay invalid.
  if (profile.portMode > PortMode::DRP || profile.sourceCurrent > SourceCurrent::MA_3000 ||
      profile.sourcePreference > SourcePreference::TRY_SRC ||
      profile.drpDutyCycle > DrpDutyCycle::PERCENT_60 || profile.ccDebounce > CcDebounce::MS_168) {
    return Status::Error(Err::INVALID_CONFIG, "Profile contains an invalid enum value");
  }

  // TI SLLSEN9F Table 7-7 pp18-19 has no preference/accessory/0A-disable field.
  // Neutral placeholders describe absent fields without authorizing their access.
  if (isBase(variant) && (profile.sourcePreference != SourcePreference::NORMAL ||
                          profile.disableUfpAccessory || profile.disableTerm)) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "Profile requests an absent base control");
  }
  // Debounce: SLLSEN9F Table 7-7 p19 versus SLLSEP2D/SLLSEQ8D Table 10 p19
  // and SLLSEO5D Table 6-10 pp19-20. Nominal times differ between these maps.
  const bool debounceSupported = isBase(variant)
      ? profile.ccDebounce == CcDebounce::MS_133 || profile.ccDebounce == CcDebounce::MS_116 ||
        profile.ccDebounce == CcDebounce::MS_151 || profile.ccDebounce == CcDebounce::MS_168
      : profile.ccDebounce == CcDebounce::MS_168 || profile.ccDebounce == CcDebounce::MS_118 ||
        profile.ccDebounce == CcDebounce::MS_134 || profile.ccDebounce == CcDebounce::MS_152;
  if (!debounceSupported) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "Debounce is unavailable on this variant");
  }
  // SLLSEO5D Table 6-6 p17 and Table 6-10 p20: no 45 register or PORT pin.
  if (variant == Variant::TUSB322I &&
      (profile.portMode == PortMode::FOLLOW_PORT_PIN || profile.disableRdRp)) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "Profile requests an absent TUSB322I control");
  }
  // TUSB322I readable advertisement/duty/accessory values remain valid settings.
  // Apply must separately compare proved actual fields and gate changed writes;
  // see docs/register-map.md and V_ADVERTISEMENT / V_FAULT_CLEAR / V_RETURN_DRP.
  return Status::Ok();
}

}  // namespace TUSB32x
