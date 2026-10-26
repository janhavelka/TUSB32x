/// @file test_reset_recovery.cpp
/// @brief Public one-shot reset/recovery scripts, uncertain effects and independent physical clocks.
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
static_assert(std::is_same_v<decltype(&Driver::startReset), Start>);
static_assert(std::is_same_v<decltype(&Driver::startRecoverDeadBattery), Start>);

int failures = 0;
std::size_t checks = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    ++failures;
    std::printf("FAIL reset/recovery line %d: %s\n", line, expression);
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr std::size_t NO_CALL = std::numeric_limits<std::size_t>::max();
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

// Two fixed framing fixtures hold preparation plus one complete F. The microsecond
// clock is test-owned; callback completion and floored owner samples are independent.
struct Bus {
  Fake segments[2]{};
  std::size_t count = 0;
  std::size_t calls = 0;
  std::size_t txBytes = 0;
  std::size_t rxBytes = 0;
  uint64_t nowUs = 0;
  uint64_t completionUs = 0;
  uint64_t followingCallUs = 0;
  std::size_t completionCall = NO_CALL;
  std::size_t followingCall = NO_CALL;
  Driver* owner = nullptr;

  void clear() { *this = Bus{}; }
  Fake::Step& read(uint8_t address, uint8_t reg, std::size_t length = 1) {
    CHECK(count < 16);
    auto& step = segments[count / 8].expectRead(address, reg, length);
    ++count;
    return step;
  }
  Fake::Step& write(uint8_t address, uint8_t reg, uint8_t value) {
    CHECK(count < 16);
    auto& step = segments[count / 8].expectWrite(address, reg, value);
    step.applyWrite = true; // Preserve earlier successful effects for the next fault boundary.
    ++count;
    return step;
  }
  Fake::Step& at(std::size_t index) { return segments[index / 8].script[index % 8]; }
  bool clean() const { return segments[0].errors == 0 && segments[1].errors == 0; }
  bool complete() const {
    return calls == count && segments[0].complete() && segments[1].complete();
  }
  void enter(std::size_t index, std::size_t txLength, std::size_t rxLength) {
    CHECK(index < 16);
    txBytes += txLength;
    rxBytes += rxLength;
    if (index == followingCall) followingCallUs = nowUs;
  }
  static t::Status writeCallback(uint8_t address, const uint8_t* tx, std::size_t length,
                                  uint32_t timeout, void* user) {
    auto& bus = *static_cast<Bus*>(user);
    const auto index = bus.calls++;
    bus.enter(index, length, 0);
    if (index >= 16) return t::Status::Error(t::Err::I2C_ERROR, "script overflow");
    if (bus.owner != nullptr) {
      CHECK(bus.owner->configurationSnapshot().state != t::ConfigurationState::KNOWN);
      CHECK(bus.owner->portSnapshot().invalidated);
      CHECK(bus.owner->portSnapshot().rawValidMask == 0);
    }
    const auto status = Fake::write(address, tx, length, timeout, &bus.segments[index / 8]);
    if (index == bus.completionCall) bus.nowUs = bus.completionUs;
    return status;
  }
  static t::Status readCallback(uint8_t address, const uint8_t* tx, std::size_t txLength,
                                 uint8_t* rx, std::size_t rxLength, uint32_t timeout,
                                 void* user) {
    auto& bus = *static_cast<Bus*>(user);
    const auto index = bus.calls++;
    bus.enter(index, txLength, rxLength);
    if (index >= 16) return t::Status::Error(t::Err::I2C_ERROR, "script overflow");
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
    bus.owner = &device;
  }
  void clearBus() { bus.clear(); bus.owner = &device; }
  void proof(uint8_t byte0A = 0x30, uint8_t byte45 = 0, uint8_t byte08 = 0x80) {
    auto& id = bus.read(part.address, 0, 8);
    std::memcpy(id.rx, part.is322 ? ID322 : ID320, 8);
    if (!part.base) bus.read(part.address, 0xA0).rx[0] = part.revision;
    bus.read(part.address, 8).rx[0] = byte08;
    bus.read(part.address, 9).rx[0] = 0x14;
    bus.read(part.address, 0x0A).rx[0] = byte0A;
    if (!part.is322) bus.read(part.address, 0x45).rx[0] = byte45;
  }
  t::Status poll(t::OperationToken token, uint64_t now, uint8_t budget = 255) {
    const auto before = bus.calls;
    bus.nowUs = now * 1000;
    const auto status = device.poll(token, now, budget);
    CHECK(bus.calls - before <= 1);
    CHECK(device.progress().lastPollTransfers == bus.calls - before);
    CHECK(device.portSnapshot().rawValidMask == 0); // No transient port read/publication.
    return status;
  }
  t::Status finish(t::OperationToken token, uint64_t now = 100, uint8_t budget = 255) {
    for (unsigned turn = 0; turn < 32; ++turn) {
      const auto status = poll(token, now, budget);
      if (!status.inProgress()) return status;
      const auto progress = device.progress();
      if (progress.waiting && progress.readyAtValid) now = progress.readyAtMs;
      else ++now;
    }
    CHECK(false);
    return t::Status::Error(t::Err::I2C_ERROR, "fixed sequence exceeded");
  }
  t::OperationResult take(t::OperationToken token) {
    t::OperationResult result{};
    CHECK(device.takeResult(token, result).ok());
    CHECK(result.token.value == token.value);
    return result;
  }
  void initialize(uint8_t byte0A = 0x30, uint8_t byte45 = 0) {
    proof(byte0A, byte45);
    t::OperationToken token{};
    CHECK(device.startInitialize({10, 1000}, token).inProgress());
    CHECK(finish(token, 10).ok());
    CHECK(take(token).configuration.state == t::ConfigurationState::KNOWN);
    CHECK(bus.complete());
    clearBus();
  }
  void resetScript(uint8_t actual = 0x30, uint8_t final = 0x30) {
    bus.read(part.address, 0x0A).rx[0] = actual;
    // Literal independent vectors: B low nibble discarded; L/A/V retained.
    bus.write(part.address, 0x0A, actual == 0xE0 ? 0xE8 : actual == 0xE2 ? 0xEA : 0x38);
    proof(final);
  }
  void recoveryScript(uint8_t before = 0, uint8_t after = 0) {
    bus.read(part.address, 0x45).rx[0] = before;
    bus.write(part.address, 0x45, 0x04);
    bus.write(part.address, 0x45, 0x00);
    bus.read(part.address, 0x45).rx[0] = after;
  }
  t::OperationToken start(bool recovery, uint64_t now = 100, uint64_t deadline = 1000) {
    t::OperationToken token{999};
    const auto calls = bus.calls;
    const auto status = recovery ? device.startRecoverDeadBattery({now, deadline}, token)
                                 : device.startReset({now, deadline}, token);
    CHECK(status.inProgress() && token.valid() && token.value != 999);
    CHECK(bus.calls == calls);
    CHECK(device.progress().kind == (recovery ? t::OperationKind::RECOVER_DEAD_BATTERY
                                            : t::OperationKind::RESET));
    return token;
  }
  // Stop after a requested callback, before its next fresh completion sample.
  uint64_t through(t::OperationToken token, std::size_t count) {
    uint64_t now = 100;
    for (unsigned turn = 0; turn < 32 && bus.calls < count; ++turn) {
      CHECK(poll(token, now).inProgress());
      const auto progress = device.progress();
      if (progress.waiting && progress.readyAtValid) now = progress.readyAtMs;
      else ++now;
    }
    CHECK(bus.calls == count);
    return now;
  }
};

