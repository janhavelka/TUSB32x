/// @file test_diagnostics.cpp
/// @brief Raw callback boundaries, retained diagnostics and independent passive health vectors.
#include <TUSB32x/TUSB32x.h>

#include "support/FakeTusb32xDevice.h"
#include "TUSB32xProtocol.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <type_traits>

namespace {
namespace t = TUSB32x;
namespace p = TUSB32x::detail;
using Driver = t::TUSB32x;
using Fake = tusb32x_test::FakeTusb32xDevice;
using Start = t::Status (Driver::*)(const t::OperationTiming&, t::OperationToken&);
static_assert(std::is_same_v<decltype(&Driver::readRegisterSync),
                            t::Status (Driver::*)(uint8_t, uint8_t&)>);
static_assert(std::is_same_v<decltype(&Driver::writeRegisterSync),
                            t::Status (Driver::*)(uint8_t, uint8_t)>);

int failures = 0;
std::size_t checks = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    if (failures < 60) std::printf("FAIL diagnostics line %d: %s\n", line, expression);
    ++failures;
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr uint32_t MAX = std::numeric_limits<uint32_t>::max();
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

bool sameStatus(const t::Status& a, const t::Status& b) {
  return a.code == b.code && a.detail == b.detail && std::strcmp(a.msg, b.msg) == 0;
}

bool sameEvidence(const t::ManagedTransportEvidence& a, const t::ManagedTransportEvidence& b) {
  return a.initialized == b.initialized && a.totalSuccess == b.totalSuccess &&
      a.totalFailures == b.totalFailures && a.consecutiveFailures == b.consecutiveFailures &&
      sameStatus(a.lastStatus, b.lastStatus) &&
      a.lastSuccessUptimeMs == b.lastSuccessUptimeMs &&
      a.lastSuccessTimeValid == b.lastSuccessTimeValid &&
      a.lastFailureUptimeMs == b.lastFailureUptimeMs &&
      a.lastFailureTimeValid == b.lastFailureTimeValid;
}

t::DriverState health(const Driver& device, uint32_t threshold = 3) {
  const auto before = device.diagnostics().managedTransport;
  t::HealthSnapshot out{};
  CHECK(t::projectHealth(before, {threshold}, out).ok());
  CHECK(sameEvidence(out.evidence, before));
  CHECK(sameEvidence(device.diagnostics().managedTransport, before));
  CHECK(out.offlineThreshold == threshold);
  return out.state;
}

struct Fixture {
  Fake fake{};
  Driver device{};
  t::DriverConfig config{};
  const Part& part;
  bool inspectInvalidation = false;
  bool expectOrdinaryWrite = true;
  t::PortSnapshot historical{};

  static t::Status write(uint8_t address, const uint8_t* tx, std::size_t length,
                         uint32_t timeout, void* user) {
    auto& f = *static_cast<Fixture*>(user);
    if (f.inspectInvalidation) {
      CHECK(f.device.diagnostics().hardwareEffect == t::HardwareEffect::INDETERMINATE);
      const auto port = f.device.portSnapshot();
      CHECK(port.invalidated);
      CHECK(port.rawStatus08 == f.historical.rawStatus08 &&
            port.rawStatus09 == f.historical.rawStatus09);
      CHECK(port.sequence == f.historical.sequence &&
            port.observedUptimeMs == f.historical.observedUptimeMs &&
            port.observedTimeValid == f.historical.observedTimeValid);
      if (f.expectOrdinaryWrite) {
        CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
        CHECK(f.device.diagnostics().configurationState == t::ConfigurationState::UNKNOWN);
        CHECK(f.device.diagnostics().configurationWriteUnresolved);
      }
    }
    return Fake::write(address, tx, length, timeout, &f.fake);
  }

  static t::Status writeRead(uint8_t address, const uint8_t* tx, std::size_t txLength,
                             uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* user) {
    auto& f = *static_cast<Fixture*>(user);
    return Fake::writeRead(address, tx, txLength, rx, rxLength, timeout, &f.fake);
  }

  explicit Fixture(const Part& selected = PARTS[4]) : part(selected) {
    config.variant = part.variant;
    config.address = static_cast<t::DeviceAddress>(part.address);
    config.i2cWrite = write;
    config.i2cWriteRead = writeRead;
    config.i2cUser = this;
    CHECK(device.bind(config).ok());
  }

  void proof(uint8_t byte08 = 0x80, uint8_t byte09 = 0x14, uint8_t byte0A = 0x30,
             uint8_t byte45 = 0) {
    auto& id = fake.expectRead(part.address, 0, 8);
    std::memcpy(id.rx, part.is322 ? ID322 : ID320, 8);
    if (!part.base) fake.expectRead(part.address, 0xA0, 1).rx[0] = part.revision;
    fake.expectRead(part.address, 8, 1).rx[0] = byte08;
    fake.expectRead(part.address, 9, 1).rx[0] = byte09;
    fake.expectRead(part.address, 0x0A, 1).rx[0] = byte0A;
    if (!part.is322) fake.expectRead(part.address, 0x45, 1).rx[0] = byte45;
  }

  t::OperationToken start(Start method, uint64_t now = 100) {
    t::OperationToken token{};
    const auto before = fake.calls;
    CHECK((device.*method)({now, now + 1000}, token).inProgress());
    CHECK(token.valid() && fake.calls == before);
    return token;
  }

  t::Status finish(t::OperationToken token, uint64_t now = 100) {
    for (unsigned turn = 0; turn < 32; ++turn) {
      const auto before = fake.calls;
      const auto status = device.poll(token, now, 255);
      CHECK(fake.calls - before <= 1);
      if (!status.inProgress()) return status;
      const auto progress = device.progress();
      if (progress.waiting && progress.readyAtValid) now = progress.readyAtMs;
      else ++now;
    }
    CHECK(false);
    return t::Status::Error(t::Err::I2C_ERROR, "bounded diagnostic fixture did not finish");
  }

  t::OperationResult take(t::OperationToken token) {
    t::OperationResult result{};
    CHECK(device.takeResult(token, result).ok());
    CHECK(result.token.value == token.value);
    return result;
  }

  void initialize(uint8_t byte08 = 0x80, uint8_t byte09 = 0x14,
                  uint8_t byte0A = 0x30, uint8_t byte45 = 0) {
    fake = Fake{};
    proof(byte08, byte09, byte0A, byte45);
    const auto token = start(&Driver::startInitialize);
    CHECK(finish(token).ok() && fake.complete());
    CHECK(take(token).configuration.state == t::ConfigurationState::KNOWN);
    CHECK(health(device) == t::DriverState::READY);
    fake = Fake{};
  }

