/// @file test_codecs.cpp
/// @brief Independent TI field tables, exhaustive byte decoding and qualified write regressions.
#include "TUSB32xCodecs.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <initializer_list>

namespace {

namespace t = TUSB32x;
namespace p = TUSB32x::detail;

int failures = 0;
std::size_t checks = 0;
unsigned int contextVariant = 0;
unsigned int contextReg = 0;
unsigned int contextValue = 0;

void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    if (failures < 30) {
      std::printf("FAIL codec line %d variant=%u reg=%02X value=%02X: %s\n", line, contextVariant,
                  contextReg, contextValue, expression);
    }
    ++failures;
  }
}

// Runtime checks deliberately do not depend on assert or NDEBUG.
#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint16_t field(t::ProfileField value) { return static_cast<uint16_t>(value); }
constexpr uint16_t field(t::PortField value) { return static_cast<uint16_t>(value); }

struct VariantCase {
  t::Variant variant;
  bool base;
  bool is322;
};

constexpr VariantCase VARIANTS[] = {
    {t::Variant::TUSB320, true, false},
    {t::Variant::TUSB320I, true, false},
    {t::Variant::TUSB320LI, false, false},
    {t::Variant::TUSB320HI, false, false},
    {t::Variant::TUSB320LAI, false, false},
    {t::Variant::TUSB320HAI, false, false},
    {t::Variant::TUSB322I, false, true},
};

// Literal oracles from SLLSEN9F Table 7-7 pp17-19, SLLSEP2D/SLLSEQ8D Tables 8-11
// pp17-20 and SLLSEO5D Tables 6-8/6-9/6-10 pp17-20. No production table/helper is
// used to construct expected bytes or interpretations, including reserved/default storage.
constexpr t::SourceCurrent ADVERTISEMENT[] = {
    t::SourceCurrent::DEFAULT, t::SourceCurrent::MA_1500, t::SourceCurrent::MA_3000};
constexpr t::DetectedCurrent DETECTION[] = {
    t::DetectedCurrent::DEFAULT, t::DetectedCurrent::MA_1500,
    t::DetectedCurrent::CHARGE_THROUGH_500_MA, t::DetectedCurrent::MA_3000};
constexpr t::Attachment ATTACHMENT[] = {
    t::Attachment::DETACHED, t::Attachment::LOCAL_SOURCE,
    t::Attachment::LOCAL_SINK, t::Attachment::ACCESSORY};
constexpr t::DrpDutyCycle DUTY[] = {
    t::DrpDutyCycle::PERCENT_30, t::DrpDutyCycle::PERCENT_40,
    t::DrpDutyCycle::PERCENT_50, t::DrpDutyCycle::PERCENT_60};
constexpr t::CcDebounce BASE_DEBOUNCE[] = {
    t::CcDebounce::MS_133, t::CcDebounce::MS_116,
    t::CcDebounce::MS_151, t::CcDebounce::MS_168};
constexpr t::CcDebounce EXTENDED_DEBOUNCE[] = {
    t::CcDebounce::MS_168, t::CcDebounce::MS_118,
    t::CcDebounce::MS_134, t::CcDebounce::MS_152};
constexpr t::PortMode MODE[] = {
    t::PortMode::FOLLOW_PORT_PIN, t::PortMode::UFP, t::PortMode::DFP, t::PortMode::DRP};
constexpr t::SourcePreference PREFERENCE[] = {
    t::SourcePreference::NORMAL, t::SourcePreference::TRY_SNK,
    t::SourcePreference::NORMAL, t::SourcePreference::TRY_SRC};

void context(const VariantCase& variant, uint8_t reg, unsigned int value = 0) {
  contextVariant = static_cast<unsigned int>(variant.variant);
  contextReg = reg;
  contextValue = value;
}

bool sameProfile(const t::DeviceProfile& left, const t::DeviceProfile& right) {
  return left.portMode == right.portMode && left.sourceCurrent == right.sourceCurrent &&
         left.sourcePreference == right.sourcePreference &&
         left.drpDutyCycle == right.drpDutyCycle && left.ccDebounce == right.ccDebounce &&
         left.disableUfpAccessory == right.disableUfpAccessory &&
         left.disableTerm == right.disableTerm && left.disableRdRp == right.disableRdRp;
}

bool sameDecode(const t::RegisterDecode& left, const t::RegisterDecode& right) {
  return left.variant == right.variant && left.reg == right.reg && left.value == right.value &&
         sameProfile(left.profileCodes, right.profileCodes) &&
         left.profileValidFields == right.profileValidFields &&
         left.attachment == right.attachment && left.orientation == right.orientation &&
         left.detectedCurrent == right.detectedCurrent &&
         left.accessory == right.accessory && left.activeCable == right.activeCable &&
         left.interruptPending == right.interruptPending &&
         left.statusValidFields == right.statusValidFields &&
         left.resetRequested == right.resetRequested && left.resetValid == right.resetValid;
}

t::RegisterDecode sentinel() {
  t::RegisterDecode result{};
  result.variant = static_cast<t::Variant>(0xE1);
  result.reg = 0xD2;
  result.value = 0xC3;
  result.profileCodes.portMode = static_cast<t::PortMode>(0xB4);
  result.profileCodes.sourceCurrent = static_cast<t::SourceCurrent>(0xA5);
  result.profileCodes.sourcePreference = static_cast<t::SourcePreference>(0x96);
  result.profileCodes.drpDutyCycle = static_cast<t::DrpDutyCycle>(0x87);
  result.profileCodes.ccDebounce = static_cast<t::CcDebounce>(0x78);
  result.profileCodes.disableUfpAccessory = true;
  result.profileCodes.disableTerm = true;
  result.profileCodes.disableRdRp = true;
  result.profileValidFields = 0xC96A;
  result.attachment = static_cast<t::Attachment>(0x69);
  result.orientation = static_cast<t::Orientation>(0x5A);
  result.detectedCurrent = static_cast<t::DetectedCurrent>(0x4B);
  result.accessory = static_cast<t::Accessory>(0x3C);
  result.activeCable = true;
  result.interruptPending = true;
  result.statusValidFields = 0x963C;
  result.resetRequested = true;
  result.resetValid = true;
  return result;
}

