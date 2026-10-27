/// @file test_profile_application.cpp
/// @brief Literal TI-sequence, public desired/actual, write-fault and physical-time checks.
#include <TUSB32x/TUSB32x.h>

#include "support/FakeTusb32xDevice.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <type_traits>

namespace {

namespace t = TUSB32x;
using Driver = t::TUSB32x;
using Fake = tusb32x_test::FakeTusb32xDevice;
using Start = t::Status (Driver::*)(const t::OperationTiming&, t::OperationToken&);
using Apply = t::Status (Driver::*)(const t::DeviceProfile&, const t::OperationTiming&,
                                    t::OperationToken&);
static_assert(std::is_same_v<decltype(&Driver::startApplyProfile), Apply>);
static_assert(std::is_same_v<decltype(&Driver::startVerifyProfile), Start>);

int failures = 0;
std::size_t checks = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    std::printf("FAIL apply line %d: %s\n", line, expression);
    ++failures;
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
struct Part {
  t::Variant variant;
  uint8_t address;
  bool base;
  bool is322;
  uint8_t revision;
};
constexpr Part PARTS[] = {
    {t::Variant::TUSB320, 0x60, true, false, 0},
    {t::Variant::TUSB320I, 0x60, true, false, 0},
    {t::Variant::TUSB320LI, 0x47, false, false, 1},
    {t::Variant::TUSB320HI, 0x47, false, false, 1},
    {t::Variant::TUSB320LAI, 0x47, false, false, 2},
    {t::Variant::TUSB320HAI, 0x47, false, false, 2},
    {t::Variant::TUSB322I, 0x47, false, true, 2},
};

bool sameProfile(const t::DeviceProfile& a, const t::DeviceProfile& b) {
  return a.portMode == b.portMode && a.sourceCurrent == b.sourceCurrent &&
      a.sourcePreference == b.sourcePreference && a.drpDutyCycle == b.drpDutyCycle &&
      a.ccDebounce == b.ccDebounce && a.disableUfpAccessory == b.disableUfpAccessory &&
      a.disableTerm == b.disableTerm && a.disableRdRp == b.disableRdRp;
}

// Reuse the protocol fixture's exact framing/timeout/error checks. Three fixed
// segments accommodate a complete multi-write apply plus F without growing the
// transport fixture or deriving expected values from production codecs.
struct Bus {
  Fake segments[3]{};
  std::size_t count = 0;
  std::size_t calls = 0;
  uint64_t nowUs = 100000;
  uint64_t modeCompletionUs = 0;
  uint64_t enableUs = 0;
  std::size_t modeCall = std::numeric_limits<std::size_t>::max();
  std::size_t enableCall = std::numeric_limits<std::size_t>::max();
  Driver* owner = nullptr;

  void reset() {
    for (auto& segment : segments) segment = Fake{};
    count = 0;
    calls = 0;
    nowUs = 100000;
    modeCompletionUs = 0;
    enableUs = 0;
    modeCall = std::numeric_limits<std::size_t>::max();
    enableCall = std::numeric_limits<std::size_t>::max();
    owner = nullptr;
  }