  void refresh(uint8_t byte09 = 0x94) {
    fake = Fake{};
    fake.expectRead(part.address, 9, 1).rx[0] = byte09;
    fake.expectRead(part.address, 8, 1).rx[0] = 0x91;
    fake.expectRead(part.address, 9, 1).rx[0] = byte09;
    const auto token = start(&Driver::startRefreshPort);
    CHECK(finish(token).ok() && fake.complete());
    historical = take(token).port;
    CHECK(!historical.invalidated && historical.observedTimeValid);
    fake = Fake{};
  }

  void managedFailure(int32_t detail = -900000) {
    fake = Fake{};
    fake.expectRead(part.address, 8, 1).status = {t::Err::I2C_BUSY, detail, "managed bus busy"};
    const auto token = start(&Driver::startReadProfile);
    CHECK(finish(token).is(t::Err::I2C_BUSY) && fake.complete());
    CHECK(take(token).status.detail == detail);
    fake = Fake{};
  }
};

void projectionTable() {
  CHECK(t::HealthPolicy{}.offlineThreshold == 3);
  for (const uint32_t threshold : {uint32_t{1}, uint32_t{3}, MAX}) {
    CHECK(t::validateHealthPolicy({threshold}).ok());
    for (const bool initialized : {false, true}) {
      for (const uint32_t streak : {uint32_t{0}, uint32_t{1}, uint32_t{2},
                                   uint32_t{3}, uint32_t{4}, MAX - 1, MAX}) {
        t::ManagedTransportEvidence evidence{};
        evidence.initialized = initialized;
        evidence.totalSuccess = MAX;
        evidence.totalFailures = 7;
        evidence.consecutiveFailures = streak;
        evidence.lastStatus = {t::Err::I2C_NACK_DATA, -2147483000, "projection evidence"};
        evidence.lastSuccessUptimeMs = std::numeric_limits<uint64_t>::max();
        evidence.lastSuccessTimeValid = true;
        evidence.lastFailureUptimeMs = 12345678901234ULL;
        evidence.lastFailureTimeValid = true;
        const auto before = evidence;
        t::HealthSnapshot out{};
        out.state = t::DriverState::OFFLINE;
        out.offlineThreshold = 55;
        CHECK(t::projectHealth(evidence, {threshold}, out).ok());
        const auto expected = !initialized ? t::DriverState::UNINIT : streak == 0
            ? t::DriverState::READY : streak < threshold
            ? t::DriverState::DEGRADED : t::DriverState::OFFLINE;
        CHECK(out.state == expected && out.offlineThreshold == threshold);
        CHECK(sameEvidence(out.evidence, before) && sameEvidence(evidence, before));
        const auto saved = out;
        CHECK(t::projectHealth(evidence, {0}, out).is(t::Err::INVALID_CONFIG));
        CHECK(out.state == saved.state && out.offlineThreshold == saved.offlineThreshold);
        CHECK(sameEvidence(out.evidence, saved.evidence) && sameEvidence(evidence, before));
      }
    }
  }
  CHECK(t::validateHealthPolicy({0}).is(t::Err::INVALID_CONFIG));
}

void rawReadMapAndSlot() {
  Driver unbound;
  uint8_t out = 0xD3;
  CHECK(unbound.readRegisterSync(0xFF, out).is(t::Err::INVALID_PARAM));
  CHECK(unbound.readRegisterSync(0, out).is(t::Err::NOT_BOUND) && out == 0xD3);
  for (const auto& part : PARTS) {
    Fixture f(part);
    for (unsigned reg = 0; reg < 256; ++reg) {
      f.fake = Fake{};
      out = 0xD3;
      const bool mapped = reg <= 0x0A || reg == 0x45 || reg == 0xA0;
      const bool absent = (reg == 0x45 && part.is322) || (reg == 0xA0 && part.base);
      if (mapped && !absent) {
        auto& step = f.fake.expectRead(part.address, static_cast<uint8_t>(reg), 1);
        step.rx[0] = static_cast<uint8_t>(reg ^ 0x5A);
        step.callerOutput = &out;
      }
      const auto before = f.device.transportStats();
      const auto managed = f.device.diagnostics().managedTransport;
      const auto status = f.device.readRegisterSync(static_cast<uint8_t>(reg), out);
      if (!mapped || absent) {
        CHECK(status.is(absent ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM));
        CHECK(out == 0xD3 && f.fake.calls == 0);
        CHECK(f.device.transportStats().totalAttempts == before.totalAttempts);
      } else {
        CHECK(status.ok() && out == static_cast<uint8_t>(reg ^ 0x5A));
        CHECK(f.fake.calls == 1 &&
              f.device.transportStats().totalAttempts == before.totalAttempts + 1);
      }
      CHECK(f.fake.complete());
      CHECK(sameEvidence(f.device.diagnostics().managedTransport, managed));
      CHECK(!f.device.diagnostics().initialized && !f.device.diagnostics().identity.identityValid);
      CHECK(f.device.portSnapshot().sequence == 0);
      const auto writeBefore = f.device.transportStats();
      const auto write = f.device.writeRegisterSync(static_cast<uint8_t>(reg), 0);
      const bool readOnly = reg <= 7 || reg == 0xA0;
      const bool gated = part.is322 && (reg == 8 || reg == 9);
      const auto expected = !mapped ? t::Err::INVALID_PARAM : absent || readOnly || gated
          ? t::Err::UNSUPPORTED_FEATURE : t::Err::NOT_INITIALIZED;
      CHECK(write.is(expected));
      CHECK(f.device.transportStats().totalAttempts == writeBefore.totalAttempts);
      CHECK(f.fake.complete());
    }
    f.fake = Fake{};
    const auto token = f.start(&Driver::startReadProfile);
    CHECK(f.device.readRegisterSync(0xFF, out).is(t::Err::INVALID_PARAM));
    CHECK(f.device.readRegisterSync(part.base ? 0xA0 : part.is322 ? 0x45 : 0xFF, out)
              .is(part.base || part.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM));
    CHECK(f.device.readRegisterSync(8, out).is(t::Err::BUSY));
    CHECK(f.device.writeRegisterSync(8, 0x40).is(part.is322
        ? t::Err::UNSUPPORTED_FEATURE : t::Err::BUSY));
    CHECK(f.device.cancel(token).ok());
    CHECK(f.device.readRegisterSync(8, out).is(t::Err::RESULT_PENDING));
    CHECK(f.device.writeRegisterSync(0x0A, 0x30).is(t::Err::RESULT_PENDING));
    CHECK(f.fake.calls == 0 && out == 0xD3);
    CHECK(f.take(token).status.is(t::Err::CANCELLED));
    f.fake.expectRead(part.address, 8, 1).rx[0] = 0x40;
    CHECK(f.device.readRegisterSync(8, out).ok() && out == 0x40 && f.fake.complete());
  }
}

void localEventsDoNotAccount() {
  Fixture f;
  f.initialize();
  f.managedFailure();
  const auto evidence = f.device.diagnostics().managedTransport;
  const auto stats = f.device.transportStats();
  auto token = f.start(&Driver::startReadProfile);
  CHECK(f.device.poll(token, 101, 0).inProgress());
  uint8_t out = 0xD3;
  CHECK(f.device.readRegisterSync(8, out).is(t::Err::BUSY) && out == 0xD3);
  CHECK(f.device.writeRegisterSync(8, 0x40).is(t::Err::BUSY));
  CHECK(f.device.cancel(token).ok());
  CHECK(f.take(token).status.is(t::Err::CANCELLED));
  token = f.start(&Driver::startReadProfile);
  CHECK(f.device.poll(token, 1100).is(t::Err::TIMEOUT));
  CHECK(f.take(token).status.is(t::Err::TIMEOUT));
  CHECK(f.fake.calls == 0 && f.fake.complete());
  CHECK(sameEvidence(f.device.diagnostics().managedTransport, evidence));
  CHECK(f.device.transportStats().totalAttempts == stats.totalAttempts &&
        f.device.transportStats().totalSuccess == stats.totalSuccess &&
        f.device.transportStats().totalFailures == stats.totalFailures &&
        sameStatus(f.device.transportStats().lastStatus, stats.lastStatus));
  CHECK(f.device.diagnostics().lastOperationStatus.is(t::Err::TIMEOUT));
  CHECK(health(f.device) == t::DriverState::DEGRADED);
}

void callbackNormalizationAndRawIsolation() {
  Fixture f;
  f.initialize();
  f.managedFailure(-7654321);
  const auto managed = f.device.diagnostics().managedTransport;
  const auto operation = f.device.diagnostics();
  f.device.clearTransportStats();
  uint32_t successes = 0;
  uint32_t errors = 0;
  for (unsigned code = 0; code < 256; ++code) {
    f.fake = Fake{};
    uint8_t out = 0xD3;
    auto& step = f.fake.expectRead(0x47, 8, 1);
    step.rx[0] = 0xA6;
    step.callerOutput = &out;
    step.status = {static_cast<t::Err>(code), -2000000000 + static_cast<int32_t>(code),
                   "callback code"};
    const auto status = f.device.readRegisterSync(8, out);
    const bool legal = code == 0 || (code >= 17 && code <= 22);
    const auto expected = legal ? static_cast<t::Err>(code) : t::Err::I2C_ERROR;
    CHECK(status.code == expected && status.detail == step.status.detail);
    if (legal) CHECK(status.msg == step.status.msg);
    CHECK(out == (code == 0 ? 0xA6 : 0xD3));
    if (code == 0) ++successes; else ++errors;
    const auto stats = f.device.transportStats();
    CHECK(stats.totalAttempts == code + 1 && stats.totalSuccess == successes &&
          stats.totalFailures == errors);
    CHECK(sameStatus(stats.lastStatus, status) && f.fake.calls == 1 && f.fake.complete());
    const auto diagnostics = f.device.diagnostics();
    CHECK(sameEvidence(diagnostics.managedTransport, managed));
    CHECK(diagnostics.lastOperationKind == operation.lastOperationKind &&
          sameStatus(diagnostics.lastOperationStatus, operation.lastOperationStatus));
    CHECK(!diagnostics.lastFailedTransportTimeValid);
    if (code != 0) CHECK(sameStatus(diagnostics.lastFailedTransportStatus, status));
    CHECK(health(f.device) == t::DriverState::DEGRADED);
  }
  const auto failure = f.device.diagnostics().lastFailedTransportStatus;
  f.fake = Fake{};
  f.fake.expectRead(0x47, 8, 1).rx[0] = 0;
  uint8_t out = 5;
  CHECK(f.device.readRegisterSync(8, out).ok() && f.fake.complete());
  CHECK(sameStatus(f.device.diagnostics().lastFailedTransportStatus, failure));
  CHECK(sameEvidence(f.device.diagnostics().managedTransport, managed));
}

void rawWriteGuards() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    CHECK(f.device.writeRegisterSync(0, 0).is(t::Err::UNSUPPORTED_FEATURE));
    CHECK(f.device.writeRegisterSync(0xFF, 0).is(t::Err::INVALID_PARAM));
    CHECK(f.device.writeRegisterSync(0xA0, 0).is(t::Err::UNSUPPORTED_FEATURE));
    CHECK(f.device.writeRegisterSync(0x45, 0).is(part.is322
        ? t::Err::UNSUPPORTED_FEATURE : t::Err::NOT_INITIALIZED));
    CHECK(f.device.writeRegisterSync(0x0A, 0x30).is(t::Err::NOT_INITIALIZED));
    CHECK(f.fake.calls == 0);
    f.initialize();
    struct Rejection { uint8_t reg; uint8_t value; };
    // Literal RO/reserved encodings, reset, mode, termination and recovery bypasses.
    constexpr Rejection REJECTED[] = {{8, 0x81}, {8, 0xC0}, {9, 0x84}, {9, 0x0C},
        {9, 0x12}, {0x0A, 0x38}, {0x0A, 0x10}, {0x0A, 0x31}, {0x0A, 0x34},
        {0x45, 0x04}, {0x45, 0x01}, {0x45, 0x08}};
    for (const auto rejected : REJECTED) {
      const auto before = f.device.diagnostics();
      const auto status = f.device.writeRegisterSync(rejected.reg, rejected.value);
      CHECK(!status.ok() && !status.inProgress());
      CHECK(f.fake.calls == 0 &&
            f.device.configurationSnapshot().state == t::ConfigurationState::KNOWN);
      CHECK(!f.device.diagnostics().configurationWriteUnresolved);
      CHECK(f.device.diagnostics().hardwareEffect == before.hardwareEffect);
      CHECK(sameEvidence(f.device.diagnostics().managedTransport, before.managedTransport));
    }
    if (part.is322) {
      CHECK(f.device.writeRegisterSync(8, 0x80).is(t::Err::UNSUPPORTED_FEATURE));
      CHECK(f.device.writeRegisterSync(9, 0x14).is(t::Err::UNSUPPORTED_FEATURE));
    }
    CHECK(f.device.invalidateDeviceState(t::Status::Ok()).ok());
    CHECK(f.device.writeRegisterSync(0x0A, 0x30).is(t::Err::NOT_INITIALIZED));
    CHECK(f.fake.calls == 0);
  }
}

