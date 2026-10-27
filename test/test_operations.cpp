/// @file test_operations.cpp
/// @brief Runtime owner lifecycle, fixed read-only initialization and fresh-time regressions.
#include <TUSB32x/TUSB32x.h>

#include "TUSB32xOwnerTime.h"
#include "support/FakeTusb32xDevice.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <type_traits>

int runProfileObservationChecks();
int runProfileApplicationChecks();
int runResetRecoveryChecks();
int runProtocolConformanceChecks();

namespace {

namespace t = TUSB32x;
using Driver = t::TUSB32x;
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

// Independent of assert/NDEBUG so the Release suite exercises every check.
#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint64_t HALF_RANGE = uint64_t{1} << 63;
constexpr uint64_t MAX_TIME = std::numeric_limits<uint64_t>::max();
constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};

struct Part {
  t::Variant variant;
  uint8_t low;
  bool base;
  bool is322;
  uint8_t revision;
  std::size_t reads;
};

// Independent TI-derived plan F from device-behavior.md: scalar controls, no writes.
constexpr Part PARTS[] = {
    {t::Variant::TUSB320, 0x60, true, false, 0, 5},
    {t::Variant::TUSB320I, 0x60, true, false, 0, 5},
    {t::Variant::TUSB320LI, 0x47, false, false, 1, 6},
    {t::Variant::TUSB320HI, 0x47, false, false, 1, 6},
    {t::Variant::TUSB320LAI, 0x47, false, false, 2, 6},
    {t::Variant::TUSB320HAI, 0x47, false, false, 2, 6},
    {t::Variant::TUSB322I, 0x47, false, true, 2, 5},
};

struct Fixture {
  Fake fake{};
  Driver device{};
  t::DriverConfig config{};
  const Part& part;

  explicit Fixture(const Part& selected = PARTS[4]) : part(selected) {
    config.i2cWrite = Fake::write;
    config.i2cWriteRead = Fake::writeRead;
    config.i2cUser = &fake;
    config.variant = part.variant;
    config.address = static_cast<t::DeviceAddress>(part.low);
  }

  void bind() { CHECK(device.bind(config).ok()); }

  void script(uint8_t config08 = 0x80, uint8_t config09 = 0x14,
              uint8_t config0A = 0x30, uint8_t config45 = 0) {
    auto& id = fake.expectRead(part.low, 0, 8);
    std::memcpy(id.rx, part.is322 ? ID322 : ID320, 8);
    if (!part.base) fake.expectRead(part.low, 0xA0, 1).rx[0] = part.revision;
    fake.expectRead(part.low, 0x08, 1).rx[0] = config08;
    fake.expectRead(part.low, 0x09, 1).rx[0] = config09;
    fake.expectRead(part.low, 0x0A, 1).rx[0] = config0A;
    if (!part.is322) fake.expectRead(part.low, 0x45, 1).rx[0] = config45;
  }

  t::OperationToken start(uint64_t now = 100, uint64_t deadline = 1000) {
    t::OperationToken token{999};
    CHECK(device.startInitialize({now, deadline}, token).code == t::Err::IN_PROGRESS);
    CHECK(token.valid() && token.value != 999);
    return token;
  }

  t::Status finish(t::OperationToken token, uint64_t now = 100, uint8_t budget = 1) {
    t::Status status{};
    for (std::size_t turn = 0; turn < part.reads + 2; ++turn) {
      const auto previous = fake.calls;
      status = device.poll(token, now, budget);
      CHECK(fake.calls - previous <= 1);
      CHECK(device.progress().lastPollTransfers <= 1);
      if (!status.inProgress()) return status;
      ++now;
    }
    CHECK(false);  // Fixed plan exceeded its callback count plus completion turn.
    return status;
  }

  t::OperationResult take(t::OperationToken token) {
    t::OperationResult out{};
    CHECK(device.takeResult(token, out).ok());
    CHECK(out.token.value == token.value);
    return out;
  }
};

bool sameStatus(t::Status left, t::Status right) {
  return left.code == right.code && left.detail == right.detail && left.msg == right.msg;
}

bool sameProgress(const t::OperationProgress& a, const t::OperationProgress& b) {
  return a.token.value == b.token.value && a.kind == b.kind && a.state == b.state &&
         a.phase == b.phase && a.transfers == b.transfers &&
         a.lastPollTransfers == b.lastPollTransfers && a.deadlineMs == b.deadlineMs &&
         a.waiting == b.waiting && a.readyAtMs == b.readyAtMs &&
         a.readyAtValid == b.readyAtValid && a.resultPending == b.resultPending;
}

void unchangedDelivery(Driver& device, t::OperationToken token, t::Err expected) {
  t::OperationResult result{};
  result.token.value = 0xAABBCCDD;
  result.status = {t::Err::I2C_BUS, -123456, "caller sentinel"};
  result.configuration.rawConfig08 = 0xD3;
  result.port.sequence = MAX_TIME;
  unsigned char original[sizeof(result)];
  std::memcpy(original, &result, sizeof(result));
  CHECK(device.resultSnapshot(token, result).code == expected);
  CHECK(std::memcmp(original, &result, sizeof(result)) == 0);
  CHECK(device.takeResult(token, result).code == expected);
  CHECK(std::memcmp(original, &result, sizeof(result)) == 0);
}