  Fake::Step& read(uint8_t address, uint8_t reg, std::size_t length = 1) {
    CHECK(count < 24);
    auto& step = segments[count / 8].expectRead(address, reg, length);
    ++count;
    return step;
  }
  Fake::Step& write(uint8_t address, uint8_t reg, uint8_t byte) {
    CHECK(count < 24);
    auto& step = segments[count / 8].expectWrite(address, reg, byte);
    ++count;
    return step;
  }
  Fake::Step& at(std::size_t index) { return segments[index / 8].script[index % 8]; }
  bool complete() const {
    return calls == count && segments[0].complete() && segments[1].complete() &&
        segments[2].complete();
  }
  bool clean() const {
    return segments[0].errors == 0 && segments[1].errors == 0 && segments[2].errors == 0;
  }
  static t::Status writeCallback(uint8_t address, const uint8_t* tx, std::size_t length,
                                  uint32_t timeout, void* user) {
    auto& bus = *static_cast<Bus*>(user);
    const auto index = bus.calls++;
    CHECK(index < 24);
    if (index >= 24) return t::Status::Error(t::Err::I2C_ERROR, "script callback limit exceeded");
    if (bus.owner != nullptr) {
      CHECK(bus.owner->configurationSnapshot().state == t::ConfigurationState::APPLYING);
      CHECK(bus.owner->portSnapshot().invalidated);
    }
    if (index == bus.enableCall) bus.enableUs = bus.nowUs;
    const auto status = Fake::write(address, tx, length, timeout, &bus.segments[index / 8]);
    if (index == bus.modeCall && bus.modeCompletionUs != 0) bus.nowUs = bus.modeCompletionUs;
    return status;
  }
  static t::Status readCallback(uint8_t address, const uint8_t* tx, std::size_t txLength,
                                 uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* user) {
    auto& bus = *static_cast<Bus*>(user);
    const auto index = bus.calls++;
    CHECK(index < 24);
    if (index >= 24) return t::Status::Error(t::Err::I2C_ERROR, "script callback limit exceeded");
    return Fake::writeRead(address, tx, txLength, rx, rxLength, timeout,
                           &bus.segments[index / 8]);
  }
};

struct Fixture {
  Bus bus{};
  Driver device{};
  t::DriverConfig config{};
  const Part& part;
  explicit Fixture(const Part& selected = PARTS[4]) : part(selected) {
    config.i2cWrite = Bus::writeCallback;
    config.i2cWriteRead = Bus::readCallback;
    config.i2cUser = &bus;
    config.variant = part.variant;
    config.address = static_cast<t::DeviceAddress>(part.address);
    CHECK(device.bind(config).ok());
  }
  void proof(uint8_t byte08 = 0x80, uint8_t byte09 = 0x14,
             uint8_t byte0A = 0x30, uint8_t byte45 = 0) {
    auto& id = bus.read(part.address, 0, 8);
    std::memcpy(id.rx, part.is322 ? ID322 : ID320, 8);
    if (!part.base) bus.read(part.address, 0xA0).rx[0] = part.revision;
    bus.read(part.address, 8).rx[0] = byte08;
    bus.read(part.address, 9).rx[0] = byte09;
    bus.read(part.address, 0x0A).rx[0] = byte0A;
    if (!part.is322) bus.read(part.address, 0x45).rx[0] = byte45;
  }
  t::Status poll(t::OperationToken token, uint64_t now, uint8_t budget = 255) {
    const auto before = bus.calls;
    bus.owner = &device;
    bus.nowUs = now * 1000;
    const auto status = device.poll(token, now, budget);
    CHECK(bus.calls - before <= 1 && device.progress().lastPollTransfers <= 1);
    CHECK(device.progress().lastPollTransfers == bus.calls - before);
    return status;
  }
  t::Status finish(t::OperationToken token, uint64_t now = 100, uint8_t budget = 255) {
    for (unsigned turn = 0; turn < 64; ++turn) {
      const auto status = poll(token, now, budget);
      if (!status.inProgress()) return status;
      const auto progress = device.progress();
      if (progress.waiting && progress.readyAtValid) now = progress.readyAtMs;
      else ++now;
    }
    CHECK(false); // Acyclic plan must finish, with no retry-until-stable behavior.
    return t::Status::Error(t::Err::I2C_ERROR, "test exceeded fixed plan");
  }
  t::OperationResult take(t::OperationToken token) {
    t::OperationResult result{};
    CHECK(device.takeResult(token, result).ok());
    CHECK(result.token.value == token.value);
    return result;
  }
  void initialize(uint8_t byte08 = 0x80, uint8_t byte09 = 0x14,
                  uint8_t byte0A = 0x30, uint8_t byte45 = 0, uint64_t now = 10) {
    proof(byte08, byte09, byte0A, byte45);
    t::OperationToken token{};
    CHECK(device.startInitialize({now, now + 1000}, token).inProgress());
    CHECK(finish(token, now).ok());
    CHECK(take(token).configuration.state == t::ConfigurationState::KNOWN);
    CHECK(bus.complete());
    bus.reset();
  }
  t::OperationToken apply(const t::DeviceProfile& profile, uint64_t now = 100,
                          uint64_t deadline = 1000) {
    t::OperationToken token{777};
    const auto before = bus.calls;
    CHECK(device.startApplyProfile(profile, {now, deadline}, token).inProgress());
    CHECK(token.valid() && token.value != 777 && bus.calls == before);
    CHECK(device.progress().kind == t::OperationKind::APPLY_PROFILE);
    return token;
  }
};

void noOpAndVerification() {
  for (const auto& part : PARTS) {
    for (const uint8_t budget : {uint8_t{1}, uint8_t{2}, uint8_t{255}}) {
      Fixture f(part);
      f.initialize();
      const auto old = f.device.configurationSnapshot();
      auto desired = old.verifiedProfile;
      f.proof();
      const auto token = f.apply(desired);
      desired.sourceCurrent = t::SourceCurrent::DEFAULT; // Admission owns a copy.
      auto image = f.device.configurationSnapshot();
      CHECK(image.hasDesiredProfile && image.desiredGeneration == 1);
      CHECK(sameProfile(image.desiredProfile, old.verifiedProfile));
      CHECK(image.state == t::ConfigurationState::KNOWN && image.verifiedGeneration == 1);
      CHECK(f.poll(token, 100, 0).inProgress() && f.bus.calls == 0);
      CHECK(f.finish(token, 100, budget).ok());
      auto result = f.take(token);
      CHECK(result.hardwareEffect == t::HardwareEffect::NONE);
      CHECK(result.configuration.state == t::ConfigurationState::KNOWN);
      CHECK(result.configuration.verifiedGeneration == 2 && f.bus.complete());
      CHECK(result.transfers == (part.base || part.is322 ? 5U : 6U));

      for (const auto method : {&Driver::startVerifyProfile, &Driver::startReconcile}) {
        f.bus.reset();
        f.proof();
        t::OperationToken next{};
        CHECK((f.device.*method)({200, 1000}, next).inProgress());
        CHECK(f.finish(next, 200, budget).ok());
        result = f.take(next);
        CHECK(result.hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
        CHECK(result.configuration.desiredGeneration == 1);
        CHECK(sameProfile(result.configuration.desiredProfile, old.verifiedProfile));
      }
    }
  }
}

void admissionAndValidation() {
  Driver unbound;
  t::OperationToken token{91};
  t::DeviceProfile candidate{};
  CHECK(unbound.startApplyProfile(candidate, {100, 1000}, token).is(t::Err::NOT_BOUND));
  CHECK(unbound.startVerifyProfile({100, 1000}, token).is(t::Err::NOT_BOUND));
  CHECK(unbound.startApplyProfile(candidate, {100, 100}, token).is(t::Err::INVALID_PARAM));
  CHECK(token.value == 91);
  for (const auto& part : PARTS) {
    Fixture f(part);
    candidate = t::DeviceProfile{};
    if (part.base) candidate.ccDebounce = t::CcDebounce::MS_133;
    CHECK(f.device.startApplyProfile(candidate, {100, 1000}, token).is(t::Err::NOT_INITIALIZED));
    CHECK(f.device.startVerifyProfile({100, 1000}, token).is(t::Err::CONFIGURATION_UNKNOWN));
    f.initialize();
    CHECK(f.device.startVerifyProfile({100, 1000}, token).is(t::Err::CONFIGURATION_UNKNOWN));
    const auto good = f.device.configurationSnapshot().verifiedProfile;
    const auto staged = f.apply(good);
    CHECK(f.device.cancel(staged).ok());
    CHECK(f.take(staged).hardwareEffect == t::HardwareEffect::NONE);
    const auto original = f.device.configurationSnapshot();
    for (unsigned field = 0; field < 5; ++field) {
      for (unsigned value = 0; value < 256; ++value) {
        candidate = good;
        switch (field) {
          case 0: candidate.portMode = static_cast<t::PortMode>(value); break;
          case 1: candidate.sourceCurrent = static_cast<t::SourceCurrent>(value); break;
          case 2: candidate.sourcePreference = static_cast<t::SourcePreference>(value); break;
          case 3: candidate.drpDutyCycle = static_cast<t::DrpDutyCycle>(value); break;
          case 4: candidate.ccDebounce = static_cast<t::CcDebounce>(value); break;
        }
        const auto validated = t::validateProfile(part.variant, candidate);
        if (validated.ok()) continue; // Valid writes are tested by literal plans below.
        token.value = 91;
        CHECK(f.device.startApplyProfile(candidate, {100, 1000}, token).code == validated.code);
        CHECK(token.value == 91 && f.bus.calls == 0);
        const auto after = f.device.configurationSnapshot();
        CHECK(after.desiredGeneration == original.desiredGeneration);
        CHECK(sameProfile(after.desiredProfile, original.desiredProfile));
        CHECK(after.state == t::ConfigurationState::KNOWN && after.verifiedGeneration == 1);
      }
    }
    // Intrinsically valid but absent boolean controls must reject the entire
    // candidate before an independently valid current field can write.
    for (unsigned control = 0; control < 3; ++control) {
      if (!((part.base && control < 2) || (part.is322 && control == 2))) continue;
      candidate = good;
      candidate.sourceCurrent = t::SourceCurrent::DEFAULT;
      if (control == 0) candidate.disableTerm = true;
      if (control == 1) candidate.disableUfpAccessory = true;
      if (control == 2) candidate.disableRdRp = true;
      CHECK(f.device.startApplyProfile(candidate, {100, 1000}, token)
                .is(t::Err::UNSUPPORTED_FEATURE));
      CHECK(f.bus.calls == 0 && f.device.configurationSnapshot().desiredGeneration == 1);
    }
    CHECK(f.device.invalidateDeviceState(t::Status::Ok()).ok());
    CHECK(f.device.startApplyProfile(good, {100, 1000}, token).is(t::Err::NOT_INITIALIZED));
    CHECK(f.bus.calls == 0 && token.value == 91);
  }
}

void literalModePlans() {
  for (const auto& part : PARTS) {
    for (const bool initiallyDisabled : {false, true}) {
      if (part.base && initiallyDisabled) continue;
      for (const bool finalDisabled : {false, true}) {
        if (part.base && finalDisabled) continue;
        Fixture f(part);
        // C2 = 152ms debounce + Try.SNK + follow PORT; preserve every
        // non-mode field. Base uses C0 (168ms + follow PORT).
        const uint8_t initial = part.base ? 0xC0 : initiallyDisabled ? 0xC3 : 0xC2;
        f.initialize(0xBF, 0xF5, initial);
        auto desired = f.device.configurationSnapshot().verifiedProfile;
        desired.portMode = t::PortMode::DFP;
        desired.disableTerm = finalDisabled;
        if (part.base) {
          f.bus.read(part.address, 9).rx[0] = 0x35;
          f.bus.write(part.address, 0x0A, 0xE0);
        } else {
          f.bus.write(part.address, 0x0A, 0xC3);
          f.bus.modeCall = f.bus.count;
          f.bus.write(part.address, 0x0A, 0xE3);
          if (!finalDisabled) {
            f.bus.enableCall = f.bus.count;
            f.bus.write(part.address, 0x0A, 0xE2);
          }
        }
        const auto writesAndPrecheck = f.bus.count;
        f.proof(0xBF, 0xF5, part.base ? 0xE0 : finalDisabled ? 0xE3 : 0xE2);
        const auto token = f.apply(desired);
        CHECK(f.finish(token).ok());
        const auto result = f.take(token);
        CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED);
        CHECK(result.transfers == writesAndPrecheck + (part.base || part.is322 ? 5U : 6U));
        CHECK(sameProfile(result.configuration.verifiedProfile, desired));
        CHECK(result.configuration.terminationState == (finalDisabled ?
            t::TerminationState::DISABLED : t::TerminationState::ENABLED));
        CHECK(result.configuration.rawConfig08 == 0xBF && result.configuration.rawConfig09 == 0xF5);
        CHECK(f.bus.complete());
      }
    }
  }
}

void ufpAndSettingsOnlyPlans() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    desired.portMode = t::PortMode::UFP;
    if (part.base) {
      f.bus.read(part.address, 9).rx[0] = 0x14;
      f.bus.write(part.address, 0x0A, 0x10);
    } else {
      // TI's distinct disable/mode/enable writes also apply to UFP.
      f.bus.write(part.address, 0x0A, 0x31);
      f.bus.write(part.address, 0x0A, 0x11);
      f.bus.write(part.address, 0x0A, 0x10);
    }
    f.proof(0x80, 0x14, 0x10);
    auto token = f.apply(desired);
    CHECK(f.finish(token).ok());
    auto result = f.take(token);
    CHECK(result.transfers == (part.base ? 7U : part.is322 ? 8U : 9U));
    CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
    CHECK(result.configuration.verifiedProfile.portMode == t::PortMode::UFP);

