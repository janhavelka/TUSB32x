// Local binding/profile validation: enum and capability boundaries, preserved inputs and zero I/O.
#include <TUSB32x/Config.h>
#include <TUSB32x/Status.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>

namespace {

namespace t = TUSB32x;

int failures = 0;
std::size_t callbackAttempts = 0;
std::size_t validations = 0;

void check(bool condition, const char* expression, int line) {
  if (!condition) {
    std::printf("FAIL line %d: %s\n", line, expression);
    ++failures;
  }
}

// Explicit checks stay active with NDEBUG in Release builds.
#define CHECK(expression) check((expression), #expression, __LINE__)

// Any attempt is a test failure even with null context; no fake device can report success.
t::Status countingWrite(uint8_t, const uint8_t*, std::size_t, uint32_t, void*) {
  ++callbackAttempts;
  return t::Status::Error(t::Err::I2C_ERROR, "unexpected validation write", -70000);
}

t::Status countingWriteRead(uint8_t, const uint8_t*, std::size_t, uint8_t*,
                            std::size_t, uint32_t, void*) {
  ++callbackAttempts;
  return t::Status::Error(t::Err::I2C_ERROR, "unexpected validation read", 100000);
}

struct VariantCase {
  t::Variant variant;
  t::DeviceAddress lowAddress;
  t::DeviceAddress highAddress;
  bool baseControls;
  bool is322;
};

// Independent expectations from docs/register-map.md, B/L/A/V tables; no register encodings here.
constexpr VariantCase VARIANTS[] = {
    {t::Variant::TUSB320, t::DeviceAddress::ADDR_LOW_0X60,
     t::DeviceAddress::ADDR_HIGH_0X61, true, false},
    {t::Variant::TUSB320I, t::DeviceAddress::ADDR_LOW_0X60,
     t::DeviceAddress::ADDR_HIGH_0X61, true, false},
    {t::Variant::TUSB320LI, t::DeviceAddress::ADDR_LOW_0X47,
     t::DeviceAddress::ADDR_HIGH_0X67, false, false},
    {t::Variant::TUSB320HI, t::DeviceAddress::ADDR_LOW_0X47,
     t::DeviceAddress::ADDR_HIGH_0X67, false, false},
    {t::Variant::TUSB320LAI, t::DeviceAddress::ADDR_LOW_0X47,
     t::DeviceAddress::ADDR_HIGH_0X67, false, false},
    {t::Variant::TUSB320HAI, t::DeviceAddress::ADDR_LOW_0X47,
     t::DeviceAddress::ADDR_HIGH_0X67, false, false},
    {t::Variant::TUSB322I, t::DeviceAddress::ADDR_LOW_0X47,
     t::DeviceAddress::ADDR_HIGH_0X67, false, true},
};

constexpr t::PortMode MODES[] = {
    t::PortMode::FOLLOW_PORT_PIN, t::PortMode::UFP, t::PortMode::DFP, t::PortMode::DRP};
constexpr t::SourceCurrent CURRENTS[] = {
    t::SourceCurrent::DEFAULT, t::SourceCurrent::MA_1500, t::SourceCurrent::MA_3000};
constexpr t::SourcePreference PREFERENCES[] = {
    t::SourcePreference::NORMAL, t::SourcePreference::TRY_SNK, t::SourcePreference::TRY_SRC};
constexpr t::DrpDutyCycle DUTIES[] = {
    t::DrpDutyCycle::PERCENT_30, t::DrpDutyCycle::PERCENT_40,
    t::DrpDutyCycle::PERCENT_50, t::DrpDutyCycle::PERCENT_60};
constexpr t::CcDebounce DEBOUNCES[] = {
    t::CcDebounce::MS_116, t::CcDebounce::MS_118, t::CcDebounce::MS_133,
    t::CcDebounce::MS_134, t::CcDebounce::MS_151, t::CcDebounce::MS_152,
    t::CcDebounce::MS_168};

template <typename Enum, std::size_t count>
bool contains(const Enum (&values)[count], Enum value) {
  for (const auto candidate : values) {
    if (candidate == value) return true;
  }
  return false;
}

bool knownVariant(t::Variant variant) {
  for (const auto& expected : VARIANTS) {
    if (expected.variant == variant) return true;
  }
  return false;
}

bool sameConfig(const t::DriverConfig& left, const t::DriverConfig& right) {
  return left.i2cWrite == right.i2cWrite && left.i2cWriteRead == right.i2cWriteRead &&
         left.i2cUser == right.i2cUser && left.variant == right.variant &&
         left.address == right.address && left.i2cTimeoutMs == right.i2cTimeoutMs;
}

bool sameProfile(const t::DeviceProfile& left, const t::DeviceProfile& right) {
  return left.portMode == right.portMode && left.sourceCurrent == right.sourceCurrent &&
         left.sourcePreference == right.sourcePreference && left.drpDutyCycle == right.drpDutyCycle &&
         left.ccDebounce == right.ccDebounce && left.disableUfpAccessory == right.disableUfpAccessory &&
         left.disableTerm == right.disableTerm && left.disableRdRp == right.disableRdRp;
}

void checkStatus(const t::Status& status, t::Err expected) {
  if (status.code != expected) {
    std::printf("Validation %zu: expected %s, received %s\n", validations,
                t::errorName(expected), t::errorName(status.code));
  }
  CHECK(status.code == expected);
  CHECK(status.detail == 0);
  CHECK(status.msg != nullptr);
  if (status.msg != nullptr) CHECK(status.msg[0] != '\0');
}

t::Status checkConfig(const t::DriverConfig& config, t::Err expected) {
  const auto before = config;
  const auto attemptsBefore = callbackAttempts;
  const auto status = t::validateDriverConfig(config);
  ++validations;
  checkStatus(status, expected);
  CHECK(callbackAttempts == attemptsBefore);
  CHECK(sameConfig(config, before));
  return status;
}

t::Status checkProfile(t::Variant variant, const t::DeviceProfile& profile, t::Err expected) {
  const auto before = profile;
  const auto attemptsBefore = callbackAttempts;
  const auto status = t::validateProfile(variant, profile);
  ++validations;
  checkStatus(status, expected);
  CHECK(callbackAttempts == attemptsBefore);
  CHECK(sameProfile(profile, before));
  return status;
}

t::DriverConfig validConfig(const VariantCase& expected) {
  t::DriverConfig config{};
  config.i2cWrite = countingWrite;
  config.i2cWriteRead = countingWriteRead;
  config.variant = expected.variant;
  config.address = expected.lowAddress;
  return config;
}

void bindingAddressesCallbacksAndTimeouts() {
  checkConfig({}, t::Err::INVALID_CONFIG);
  int context = 42;
  for (const auto& expected : VARIANTS) {
    auto config = validConfig(expected);
    for (unsigned int byte = 0; byte <= 255; ++byte) {
      config.address = static_cast<t::DeviceAddress>(byte);
      const bool supported = config.address == expected.lowAddress ||
                             config.address == expected.highAddress;
      checkConfig(config, supported ? t::Err::OK : t::Err::INVALID_CONFIG);
    }
    config.address = expected.lowAddress;
    constexpr uint32_t TIMEOUTS[] = {1, 20, std::numeric_limits<uint32_t>::max()};
    for (const auto timeout : TIMEOUTS) {
      config.i2cTimeoutMs = timeout;
      config.i2cUser = nullptr;
      checkConfig(config, t::Err::OK);
      config.i2cUser = &context;
      checkConfig(config, t::Err::OK);
    }
    config.i2cTimeoutMs = 0;
    checkConfig(config, t::Err::INVALID_CONFIG);
    config.i2cTimeoutMs = 1;
    for (unsigned int missing = 1; missing <= 3; ++missing) {
      config.i2cWrite = (missing & 1U) != 0 ? nullptr : countingWrite;
      config.i2cWriteRead = (missing & 2U) != 0 ? nullptr : countingWriteRead;
      checkConfig(config, t::Err::INVALID_CONFIG);
    }
  }
  CHECK(context == 42);

  auto config = validConfig(VARIANTS[0]);
  for (unsigned int byte = 0; byte <= 255; ++byte) {
    config.variant = static_cast<t::Variant>(byte);
    if (!knownVariant(config.variant)) checkConfig(config, t::Err::INVALID_CONFIG);
  }
}

void profileCapabilityCombinations() {
  for (const auto& expected : VARIANTS) {
    checkProfile(expected.variant, {}, t::Err::OK);
    for (const auto mode : MODES) {
      for (const auto current : CURRENTS) {
        for (const auto preference : PREFERENCES) {
          for (const auto duty : DUTIES) {
            for (const auto debounce : DEBOUNCES) {
              for (unsigned int flags = 0; flags < 8; ++flags) {
                t::DeviceProfile profile{};
                profile.portMode = mode;
                profile.sourceCurrent = current;
                profile.sourcePreference = preference;
                profile.drpDutyCycle = duty;
                profile.ccDebounce = debounce;
                profile.disableUfpAccessory = (flags & 1U) != 0;
                profile.disableTerm = (flags & 2U) != 0;
                profile.disableRdRp = (flags & 4U) != 0;
                const bool supportedDebounce = expected.baseControls
                    ? debounce == t::CcDebounce::MS_116 || debounce == t::CcDebounce::MS_133 ||
                      debounce == t::CcDebounce::MS_151 || debounce == t::CcDebounce::MS_168
                    : debounce == t::CcDebounce::MS_118 || debounce == t::CcDebounce::MS_134 ||
                      debounce == t::CcDebounce::MS_152 || debounce == t::CcDebounce::MS_168;
                const bool supportedControls = !expected.baseControls ||
                    (preference == t::SourcePreference::NORMAL &&
                     !profile.disableUfpAccessory && !profile.disableTerm);
                const bool supported322 = !expected.is322 ||
                    (mode != t::PortMode::FOLLOW_PORT_PIN && !profile.disableRdRp);
                const auto result = supportedDebounce && supportedControls && supported322
                    ? t::Err::OK : t::Err::UNSUPPORTED_FEATURE;
                checkProfile(expected.variant, profile, result);
              }
            }
          }
        }
      }
    }
  }
}

// Malformed enums take precedence even when another well-formed field is unsupported.
void malformedProfileEnums() {
  for (const auto& expected : VARIANTS) {
    t::DeviceProfile valid{};
    t::DeviceProfile unsupported{};
    if (expected.baseControls) {
      unsupported.disableTerm = true;
      unsupported.sourcePreference = t::SourcePreference::TRY_SNK;
      unsupported.ccDebounce = t::CcDebounce::MS_118;
    } else {
      unsupported.ccDebounce = t::CcDebounce::MS_116;
      if (expected.is322) unsupported.disableRdRp = true;
    }
    checkProfile(expected.variant, unsupported, t::Err::UNSUPPORTED_FEATURE);
    const t::DeviceProfile INITIALS[] = {valid, unsupported};
    for (unsigned int byte = 0; byte <= 255; ++byte) {
      const auto mode = static_cast<t::PortMode>(byte);
      const auto current = static_cast<t::SourceCurrent>(byte);
      const auto preference = static_cast<t::SourcePreference>(byte);
      const auto duty = static_cast<t::DrpDutyCycle>(byte);
      const auto debounce = static_cast<t::CcDebounce>(byte);
      for (const auto& initial : INITIALS) {
        if (!contains(MODES, mode)) {
          auto profile = initial;
          profile.portMode = mode;
          checkProfile(expected.variant, profile, t::Err::INVALID_CONFIG);
        }
        if (!contains(CURRENTS, current)) {
          auto profile = initial;
          profile.sourceCurrent = current;
          checkProfile(expected.variant, profile, t::Err::INVALID_CONFIG);
        }
        if (!contains(PREFERENCES, preference)) {
          auto profile = initial;
          profile.sourcePreference = preference;
          checkProfile(expected.variant, profile, t::Err::INVALID_CONFIG);
        }
        if (!contains(DUTIES, duty)) {
          auto profile = initial;
          profile.drpDutyCycle = duty;
          checkProfile(expected.variant, profile, t::Err::INVALID_CONFIG);
        }
        if (!contains(DEBOUNCES, debounce)) {
          auto profile = initial;
          profile.ccDebounce = debounce;
          checkProfile(expected.variant, profile, t::Err::INVALID_CONFIG);
        }
      }
    }
  }
  for (unsigned int byte = 0; byte <= 255; ++byte) {
    const auto variant = static_cast<t::Variant>(byte);
    if (!knownVariant(variant)) checkProfile(variant, {}, t::Err::INVALID_CONFIG);
  }
}

void retainedStatusText() {
  t::Status bindingError{};
  t::Status profileError{};
  t::Status unsupported{};
  {
    auto config = validConfig(VARIANTS[0]);
    config.i2cTimeoutMs = 0;
    bindingError = checkConfig(config, t::Err::INVALID_CONFIG);
    t::DeviceProfile profile{};
    profile.sourceCurrent = static_cast<t::SourceCurrent>(255);
    profileError = checkProfile(t::Variant::TUSB320LAI, profile, t::Err::INVALID_CONFIG);
    profile = {};
    profile.disableTerm = true;
    unsupported = checkProfile(t::Variant::TUSB320, profile, t::Err::UNSUPPORTED_FEATURE);
  }
  // Retain diagnostic bytes after caller values die and after many later validator calls.
  const std::string bindingText = bindingError.msg == nullptr ? "" : bindingError.msg;
  const std::string profileText = profileError.msg == nullptr ? "" : profileError.msg;
  const std::string unsupportedText = unsupported.msg == nullptr ? "" : unsupported.msg;
  bindingAddressesCallbacksAndTimeouts();
  profileCapabilityCombinations();
  malformedProfileEnums();
  if (bindingError.msg != nullptr) CHECK(std::strcmp(bindingError.msg, bindingText.c_str()) == 0);
  if (profileError.msg != nullptr) CHECK(std::strcmp(profileError.msg, profileText.c_str()) == 0);
  if (unsupported.msg != nullptr) CHECK(std::strcmp(unsupported.msg, unsupportedText.c_str()) == 0);
}

#undef CHECK

}  // namespace

int main() {
  retainedStatusText();
  if (callbackAttempts != 0) {
    std::printf("FAIL validation invoked %zu callbacks\n", callbackAttempts);
    ++failures;
  }
  if (failures != 0) return 1;
  std::printf("PASS %zu binding/profile validations; zero transport callbacks\n", validations);
  return 0;
}