void emptyAndAdmission() {
  static_assert(!std::is_copy_constructible_v<Driver>);
  static_assert(!std::is_copy_assignable_v<Driver>);
  static_assert(!std::is_move_constructible_v<Driver>);
  static_assert(!std::is_move_assignable_v<Driver>);
  Fixture f;
  CHECK(!f.device.diagnostics().bound && !f.device.diagnostics().initialized);
  CHECK(f.device.progress().state == t::OperationState::IDLE);
  CHECK(!f.device.progress().token.valid());
  CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNCONFIGURED);
  CHECK(f.device.portSnapshot().invalidated && f.device.portSnapshot().rawValidMask == 0);
  unchangedDelivery(f.device, {}, t::Err::INVALID_PARAM);
  unchangedDelivery(f.device, {1}, t::Err::RESULT_NOT_AVAILABLE);
  CHECK(f.device.poll({}, 0).code == t::Err::INVALID_PARAM);
  CHECK(f.device.cancel({}).code == t::Err::INVALID_PARAM);
  CHECK(f.device.poll({1}, 0).code == t::Err::RESULT_NOT_AVAILABLE);
  CHECK(f.device.cancel({1}).code == t::Err::RESULT_NOT_AVAILABLE);

  t::OperationToken out{77};
  CHECK(f.device.startInitialize({}, out).code == t::Err::INVALID_PARAM);
  CHECK(out.value == 77);
  CHECK(f.device.startInitialize({100, 99}, out).code == t::Err::INVALID_PARAM);
  CHECK(f.device.startInitialize({0, HALF_RANGE}, out).code == t::Err::INVALID_PARAM);
  CHECK(f.device.startInitialize({100, 101}, out).code == t::Err::NOT_BOUND);
  CHECK(out.value == 77);
  auto invalid = f.config;
  invalid.i2cWrite = nullptr;
  CHECK(f.device.bind(invalid).code == t::Err::INVALID_CONFIG);
  CHECK(!f.device.diagnostics().bound);
  CHECK(f.device.end().ok());
  CHECK(f.device.forgetDeviceEvidence().ok());
  f.bind();
  CHECK(f.device.forgetDeviceEvidence().code == t::Err::INVALID_CONFIG);
  const auto token = f.start();
  CHECK(token.value == 1);
  const auto admitted = f.device.progress();
  CHECK(admitted.kind == t::OperationKind::INITIALIZE);
  CHECK(admitted.state == t::OperationState::ACTIVE && !admitted.resultPending);
  CHECK(admitted.transfers == 0 && admitted.deadlineMs == 1000);
  unchangedDelivery(f.device, token, t::Err::RESULT_NOT_AVAILABLE);
  unchangedDelivery(f.device, {token.value + 1}, t::Err::TOKEN_MISMATCH);
  CHECK(f.device.bind(invalid).code == t::Err::INVALID_CONFIG);
  CHECK(f.device.bind(f.config).code == t::Err::BUSY);
  CHECK(f.device.forgetDeviceEvidence().code == t::Err::BUSY);
  CHECK(f.device.startInitialize({}, out).code == t::Err::INVALID_PARAM);
  CHECK(f.device.startInitialize({100, 1000}, out).code == t::Err::BUSY);
  CHECK(out.value == 77 && sameProgress(f.device.progress(), admitted));
  CHECK(f.device.poll({}, 0).code == t::Err::INVALID_PARAM);
  CHECK(f.device.poll({token.value + 1}, 0).code == t::Err::TOKEN_MISMATCH);
  CHECK(f.device.cancel({token.value + 1}).code == t::Err::TOKEN_MISMATCH);
  CHECK(sameProgress(f.device.progress(), admitted));
  CHECK(f.device.cancel(token).ok());
  CHECK(f.device.bind(f.config).code == t::Err::RESULT_PENDING);
  CHECK(f.device.bind(invalid).code == t::Err::INVALID_CONFIG);
  CHECK(f.device.forgetDeviceEvidence().code == t::Err::RESULT_PENDING);
  CHECK(f.device.startInitialize({100, 1000}, out).code == t::Err::RESULT_PENDING);
  CHECK(out.value == 77);
  const auto terminal = f.device.progress();
  unchangedDelivery(f.device, {token.value + 1}, t::Err::TOKEN_MISMATCH);
  CHECK(f.device.poll({token.value + 1}, 0).code == t::Err::TOKEN_MISMATCH);
  CHECK(f.device.cancel({token.value + 1}).code == t::Err::TOKEN_MISMATCH);
  CHECK(f.device.poll(token, MAX_TIME, 255).code == t::Err::CANCELLED);
  CHECK(f.device.cancel(token).ok());
  CHECK(sameProgress(f.device.progress(), terminal));
  t::OperationResult first{};
  t::OperationResult second{};
  CHECK(f.device.resultSnapshot(token, first).ok());
  CHECK(f.device.resultSnapshot(token, second).ok());
  CHECK(first.status.code == t::Err::CANCELLED && sameStatus(first.status, second.status));
  CHECK(first.hardwareEffect == t::HardwareEffect::NONE && !first.completedTimeValid);
  CHECK(first.kind == t::OperationKind::INITIALIZE && first.transfers == 0);
  CHECK(f.device.takeResult(token, second).ok());
  CHECK(sameStatus(first.status, second.status));
  unchangedDelivery(f.device, token, t::Err::RESULT_NOT_AVAILABLE);
  CHECK(f.device.progress().state == t::OperationState::IDLE);
  CHECK(!f.device.progress().token.valid());
  CHECK(f.device.poll(token, 100).code == t::Err::RESULT_NOT_AVAILABLE);
  CHECK(f.device.cancel(token).code == t::Err::RESULT_NOT_AVAILABLE);
  const auto next = f.start();
  CHECK(next.value == token.value + 1);
  CHECK(f.device.poll(token, 100).code == t::Err::TOKEN_MISMATCH);
  CHECK(f.device.cancel(token).code == t::Err::TOKEN_MISMATCH);
  unchangedDelivery(f.device, token, t::Err::TOKEN_MISMATCH);
  CHECK(f.fake.complete() && f.device.transportStats().totalAttempts == 0);
}

void initializationAndBudgets() {
  for (const auto& part : PARTS) {
    for (const uint8_t budget : {uint8_t{1}, uint8_t{2}, uint8_t{255}}) {
      Fixture f(part);
      f.bind();
      f.script();
      auto timing = t::OperationTiming{100, 1000};
      t::OperationToken token{};
      CHECK(f.device.startInitialize(timing, token).inProgress());
      timing.deadlineMs = 101; // Admission owns its immutable copy.
      const auto detachedConfig = f.device.configurationSnapshot();
      CHECK(f.device.poll(token, 100, 0).inProgress());
      CHECK(f.fake.calls == 0 && f.device.progress().lastPollTransfers == 0);
      CHECK(f.device.progress().deadlineMs == 1000);
      // Each next turn samples fresh time, even though the floored tick is equal.
      for (std::size_t i = 0; i < part.reads; ++i) {
        CHECK(f.device.poll(token, 100, budget).inProgress());
        CHECK(f.fake.calls == i + 1);
        CHECK(f.device.progress().transfers == i + 1);
        CHECK(f.device.progress().lastPollTransfers == 1);
        CHECK(!f.device.diagnostics().initialized);
        CHECK(!f.device.progress().resultPending);
      }
      CHECK(f.fake.complete());
      unchangedDelivery(f.device, token, t::Err::RESULT_NOT_AVAILABLE);
      CHECK(f.device.configurationSnapshot().rawConfigValidMask == 0);
      // No callback or further owner sample: no success can appear by querying.
      CHECK(f.device.progress().state == t::OperationState::ACTIVE);
      CHECK(!f.device.diagnostics().initialized);
      CHECK(f.device.poll(token, 100, 0).ok());
      CHECK(f.fake.calls == part.reads);
      const auto diagnostics = f.device.diagnostics();
      CHECK(diagnostics.initialized && diagnostics.managedTransport.initialized);
      CHECK(diagnostics.identity.identityValid && diagnostics.identity.identityMatches);
      CHECK(diagnostics.identity.variant == part.variant);
      CHECK(diagnostics.identity.address == f.config.address);
      CHECK(std::memcmp(diagnostics.identity.rawIdentity, part.is322 ? ID322 : ID320, 8) == 0);
      CHECK(diagnostics.identity.revisionValid == !part.base);
      CHECK(diagnostics.identity.revisionMatches == !part.base);
      if (!part.base) CHECK(diagnostics.identity.rawRevision == part.revision);
      const auto config = f.device.configurationSnapshot();
      CHECK(config.state == t::ConfigurationState::KNOWN);
      CHECK(config.hasObservedProfile && config.hasVerifiedProfile && !config.hasDesiredProfile);
      CHECK(config.verifiedGeneration == 1 && config.desiredGeneration == 0);
      CHECK(config.rawConfigValidMask == (part.is322 ? 0x07 : 0x0F));
      CHECK(config.observedValidFields == (part.base ? 0x9B : part.is322 ? 0x7F : 0xFF));
      CHECK(config.rawConfig08 == 0x80 && config.rawConfig09 == 0x14 && config.rawConfig0A == 0x30);
      CHECK(config.observedTimeValid && config.observedUptimeMs == 100);
      CHECK(config.observedProfile.portMode == t::PortMode::DRP);
      CHECK(config.observedProfile.sourceCurrent == t::SourceCurrent::MA_3000);
      CHECK(config.observedProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_50);
      CHECK(config.observedProfile.ccDebounce == (part.base ? t::CcDebounce::MS_133 : t::CcDebounce::MS_168));
      CHECK(config.terminationState == t::TerminationState::ENABLED);
      CHECK(!config.mismatchValid);
      CHECK(detachedConfig.rawConfigValidMask == 0 && !detachedConfig.hasVerifiedProfile);
      CHECK(f.device.portSnapshot().rawValidMask == 0 && f.device.portSnapshot().sequence == 0);
      const auto stats = f.device.transportStats();
      CHECK(stats.totalAttempts == part.reads && stats.totalSuccess == part.reads);
      CHECK(stats.totalFailures == 0 && stats.lastStatus.ok());
      CHECK(diagnostics.managedTransport.totalSuccess == part.reads);
      CHECK(!diagnostics.managedTransport.lastSuccessTimeValid);
      const auto result = f.take(token);
      CHECK(result.state == t::OperationState::SUCCEEDED && result.status.ok());
      CHECK(result.hardwareEffect == t::HardwareEffect::NONE && result.transfers == part.reads);
      CHECK(result.startedUptimeMs == 100 && result.completedTimeValid && result.completedUptimeMs == 100);
      CHECK(result.configuration.state == t::ConfigurationState::KNOWN);
      CHECK(!result.interrupt.acknowledgeAttempted && !result.interrupt.observationValid);
      CHECK(f.device.diagnostics().initialized);
      CHECK(f.device.configurationSnapshot().verifiedGeneration == 1);
    }
  }
}

