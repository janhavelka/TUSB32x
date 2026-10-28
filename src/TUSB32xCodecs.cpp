/// @file TUSB32xCodecs.cpp
/// @brief Pure TI field/port interpretation and one actual-state-qualified write composer.
#include "TUSB32xCodecs.h"

#include "TUSB32xProtocol.h"

namespace TUSB32x {
namespace {

// TI SLLSEN9F Table 7-7 pp17-19; SLLSEP2D/SLLSEQ8D Tables 8-10 pp17-19;
// SLLSEO5D Tables 6-8..6-10 pp17-20. Persistent masks exclude live RO, reserved,
// IRQ W1C and reset-command bits. Readback is never an ordinary write image.
constexpr uint8_t ADVERTISEMENT = 0xC0;
constexpr uint8_t DETECTION = 0x30;
constexpr uint8_t ACCESSORY = 0x0E;
constexpr uint8_t ACTIVE_CABLE = 0x01;
constexpr uint8_t ATTACHMENT = 0xC0;
constexpr uint8_t DIRECTION = 0x20;
constexpr uint8_t IRQ_W1C = 0x10;
constexpr uint8_t DUTY = 0x06;
constexpr uint8_t DISABLE_ACCESSORY = 0x01;
constexpr uint8_t BASE_09_SETTINGS = 0x06;
constexpr uint8_t EXTENDED_09_SETTINGS = 0x07;
constexpr uint8_t DEBOUNCE = 0xC0;
constexpr uint8_t MODE = 0x30;
constexpr uint8_t RESET_COMMAND = 0x08;
constexpr uint8_t PREFERENCE = 0x06;
constexpr uint8_t DISABLE_TERM = 0x01;
constexpr uint8_t BASE_0A_SETTINGS = 0xF0;
constexpr uint8_t EXTENDED_0A_SETTINGS = 0xF7;
// SLLSEN9F Table 7-7 p19; SLLSEP2D/SLLSEQ8D Table 11 p20. Bits 1:0 are
// TI-internal, not user settings; preserve them, and qualify zero before writing.
constexpr uint8_t DISABLE_RD_RP = 0x04;
constexpr uint8_t INTERNAL_45 = 0x03;

constexpr uint16_t field(ProfileField value) { return static_cast<uint16_t>(value); }
constexpr uint16_t field(PortField value) { return static_cast<uint16_t>(value); }

constexpr DrpDutyCycle DUTIES[] = {DrpDutyCycle::PERCENT_30, DrpDutyCycle::PERCENT_40,
                                  DrpDutyCycle::PERCENT_50, DrpDutyCycle::PERCENT_60};
constexpr CcDebounce BASE_DEBOUNCES[] = {CcDebounce::MS_133, CcDebounce::MS_116,
                                       CcDebounce::MS_151, CcDebounce::MS_168};
constexpr CcDebounce EXTENDED_DEBOUNCES[] = {CcDebounce::MS_168, CcDebounce::MS_118,
                                           CcDebounce::MS_134, CcDebounce::MS_152};
constexpr DetectedCurrent CURRENTS[] = {DetectedCurrent::DEFAULT, DetectedCurrent::MA_1500,
    DetectedCurrent::CHARGE_THROUGH_500_MA, DetectedCurrent::MA_3000};
constexpr Attachment ATTACHMENTS[] = {Attachment::DETACHED, Attachment::LOCAL_SOURCE,
                                     Attachment::LOCAL_SINK, Attachment::ACCESSORY};
constexpr PortMode MODES[] = {PortMode::FOLLOW_PORT_PIN, PortMode::UFP, PortMode::DFP,
                             PortMode::DRP};

// Select whole semantic fields; includeAbsent detects unsupported requests
// without presenting absent controls as actual observations.
uint16_t registerFields(const detail::VariantInfo& info, uint8_t reg, bool includeAbsent = false) {
  const bool extended = info.hasExtendedControls || includeAbsent;
  switch (reg) {
    case 0x08: return field(ProfileField::SOURCE_CURRENT);
    case 0x09:
      return static_cast<uint16_t>(field(ProfileField::DRP_DUTY_CYCLE) |
          (extended ? field(ProfileField::DISABLE_UFP_ACCESSORY) : 0U));
    case 0x0A:
      return static_cast<uint16_t>(field(ProfileField::PORT_MODE) |
          field(ProfileField::CC_DEBOUNCE) |
          (extended
              ? field(ProfileField::SOURCE_PREFERENCE) | field(ProfileField::DISABLE_TERM) : 0U));
    case 0x45: return info.hasRdRp ? field(ProfileField::DISABLE_RD_RP) : 0;
    default: return 0;
  }
}

// Settings images exclude commands and live status, even a reset read as one.
uint8_t persistentMask(const detail::VariantInfo& info, uint8_t reg) {
  switch (reg) {
    case 0x08: return ADVERTISEMENT;
    case 0x09: return info.hasExtendedControls ? EXTENDED_09_SETTINGS : BASE_09_SETTINGS;
    case 0x0A: return info.hasExtendedControls ? EXTENDED_0A_SETTINGS : BASE_0A_SETTINGS;
    case 0x45: return DISABLE_RD_RP;
    default: return 0;
  }
}

// Distinguish absent controls from bytes with no persistent user settings.
Status settingRegister(const detail::VariantInfo& info, uint8_t reg) {
  if (reg == 0x45 && !info.hasRdRp) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "Register is absent on the selected variant");
  }
  if (registerFields(info, reg) == 0) {
    return Status::Error(Err::INVALID_PARAM, "Not a persistent settings register");
  }
  return Status::Ok();
}

