/// @file test_owner_scheduling.cpp
/// @brief Public owners sharing terminal bus callbacks through a fixed external scheduler.
#include <TUSB32x/TUSB32x.h>

#include "support/FakeTusb32xDevice.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace {
namespace t = TUSB32x;
using Driver = t::TUSB32x;
using Fake = tusb32x_test::FakeTusb32xDevice;
int failures = 0;
std::size_t checks = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    if (failures < 30) std::printf("FAIL scheduling line %d: %s\n", line, expression);
    ++failures;
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint8_t ADDRESSES[] = {0x60, 0x47};
constexpr uint8_t ID[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};

// One serialized application bus, two distinct physical targets, one owner each.
// Scripts only provide explicit bytes/effects; this is not a silicon emulator.
struct Bus {
  Fake segments[2][2]{};
  std::size_t lengths[2]{};
  std::size_t calls[2]{};
  uint64_t startedUs[2][16]{};
  uint64_t completedUs[2][16]{};
  uint64_t physicalUs = 10000;
  uint64_t forcedCompletionUs = 0;
  bool locked = false;
  const uint8_t* borrowedTx = nullptr;
  uint8_t* borrowedRx = nullptr;

  Fake::Step& read(std::size_t target, uint8_t reg, std::size_t length = 1) {
    CHECK(lengths[target] < 16);
    const auto index = lengths[target]++;
    return segments[target][index / 8].expectRead(ADDRESSES[target], reg, length);
  }
  Fake::Step& write(std::size_t target, uint8_t reg, uint8_t value) {
    CHECK(lengths[target] < 16);
    const auto index = lengths[target]++;
    auto& step = segments[target][index / 8].expectWrite(ADDRESSES[target], reg, value);
    step.applyWrite = true;
    return step;
  }
  void profile(std::size_t target, bool identity = false, uint8_t byte08 = 0) {
    if (identity) {
      auto& id = read(target, 0, 8);
      std::memcpy(id.rx, ID, sizeof(ID));
      if (target == 1) read(target, 0xA0).rx[0] = 2;
    }
    read(target, 8).rx[0] = byte08;
    read(target, 9).rx[0] = 0x14;
    read(target, 0x0A).rx[0] = 0x30;
    read(target, 0x45).rx[0] = 0;
  }
  void service(std::size_t target) {
    // A level remains asserted after each explicit ACK. This scripted assumption
    // exercises bounded service, not lossless IRQ capture or hardware race timing.
    read(target, 9).rx[0] = 0x14;
    read(target, 8).rx[0] = 0;
    read(target, 9).rx[0] = 0x14;
    write(target, 9, 0x14);
    read(target, 9).rx[0] = 0x14;
  }
  bool released() const { return !locked && borrowedTx == nullptr && borrowedRx == nullptr; }
  std::size_t total() const { return calls[0] + calls[1]; }
  bool complete(std::size_t target) const {
    return calls[target] == lengths[target] && segments[target][0].complete() &&
        segments[target][1].complete();
  }
  void clearScripts() {
    CHECK(released());
    for (std::size_t target = 0; target < 2; ++target) {
      CHECK(complete(target));
      segments[target][0] = Fake{};
      segments[target][1] = Fake{};
      lengths[target] = 0;
      calls[target] = 0;
    }
  }
  t::Status invoke(bool writing, uint8_t address, const uint8_t* tx, std::size_t txLength,
                   uint8_t* rx, std::size_t rxLength, uint32_t timeout) {
    CHECK(released());
    locked = true;
    borrowedTx = tx;
    borrowedRx = rx;
    const std::size_t target = address == ADDRESSES[0] ? 0 : 1;
    CHECK(address == ADDRESSES[target]);
    const auto index = calls[target]++;
    t::Status status = t::Status::Error(t::Err::I2C_ERROR, "scheduler script overflow");
    if (index < 16) {
      startedUs[target][index] = physicalUs;
      auto& fake = segments[target][index / 8];
      // A combined read is one terminal callback, even if its selected backend
      // implements pointer selection and receive as two physical transactions.
      status = writing ? Fake::write(address, tx, txLength, timeout, &fake)
                       : Fake::writeRead(address, tx, txLength, rx, rxLength, timeout, &fake);
    } else {
      CHECK(false);
    }
    // Physical sub-ms progress occurs inside a callback, independently of the
    // integer sample supplied before it. Status does not convey this completion.
    physicalUs = forcedCompletionUs != 0 ? forcedCompletionUs : physicalUs + 100;
    if (index < 16) completedUs[target][index] = physicalUs;
    forcedCompletionUs = 0;
    borrowedTx = nullptr;
    borrowedRx = nullptr;
    locked = false;
    return status;
  }
  static t::Status writeCallback(uint8_t address, const uint8_t* tx, std::size_t length,
                                  uint32_t timeout, void* user) {
    return static_cast<Bus*>(user)->invoke(true, address, tx, length, nullptr, 0, timeout);
  }
  static t::Status readCallback(uint8_t address, const uint8_t* tx, std::size_t txLength,
                                 uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* user) {
    return static_cast<Bus*>(user)->invoke(false, address, tx, txLength, rx, rxLength, timeout);
  }
};