t::RegisterDecode expectedDecode(const VariantCase& variant, uint8_t reg, uint8_t value) {
  t::RegisterDecode result{};
  result.variant = variant.variant;
  result.reg = reg;
  result.value = value;
  if (reg == 0x08) {
    const unsigned int advertisement = value >> 6;
    if (advertisement != 3) {
      result.profileCodes.sourceCurrent = ADVERTISEMENT[advertisement];
      result.profileValidFields = field(t::ProfileField::SOURCE_CURRENT);
    }
    result.detectedCurrent = DETECTION[(value >> 4) & 3U];
    result.activeCable = (value & 1U) != 0;
    result.statusValidFields = field(t::PortField::DETECTED_CURRENT) |
                              field(t::PortField::ACTIVE_CABLE);
    const unsigned int accessory = (value >> 1) & 7U;
    constexpr t::Accessory ACCESSORIES[] = {
        t::Accessory::NONE, t::Accessory::UNKNOWN, t::Accessory::UNKNOWN, t::Accessory::UNKNOWN,
        t::Accessory::AUDIO, t::Accessory::CHARGE_THROUGH,
        t::Accessory::DEBUG_LOCAL_DFP, t::Accessory::DEBUG_LOCAL_UFP};
    result.accessory = ACCESSORIES[accessory];
    if (variant.base && accessory == 6) result.accessory = t::Accessory::DEBUG;
    if (variant.base && accessory == 7) result.accessory = t::Accessory::UNKNOWN;
    if (result.accessory != t::Accessory::UNKNOWN) {
      result.statusValidFields |= field(t::PortField::ACCESSORY);
    }
  } else if (reg == 0x09) {
    result.profileCodes.drpDutyCycle = DUTY[(value >> 1) & 3U];
    result.profileValidFields = field(t::ProfileField::DRP_DUTY_CYCLE);
    if (!variant.base) {
      result.profileCodes.disableUfpAccessory = (value & 1U) != 0;
      result.profileValidFields |= field(t::ProfileField::DISABLE_UFP_ACCESSORY);
    }
    result.attachment = ATTACHMENT[value >> 6];
    result.orientation = (value & 0x20U) != 0 ? t::Orientation::CC2 : t::Orientation::CC1;
    result.interruptPending = (value & 0x10U) != 0;
    result.statusValidFields = field(t::PortField::ATTACHMENT) | field(t::PortField::ORIENTATION) |
                              field(t::PortField::INTERRUPT_PENDING);
  } else if (reg == 0x0A) {
    result.profileCodes.ccDebounce =
        variant.base ? BASE_DEBOUNCE[value >> 6] : EXTENDED_DEBOUNCE[value >> 6];
    result.profileCodes.portMode = MODE[(value >> 4) & 3U];
    if (variant.is322 && (value & 0x30U) == 0) result.profileCodes.portMode = t::PortMode::DRP;
    result.profileValidFields = field(t::ProfileField::CC_DEBOUNCE) |
                               field(t::ProfileField::PORT_MODE);
    if (!variant.base) {
      const unsigned int preference = (value >> 1) & 3U;
      if (preference != 2) {
        result.profileCodes.sourcePreference = PREFERENCE[preference];
        result.profileValidFields |= field(t::ProfileField::SOURCE_PREFERENCE);
      }
      result.profileCodes.disableTerm = (value & 1U) != 0;
      result.profileValidFields |= field(t::ProfileField::DISABLE_TERM);
    }
    result.resetRequested = (value & 8U) != 0;
    result.resetValid = true;
  } else if (reg == 0x45) {
    result.profileCodes.disableRdRp = (value & 4U) != 0;
    result.profileValidFields = field(t::ProfileField::DISABLE_RD_RP);
  }
  return result;
}

void checkStatus(t::Status status, t::Err expected) {
  if (status.code != expected && failures < 30) {
    std::printf("  status=%s expected=%s\n", t::errorName(status.code), t::errorName(expected));
  }
  CHECK(status.code == expected);
  CHECK(status.detail == 0);
  CHECK(status.msg != nullptr);
}

void decodeBytesAndErrors() {
  for (const auto& variant : VARIANTS) {
    for (unsigned int reg = 0; reg < 256; ++reg) {
      const bool mapped = reg <= 0x0A || reg == 0x45 || reg == 0xA0;
      const bool absent = (variant.base && reg == 0xA0) || (variant.is322 && reg == 0x45);
      // All 256 bytes on every mapped register, including identity/revision raw-only results.
      const unsigned int limit = mapped ? 256U : 2U;
      for (unsigned int value = 0; value < limit; ++value) {
        context(variant, static_cast<uint8_t>(reg), value);
        auto actual = sentinel();
        const auto before = actual;
        const auto status = t::decodeRegister(variant.variant, static_cast<uint8_t>(reg),
                                              static_cast<uint8_t>(value), actual);
        checkStatus(status, !mapped ? t::Err::INVALID_PARAM
                                   : absent ? t::Err::UNSUPPORTED_FEATURE : t::Err::OK);
        CHECK(sameDecode(actual, !mapped || absent
                                    ? before
                                    : expectedDecode(variant, static_cast<uint8_t>(reg),
                                                     static_cast<uint8_t>(value))));
      }
    }
  }
  for (unsigned int variant = 0; variant < 256; ++variant) {
    if (variant >= 1 && variant <= 7) continue;
    for (const uint8_t reg : {uint8_t{0x08}, uint8_t{0x45}, uint8_t{0xA0}, uint8_t{0xFF}}) {
      auto actual = sentinel();
      const auto before = actual;
      checkStatus(t::decodeRegister(static_cast<t::Variant>(variant), reg, 0xA5, actual),
                  t::Err::INVALID_PARAM);
      CHECK(sameDecode(actual, before));
    }
  }
}

