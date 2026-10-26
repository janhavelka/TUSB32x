/// @file test_protocol.cpp
/// @brief Independent terminal transport bytes, local allowlists, fault and identity regressions.
#include "TUSB32xProtocol.h"
#include "TUSB32xCodecs.h"
#include "support/FakeTusb32xDevice.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

namespace {

namespace t = TUSB32x;
namespace p = TUSB32x::detail;
using Fake = tusb32x_test::FakeTusb32xDevice;

int failures = 0;
std::size_t checks = 0;

void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    std::printf("FAIL line %d: %s\n", line, expression);
    ++failures;
  }
}

// Remains active under NDEBUG, including strict Release builds.
#define CHECK(expression) check((expression), #expression, __LINE__)

struct VariantCase {
  t::Variant variant;
  uint8_t low;
  uint8_t high;
  bool base;
  bool is322;
  uint8_t revision;
};

// TI SLLSEN9F Tables 7-5/7-7 pp15-19; SLLSEP2D and SLLSEQ8D Tables 5/7-12
// pp15-20; SLLSEO5D Tables 6-5/6-7 through 6-11 pp15-20. These literals are
// independent expected values, not expressions derived from production helpers.
constexpr VariantCase VARIANTS[] = {
    {t::Variant::TUSB320, 0x60, 0x61, true, false, 0},
    {t::Variant::TUSB320I, 0x60, 0x61, true, false, 0},
    {t::Variant::TUSB320LI, 0x47, 0x67, false, false, 0x01},
    {t::Variant::TUSB320HI, 0x47, 0x67, false, false, 0x01},
    {t::Variant::TUSB320LAI, 0x47, 0x67, false, false, 0x02},
    {t::Variant::TUSB320HAI, 0x47, 0x67, false, false, 0x02},
    {t::Variant::TUSB322I, 0x47, 0x67, false, true, 0x02},
};

struct Fixture {
  Fake fake{};
  t::DriverConfig config{};
  t::TransportStats stats{};
  t::DriverDiagnostics diagnostics{};

  explicit Fixture(const VariantCase& variant = VARIANTS[4]) {
    config.i2cWrite = Fake::write;
    config.i2cWriteRead = Fake::writeRead;
    config.i2cUser = &fake;
    config.variant = variant.variant;
    config.address = static_cast<t::DeviceAddress>(variant.low);
  }

  p::ProtocolContext context() { return {config, stats, diagnostics}; }
};

bool sameStatus(const t::Status& left, const t::Status& right) {
  return left.code == right.code && left.detail == right.detail && left.msg == right.msg;
}

void checkLocal(const Fixture& f, const p::TransferResult& result, t::Err expected) {
  CHECK(result.status.code == expected);
  CHECK(result.status.detail == 0);
  CHECK(result.status.msg != nullptr);
  CHECK(!result.callbackInvoked);
  CHECK(result.writeAcceptance == p::WriteAcceptance::NOT_ATTEMPTED);
  CHECK(f.fake.complete());
  CHECK(f.stats.totalAttempts == 0);
  CHECK(f.stats.totalSuccess == 0);
  CHECK(f.stats.totalFailures == 0);
  CHECK(f.stats.lastStatus.code == t::Err::OK);
  CHECK(f.diagnostics.lastFailedTransportStatus.code == t::Err::OK);
  CHECK(f.diagnostics.managedTransport.totalSuccess == 0);
  CHECK(f.diagnostics.managedTransport.totalFailures == 0);
}

void checkSuccess(const Fixture& f, const p::TransferResult& result, bool write) {
  CHECK(result.status.ok());
  CHECK(result.callbackInvoked);
  CHECK(result.writeAcceptance == (write ? p::WriteAcceptance::CALLBACK_SUCCEEDED
                                       : p::WriteAcceptance::NOT_ATTEMPTED));
  CHECK(f.fake.complete());
  CHECK(f.stats.totalAttempts == 1);
  CHECK(f.stats.totalSuccess == 1);
  CHECK(f.stats.totalFailures == 0);
}