enum class Job : uint8_t { INITIALIZE, PROFILE, RECOVERY, SERVICE };
struct Request {
  Job job = Job::INITIALIZE;
  uint32_t id = 0;
};
struct Delivery {
  uint32_t requestId = 0;
  t::OperationResult result{};
};

// This test-owned scheduler has two queued requests and one delivered result per
// owner. A second terminal result remains in the core until delivery has space.
// It adds no retries, threads, framework calls, dynamic storage or shared tokens.
struct Scheduler {
  // Declaration order keeps the borrowed bus/context alive through both drivers'
  // destruction; shutdown still ends each binding explicitly while it is live.
  Bus bus{};
  struct Lane {
    Driver driver{};
    Request queue[2]{};
    std::size_t head = 0;
    std::size_t count = 0;
    Request active{};
    t::OperationToken token{};
    Delivery delivery{};
    bool resultFull = false;
  } lanes[2]{};
  std::size_t nextLane = 0;
  uint32_t nextId = 1;

  Scheduler() {
    for (std::size_t target = 0; target < 2; ++target) {
      t::DriverConfig config{};
      config.variant = target == 0 ? t::Variant::TUSB320 : t::Variant::TUSB320LAI;
      config.address = static_cast<t::DeviceAddress>(ADDRESSES[target]);
      config.i2cWrite = Bus::writeCallback;
      config.i2cWriteRead = Bus::readCallback;
      config.i2cUser = &bus;
      CHECK(lanes[target].driver.bind(config).ok());
    }
    CHECK(bus.total() == 0);
  }
  ~Scheduler() {
    const auto before = bus.total();
    for (auto& lane : lanes) CHECK(lane.driver.end().ok());
    CHECK(bus.total() == before && bus.released());
  }
  t::Status poll(std::size_t target, t::OperationToken token) {
    CHECK(bus.released());
    const auto before = bus.total();
    const auto status = lanes[target].driver.poll(token, bus.physicalUs / 1000, 1);
    CHECK(bus.total() - before <= 1);
    CHECK(bus.released());
    bus.physicalUs += 1000;
    return status;
  }
  t::Status finish(std::size_t target, t::OperationToken token) {
    for (unsigned turn = 0; turn < 16; ++turn) {
      const auto status = poll(target, token);
      if (!status.inProgress()) return status;
    }
    CHECK(false); // Fixed read plans below must finish without retries or waits.
    return t::Status::Error(t::Err::I2C_ERROR, "owner read plan exceeded turn limit");
  }
  bool enqueue(std::size_t target, Job job, uint32_t& id) {
    auto& lane = lanes[target];
    if (lane.count == 2) return false;
    const auto index = (lane.head + lane.count) % 2;
    lane.queue[index] = {job, nextId++};
    id = lane.queue[index].id;
    ++lane.count;
    return true;
  }
  void turn(uint8_t budget) {
    CHECK(bus.released());
    auto& lane = lanes[nextLane];
    nextLane ^= 1;
    const auto before = bus.total();
    // Equal fresh integer ticks are valid; the sample still follows the last callback.
    const uint64_t now = bus.physicalUs / 1000;
    if (!lane.token.valid() && lane.count != 0) {
      lane.active = lane.queue[lane.head];
      lane.head = (lane.head + 1) % 2;
      --lane.count;
      const t::OperationTiming timing{now, now + 1000};
      t::Status status{};
      switch (lane.active.job) {
        case Job::INITIALIZE: status = lane.driver.startInitialize(timing, lane.token); break;
        case Job::PROFILE: status = lane.driver.startReadProfile(timing, lane.token); break;
        case Job::RECOVERY: status = lane.driver.startRecoverDeadBattery(timing, lane.token); break;
        case Job::SERVICE: status = lane.driver.startServiceInterrupt(timing, lane.token); break;
      }
      CHECK(status.inProgress() && lane.token.valid());
      CHECK(bus.total() == before);
    }
    if (lane.token.valid()) {
      const auto status = lane.driver.poll(lane.token, now, budget);
      CHECK(bus.total() - before <= (budget == 0 ? 0U : 1U));
      if (!status.inProgress() && !lane.resultFull) {
        lane.delivery.requestId = lane.active.id;
        CHECK(lane.driver.takeResult(lane.token, lane.delivery.result).ok());
        CHECK(lane.delivery.result.token.value == lane.token.value);
        lane.resultFull = true;
        lane.token = {};
      }
    }
    CHECK(bus.total() - before <= 1);
    CHECK(bus.released());
    bus.physicalUs += 100;
  }
  void turns(unsigned count, uint8_t budget = 255) {
    for (unsigned i = 0; i < count; ++i) turn(budget);
  }
  Delivery take(std::size_t target) {
    auto& lane = lanes[target];
    CHECK(lane.resultFull);
    const auto result = lane.delivery;
    lane.resultFull = false;
    return result;
  }
  void initialize() {
    uint32_t ids[2]{};
    for (std::size_t target = 0; target < 2; ++target) {
      bus.profile(target, true);
      CHECK(enqueue(target, Job::INITIALIZE, ids[target]));
    }
    turns(2, 0);
    CHECK(bus.total() == 0);
    turns(18, 1);
    for (std::size_t target = 0; target < 2; ++target) {
      const auto result = take(target);
      CHECK(result.requestId == ids[target] && result.result.status.ok());
      CHECK(result.result.configuration.state == t::ConfigurationState::KNOWN);
      CHECK(result.result.token.value == 1); // Equal token numbers identify different owner slots.
    }
    bus.clearScripts();
  }
};