    // An ordinary 0A field change leaves mode unchanged and has one write,
    // with no detached precheck, role procedure or hardware wait. Equal fresh
    // owner ticks make any accidental wait fail this finite completion bound.
    const unsigned changes = part.base ? 1U : 2U;
    for (unsigned change = 0; change < changes; ++change) {
      f.bus.reset();
      uint8_t expected = 0x10;
      if (part.base) {
        desired.ccDebounce = t::CcDebounce::MS_116;
        expected = 0x50;
      } else {
        desired.disableTerm = change == 0;
        expected = desired.disableTerm ? 0x11 : 0x10;
      }
      f.bus.write(part.address, 0x0A, expected);
      f.proof(0x80, 0x14, expected);
      const uint64_t now = 300 + change;
      token = f.apply(desired, now);
      const std::size_t callbacks = part.base || part.is322 ? 6U : 7U;
      t::Status status{};
      for (std::size_t turn = 0; turn <= callbacks; ++turn) {
        status = f.poll(token, now);
        CHECK(!f.device.progress().waiting && !f.device.progress().readyAtValid);
        if (!status.inProgress()) break;
      }
      CHECK(status.ok());
      result = f.take(token);
      CHECK(result.transfers == callbacks && f.bus.complete());
      CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED);
      CHECK(sameProfile(result.configuration.verifiedProfile, desired));
      CHECK(result.configuration.terminationState == (desired.disableTerm ?
          t::TerminationState::DISABLED : t::TerminationState::ENABLED));
      CHECK(!f.device.diagnostics().modeTransitionUnresolved);
    }
  }
}

void baseAttachmentAndWriteOrder() {
  for (const auto& part : PARTS) {
    if (part.base) {
      for (const uint8_t attachment : {uint8_t{0x40}, uint8_t{0x80}, uint8_t{0xC0}}) {
        Fixture f(part);
        f.initialize();
        auto desired = f.device.configurationSnapshot().verifiedProfile;
        desired.portMode = t::PortMode::DFP;
        desired.sourceCurrent = t::SourceCurrent::DEFAULT;
        desired.disableRdRp = true;
        f.bus.read(part.address, 9).rx[0] = static_cast<uint8_t>(attachment | 0x14);
        const auto token = f.apply(desired);
        CHECK(f.finish(token).is(t::Err::CONFIGURATION_UNKNOWN));
        const auto result = f.take(token);
        CHECK(result.hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
        CHECK(result.configuration.state == t::ConfigurationState::KNOWN);
        CHECK(result.configuration.desiredProfile.portMode == t::PortMode::DFP);
      }
      Fixture current(part);
      current.initialize(0x80, 0xD4);
      auto desired = current.device.configurationSnapshot().verifiedProfile;
      desired.sourceCurrent = t::SourceCurrent::MA_1500;
      current.bus.write(part.address, 8, 0x40);
      current.proof(0x40, 0xD4);
      const auto token = current.apply(desired);
      CHECK(current.finish(token).ok());
      CHECK(current.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED);
      CHECK(current.bus.complete()); // No attached precheck for an unchanged mode.
    }
    if (part.is322) continue;
    for (const bool enable45 : {false, true}) {
      Fixture f(part);
      f.initialize(0xBF, 0xF5, 0xC0, enable45 ? 4 : 0);
      auto desired = f.device.configurationSnapshot().verifiedProfile;
      desired.portMode = t::PortMode::DFP;
      desired.sourceCurrent = t::SourceCurrent::MA_1500;
      desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_60;
      desired.ccDebounce = part.base ? t::CcDebounce::MS_116 : t::CcDebounce::MS_118;
      desired.disableRdRp = !enable45;
      if (!part.base) {
        desired.sourcePreference = t::SourcePreference::TRY_SRC;
        desired.disableUfpAccessory = false;
      }
      if (part.base) {
        f.bus.read(part.address, 9).rx[0] = 0x35;
        f.bus.write(part.address, 0x0A, 0x60);
      }
      if (!enable45) f.bus.write(part.address, 0x45, 4);
      f.bus.write(part.address, 8, 0x40);
      f.bus.write(part.address, 9, 6); // No W1C/RO or staged 0A fields.
      if (!part.base) {
        f.bus.write(part.address, 0x0A, 0xC1);
        f.bus.write(part.address, 0x0A, 0x67);
        f.bus.write(part.address, 0x0A, 0x66);
      }
      if (enable45) f.bus.write(part.address, 0x45, 0);
      f.proof(0x7F, 0xF6, part.base ? 0x60 : 0x66, enable45 ? 0 : 4);
      const auto token = f.apply(desired);
      CHECK(f.finish(token).ok());
      const auto result = f.take(token);
      CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED);
      CHECK(sameProfile(result.configuration.verifiedProfile, desired));
      CHECK(result.configuration.terminationState == (enable45 ?
          t::TerminationState::ENABLED : t::TerminationState::DISABLED));
      CHECK(f.bus.complete());
    }
  }
}

void gated322Changes() {
  Fixture f(PARTS[6]);
  f.initialize();
  const auto good = f.device.configurationSnapshot().verifiedProfile;
  t::OperationToken rejected{81};
  for (unsigned field = 0; field < 3; ++field) {
    auto desired = good;
    if (field == 0) desired.sourceCurrent = t::SourceCurrent::DEFAULT;
    if (field == 1) desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_60;
    if (field == 2) desired.disableUfpAccessory = true;
    CHECK(f.device.startApplyProfile(desired, {100, 1000}, rejected)
              .is(t::Err::UNSUPPORTED_FEATURE));
    CHECK(rejected.value == 81 && f.bus.calls == 0);
    CHECK(!f.device.configurationSnapshot().hasDesiredProfile);
  }
  // DRP00 is semantically the same mode: preserve the actual representation.
  Fixture equivalent(PARTS[6]);
  equivalent.initialize(0x80, 0x14, 0x00);
  auto desired = equivalent.device.configurationSnapshot().verifiedProfile;
  desired.sourcePreference = t::SourcePreference::TRY_SRC;
  equivalent.bus.write(0x47, 0x0A, 0x06);
  equivalent.proof(0x80, 0x1C, 0x06); // Raw VCONN fault does not become an ACK.
  auto token = equivalent.apply(desired);
  CHECK(equivalent.finish(token).ok());
  CHECK(equivalent.take(token).configuration.rawConfig09 == 0x1C && equivalent.bus.complete());
  Fixture dfp(PARTS[6]);
  dfp.initialize(0x80, 0x14, 0x20);
  desired = dfp.device.configurationSnapshot().verifiedProfile;
  desired.portMode = t::PortMode::DRP;
  CHECK(dfp.device.startApplyProfile(desired, {100, 1000}, rejected)
            .is(t::Err::UNSUPPORTED_FEATURE));
  CHECK(dfp.bus.calls == 0 && rejected.value == 81);
}