void addressesAndBindings() {
  for (const auto& variant : VARIANTS) {
    // Includes wire address bytes C0/C1/C2/C3/8E/8F/CE/CF and register45-as-address.
    for (unsigned int address = 0; address <= 255; ++address) {
      Fixture f(variant);
      f.config.address = static_cast<t::DeviceAddress>(address);
      uint8_t output = 0xD3;
      const bool legal = address == variant.low || address == variant.high;
      if (legal) f.fake.expectRead(static_cast<uint8_t>(address), 0x08, 1).rx[0] = 0x53;
      const auto result = p::readRegisters(f.context(), p::TrafficSource::MANAGED,
                                           0x08, &output, 1, 20);
      if (legal) {
        checkSuccess(f, result, false);
        CHECK(output == 0x53);
      } else {
        checkLocal(f, result, t::Err::INVALID_CONFIG);
        CHECK(output == 0xD3);
      }

      Fixture w(variant);
      w.config.address = static_cast<t::DeviceAddress>(address);
      if (legal) w.fake.expectWrite(static_cast<uint8_t>(address), 0x0A, 0x30);
      const auto written = p::writeRegister(w.context(), p::TrafficSource::RAW, 0x0A, 0x30, 20);
      if (legal) checkSuccess(w, written, true);
      else checkLocal(w, written, t::Err::INVALID_CONFIG);
    }
  }
  for (unsigned int value = 0; value <= 255; ++value) {
    bool valid = false;
    for (const auto& variant : VARIANTS) {
      if (static_cast<unsigned int>(variant.variant) == value) valid = true;
    }
    if (valid) continue;
    Fixture f;
    f.config.variant = static_cast<t::Variant>(value);
    uint8_t output = 0xD3;
    checkLocal(f, p::readRegisters(f.context(), p::TrafficSource::RAW, 0x08, &output, 1, 20),
               t::Err::INVALID_CONFIG);
    checkLocal(f, p::writeRegister(f.context(), p::TrafficSource::RAW, 0x0A, 0, 20),
               t::Err::INVALID_CONFIG);
    CHECK(output == 0xD3);
  }
  for (unsigned int missing = 1; missing <= 3; ++missing) {
    Fixture f;
    if ((missing & 1U) != 0) f.config.i2cWrite = nullptr;
    if ((missing & 2U) != 0) f.config.i2cWriteRead = nullptr;
    uint8_t output = 0xD3;
    checkLocal(f, p::readRegisters(f.context(), p::TrafficSource::RAW, 0x08, &output, 1, 20),
               t::Err::INVALID_CONFIG);
    checkLocal(f, p::writeRegister(f.context(), p::TrafficSource::RAW, 0x0A, 0, 20),
               t::Err::INVALID_CONFIG);
    CHECK(output == 0xD3);
  }
  Fixture f;
  f.config.i2cTimeoutMs = 0;
  uint8_t output = 0xD3;
  checkLocal(f, p::readRegisters(f.context(), p::TrafficSource::RAW, 0x08, &output, 1, 20),
             t::Err::INVALID_CONFIG);
  checkLocal(f, p::writeRegister(f.context(), p::TrafficSource::RAW, 0x0A, 0, 20),
             t::Err::INVALID_CONFIG);
  CHECK(output == 0xD3);
}

void accessAndRanges() {
  for (const auto& variant : VARIANTS) {
    for (unsigned int reg = 0; reg <= 255; ++reg) {
      Fixture r(variant);
      uint8_t output = 0xD3;
      const auto offset = static_cast<uint8_t>(reg);
      const bool readable = reg <= 0x0A || (reg == 0x45 && !variant.is322) ||
                            (reg == 0xA0 && !variant.base);
      if (readable) r.fake.expectRead(variant.low, offset, 1).rx[0] = 0x69;
      const auto read =
          p::readRegisters(r.context(), p::TrafficSource::RAW, offset, &output, 1, 20);
      if (readable) {
        checkSuccess(r, read, false);
        CHECK(output == 0x69);
      } else {
        const bool absent = reg == 0x45 || reg == 0xA0;
        checkLocal(r, read, absent ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM);
        CHECK(output == 0xD3);
      }

      Fixture w(variant);
      const bool writable = reg == 0x0A || ((reg == 0x08 || reg == 0x09 || reg == 0x45) &&
                                           !variant.is322);
      const uint8_t writeByte = reg == 0x08 ? 0x80 : reg == 0x09 ? 0x06 : reg == 0x45 ? 0x04 : 0x30;
      if (writable) w.fake.expectWrite(variant.low, offset, writeByte);
      const auto write =
          p::writeRegister(w.context(), p::TrafficSource::MANAGED, offset, writeByte, 20);
      if (writable) checkSuccess(w, write, true);
      else {
        const bool gated = (reg == 0x45 && variant.is322) ||
                           ((reg == 0x08 || reg == 0x09) && variant.is322) ||
                           (reg == 0xA0 && variant.base);
        checkLocal(w, write, gated ? t::Err::UNSUPPORTED_FEATURE : t::Err::INVALID_PARAM);
      }
    }

    // Every start/length boundary inside 00..0A, including the easy-to-reject valid 07+4.
    for (uint8_t reg = 0; reg <= 0x0A; ++reg) {
      for (std::size_t length = 1; length <= 12; ++length) {
        Fixture f(variant);
        uint8_t output[12];
        std::memset(output, 0xD3, sizeof(output));
        const bool legal = length <= static_cast<std::size_t>(0x0B - reg);
        if (legal) {
          auto& step = f.fake.expectRead(variant.low, reg, length);
          step.callerOutput = output;
          for (std::size_t i = 0; i < length; ++i) step.rx[i] = static_cast<uint8_t>(0x70 + i);
        }
        const auto result = p::readRegisters(f.context(), p::TrafficSource::MANAGED,
                                             reg, output, length, 20);
        if (legal) checkSuccess(f, result, false);
        else checkLocal(f, result, t::Err::INVALID_PARAM);
        for (std::size_t i = 0; i < sizeof(output); ++i) {
          CHECK(output[i] == (legal && i < length ? static_cast<uint8_t>(0x70 + i) : 0xD3));
        }
      }
    }
    constexpr uint8_t SPECIAL_REGS[] = {0x45, 0xA0, 0xFF};
    constexpr std::size_t BAD_LENGTHS[] = {0, 2, 11, 12, 255, 256,
                                          std::numeric_limits<std::size_t>::max()};
    for (const auto reg : SPECIAL_REGS) {
      for (const auto length : BAD_LENGTHS) {
        Fixture f(variant);
        uint8_t output[12];
        std::memset(output, 0xD3, sizeof(output));
        const auto result = p::readRegisters(f.context(), p::TrafficSource::RAW,
                                             reg, output, length, 20);
        CHECK(!result.status.ok());
        CHECK(!result.callbackInvoked);
        CHECK(f.fake.complete());
        CHECK(f.stats.totalAttempts == 0);
        for (const auto value : output) CHECK(value == 0xD3);
      }
    }
  }
  constexpr std::size_t BAD_LENGTHS[] = {0, 12, 256, std::numeric_limits<std::size_t>::max()};
  for (const auto length : BAD_LENGTHS) {
    Fixture f;
    uint8_t output = 0xD3;
    checkLocal(f, p::readRegisters(f.context(), p::TrafficSource::RAW, 0, &output, length, 20),
               t::Err::INVALID_PARAM);
    CHECK(output == 0xD3);
  }
  Fixture f;
  checkLocal(f, p::readRegisters(f.context(), p::TrafficSource::RAW, 0, nullptr, 1, 20),
             t::Err::INVALID_PARAM);
  uint8_t output = 0xD3;
  checkLocal(f, p::readRegisters(f.context(), p::TrafficSource::RAW, 0, &output, 1, 0),
             t::Err::INVALID_PARAM);
  checkLocal(f, p::writeRegister(f.context(), p::TrafficSource::RAW, 0x0A, 0, 0),
             t::Err::INVALID_PARAM);
  for (unsigned int value = 2; value <= 255; ++value) {
    const auto source = static_cast<p::TrafficSource>(value);
    checkLocal(f, p::readRegisters(f.context(), source, 0, &output, 1, 20), t::Err::INVALID_PARAM);
    checkLocal(f, p::writeRegister(f.context(), source, 0x0A, 0, 20), t::Err::INVALID_PARAM);
  }
  CHECK(output == 0xD3);
}