void waitingOwnerDoesNotHoldBus() {
  Scheduler s;
  s.initialize();
  s.bus.read(0, 0x45).rx[0] = 0;
  s.bus.write(0, 0x45, 4);
  s.bus.write(0, 0x45, 0);
  s.bus.read(0, 0x45).rx[0] = 0;
  s.bus.profile(1);
  s.bus.profile(1);
  uint32_t recovery = 0;
  uint32_t first = 0;
  uint32_t second = 0;
  CHECK(s.enqueue(0, Job::RECOVERY, recovery));
  // The application has established VDD/I2C readiness and selected source-capable
  // base-part recovery after read-only identity. Only the existing core operation
  // performs that procedure; the scheduler never writes a recovery sequence.
  CHECK(s.enqueue(1, Job::PROFILE, first));
  CHECK(s.enqueue(1, Job::PROFILE, second));
  s.turns(24);
  CHECK(s.lanes[0].driver.progress().waiting && s.bus.calls[0] == 2);
  const auto peerFirst = s.take(1);
  CHECK(peerFirst.requestId == first && peerFirst.result.status.ok());
  CHECK(s.lanes[1].driver.progress().resultPending && s.bus.calls[1] == 8);
  s.turns(2, 0); // Result delivery needs no callback budget.
  const auto peerSecond = s.take(1);
  CHECK(peerSecond.requestId == second && peerSecond.result.status.ok());
  CHECK(peerSecond.result.token.value == peerFirst.result.token.value + 1);
  CHECK(s.lanes[0].driver.progress().waiting && s.bus.calls[0] == 2);
  s.turns(700);
  const auto recovered = s.take(0);
  CHECK(recovered.requestId == recovery && recovered.result.status.ok());
  CHECK(recovered.result.hardwareEffect == t::HardwareEffect::CONFIRMED);
  CHECK(s.bus.startedUs[0][2] - s.bus.completedUs[0][1] >= 30000);
  CHECK(s.bus.complete(0) && s.bus.complete(1));
}