void everyFailureBoundary() {
  constexpr t::Err ERRORS[] = {t::Err::I2C_ERROR, t::Err::I2C_NACK_ADDR, t::Err::I2C_NACK_DATA,
                             t::Err::I2C_TIMEOUT, t::Err::I2C_BUS, t::Err::I2C_BUSY};
  for (const auto& part : PARTS) {
    for (std::size_t failedAt = 0; failedAt < part.reads; ++failedAt) {
      for (const auto error : ERRORS) {
        Fixture f(part);
        f.bind();
        f.script();
        auto& failed = f.fake.script[failedAt];
        failed.status = {error, -987654, "backend owner test fault"};
        failed.fillLength = 0; // Unpopulated receive storage never becomes observation.
        const auto token = f.start();
        for (std::size_t turn = 0; turn < failedAt; ++turn)
          CHECK(f.device.poll(token, 100, 255).inProgress());
        const auto status = f.device.poll(token, 100, 255);
        CHECK(sameStatus(status, failed.status));
        CHECK(f.fake.calls == failedAt + 1 && f.fake.errors == 0);
        CHECK(f.device.progress().lastPollTransfers == 1);
        CHECK(!f.device.diagnostics().initialized);
        CHECK(!f.device.diagnostics().managedTransport.initialized);
        CHECK(f.device.transportStats().totalAttempts == failedAt + 1);
        CHECK(f.device.transportStats().totalSuccess == failedAt);
        CHECK(f.device.transportStats().totalFailures == 1);
        CHECK(f.device.diagnostics().managedTransport.totalFailures == 1);
        CHECK(sameStatus(f.device.diagnostics().lastFailedTransportStatus, failed.status));
        CHECK(!f.device.diagnostics().lastFailedTransportTimeValid);
        CHECK(!f.device.diagnostics().managedTransport.lastFailureTimeValid);
        CHECK(f.device.poll(token, 2000).code == error); // Actual failure stays primary after deadline.
        CHECK(f.device.cancel(token).ok());
        const auto result = f.take(token);
        CHECK(sameStatus(result.status, failed.status));
        CHECK(result.state == t::OperationState::FAILED && !result.completedTimeValid);
        CHECK(result.transfers == failedAt + 1 && result.hardwareEffect == t::HardwareEffect::NONE);
        CHECK(result.configuration.rawConfigValidMask == 0);
        CHECK(!result.configuration.hasObservedProfile && !result.configuration.hasVerifiedProfile);
        CHECK(f.fake.calls == failedAt + 1);
      }
    }
  }
  // Illegal callback statuses must use the same wrapper normalization path.
  for (const auto illegal : {t::Err::IN_PROGRESS, t::Err::CANCELLED, static_cast<t::Err>(255)}) {
    Fixture f;
    f.bind();
    auto& read = f.fake.expectRead(0x47, 0, 8);
    read.fillLength = 0;
    read.status = {illegal, std::numeric_limits<int32_t>::min(), "illegal callback status"};
    const auto token = f.start();
    CHECK(f.device.poll(token, 100).code == t::Err::I2C_ERROR);
    CHECK(f.fake.complete());
    const auto result = f.take(token);
    CHECK(result.status.detail == std::numeric_limits<int32_t>::min());
    CHECK(result.transfers == 1 && !result.completedTimeValid);
  }
}

void interruptionAtEveryPhase() {
  for (const auto& part : PARTS) {
    for (std::size_t boundary = 0; boundary <= part.reads; ++boundary) {
      for (unsigned action = 0; action < 3; ++action) {
        Fixture f(part);
        f.bind();
        f.script();
        const auto token = f.start();
        for (std::size_t turn = 0; turn < boundary; ++turn)
          CHECK(f.device.poll(token, 100, 255).inProgress());
        if (action == 0) CHECK(f.device.cancel(token).ok());
        if (action == 1) CHECK(f.device.end().ok());
        if (action == 2) CHECK(f.device.invalidateDeviceState(
            {t::Err::I2C_BUS, -110022, "external power loss"}).ok());
        CHECK(f.fake.calls == boundary && f.fake.errors == 0);
        CHECK(!f.device.diagnostics().initialized);
        CHECK(f.device.diagnostics().bound == (action != 1));
        CHECK(f.device.portSnapshot().invalidated);
        CHECK(f.device.progress().resultPending);
        CHECK(f.device.poll(token, MAX_TIME).code == t::Err::CANCELLED);
        CHECK(f.device.end().ok());
        CHECK(f.device.end().ok());
        CHECK(f.device.forgetDeviceEvidence().code == t::Err::RESULT_PENDING);
        const auto result = f.take(token);
        CHECK(result.state == t::OperationState::CANCELLED && result.status.code == t::Err::CANCELLED);
        CHECK(result.hardwareEffect == t::HardwareEffect::NONE && !result.completedTimeValid);
        CHECK(result.transfers == boundary && result.port.invalidated);
        CHECK(result.configuration.rawConfigValidMask == 0);
        if (action == 2) CHECK(result.status.detail == -110022);
        CHECK(f.fake.calls == boundary);
        CHECK(f.device.forgetDeviceEvidence().ok());
        CHECK(f.device.transportStats().totalAttempts == boundary);
        CHECK(f.device.diagnostics().lastOperationStatus.code == t::Err::CANCELLED);
        f.bind();
        const auto next = f.start();
        CHECK(next.value == token.value + 1);
        CHECK(f.fake.calls == boundary);
      }
    }
  }
}