void framingTimeoutsAndScratch() {
  Fixture f;
  uint8_t output[4] = {};
  auto& first = f.fake.expectRead(0x47, 0x07, 4);
  first.rx[0] = 0;
  first.rx[1] = 0xA1;
  first.rx[2] = 0xB2;
  first.rx[3] = 0xC3;
  first.callerOutput = output;
  f.fake.expectWrite(0x47, 0x0A, 0x36);
  auto& second = f.fake.expectRead(0x47, 0x07, 4);
  second.rx[0] = 0;
  second.rx[1] = 0xD4;
  second.rx[2] = 0xE5;
  second.rx[3] = 0xF6;
  second.callerOutput = output;
  CHECK(p::readRegisters(f.context(), p::TrafficSource::RAW, 0x07, output, 4, 20).status.ok());
  CHECK(output[0] == 0 && output[1] == 0xA1 && output[2] == 0xB2 && output[3] == 0xC3);
  CHECK(p::writeRegister(f.context(), p::TrafficSource::RAW, 0x0A, 0x36, 20).status.ok());
  CHECK(p::readRegisters(f.context(), p::TrafficSource::RAW, 0x07, output, 4, 20).status.ok());
  CHECK(output[0] == 0 && output[1] == 0xD4 && output[2] == 0xE5 && output[3] == 0xF6);
  CHECK(f.fake.complete());
  CHECK(f.stats.totalAttempts == 3);

  // Framing only: arbitrary bytes remain unchanged. This private primitive supplies no composer,
  // identity or silicon-procedure permission; its serialized caller owns those prerequisites.
  constexpr uint8_t PRECOMPOSED_BYTES[] = {0x00, 0x08, 0x80, 0xFF};
  for (const auto value : PRECOMPOSED_BYTES) {
    Fixture w;
    w.fake.expectWrite(0x47, 0x0A, value);
    checkSuccess(w, p::writeRegister(w.context(), p::TrafficSource::RAW, 0x0A, value, 20), true);
  }

  struct TimeoutCase { uint32_t cap; uint64_t remaining; uint32_t expected; };
  constexpr auto MAX32 = std::numeric_limits<uint32_t>::max();
  constexpr TimeoutCase TIMEOUTS[] = {
      {20, 1, 1}, {20, 19, 19}, {20, 20, 20}, {20, 21, 20},
      {20, uint64_t{1} << 32U, 20}, {20, std::numeric_limits<uint64_t>::max(), 20},
      {MAX32, MAX32, MAX32}, {MAX32, uint64_t{1} << 32U, MAX32},
      {MAX32, std::numeric_limits<uint64_t>::max(), MAX32}, {1, 100, 1}};
  for (const auto& timeout : TIMEOUTS) {
    Fixture r;
    r.config.i2cTimeoutMs = timeout.cap;
    r.fake.expectRead(0x47, 0x08, 1, timeout.expected).rx[0] = 0x61;
    uint8_t byte = 0;
    checkSuccess(r, p::readRegisters(r.context(), p::TrafficSource::RAW, 0x08, &byte, 1,
                                    timeout.remaining), false);
    CHECK(byte == 0x61);
    Fixture w;
    w.config.i2cTimeoutMs = timeout.cap;
    w.fake.expectWrite(0x47, 0x0A, 0x36, timeout.expected);
    checkSuccess(w, p::writeRegister(w.context(), p::TrafficSource::RAW, 0x0A, 0x36,
                                     timeout.remaining), true);
  }
}

unsigned int nullContextCalls = 0;

t::Status nullContextWrite(uint8_t address, const uint8_t* bytes, std::size_t length,
                            uint32_t timeoutMs, void* user) {
  ++nullContextCalls;
  CHECK(user == nullptr);
  CHECK(address == 0x47 && bytes != nullptr && length == 2 && timeoutMs == 20);
  if (bytes != nullptr && length == 2) CHECK(bytes[0] == 0x0A && bytes[1] == 0x36);
  return t::Status::Ok();
}