void queueAndResultBackpressure() {
  Scheduler s;
  s.initialize();
  for (unsigned i = 0; i < 3; ++i) s.bus.profile(0);
  s.bus.profile(1);
  uint32_t ids[3]{};
  uint32_t peer = 0;
  CHECK(s.enqueue(0, Job::PROFILE, ids[0]));
  CHECK(s.enqueue(1, Job::PROFILE, peer));
  s.turns(2, 1); // Start both; target zero's two queue positions are now free.
  CHECK(s.enqueue(0, Job::PROFILE, ids[1]));
  CHECK(s.enqueue(0, Job::PROFILE, ids[2]));
  uint32_t rejected = 0xDEADBEEF;
  const auto beforeReject = s.bus.total();
  CHECK(!s.enqueue(0, Job::PROFILE, rejected));
  CHECK(rejected == 0xDEADBEEF && s.lanes[0].count == 2);
  CHECK(s.bus.total() == beforeReject);
  s.turns(24);
  CHECK(s.take(1).requestId == peer);
  CHECK(s.lanes[0].driver.progress().resultPending && s.lanes[0].count == 1);
  t::OperationResult retained{};
  const auto retainedToken = s.lanes[0].token;
  CHECK(s.lanes[0].driver.resultSnapshot(retainedToken, retained).ok());
  const auto before = s.bus.total();
  t::OperationToken unchanged{777};
  CHECK(s.lanes[0].driver.startReadProfile({50, 1000}, unchanged).is(t::Err::RESULT_PENDING));
  CHECK(unchanged.value == 777);
  s.turns(12);
  t::OperationResult again{};
  CHECK(s.lanes[0].driver.resultSnapshot(retainedToken, again).ok());
  CHECK(again.token.value == retained.token.value && again.status.code == retained.status.code);
  CHECK(again.completedUptimeMs == retained.completedUptimeMs && again.transfers == 4);
  CHECK(s.bus.total() == before);
  const auto first = s.take(0);
  CHECK(first.requestId == ids[0]);
  s.turns(2, 0);
  const auto second = s.take(0);
  CHECK(second.requestId == ids[1] && second.result.token.value == retainedToken.value);
  CHECK(second.result.token.value == first.result.token.value + 1);
  s.turns(12);
  const auto third = s.take(0);
  CHECK(third.requestId == ids[2] && third.result.status.ok());
  CHECK(third.result.token.value == second.result.token.value + 1);
  CHECK(s.lanes[0].count == 0 && s.bus.complete(0) && s.bus.complete(1));
}

void failedOrRepeatedIrqOwnerDoesNotStarvePeer() {
  for (const bool fail : {false, true}) {
    Scheduler s;
    s.initialize();
    if (fail) {
      auto& step = s.bus.read(0, 8);
      step.fillLength = 0;
      step.status = {t::Err::I2C_BUS, -2147483001, "shared bus target failure"};
    } else {
      s.bus.service(0);
      s.bus.service(0);
    }
    s.bus.profile(1);
    s.bus.profile(1);
    uint32_t first = 0;
    uint32_t second = 0;
    uint32_t peers[2]{};
    CHECK(s.enqueue(0, fail ? Job::PROFILE : Job::SERVICE, first));
    if (!fail) CHECK(s.enqueue(0, Job::SERVICE, second));
    CHECK(s.enqueue(1, Job::PROFILE, peers[0]));
    CHECK(s.enqueue(1, Job::PROFILE, peers[1]));
    s.turns(30, 1);
    const auto firstResult = s.take(0);
    CHECK(firstResult.requestId == first);
    if (fail) {
      CHECK(firstResult.result.status.code == t::Err::I2C_BUS);
      CHECK(firstResult.result.status.detail == -2147483001);
      CHECK(firstResult.result.transfers == 1 && !firstResult.result.completedTimeValid);
    } else {
      CHECK(firstResult.result.status.ok() && firstResult.result.transfers == 5);
      CHECK(firstResult.result.interrupt.pendingAfterValid &&
            firstResult.result.interrupt.pendingAfter);
    }
    const auto peerFirst = s.take(1);
    CHECK(peerFirst.requestId == peers[0] && peerFirst.result.status.ok());
    CHECK(peerFirst.result.configuration.state == t::ConfigurationState::KNOWN);
    s.turns(2, 0);
    const auto peerSecond = s.take(1);
    CHECK(peerSecond.requestId == peers[1] && peerSecond.result.status.ok());
    if (!fail) {
      const auto secondResult = s.take(0);
      CHECK(secondResult.requestId == second && secondResult.result.status.ok());
      CHECK(secondResult.result.interrupt.pendingAfter && secondResult.result.transfers == 5);
    }
    CHECK(s.bus.complete(0) && s.bus.complete(1));
  }
}