void timeBoundaries() {
  for (const auto& part : PARTS) {
    for (std::size_t boundary = 0; boundary <= part.reads; ++boundary) {
      Fixture stopped(part);
      stopped.bind();
      stopped.script();
      const auto token = stopped.start(100, 130);
      for (std::size_t i = 0; i < boundary; ++i)
        CHECK(stopped.device.poll(token, 100, 255).inProgress());
      CHECK(stopped.device.poll(token, 130, 255).code == t::Err::TIMEOUT);
      CHECK(stopped.fake.calls == boundary && stopped.fake.errors == 0);
      CHECK(stopped.device.progress().lastPollTransfers == 0);
      CHECK(stopped.device.transportStats().totalAttempts == boundary);
      CHECK(stopped.device.transportStats().totalFailures == 0);
      CHECK(!stopped.device.diagnostics().initialized);
      const auto result = stopped.take(token);
      CHECK(result.completedTimeValid && result.completedUptimeMs == 130);
      CHECK(result.transfers == boundary && result.hardwareEffect == t::HardwareEffect::NONE);
      CHECK(result.configuration.rawConfigValidMask == 0);
    }
  }
  // Zero budget can expire a deadline, with zero attempts and a valid software completion time.
  for (const uint64_t now : {uint64_t{110}, uint64_t{111}}) {
    Fixture f;
    f.bind();
    const auto token = f.start(100, 110);
    CHECK(f.device.poll(token, 109, 0).inProgress());
    CHECK(f.device.poll(token, now, 0).code == t::Err::TIMEOUT);
    const auto result = f.take(token);
    CHECK(result.state == t::OperationState::TIMED_OUT && result.transfers == 0);
    CHECK(result.completedTimeValid && result.completedUptimeMs == now);
    CHECK(f.fake.complete() && f.device.transportStats().totalAttempts == 0);
  }
  // Fresh final completion strictly before the deadline succeeds; equality and lateness cannot.
  for (const uint64_t completed : {uint64_t{109}, uint64_t{110}, uint64_t{111}}) {
    Fixture f;
    f.bind();
    f.script();
    for (std::size_t i = 0; i < f.part.reads; ++i) f.fake.script[i].timeoutMs = 10;
    const auto token = f.start(100, 110);
    for (std::size_t i = 0; i < f.part.reads; ++i)
      CHECK(f.device.poll(token, 100, 255).inProgress());
    CHECK(!f.device.diagnostics().initialized);
    CHECK(f.device.poll(token, completed, 0).code ==
          (completed < 110 ? t::Err::OK : t::Err::TIMEOUT));
    const auto result = f.take(token);
    CHECK(result.completedTimeValid && result.completedUptimeMs == completed);
    CHECK(f.device.diagnostics().initialized == (completed < 110));
    CHECK(result.configuration.rawConfigValidMask == (completed < 110 ? 0x0F : 0));
    CHECK(f.fake.complete() && f.device.transportStats().totalFailures == 0);
  }
  // Clip in uint64 before narrowing, including a one-millisecond remaining budget.
  for (const uint64_t interval : {uint64_t{1}, uint64_t{7}, uint64_t{20},
                                  uint64_t{21}, uint64_t{0x100000001ULL}, HALF_RANGE - 1}) {
    Fixture f;
    f.bind();
    auto& read = f.fake.expectRead(0x47, 0, 8,
        interval < 20 ? static_cast<uint32_t>(interval) : 20);
    std::memcpy(read.rx, ID320, 8);
    const auto token = f.start(0, interval);
    CHECK(f.device.poll(token, 0).inProgress());
    CHECK(f.fake.complete());
    CHECK(f.device.progress().deadlineMs == interval);
  }
  // Invalid time preserves the last accepted sample, staged success and exact progress.
  Fixture f;
  f.bind();
  f.script();
  const auto token = f.start();
  CHECK(f.device.poll(token, 100).inProgress());
  const auto pending = f.device.progress();
  for (const uint64_t invalid : {uint64_t{99}, uint64_t{100} + HALF_RANGE,
                                 uint64_t{101} + HALF_RANGE}) {
    CHECK(f.device.poll(token, invalid, 255).code == t::Err::INVALID_PARAM);
    CHECK(sameProgress(pending, f.device.progress()));
    CHECK(f.fake.calls == 1);
  }
  CHECK(f.device.poll(token, 100, 0).inProgress());
  CHECK(f.fake.calls == 1 && f.device.progress().lastPollTransfers == 0);
  CHECK(f.device.poll(token, 100, 0).inProgress());
  CHECK(f.fake.calls == 1); // Equal clock + zero budget cannot drain the plan.
  CHECK(f.finish(token, 100).ok());
  CHECK(f.fake.complete());

  Fixture wrap;
  wrap.bind();
  wrap.script();
  for (std::size_t i = 0; i < wrap.part.reads; ++i) wrap.fake.script[i].timeoutMs = 10;
  const auto wrapped = wrap.start(MAX_TIME - 4, 5);
  for (std::size_t i = 0; i < wrap.part.reads; ++i)
    CHECK(wrap.device.poll(wrapped, MAX_TIME - 4, 255).inProgress());
  CHECK(wrap.device.poll(wrapped, 0, 0).ok());
  CHECK(wrap.fake.complete());
  const auto wrappedResult = wrap.take(wrapped);
  CHECK(wrappedResult.completedTimeValid && wrappedResult.completedUptimeMs == 0);
  CHECK(wrappedResult.startedUptimeMs == MAX_TIME - 4);
  CHECK(wrappedResult.configuration.observedUptimeMs == MAX_TIME - 4);
}