t::Status nullContextRead(uint8_t address, const uint8_t* tx, std::size_t txLength,
                           uint8_t* rx, std::size_t rxLength, uint32_t timeoutMs, void* user) {
  ++nullContextCalls;
  CHECK(user == nullptr);
  CHECK(address == 0x47 && tx != nullptr && txLength == 1 && timeoutMs == 20);
  CHECK(rx != nullptr && rxLength == 1);
  if (tx != nullptr && txLength == 1) CHECK(tx[0] == 0x08);
  if (rx != nullptr && rxLength == 1) rx[0] = 0x53;
  return t::Status::Ok();
}

void validNullContext() {
  Fixture f;
  f.config.i2cUser = nullptr;
  f.config.i2cWrite = nullContextWrite;
  f.config.i2cWriteRead = nullContextRead;
  uint8_t output = 0;
  CHECK(p::readRegisters(f.context(), p::TrafficSource::RAW, 0x08, &output, 1, 20).status.ok());
  CHECK(p::writeRegister(f.context(), p::TrafficSource::MANAGED, 0x0A, 0x36, 20).status.ok());
  CHECK(output == 0x53);
  CHECK(nullContextCalls == 2);
  CHECK(f.stats.totalAttempts == 2 && f.stats.totalSuccess == 2 && f.stats.totalFailures == 0);
  CHECK(f.diagnostics.managedTransport.totalSuccess == 1);
}

void normalizedStatusesAndAcceptance() {
  // Exhaust every uint8 Err representation, including owner/validation statuses and invalid casts.
  for (unsigned int value = 0; value <= 255; ++value) {
    const auto code = static_cast<t::Err>(value);
    const bool legal = code == t::Err::OK || code == t::Err::I2C_ERROR ||
        code == t::Err::I2C_NACK_ADDR || code == t::Err::I2C_NACK_DATA ||
        code == t::Err::I2C_TIMEOUT || code == t::Err::I2C_BUS || code == t::Err::I2C_BUSY;
    const auto expected = legal ? code : t::Err::I2C_ERROR;
    const int32_t detail = (value & 1U) != 0 ? std::numeric_limits<int32_t>::min()
                                           : std::numeric_limits<int32_t>::max();
    constexpr const char* MESSAGE = "backend terminal evidence";
    for (unsigned int applied = 0; applied <= 1; ++applied) {
      Fixture w;
      auto& step = w.fake.expectWrite(0x47, 0x0A, 0x36);
      step.status = {code, detail, MESSAGE};
      step.applyWrite = applied != 0;
      step.observedStats = &w.stats;
      step.expectedAttempts = 1;
      const auto result = p::writeRegister(w.context(), p::TrafficSource::MANAGED, 0x0A, 0x36, 20);
      CHECK(result.callbackInvoked);
      CHECK(result.status.code == expected);
      CHECK(result.status.detail == detail);
      if (legal) CHECK(result.status.msg == MESSAGE);
      else CHECK(result.status.msg != nullptr && result.status.msg[0] != '\0');
      CHECK(result.writeAcceptance == (expected == t::Err::OK
          ? p::WriteAcceptance::CALLBACK_SUCCEEDED : p::WriteAcceptance::POSSIBLE));
      CHECK(w.fake.complete());
      CHECK(w.fake.registers[0x0A] == (applied != 0 ? 0x36 : 0));
      CHECK(w.stats.totalAttempts == 1);
      CHECK(w.stats.totalSuccess == (expected == t::Err::OK ? 1U : 0U));
      CHECK(w.stats.totalFailures == (expected == t::Err::OK ? 0U : 1U));
      CHECK(sameStatus(w.stats.lastStatus, result.status));
      CHECK(sameStatus(w.diagnostics.managedTransport.lastStatus, result.status));
      if (expected != t::Err::OK) CHECK(sameStatus(w.diagnostics.lastFailedTransportStatus,
                                                result.status));
    }
    for (std::size_t filled = 0; filled <= 4; ++filled) {
      Fixture r;
      uint8_t output[] = {0xD3, 0xD3, 0xD3, 0xD3};
      auto& step = r.fake.expectRead(0x47, 0x07, 4);
      step.rx[0] = 0x11;
      step.rx[1] = 0x22;
      step.rx[2] = 0x33;
      step.rx[3] = 0x44;
      step.fillLength = filled;
      // An incomplete adapter result must be a failure; Status alone cannot detect short OK.
      step.status = {code == t::Err::OK && filled != 4 ? t::Err::I2C_ERROR : code, detail, MESSAGE};
      step.callerOutput = output;
      step.observedStats = &r.stats;
      step.expectedAttempts = 1;
      const auto result =
          p::readRegisters(r.context(), p::TrafficSource::MANAGED, 0x07, output, 4, 20);
      const auto readExpected = code == t::Err::OK && filled != 4 ? t::Err::I2C_ERROR : expected;
      CHECK(result.status.code == readExpected);
      CHECK(result.status.detail == detail);
      if (legal) CHECK(result.status.msg == MESSAGE);
      else CHECK(result.status.msg != nullptr && result.status.msg[0] != '\0');
      CHECK(result.callbackInvoked);
      CHECK(result.writeAcceptance == p::WriteAcceptance::NOT_ATTEMPTED);
      CHECK(r.fake.complete());
      for (std::size_t i = 0; i < sizeof(output); ++i) {
        CHECK(output[i] == (readExpected == t::Err::OK ? step.rx[i] : 0xD3));
      }
      CHECK(r.stats.totalAttempts == 1);
      CHECK(r.stats.totalSuccess == (readExpected == t::Err::OK ? 1U : 0U));
      CHECK(r.stats.totalFailures == (readExpected == t::Err::OK ? 0U : 1U));
      CHECK(sameStatus(r.stats.lastStatus, result.status));
    }
  }
}