void rawOrdinaryWrites() {
  struct Write { uint8_t reg; uint8_t value; };
  constexpr Write WRITES[] = {{8, 0x40}, {8, 0x80}, {9, 0x02}, {9, 0x04},
                              {0x0A, 0x70}, {0x0A, 0x30}, {0x45, 0}};
  for (const auto& part : PARTS) {
    for (const auto request : WRITES) {
      if (part.is322 && (request.reg == 8 || request.reg == 9 || request.reg == 0x45)) continue;
      for (const auto error : {t::Err::OK, t::Err::I2C_ERROR, t::Err::I2C_NACK_ADDR,
                              t::Err::I2C_NACK_DATA, t::Err::I2C_TIMEOUT,
                              t::Err::I2C_BUS, t::Err::I2C_BUSY}) {
        Fixture f(part);
        f.initialize();
        f.refresh();
        const auto before = f.device.diagnostics();
        const auto stats = f.device.transportStats();
        f.inspectInvalidation = true;
        auto& step = f.fake.expectWrite(part.address, request.reg, request.value);
        step.status = {error, -456789123, "raw ordinary write"};
        step.applyWrite = true; // Failed writes can still reach the peripheral.
        const auto result = f.device.writeRegisterSync(request.reg, request.value);
        CHECK(sameStatus(result, step.status) && f.fake.calls == 1 && f.fake.complete());
        CHECK(f.fake.registers[request.reg] == request.value);
        const auto after = f.device.diagnostics();
        CHECK(after.initialized && after.identity.identityValid && after.identity.identityMatches);
        CHECK(after.configurationState == t::ConfigurationState::UNKNOWN &&
              after.configurationWriteUnresolved);
        CHECK(after.hardwareEffect == (error == t::Err::OK
            ? t::HardwareEffect::PARTIAL : t::HardwareEffect::INDETERMINATE));
        CHECK(after.lastOperationKind == before.lastOperationKind &&
              sameStatus(after.lastOperationStatus, before.lastOperationStatus));
        CHECK(sameEvidence(after.managedTransport, before.managedTransport));
        CHECK(f.device.transportStats().totalAttempts == stats.totalAttempts + 1);
        CHECK(f.device.transportStats().totalSuccess ==
              stats.totalSuccess + (error == t::Err::OK ? 1U : 0U));
        CHECK(f.device.transportStats().totalFailures ==
              stats.totalFailures + (error == t::Err::OK ? 0U : 1U));
        CHECK(health(f.device) == t::DriverState::READY);
        CHECK(f.device.writeRegisterSync(request.reg, request.value)
                  .is(t::Err::CONFIGURATION_UNKNOWN));
        CHECK(f.fake.calls == 1); // No hidden read to repair the missing safe image.
        f.device.clearTransportStats();
        CHECK(f.device.transportStats().totalAttempts == 0 &&
              f.device.transportStats().lastStatus.ok());
        CHECK(f.device.diagnostics().configurationWriteUnresolved &&
              f.device.diagnostics().hardwareEffect == after.hardwareEffect);
        CHECK(sameEvidence(f.device.diagnostics().managedTransport, after.managedTransport));
        CHECK(f.device.portSnapshot().sequence == f.historical.sequence &&
              f.device.portSnapshot().invalidated);
      }
    }
  }
}