void literalLateWriteKeepsOutcomeAndEffect() {
  for (const auto error : {t::Err::OK, t::Err::I2C_TIMEOUT, t::Err::I2C_BUS, t::Err::I2C_ERROR}) {
    Scheduler s;
    s.initialize();
    auto& owner = s.lanes[0].driver;
    auto desired = owner.configurationSnapshot().verifiedProfile;
    desired.sourceCurrent = t::SourceCurrent::MA_1500;
    auto& write = s.bus.write(0, 8, 0x40);
    write.timeoutMs = 2; // Literal remaining interval: deadline 110 minus callback start 108.
    write.status = {error, error == t::Err::OK ? 0 : -2147483002, "late write callback"};
    t::OperationToken token{};
    CHECK(owner.startApplyProfile(desired, {107, 110}, token).inProgress());
    s.bus.physicalUs = 108000;
    s.bus.forcedCompletionUs = 111000;
    const auto status = owner.poll(token, 108, 255);
    CHECK(status.code == (error == t::Err::OK ? t::Err::IN_PROGRESS : error));
    CHECK(s.bus.physicalUs == 111000 && s.bus.calls[0] == 1 && s.bus.released());
    CHECK(s.bus.segments[0][0].registers[8] == 0x40); // Explicit possible acceptance even on error.
    CHECK(owner.poll(token, s.bus.physicalUs / 1000, 0).code ==
          (error == t::Err::OK ? t::Err::TIMEOUT : error));
    t::OperationResult result{};
    CHECK(owner.takeResult(token, result).ok());
    CHECK(result.status.code == (error == t::Err::OK ? t::Err::TIMEOUT : error));
    const auto expectedEffect = error == t::Err::OK ? t::HardwareEffect::PARTIAL
                                                   : t::HardwareEffect::INDETERMINATE;
    CHECK(result.hardwareEffect == expectedEffect);
    CHECK(result.completedTimeValid == (error == t::Err::OK));
    if (error != t::Err::OK) CHECK(result.status.detail == -2147483002);
    CHECK(result.transfers == 1 && result.configuration.state == t::ConfigurationState::UNKNOWN);
    CHECK(owner.transportStats().totalFailures == (error == t::Err::OK ? 0U : 1U));
    CHECK(s.lanes[1].driver.configurationSnapshot().state == t::ConfigurationState::KNOWN);
    CHECK(s.bus.complete(0) && s.bus.complete(1));
  }
}

void staleRequestsPreserveBothOwners() {
  Scheduler s;
  s.initialize();
  auto& owner = s.lanes[0].driver;
  s.bus.profile(0);
  s.bus.profile(1);
  t::OperationToken token{};
  const auto now = s.bus.physicalUs / 1000;
  CHECK(owner.startReadProfile({now, now + 1000}, token).inProgress());
  const t::OperationToken stale{token.value - 1}; // Previously consumed initialize token.
  uint32_t peer = 0;
  CHECK(s.enqueue(1, Job::PROFILE, peer));
  const auto peerBefore = s.lanes[1].driver.configurationSnapshot();
  t::OperationResult output{};
  output.token.value = 777;
  output.status = {t::Err::I2C_BUS, -12345, "unchanged caller result"};
  for (const bool terminal : {false, true}) {
    if (terminal) {
      CHECK(s.finish(0, token).ok());
      CHECK(owner.progress().resultPending);
    }
    const auto before = s.bus.total();
    const auto progress = owner.progress();
    // Token rejection precedes time validation, so even this old timestamp must
    // neither advance nor terminate the currently represented operation.
    CHECK(owner.poll(stale, 0, 255).is(t::Err::TOKEN_MISMATCH));
    CHECK(owner.cancel(stale).is(t::Err::TOKEN_MISMATCH));
    CHECK(owner.takeResult(stale, output).is(t::Err::TOKEN_MISMATCH));
    CHECK(output.token.value == 777 && output.status.detail == -12345);
    CHECK(owner.progress().token.value == token.value);
    CHECK(owner.progress().state == progress.state);
    CHECK(owner.progress().transfers == progress.transfers);
    CHECK(owner.progress().resultPending == terminal);
    CHECK(s.bus.total() == before && s.bus.released());
    CHECK(s.lanes[1].driver.configurationSnapshot().verifiedGeneration ==
          peerBefore.verifiedGeneration);
    CHECK(s.lanes[1].driver.configurationSnapshot().state == t::ConfigurationState::KNOWN);
  }
  s.turns(12, 1);
  const auto peerResult = s.take(1);
  CHECK(peerResult.requestId == peer && peerResult.result.status.ok());
  CHECK(owner.progress().resultPending);
  CHECK(owner.takeResult(token, output).ok());
  CHECK(output.token.value == token.value && output.status.ok() && output.transfers == 4);
  CHECK(owner.takeResult(token, output).is(t::Err::RESULT_NOT_AVAILABLE));
  CHECK(s.bus.complete(0) && s.bus.complete(1));
}