void accountingHistory() {
  Fixture f;
  f.diagnostics.bound = true;
  f.diagnostics.initialized = true;
  f.diagnostics.configurationState = t::ConfigurationState::UNKNOWN;
  f.diagnostics.hardwareEffect = t::HardwareEffect::INDETERMINATE;
  f.diagnostics.terminationState = t::TerminationState::DISABLED;
  f.diagnostics.resetState = t::ResetState::MAY_BE_PENDING;
  f.diagnostics.configurationWriteUnresolved = true;
  f.diagnostics.recoverySequenceUnresolved = true;
  f.diagnostics.modeTransitionUnresolved = true;
  f.diagnostics.resetAcceptanceUncertain = true;
  f.diagnostics.lastOperationKind = t::OperationKind::RESET;
  f.diagnostics.lastOperationStatus = {t::Err::CANCELLED, -100000, "operation retained"};
  f.diagnostics.lastFailedTransportUptimeMs = 1234567890123ULL;
  f.diagnostics.lastFailedTransportTimeValid = true;
  f.diagnostics.identity.rawIdentity[0] = 0xF3;
  f.diagnostics.identity.rawRevision = 0x06;
  f.diagnostics.interrupt.pendingBefore = true;
  auto& managed = f.diagnostics.managedTransport;
  managed.initialized = true;
  managed.lastSuccessUptimeMs = 400;
  managed.lastSuccessTimeValid = true;
  managed.lastFailureUptimeMs = 500;
  managed.lastFailureTimeValid = true;
  const t::Status failure{t::Err::I2C_TIMEOUT, -7654321, "terminal timeout"};
  const t::Status rawFailure{t::Err::I2C_BUSY, 7654321, "bounded bus lock failure"};
  auto& first = f.fake.expectRead(0x47, 0x08, 1);
  first.status = failure;
  auto& second = f.fake.expectWrite(0x47, 0x0A, 0x36);
  second.status = rawFailure;
  f.fake.expectRead(0x47, 0x08, 1);
  f.fake.expectWrite(0x47, 0x0A, 0x36);
  auto& fifth = f.fake.expectRead(0x47, 0x08, 1);
  fifth.status = failure;
  uint8_t output = 0xD3;
  CHECK(p::readRegisters(f.context(), p::TrafficSource::MANAGED, 0x08, &output, 1, 20)
            .status.code == t::Err::I2C_TIMEOUT);
  CHECK(output == 0xD3);
  CHECK(managed.totalFailures == 1 && managed.consecutiveFailures == 1);
  CHECK(sameStatus(managed.lastStatus, failure));
  CHECK(!managed.lastFailureTimeValid);
  CHECK(managed.lastFailureUptimeMs == 500);
  CHECK(managed.lastSuccessTimeValid && managed.lastSuccessUptimeMs == 400);
  CHECK(!f.diagnostics.lastFailedTransportTimeValid);
  CHECK(f.diagnostics.lastFailedTransportUptimeMs == 1234567890123ULL);

  CHECK(p::writeRegister(f.context(), p::TrafficSource::RAW, 0x0A, 0x36, 20).status.code ==
        t::Err::I2C_BUSY);
  CHECK(managed.totalFailures == 1 && managed.consecutiveFailures == 1);
  CHECK(managed.totalSuccess == 0);
  CHECK(sameStatus(managed.lastStatus, failure));
  CHECK(managed.lastSuccessTimeValid && managed.lastSuccessUptimeMs == 400);
  CHECK(!managed.lastFailureTimeValid && managed.lastFailureUptimeMs == 500);
  CHECK(sameStatus(f.diagnostics.lastFailedTransportStatus, rawFailure));
  CHECK(!f.diagnostics.lastFailedTransportTimeValid);
  CHECK(f.diagnostics.lastFailedTransportUptimeMs == 1234567890123ULL);
  CHECK(p::readRegisters(f.context(), p::TrafficSource::RAW, 0x08, &output, 1, 20).status.ok());
  CHECK(managed.totalFailures == 1 && managed.consecutiveFailures == 1);
  CHECK(managed.totalSuccess == 0);
  CHECK(sameStatus(managed.lastStatus, failure));
  CHECK(managed.lastSuccessTimeValid && managed.lastSuccessUptimeMs == 400);
  CHECK(!managed.lastFailureTimeValid && managed.lastFailureUptimeMs == 500);
  CHECK(sameStatus(f.diagnostics.lastFailedTransportStatus, rawFailure));
  CHECK(!f.diagnostics.lastFailedTransportTimeValid);
  CHECK(f.diagnostics.lastFailedTransportUptimeMs == 1234567890123ULL);

  CHECK(p::writeRegister(f.context(), p::TrafficSource::MANAGED, 0x0A, 0x36, 20).status.ok());
  CHECK(managed.totalSuccess == 1 && managed.totalFailures == 1 &&
        managed.consecutiveFailures == 0);
  CHECK(!managed.lastSuccessTimeValid && managed.lastSuccessUptimeMs == 400);
  CHECK(!managed.lastFailureTimeValid && managed.lastFailureUptimeMs == 500);
  CHECK(sameStatus(f.diagnostics.lastFailedTransportStatus, rawFailure));
  CHECK(p::readRegisters(f.context(), p::TrafficSource::MANAGED, 0x08, &output, 1, 20)
            .status.code == t::Err::I2C_TIMEOUT);
  CHECK(managed.totalSuccess == 1 && managed.totalFailures == 2 &&
        managed.consecutiveFailures == 1);
  CHECK(sameStatus(f.diagnostics.lastFailedTransportStatus, failure));
  CHECK(f.stats.totalAttempts == 5 && f.stats.totalSuccess == 2 && f.stats.totalFailures == 3);
  CHECK(f.fake.complete());
  CHECK(f.diagnostics.bound && f.diagnostics.initialized && managed.initialized);
  CHECK(f.diagnostics.configurationState == t::ConfigurationState::UNKNOWN);
  CHECK(f.diagnostics.hardwareEffect == t::HardwareEffect::INDETERMINATE);
  CHECK(f.diagnostics.terminationState == t::TerminationState::DISABLED);
  CHECK(f.diagnostics.resetState == t::ResetState::MAY_BE_PENDING);
  CHECK(f.diagnostics.configurationWriteUnresolved && f.diagnostics.recoverySequenceUnresolved &&
        f.diagnostics.modeTransitionUnresolved && f.diagnostics.resetAcceptanceUncertain);
  CHECK(f.diagnostics.lastOperationKind == t::OperationKind::RESET);
  CHECK(f.diagnostics.lastOperationStatus.code == t::Err::CANCELLED);
  CHECK(f.diagnostics.lastOperationStatus.detail == -100000);
  CHECK(f.diagnostics.identity.rawIdentity[0] == 0xF3);
  CHECK(f.diagnostics.identity.rawRevision == 0x06);
  CHECK(!f.diagnostics.identity.identityValid && !f.diagnostics.identity.identityMatches);
  CHECK(!f.diagnostics.identity.revisionValid && !f.diagnostics.identity.revisionMatches);
  CHECK(f.diagnostics.interrupt.pendingBefore && !f.diagnostics.interrupt.observationValid);

  Fixture independent;
  CHECK(independent.stats.totalAttempts == 0);
  CHECK(independent.diagnostics.managedTransport.totalFailures == 0);
  independent.fake.expectRead(0x47, 0x08, 1);
  CHECK(p::readRegisters(independent.context(), p::TrafficSource::MANAGED, 0x08, &output, 1, 20)
            .status.ok());
  CHECK(independent.fake.complete());
  CHECK(independent.stats.totalAttempts == 1);
  CHECK(independent.diagnostics.managedTransport.totalSuccess == 1);
  CHECK(f.stats.totalAttempts == 5 && managed.totalSuccess == 1 && managed.totalFailures == 2);
}