void physicalModeWait() {
  // Independent microseconds measure the physical interval; production receives
  // only floor(us/1000). Callback completion never supplies a public timestamp.
  for (const uint64_t completionUs : {uint64_t{108100}, uint64_t{108900}}) {
    for (const uint64_t freshUs : {uint64_t{108950}, uint64_t{110200}}) {
      Fixture f;
      f.initialize(0x80, 0x14, 0x00);
      auto desired = f.device.configurationSnapshot().verifiedProfile;
      desired.portMode = t::PortMode::DRP;
      f.bus.write(0x47, 0x0A, 1);
      f.bus.write(0x47, 0x0A, 0x31);
      f.bus.write(0x47, 0x0A, 0x30);
      f.proof();
      f.bus.modeCall = 1;
      f.bus.enableCall = 2;
      f.bus.modeCompletionUs = completionUs;
      const auto token = f.apply(desired);
      CHECK(f.poll(token, 100).inProgress() && f.bus.calls == 1);
      CHECK(f.poll(token, 101).inProgress() && f.bus.calls == 2);
      CHECK(f.bus.nowUs == completionUs);
      f.bus.nowUs = freshUs;
      CHECK(f.device.poll(token, freshUs / 1000, 0).inProgress());
      const uint64_t ready = freshUs / 1000 + 6;
      CHECK(f.device.progress().waiting && f.device.progress().readyAtValid);
      CHECK(f.device.progress().readyAtMs == ready);
      CHECK(f.poll(token, 113).inProgress() && f.bus.calls == 2);
      for (uint64_t now = 113; now < ready; ++now) {
        CHECK(f.poll(token, now).inProgress());
        CHECK(f.bus.calls == 2 && f.bus.enableUs == 0);
      }
      // The exact boundary's elapsed physical time is independently >=5ms.
      CHECK(f.poll(token, ready).inProgress() && f.bus.calls == 3);
      CHECK(f.bus.enableUs - completionUs >= 5000);
      CHECK(f.bus.enableUs == ready * 1000);
      CHECK(f.finish(token, ready).ok());
      CHECK(f.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
    }
  }
}

void prewriteAndEveryBoundary() {
  for (const auto& part : PARTS) {
    for (unsigned interruption = 0; interruption < 3; ++interruption) {
      Fixture f(part);
      f.initialize();
      const auto old = f.device.configurationSnapshot();
      auto desired = old.verifiedProfile;
      desired.portMode = t::PortMode::DFP;
      const auto token = f.apply(desired, 100, 200);
      CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::KNOWN);
      if (interruption == 0) CHECK(f.device.cancel(token).ok());
      if (interruption == 1) CHECK(f.poll(token, 200, 0).is(t::Err::TIMEOUT));
      if (interruption == 2) CHECK(f.device.end().ok());
      const auto result = f.take(token);
      CHECK(result.hardwareEffect == t::HardwareEffect::NONE && f.bus.calls == 0);
      CHECK(result.configuration.hasDesiredProfile && result.configuration.desiredGeneration == 1);
      CHECK(sameProfile(result.configuration.desiredProfile, desired));
      CHECK(result.configuration.verifiedGeneration == old.verifiedGeneration);
      CHECK(sameProfile(result.configuration.verifiedProfile, old.verifiedProfile));
      if (interruption != 2) CHECK(result.configuration.state == t::ConfigurationState::KNOWN);
    }
  }
  // A representative complete L/A plan covers every write and F read, before
  // and after callbacks, including the final fresh-time publication boundary.
  for (unsigned action = 0; action < 4; ++action) {
    for (std::size_t boundary = 0; boundary <= 9; ++boundary) {
      Fixture f;
      f.initialize(0x80, 0x14, 0x00);
      auto desired = f.device.configurationSnapshot().verifiedProfile;
      desired.portMode = t::PortMode::DRP;
      f.bus.write(0x47, 0x0A, 1);
      f.bus.write(0x47, 0x0A, 0x31);
      f.bus.write(0x47, 0x0A, 0x30);
      f.proof();
      const auto token = f.apply(desired, 100, 500);
      uint64_t now = 100;
      for (unsigned turn = 0; f.bus.calls < boundary && turn < 40; ++turn) {
        CHECK(f.poll(token, now).inProgress());
        const auto progress = f.device.progress();
        now = progress.waiting && progress.readyAtValid ? progress.readyAtMs : now + 1;
      }
      CHECK(f.bus.calls == boundary);
      const auto count = f.bus.calls;
      if (action == 0) CHECK(f.device.cancel(token).ok());
      if (action == 1) CHECK(f.poll(token, 500, 255).is(t::Err::TIMEOUT));
      if (action == 2) CHECK(f.device.end().ok());
      if (action == 3) CHECK(f.device.invalidateDeviceState(t::Status::Ok()).ok());
      const auto result = f.take(token);
      CHECK(result.hardwareEffect == (boundary == 0 ? t::HardwareEffect::NONE :
                                                               t::HardwareEffect::PARTIAL));
      CHECK(result.configuration.desiredGeneration == 1);
      CHECK(sameProfile(result.configuration.desiredProfile, desired));
      CHECK(result.configuration.verifiedGeneration == 1);
      CHECK(f.bus.calls == count && f.bus.clean());
      if (boundary != 0) {
        CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
        CHECK(result.port.invalidated);
      }
      CHECK(result.completedTimeValid == (action == 1));
      if (action == 2) {
        CHECK(f.device.bind(f.config).ok());
        CHECK(f.bus.calls == count); // Rebinding never replays desired or cleanup.
      }
    }
  }
}

void faultsAtEveryWriteAndProofRead() {
  constexpr t::Err ERRORS[] = {t::Err::I2C_ERROR, t::Err::I2C_NACK_ADDR,
      t::Err::I2C_NACK_DATA, t::Err::I2C_TIMEOUT, t::Err::I2C_BUS, t::Err::I2C_BUSY};
  // Each mutable register and all three role writes appear in one independent
  // literal script. Both no acceptance and acceptance-then-error are injected.
  for (const bool enable45 : {false, true}) {
    for (std::size_t failed = 0; failed < 12; ++failed) {
      for (const bool applied : {false, true}) {
        for (const auto error : ERRORS) {
          Fixture f;
          f.initialize(0x80, 0x14, 0x00, enable45 ? 4 : 0);
          auto desired = f.device.configurationSnapshot().verifiedProfile;
          desired.portMode = t::PortMode::DRP;
          desired.sourceCurrent = t::SourceCurrent::MA_1500;
          desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_60;
          desired.disableRdRp = !enable45;
          if (!enable45) f.bus.write(0x47, 0x45, 4);
          f.bus.write(0x47, 8, 0x40);
          f.bus.write(0x47, 9, 6);
          f.bus.write(0x47, 0x0A, 1);
          f.bus.write(0x47, 0x0A, 0x31);
          f.bus.write(0x47, 0x0A, 0x30);
          if (enable45) f.bus.write(0x47, 0x45, 0);
          f.proof(0x40, 0x16, 0x30, enable45 ? 0 : 4);
          const auto firstProof = std::size_t{6};
          auto& step = f.bus.at(failed);
          step.status = {error, -123456789, "injected original write/read error"};
          step.applyWrite = applied;
          if (step.kind == Fake::Kind::READ) step.fillLength = applied ? step.rxLength : 0;
          const auto statsBefore = f.device.transportStats();
          const auto token = f.apply(desired);
          CHECK(f.finish(token).code == error);
          // The callback's original error is terminal even when the next owner
          // sample is beyond the immutable deadline; no cleanup may replace it.
          CHECK(f.device.poll(token, 1000, 255).code == error);
          const auto result = f.take(token);
          CHECK(result.status.code == error && result.status.detail == -123456789);
          CHECK(result.status.msg == step.status.msg);
          if (step.kind == Fake::Kind::WRITE) {
            CHECK(f.bus.segments[failed / 8].registers[step.tx[0]] ==
                  (applied ? step.tx[1] : 0));
          }
          CHECK(result.transfers == failed + 1 && f.bus.calls == failed + 1);
          CHECK(f.device.transportStats().totalAttempts - statsBefore.totalAttempts == failed + 1);
          CHECK(f.device.transportStats().totalFailures - statsBefore.totalFailures == 1);
          CHECK(result.hardwareEffect == (failed < firstProof ?
              t::HardwareEffect::INDETERMINATE : t::HardwareEffect::PARTIAL));
          CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
          CHECK(result.configuration.desiredGeneration == 1);
          CHECK(result.configuration.verifiedGeneration == 1 && result.port.invalidated);
          CHECK(sameProfile(result.configuration.desiredProfile, desired) && f.bus.clean());
          const auto frozen = result;
          CHECK(f.device.end().ok());
          CHECK(f.bus.calls == failed + 1 && frozen.status.detail == -123456789);
        }
      }
    }
  }
}