void reinitializationAndAtomicBinding() {
  Fixture f;
  f.bind();
  f.script();
  const auto original = f.start();
  CHECK(f.finish(original).ok());
  const auto frozen = f.take(original);
  CHECK(f.device.diagnostics().initialized);
  auto invalid = f.config;
  invalid.i2cTimeoutMs = 0;
  CHECK(f.device.bind(invalid).code == t::Err::INVALID_CONFIG);
  CHECK(f.device.diagnostics().initialized);
  CHECK(f.device.configurationSnapshot().verifiedGeneration == 1);
  CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::KNOWN);
  // Same target retains history, and a copied binding ignores subsequent caller mutation.
  auto replacement = f.config;
  replacement.i2cTimeoutMs = 9;
  CHECK(f.device.bind(replacement).ok());
  replacement.i2cWriteRead = nullptr;
  replacement.i2cUser = nullptr;
  replacement.address = t::DeviceAddress::ADDR_HIGH_0X67;
  CHECK(!f.device.diagnostics().initialized);
  CHECK(f.device.configurationSnapshot().verifiedGeneration == 1);
  CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  f.fake = Fake{};
  auto& fault = f.fake.expectRead(0x47, 0, 8, 9);
  fault.fillLength = 3;
  fault.rx[0] = 0xFF;
  fault.status = {t::Err::I2C_TIMEOUT, -200001, "failed reinitialization"};
  const auto retry = f.start(200, 1000);
  CHECK(retry.value == original.value + 1);
  CHECK(!f.device.diagnostics().initialized);
  CHECK(f.device.poll(retry, 200).code == t::Err::I2C_TIMEOUT);
  CHECK(!f.device.diagnostics().initialized);
  CHECK(!f.device.diagnostics().managedTransport.initialized);
  const auto failed = f.take(retry);
  CHECK(failed.configuration.verifiedGeneration == 1);
  CHECK(failed.configuration.rawConfig08 == 0x80);
  CHECK(failed.configuration.state == t::ConfigurationState::UNKNOWN);
  CHECK(f.device.transportStats().totalAttempts == 7 && f.fake.complete());
  CHECK(frozen.status.ok() && frozen.configuration.state == t::ConfigurationState::KNOWN);
  CHECK(f.device.end().ok());
  auto other = f.config;
  other.address = t::DeviceAddress::ADDR_HIGH_0X67;
  CHECK(f.device.bind(other).ok()); // UNKNOWN alone is not unresolved hardware effect.
  CHECK(f.device.configurationSnapshot().rawConfigValidMask == 0);
  CHECK(!f.device.configurationSnapshot().hasVerifiedProfile);
  CHECK(!f.device.diagnostics().identity.identityValid);
  CHECK(f.device.portSnapshot().rawValidMask == 0);
  CHECK(f.device.transportStats().totalAttempts == 7);
  CHECK(f.device.diagnostics().lastOperationStatus.code == t::Err::I2C_TIMEOUT);
}

void failedReinitializationAtEveryPhase() {
  for (const auto& part : PARTS) {
    for (std::size_t failedAt = 0; failedAt < part.reads; ++failedAt) {
      Fixture f(part);
      f.bind();
      f.script();
      const auto original = f.start();
      CHECK(f.finish(original).ok());
      CHECK(f.take(original).status.ok());
      const auto before = f.device.configurationSnapshot();
      f.fake = Fake{};
      f.script(0x40, 0x02, 0x20);
      f.fake.script[failedAt].status = {t::Err::I2C_BUS, -550001, "reinit callback failed"};
      f.fake.script[failedAt].fillLength = 0;
      const auto retry = f.start(200, 1000);
      CHECK(!f.device.diagnostics().initialized);
      CHECK(!f.device.diagnostics().managedTransport.initialized);
      CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
      for (std::size_t i = 0; i < failedAt; ++i)
        CHECK(f.device.poll(retry, 200, 255).inProgress());
      CHECK(f.device.poll(retry, 200, 255).code == t::Err::I2C_BUS);
      CHECK(!f.device.diagnostics().initialized && !f.device.diagnostics().managedTransport.initialized);
      const auto after = f.device.configurationSnapshot();
      CHECK(after.rawConfigValidMask == before.rawConfigValidMask);
      CHECK(after.rawConfig08 == before.rawConfig08 && after.rawConfig09 == before.rawConfig09);
      CHECK(after.rawConfig0A == before.rawConfig0A && after.rawConfig45 == before.rawConfig45);
      CHECK(after.observedUptimeMs == before.observedUptimeMs);
      CHECK(after.verifiedGeneration == before.verifiedGeneration);
      CHECK(after.hasVerifiedProfile && after.hasObservedProfile);
      CHECK(after.state == t::ConfigurationState::UNKNOWN);
      f.device.clearTransportStats();
      CHECK(f.device.transportStats().totalAttempts == 0);
      CHECK(f.device.transportStats().totalFailures == 0);
      CHECK(f.device.diagnostics().lastFailedTransportStatus.detail == -550001);
      CHECK(f.device.diagnostics().managedTransport.totalFailures == 1);
      CHECK(f.device.progress().resultPending);
      CHECK(f.device.end().ok());
      t::OperationToken rejected{999};
      CHECK(f.device.startInitialize({200, 1000}, rejected).code == t::Err::NOT_BOUND);
      CHECK(rejected.value == 999);
      const auto result = f.take(retry);
      CHECK(result.status.code == t::Err::I2C_BUS && result.status.detail == -550001);
      CHECK(!result.completedTimeValid && result.configuration.state == t::ConfigurationState::UNKNOWN);
      CHECK(f.fake.calls == failedAt + 1 && f.fake.errors == 0);
    }
  }
}

void invalidationAndFrozenResults() {
  Fixture f;
  f.bind();
  const auto token = f.start();
  const auto admitted = f.device.progress();
  for (const auto invalid : {t::Err::IN_PROGRESS, static_cast<t::Err>(255)}) {
    CHECK(f.device.invalidateDeviceState({invalid, 77, "rejected reason"}).code == t::Err::INVALID_PARAM);
    CHECK(sameProgress(admitted, f.device.progress()));
  }
  CHECK(f.device.invalidateDeviceState(t::Status::Ok()).ok());
  const auto cancelled = f.take(token);
  CHECK(cancelled.status.code == t::Err::CANCELLED && cancelled.status.detail == 0);
  CHECK(cancelled.status.msg != nullptr && cancelled.status.msg[0] != '\0');
  CHECK(!cancelled.completedTimeValid);
  CHECK(f.device.diagnostics().bound && !f.device.diagnostics().initialized);
  const auto previousStatus = f.device.diagnostics().lastOperationStatus;
  CHECK(f.device.invalidateDeviceState({t::Err::I2C_ERROR, 88, "idle reason"}).ok());
  CHECK(sameStatus(f.device.diagnostics().lastOperationStatus, previousStatus));
  CHECK(f.device.transportStats().totalAttempts == 0);
  f.script();
  const auto successful = f.start();
  CHECK(f.finish(successful).ok());
  t::OperationResult before{};
  CHECK(f.device.resultSnapshot(successful, before).ok());
  CHECK(f.device.invalidateDeviceState({t::Err::CONFIGURATION_UNKNOWN, 55, "changed externally"}).ok());
  CHECK(!f.device.diagnostics().initialized);
  CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  CHECK(f.device.end().ok());
  t::OperationResult after{};
  CHECK(f.device.resultSnapshot(successful, after).ok());
  CHECK(sameStatus(before.status, after.status));
  CHECK(after.state == t::OperationState::SUCCEEDED && after.completedTimeValid);
  CHECK(after.completedUptimeMs == before.completedUptimeMs);
  CHECK(after.configuration.state == t::ConfigurationState::KNOWN);
  CHECK(after.configuration.rawConfig08 == before.configuration.rawConfig08);
  CHECK(after.identity.identityMatches);
  f.device.clearTransportStats();
  CHECK(f.device.transportStats().totalAttempts == 0 && f.device.transportStats().lastStatus.ok());
  CHECK(f.device.diagnostics().managedTransport.totalSuccess == 6);
  CHECK(f.device.progress().resultPending);
  CHECK(f.take(successful).status.ok());
  CHECK(f.device.forgetDeviceEvidence().ok());
  CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNCONFIGURED);
  CHECK(f.device.diagnostics().lastOperationKind == t::OperationKind::INITIALIZE);
  CHECK(f.fake.complete());

  // Every defined terminal notification is accepted, and stays separate from transport accounting.
  for (unsigned value = 0; value <= static_cast<unsigned>(t::Err::I2C_BUSY); ++value) {
    if (value == static_cast<unsigned>(t::Err::IN_PROGRESS)) continue;
    Fixture notified;
    notified.bind();
    const auto active = notified.start();
    const auto reason = static_cast<t::Err>(value);
    CHECK(notified.device.invalidateDeviceState({reason, -334455, "terminal cause"}).ok());
    const auto result = notified.take(active);
    CHECK(result.status.code == t::Err::CANCELLED);
    CHECK(result.status.detail == (reason == t::Err::OK ? 0 : -334455));
    if (reason != t::Err::OK) CHECK(std::strcmp(result.status.msg, "terminal cause") == 0);
    CHECK(notified.fake.complete() && notified.device.transportStats().totalAttempts == 0);
    CHECK(notified.device.diagnostics().lastFailedTransportStatus.ok());
  }
}