void saturatingCountersAndLocalHistory() {
  constexpr auto MAX = std::numeric_limits<uint32_t>::max();
  Fixture f;
  f.stats.totalAttempts = MAX - 1;
  f.stats.totalSuccess = MAX - 1;
  f.stats.totalFailures = MAX - 1;
  auto& managed = f.diagnostics.managedTransport;
  managed.totalSuccess = MAX - 1;
  managed.totalFailures = MAX - 1;
  managed.consecutiveFailures = MAX - 1;
  const t::Status failure{t::Err::I2C_ERROR, -100000, "saturating failure"};
  for (unsigned int i = 0; i < 2; ++i) {
    auto& step = f.fake.expectWrite(0x47, 0x0A, 0x36);
    step.status = failure;
    step.observedStats = &f.stats;
    step.expectedAttempts = MAX;
    step.expectedSuccess = MAX - 1;
    step.expectedFailures = i == 0 ? MAX - 1 : MAX;
  }
  f.fake.expectWrite(0x47, 0x0A, 0x36);
  f.fake.expectWrite(0x47, 0x0A, 0x36);
  for (unsigned int i = 0; i < 2; ++i) {
    const auto result = p::writeRegister(f.context(), p::TrafficSource::MANAGED, 0x0A, 0x36, 20);
    CHECK(result.callbackInvoked && result.status.code == t::Err::I2C_ERROR);
    CHECK(result.writeAcceptance == p::WriteAcceptance::POSSIBLE);
    CHECK(f.stats.totalAttempts == MAX && f.stats.totalFailures == MAX);
    CHECK(managed.totalFailures == MAX && managed.consecutiveFailures == MAX);
  }
  for (unsigned int i = 0; i < 2; ++i) {
    CHECK(p::writeRegister(f.context(), p::TrafficSource::MANAGED, 0x0A, 0x36, 20).status.ok());
    CHECK(f.stats.totalAttempts == MAX && f.stats.totalSuccess == MAX &&
          f.stats.totalFailures == MAX);
    CHECK(managed.totalSuccess == MAX && managed.totalFailures == MAX &&
          managed.consecutiveFailures == 0);
    CHECK(sameStatus(f.diagnostics.lastFailedTransportStatus, failure));
  }
  CHECK(f.fake.complete());
  const auto statusBefore = f.stats.lastStatus;
  const auto managedBefore = managed.lastStatus;
  uint8_t output = 0xD3;
  const auto rejected =
      p::readRegisters(f.context(), p::TrafficSource::MANAGED, 0xFF, &output, 1, 20);
  CHECK(!rejected.callbackInvoked && rejected.status.code == t::Err::INVALID_PARAM);
  CHECK(output == 0xD3);
  CHECK(sameStatus(f.stats.lastStatus, statusBefore));
  CHECK(sameStatus(managed.lastStatus, managedBefore));
  CHECK(sameStatus(f.diagnostics.lastFailedTransportStatus, failure));
  CHECK(f.stats.totalAttempts == MAX && f.stats.totalSuccess == MAX &&
        f.stats.totalFailures == MAX);
  CHECK(managed.totalSuccess == MAX && managed.totalFailures == MAX &&
        managed.consecutiveFailures == 0);
  CHECK(f.fake.complete());
}