void admissionAndPrechecks() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    t::OperationToken sentinel{99};
    CHECK(f.device.startReset({100, 100}, sentinel).is(t::Err::INVALID_PARAM));
    CHECK(f.device.startReset({100, 1000}, sentinel).is(t::Err::NOT_INITIALIZED));
    const auto status = f.device.startRecoverDeadBattery({100, 1000}, sentinel);
    CHECK(status.is(part.base ? t::Err::NOT_INITIALIZED : t::Err::UNSUPPORTED_FEATURE));
    CHECK(sentinel.value == 99 && f.bus.calls == 0);
    f.initialize();
    if (!part.base) {
      CHECK(f.device.startRecoverDeadBattery({100, 1000}, sentinel)
                .is(t::Err::UNSUPPORTED_FEATURE));
      CHECK(sentinel.value == 99 && f.bus.calls == 0);
    }
    f.bus.read(part.address, 0x0A).rx[0] = 0x38;
    const auto token = f.start(false);
    CHECK(f.finish(token).is(t::Err::CONFIGURATION_UNKNOWN));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
    CHECK(f.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    if (part.base) {
      CHECK(f.device.startRecoverDeadBattery({200, 1000}, sentinel)
                .is(t::Err::CONFIGURATION_UNKNOWN));
      CHECK(f.bus.calls == 1 && sentinel.value == 99);
    }
  }
  for (const uint8_t internal : {uint8_t{1}, uint8_t{2}, uint8_t{3}}) {
    for (const auto& part : {PARTS[0], PARTS[1]}) {
      Fixture f(part);
      f.initialize();
      f.bus.read(part.address, 0x45).rx[0] = internal;
      const auto token = f.start(true);
      CHECK(f.finish(token).is(t::Err::CONFIGURATION_UNKNOWN));
      const auto result = f.take(token);
      CHECK(result.transfers == 1 && result.hardwareEffect == t::HardwareEffect::NONE);
      CHECK(!f.device.diagnostics().recoverySequenceUnresolved && f.bus.complete());
    }
  }
  Fixture reserved;
  reserved.initialize();
  reserved.bus.read(0x47, 0x0A).rx[0] = 0x34; // TI reserved preference code 10; Try.SRC is 11.
  const auto token = reserved.start(false);
  CHECK(reserved.finish(token).is(t::Err::CONFIGURATION_UNKNOWN));
  CHECK(reserved.take(token).hardwareEffect == t::HardwareEffect::NONE);
  CHECK(reserved.bus.complete());
}