void tusb322ModeWriteFailures() {
  // SLLSEO5D 6.3.2.1-2 pp10-11 and Table6-10 p20: preserve debounce/
  // preference, set disable, select DFP/UFP, then clear disable. Unlike L/A,
  // 322I has no specified 5ms interval and has no register45 proof read.
  for (const bool dfp : {false, true}) {
    const uint8_t commands[] = {0x31, static_cast<uint8_t>(dfp ? 0x21 : 0x11),
        static_cast<uint8_t>(dfp ? 0x20 : 0x10)};
    for (std::size_t failed = 0; failed < 3; ++failed) {
      for (const bool applied : {false, true}) {
        Fixture f(PARTS[6]);
        f.initialize();
        const auto old = f.device.configurationSnapshot();
        auto desired = old.verifiedProfile;
        desired.portMode = dfp ? t::PortMode::DFP : t::PortMode::UFP;
        f.bus.segments[0].registers[0x0A] = 0x30;
        for (const auto command : commands)
          f.bus.write(0x47, 0x0A, command).applyWrite = true;
        f.proof(0x80, 0x14, commands[2]);
        auto& fault = f.bus.at(failed);
        fault.status = {t::Err::I2C_TIMEOUT, -1987654321, "322I mode acceptance unknown"};
        fault.applyWrite = applied;
        const auto attempts = f.device.transportStats().totalAttempts;
        const auto token = f.apply(desired);
        // Equal ticks expose any accidental borrowed L/A settling wait.
        for (std::size_t index = 0; index < failed; ++index)
          CHECK(f.poll(token, 100).inProgress());
        CHECK(f.poll(token, 100).is(t::Err::I2C_TIMEOUT));
        const auto expected = applied ? commands[failed] : failed == 0 ? 0x30 : commands[failed - 1];
        CHECK(f.bus.segments[0].registers[0x0A] == expected);
        CHECK(f.bus.calls == failed + 1 && f.bus.clean());
        CHECK(f.device.transportStats().totalAttempts == attempts + failed + 1);
        CHECK(f.device.diagnostics().modeTransitionUnresolved);
        CHECK(f.device.diagnostics().configurationWriteUnresolved);
        CHECK(f.device.poll(token, 1000, 255).is(t::Err::I2C_TIMEOUT));
        CHECK(f.device.cancel(token).ok());
        const auto result = f.take(token);
        CHECK(result.status.code == fault.status.code && result.status.detail == fault.status.detail);
        CHECK(result.status.msg == fault.status.msg && !result.completedTimeValid);
        CHECK(result.transfers == failed + 1 && result.hardwareEffect == t::HardwareEffect::INDETERMINATE);
        CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
        CHECK(result.configuration.terminationState == t::TerminationState::UNKNOWN);
        CHECK(result.configuration.verifiedGeneration == old.verifiedGeneration);
        CHECK(sameProfile(result.configuration.verifiedProfile, old.verifiedProfile));
        CHECK(sameProfile(result.configuration.desiredProfile, desired));
        CHECK(result.port.invalidated && f.bus.calls == failed + 1);

        // A later unrelated successful byte is neither transition proof nor a
        // reason to retry any of the three writes or replace the primary error.
        f.bus.reset();
        f.bus.read(0x47, 0).rx[0] = 0x32;
        uint8_t idByte = 0;
        CHECK(f.device.readRegisterSync(0, idByte).ok() && idByte == 0x32);
        CHECK(f.bus.complete() && f.bus.calls == 1);
        CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
        CHECK(f.device.diagnostics().modeTransitionUnresolved &&
              f.device.diagnostics().configurationWriteUnresolved);
        CHECK(f.device.diagnostics().lastFailedTransportStatus.detail == -1987654321);
        CHECK(f.device.diagnostics().lastOperationStatus.code == result.status.code &&
              f.device.diagnostics().lastOperationStatus.detail == result.status.detail);
        CHECK(f.device.end().ok() && f.device.bind(f.config).ok());
        CHECK(f.bus.calls == 1 && f.device.diagnostics().modeTransitionUnresolved);
      }
    }
  }
}

void tusb322ModeInterruptionBoundaries() {
  // Three literal writes followed by I/revision/08/09/0A: each callback and
  // the final fresh publication turn is independently interruptible. Generic
  // error normalization and L/A wait interruption already have their own cases.
  constexpr std::size_t CALLBACKS = 8;
  for (const bool dfp : {false, true}) {
    const uint8_t mode = dfp ? 0x20 : 0x10;
    for (std::size_t boundary = 0; boundary <= CALLBACKS; ++boundary) {
      for (const bool deadline : {false, true}) {
        Fixture f(PARTS[6]);
        f.initialize();
        const auto old = f.device.configurationSnapshot();
        auto desired = old.verifiedProfile;
        desired.portMode = dfp ? t::PortMode::DFP : t::PortMode::UFP;
        f.bus.write(0x47, 0x0A, 0x31);
        f.bus.write(0x47, 0x0A, static_cast<uint8_t>(mode | 0x01));
        f.bus.write(0x47, 0x0A, mode);
        f.proof(0x80, 0x14, mode);
        const auto token = f.apply(desired, 100, 500);
        for (std::size_t index = 0; index < boundary; ++index) {
          CHECK(f.poll(token, 100).inProgress());
          CHECK(!f.device.progress().waiting && !f.device.progress().readyAtValid);
        }
        CHECK(f.bus.calls == boundary && f.bus.clean());
        CHECK(f.device.configurationSnapshot().verifiedGeneration == old.verifiedGeneration);
        if (deadline) CHECK(f.poll(token, 500, 0).is(t::Err::TIMEOUT));
        else CHECK(f.device.cancel(token).ok());
        t::OperationResult retained{};
        CHECK(f.device.resultSnapshot(token, retained).ok());
        CHECK(retained.status.is(deadline ? t::Err::TIMEOUT : t::Err::CANCELLED));
        CHECK(retained.completedTimeValid == deadline);
        CHECK(retained.transfers == boundary && retained.token.value == token.value);
        CHECK(retained.hardwareEffect == (boundary == 0 ? t::HardwareEffect::NONE
                                                         : t::HardwareEffect::PARTIAL));
        CHECK(retained.configuration.state == (boundary == 0 ? t::ConfigurationState::KNOWN
                                                             : t::ConfigurationState::UNKNOWN));
        CHECK(retained.configuration.verifiedGeneration == old.verifiedGeneration);
        CHECK(sameProfile(retained.configuration.verifiedProfile, old.verifiedProfile));
        CHECK(sameProfile(retained.configuration.desiredProfile, desired));
        CHECK(f.device.diagnostics().modeTransitionUnresolved == (boundary != 0));
        CHECK(f.device.diagnostics().configurationWriteUnresolved == (boundary != 0));
        CHECK(f.device.poll(token, 501).code == retained.status.code);
        CHECK(f.device.cancel(token).ok());
        CHECK(f.take(token).status.code == retained.status.code);
        CHECK(f.device.end().ok() && f.device.bind(f.config).ok());
        CHECK(f.bus.calls == boundary && f.bus.clean());
        CHECK(f.device.diagnostics().modeTransitionUnresolved == (boundary != 0));
      }
    }

    Fixture completed(PARTS[6]);
    completed.initialize();
    auto desired = completed.device.configurationSnapshot().verifiedProfile;
    desired.portMode = dfp ? t::PortMode::DFP : t::PortMode::UFP;
    completed.bus.write(0x47, 0x0A, 0x31);
    completed.bus.write(0x47, 0x0A, static_cast<uint8_t>(mode | 0x01));
    completed.bus.write(0x47, 0x0A, mode);
    completed.proof(0x80, 0x14, mode);
    const auto token = completed.apply(desired, 100, 500);
    for (std::size_t index = 0; index < CALLBACKS; ++index)
      CHECK(completed.poll(token, 100).inProgress());
    CHECK(completed.device.configurationSnapshot().verifiedGeneration == 1);
    CHECK(completed.poll(token, 499, 0).ok());
    CHECK(completed.device.cancel(token).ok()); // Terminal success remains frozen.
    const auto result = completed.take(token);
    CHECK(result.status.ok() && result.completedUptimeMs == 499 && result.completedTimeValid);
    CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED && result.transfers == CALLBACKS);
    CHECK(result.configuration.state == t::ConfigurationState::KNOWN);
    CHECK(result.configuration.verifiedGeneration == 2);
    CHECK(sameProfile(result.configuration.verifiedProfile, desired));
    CHECK(!completed.device.diagnostics().modeTransitionUnresolved);
    CHECK(completed.bus.complete() && completed.bus.calls == CALLBACKS);
  }
}