void rawAckUsesOnlyActualControls() {
  constexpr t::DrpDutyCycle DUTY_CODES[] = {t::DrpDutyCycle::PERCENT_30,
      t::DrpDutyCycle::PERCENT_40, t::DrpDutyCycle::PERCENT_50, t::DrpDutyCycle::PERCENT_60};
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    const unsigned controlsCount = part.base ? 4U : 8U;
    for (unsigned code = 0; code < controlsCount; ++code) {
      const uint8_t controls = static_cast<uint8_t>(part.base ? code * 2 : code);
      for (const auto error : {t::Err::OK, t::Err::I2C_BUSY}) {
        Fixture f(part);
        // Nonzero live/IRQ bits are observation, never a write image.
        f.initialize(0x80, static_cast<uint8_t>(0xF0 | controls));
        f.refresh(static_cast<uint8_t>(0x90 | controls));
        const auto actual = f.device.configurationSnapshot();
        auto staged = actual.verifiedProfile;
        const unsigned changedDutyCode = ((controls & 0x06U) / 2U) ^ 1U;
        staged.drpDutyCycle = DUTY_CODES[changedDutyCode];
        if (!part.base) staged.disableUfpAccessory = !staged.disableUfpAccessory;
        t::OperationToken token{};
        CHECK(f.device.startApplyProfile(staged, {100, 1000}, token).inProgress());
        CHECK(token.valid() && f.fake.calls == 0);
        CHECK(f.device.cancel(token).ok());
        const auto cancelled = f.take(token);
        CHECK(cancelled.status.is(t::Err::CANCELLED) &&
              cancelled.hardwareEffect == t::HardwareEffect::NONE && cancelled.transfers == 0);
        CHECK(f.fake.calls == 0);
        const auto configuration = f.device.configurationSnapshot();
        CHECK(configuration.state == t::ConfigurationState::KNOWN);
        CHECK(configuration.hasDesiredProfile &&
              configuration.desiredProfile.drpDutyCycle == staged.drpDutyCycle &&
              configuration.desiredProfile.drpDutyCycle != actual.verifiedProfile.drpDutyCycle);
        CHECK(configuration.verifiedProfile.drpDutyCycle == actual.verifiedProfile.drpDutyCycle &&
              configuration.rawConfig09 == actual.rawConfig09 &&
              configuration.verifiedGeneration == actual.verifiedGeneration);
        CHECK(configuration.desiredProfile.disableUfpAccessory == staged.disableUfpAccessory &&
              configuration.verifiedProfile.disableUfpAccessory ==
              actual.verifiedProfile.disableUfpAccessory);
        const auto before = f.device.diagnostics();
        const uint8_t command = static_cast<uint8_t>(controls | 0x10);
        const uint8_t desiredCommand = static_cast<uint8_t>(command ^ (part.base ? 0x02 : 0x03));
        CHECK(f.device.writeRegisterSync(9, desiredCommand).is(t::Err::UNSUPPORTED_FEATURE));
        CHECK(!f.device.writeRegisterSync(9, static_cast<uint8_t>(command | 0x80)).ok());
        CHECK(f.fake.calls == 0);
        auto& step = f.fake.expectWrite(part.address, 9, command);
        step.status = {error, -3456789, "raw ACK"};
        CHECK(sameStatus(f.device.writeRegisterSync(9, command), step.status));
        CHECK(f.fake.complete() && f.fake.calls == 1);
        CHECK(sameEvidence(f.device.diagnostics().managedTransport, before.managedTransport));
        CHECK(f.device.diagnostics().lastOperationKind == before.lastOperationKind &&
              sameStatus(f.device.diagnostics().lastOperationStatus, before.lastOperationStatus));
        CHECK(f.device.configurationSnapshot().state == configuration.state);
        CHECK(!f.device.diagnostics().configurationWriteUnresolved);
        CHECK(f.device.diagnostics().hardwareEffect == (error == t::Err::OK
            ? t::HardwareEffect::CONFIRMED : t::HardwareEffect::INDETERMINATE));
        CHECK(!f.device.portSnapshot().invalidated);
      }
    }
  }
}