// Select only complete semantic fields from an already register-checked request.
// This is deliberately not an arbitrary caller-provided byte RMW mask.
uint8_t requestedMask(uint16_t fields) {
  uint8_t mask = 0;
  if ((fields & field(ProfileField::SOURCE_CURRENT)) != 0) mask |= ADVERTISEMENT;
  if ((fields & field(ProfileField::DRP_DUTY_CYCLE)) != 0) mask |= DUTY;
  if ((fields & field(ProfileField::DISABLE_UFP_ACCESSORY)) != 0) mask |= DISABLE_ACCESSORY;
  if ((fields & field(ProfileField::PORT_MODE)) != 0) mask |= MODE;
  if ((fields & field(ProfileField::CC_DEBOUNCE)) != 0) mask |= DEBOUNCE;
  if ((fields & field(ProfileField::SOURCE_PREFERENCE)) != 0) mask |= PREFERENCE;
  if ((fields & field(ProfileField::DISABLE_TERM)) != 0) mask |= DISABLE_TERM;
  if ((fields & field(ProfileField::DISABLE_RD_RP)) != 0) mask |= DISABLE_RD_RP;
  return mask;
}

Status unknownEvidence() {
  return Status::Error(Err::CONFIGURATION_UNKNOWN,
                       "Required actual control or sequence proof is absent");
}

// Reserved actual fields cannot prove a safe image for preserving controls.
Status actualFields(const detail::VariantInfo& info, uint8_t reg, uint8_t value,
                    RegisterDecode& decoded) {
  const Status status = decodeRegister(info.variant, reg, value, decoded);
  if (!status.ok()) return status;
  const uint16_t required = registerFields(info, reg);
  if ((decoded.profileValidFields & required) != required) return unknownEvidence();
  return Status::Ok();
}

}  // namespace