void literalSuccessAndDesiredRetention() {
  for (const auto& part : PARTS) {
    for (const uint8_t budget : {uint8_t{1}, uint8_t{2}, uint8_t{255}}) {
      Fixture f(part);
      const uint8_t actual = part.base ? 0xE0 : 0xE2;
      f.initialize(actual);
      const auto desired = f.device.configurationSnapshot().verifiedProfile;
      f.proof(actual);
      t::OperationToken token{};
      CHECK(f.device.startApplyProfile(desired, {30, 1000}, token).inProgress());
      CHECK(f.finish(token, 30).ok());
      CHECK(f.take(token).hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
      f.clearBus();
      f.resetScript(actual);
      token = f.start(false);
      CHECK(f.poll(token, 100, 0).inProgress() && f.bus.calls == 0);
      CHECK(f.finish(token, 100, budget).ok());
      const auto result = f.take(token);
      CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED);
      CHECK(result.configuration.resetState == t::ResetState::COMPLETED);
      CHECK(result.configuration.state == t::ConfigurationState::KNOWN);
      CHECK(result.configuration.hasDesiredProfile && result.configuration.desiredGeneration == 1);
      CHECK(result.configuration.desiredProfile.portMode == desired.portMode);
      CHECK(result.configuration.desiredProfile.ccDebounce == desired.ccDebounce);
      CHECK(result.configuration.verifiedProfile.portMode == t::PortMode::DRP);
      CHECK(result.port.invalidated && result.port.rawValidMask == 0 && f.bus.complete());
      CHECK(result.transfers == (part.base || part.is322 ? 7U : 8U));
      CHECK(f.bus.txBytes == (part.base || part.is322 ? 8U : 9U));
      CHECK(f.bus.rxBytes == (part.base || part.is322 ? 13U : 14U));
      const auto calls = f.bus.calls;
      CHECK(f.device.poll(token, 999).is(t::Err::RESULT_NOT_AVAILABLE));
      CHECK(f.bus.calls == calls); // No desired replay after success or exact take.
    }
  }
  for (const auto& part : {PARTS[0], PARTS[1]}) {
    for (const uint8_t initial : {uint8_t{0}, uint8_t{4}, uint8_t{0xF8}}) {
      Fixture f(part);
      f.initialize();
      f.recoveryScript(initial, 0xF8); // Only the qualified low three bits matter.
      const auto token = f.start(true);
      CHECK(f.finish(token).ok());
      const auto result = f.take(token);
      CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
      CHECK(result.transfers == 4 && f.bus.txBytes == 6 && f.bus.rxBytes == 2);
      CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
      CHECK(!f.device.diagnostics().recoverySequenceUnresolved);
      CHECK(result.configuration.terminationState == t::TerminationState::ENABLED);
      CHECK(result.port.rawValidMask == 0 && result.port.invalidated);
    }
  }
}