void rawReadSafetyEvidence() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    f.refresh();
    const auto before = f.device.diagnostics();
    // Scalar raw identity/revision bytes neither replace I nor revoke it.
    for (const uint8_t reg : {uint8_t{0}, uint8_t{0xA0}}) {
      if (part.base && reg == 0xA0) continue;
      f.fake = Fake{};
      f.fake.expectRead(part.address, reg, 1).rx[0] = 0xFF;
      uint8_t out = 0;
      CHECK(f.device.readRegisterSync(reg, out).ok() && out == 0xFF && f.fake.complete());
      CHECK(f.device.diagnostics().initialized && f.device.diagnostics().identity.identityMatches);
      CHECK(f.device.diagnostics().identity.rawIdentity[0] == before.identity.rawIdentity[0]);
      CHECK(f.device.diagnostics().identity.rawRevision == before.identity.rawRevision);
      CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::KNOWN);
      CHECK(!f.device.portSnapshot().invalidated);
    }
    f.fake = Fake{};
    f.fake.expectRead(part.address, 0x0A, 1).rx[0] = 0x38;
    uint8_t out = 0;
    CHECK(f.device.readRegisterSync(0x0A, out).ok() && out == 0x38 && f.fake.complete());
    const auto pending = f.device.diagnostics();
    CHECK(pending.initialized && pending.identity.identityMatches);
    CHECK(pending.resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(pending.configurationState == t::ConfigurationState::UNKNOWN);
    CHECK(f.device.portSnapshot().invalidated &&
          f.device.portSnapshot().sequence == f.historical.sequence);
    CHECK(sameEvidence(pending.managedTransport, before.managedTransport));
    CHECK(pending.hardwareEffect == before.hardwareEffect &&
          pending.lastOperationKind == before.lastOperationKind);
    CHECK(health(f.device) == t::DriverState::READY);
    f.fake = Fake{};
    f.fake.expectRead(part.address, 0x0A, 1).rx[0] = 0x30;
    CHECK(f.device.readRegisterSync(0x0A, out).ok() && out == 0x30 && f.fake.complete());
    CHECK(f.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    f.device.clearTransportStats();
    CHECK(f.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(sameEvidence(f.device.diagnostics().managedTransport, before.managedTransport));
    f.fake = Fake{};
    const auto token = f.start(&Driver::startReconcile, 200);
    CHECK(f.device.poll(token, 200, 0).inProgress());
    CHECK(f.device.progress().waiting && f.device.progress().readyAtValid &&
          f.device.progress().readyAtMs == 296);
    CHECK(f.device.poll(token, 295, 0).inProgress() && f.fake.calls == 0);
    CHECK(f.device.poll(token, 296, 0).inProgress() && f.fake.calls == 0);
    f.proof();
    CHECK(f.finish(token, 296).ok() && f.fake.complete());
    CHECK(f.take(token).configuration.state == t::ConfigurationState::KNOWN);
    CHECK(f.device.diagnostics().resetState == t::ResetState::COMPLETED);
  }
}

void preservedRawRepresentations() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    Fixture f(part);
    f.initialize(0x80, 0x14, 0x30, 4);
    CHECK(f.device.writeRegisterSync(0x45, 0).is(t::Err::UNSUPPORTED_FEATURE));
    CHECK(f.fake.calls == 0);
    f.fake.expectWrite(part.address, 0x45, 4);
    CHECK(f.device.writeRegisterSync(0x45, 4).ok() && f.fake.complete());
    CHECK(f.device.diagnostics().configurationWriteUnresolved);
    CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    CHECK(f.device.diagnostics().hardwareEffect == t::HardwareEffect::PARTIAL);
  }
  Fixture f(PARTS[6]);
  f.initialize(0x80, 0x14, 0x02);
  CHECK(f.device.writeRegisterSync(0x0A, 0x32).is(t::Err::UNSUPPORTED_FEATURE));
  CHECK(f.fake.calls == 0);
  f.fake.expectWrite(f.part.address, 0x0A, 0x42);
  CHECK(f.device.writeRegisterSync(0x0A, 0x42).ok() && f.fake.complete());
  CHECK(f.device.diagnostics().hardwareEffect == t::HardwareEffect::PARTIAL);
}

void protected45EvidenceIsIndependent() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (const uint8_t internal : {uint8_t{1}, uint8_t{2}, uint8_t{3}}) {
      Fixture f(part);
      f.initialize();
      const auto before = f.device.configurationSnapshot();
      f.fake.expectRead(part.address, 0x45, 1).rx[0] = internal;
      uint8_t out = 0;
      CHECK(f.device.readRegisterSync(0x45, out).ok() && out == internal && f.fake.complete());
      CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::KNOWN);
      CHECK(f.device.configurationSnapshot().rawConfig45 == before.rawConfig45);
      CHECK(f.device.writeRegisterSync(0x45, 0).is(t::Err::CONFIGURATION_UNKNOWN));
      CHECK(f.fake.calls == 1);
      f.fake = Fake{};
      f.fake.expectRead(part.address, 0x45, 1).rx[0] = 0;
      CHECK(f.device.readRegisterSync(0x45, out).ok() && out == 0 && f.fake.complete());
      CHECK(f.device.writeRegisterSync(0x45, 0).is(t::Err::CONFIGURATION_UNKNOWN));
      CHECK(f.fake.calls == 1); // A raw byte cannot renew the qualified write image.
      auto requested = before.verifiedProfile;
      requested.disableRdRp = true;
      t::OperationToken rejected{77};
      const auto admission = f.device.startApplyProfile(requested, {100, 1000}, rejected);
      CHECK(admission.is(t::Err::CONFIGURATION_UNKNOWN));
      CHECK(rejected.value == 77 && f.fake.calls == 1);
      if (admission.inProgress()) {
        CHECK(f.device.cancel(rejected).ok());
        CHECK(f.take(rejected).hardwareEffect == t::HardwareEffect::NONE);
      }
      requested.disableRdRp = false;
      requested.sourceCurrent = t::SourceCurrent::MA_1500;
      t::OperationToken allowed{};
      CHECK(f.device.startApplyProfile(requested, {100, 1000}, allowed).inProgress());
      CHECK(allowed.valid() && f.fake.calls == 1);
      CHECK(f.device.cancel(allowed).ok());
      CHECK(f.take(allowed).hardwareEffect == t::HardwareEffect::NONE);
      CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::KNOWN);
      f.fake = Fake{};
      f.fake.expectWrite(part.address, 8, 0x40);
      CHECK(f.device.writeRegisterSync(8, 0x40).ok() && f.fake.complete());
      f.fake = Fake{};
      f.proof(0x40);
      const auto reconciled = f.start(&Driver::startReconcile);
      CHECK(f.finish(reconciled).ok() && f.fake.complete());
      CHECK(f.take(reconciled).configuration.state == t::ConfigurationState::KNOWN);
      f.fake = Fake{};
      f.fake.expectWrite(part.address, 0x45, 0);
      CHECK(f.device.writeRegisterSync(0x45, 0).ok() && f.fake.complete());
    }
    Fixture f(part);
    f.initialize(0x80, 0x14, 0x30, 1);
    CHECK(f.device.writeRegisterSync(0x45, 1).is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(f.fake.calls == 0);
    f.fake.expectWrite(part.address, 8, 0x40);
    CHECK(f.device.writeRegisterSync(8, 0x40).ok() && f.fake.complete());
  }
}