Status decodeRegister(Variant variant, uint8_t reg, uint8_t value, RegisterDecode& out) {
  const detail::VariantInfo* info = detail::findVariant(variant);
  if (info == nullptr) {
    return Status::Error(Err::INVALID_PARAM, "Select an explicit supported variant");
  }
  if (reg > 0x0A && reg != 0x45 && reg != 0xA0) {
    return Status::Error(Err::INVALID_PARAM, "Unmapped register");
  }
  if ((reg == 0x45 && !info->hasRdRp) || (reg == 0xA0 && !info->hasRevision)) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "Register is absent on the selected variant");
  }
  RegisterDecode decoded{};
  decoded.variant = variant;
  decoded.reg = reg;
  decoded.value = value;
  if (reg == 0x08) {
    switch (value & ADVERTISEMENT) {
      case 0x00: decoded.profileCodes.sourceCurrent = SourceCurrent::DEFAULT; break;
      case 0x40: decoded.profileCodes.sourceCurrent = SourceCurrent::MA_1500; break;
      case 0x80: decoded.profileCodes.sourceCurrent = SourceCurrent::MA_3000; break;
      default: break;  // Reserved 11 retains raw evidence without a valid placeholder.
    }
    if ((value & ADVERTISEMENT) != ADVERTISEMENT) {
      decoded.profileValidFields |= field(ProfileField::SOURCE_CURRENT);
    }
    decoded.detectedCurrent = CURRENTS[(value & DETECTION) >> 4];
    decoded.statusValidFields = field(PortField::DETECTED_CURRENT) | field(PortField::ACTIVE_CABLE);
    decoded.activeCable = (value & ACTIVE_CABLE) != 0;
    switch (value & ACCESSORY) {
      case 0x00: decoded.accessory = Accessory::NONE; break;
      case 0x08: decoded.accessory = Accessory::AUDIO; break;
      case 0x0A: decoded.accessory = Accessory::CHARGE_THROUGH; break;
      case 0x0C:
        decoded.accessory = info->hasExtendedControls
            ? Accessory::DEBUG_LOCAL_DFP : Accessory::DEBUG;
        break;
      case 0x0E:
        if (info->hasExtendedControls) decoded.accessory = Accessory::DEBUG_LOCAL_UFP;
        break;
      default: break;
    }
    if (decoded.accessory != Accessory::UNKNOWN) {
      decoded.statusValidFields |= field(PortField::ACCESSORY);
    }
  } else if (reg == 0x09) {
    decoded.attachment = ATTACHMENTS[(value & ATTACHMENT) >> 6];
    // Raw field meaning only, including detached CC2; applicability needs paired port context.
    decoded.orientation = (value & DIRECTION) != 0 ? Orientation::CC2 : Orientation::CC1;
    decoded.interruptPending = (value & IRQ_W1C) != 0;
    decoded.statusValidFields = field(PortField::ATTACHMENT) | field(PortField::ORIENTATION) |
                               field(PortField::INTERRUPT_PENDING);
    decoded.profileCodes.drpDutyCycle = DUTIES[(value & DUTY) >> 1];
    decoded.profileValidFields = field(ProfileField::DRP_DUTY_CYCLE);
    if (info->hasExtendedControls) {
      decoded.profileCodes.disableUfpAccessory = (value & DISABLE_ACCESSORY) != 0;
      decoded.profileValidFields |= field(ProfileField::DISABLE_UFP_ACCESSORY);
    }
    // TUSB322I 09 bit 3 (mask 0x08) is independent raw VCONN fault evidence.
    // SLLSEO5D Table 6-9 p19 labels it RCU without specifying a fault-clear
    // transaction: no typed fault clear or implied ACK.
  } else if (reg == 0x0A) {
    decoded.profileCodes.ccDebounce = info->hasExtendedControls
        ? EXTENDED_DEBOUNCES[(value & DEBOUNCE) >> 6] : BASE_DEBOUNCES[(value & DEBOUNCE) >> 6];
    decoded.profileCodes.portMode = MODES[(value & MODE) >> 4];
    // SLLSEO5D Table 6-10 p20: both 00 and 11 request DRP; raw evidence is retained.
    if (variant == Variant::TUSB322I && (value & MODE) == 0) {
      decoded.profileCodes.portMode = PortMode::DRP;
    }
    decoded.profileValidFields = field(ProfileField::CC_DEBOUNCE) | field(ProfileField::PORT_MODE);
    if (info->hasExtendedControls) {
      switch (value & PREFERENCE) {
        case 0x00: decoded.profileCodes.sourcePreference = SourcePreference::NORMAL; break;
        case 0x02: decoded.profileCodes.sourcePreference = SourcePreference::TRY_SNK; break;
        case 0x06: decoded.profileCodes.sourcePreference = SourcePreference::TRY_SRC; break;
        default: break;  // 04 is reserved, not Try.SRC.
      }
      if ((value & PREFERENCE) != 0x04) {
        decoded.profileValidFields |= field(ProfileField::SOURCE_PREFERENCE);
      }
      decoded.profileCodes.disableTerm = (value & DISABLE_TERM) != 0;
      decoded.profileValidFields |= field(ProfileField::DISABLE_TERM);
    }
    decoded.resetValid = true;
    decoded.resetRequested = (value & RESET_COMMAND) != 0;
  } else if (reg == 0x45) {
    decoded.profileCodes.disableRdRp = (value & DISABLE_RD_RP) != 0;
    decoded.profileValidFields = field(ProfileField::DISABLE_RD_RP);
  }
  out = decoded;
  return Status::Ok();
}