void exactWaitGatesAndLateFinalTime() {
  for (const auto& part : PARTS) {
    if (part.base) continue;
    for (const bool finalDisabled : {false, true}) {
      Fixture f(part);
      f.initialize(0x80, 0x14, 0);
      auto desired = f.device.configurationSnapshot().verifiedProfile;
      desired.portMode = t::PortMode::DFP;
      desired.disableTerm = finalDisabled;
      f.bus.write(part.address, 0x0A, 1);
      f.bus.write(part.address, 0x0A, 0x21);
      if (!finalDisabled) f.bus.write(part.address, 0x0A, 0x20);
      f.proof(0x80, 0x14, finalDisabled ? 0x21 : 0x20);
      const auto token = f.apply(desired);
      CHECK(f.poll(token, 100).inProgress() && f.bus.calls == 1);
      CHECK(f.poll(token, 100).inProgress() && f.bus.calls == 2);
      CHECK(f.poll(token, 108, 0).inProgress() && f.bus.calls == 2);
      CHECK(f.device.progress().waiting == !part.is322);
      if (part.is322) {
        // Fresh completion at 108 immediately permits the next action, even
        // with an equal floor tick: the 322 source specifies no 5ms wait.
        CHECK(f.poll(token, 108).inProgress() && f.bus.calls == 3);
      } else {
        CHECK(f.device.progress().readyAtMs == 114);
        CHECK(f.poll(token, 113).inProgress() && f.bus.calls == 2);
        CHECK(f.poll(token, 114).inProgress() && f.bus.calls == 3);
      }
      CHECK(f.finish(token, part.is322 ? 108 : 114).ok());
      CHECK(f.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
    }
  }
  for (unsigned action = 0; action < 3; ++action) {
    Fixture f;
    f.initialize(0x80, 0x14, 0);
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    desired.portMode = t::PortMode::DRP;
    f.bus.write(0x47, 0x0A, 1).timeoutMs = 14;
    f.bus.write(0x47, 0x0A, 0x31).timeoutMs = 13;
    const auto token = f.apply(desired, 100, 114);
    CHECK(f.poll(token, 100).inProgress());
    CHECK(f.poll(token, 101).inProgress());
    CHECK(f.poll(token, 108, 0).inProgress());
    CHECK(f.device.progress().waiting && f.device.progress().readyAtMs == 114);
    CHECK(f.poll(token, 113, 255).inProgress() && f.bus.calls == 2);
    if (action == 0) CHECK(f.poll(token, 114).is(t::Err::TIMEOUT));
    if (action == 1) CHECK(f.device.cancel(token).ok());
    if (action == 2) CHECK(f.device.end().ok());
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::PARTIAL && f.bus.complete());
    CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
    CHECK(f.device.diagnostics().modeTransitionUnresolved);
  }
  // Final successful readback cannot publish success or trust until fresh time;
  // callback time and the immutable deadline remain distinct evidence.
  for (const auto method : {&Driver::startVerifyProfile, &Driver::startReconcile}) {
    Fixture f;
    f.initialize();
    const auto good = f.device.configurationSnapshot().verifiedProfile;
    auto token = f.apply(good);
    CHECK(f.device.cancel(token).ok());
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::NONE);
    f.proof();
    CHECK((f.device.*method)({100, 130}, token).inProgress());
    for (uint64_t now = 100; f.bus.calls < 6; ++now) CHECK(f.poll(token, now).inProgress());
    CHECK(f.device.configurationSnapshot().verifiedGeneration == 1);
    CHECK(f.poll(token, 130, 0).is(t::Err::TIMEOUT));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
    CHECK(result.configuration.verifiedGeneration == 1);
  }
}

void completeApplyMismatchAndPrecheckContradiction() {
  for (const bool write : {false, true}) {
    Fixture f;
    f.initialize();
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    if (write) {
      desired.sourceCurrent = t::SourceCurrent::MA_1500;
      f.bus.write(0x47, 8, 0x40);
      f.proof(); // Complete actual proof contradicts the write request.
    } else f.proof(0x40); // A no-op request can discover external changes.
    const auto token = f.apply(desired);
    CHECK(f.finish(token).is(t::Err::CONFIGURATION_MISMATCH));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == (write ? t::HardwareEffect::PARTIAL : t::HardwareEffect::NONE));
    CHECK(result.configuration.state == t::ConfigurationState::KNOWN && f.bus.complete());
    CHECK(result.configuration.verifiedGeneration == 2 && result.configuration.mismatchValid);
    CHECK(sameProfile(result.configuration.desiredProfile, desired));
  }
  for (const auto& part : {PARTS[0], PARTS[1]}) {
    Fixture f(part);
    f.initialize();
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    desired.portMode = t::PortMode::DFP;
    desired.sourceCurrent = t::SourceCurrent::DEFAULT;
    f.bus.read(part.address, 9).rx[0] = 0x16; // Detached but persistent duty changed.
    const auto token = f.apply(desired);
    CHECK(f.finish(token).is(t::Err::CONFIGURATION_UNKNOWN));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
    CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
  }
}

void applyLateReadbackAndSlotOwnership() {
  for (const bool write : {false, true}) {
    Fixture f;
    f.initialize();
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    if (write) {
      desired.sourceCurrent = t::SourceCurrent::MA_1500;
      f.bus.write(0x47, 8, 0x40);
    }
    f.proof(write ? 0x40 : 0x80);
    auto timing = t::OperationTiming{100, 130};
    t::OperationToken token{};
    CHECK(f.device.startApplyProfile(desired, timing, token).inProgress());
    timing.deadlineMs = 101;
    CHECK(f.device.progress().deadlineMs == 130);
    t::OperationToken rejected{92};
    CHECK(f.device.startApplyProfile(desired, {100, 130}, rejected).is(t::Err::BUSY));
    CHECK(f.device.startVerifyProfile({100, 130}, rejected).is(t::Err::BUSY));
    CHECK(rejected.value == 92 && f.bus.calls == 0);
    for (uint64_t now = 100; f.bus.calls < f.bus.count; ++now) CHECK(f.poll(token, now).inProgress());
    CHECK(f.device.configurationSnapshot().verifiedGeneration == 1);
    CHECK(f.poll(token, 130, 0).is(t::Err::TIMEOUT));
    CHECK(f.device.startApplyProfile(desired, {130, 200}, rejected).is(t::Err::RESULT_PENDING));
    CHECK(f.device.startVerifyProfile({130, 200}, rejected).is(t::Err::RESULT_PENDING));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == (write ? t::HardwareEffect::PARTIAL : t::HardwareEffect::NONE));
    CHECK(result.configuration.verifiedGeneration == 1 && f.bus.complete());
    CHECK(result.configuration.desiredGeneration == 1);
    CHECK(result.completedTimeValid && result.completedUptimeMs == 130);
    CHECK(result.configuration.state == (write ? t::ConfigurationState::UNKNOWN :
                                                               t::ConfigurationState::KNOWN));
  }
}