void rawStatusPrecedence() {
  Driver unbound;
  uint8_t out = 0xD3;
  CHECK(unbound.writeRegisterSync(0x0A, 0x08).is(t::Err::UNSUPPORTED_FEATURE));
  CHECK(unbound.writeRegisterSync(0x08, 0xC0).is(t::Err::INVALID_PARAM));
  CHECK(unbound.writeRegisterSync(0x09, 0x80).is(t::Err::INVALID_PARAM));
  CHECK(unbound.writeRegisterSync(0x45, 0xF8).is(t::Err::INVALID_PARAM));
  CHECK(unbound.writeRegisterSync(0x0A, 0x30).is(t::Err::NOT_BOUND));
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    struct Case { uint8_t reg; uint8_t value; t::Err error; };
    const Case cases[] = {
        {0, 0x00, t::Err::UNSUPPORTED_FEATURE},
        {0xA0, 0x00, t::Err::UNSUPPORTED_FEATURE},
        {0xFF, 0x00, t::Err::INVALID_PARAM},
        {8, 0x81, part.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM},
        {8, 0xC0, part.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM},
        {9, 0x80, part.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM},
        {9, 0x08, part.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM},
        {0x0A, 0x08, t::Err::UNSUPPORTED_FEATURE},
        {0x0A, 0x04, t::Err::INVALID_PARAM},
        {0x45, 0xF8, part.is322 ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM},
    };
    for (const auto item : cases) {
      CHECK(f.device.writeRegisterSync(item.reg, item.value).is(item.error));
    }
    CHECK(f.device.writeRegisterSync(0x0A, 0x10).is(t::Err::UNSUPPORTED_FEATURE));
    CHECK(f.device.writeRegisterSync(0x0A, 0x31)
              .is(part.base ? t::Err::INVALID_PARAM : t::Err::UNSUPPORTED_FEATURE));
    if (!part.is322) {
      CHECK(f.device.writeRegisterSync(9, 0x12).is(t::Err::UNSUPPORTED_FEATURE));
      CHECK(f.device.writeRegisterSync(0x45, 0x04).is(t::Err::UNSUPPORTED_FEATURE));
      CHECK(f.device.writeRegisterSync(0x45, 0x01).is(t::Err::INVALID_PARAM));
    }
    auto token = f.start(&Driver::startReadProfile);
    for (const auto item : cases) {
      CHECK(f.device.writeRegisterSync(item.reg, item.value).is(item.error));
    }
    CHECK(f.device.writeRegisterSync(0x0A, 0x30).is(t::Err::BUSY));
    CHECK(f.device.readRegisterSync(8, out).is(t::Err::BUSY) && out == 0xD3);
    CHECK(f.device.cancel(token).ok());
    for (const auto item : cases) {
      CHECK(f.device.writeRegisterSync(item.reg, item.value).is(item.error));
    }
    CHECK(f.device.writeRegisterSync(0x0A, 0x30).is(t::Err::RESULT_PENDING));
    CHECK(f.take(token).status.is(t::Err::CANCELLED));
    CHECK(f.fake.calls == 0 && f.fake.complete());
  }
}

void writeCallbackNormalization() {
  for (unsigned code = 0; code < 256; ++code) {
    Fixture f;
    f.initialize();
    const auto managed = f.device.diagnostics().managedTransport;
    f.device.clearTransportStats();
    auto& step = f.fake.expectWrite(0x47, 8, 0x40);
    step.status = {static_cast<t::Err>(code), std::numeric_limits<int32_t>::min() +
                  static_cast<int32_t>(code), "raw write callback code"};
    const auto result = f.device.writeRegisterSync(8, 0x40);
    const bool legal = code == 0 || (code >= 17 && code <= 22);
    CHECK(result.code == (legal ? static_cast<t::Err>(code) : t::Err::I2C_ERROR));
    CHECK(result.detail == step.status.detail);
    CHECK(f.fake.calls == 1 && f.fake.complete());
    const auto stats = f.device.transportStats();
    CHECK(stats.totalAttempts == 1 && stats.totalSuccess == (code == 0 ? 1U : 0U) &&
          stats.totalFailures == (code == 0 ? 0U : 1U));
    CHECK(sameStatus(stats.lastStatus, result));
    CHECK(sameEvidence(f.device.diagnostics().managedTransport, managed));
    CHECK(f.device.diagnostics().configurationWriteUnresolved);
    CHECK(f.device.diagnostics().hardwareEffect == (code == 0
        ? t::HardwareEffect::PARTIAL : t::HardwareEffect::INDETERMINATE));
    if (code != 0) {
      CHECK(sameStatus(f.device.diagnostics().lastFailedTransportStatus, result));
      CHECK(!f.device.diagnostics().lastFailedTransportTimeValid);
    }
  }
}