// Independent validation and inverse tables, indexed by the public semantic enum domains.
t::Err profileStatus(const VariantCase& variant, const t::DeviceProfile& profile) {
  if (static_cast<unsigned int>(profile.portMode) > 3 ||
      static_cast<unsigned int>(profile.sourceCurrent) > 2 ||
      static_cast<unsigned int>(profile.sourcePreference) > 2 ||
      static_cast<unsigned int>(profile.drpDutyCycle) > 3 ||
      static_cast<unsigned int>(profile.ccDebounce) > 6) {
    return t::Err::INVALID_CONFIG;
  }
  const auto debounce = profile.ccDebounce;
  if (variant.base) {
    if (profile.sourcePreference != t::SourcePreference::NORMAL || profile.disableUfpAccessory ||
        profile.disableTerm || debounce == t::CcDebounce::MS_118 ||
        debounce == t::CcDebounce::MS_134 || debounce == t::CcDebounce::MS_152) {
      return t::Err::UNSUPPORTED_FEATURE;
    }
  } else if (debounce == t::CcDebounce::MS_116 || debounce == t::CcDebounce::MS_133 ||
             debounce == t::CcDebounce::MS_151) {
    return t::Err::UNSUPPORTED_FEATURE;
  }
  if (variant.is322 && (profile.portMode == t::PortMode::FOLLOW_PORT_PIN || profile.disableRdRp)) {
    return t::Err::UNSUPPORTED_FEATURE;
  }
  return t::Err::OK;
}

uint8_t expectedEncoding(const VariantCase& variant, uint8_t reg, const t::DeviceProfile& profile) {
  constexpr uint8_t SOURCE[] = {0x00, 0x40, 0x80};
  constexpr uint8_t DUTY_BITS[] = {0x00, 0x02, 0x04, 0x06};
  constexpr uint8_t MODE_BITS[] = {0x00, 0x10, 0x20, 0x30};
  constexpr uint8_t PREF_BITS[] = {0x00, 0x02, 0x06};
  // Public order: 116,118,133,134,151,152,168. Unsupported entries are never consumed.
  constexpr uint8_t BASE_BITS[] = {0x40, 0x00, 0x00, 0x00, 0x80, 0x00, 0xC0};
  constexpr uint8_t EXT_BITS[] = {0x00, 0x40, 0x00, 0x80, 0x00, 0xC0, 0x00};
  if (reg == 0x08) return SOURCE[static_cast<unsigned int>(profile.sourceCurrent)];
  if (reg == 0x09) {
    return static_cast<uint8_t>(DUTY_BITS[static_cast<unsigned int>(profile.drpDutyCycle)] |
                                (profile.disableUfpAccessory ? 1U : 0U));
  }
  if (reg == 0x45) return profile.disableRdRp ? 0x04 : 0x00;
  return static_cast<uint8_t>((variant.base ? BASE_BITS : EXT_BITS)
                                 [static_cast<unsigned int>(profile.ccDebounce)] |
                             MODE_BITS[static_cast<unsigned int>(profile.portMode)] |
                             PREF_BITS[static_cast<unsigned int>(profile.sourcePreference)] |
                             (profile.disableTerm ? 1U : 0U));
}

void checkEncoding(const VariantCase& variant, const t::DeviceProfile& profile) {
  const auto before = profile;
  for (const uint8_t reg : {uint8_t{0x08}, uint8_t{0x09}, uint8_t{0x0A}, uint8_t{0x45}}) {
    context(variant, reg);
    uint8_t output = 0xD3;
    auto expected = profileStatus(variant, profile);
    if (expected == t::Err::OK && variant.is322 && reg == 0x45) {
      expected = t::Err::UNSUPPORTED_FEATURE;
    }
    checkStatus(p::encodeProfileRegister(variant.variant, reg, profile, output), expected);
    CHECK(output == (expected == t::Err::OK ? expectedEncoding(variant, reg, profile) : 0xD3));
    CHECK(sameProfile(profile, before));
  }
}