namespace detail {

Status decodePort(Variant variant, uint8_t first09, uint8_t status08, uint8_t last09,
                  PortSnapshot& out) {
  RegisterDecode status{}, attachment{};
  Status result = decodeRegister(variant, 0x08, status08, status);
  if (!result.ok()) return result;
  result = decodeRegister(variant, 0x09, last09, attachment);
  if (!result.ok()) return result;
  PortSnapshot decoded{};
  decoded.rawStatus08 = status08;
  decoded.rawStatus09 = last09;
  decoded.rawValidMask = STATUS_08_VALID | STATUS_09_VALID;
  decoded.attachment = attachment.attachment;
  decoded.interruptPending = attachment.interruptPending;
  decoded.activeCable = status.activeCable; // Raw flag retained even without usable context.
  decoded.validFields = field(PortField::ATTACHMENT) | field(PortField::INTERRUPT_PENDING) |
      field(PortField::LOCAL_ROLE) | field(PortField::PEER_ROLE);
  decoded.localRole = PortRole::NOT_APPLICABLE;
  decoded.peerRole = PortRole::NOT_APPLICABLE;
  if ((status.profileValidFields & field(ProfileField::SOURCE_CURRENT)) != 0) {
    decoded.advertisedCurrent = status.profileCodes.sourceCurrent;
    decoded.validFields |= field(PortField::ADVERTISED_CURRENT);
  }
  // TI SLLSEN9F Table 7-7 pp17-18; SLLSEP2D/SLLSEQ8D Tables 8-9 pp17-18;
  // SLLSEO5D Tables 6-8/6-9 pp17-19.
  // Applicability is conservative library policy: contradictory/reserved accessory
  // combinations remain unknown. No normal power roles are inferred for accessories.
  const bool noAccessory = status.accessory == Accessory::NONE;
  switch (decoded.attachment) {
    case Attachment::DETACHED:
      decoded.orientation = Orientation::NOT_APPLICABLE;
      decoded.detectedCurrent = DetectedCurrent::NOT_APPLICABLE;
      decoded.validFields |= field(PortField::ORIENTATION);
      if (noAccessory) decoded.accessory = Accessory::NOT_APPLICABLE;
      break;
    case Attachment::LOCAL_SOURCE:
    case Attachment::LOCAL_SINK: {
      const bool source = decoded.attachment == Attachment::LOCAL_SOURCE;
      decoded.localRole = source ? PortRole::SOURCE : PortRole::SINK;
      decoded.peerRole = source ? PortRole::SINK : PortRole::SOURCE;
      decoded.orientation = attachment.orientation;
      decoded.validFields |= field(PortField::ORIENTATION);
      if (noAccessory) decoded.accessory = Accessory::NONE;
      if (source) {
        decoded.detectedCurrent = DetectedCurrent::NOT_APPLICABLE;
        if (noAccessory) decoded.validFields |= field(PortField::ACTIVE_CABLE);
      } else if (noAccessory && status.detectedCurrent != DetectedCurrent::CHARGE_THROUGH_500_MA) {
        decoded.detectedCurrent = status.detectedCurrent;
      }
      break;
    }
    case Attachment::ACCESSORY:
      if (!noAccessory) decoded.accessory = status.accessory;
      switch (decoded.accessory) {
        case Accessory::AUDIO:
        case Accessory::DEBUG:
        case Accessory::DEBUG_LOCAL_DFP:
          decoded.detectedCurrent = DetectedCurrent::NOT_APPLICABLE;
          break;
        case Accessory::CHARGE_THROUGH:
          if (status.detectedCurrent == DetectedCurrent::CHARGE_THROUGH_500_MA)
            decoded.detectedCurrent = status.detectedCurrent;
          break;
        default: break;
      }
      // Both-CC accessory detection does not prove usable direction. UFP debug
      // current applicability is withheld pending stronger evidence; preserve raw codes.
      break;
    default: break; // Every two-bit attachment code is currently documented.
  }
  if (decoded.accessory != Accessory::UNKNOWN) decoded.validFields |= field(PortField::ACCESSORY);
  if (decoded.detectedCurrent != DetectedCurrent::UNKNOWN)
    decoded.validFields |= field(PortField::DETECTED_CURRENT);
  decoded.coherence = ((first09 ^ last09) & 0xE0U) == 0 ?
      SnapshotCoherence::BOOKENDS_MATCH : SnapshotCoherence::CHANGED_DURING_READ;
  if (decoded.coherence == SnapshotCoherence::CHANGED_DURING_READ) {
    const uint16_t dependent = field(PortField::DETECTED_CURRENT) | field(PortField::ACCESSORY) |
        field(PortField::ACTIVE_CABLE);
    decoded.validFields &= static_cast<uint16_t>(~dependent);
    decoded.detectedCurrent = DetectedCurrent::UNKNOWN;
    decoded.accessory = Accessory::UNKNOWN;
  }
  out = decoded;
  return Status::Ok();
}

Status encodeProfileRegister(Variant variant, uint8_t reg, const DeviceProfile& profile,
                             uint8_t& out) {
  Status status = validateProfile(variant, profile);
  if (!status.ok()) return status;
  const VariantInfo& info = *findVariant(variant);
  status = settingRegister(info, reg);
  if (!status.ok()) return status;
  uint8_t value = 0;
  switch (reg) {
    case 0x08:
      switch (profile.sourceCurrent) {
        case SourceCurrent::DEFAULT: value = 0x00; break;
        case SourceCurrent::MA_1500: value = 0x40; break;
        case SourceCurrent::MA_3000: value = 0x80; break;
      }
      break;
    case 0x09:
      switch (profile.drpDutyCycle) {
        case DrpDutyCycle::PERCENT_30: value = 0x00; break;
        case DrpDutyCycle::PERCENT_40: value = 0x02; break;
        case DrpDutyCycle::PERCENT_50: value = 0x04; break;
        case DrpDutyCycle::PERCENT_60: value = 0x06; break;
      }
      if (info.hasExtendedControls && profile.disableUfpAccessory) value |= DISABLE_ACCESSORY;
      break;
    case 0x0A: {
      const CcDebounce* debounces = info.hasExtendedControls ? EXTENDED_DEBOUNCES : BASE_DEBOUNCES;
      for (uint8_t code = 0; code < 4; ++code) {
        if (profile.ccDebounce == debounces[code]) value = static_cast<uint8_t>(code << 6);
      }
      switch (profile.portMode) {
        case PortMode::FOLLOW_PORT_PIN: break;
        case PortMode::UFP: value |= 0x10; break;
        case PortMode::DFP: value |= 0x20; break;
        case PortMode::DRP: value |= 0x30; break;
      }
      if (info.hasExtendedControls) {
        switch (profile.sourcePreference) {
          case SourcePreference::NORMAL: break;
          case SourcePreference::TRY_SNK: value |= 0x02; break;
          case SourcePreference::TRY_SRC: value |= 0x06; break;
        }
        if (profile.disableTerm) value |= DISABLE_TERM;
      }
      break;
    }
    case 0x45: if (profile.disableRdRp) value = DISABLE_RD_RP; break;
    default: break;  // settingRegister has already rejected every other offset.
  }
  out = value;
  return Status::Ok();
}

Status comparePersistentSettings(Variant variant, uint8_t reg, uint8_t left, uint8_t right,
                                 bool& equal) {
  const VariantInfo* info = findVariant(variant);
  if (info == nullptr) {
    return Status::Error(Err::INVALID_PARAM, "Select an explicit supported variant");
  }
  Status status = settingRegister(*info, reg);
  if (!status.ok()) return status;
  RegisterDecode leftFields{}, rightFields{};
  status = actualFields(*info, reg, left, leftFields);
  if (!status.ok()) return status;
  status = actualFields(*info, reg, right, rightFields);
  if (!status.ok()) return status;
  uint8_t mask = persistentMask(*info, reg);
  if (variant == Variant::TUSB322I && reg == 0x0A) {
    mask = static_cast<uint8_t>(mask & static_cast<uint8_t>(~MODE));
    equal = leftFields.profileCodes.portMode == rightFields.profileCodes.portMode &&
            (left & mask) == (right & mask);
  } else {
    equal = (left & mask) == (right & mask);
  }
  return Status::Ok();
}

Status composeWrite(const WriteRequest& request, const WriteEvidence& evidence, uint8_t& out) {
  const VariantInfo* info = findVariant(request.variant);
  if (info == nullptr) {
    return Status::Error(Err::INVALID_CONFIG, "Select an explicit supported variant");
  }
  if (request.intent > WriteIntent::RECOVERY_ENABLE) {
    return Status::Error(Err::INVALID_PARAM, "Invalid write intent");
  }
  Status status = settingRegister(*info, request.reg);
  if (!status.ok()) return status;
  // SLLSEO5D Figure 6-3 vs Table 6-8 and Table 6-9: unresolved advertisement
  // access / fault clear gate the entire byte, even an unchanged 00 or IRQ-only write.
  if (request.variant == Variant::TUSB322I && (request.reg == 0x08 || request.reg == 0x09)) {
    return Status::Error(Err::UNSUPPORTED_FEATURE,
                         "TUSB322I register write evidence is unresolved");
  }
  // Reject intrinsic intent/access errors without requiring a hardware observation first.
  switch (request.intent) {
    case WriteIntent::ACK_IRQ:
      if (request.reg != 0x09) return Status::Error(Err::INVALID_PARAM, "ACK requires register 09");
      break;
    case WriteIntent::RESET:
      if (request.reg != 0x0A) {
        return Status::Error(Err::INVALID_PARAM, "Reset requires register 0A");
      }
      break;
    case WriteIntent::BASE_MODE_CHANGE:
      if (info->hasExtendedControls) {
        return Status::Error(Err::UNSUPPORTED_FEATURE, "Base mode procedure is unavailable");
      }
      if (request.reg != 0x0A) {
        return Status::Error(Err::INVALID_PARAM, "Mode requires register 0A");
      }
      break;
    case WriteIntent::MODE_DISABLE:
    case WriteIntent::MODE_CHANGE:
    case WriteIntent::MODE_ENABLE:
      if (!info->hasExtendedControls) {
        return Status::Error(Err::UNSUPPORTED_FEATURE, "Extended mode procedure is unavailable");
      }
      if (request.reg != 0x0A) {
        return Status::Error(Err::INVALID_PARAM, "Mode requires register 0A");
      }
      break;
    case WriteIntent::RECOVERY_DISABLE:
    case WriteIntent::RECOVERY_ENABLE:
      if (info->hasExtendedControls) {
        return Status::Error(Err::UNSUPPORTED_FEATURE, "Original-device recovery is unavailable");
      }
      if (request.reg != 0x45) {
        return Status::Error(Err::INVALID_PARAM, "Recovery requires register 45");
      }
      break;
    case WriteIntent::SETTINGS: break;
  }
  const bool typed = request.intent == WriteIntent::SETTINGS ||
      request.intent == WriteIntent::BASE_MODE_CHANGE || request.intent == WriteIntent::MODE_CHANGE;
  uint8_t encoded = 0;
  if (typed) {
    status = encodeProfileRegister(request.variant, request.reg, request.desired, encoded);
    if (!status.ok()) return status;
    const uint16_t familyFields = registerFields(*info, request.reg, true);
    if (request.fields == 0 || (request.fields & static_cast<uint16_t>(~familyFields)) != 0) {
      return Status::Error(Err::INVALID_PARAM, "Select complete fields of this register");
    }
    if ((request.fields & static_cast<uint16_t>(~registerFields(*info, request.reg))) != 0) {
      return Status::Error(Err::UNSUPPORTED_FEATURE, "Requested field is absent on this variant");
    }
  } else if (request.fields != 0) {
    return Status::Error(Err::INVALID_PARAM, "Fixed-purpose writes accept no desired fields");
  }
  if (!evidence.actualValid || (!evidence.effectsResolved &&
      request.intent != WriteIntent::ACK_IRQ && request.intent != WriteIntent::RESET)) {
    return unknownEvidence();
  }
  RegisterDecode actual{};
  status = actualFields(*info, request.reg, evidence.actual, actual);
  if (!status.ok()) return status;
  // Internal-zero qualification is the L/A documented default and conservative B policy.
  // Even perfect preservation arithmetic cannot qualify an abnormal internal image.
  if ((request.reg == 0x0A && actual.resetRequested) ||
      (request.reg == 0x45 && (evidence.actual & INTERNAL_45) != 0)) return unknownEvidence();

  const bool modeChanges = request.reg == 0x0A &&
      (request.fields & field(ProfileField::PORT_MODE)) != 0 &&
      actual.profileCodes.portMode != request.desired.portMode;
  if (modeChanges && request.variant == Variant::TUSB322I &&
      request.desired.portMode == PortMode::DRP) {
    return Status::Error(Err::UNSUPPORTED_FEATURE, "TUSB322I non-reset return to DRP is gated");
  }
  // Preserve the observed 322I DRP 00 representation on unrelated/equivalent updates.
  uint8_t update = requestedMask(request.fields);
  if (request.reg == 0x0A && !modeChanges) update = static_cast<uint8_t>(update & ~MODE);
  const uint8_t retained =
      static_cast<uint8_t>(evidence.actual & persistentMask(*info, request.reg));
  uint8_t value =
      static_cast<uint8_t>((retained & static_cast<uint8_t>(~update)) | (encoded & update));
  switch (request.intent) {
    case WriteIntent::SETTINGS:
      if (modeChanges) return unknownEvidence();
      if (request.reg == 0x45) value |= static_cast<uint8_t>(evidence.actual & INTERNAL_45);
      break;
    case WriteIntent::ACK_IRQ:
      value = static_cast<uint8_t>(retained | IRQ_W1C);
      break;
    case WriteIntent::RESET:
      value = static_cast<uint8_t>(retained | RESET_COMMAND);
      break;
    case WriteIntent::BASE_MODE_CHANGE:
      if (!modeChanges || !evidence.detached) return unknownEvidence();
      break;
    case WriteIntent::MODE_DISABLE:
    case WriteIntent::MODE_CHANGE:
    case WriteIntent::MODE_ENABLE:
      if (request.intent == WriteIntent::MODE_DISABLE) {
        value = static_cast<uint8_t>(retained | DISABLE_TERM);
      } else {
        if (!evidence.disableWritten || !actual.profileCodes.disableTerm) return unknownEvidence();
        if (request.intent == WriteIntent::MODE_CHANGE) {
          if (!modeChanges) return unknownEvidence();
          value |= DISABLE_TERM;
        } else {
          if (!evidence.modeWritten ||
              (request.variant != Variant::TUSB322I && !evidence.minimumWaitElapsed)) {
            return unknownEvidence();
          }
          value = static_cast<uint8_t>(retained & static_cast<uint8_t>(~DISABLE_TERM));
        }
      }
      break;
    case WriteIntent::RECOVERY_DISABLE:
    case WriteIntent::RECOVERY_ENABLE:
      // SLLSEN9F section 7.4.3 p14: explicit original-part 04, >=30ms, 00.
      // The owner supplies qualification and wait proof; this helper never performs the job.
      if (!evidence.recoveryQualified) return unknownEvidence();
      if (request.intent == WriteIntent::RECOVERY_ENABLE &&
          (!evidence.disableWritten || !evidence.minimumWaitElapsed ||
           !actual.profileCodes.disableRdRp)) return unknownEvidence();
      value = request.intent == WriteIntent::RECOVERY_DISABLE ? DISABLE_RD_RP : 0x00;
      break;
  }
  out = value;
  return Status::Ok();
}

}  // namespace detail
}  // namespace TUSB32x