void unresolvedRecoverySurvivesConsumption() {
  for (unsigned partIndex = 0; partIndex < 2; ++partIndex) {
    Fixture f(PARTS[partIndex]);
    f.initialize();
    f.refresh();
    f.fake.expectRead(f.part.address, 0x45, 1).rx[0] = 0;
    auto& write = f.fake.expectWrite(f.part.address, 0x45, 0x04);
    write.status = {t::Err::I2C_TIMEOUT, -1876543210, "uncertain recovery disable"};
    write.applyWrite = true;
    const auto token = f.start(&Driver::startRecoverDeadBattery);
    CHECK(f.finish(token).is(t::Err::I2C_TIMEOUT) && f.fake.complete());
    const auto result = f.take(token);
    CHECK(result.status.detail == -1876543210 &&
          result.hardwareEffect == t::HardwareEffect::INDETERMINATE);
    auto evidence = f.device.diagnostics();
    CHECK(evidence.recoverySequenceUnresolved && evidence.configurationWriteUnresolved);
    CHECK(evidence.lastOperationKind == t::OperationKind::RECOVER_DEAD_BATTERY);
    CHECK(evidence.hardwareEffect == t::HardwareEffect::INDETERMINATE);
    CHECK(f.device.progress().state == t::OperationState::IDLE);
    f.device.clearTransportStats();
    CHECK(f.device.diagnostics().recoverySequenceUnresolved &&
          f.device.diagnostics().configurationWriteUnresolved);
    CHECK(sameStatus(f.device.diagnostics().lastOperationStatus, result.status));
    CHECK(sameStatus(f.device.diagnostics().lastFailedTransportStatus, result.status));
    CHECK(sameEvidence(f.device.diagnostics().managedTransport, evidence.managedTransport));
    CHECK(f.device.portSnapshot().invalidated &&
          f.device.portSnapshot().sequence == f.historical.sequence);
    // A successful later read-only result cannot certify the historical sequence.
    f.refresh(0);
    evidence = f.device.diagnostics();
    CHECK(evidence.hardwareEffect == t::HardwareEffect::NONE);
    CHECK(evidence.lastOperationKind == t::OperationKind::REFRESH_PORT);
    CHECK(evidence.recoverySequenceUnresolved && evidence.configurationWriteUnresolved);
    CHECK(evidence.lastFailedTransportStatus.detail == -1876543210);
    CHECK(health(f.device) == t::DriverState::READY);
    f.device.clearTransportStats();
    CHECK(f.device.diagnostics().recoverySequenceUnresolved &&
          f.device.diagnostics().configurationWriteUnresolved);
    CHECK(f.device.end().ok() && f.device.diagnostics().recoverySequenceUnresolved);
    CHECK(health(f.device) == t::DriverState::UNINIT);
    CHECK(f.device.bind(f.config).ok() && f.device.diagnostics().recoverySequenceUnresolved);
  }
}

void settledResetAllowsOrdinaryRawWrite() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    f.fake.expectRead(part.address, 0x0A, 1).rx[0] = 0x30;
    f.fake.expectWrite(part.address, 0x0A, 0x38).status =
        {t::Err::I2C_BUSY, -345678901, "reset acceptance unknown"};
    auto token = f.start(&Driver::startReset);
    CHECK(f.finish(token).is(t::Err::I2C_BUSY) && f.fake.complete());
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::INDETERMINATE);
    CHECK(f.device.diagnostics().resetAcceptanceUncertain);
    f.fake = Fake{};
    f.proof();
    token = f.start(&Driver::startReconcile, 200);
    CHECK(f.finish(token, 200).ok() && f.fake.complete());
    CHECK(f.take(token).configuration.state == t::ConfigurationState::KNOWN);
    CHECK(f.device.diagnostics().resetState == t::ResetState::COMPLETED);
    CHECK(f.device.diagnostics().resetAcceptanceUncertain);
    f.fake = Fake{};
    f.fake.expectWrite(part.address, 0x0A, 0x70);
    CHECK(f.device.writeRegisterSync(0x0A, 0x70).ok() && f.fake.complete());
    CHECK(f.device.diagnostics().resetAcceptanceUncertain);
    CHECK(f.device.diagnostics().configurationWriteUnresolved);
  }
}

void managedHealthAndClear() {
  for (const auto& part : PARTS) {
    Fixture f(part), other(part);
    CHECK(health(f.device) == t::DriverState::UNINIT);
    f.initialize();
    f.refresh();
    for (unsigned streak = 1; streak <= 4; ++streak) {
      f.managedFailure(-1000000 - static_cast<int32_t>(streak));
      CHECK(f.device.diagnostics().managedTransport.consecutiveFailures == streak);
      CHECK(health(f.device) == (streak < 3 ? t::DriverState::DEGRADED : t::DriverState::OFFLINE));
    }
    CHECK(health(other.device) == t::DriverState::UNINIT && other.fake.calls == 0);
    CHECK(other.device.transportStats().totalAttempts == 0);
    const auto before = f.device.diagnostics();
    const auto port = f.device.portSnapshot();
    f.device.clearTransportStats();
    CHECK(sameEvidence(f.device.diagnostics().managedTransport, before.managedTransport));
    CHECK(sameStatus(f.device.diagnostics().lastFailedTransportStatus,
                     before.lastFailedTransportStatus));
    CHECK(f.device.diagnostics().lastFailedTransportUptimeMs ==
          before.lastFailedTransportUptimeMs);
    CHECK(f.device.diagnostics().lastFailedTransportTimeValid ==
          before.lastFailedTransportTimeValid);
    CHECK(f.device.portSnapshot().sequence == port.sequence &&
          f.device.portSnapshot().observedUptimeMs == port.observedUptimeMs &&
          f.device.portSnapshot().invalidated == port.invalidated);
    CHECK(health(f.device) == t::DriverState::OFFLINE);
    f.fake.expectRead(part.address, 8, 1).rx[0] = 0x40;
    uint8_t out = 0;
    CHECK(f.device.readRegisterSync(8, out).ok() && out == 0x40 && f.fake.complete());
    CHECK(health(f.device) == t::DriverState::OFFLINE);
    f.fake = Fake{};
    f.fake.expectRead(part.address, 8, 1).status =
        {t::Err::I2C_NACK_DATA, -123456789, "raw failure"};
    CHECK(f.device.readRegisterSync(8, out).is(t::Err::I2C_NACK_DATA) && out == 0x40);
    CHECK(!f.device.diagnostics().lastFailedTransportTimeValid &&
          health(f.device) == t::DriverState::OFFLINE);
    f.fake = Fake{};
    f.fake.expectRead(part.address, 9, 1).rx[0] = 0;
    f.fake.expectRead(part.address, 8, 1).rx[0] = 0;
    f.fake.expectRead(part.address, 9, 1).rx[0] = 0;
    const auto token = f.start(&Driver::startRefreshPort);
    const auto active = f.device.progress();
    f.device.clearTransportStats();
    CHECK(f.device.progress().state == active.state &&
          f.device.progress().token.value == token.value);
    CHECK(f.device.poll(token, 101).inProgress());
    CHECK(health(f.device) == t::DriverState::READY);
    CHECK(f.device.diagnostics().managedTransport.consecutiveFailures == 0);
    CHECK(f.finish(token, 102).ok() && f.fake.complete());
    t::OperationResult retained{};
    CHECK(f.device.resultSnapshot(token, retained).ok());
    const auto managed = f.device.diagnostics().managedTransport;
    f.device.clearTransportStats();
    t::OperationResult copied{};
    CHECK(f.device.resultSnapshot(token, copied).ok());
    CHECK(copied.token.value == retained.token.value && copied.transfers == retained.transfers &&
          copied.port.sequence == retained.port.sequence &&
          copied.completedUptimeMs == retained.completedUptimeMs);
    CHECK(sameEvidence(f.device.diagnostics().managedTransport, managed));
    CHECK(f.take(token).port.attachment == t::Attachment::DETACHED);
    CHECK(sameStatus(f.device.diagnostics().lastFailedTransportStatus,
                     {t::Err::I2C_NACK_DATA, -123456789, "raw failure"}));
    CHECK(f.device.end().ok() && health(f.device) == t::DriverState::UNINIT);
    CHECK(sameEvidence(f.device.diagnostics().managedTransport, [&] {
      auto ended = managed; ended.initialized = false; return ended;
    }()));
    CHECK(f.device.bind(f.config).ok() && health(f.device) == t::DriverState::UNINIT);
    CHECK(f.device.invalidateDeviceState({t::Err::I2C_BUS, -99999, "external cause"}).ok());
    CHECK(health(f.device) == t::DriverState::UNINIT &&
          f.device.transportStats().totalAttempts == 0);
  }
}