void cancelledApplyNeedsResultTakeAndSeparateReconcile() {
  for (const bool writeAttempted : {false, true}) {
    Scheduler s;
    s.initialize();
    auto& owner = s.lanes[0].driver;
    auto desired = owner.configurationSnapshot().verifiedProfile;
    desired.sourceCurrent = t::SourceCurrent::MA_1500;
    if (writeAttempted) s.bus.write(0, 8, 0x40);
    s.bus.profile(1);
    const auto now = s.bus.physicalUs / 1000;
    t::OperationToken token{};
    CHECK(owner.startApplyProfile(desired, {now, now + 1000}, token).inProgress());
    if (writeAttempted) CHECK(s.poll(0, token).inProgress());
    const auto beforeCancel = s.bus.total();
    CHECK(owner.cancel(token).ok());
    CHECK(s.bus.total() == beforeCancel && s.bus.released());
    t::OperationResult cancelled{};
    CHECK(owner.resultSnapshot(token, cancelled).ok());
    CHECK(cancelled.token.value == token.value && cancelled.status.is(t::Err::CANCELLED));
    CHECK(cancelled.state == t::OperationState::CANCELLED && !cancelled.completedTimeValid);
    CHECK(cancelled.transfers == (writeAttempted ? 1U : 0U));
    CHECK(cancelled.hardwareEffect == (writeAttempted ? t::HardwareEffect::PARTIAL
                                                    : t::HardwareEffect::NONE));
    CHECK(cancelled.configuration.state == (writeAttempted ? t::ConfigurationState::UNKNOWN
                                                         : t::ConfigurationState::KNOWN));
    CHECK(owner.diagnostics().configurationWriteUnresolved == writeAttempted);
    t::OperationToken rejected{777};
    CHECK(owner.startReconcile({now + 10, now + 1000}, rejected).is(t::Err::RESULT_PENDING));
    CHECK(rejected.value == 777);
    uint32_t peer = 0;
    CHECK(s.enqueue(1, Job::PROFILE, peer));
    s.turns(12, 1);
    CHECK(s.take(1).requestId == peer);
    CHECK(s.bus.calls[0] == (writeAttempted ? 1U : 0U));
    t::OperationResult stillRetained{};
    CHECK(owner.resultSnapshot(token, stillRetained).ok());
    CHECK(stillRetained.status.code == cancelled.status.code &&
          stillRetained.hardwareEffect == cancelled.hardwareEffect &&
          stillRetained.transfers == cancelled.transfers);
    CHECK(owner.takeResult(token, stillRetained).ok()); // Delivery OK, operation CANCELLED.
    CHECK(stillRetained.status.is(t::Err::CANCELLED));
    CHECK(owner.diagnostics().configurationWriteUnresolved == writeAttempted);
    s.bus.clearScripts();

    // A separate application decision observes the actual image. These scripts
    // expect reads only; reconciliation cannot replay the cancelled desired write.
    s.bus.profile(0, true, writeAttempted ? uint8_t{0x40} : uint8_t{0});
    const auto reconcileNow = s.bus.physicalUs / 1000;
    t::OperationToken reconcile{};
    CHECK(owner.startReconcile({reconcileNow, reconcileNow + 1000}, reconcile).inProgress());
    CHECK(s.bus.total() == 0);
    CHECK(s.finish(0, reconcile).code ==
          (writeAttempted ? t::Err::OK : t::Err::CONFIGURATION_MISMATCH));
    t::OperationResult reconciled{};
    CHECK(owner.takeResult(reconcile, reconciled).ok());
    CHECK(reconciled.kind == t::OperationKind::RECONCILE && reconciled.transfers == 5);
    CHECK(reconciled.hardwareEffect == t::HardwareEffect::NONE);
    CHECK(reconciled.configuration.state == t::ConfigurationState::KNOWN);
    CHECK(reconciled.configuration.desiredProfile.sourceCurrent == t::SourceCurrent::MA_1500);
    CHECK(reconciled.configuration.verifiedProfile.sourceCurrent ==
          (writeAttempted ? t::SourceCurrent::MA_1500 : t::SourceCurrent::DEFAULT));
    CHECK(!owner.diagnostics().configurationWriteUnresolved);
    CHECK(s.lanes[1].driver.configurationSnapshot().state == t::ConfigurationState::KNOWN);
    CHECK(s.bus.complete(0) && s.bus.complete(1));
  }
}