void encodeDomains() {
  for (const auto& variant : VARIANTS) {
    // Every complete semantic combination, including all unsupported variant/value pairs.
    for (unsigned int mode = 0; mode < 4; ++mode) {
      for (unsigned int source = 0; source < 3; ++source) {
        for (unsigned int preference = 0; preference < 3; ++preference) {
          for (unsigned int duty = 0; duty < 4; ++duty) {
            for (unsigned int debounce = 0; debounce < 7; ++debounce) {
              for (unsigned int flags = 0; flags < 8; ++flags) {
                t::DeviceProfile profile{};
                profile.portMode = static_cast<t::PortMode>(mode);
                profile.sourceCurrent = static_cast<t::SourceCurrent>(source);
                profile.sourcePreference = static_cast<t::SourcePreference>(preference);
                profile.drpDutyCycle = static_cast<t::DrpDutyCycle>(duty);
                profile.ccDebounce = static_cast<t::CcDebounce>(debounce);
                profile.disableUfpAccessory = (flags & 1U) != 0;
                profile.disableTerm = (flags & 2U) != 0;
                profile.disableRdRp = (flags & 4U) != 0;
                checkEncoding(variant, profile);
              }
            }
          }
        }
      }
    }
    for (unsigned int value = 0; value < 256; ++value) {
      t::DeviceProfile profile{};
      profile.portMode = static_cast<t::PortMode>(value);
      checkEncoding(variant, profile);
      profile = {};
      profile.sourceCurrent = static_cast<t::SourceCurrent>(value);
      checkEncoding(variant, profile);
      profile = {};
      profile.sourcePreference = static_cast<t::SourcePreference>(value);
      checkEncoding(variant, profile);
      profile = {};
      profile.drpDutyCycle = static_cast<t::DrpDutyCycle>(value);
      checkEncoding(variant, profile);
      profile = {};
      profile.ccDebounce = static_cast<t::CcDebounce>(value);
      checkEncoding(variant, profile);
    }
    for (unsigned int reg = 0; reg < 256; ++reg) {
      if (reg == 0x08 || reg == 0x09 || reg == 0x0A || reg == 0x45) continue;
      uint8_t output = 0xB7;
      checkStatus(p::encodeProfileRegister(variant.variant, static_cast<uint8_t>(reg), {}, output),
                  t::Err::INVALID_PARAM);
      CHECK(output == 0xB7);
    }
  }
  for (unsigned int variant = 0; variant < 256; ++variant) {
    if (variant >= 1 && variant <= 7) continue;
    uint8_t output = 0x6A;
    checkStatus(p::encodeProfileRegister(static_cast<t::Variant>(variant), 0x08, {}, output),
                t::Err::INVALID_CONFIG);
    CHECK(output == 0x6A);
  }
}

void compareSettings() {
  for (const auto& variant : VARIANTS) {
    for (const uint8_t reg : {uint8_t{0x08}, uint8_t{0x09}, uint8_t{0x0A}, uint8_t{0x45}}) {
      if (variant.is322 && reg == 0x45) continue;
      for (unsigned int left = 0; left < 256; ++left) {
        for (unsigned int right = 0; right < 256; ++right) {
          context(variant, reg, left);
          const bool invalid =
              reg == 0x08 ? ((left >> 6) == 3 || (right >> 6) == 3)
              : reg == 0x0A && !variant.base ? ((left & 6U) == 4 || (right & 6U) == 4)
                                           : false;
          unsigned int mask = reg == 0x08   ? 0xC0U
                              : reg == 0x09 ? (variant.base ? 6U : 7U)
                              : reg == 0x0A ? (variant.base ? 0xF0U : 0xF7U)
                                            : 4U;
          unsigned int lhs = left & mask;
          unsigned int rhs = right & mask;
          if (variant.is322 && reg == 0x0A) {
            if ((lhs & 0x30U) == 0) lhs |= 0x30U;
            if ((rhs & 0x30U) == 0) rhs |= 0x30U;
          }
          // Start both ways so failure cannot accidentally look like a legitimate equality.
          bool output = ((left + right) & 1U) != 0;
          const bool before = output;
          checkStatus(p::comparePersistentSettings(variant.variant, reg, static_cast<uint8_t>(left),
                                                   static_cast<uint8_t>(right), output),
                      invalid ? t::Err::CONFIGURATION_UNKNOWN : t::Err::OK);
          CHECK(output == (invalid ? before : lhs == rhs));
        }
      }
    }
    for (unsigned int reg = 0; reg < 256; ++reg) {
      if (reg == 0x08 || reg == 0x09 || reg == 0x0A || (!variant.is322 && reg == 0x45)) continue;
      bool output = true;
      checkStatus(p::comparePersistentSettings(variant.variant, static_cast<uint8_t>(reg), 0, 0,
                                               output),
                  variant.is322 && reg == 0x45 ? t::Err::UNSUPPORTED_FEATURE
                                              : t::Err::INVALID_PARAM);
      CHECK(output);
    }
  }
  for (unsigned int variant = 0; variant < 256; ++variant) {
    if (variant >= 1 && variant <= 7) continue;
    bool output = false;
    checkStatus(p::comparePersistentSettings(static_cast<t::Variant>(variant), 0x08, 0, 0, output),
                t::Err::INVALID_PARAM);
    CHECK(!output);
  }
  bool output = false;
  checkStatus(p::comparePersistentSettings(t::Variant::TUSB322I, 0x0A, 0x02, 0x32, output),
              t::Err::OK);
  CHECK(output);
  checkStatus(p::comparePersistentSettings(t::Variant::TUSB322I, 0x0A, 0x02, 0x30, output),
              t::Err::OK);
  CHECK(!output);
}

p::WriteEvidence evidence(uint8_t actual) {
  p::WriteEvidence result{};
  result.actual = actual;
  result.actualValid = true;
  result.effectsResolved = true;
  return result;
}

p::WriteRequest request(t::Variant variant, uint8_t reg, p::WriteIntent intent,
                        uint16_t fields = 0) {
  p::WriteRequest result{};
  result.variant = variant;
  result.reg = reg;
  result.intent = intent;
  result.fields = fields;
  return result;
}

void compose(const p::WriteRequest& input, const p::WriteEvidence& actual, t::Err expected,
             uint8_t byte = 0) {
  contextVariant = static_cast<unsigned int>(input.variant);
  contextReg = input.reg;
  contextValue = actual.actual;
  uint8_t output = 0xD3;
  checkStatus(p::composeWrite(input, actual, output), expected);
  CHECK(output == (expected == t::Err::OK ? byte : 0xD3));
}