void failedInitializationAndUnknownTrust() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.fake.expectRead(part.address, 0, 8).status =
        {t::Err::I2C_TIMEOUT, -777777, "failed initialization"};
    auto token = f.start(&Driver::startInitialize);
    CHECK(f.finish(token).is(t::Err::I2C_TIMEOUT));
    CHECK(f.take(token).status.detail == -777777);
    CHECK(health(f.device) == t::DriverState::UNINIT && f.fake.complete());
    CHECK(f.device.diagnostics().managedTransport.totalFailures == 1);
    f.fake = Fake{};
    f.proof();
    f.fake.script[0].rx[0] ^= 1;
    token = f.start(&Driver::startInitialize);
    CHECK(f.finish(token).is(t::Err::CHIP_ID_MISMATCH));
    CHECK(f.take(token).status.is(t::Err::CHIP_ID_MISMATCH));
    CHECK(f.fake.calls == 1 && f.fake.errors == 0);
    CHECK(f.device.diagnostics().managedTransport.totalFailures == 1 &&
          f.device.diagnostics().managedTransport.consecutiveFailures == 0);
    CHECK(health(f.device) == t::DriverState::UNINIT);
    f.fake = Fake{};
    f.proof(0xC0); // Reserved current code; complete I/P still establishes readiness.
    token = f.start(&Driver::startInitialize);
    CHECK(f.finish(token).is(t::Err::CONFIGURATION_UNKNOWN) && f.fake.complete());
    CHECK(f.take(token).configuration.state == t::ConfigurationState::UNKNOWN);
    CHECK(f.device.diagnostics().initialized && health(f.device) == t::DriverState::READY);
    CHECK(f.device.diagnostics().lastFailedTransportStatus.detail == -777777);
  }
}

void saturationWithoutPublicSeeds() {
  Fake fake{};
  t::DriverConfig config{};
  config.variant = t::Variant::TUSB320LAI;
  config.i2cWrite = Fake::write;
  config.i2cWriteRead = Fake::writeRead;
  config.i2cUser = &fake;
  t::TransportStats stats{};
  t::DriverDiagnostics diagnostics{};
  stats.totalAttempts = MAX - 1;
  stats.totalSuccess = MAX - 1;
  stats.totalFailures = MAX - 1;
  auto& managed = diagnostics.managedTransport;
  managed.initialized = true;
  managed.totalSuccess = MAX - 1;
  managed.totalFailures = MAX - 1;
  managed.consecutiveFailures = MAX - 1;
  managed.lastSuccessUptimeMs = 12345678901234ULL;
  managed.lastSuccessTimeValid = true;
  managed.lastFailureUptimeMs = 23456789012345ULL;
  managed.lastFailureTimeValid = true;
  diagnostics.lastFailedTransportUptimeMs = 34567890123456ULL;
  diagnostics.lastFailedTransportTimeValid = true;
  const p::ProtocolContext context{config, stats, diagnostics};
  for (unsigned turn = 0; turn < 4; ++turn) {
    auto& step = fake.expectRead(0x47, 8, 1);
    if (turn < 2) step.status = {t::Err::I2C_BUSY, -2000000000, "saturated failure"};
    step.observedStats = &stats;
    step.expectedAttempts = MAX;
    step.expectedSuccess = turn == 3 ? MAX : MAX - 1;
    step.expectedFailures = turn == 0 ? MAX - 1 : MAX;
    uint8_t out = 0xD3;
    const auto result = p::readRegisters(context, p::TrafficSource::MANAGED, 8, &out, 1, 20);
    CHECK(result.callbackInvoked && result.status.ok() == (turn >= 2));
    CHECK(stats.totalAttempts == MAX && stats.totalFailures == MAX);
    CHECK(managed.totalFailures == MAX && managed.consecutiveFailures == (turn < 2 ? MAX : 0));
  }
  CHECK(fake.complete() && stats.totalSuccess == MAX && managed.totalSuccess == MAX);
  CHECK(!managed.lastSuccessTimeValid && !managed.lastFailureTimeValid &&
        !diagnostics.lastFailedTransportTimeValid);
  CHECK(managed.lastSuccessUptimeMs == 12345678901234ULL &&
        managed.lastFailureUptimeMs == 23456789012345ULL);
  CHECK(diagnostics.lastFailedTransportUptimeMs == 34567890123456ULL &&
        diagnostics.lastFailedTransportStatus.detail == -2000000000);
  fake = Fake{};
  diagnostics.lastFailedTransportTimeValid = true;
  managed.lastSuccessTimeValid = true;
  managed.lastFailureTimeValid = true;
  const auto beforeRaw = managed;
  fake.expectRead(0x47, 8, 1).status =
      {t::Err::I2C_TIMEOUT, std::numeric_limits<int32_t>::min(), "untimed raw failure"};
  uint8_t out = 0xD3;
  CHECK(p::readRegisters(context, p::TrafficSource::RAW, 8, &out, 1, 20)
            .status.is(t::Err::I2C_TIMEOUT));
  CHECK(out == 0xD3 && fake.complete());
  CHECK(stats.totalAttempts == MAX && stats.totalFailures == MAX && stats.totalSuccess == MAX);
  CHECK(sameEvidence(managed, beforeRaw));
  CHECK(!diagnostics.lastFailedTransportTimeValid &&
        diagnostics.lastFailedTransportUptimeMs == 34567890123456ULL);
  CHECK(diagnostics.lastFailedTransportStatus.detail == std::numeric_limits<int32_t>::min());
}

}  // namespace

int main() {
  projectionTable();
  rawReadMapAndSlot();
  localEventsDoNotAccount();
  callbackNormalizationAndRawIsolation();
  rawWriteGuards();
  rawOrdinaryWrites();
  rawAckUsesOnlyActualControls();
  rawReadSafetyEvidence();
  preservedRawRepresentations();
  rawStatusPrecedence();
  protected45EvidenceIsIndependent();
  writeCallbackNormalization();
  unresolvedRecoverySurvivesConsumption();
  settledResetAllowsOrdinaryRawWrite();
  managedHealthAndClear();
  failedInitializationAndUnknownTrust();
  saturationWithoutPublicSeeds();
  std::printf("diagnostics: %zu checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