void freshResetControlsAndIdentityContradictions() {
  for (const auto& part : PARTS) {
    Fixture fresh(part);
    fresh.initialize(0x30);
    // The live precheck differs in debounce/mode and extended-only preference.
    // Preserve its actual controls, including debounce, in the one reset command.
    fresh.resetScript(part.base ? 0xE0 : 0xE2);
    const auto token = fresh.start(false);
    CHECK(fresh.finish(token).ok());
    CHECK(fresh.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED);
    CHECK(fresh.bus.complete());

    for (const bool revision : {false, true}) {
      if (revision && part.base) continue;
      Fixture f(part);
      f.initialize();
      f.resetScript();
      const std::size_t badRead = revision ? 3 : 2;
      f.bus.at(badRead).rx[0] = 0xFF;
      const auto bad = f.start(false);
      const auto now = f.through(bad, badRead + 1);
      // Contradiction invalidates readiness before another fresh owner sample.
      CHECK(!f.device.diagnostics().initialized);
      CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
      CHECK(f.poll(bad, now, 0).is(t::Err::CHIP_ID_MISMATCH));
      const auto result = f.take(bad);
      CHECK(result.hardwareEffect == t::HardwareEffect::PARTIAL);
      CHECK(result.completedTimeValid && result.transfers == badRead + 1);
      CHECK(result.configuration.resetState == t::ResetState::MAY_BE_PENDING);
      CHECK(f.bus.calls == badRead + 1 && f.bus.clean());
    }
  }
}

void clippedProcedureTimeouts() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    f.resetScript();
    // R0A at100, W0A at101, fresh anchor102, ready198. Remaining
    // callback budgets are literal milliseconds until the immutable deadline205.
    const uint32_t timeouts[] = {20, 20, 7, 6, 5, 4, 3, 2};
    for (std::size_t i = 0; i < f.bus.count; ++i) f.bus.at(i).timeoutMs = timeouts[i];
    const auto token = f.start(false, 100, 205);
    CHECK(f.finish(token).ok());
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
    CHECK(result.completedUptimeMs == (part.base || part.is322 ? 203U : 204U));
  }
  for (const auto& part : {PARTS[0], PARTS[1]}) {
    Fixture f(part);
    f.initialize();
    f.recoveryScript();
    // R45 at100, W04 at101, fresh anchor102, W00 at133, R45 at134.
    f.bus.at(2).timeoutMs = 7;
    f.bus.at(3).timeoutMs = 6;
    const auto token = f.start(true, 100, 140);
    CHECK(f.finish(token).ok());
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
    CHECK(result.completedUptimeMs == 135);
  }
}

void boundedPostconditionFailures() {
  for (const uint8_t value : {uint8_t{0}, uint8_t{0x30}, uint8_t{0x10}, uint8_t{0x20}, uint8_t{0x28}}) {
    Fixture f(PARTS[6]);
    f.initialize(0x20);
    f.resetScript(0x30, value);
    const auto token = f.start(false);
    const bool drp = value == 0 || value == 0x30;
    const auto status = f.finish(token);
    CHECK(drp ? status.ok() : status.is(value == 0x28 ? t::Err::CONFIGURATION_UNKNOWN
                                                     : t::Err::CONFIGURATION_MISMATCH));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == (drp ? t::HardwareEffect::CONFIRMED : t::HardwareEffect::PARTIAL));
    CHECK(result.configuration.rawConfig0A == value && f.bus.complete());
    if (!drp) {
      CHECK(result.configuration.resetState == t::ResetState::MAY_BE_PENDING);
      CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
      CHECK(result.configuration.terminationState == t::TerminationState::UNKNOWN);
    }
  }
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    f.resetScript(0x30, 0x38);
    const auto token = f.start(false);
    CHECK(f.finish(token).is(t::Err::CONFIGURATION_UNKNOWN));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::PARTIAL && f.bus.complete());
    CHECK(result.configuration.resetState == t::ResetState::MAY_BE_PENDING);
  }
  for (const uint8_t value : {uint8_t{1}, uint8_t{2}, uint8_t{3}, uint8_t{4}, uint8_t{7}}) {
    Fixture f(PARTS[0]);
    f.initialize();
    f.recoveryScript(0, value);
    const auto token = f.start(true);
    CHECK(f.finish(token).is(t::Err::CONFIGURATION_MISMATCH));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::PARTIAL && f.bus.complete());
    CHECK(f.device.diagnostics().recoverySequenceUnresolved);
    CHECK(result.configuration.terminationState == (value == 4 ? t::TerminationState::DISABLED
                                                              : t::TerminationState::UNKNOWN));
  }
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    f.resetScript(0x30, part.is322 ? 0x20 : 0x30);
    // 08 advertisement code11 is reserved independently of the valid mode field.
    f.bus.at(part.base ? 3 : 4).rx[0] = 0xC0;
    const auto token = f.start(false);
    CHECK(f.finish(token).is(t::Err::CONFIGURATION_UNKNOWN));
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::PARTIAL && f.bus.complete());
    CHECK(result.configuration.rawConfig08 == 0xC0);
    CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
    if (part.is322) {
      CHECK(result.configuration.resetState == t::ResetState::MAY_BE_PENDING);
      CHECK(result.configuration.terminationState == t::TerminationState::UNKNOWN);
    }
  }
}