void identityAndUnsafeControls() {
  for (const auto& part : PARTS) {
    // Every signature byte and a wrong revision are independently observable contradictions.
    for (std::size_t index = 0; index < (part.base ? 8U : 9U); ++index) {
      Fixture f(part);
      f.bind();
      f.script();
      const std::size_t step = index < 8 ? 0 : 1;
      if (index < 8) f.fake.script[0].rx[index] ^= 0xFF;
      else f.fake.script[1].rx[0] = 6;
      const auto token = f.start();
      for (std::size_t i = 0; i <= step; ++i)
        CHECK(f.device.poll(token, 100).inProgress());
      const auto immediate = f.device.diagnostics();
      CHECK(!immediate.initialized);
      CHECK(immediate.identity.identityValid);
      if (index < 8) CHECK(!immediate.identity.identityMatches);
      else CHECK(immediate.identity.revisionValid && !immediate.identity.revisionMatches);
      CHECK(f.device.poll(token, 101, 0).code == t::Err::CHIP_ID_MISMATCH);
      CHECK(f.fake.calls == step + 1 && f.fake.errors == 0);
      const auto result = f.take(token);
      CHECK(result.status.code == t::Err::CHIP_ID_MISMATCH);
      CHECK(result.completedTimeValid && result.completedUptimeMs == 101);
      CHECK(result.configuration.rawConfigValidMask == 0);
    }
  }
  for (const auto& part : PARTS) {
    for (unsigned condition = 0; condition < 4; ++condition) {
      Fixture f(part);
      f.bind();
      f.script(condition == 0 ? 0xC0 : 0x80, 0x14,
               condition == 1 ? 0x38 : condition == 2 && !part.base ? 0x34 : 0x30,
               condition == 3 ? 0x04 : 0);
      const auto token = f.start();
      const auto status = f.finish(token);
      const bool unknown = condition == 0 || condition == 1 || (condition == 2 && !part.base);
      CHECK(status.code == (unknown ? t::Err::CONFIGURATION_UNKNOWN : t::Err::OK));
      CHECK(f.device.diagnostics().initialized && f.device.diagnostics().managedTransport.initialized);
      const auto config = f.device.configurationSnapshot();
      CHECK(config.rawConfigValidMask == (part.is322 ? 0x07 : 0x0F));
      CHECK(config.state == (unknown ? t::ConfigurationState::UNKNOWN : t::ConfigurationState::KNOWN));
      CHECK(config.hasObservedProfile == !(condition == 0 || (condition == 2 && !part.base)));
      CHECK(config.hasVerifiedProfile == !unknown);
      CHECK(!config.hasDesiredProfile);
      if (condition == 1) CHECK(config.resetState == t::ResetState::MAY_BE_PENDING);
      if (condition == 3) CHECK(config.terminationState ==
          (part.is322 ? t::TerminationState::ENABLED : t::TerminationState::DISABLED));
      CHECK(f.fake.complete());
    }
  }
  // Reset assertion takes effect immediately, before any fresh completion turn.
  Fixture reset;
  reset.bind();
  reset.script(0x80, 0x14, 0x38);
  const auto token = reset.start();
  for (std::size_t i = 0; i < 5; ++i) CHECK(reset.device.poll(token, 100).inProgress());
  CHECK(reset.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
  CHECK(reset.device.end().ok());
  CHECK(reset.take(token).configuration.resetState == t::ResetState::MAY_BE_PENDING);
  auto different = reset.config;
  different.address = t::DeviceAddress::ADDR_HIGH_0X67;
  CHECK(reset.device.bind(different).code == t::Err::CONFIGURATION_UNKNOWN);
  CHECK(!reset.device.diagnostics().bound);
  CHECK(reset.device.bind(reset.config).ok()); // Association preserves the obligation.
  CHECK(reset.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
  CHECK(reset.device.end().ok());
  CHECK(reset.device.forgetDeviceEvidence().ok());
  CHECK(reset.device.bind(different).ok());
  CHECK(reset.device.diagnostics().resetState == t::ResetState::NONE);

  // Both distinct disable controls qualify termination; neither clear bit can mask the other.
  for (const auto& part : PARTS) {
    for (unsigned controls = 0; controls < 4; ++controls) {
      if ((part.base && (controls & 1U) != 0) || (part.is322 && (controls & 2U) != 0)) continue;
      Fixture f(part);
      f.bind();
      f.script(0x80, 0x14, (controls & 1U) != 0 ? 0x31 : 0x30,
               (controls & 2U) != 0 ? 4 : 0);
      const auto initialization = f.start();
      CHECK(f.finish(initialization).ok());
      const auto config = f.device.configurationSnapshot();
      CHECK(config.terminationState == (controls == 0 ? t::TerminationState::ENABLED :
                                                                       t::TerminationState::DISABLED));
      CHECK(f.device.diagnostics().terminationState == config.terminationState);
      CHECK(f.fake.complete());
    }
  }
}

void resetEvidenceSurvivesReinitialization() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.bind();
    f.script(0x80, 0x14, 0x38);
    const auto first = f.start();
    CHECK(f.finish(first).is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(f.take(first).configuration.resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(f.device.end().ok());
    CHECK(f.device.bind(f.config).ok());

    // Clear current bytes cannot prove the earlier reset's settling interval.
    f.fake = Fake{};
    f.script();
    const auto second = f.start(200, 1000);
    CHECK(f.finish(second, 200).is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(f.fake.complete());
    const auto result = f.take(second);
    CHECK(result.configuration.rawConfig0A == 0x30);
    CHECK(result.configuration.hasObservedProfile);
    CHECK(result.configuration.resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
    CHECK(result.configuration.terminationState == t::TerminationState::UNKNOWN);
    CHECK(result.hardwareEffect == t::HardwareEffect::NONE);
    CHECK(f.device.diagnostics().initialized);
    CHECK(f.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
    auto changed = f.config;
    changed.address = static_cast<t::DeviceAddress>(part.base ? 0x61 : 0x67);
    CHECK(f.device.bind(changed).is(t::Err::CONFIGURATION_UNKNOWN));
  }
}

void independentTerminationEvidence() {
  for (const auto& part : PARTS) {
    for (unsigned controls = 0; controls < 4; ++controls) {
      if ((part.base && (controls & 1U) != 0) || (part.is322 && (controls & 2U) != 0)) continue;
      Fixture f(part);
      f.bind();
      // Advertisement code 11 is reserved, while both applicable disable fields remain readable.
      f.script(0xC0, 0x14, (controls & 1U) != 0 ? 0x31 : 0x30,
               (controls & 2U) != 0 ? 4 : 0);
      const auto token = f.start();
      CHECK(f.finish(token).code == t::Err::CONFIGURATION_UNKNOWN);
      const auto config = f.device.configurationSnapshot();
      CHECK(!config.hasObservedProfile && !config.hasVerifiedProfile);
      CHECK(config.state == t::ConfigurationState::UNKNOWN);
      CHECK(config.terminationState == (controls == 0 ? t::TerminationState::ENABLED :
                                                                       t::TerminationState::DISABLED));
      CHECK(f.device.diagnostics().terminationState == config.terminationState);
      CHECK(f.device.diagnostics().initialized);
      CHECK(f.fake.complete());
    }
    if (!part.base) {
      for (const uint8_t config0A : {uint8_t{0x34}, uint8_t{0x35}, uint8_t{0x3C}}) {
        Fixture f(part);
        f.bind();
        f.script(0x80, 0x14, config0A);
        const auto token = f.start();
        CHECK(f.finish(token).code == t::Err::CONFIGURATION_UNKNOWN);
        const auto config = f.device.configurationSnapshot();
        const auto termination = config0A == 0x3C ? t::TerminationState::UNKNOWN :
            config0A == 0x35 ? t::TerminationState::DISABLED : t::TerminationState::ENABLED;
        CHECK(config.state == t::ConfigurationState::UNKNOWN);
        CHECK(!config.hasObservedProfile && config.terminationState == termination);
        CHECK(f.device.diagnostics().terminationState == termination);
        CHECK(f.fake.complete());
      }
    }
  }
}

void identityPublicationNeedsFreshTime() {
  for (const auto& part : PARTS) {
    for (unsigned segment = 0; segment < (part.base ? 1U : 2U); ++segment) {
      for (unsigned action = 0; action < 5; ++action) {
        Fixture f(part);
        f.bind();
        f.script();
        const auto token = f.start(100, 130);
        CHECK(f.device.poll(token, 100, 255).inProgress());
        if (segment == 1) CHECK(f.device.poll(token, 101, 255).inProgress());
        auto identity = f.device.diagnostics().identity;
        // A complete matching segment is still pending until the next valid owner sample.
        CHECK(identity.identityValid == (segment == 1));
        CHECK(identity.identityMatches == (segment == 1));
        CHECK(!identity.revisionValid && !identity.revisionMatches);
        if (action == 4) {
          CHECK(f.device.poll(token, 102, 0).inProgress());
          identity = f.device.diagnostics().identity;
          CHECK(identity.identityValid && identity.identityMatches);
          CHECK(identity.revisionValid == (segment == 1));
          CHECK(identity.revisionMatches == (segment == 1));
          CHECK(f.device.cancel(token).ok());
        } else if (action == 0) CHECK(f.device.cancel(token).ok());
        else if (action == 1) CHECK(f.device.end().ok());
        else if (action == 2) CHECK(f.device.invalidateDeviceState(t::Status::Ok()).ok());
        else CHECK(f.device.poll(token, 130, 255).code == t::Err::TIMEOUT);
        const auto result = f.take(token);
        CHECK(result.identity.identityValid == (segment == 1 || action == 4));
        CHECK(result.identity.identityMatches == (segment == 1 || action == 4));
        CHECK(result.identity.revisionValid == (segment == 1 && action == 4));
        CHECK(result.identity.revisionMatches == (segment == 1 && action == 4));
        CHECK(!f.device.diagnostics().initialized);
        CHECK(f.fake.calls == segment + 1 && f.fake.errors == 0);
      }
    }
    Fixture adverse(part);
    adverse.bind();
    adverse.script();
    adverse.fake.script[0].rx[7] = 1;
    const auto mismatch = adverse.start(100, 130);
    CHECK(adverse.device.poll(mismatch, 100).inProgress());
    CHECK(adverse.device.diagnostics().identity.identityValid);
    CHECK(!adverse.device.diagnostics().identity.identityMatches);
    CHECK(adverse.device.poll(mismatch, 130, 0).code == t::Err::TIMEOUT);
    const auto expired = adverse.take(mismatch);
    CHECK(expired.status.code == t::Err::TIMEOUT && expired.identity.identityValid);
    CHECK(!expired.identity.identityMatches && expired.identity.rawIdentity[7] == 1);
    CHECK(expired.completedTimeValid && expired.completedUptimeMs == 130);
    CHECK(adverse.fake.calls == 1 && adverse.fake.errors == 0);
  }

  // A newly matching read also cannot replace older contradictory raw history before fresh time.
  Fixture historical;
  historical.bind();
  historical.script();
  historical.fake.script[0].rx[0] = 0xFF;
  const auto bad = historical.start();
  CHECK(historical.device.poll(bad, 100).inProgress());
  CHECK(historical.device.poll(bad, 101, 0).code == t::Err::CHIP_ID_MISMATCH);
  CHECK(historical.take(bad).identity.rawIdentity[0] == 0xFF);
  historical.fake = Fake{};
  historical.script();
  const auto retry = historical.start(200, 1000);
  CHECK(historical.device.poll(retry, 200).inProgress());
  CHECK(historical.device.diagnostics().identity.rawIdentity[0] == 0xFF);
  CHECK(!historical.device.diagnostics().identity.identityMatches);
  CHECK(historical.device.cancel(retry).ok());
  CHECK(historical.take(retry).identity.rawIdentity[0] == 0xFF);
  CHECK(historical.fake.calls == 1 && historical.fake.errors == 0);
}

// Null contexts can still identify different buses through either callback pointer.
t::Status nullWriteA(uint8_t, const uint8_t*, std::size_t, uint32_t, void*) {
  return {t::Err::I2C_ERROR, -1, "unexpected null-context write"};
}
t::Status nullWriteB(uint8_t, const uint8_t*, std::size_t, uint32_t, void*) {
  return {t::Err::I2C_ERROR, -2, "unexpected second null-context write"};
}

Fake* nullContextBus = nullptr;
t::Status nullReadA(uint8_t address, const uint8_t* tx, std::size_t txLength,
                    uint8_t* rx, std::size_t rxLength, uint32_t timeout, void*) {
  return Fake::writeRead(address, tx, txLength, rx, rxLength, timeout, nullContextBus);
}
t::Status nullReadB(uint8_t address, const uint8_t* tx, std::size_t txLength,
                    uint8_t* rx, std::size_t rxLength, uint32_t timeout, void*) {
  return Fake::writeRead(address, tx, txLength, rx, rxLength, timeout, nullContextBus);
}

void callbackAssociationAndNullContext() {
  for (unsigned change = 0; change < 5; ++change) {
    for (const bool unresolved : {false, true}) {
      Fixture f;
      f.config.i2cUser = nullptr;
      f.config.i2cWrite = nullWriteA;
      f.config.i2cWriteRead = nullReadA;
      nullContextBus = &f.fake;
      f.bind();
      f.script(0x80, 0x14, unresolved ? 0x38 : 0x30);
      const auto token = f.start();
      CHECK(f.finish(token).code == (unresolved ? t::Err::CONFIGURATION_UNKNOWN : t::Err::OK));
      CHECK(f.take(token).configuration.resetState ==
            (unresolved ? t::ResetState::MAY_BE_PENDING : t::ResetState::NONE));
      CHECK(f.device.end().ok());
      auto changed = f.config;
      if (change == 0) changed.i2cWrite = nullWriteB;
      if (change == 1) changed.i2cWriteRead = nullReadB;
      if (change == 2) changed.i2cUser = &f.fake;
      if (change == 3) changed.variant = t::Variant::TUSB320HAI;
      if (change == 4) changed.address = t::DeviceAddress::ADDR_HIGH_0X67;
      if (!unresolved) {
        CHECK(f.device.bind(changed).ok());
        CHECK(f.device.configurationSnapshot().rawConfigValidMask == 0);
        CHECK(!f.device.configurationSnapshot().hasObservedProfile);
        CHECK(!f.device.configurationSnapshot().hasVerifiedProfile);
        CHECK(f.device.configurationSnapshot().verifiedGeneration == 0);
        CHECK(!f.device.diagnostics().identity.identityValid);
        CHECK(f.device.transportStats().totalAttempts == 6);
        CHECK(f.fake.complete());
        continue;
      }
      CHECK(f.device.bind(changed).code == t::Err::CONFIGURATION_UNKNOWN);
      CHECK(!f.device.diagnostics().bound);
      auto same = f.config;
      same.i2cTimeoutMs = 1;
      CHECK(f.device.bind(same).ok());
      CHECK(f.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
      CHECK(f.device.end().ok());
      CHECK(f.device.forgetDeviceEvidence().ok());
      CHECK(f.device.bind(changed).ok());
      CHECK(f.device.configurationSnapshot().rawConfigValidMask == 0);
      CHECK(!f.device.diagnostics().identity.identityValid);
      CHECK(f.device.diagnostics().resetState == t::ResetState::NONE);
      CHECK(f.fake.complete());
    }
  }
  nullContextBus = nullptr;
}

void twoInstancesAndStatistics() {
  Fake bus;
  Driver first;
  Driver second;
  t::DriverConfig config{};
  config.variant = t::Variant::TUSB320LAI;
  config.i2cWrite = Fake::write;
  config.i2cWriteRead = Fake::writeRead;
  config.i2cUser = &bus;
  CHECK(first.bind(config).ok());
  config.address = t::DeviceAddress::ADDR_HIGH_0X67;
  CHECK(second.bind(config).ok());
  auto& firstId = bus.expectRead(0x47, 0, 8);
  std::memcpy(firstId.rx, ID320, 8);
  auto& failed = bus.expectRead(0x67, 0, 8);
  failed.fillLength = 0;
  failed.status = {t::Err::I2C_NACK_ADDR, -90003, "second target absent"};
  bus.expectRead(0x47, 0xA0, 1).rx[0] = 2;
  bus.expectRead(0x47, 8, 1).rx[0] = 0x40;
  bus.expectRead(0x47, 9, 1).rx[0] = 0x04;
  bus.expectRead(0x47, 0x0A, 1).rx[0] = 0x30;
  bus.expectRead(0x47, 0x45, 1).rx[0] = 0;
  t::OperationToken a{}, b{};
  CHECK(first.startInitialize({100, 1000}, a).inProgress());
  CHECK(second.startInitialize({100, 1000}, b).inProgress());
  CHECK(a.value == 1 && b.value == 1); // Instance-local equality is intentional.
  CHECK(first.poll(a, 100).inProgress());
  CHECK(second.poll(b, 101).code == t::Err::I2C_NACK_ADDR);
  CHECK(first.progress().state == t::OperationState::ACTIVE);
  first.clearTransportStats(); // Active operation count and managed evidence survive.
  CHECK(first.transportStats().totalAttempts == 0);
  CHECK(first.progress().transfers == 1);
  CHECK(first.diagnostics().managedTransport.totalSuccess == 1);
  for (unsigned turn = 0; turn < 5; ++turn) CHECK(first.poll(a, 102 + turn, 255).inProgress());
  CHECK(first.poll(a, 107, 0).ok());
  CHECK(bus.complete());
  CHECK(first.diagnostics().initialized && !second.diagnostics().initialized);
  CHECK(first.configurationSnapshot().observedProfile.sourceCurrent == t::SourceCurrent::MA_1500);
  CHECK(second.configurationSnapshot().rawConfigValidMask == 0);
  CHECK(first.transportStats().totalAttempts == 5 && first.transportStats().totalFailures == 0);
  CHECK(second.transportStats().totalAttempts == 1 && second.transportStats().totalFailures == 1);
  CHECK(second.diagnostics().lastFailedTransportStatus.detail == -90003);
  t::OperationResult firstResult{}, secondResult{};
  CHECK(second.takeResult(b, secondResult).ok());
  CHECK(secondResult.status.code == t::Err::I2C_NACK_ADDR);
  CHECK(first.progress().resultPending);
  CHECK(first.takeResult(a, firstResult).ok());
  CHECK(firstResult.status.ok() && firstResult.transfers == 6);
  CHECK(firstResult.configuration.rawConfig08 == 0x40);
  CHECK(firstResult.configuration.observedTimeValid && firstResult.configuration.observedUptimeMs == 103);
  CHECK(firstResult.completedTimeValid && firstResult.completedUptimeMs == 107);
  CHECK(first.end().ok() && second.end().ok());
  CHECK(first.forgetDeviceEvidence().ok());
  CHECK(first.transportStats().totalAttempts == 5);
  CHECK(second.diagnostics().lastFailedTransportStatus.detail == -90003);
}

void tokenBoundaryArithmetic() {
  uint64_t next = 123;
  CHECK(t::detail::nextToken(0, next) && next == 1);
  CHECK(t::detail::nextToken(1, next) && next == 2);
  CHECK(t::detail::nextToken(MAX_TIME - 1, next) && next == MAX_TIME);
  next = 123;
  CHECK(!t::detail::nextToken(MAX_TIME, next) && next == 123);
  // Bounded arithmetic fixture, with no mutable public token seed or exhaustion loop.
}

} // namespace

int main() {
  emptyAndAdmission();
  initializationAndBudgets();
  everyFailureBoundary();
  interruptionAtEveryPhase();
  timeBoundaries();
  reinitializationAndAtomicBinding();
  failedReinitializationAtEveryPhase();
  invalidationAndFrozenResults();
  identityAndUnsafeControls();
  resetEvidenceSurvivesReinitialization();
  independentTerminationEvidence();
  identityPublicationNeedsFreshTime();
  callbackAssociationAndNullContext();
  twoInstancesAndStatistics();
  tokenBoundaryArithmetic();
  failures += runProfileObservationChecks();
  failures += runProfileApplicationChecks();
  failures += runResetRecoveryChecks();
  failures += runProtocolConformanceChecks();
  if (failures != 0) {
    std::printf("FAIL %d of %zu owner checks\n", failures, checks);
    return 1;
  }
  std::printf("PASS %zu owner checks; fixed read-only plans, lifecycle and owner time\n", checks);
  return 0;
}