void failedAttemptBudgetAndUnknownActual() {
  for (const uint8_t budget : {uint8_t{1}, uint8_t{2}, uint8_t{255}}) {
    Fixture f;
    f.initialize();
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    desired.sourceCurrent = t::SourceCurrent::MA_1500;
    auto& write = f.bus.write(0x47, 8, 0x40);
    write.status = {t::Err::I2C_BUSY, -7654321, "aggregate lock failure"};
    const auto token = f.apply(desired);
    const auto before = f.device.transportStats();
    CHECK(f.poll(token, 100, 0).inProgress());
    CHECK(f.device.transportStats().totalAttempts == before.totalAttempts && f.bus.calls == 0);
    CHECK(f.poll(token, 100, budget).is(t::Err::I2C_BUSY));
    const auto result = f.take(token);
    CHECK(result.status.detail == -7654321 && result.transfers == 1);
    CHECK(result.hardwareEffect == t::HardwareEffect::INDETERMINATE && f.bus.complete());
    CHECK(f.device.transportStats().totalAttempts == before.totalAttempts + 1);
    CHECK(f.device.transportStats().totalFailures == before.totalFailures + 1);
  }
  for (const auto& part : PARTS) {
    Fixture f(part);
    // Matching identity plus complete raw P is initialized, even when a reserved
    // advertisement leaves actual typed fields unknown. Apply cannot guess it.
    f.proof(0xC0);
    t::OperationToken token{};
    CHECK(f.device.startInitialize({10, 1000}, token).inProgress());
    CHECK(f.finish(token, 10).is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
    CHECK(f.device.diagnostics().initialized);
    f.bus.reset();
    t::DeviceProfile desired{};
    if (part.base) desired.ccDebounce = t::CcDebounce::MS_133;
    token.value = 92;
    CHECK(f.device.startApplyProfile(desired, {100, 1000}, token)
              .is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(token.value == 92 && f.bus.calls == 0);
    CHECK(!f.device.configurationSnapshot().hasDesiredProfile);
  }
}

void internal45QualificationAndStagedControls() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    Fixture f(part);
    f.initialize(0x80, 0x14, 0x30, 3);
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    desired.disableRdRp = true;
    t::OperationToken rejected{92};
    CHECK(f.device.startApplyProfile(desired, {100, 1000}, rejected)
              .is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(f.bus.calls == 0 && rejected.value == 92);
    CHECK(!f.device.configurationSnapshot().hasDesiredProfile);
    // Qualification for the TI-internal 45 bits gates a requested 45 change,
    // not a permitted independent advertisement change.
    desired.disableRdRp = false;
    desired.sourceCurrent = t::SourceCurrent::MA_1500;
    f.bus.write(part.address, 8, 0x40);
    f.proof(0x40, 0x14, 0x30, 3);
    auto token = f.apply(desired);
    CHECK(f.finish(token).ok());
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());

    f.bus.reset();
    const auto actual = f.device.configurationSnapshot().verifiedProfile;
    desired = actual;
    desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_60;
    desired.portMode = t::PortMode::UFP;
    if (!part.base) desired.sourcePreference = t::SourcePreference::TRY_SRC;
    token = f.apply(desired, 200);
    CHECK(f.device.cancel(token).ok());
    const auto staged = f.take(token);
    CHECK(staged.hardwareEffect == t::HardwareEffect::NONE);
    CHECK(sameProfile(staged.configuration.verifiedProfile, actual));
    // A subsequent independent current change uses actual09/0A. Previously
    // staged duty/mode/preference cannot sneak into any composed write or F.
    desired = actual;
    desired.sourceCurrent = t::SourceCurrent::DEFAULT;
    f.bus.write(part.address, 8, 0);
    f.proof(0, 0x14, 0x30, 3);
    token = f.apply(desired, 300);
    CHECK(f.finish(token, 300).ok());
    const auto result = f.take(token);
    CHECK(sameProfile(result.configuration.verifiedProfile, desired) && f.bus.complete());
    CHECK(result.configuration.desiredGeneration == 3);
  }
}