constexpr t::Err FAULTS[] = {t::Err::I2C_ERROR, t::Err::I2C_NACK_ADDR, t::Err::I2C_NACK_DATA,
                            t::Err::I2C_TIMEOUT, t::Err::I2C_BUS, t::Err::I2C_BUSY};
void everyCallbackFault() {
  for (const bool recovery : {false, true}) {
    for (const auto& part : PARTS) {
      if (recovery && !part.base) continue;
      const std::size_t count = recovery ? 4 : part.base || part.is322 ? 7 : 8;
      for (std::size_t index = 0; index < count; ++index) {
        for (const auto error : FAULTS) {
          for (const bool applied : {false, true}) {
            Fixture f(part);
            f.initialize();
            if (recovery) f.recoveryScript(); else f.resetScript();
            auto& fault = f.bus.at(index);
            const bool write = fault.kind == Fake::Kind::WRITE;
            if (!write && applied) continue;
            fault.status = {error, -2147000123, "injected reset/recovery callback"};
            fault.applyWrite = applied;
            fault.fillLength = 0;
            const auto attempts = f.device.transportStats().totalAttempts;
            const auto token = f.start(recovery);
            const auto status = f.finish(token);
            CHECK(status.code == error && status.detail == -2147000123);
            CHECK(status.msg == fault.status.msg);
            CHECK(f.bus.calls == index + 1 && f.bus.clean());
            const auto result = f.take(token);
            CHECK(result.status.code == error && result.status.detail == -2147000123);
            CHECK(result.transfers == index + 1 && !result.completedTimeValid);
            CHECK(result.hardwareEffect == (write ? t::HardwareEffect::INDETERMINATE
                                                   : index < 2 ? t::HardwareEffect::NONE
                                                               : t::HardwareEffect::PARTIAL));
            CHECK(f.device.transportStats().totalAttempts == attempts + index + 1);
            CHECK(f.device.diagnostics().lastFailedTransportStatus.detail == -2147000123);
            if (index != 0) {
              CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
              CHECK(recovery ? f.device.diagnostics().recoverySequenceUnresolved
                             : f.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
            }
            if (write) {
              const uint8_t preceding = recovery && index == 2 ? 0x04 : 0x00;
              CHECK(f.bus.segments[index / 8].registers[fault.tx[0]] ==
                    (applied ? fault.tx[1] : preceding));
            }
          }
        }
      }
    }
  }
}

void everyInterruptionBoundary() {
  for (const bool recovery : {false, true}) {
    for (const auto& part : PARTS) {
      if (recovery && !part.base) continue;
      const std::size_t count = recovery ? 4 : part.base || part.is322 ? 7 : 8;
      // Each callback boundary includes final read before its required fresh sample.
      // The extra boundary is the anchored/stalled hardware wait after callback two.
      for (std::size_t boundary = 0; boundary <= count + 1; ++boundary) {
        for (unsigned action = 0; action < 4; ++action) {
          Fixture f(part);
          f.initialize();
          if (recovery) f.recoveryScript(); else f.resetScript();
          const auto token = f.start(recovery);
          const auto calls = boundary > count ? std::size_t{2} : boundary;
          auto now = f.through(token, calls);
          if (boundary > count) {
            CHECK(f.poll(token, now, 0).inProgress());
            CHECK(f.device.progress().waiting && f.device.progress().readyAtValid);
            CHECK(f.bus.calls == 2);
          }
          const auto before = f.bus.calls;
          if (action == 0) CHECK(f.device.cancel(token).ok());
          if (action == 1) CHECK(f.device.end().ok());
          if (action == 2) CHECK(f.poll(token, 1000, 0).is(t::Err::TIMEOUT));
          if (action == 3) CHECK(f.device.invalidateDeviceState(
              {t::Err::CONFIGURATION_UNKNOWN, -12345, "external loss"}).ok());
          CHECK(f.bus.calls == before && f.bus.clean());
          t::OperationResult first{};
          CHECK(f.device.resultSnapshot(token, first).ok());
          CHECK(first.status.is(action == 2 ? t::Err::TIMEOUT : t::Err::CANCELLED));
          CHECK(first.hardwareEffect == (calls < 2 ? t::HardwareEffect::NONE : t::HardwareEffect::PARTIAL));
          CHECK(first.completedTimeValid == (action == 2));
          CHECK(first.transfers == before && first.port.rawValidMask == 0);
          if (calls >= 2) {
            CHECK(recovery ? f.device.diagnostics().recoverySequenceUnresolved
                           : f.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
          }
          t::OperationToken rejected{77};
          const auto pending = recovery ? f.device.startRecoverDeadBattery({1001, 2000}, rejected)
                                        : f.device.startReset({1001, 2000}, rejected);
          CHECK(pending.is(action == 1 ? t::Err::NOT_BOUND : t::Err::RESULT_PENDING));
          CHECK(rejected.value == 77);
          CHECK(f.device.poll(token, 1001).code == first.status.code);
          CHECK(f.take(token).hardwareEffect == first.hardwareEffect);
          CHECK(f.bus.calls == before);
        }
      }
    }
  }
}

void independentPhysicalWaits() {
  for (const bool recovery : {false, true}) {
    for (const uint64_t fractional : {uint64_t{100}, uint64_t{900}}) {
      for (const uint64_t freshDelay : {uint64_t{50}, uint64_t{2050}}) {
        Fixture f(recovery ? PARTS[0] : PARTS[4]);
        f.initialize();
        if (recovery) f.recoveryScript(); else f.resetScript();
        const auto token = f.start(recovery);
        const uint64_t completed = (recovery ? 200000U : 300000U) + fractional;
        f.bus.completionCall = 1;
        f.bus.completionUs = completed;
        f.bus.followingCall = 2;
        CHECK(f.poll(token, 100).inProgress());
        CHECK(f.poll(token, 101).inProgress());
        CHECK(f.bus.nowUs == completed && f.bus.calls == 2);
        CHECK(f.device.progress().waiting && !f.device.progress().readyAtValid);
        // Querying cached values supplies no completion clock or wait progress.
        CHECK(f.device.progress().transfers == 2 && !f.device.progress().readyAtValid);
        const uint64_t freshPhysical = completed + freshDelay;
        const uint64_t freshTick = freshPhysical / 1000;
        f.bus.nowUs = freshPhysical;
        CHECK(f.device.poll(token, freshTick, 0).inProgress());
        CHECK(f.bus.calls == 2);
        const uint64_t ticks = recovery ? 31 : 96;
        CHECK(f.device.progress().readyAtMs == freshTick + ticks);
        for (unsigned stalled = 0; stalled < 3; ++stalled) {
          CHECK(f.poll(token, freshTick, 255).inProgress() && f.bus.calls == 2);
        }
        CHECK(f.poll(token, freshTick + ticks - 1).inProgress() && f.bus.calls == 2);
        CHECK(f.poll(token, freshTick + ticks, 0).inProgress() && f.bus.calls == 2);
        CHECK(f.poll(token, freshTick + ticks).inProgress() && f.bus.calls == 3);
        CHECK(f.bus.followingCallUs - completed >= (recovery ? 30000U : 95000U));
        CHECK(f.finish(token, freshTick + ticks).ok());
        CHECK(f.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
      }
    }
    // Deadline equality wins over readiness, including the software margin.
    Fixture f(recovery ? PARTS[0] : PARTS[4]);
    f.initialize();
    if (recovery) f.recoveryScript(); else f.resetScript();
    const uint64_t deadline = recovery ? 231 : 296;
    const auto token = f.start(recovery, 100, deadline);
    CHECK(f.poll(token, 100).inProgress());
    CHECK(f.poll(token, 101).inProgress());
    CHECK(f.poll(token, 200, 0).inProgress());
    CHECK(f.device.progress().readyAtMs == deadline);
    CHECK(f.poll(token, deadline, 255).is(t::Err::TIMEOUT) && f.bus.calls == 2);
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::PARTIAL && f.bus.clean());
  }
}

void wrappedWaitAndDisabledControls() {
  for (const bool recovery : {false, true}) {
    Fixture f(recovery ? PARTS[0] : PARTS[4]);
    f.initialize();
    if (recovery) f.recoveryScript(); else f.resetScript();
    const uint64_t begin = std::numeric_limits<uint64_t>::max() - 20;
    const auto token = f.start(recovery, begin, 200);
    CHECK(f.poll(token, begin).inProgress());
    CHECK(f.poll(token, begin + 1).inProgress());
    CHECK(f.poll(token, begin + 2, 0).inProgress());
    const uint64_t ready = recovery ? 12 : 77;
    CHECK(f.device.progress().readyAtMs == ready);
    CHECK(f.poll(token, ready - 1).inProgress() && f.bus.calls == 2);
    CHECK(f.poll(token, ready).inProgress() && f.bus.calls == 3);
    CHECK(f.finish(token, ready).ok());
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
  }
  for (const auto& part : PARTS) {
    if (part.base) continue;
    Fixture f(part);
    f.initialize(0xE3);
    f.bus.read(part.address, 0x0A).rx[0] = 0xE3;
    f.bus.write(part.address, 0x0A, 0xEB); // Retain both low persistent controls.
    f.proof(0x31);
    const auto token = f.start(false);
    CHECK(f.finish(token).ok());
    const auto result = f.take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
    CHECK(result.configuration.terminationState == t::TerminationState::DISABLED);
  }
}

void recoveryHistoryAndExplicitRestart() {
  for (const bool accepted : {false, true}) {
    Fixture f(PARTS[0]);
    f.initialize();
    f.recoveryScript();
    auto& failedWrite = f.bus.at(1);
    failedWrite.status = {t::Err::I2C_TIMEOUT, -99123, "recovery acceptance unknown"};
    failedWrite.applyWrite = accepted;
    const auto token = f.start(true);
    CHECK(f.finish(token).is(t::Err::I2C_TIMEOUT));
    CHECK(f.device.end().ok());
    t::OperationResult saved{};
    CHECK(f.device.resultSnapshot(token, saved).ok());
    CHECK(saved.hardwareEffect == t::HardwareEffect::INDETERMINATE);
    CHECK(f.take(token).status.detail == -99123);
    CHECK(f.device.bind(f.config).ok());
    CHECK(f.device.diagnostics().recoverySequenceUnresolved);
    f.clearBus();
    f.proof();
    t::OperationToken next{};
    CHECK(f.device.startInitialize({200, 1000}, next).inProgress());
    CHECK(f.finish(next, 200).is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(f.take(next).hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
    CHECK(f.device.diagnostics().initialized && f.device.diagnostics().recoverySequenceUnresolved);
    f.clearBus();
    f.proof();
    CHECK(f.device.startReconcile({300, 1000}, next).inProgress());
    CHECK(f.finish(next, 300).is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(f.take(next).hardwareEffect == t::HardwareEffect::NONE && f.bus.complete());
    CHECK(f.device.diagnostics().recoverySequenceUnresolved);
    // A complete new reset proves only itself and must preserve old recovery history.
    f.clearBus();
    f.resetScript();
    next = f.start(false, 400, 1000);
    CHECK(f.finish(next, 400).ok());
    CHECK(f.take(next).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
    CHECK(f.device.diagnostics().recoverySequenceUnresolved);
    CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    // Own visible historical obligation cannot make a fresh explicit attempt inadmissible.
    f.clearBus();
    f.recoveryScript();
    next = f.start(true, 600, 1000);
    CHECK(f.finish(next, 600).ok());
    CHECK(f.take(next).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
    CHECK(!f.device.diagnostics().recoverySequenceUnresolved);
    CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    f.clearBus();
    f.proof();
    CHECK(f.device.startReconcile({800, 1500}, next).inProgress());
    CHECK(f.finish(next, 800).ok());
    CHECK(f.take(next).configuration.state == t::ConfigurationState::KNOWN && f.bus.complete());
    CHECK(saved.status.detail == -99123 && saved.hardwareEffect == t::HardwareEffect::INDETERMINATE);
  }
}

void resetAcceptanceAndOtherTransitionGate() {
  for (const auto& part : PARTS) {
    for (const bool accepted : {false, true}) {
      Fixture f(part);
      f.initialize();
      f.resetScript();
      f.bus.at(1).status = {t::Err::I2C_TIMEOUT, -88776, "reset acceptance unknown"};
      f.bus.at(1).applyWrite = accepted;
      const auto token = f.start(false);
      CHECK(f.finish(token).is(t::Err::I2C_TIMEOUT));
      CHECK(f.device.diagnostics().resetAcceptanceUncertain);
      CHECK(f.device.end().ok());
      const auto saved = f.take(token);
      CHECK(f.device.bind(f.config).ok());
      CHECK(f.device.diagnostics().resetAcceptanceUncertain);
      f.clearBus();
      f.proof();
      t::OperationToken next{};
      CHECK(f.device.startReconcile({300, 1000}, next).inProgress());
      CHECK(f.poll(next, 300).inProgress() && f.bus.calls == 0);
      CHECK(f.poll(next, 395).inProgress() && f.bus.calls == 0);
      CHECK(f.finish(next, 396).ok());
      const auto proof = f.take(next);
      CHECK(proof.configuration.resetState == t::ResetState::COMPLETED && f.bus.complete());
      CHECK(f.device.diagnostics().resetAcceptanceUncertain);
      CHECK(saved.hardwareEffect == t::HardwareEffect::INDETERMINATE && saved.status.detail == -88776);
      f.clearBus();
      f.resetScript();
      next = f.start(false, 500, 1000);
      CHECK(f.finish(next, 500).ok());
      CHECK(f.take(next).hardwareEffect == t::HardwareEffect::CONFIRMED && f.bus.complete());
      CHECK(f.device.diagnostics().resetAcceptanceUncertain); // New acceptance cannot prove old acceptance.
    }
  }
  Fixture mode(PARTS[0]);
  mode.initialize(0x20);
  auto desired = mode.device.configurationSnapshot().verifiedProfile;
  desired.portMode = t::PortMode::DRP;
  mode.bus.read(0x60, 9).rx[0] = 0x14;
  mode.bus.write(0x60, 0x0A, 0x30).status =
      {t::Err::I2C_TIMEOUT, -44556, "mode acceptance unknown"};
  t::OperationToken token{};
  CHECK(mode.device.startApplyProfile(desired, {100, 1000}, token).inProgress());
  CHECK(mode.finish(token).is(t::Err::I2C_TIMEOUT));
  CHECK(mode.take(token).hardwareEffect == t::HardwareEffect::INDETERMINATE);
  CHECK(mode.device.diagnostics().modeTransitionUnresolved && mode.bus.complete());
  token.value = 99;
  CHECK(mode.device.startRecoverDeadBattery({200, 1000}, token)
            .is(t::Err::CONFIGURATION_UNKNOWN));
  CHECK(mode.bus.calls == 2 && token.value == 99);
  mode.clearBus();
  mode.resetScript();
  token = mode.start(false, 300);
  CHECK(mode.finish(token, 300).ok());
  CHECK(mode.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && mode.bus.complete());
  CHECK(!mode.device.diagnostics().modeTransitionUnresolved);

  Fixture extended;
  extended.initialize(0);
  desired = extended.device.configurationSnapshot().verifiedProfile;
  desired.portMode = t::PortMode::DRP;
  extended.bus.write(0x47, 0x0A, 1);
  extended.bus.write(0x47, 0x0A, 0x31).status =
      {t::Err::I2C_TIMEOUT, -44557, "extended mode acceptance unknown"};
  CHECK(extended.device.startApplyProfile(desired, {100, 1000}, token).inProgress());
  CHECK(extended.finish(token).is(t::Err::I2C_TIMEOUT));
  CHECK(extended.take(token).hardwareEffect == t::HardwareEffect::INDETERMINATE);
  CHECK(extended.device.diagnostics().modeTransitionUnresolved && extended.bus.complete());
  extended.clearBus();
  extended.resetScript();
  token = extended.start(false, 300);
  CHECK(extended.finish(token, 300).ok());
  CHECK(extended.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED && extended.bus.complete());
  CHECK(!extended.device.diagnostics().modeTransitionUnresolved);
}
} // namespace

int runResetRecoveryChecks() {
  admissionAndPrechecks();
  literalSuccessAndDesiredRetention();
  freshResetControlsAndIdentityContradictions();
  clippedProcedureTimeouts();
  boundedPostconditionFailures();
  everyCallbackFault();
  everyInterruptionBoundary();
  independentPhysicalWaits();
  wrappedWaitAndDisabledControls();
  recoveryHistoryAndExplicitRestart();
  resetAcceptanceAndOtherTransitionGate();
  std::printf("%s %zu reset/recovery checks\n", failures == 0 ? "PASS" : "FAIL", checks);
  return failures;
}