void identityAndRevision() {
  constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
  constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
  for (const auto& variant : VARIANTS) {
    const auto* expected = variant.is322 ? ID322 : ID320;
    uint8_t identity[8];
    std::memcpy(identity, expected, sizeof(identity));
    CHECK(p::checkIdentity(variant.variant, identity, 8).ok());
    CHECK(std::memcmp(identity, expected, sizeof(identity)) == 0);
    // Each byte matters, including the terminal NUL; no string or host-word shortcut.
    for (std::size_t i = 0; i < sizeof(identity); ++i) {
      identity[i] ^= 0xFF;
      CHECK(p::checkIdentity(variant.variant, identity, 8).code == t::Err::CHIP_ID_MISMATCH);
      identity[i] ^= 0xFF;
    }
    for (std::size_t i = 0; i < sizeof(identity); ++i) identity[i] = expected[7 - i];
    CHECK(p::checkIdentity(variant.variant, identity, 8).code == t::Err::CHIP_ID_MISMATCH);
    CHECK(p::checkIdentity(variant.variant, variant.is322 ? ID320 : ID322, 8).code ==
          t::Err::CHIP_ID_MISMATCH);
    for (std::size_t length = 0; length < 8; ++length) {
      CHECK(p::checkIdentity(variant.variant, expected, length).code == t::Err::INVALID_PARAM);
    }
    CHECK(p::checkIdentity(variant.variant, expected, 9).code == t::Err::INVALID_PARAM);
    CHECK(p::checkIdentity(variant.variant, expected, std::numeric_limits<std::size_t>::max())
              .code == t::Err::INVALID_PARAM);
    CHECK(p::checkIdentity(variant.variant, nullptr, 8).code == t::Err::INVALID_PARAM);
    for (unsigned int revision = 0; revision <= 255; ++revision) {
      const auto result = p::checkRevision(variant.variant, static_cast<uint8_t>(revision));
      CHECK(result.code == (variant.base ? t::Err::UNSUPPORTED_FEATURE
          : revision == variant.revision ? t::Err::OK : t::Err::CHIP_ID_MISMATCH));
      CHECK(result.detail == 0);
    }
  }
  for (unsigned int value = 0; value <= 255; ++value) {
    bool valid = false;
    for (const auto& variant : VARIANTS) {
      if (static_cast<unsigned int>(variant.variant) == value) valid = true;
    }
    if (valid) continue;
    const auto variant = static_cast<t::Variant>(value);
    CHECK(p::checkIdentity(variant, ID320, 8).code == t::Err::INVALID_CONFIG);
    CHECK(p::checkRevision(variant, 2).code == t::Err::INVALID_CONFIG);
  }
}

void transportedIdentityAndRevision() {
  // Compose only the private primitives under test; this is not initialization or an owner engine.
  constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
  constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
  for (const auto& variant : VARIANTS) {
    Fixture f(variant);
    const auto* expected = variant.is322 ? ID322 : ID320;
    uint8_t identity[8];
    std::memset(identity, 0xD3, sizeof(identity));
    auto& idRead = f.fake.expectRead(variant.low, 0x00, 8);
    std::memcpy(idRead.rx, expected, sizeof(identity));
    idRead.callerOutput = identity;
    const auto transferred =
        p::readRegisters(f.context(), p::TrafficSource::MANAGED, 0x00, identity, 8, 20);
    checkSuccess(f, transferred, false);
    CHECK(std::memcmp(identity, expected, sizeof(identity)) == 0);
    CHECK(p::checkIdentity(variant.variant, identity, sizeof(identity)).ok());
    CHECK(f.stats.totalAttempts == 1 && f.stats.totalSuccess == 1);

    if (!variant.base) {
      uint8_t revision = 0xD3;
      f.fake.expectRead(variant.low, 0xA0, 1).rx[0] = variant.revision;
      CHECK(p::readRegisters(f.context(), p::TrafficSource::MANAGED, 0xA0, &revision, 1, 20)
                .status.ok());
      CHECK(revision == variant.revision);
      CHECK(p::checkRevision(variant.variant, revision).ok());
      CHECK(f.stats.totalAttempts == 2 && f.stats.totalSuccess == 2);

      f.fake.expectRead(variant.low, 0xA0, 1).rx[0] = 0x06;
      CHECK(p::readRegisters(f.context(), p::TrafficSource::RAW, 0xA0, &revision, 1, 20)
                .status.ok());
      const auto mismatch = p::checkRevision(variant.variant, revision);
      CHECK(mismatch.code == t::Err::CHIP_ID_MISMATCH);
      CHECK(revision == 0x06);
      CHECK(f.stats.totalAttempts == 3 && f.stats.totalSuccess == 3 &&
            f.stats.totalFailures == 0);
      CHECK(f.stats.lastStatus.ok());
      CHECK(f.diagnostics.managedTransport.totalSuccess == 2);
      CHECK(f.diagnostics.lastFailedTransportStatus.ok());
      CHECK(f.fake.complete());
    }

    auto& partial = f.fake.expectRead(variant.low, 0x00, 8);
    partial.fillLength = 3;
    partial.rx[0] = 0xFF;
    partial.rx[1] = 0xEE;
    partial.rx[2] = 0xDD;
    partial.status = {t::Err::I2C_NACK_DATA, -70000, "partial identity transfer"};
    partial.callerOutput = identity;
    const auto failed =
        p::readRegisters(f.context(), p::TrafficSource::MANAGED, 0x00, identity, 8, 20);
    CHECK(failed.callbackInvoked && sameStatus(failed.status, partial.status));
    CHECK(failed.writeAcceptance == p::WriteAcceptance::NOT_ATTEMPTED);
    CHECK(std::memcmp(identity, expected, sizeof(identity)) == 0);
    CHECK(f.stats.totalAttempts == (variant.base ? 2U : 4U));
    CHECK(f.stats.totalSuccess == (variant.base ? 1U : 3U));
    CHECK(f.stats.totalFailures == 1);
    CHECK(f.diagnostics.managedTransport.totalSuccess == (variant.base ? 1U : 2U));
    CHECK(f.diagnostics.managedTransport.totalFailures == 1);
    CHECK(!f.diagnostics.initialized && !f.diagnostics.managedTransport.initialized);
    CHECK(!f.diagnostics.identity.identityValid && !f.diagnostics.identity.revisionValid);
    CHECK(f.fake.complete());
  }
}