void wrappedModeWait() {
  constexpr uint64_t LAST = std::numeric_limits<uint64_t>::max();
  Fixture f;
  f.initialize(0x80, 0x14, 0, 0, LAST - 30);
  auto desired = f.device.configurationSnapshot().verifiedProfile;
  desired.portMode = t::PortMode::DRP;
  f.bus.write(0x47, 0x0A, 1);
  f.bus.write(0x47, 0x0A, 0x31);
  f.bus.write(0x47, 0x0A, 0x30);
  f.proof();
  const auto token = f.apply(desired, LAST - 10, 40);
  CHECK(f.poll(token, LAST - 10).inProgress());
  CHECK(f.poll(token, LAST - 9).inProgress());
  CHECK(f.poll(token, LAST - 2, 0).inProgress());
  CHECK(f.device.progress().waiting && f.device.progress().readyAtMs == 3);
  CHECK(f.poll(token, 2).inProgress() && f.bus.calls == 2);
  CHECK(f.poll(token, 3).inProgress() && f.bus.calls == 3);
  CHECK(f.finish(token, 3).ok());
  CHECK(f.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
}

void interruptedBaseModeEvidence() {
  for (const auto& part : {PARTS[0], PARTS[1]}) {
    for (const bool callbackFailed : {false, true}) {
      for (const bool applied : {false, true}) {
        Fixture f(part);
        f.initialize();
        auto desired = f.device.configurationSnapshot().verifiedProfile;
        desired.portMode = t::PortMode::DFP;
        f.bus.read(part.address, 9).rx[0] = 0x14;
        auto& mode = f.bus.write(part.address, 0x0A, 0x20);
        mode.applyWrite = applied;
        if (callbackFailed)
          mode.status = {t::Err::I2C_TIMEOUT, -89123, "base mode acceptance unknown"};
        auto token = f.apply(desired);
        CHECK(f.poll(token, 100).inProgress());
        CHECK(!f.device.diagnostics().modeTransitionUnresolved);
        const auto written = f.poll(token, 101);
        CHECK(written.code == (callbackFailed ? t::Err::I2C_TIMEOUT : t::Err::IN_PROGRESS));
        // Original parts have no added settling interval, but an attempted role
        // change still leaves transition proof unresolved until complete F.
        CHECK(f.device.diagnostics().modeTransitionUnresolved);
        if (!callbackFailed) CHECK(f.device.cancel(token).ok());
        const auto interrupted = f.take(token);
        CHECK(interrupted.hardwareEffect == (callbackFailed ?
            t::HardwareEffect::INDETERMINATE : t::HardwareEffect::PARTIAL));
        CHECK(interrupted.configuration.state == t::ConfigurationState::UNKNOWN);
        CHECK(f.device.diagnostics().modeTransitionUnresolved && f.bus.complete());
        CHECK(sameProfile(interrupted.configuration.desiredProfile, desired));

        f.bus.reset();
        f.proof(0x80, 0x14, applied ? 0x20 : 0x30);
        CHECK(f.device.startReconcile({200, 1000}, token).inProgress());
        CHECK(f.poll(token, 200, 0).inProgress() && f.bus.calls == 0);
        CHECK(!f.device.progress().waiting && !f.device.progress().readyAtValid);
        CHECK(f.device.diagnostics().modeTransitionUnresolved);
        // Same fresh integer tick permits I immediately: no inherited L/A wait.
        CHECK(f.poll(token, 200, 255).inProgress() && f.bus.calls == 1);
        CHECK(f.device.diagnostics().modeTransitionUnresolved);
        CHECK(f.finish(token, 200).code ==
            (applied ? t::Err::OK : t::Err::CONFIGURATION_MISMATCH));
        const auto actual = f.take(token);
        CHECK(actual.hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
        CHECK(actual.configuration.state == t::ConfigurationState::KNOWN);
        CHECK(!f.device.diagnostics().modeTransitionUnresolved);
        CHECK(actual.configuration.verifiedProfile.portMode ==
            (applied ? t::PortMode::DFP : t::PortMode::DRP));
        CHECK(sameProfile(actual.configuration.desiredProfile, desired));
        CHECK(interrupted.status.code ==
            (callbackFailed ? t::Err::I2C_TIMEOUT : t::Err::CANCELLED));
        if (callbackFailed) CHECK(interrupted.status.detail == -89123);
      }
    }
  }
}

void activeApplyPreservesTerminalDiagnostics() {
  for (const bool priorConfirmed : {false, true}) {
    Fixture f;
    f.initialize();
    if (priorConfirmed) {
      auto profile = f.device.configurationSnapshot().verifiedProfile;
      profile.sourceCurrent = t::SourceCurrent::MA_1500;
      f.bus.write(0x47, 8, 0x40);
      f.proof(0x40);
      const auto first = f.apply(profile);
      CHECK(f.finish(first).ok());
      CHECK(f.take(first).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
      f.bus.reset();
    }
    const auto previous = f.device.diagnostics();
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    desired.sourceCurrent = t::SourceCurrent::DEFAULT;
    f.bus.write(0x47, 8, 0);
    const auto token = f.apply(desired, 300);
    CHECK(f.poll(token, 300).inProgress());
    const auto active = f.device.diagnostics();
    // Latest managed terminal history remains intact while separate flags
    // describe current uncertain hardware and the active result is unavailable.
    CHECK(active.hardwareEffect == previous.hardwareEffect);
    CHECK(active.lastOperationKind == previous.lastOperationKind);
    CHECK(active.lastOperationStatus.code == previous.lastOperationStatus.code);
    CHECK(active.lastOperationStatus.detail == previous.lastOperationStatus.detail);
    CHECK(active.lastOperationStatus.msg == previous.lastOperationStatus.msg);
    CHECK(active.configurationWriteUnresolved);
    CHECK(f.device.cancel(token).ok());
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::PARTIAL && f.bus.complete());
    const auto terminal = f.device.diagnostics();
    CHECK(terminal.hardwareEffect == t::HardwareEffect::PARTIAL);
    CHECK(terminal.lastOperationKind == t::OperationKind::APPLY_PROFILE);
    CHECK(terminal.lastOperationStatus.is(t::Err::CANCELLED));
  }
}

void mismatchAndExplicitRecovery() {
  for (const auto method : {&Driver::startVerifyProfile, &Driver::startReconcile}) {
    Fixture f;
    f.initialize();
    auto desired = f.device.configurationSnapshot().verifiedProfile;
    desired.sourceCurrent = t::SourceCurrent::MA_1500;
    auto token = f.apply(desired);
    CHECK(f.device.cancel(token).ok());
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::NONE);
    f.proof();
    CHECK((f.device.*method)({200, 1000}, token).inProgress());
    CHECK(f.finish(token, 200).is(t::Err::CONFIGURATION_MISMATCH));
    const auto mismatch = f.take(token);
    CHECK(mismatch.hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
    CHECK(mismatch.configuration.state == t::ConfigurationState::KNOWN);
    CHECK(mismatch.configuration.verifiedProfile.sourceCurrent == t::SourceCurrent::MA_3000);
    CHECK(sameProfile(mismatch.configuration.desiredProfile, desired));
    CHECK(mismatch.configuration.mismatchValid && mismatch.configuration.mismatchRegister == 8);
    CHECK(mismatch.configuration.mismatchMask == 0xC0);
    CHECK(mismatch.configuration.mismatchExpected == 0x40 &&
          mismatch.configuration.mismatchObserved == 0x80);
    f.bus.reset();
    f.bus.write(0x47, 8, 0x40);
    f.proof(0x40);
    token = f.apply(desired, 300);
    CHECK(f.finish(token, 300).ok());
    CHECK(f.take(token).configuration.desiredGeneration == 2 && f.bus.complete());
    CHECK(mismatch.status.is(t::Err::CONFIGURATION_MISMATCH)); // Historical result stays frozen.
  }
  for (const bool applied : {false, true}) {
    for (const bool rebind : {false, true}) {
      for (const auto method : {&Driver::startVerifyProfile, &Driver::startReconcile}) {
        Fixture f;
        f.initialize(0x80, 0x14, 0);
        auto desired = f.device.configurationSnapshot().verifiedProfile;
        desired.portMode = t::PortMode::DRP;
        f.bus.write(0x47, 0x0A, 1);
        auto& mode = f.bus.write(0x47, 0x0A, 0x31);
        mode.status = {t::Err::I2C_TIMEOUT, -99123, "mode acceptance unknown"};
        mode.applyWrite = applied;
        auto token = f.apply(desired);
        CHECK(f.finish(token).is(t::Err::I2C_TIMEOUT));
        const auto failed = f.take(token);
        CHECK(failed.hardwareEffect == t::HardwareEffect::INDETERMINATE);
        CHECK(failed.configuration.state == t::ConfigurationState::UNKNOWN && f.bus.complete());
        CHECK(f.device.diagnostics().modeTransitionUnresolved);
        if (rebind) {
          CHECK(f.device.end().ok());
          CHECK(f.device.bind(f.config).ok());
          CHECK(f.device.diagnostics().modeTransitionUnresolved);
        }
        f.bus.reset();
        f.proof(0x80, 0x14, applied ? 0x31 : 1);
        CHECK((f.device.*method)({200, 1000}, token).inProgress());
        CHECK(f.finish(token, 200).is(t::Err::CONFIGURATION_MISMATCH));
        const auto actual = f.take(token);
        CHECK(actual.hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
        CHECK(actual.configuration.state == t::ConfigurationState::KNOWN);
        CHECK(actual.configuration.terminationState == t::TerminationState::DISABLED);
        CHECK(!f.device.diagnostics().modeTransitionUnresolved);
        CHECK(sameProfile(actual.configuration.desiredProfile, desired));
        f.bus.reset();
        if (!applied) {
          f.bus.write(0x47, 0x0A, 1);
          f.bus.write(0x47, 0x0A, 0x31);
        }
        f.bus.write(0x47, 0x0A, 0x30);
        f.proof();
        token = f.apply(desired, 300);
        CHECK(f.finish(token, 300).ok());
        CHECK(f.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
        CHECK(failed.status.detail == -99123);
      }
    }
  }
}

} // namespace

int runProfileApplicationChecks() {
  noOpAndVerification();
  admissionAndValidation();
  literalModePlans();
  ufpAndSettingsOnlyPlans();
  baseAttachmentAndWriteOrder();
  gated322Changes();
  physicalModeWait();
  prewriteAndEveryBoundary();
  faultsAtEveryWriteAndProofRead();
  tusb322ModeWriteFailures();
  tusb322ModeInterruptionBoundaries();
  exactWaitGatesAndLateFinalTime();
  completeApplyMismatchAndPrecheckContradiction();
  applyLateReadbackAndSlotOwnership();
  failedAttemptBudgetAndUnknownActual();
  internal45QualificationAndStagedControls();
  wrappedModeWait();
  interruptedBaseModeEvidence();
  activeApplyPreservesTerminalDiagnostics();
  mismatchAndExplicitRecovery();
  std::printf("%s %zu profile application/verification checks\n", failures == 0 ? "PASS" : "FAIL", checks);
  return failures;
}