void composeIrqResetAndOrdinary() {
  for (const auto& variant : VARIANTS) {
    for (unsigned int value = 0; value < 256; ++value) {
      const auto actual = evidence(static_cast<uint8_t>(value));
      auto ack = request(variant.variant, 0x09, p::WriteIntent::ACK_IRQ);
      ack.desired.portMode = static_cast<t::PortMode>(0xFF);  // Ignored for fixed intents.
      compose(ack, actual, variant.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::OK,
              static_cast<uint8_t>((value & (variant.base ? 6U : 7U)) | 0x10U));

      auto reset = request(variant.variant, 0x0A, p::WriteIntent::RESET);
      reset.desired.sourceCurrent = static_cast<t::SourceCurrent>(0xFF);
      const bool unknown = (value & 8U) != 0 || (!variant.base && (value & 6U) == 4);
      compose(reset, actual, unknown ? t::Err::CONFIGURATION_UNKNOWN : t::Err::OK,
              static_cast<uint8_t>((value & (variant.base ? 0xF0U : 0xF7U)) | 8U));

      auto source = request(variant.variant, 0x08, p::WriteIntent::SETTINGS,
                            field(t::ProfileField::SOURCE_CURRENT));
      source.desired.sourceCurrent = t::SourceCurrent::MA_1500;
      compose(source, actual,
              variant.is322 ? t::Err::UNSUPPORTED_FEATURE
              : (value & 0xC0U) == 0xC0 ? t::Err::CONFIGURATION_UNKNOWN
                                      : t::Err::OK,
              0x40);

      auto duty = request(variant.variant, 0x09, p::WriteIntent::SETTINGS,
                          field(t::ProfileField::DRP_DUTY_CYCLE));
      duty.desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_40;
      compose(duty, actual, variant.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::OK,
              static_cast<uint8_t>(0x02U | (variant.base ? 0U : value & 1U)));
    }
    auto ack = request(variant.variant, 0x09, p::WriteIntent::ACK_IRQ);
    ack.desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_40;
    compose(ack, evidence(variant.base ? 0xB4 : 0xB5),
            variant.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::OK, variant.base ? 0x14 : 0x15);
    compose(ack, evidence(0x06), variant.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::OK, 0x16);
    auto unknownEffects = evidence(0x06);
    unknownEffects.effectsResolved = false;
    compose(ack, unknownEffects, variant.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::OK, 0x16);
    auto reset = request(variant.variant, 0x0A, p::WriteIntent::RESET);
    unknownEffects.actual = 0xC0;
    compose(reset, unknownEffects, t::Err::OK, 0xC8);
    unknownEffects.actualValid = false;
    compose(reset, unknownEffects, t::Err::CONFIGURATION_UNKNOWN);
    if (!variant.base && !variant.is322) {
      auto settings =
          request(variant.variant, 0x09, p::WriteIntent::SETTINGS,
                  field(t::ProfileField::DRP_DUTY_CYCLE) |
                      field(t::ProfileField::DISABLE_UFP_ACCESSORY));
      settings.desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_40;
      compose(settings, evidence(0xB5), t::Err::OK, 0x02);
    }
    // Complete valid 0A controls are preserved while every available debounce value changes.
    for (unsigned int value = 0; value < 256; ++value) {
      for (unsigned int debounceCode = 0; debounceCode < 4; ++debounceCode) {
        auto settings = request(variant.variant, 0x0A, p::WriteIntent::SETTINGS,
                                field(t::ProfileField::CC_DEBOUNCE));
        settings.desired.ccDebounce =
            variant.base ? BASE_DEBOUNCE[debounceCode] : EXTENDED_DEBOUNCE[debounceCode];
        const bool unknown = (value & 8U) != 0 || (!variant.base && (value & 6U) == 4);
        const uint8_t expected = static_cast<uint8_t>((debounceCode << 6) |
                                                       (value & (variant.base ? 0x30U : 0x37U)));
        compose(settings, evidence(static_cast<uint8_t>(value)),
                unknown ? t::Err::CONFIGURATION_UNKNOWN : t::Err::OK, expected);
      }
    }
    if (!variant.base) {
      auto term = request(variant.variant, 0x0A, p::WriteIntent::SETTINGS,
                          field(t::ProfileField::DISABLE_TERM));
      term.desired.disableTerm = true;
      compose(term, evidence(0xC2), t::Err::OK, 0xC3);
      term.desired.disableTerm = false;
      compose(term, evidence(0xC3), t::Err::OK, 0xC2);
    }
  }
  compose(request(t::Variant::TUSB320LAI, 0x0A, p::WriteIntent::RESET), evidence(0xE2),
          t::Err::OK, 0xEA);
  compose(request(t::Variant::TUSB320, 0x0A, p::WriteIntent::RESET), evidence(0xE7),
          t::Err::OK, 0xE8);
  compose(request(t::Variant::TUSB322I, 0x0A, p::WriteIntent::RESET), evidence(0xC2),
          t::Err::OK, 0xCA);

  auto debounce = request(t::Variant::TUSB320LAI, 0x0A, p::WriteIntent::SETTINGS,
                          field(t::ProfileField::CC_DEBOUNCE));
  debounce.desired.ccDebounce = t::CcDebounce::MS_118;
  compose(debounce, evidence(0xDE), t::Err::CONFIGURATION_UNKNOWN);
  // C09 proves the field arithmetic separately. A read reset=1 forbids a safe write.
  t::RegisterDecode decoded{};
  checkStatus(t::decodeRegister(t::Variant::TUSB320LAI, 0x0A, 0xDE, decoded), t::Err::OK);
  CHECK(decoded.resetRequested);
  decoded.profileCodes.ccDebounce = t::CcDebounce::MS_118;
  uint8_t encoded = 0;
  checkStatus(p::encodeProfileRegister(t::Variant::TUSB320LAI, 0x0A, decoded.profileCodes, encoded),
              t::Err::OK);
  CHECK(encoded == 0x56);
  compose(debounce, evidence(0xD6), t::Err::OK, 0x56);
}