void scriptedFixtureRejectsMismatches() {
  // Exercise the negative fixture paths so a permissive fake cannot certify protocol framing.
  for (unsigned int mismatch = 0; mismatch < 7; ++mismatch) {
    Fake fake;
    fake.expectRead(0x47, 0x08, 1);
    uint8_t tx[] = {0x08, 0x00};
    uint8_t rx[2] = {};
    if (mismatch == 0) {
      CHECK(!Fake::write(0x47, tx, 1, 20, &fake).ok());
    } else {
      if (mismatch == 2) tx[0] = 0x09;
      CHECK(!Fake::writeRead(mismatch == 1 ? 0x67 : 0x47,
                             mismatch == 6 ? nullptr : tx, mismatch == 3 ? 2U : 1U,
                             rx, mismatch == 4 ? 2U : 1U,
                             mismatch == 5 ? 19U : 20U, &fake).ok());
    }
    CHECK(fake.errors == 1);
    CHECK(!fake.complete());
  }
  Fake fake;
  uint8_t tx = 0x08;
  uint8_t rx = 0;
  CHECK(!Fake::writeRead(0x47, &tx, 1, &rx, 1, 20, &fake).ok());
  CHECK(fake.errors == 1 && fake.calls == 1 && !fake.complete());
}

// Manually connect pure composition to one terminal callback. These scripts prove
// the actual [register,value] boundary, not execution/timing of an owner procedure.
void composedWriteFrames() {
  for (const auto& variant : VARIANTS) {
    for (unsigned int kind = 0; kind < 3; ++kind) {
      if (variant.is322 && kind != 2) continue;
      Fixture f(variant);
      p::WriteRequest request{};
      request.variant = variant.variant;
      request.reg = kind == 2 ? 0x0A : 0x09;
      request.intent = kind == 0 ? p::WriteIntent::ACK_IRQ
          : kind == 1 ? p::WriteIntent::SETTINGS : p::WriteIntent::RESET;
      request.desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_40;
      if (kind == 1) {
        request.fields = static_cast<uint16_t>(t::ProfileField::DRP_DUTY_CYCLE);
        if (!variant.base) {
          request.fields |= static_cast<uint16_t>(t::ProfileField::DISABLE_UFP_ACCESSORY);
        }
      }
      p::WriteEvidence evidence{};
      evidence.actualValid = true;
      evidence.effectsResolved = kind == 1;
      evidence.actual = kind == 2 ? (variant.base ? 0xE7 : 0xE2)
                                 : (variant.base ? 0xB4 : 0xB5);
      // All expected bytes are independent literals from the TI register tables.
      const uint8_t expected = kind == 2 ? (variant.base ? 0xE8 : 0xEA)
          : kind == 1 ? 0x02 : (variant.base ? 0x14 : 0x15);
      t::RegisterDecode decoded{};
      CHECK(t::decodeRegister(variant.variant, request.reg, evidence.actual, decoded).ok());
      uint8_t value = 0xFF;
      CHECK(p::composeWrite(request, evidence, value).ok());
      CHECK(value == expected);
      CHECK(f.fake.calls == 0 && f.stats.totalAttempts == 0);
      f.fake.expectWrite(variant.low, request.reg, expected);
      checkSuccess(f, p::writeRegister(f.context(), p::TrafficSource::MANAGED,
                                     request.reg, value, 20), true);
    }
  }
}

#undef CHECK

}  // namespace

int runCodecTests();

int main() {
  const int codecResult = runCodecTests();
  scriptedFixtureRejectsMismatches();
  addressesAndBindings();
  accessAndRanges();
  framingTimeoutsAndScratch();
  validNullContext();
  normalizedStatusesAndAcceptance();
  accountingHistory();
  saturatingCountersAndLocalHistory();
  identityAndRevision();
  transportedIdentityAndRevision();
  composedWriteFrames();
  if (failures != 0 || codecResult != 0) {
    std::printf("FAIL %d of %zu protocol checks\n", failures, checks);
    return 1;
  }
  std::printf("PASS %zu protocol checks; scripted terminal callbacks and pure identity policy\n",
              checks);
  return 0;
}