void externalPowerEventNeedsFreshProofAndPortPublication() {
  Scheduler s;
  s.initialize();
  auto& owner = s.lanes[0].driver;
  // Publish a real cached port through the public fixed read plan first.
  s.bus.read(0, 9).rx[0] = 0x14;
  s.bus.read(0, 8).rx[0] = 0;
  s.bus.read(0, 9).rx[0] = 0x14;
  t::OperationToken token{};
  const auto firstNow = s.bus.physicalUs / 1000;
  CHECK(owner.startRefreshPort({firstNow, firstNow + 1000}, token).inProgress());
  CHECK(s.finish(0, token).ok());
  t::OperationResult result{};
  CHECK(owner.takeResult(token, result).ok());
  const auto historical = owner.portSnapshot();
  CHECK(!historical.invalidated && historical.observedTimeValid && historical.sequence == 1);
  const auto configBefore = owner.configurationSnapshot();
  s.bus.clearScripts();

  // The notification is application knowledge of a reset/power event. Readiness
  // before subsequent requests is also the application's responsibility.
  CHECK(owner.invalidateDeviceState({t::Err::CONFIGURATION_UNKNOWN, 71,
                                    "application observed VDD loss"}).ok());
  CHECK(s.bus.total() == 0 && s.bus.released());
  CHECK(!owner.diagnostics().initialized && owner.diagnostics().bound);
  CHECK(owner.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  CHECK(owner.portSnapshot().invalidated);
  s.bus.read(0, 0).rx[0] = 0x30;
  uint8_t raw = 0xFF;
  CHECK(owner.readRegisterSync(0, raw).ok() && raw == 0x30);
  CHECK(s.bus.calls[0] == 1 && s.bus.released());
  CHECK(!owner.diagnostics().initialized);
  CHECK(owner.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  CHECK(owner.configurationSnapshot().verifiedGeneration == configBefore.verifiedGeneration);
  CHECK(owner.portSnapshot().invalidated && owner.portSnapshot().sequence == historical.sequence);
  CHECK(owner.portSnapshot().observedUptimeMs == historical.observedUptimeMs);
  CHECK(!historical.invalidated); // Already copied evidence is not rewritten.

  s.bus.profile(1);
  uint32_t peer = 0;
  CHECK(s.enqueue(1, Job::PROFILE, peer));
  s.turns(12, 1);
  const auto peerResult = s.take(1);
  CHECK(peerResult.requestId == peer && peerResult.result.status.ok());
  CHECK(peerResult.result.configuration.state == t::ConfigurationState::KNOWN);
  CHECK(s.bus.calls[0] == 1 && owner.portSnapshot().invalidated);
  s.bus.profile(0, true);
  const auto reconcileNow = s.bus.physicalUs / 1000;
  CHECK(owner.startReconcile({reconcileNow, reconcileNow + 1000}, token).inProgress());
  CHECK(s.finish(0, token).ok());
  CHECK(owner.takeResult(token, result).ok());
  CHECK(result.kind == t::OperationKind::RECONCILE && result.hardwareEffect == t::HardwareEffect::NONE);
  CHECK(owner.diagnostics().initialized);
  CHECK(owner.configurationSnapshot().state == t::ConfigurationState::KNOWN);
  CHECK(owner.portSnapshot().invalidated && owner.portSnapshot().sequence == historical.sequence);
  s.bus.read(0, 9).rx[0] = 0x14;
  s.bus.read(0, 8).rx[0] = 0;
  s.bus.read(0, 9).rx[0] = 0x14;
  const auto refreshNow = s.bus.physicalUs / 1000;
  CHECK(owner.startRefreshPort({refreshNow, refreshNow + 1000}, token).inProgress());
  CHECK(s.finish(0, token).ok());
  CHECK(owner.takeResult(token, result).ok());
  const auto refreshed = owner.portSnapshot();
  CHECK(!refreshed.invalidated && refreshed.sequence == historical.sequence + 1);
  CHECK(refreshed.observedUptimeMs > historical.observedUptimeMs);
  CHECK(refreshed.configurationState == t::ConfigurationState::KNOWN);
  CHECK(s.bus.complete(0) && s.bus.complete(1));
}

void shutdownEndsBothOwnersBeforeBorrowedBusDies() {
  Scheduler s;
  s.initialize();
  auto& owner = s.lanes[0].driver;
  auto desired = owner.configurationSnapshot().verifiedProfile;
  desired.sourceCurrent = t::SourceCurrent::MA_1500;
  s.bus.write(0, 8, 0x40);
  t::OperationToken tokens[2]{};
  const auto now = s.bus.physicalUs / 1000;
  CHECK(owner.startApplyProfile(desired, {now, now + 1000}, tokens[0]).inProgress());
  CHECK(s.lanes[1].driver.startReadProfile({now, now + 1000}, tokens[1]).inProgress());
  CHECK(s.poll(0, tokens[0]).inProgress());
  const auto before = s.bus.total();
  for (std::size_t target = 0; target < 2; ++target) {
    auto& driver = s.lanes[target].driver;
    CHECK(driver.end().ok());
    CHECK(!driver.diagnostics().bound && !driver.diagnostics().initialized);
    CHECK(s.bus.total() == before && s.bus.released());
    t::OperationResult retained{};
    CHECK(driver.resultSnapshot(tokens[target], retained).ok());
    CHECK(retained.token.value == tokens[target].value);
    CHECK(retained.status.is(t::Err::CANCELLED) && !retained.completedTimeValid);
    CHECK(retained.transfers == (target == 0 ? 1U : 0U));
    CHECK(retained.hardwareEffect == (target == 0 ? t::HardwareEffect::PARTIAL
                                                : t::HardwareEffect::NONE));
    CHECK(driver.end().ok()); // Idempotent end preserves the exact retained cancellation.
    t::OperationResult delivered{};
    CHECK(driver.takeResult(tokens[target], delivered).ok());
    CHECK(delivered.token.value == retained.token.value && delivered.status.code == retained.status.code);
    CHECK(delivered.hardwareEffect == retained.hardwareEffect && delivered.transfers == retained.transfers);
    CHECK(driver.takeResult(tokens[target], delivered).is(t::Err::RESULT_NOT_AVAILABLE));
  }
  CHECK(owner.diagnostics().configurationWriteUnresolved);
  CHECK(s.bus.total() == before && s.bus.complete(0) && s.bus.complete(1) && s.bus.released());
}
}  // namespace

int runOwnerSchedulingChecks() {
  waitingOwnerDoesNotHoldBus();
  queueAndResultBackpressure();
  failedOrRepeatedIrqOwnerDoesNotStarvePeer();
  literalLateWriteKeepsOutcomeAndEffect();
  staleRequestsPreserveBothOwners();
  cancelledApplyNeedsResultTakeAndSeparateReconcile();
  externalPowerEventNeedsFreshProofAndPortPublication();
  shutdownEndsBothOwnersBeforeBorrowedBusDies();
  std::printf("Owner scheduling: %zu checks, %d failures\n", checks, failures);
  return failures;
}