void composeModesAndRecovery() {
  for (const auto& variant : VARIANTS) {
    if (!variant.base) {
      auto disable = request(variant.variant, 0x0A, p::WriteIntent::MODE_DISABLE);
      compose(disable, evidence(0x00), t::Err::OK, 0x01);
      auto change = request(variant.variant, 0x0A, p::WriteIntent::MODE_CHANGE,
                            field(t::ProfileField::PORT_MODE));
      auto actual = evidence(0x01);
      compose(change, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.disableWritten = true;
      compose(change, actual, variant.is322 ? t::Err::CONFIGURATION_UNKNOWN : t::Err::OK, 0x31);
      actual.actual = 0x31;
      compose(change, actual, t::Err::CONFIGURATION_UNKNOWN);  // No actual mode change.
      actual.actual = 0x01;
      change.fields = field(t::ProfileField::CC_DEBOUNCE);
      compose(change, actual, t::Err::CONFIGURATION_UNKNOWN);  // Mode field must be selected.
      change.fields = field(t::ProfileField::PORT_MODE);
      if (variant.is322) {
        actual.actual = 0x11;  // UFP -> DRP is the explicit unresolved return gate.
        compose(change, actual, t::Err::UNSUPPORTED_FEATURE);
        change.desired.portMode = t::PortMode::DFP;
        compose(change, actual, t::Err::OK, 0x21);
      }
      auto enable = request(variant.variant, 0x0A, p::WriteIntent::MODE_ENABLE);
      actual = evidence(0x31);
      compose(enable, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.disableWritten = true;
      compose(enable, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.modeWritten = true;
      compose(enable, actual, variant.is322 ? t::Err::OK : t::Err::CONFIGURATION_UNKNOWN, 0x30);
      actual.minimumWaitElapsed = true;
      compose(enable, actual, t::Err::OK, 0x30);
      actual.actual = 0x30;
      compose(enable, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual = evidence(0x01);
      actual.disableWritten = true;
      change.desired.portMode = t::PortMode::DFP;
      actual.actual = 0x00;
      compose(change, actual, t::Err::CONFIGURATION_UNKNOWN);
    } else {
      auto change = request(variant.variant, 0x0A, p::WriteIntent::BASE_MODE_CHANGE,
                            field(t::ProfileField::PORT_MODE));
      change.desired.portMode = t::PortMode::DFP;
      auto actual = evidence(0xC0);
      compose(change, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.detached = true;
      compose(change, actual, t::Err::OK, 0xE0);
      change.fields = field(t::ProfileField::CC_DEBOUNCE);
      compose(change, actual, t::Err::CONFIGURATION_UNKNOWN);
      change.fields = field(t::ProfileField::PORT_MODE);
      actual.actual = 0xE0;
      compose(change, actual, t::Err::CONFIGURATION_UNKNOWN);
    }
    if (!variant.is322) {
      auto ordinary = request(variant.variant, 0x45, p::WriteIntent::SETTINGS,
                              field(t::ProfileField::DISABLE_RD_RP));
      for (unsigned int value = 0; value < 256; ++value) {
        ordinary.desired.disableRdRp = true;
        compose(ordinary, evidence(static_cast<uint8_t>(value)),
                (value & 3U) != 0 ? t::Err::CONFIGURATION_UNKNOWN : t::Err::OK, 0x04);
        ordinary.desired.disableRdRp = false;
        compose(ordinary, evidence(static_cast<uint8_t>(value)),
                (value & 3U) != 0 ? t::Err::CONFIGURATION_UNKNOWN : t::Err::OK, 0x00);
      }
    }
    auto recover = request(variant.variant, 0x45, p::WriteIntent::RECOVERY_DISABLE);
    auto actual = evidence(0x00);
    if (variant.base) {
      compose(recover, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.recoveryQualified = true;
      compose(recover, actual, t::Err::OK, 0x04);
      actual.actual = 0x03;
      compose(recover, actual, t::Err::CONFIGURATION_UNKNOWN);
      recover.intent = p::WriteIntent::RECOVERY_ENABLE;
      actual.actual = 0x04;
      compose(recover, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.disableWritten = true;
      compose(recover, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.minimumWaitElapsed = true;
      compose(recover, actual, t::Err::OK, 0x00);
      actual.actual = 0x00;
      compose(recover, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.actual = 0x07;
      compose(recover, actual, t::Err::CONFIGURATION_UNKNOWN);
      actual.actual = 0x04;
      actual.recoveryQualified = false;
      compose(recover, actual, t::Err::CONFIGURATION_UNKNOWN);
    } else {
      actual.recoveryQualified = true;
      compose(recover, actual, t::Err::UNSUPPORTED_FEATURE);
      recover.intent = p::WriteIntent::RECOVERY_ENABLE;
      actual.actual = 0x04;
      actual.disableWritten = true;
      actual.minimumWaitElapsed = true;
      compose(recover, actual, t::Err::UNSUPPORTED_FEATURE);
    }
  }
  auto preference = request(t::Variant::TUSB322I, 0x0A, p::WriteIntent::SETTINGS,
                            field(t::ProfileField::SOURCE_PREFERENCE) |
                            field(t::ProfileField::PORT_MODE));
  preference.desired.sourcePreference = t::SourcePreference::TRY_SRC;
  compose(preference, evidence(0x02), t::Err::OK, 0x06);  // Preserve actual DRP00.
  compose(preference, evidence(0x32), t::Err::OK, 0x36);
  auto current = request(t::Variant::TUSB320, 0x08, p::WriteIntent::SETTINGS,
                        field(t::ProfileField::SOURCE_CURRENT));
  current.desired.sourceCurrent = t::SourceCurrent::MA_3000;
  compose(current, evidence(0x40), t::Err::OK, 0x80);  // No detached proof needed for current.
}

void composeRejections() {
  for (const auto& variant : VARIANTS) {
    // Missing actual evidence and unrelated unresolved effects cannot be repaired by desired state.
    auto settings = request(variant.variant, 0x0A, p::WriteIntent::SETTINGS,
                            field(t::ProfileField::CC_DEBOUNCE));
    auto actual = evidence(0xC0);
    actual.actualValid = false;
    compose(settings, actual, t::Err::CONFIGURATION_UNKNOWN);
    actual = evidence(0xC0);
    actual.effectsResolved = false;
    compose(settings, actual, t::Err::CONFIGURATION_UNKNOWN);
    settings.fields = 0;
    compose(settings, evidence(0xC0), t::Err::INVALID_PARAM);
    for (unsigned int bit = 0x0100; bit <= 0x8000; bit <<= 1) {
      settings.fields = static_cast<uint16_t>(bit);
      compose(settings, evidence(0xC0), t::Err::INVALID_PARAM);
    }
    settings.fields = field(t::ProfileField::SOURCE_CURRENT);
    compose(settings, evidence(0xC0), t::Err::INVALID_PARAM);
    settings.fields = field(t::ProfileField::CC_DEBOUNCE);
    settings.desired.portMode = static_cast<t::PortMode>(0xFF);
    compose(settings, evidence(0xC0), t::Err::INVALID_CONFIG);
    settings.desired = {};
    settings.fields = field(t::ProfileField::PORT_MODE);
    compose(settings, evidence(0xD0),
            variant.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::CONFIGURATION_UNKNOWN);
    settings.fields = field(t::ProfileField::CC_DEBOUNCE);
    compose(settings, evidence(0xC8), t::Err::CONFIGURATION_UNKNOWN);
    if (!variant.base) {
      // Even when preference is being replaced, actual reserved preference is insufficient proof.
      settings.fields = field(t::ProfileField::SOURCE_PREFERENCE);
      compose(settings, evidence(0xC4), t::Err::CONFIGURATION_UNKNOWN);
    } else {
      settings.fields = field(t::ProfileField::SOURCE_PREFERENCE);
      compose(settings, evidence(0xC0), t::Err::UNSUPPORTED_FEATURE);
      settings.fields = field(t::ProfileField::DISABLE_TERM);
      compose(settings, evidence(0xC0), t::Err::UNSUPPORTED_FEATURE);
      settings.reg = 0x09;
      settings.fields = field(t::ProfileField::DISABLE_UFP_ACCESSORY);
      compose(settings, evidence(0), t::Err::UNSUPPORTED_FEATURE);
      settings.reg = 0x0A;
    }
    for (unsigned int intent = 9; intent < 256; ++intent) {
      settings.intent = static_cast<p::WriteIntent>(intent);
      settings.fields = 0;
      compose(settings, evidence(0), t::Err::INVALID_PARAM);
    }
    for (const auto intent : {p::WriteIntent::ACK_IRQ, p::WriteIntent::RESET,
                             p::WriteIntent::MODE_DISABLE, p::WriteIntent::MODE_ENABLE,
                             p::WriteIntent::RECOVERY_DISABLE, p::WriteIntent::RECOVERY_ENABLE}) {
      if (variant.base && (intent == p::WriteIntent::MODE_DISABLE ||
                           intent == p::WriteIntent::MODE_ENABLE)) {
        continue;
      }
      if (!variant.base && (intent == p::WriteIntent::RECOVERY_DISABLE ||
                            intent == p::WriteIntent::RECOVERY_ENABLE)) {
        continue;
      }
      const uint8_t reg =
          intent == p::WriteIntent::ACK_IRQ ? 0x09
          : intent == p::WriteIntent::RECOVERY_DISABLE ||
                    intent == p::WriteIntent::RECOVERY_ENABLE ? 0x45
                                                            : 0x0A;
      if (variant.is322 && reg == 0x09) continue;
      auto invalid = request(variant.variant, reg, intent, field(t::ProfileField::CC_DEBOUNCE));
      compose(invalid, evidence(0), t::Err::INVALID_PARAM);
    }
    for (unsigned int reg = 0; reg < 256; ++reg) {
      if (reg == 0x08 || reg == 0x09 || reg == 0x0A || reg == 0x45) continue;
      auto invalid = request(variant.variant, static_cast<uint8_t>(reg), p::WriteIntent::SETTINGS,
                             field(t::ProfileField::SOURCE_CURRENT));
      compose(invalid, evidence(0), t::Err::INVALID_PARAM);
    }
  }
  for (unsigned int variant = 0; variant < 256; ++variant) {
    if (variant >= 1 && variant <= 7) continue;
    compose(request(static_cast<t::Variant>(variant), 0x0A, p::WriteIntent::RESET), evidence(0),
            t::Err::INVALID_CONFIG);
  }
}

void composeIntentRegisterContracts() {
  for (const auto& variant : VARIANTS) {
    for (const uint8_t reg : {uint8_t{0x08}, uint8_t{0x09}, uint8_t{0x0A}, uint8_t{0x45}}) {
      for (unsigned int rawIntent = 0; rawIntent <= 8; ++rawIntent) {
        const auto intent = static_cast<p::WriteIntent>(rawIntent);
        uint16_t fields = 0;
        if (intent == p::WriteIntent::SETTINGS || intent == p::WriteIntent::BASE_MODE_CHANGE ||
            intent == p::WriteIntent::MODE_CHANGE) {
          fields = reg == 0x08 ? field(t::ProfileField::SOURCE_CURRENT)
                   : reg == 0x09 ? field(t::ProfileField::DRP_DUTY_CYCLE)
                   : reg == 0x0A ? field(t::ProfileField::PORT_MODE)
                                  : field(t::ProfileField::DISABLE_RD_RP);
        }
        auto input = request(variant.variant, reg, intent, fields);
        if (fields == 0) {
          input.desired.portMode = static_cast<t::PortMode>(0xFF);
          input.desired.ccDebounce = static_cast<t::CcDebounce>(0xFE);
        }
        auto actual = evidence(reg == 0x0A ? 0x11 : 0x04);
        actual.detached = true;
        actual.disableWritten = true;
        actual.modeWritten = true;
        actual.minimumWaitElapsed = true;
        actual.recoveryQualified = true;
        t::Err expected = t::Err::OK;
        uint8_t byte = 0;
        if (variant.is322 && (reg == 0x08 || reg == 0x09 || reg == 0x45)) {
          expected = t::Err::UNSUPPORTED_FEATURE;
        } else {
          switch (intent) {
            case p::WriteIntent::SETTINGS:
              if (reg == 0x0A) {
                expected =
                    variant.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::CONFIGURATION_UNKNOWN;
              } else if (reg == 0x09) {
                byte = 0x04;
              }
              break;
            case p::WriteIntent::ACK_IRQ:
              expected = reg == 0x09 ? t::Err::OK : t::Err::INVALID_PARAM;
              byte = 0x14;
              break;
            case p::WriteIntent::RESET:
              expected = reg == 0x0A ? t::Err::OK : t::Err::INVALID_PARAM;
              byte = variant.base ? 0x18 : 0x19;
              break;
            case p::WriteIntent::BASE_MODE_CHANGE:
              expected = !variant.base ? t::Err::UNSUPPORTED_FEATURE
                         : reg != 0x0A ? t::Err::INVALID_PARAM : t::Err::OK;
              byte = 0x30;
              break;
            case p::WriteIntent::MODE_DISABLE:
              expected = variant.base ? t::Err::UNSUPPORTED_FEATURE
                         : reg != 0x0A ? t::Err::INVALID_PARAM : t::Err::OK;
              byte = 0x11;
              break;
            case p::WriteIntent::MODE_CHANGE:
              expected = variant.base ? t::Err::UNSUPPORTED_FEATURE
                         : reg != 0x0A ? t::Err::INVALID_PARAM
                         : variant.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::OK;
              byte = 0x31;
              break;
            case p::WriteIntent::MODE_ENABLE:
              expected = variant.base ? t::Err::UNSUPPORTED_FEATURE
                         : reg != 0x0A ? t::Err::INVALID_PARAM : t::Err::OK;
              byte = 0x10;
              break;
            case p::WriteIntent::RECOVERY_DISABLE:
            case p::WriteIntent::RECOVERY_ENABLE:
              expected = !variant.base ? t::Err::UNSUPPORTED_FEATURE
                         : reg != 0x45 ? t::Err::INVALID_PARAM : t::Err::OK;
              byte = intent == p::WriteIntent::RECOVERY_DISABLE ? 0x04 : 0x00;
              break;
          }
        }
        compose(input, actual, expected, byte);
        // Intrinsic access errors precede actual-state proof. The 322I return-to-DRP
        // gate is different: its actual mode must be known before identifying a change.
        const bool stateDependentGate = variant.is322 && reg == 0x0A &&
                                        (intent == p::WriteIntent::SETTINGS ||
                                         intent == p::WriteIntent::MODE_CHANGE);
        if (!stateDependentGate &&
            (expected == t::Err::INVALID_PARAM || expected == t::Err::UNSUPPORTED_FEATURE)) {
          auto missingProof = actual;
          missingProof.actualValid = false;
          compose(input, missingProof, expected);
          missingProof.actualValid = true;
          missingProof.effectsResolved = false;
          compose(input, missingProof, expected);
          missingProof.actualValid = false;
          compose(input, missingProof, expected);
        }
        if (expected == t::Err::OK) {
          actual.actualValid = false;
          compose(input, actual, t::Err::CONFIGURATION_UNKNOWN);
          actual.actualValid = true;
          actual.effectsResolved = false;
          const bool ignoresUnrelatedEffects =
              intent == p::WriteIntent::ACK_IRQ || intent == p::WriteIntent::RESET;
          compose(input, actual,
                  ignoresUnrelatedEffects ? t::Err::OK : t::Err::CONFIGURATION_UNKNOWN, byte);
        }
        if (expected == t::Err::OK && fields != 0) {
          input.desired.sourceCurrent = static_cast<t::SourceCurrent>(0xFF);
          compose(input, actual, t::Err::INVALID_CONFIG);
        }
      }
    }
  }
}

}  // namespace

/// @brief Run codec/composer checks as part of the registered protocol suite; active in Release.
/// @return Zero only if every independent field, preservation and procedure-proof check passes.
int runCodecTests() {
  decodeBytesAndErrors();
  encodeDomains();
  compareSettings();
  composeIrqResetAndOrdinary();
  composeModesAndRecovery();
  composeRejections();
  composeIntentRegisterContracts();
  std::printf("Codec checks: %zu, failures: %d\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
