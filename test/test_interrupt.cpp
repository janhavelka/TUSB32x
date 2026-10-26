/// @file test_interrupt.cpp
/// @brief Explicit IRQ writes, finite service, retained observations and coalescing fake races.
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
static_assert(std::is_same_v<decltype(&Driver::startAcknowledgeInterrupt), Start>);
static_assert(std::is_same_v<decltype(&Driver::startServiceInterrupt), Start>);

int failures = 0;
std::size_t checks = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    if (failures < 50) std::printf("FAIL interrupt line %d: %s\n", line, expression);
    ++failures;
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr uint64_t HALF_RANGE = uint64_t{1} << 63;
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

bool sameIrq(const t::InterruptSnapshot& a, const t::InterruptSnapshot& b) {
  return a.observationValid == b.observationValid && a.pendingBefore == b.pendingBefore &&
      a.acknowledgeAttempted == b.acknowledgeAttempted &&
      a.acknowledgeConfirmed == b.acknowledgeConfirmed &&
      a.pendingAfterValid == b.pendingAfterValid && a.pendingAfter == b.pendingAfter;
}

bool samePort(const t::PortSnapshot& a, const t::PortSnapshot& b) {
  return a.rawStatus08 == b.rawStatus08 && a.rawStatus09 == b.rawStatus09 &&
      a.rawValidMask == b.rawValidMask && a.validFields == b.validFields &&
      a.invalidated == b.invalidated && a.attachment == b.attachment &&
      a.localRole == b.localRole && a.peerRole == b.peerRole && a.orientation == b.orientation &&
      a.detectedCurrent == b.detectedCurrent && a.advertisedCurrent == b.advertisedCurrent &&
      a.accessory == b.accessory && a.activeCable == b.activeCable &&
      a.interruptPending == b.interruptPending && a.sequence == b.sequence &&
      a.observedUptimeMs == b.observedUptimeMs && a.observedTimeValid == b.observedTimeValid &&
      a.configGeneration == b.configGeneration && a.configurationState == b.configurationState &&
      a.coherence == b.coherence;
}

struct Fixture {
  Fake fake{};
  Driver device{};
  t::DriverConfig config{};
  const Part& part;
  std::size_t txBytes = 0;
  std::size_t rxBytes = 0;
  bool watchAck = false;
  t::PortSnapshot beforeAck{};
  t::ConfigurationState expectedTrust = t::ConfigurationState::KNOWN;

  // Optional explicit fake-latch model. Reads sample without clearing. W1C
  // clears one coalescing level; an event may reassert it, or a stuck source may
  // hold it. Extended L/A RU-change wording and base CSR wording do not imply
  // that every control write raises IRQ; only explicitly scheduled events do.
  // This assumption is not inferred from TI's RCU abbreviation, and does not
  // establish physical IRQ timing, GPIO polarity, fault clearing or edge capture.
  bool latchModel = false;
  bool latch = true;
  bool stuck = false;
  bool clearOnFailedWrite = false;
  std::size_t assertBefore = NO_CALL;
  std::size_t assertAfter = NO_CALL;
  unsigned simulatedEvents = 0;

  void event() { latch = true; ++simulatedEvents; }

  static t::Status write(uint8_t address, const uint8_t* tx, std::size_t length,
                         uint32_t timeout, void* user) {
    auto& f = *static_cast<Fixture*>(user);
    const auto index = f.fake.calls;
    f.txBytes += length;
    if (index == f.assertBefore) f.event();
    if (f.watchAck) {
      CHECK(samePort(f.device.portSnapshot(), f.beforeAck));
      CHECK(f.device.configurationSnapshot().state == f.expectedTrust);
    }
    const auto status = Fake::write(address, tx, length, timeout, &f.fake);
    if (f.latchModel && tx[0] == 9 && (tx[1] & 0x10U) != 0 &&
        (status.ok() || f.clearOnFailedWrite)) f.latch = f.stuck;
    if (index == f.assertAfter) f.event();
    return status;
  }

  static t::Status read(uint8_t address, const uint8_t* tx, std::size_t txLength,
                        uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* user) {
    auto& f = *static_cast<Fixture*>(user);
    const auto index = f.fake.calls;
    f.txBytes += txLength;
    f.rxBytes += rxLength;
    if (index == f.assertBefore) f.event();
    if (f.latchModel && index < f.fake.stepCount && tx[0] == 9) {
      auto& image = f.fake.script[index].rx[0];
      image = static_cast<uint8_t>((image & 0xEFU) | (f.latch ? 0x10U : 0U));
    }
    const auto status = Fake::writeRead(address, tx, txLength, rx, rxLength, timeout, &f.fake);
    if (index == f.assertAfter) f.event();
    return status;
  }

  explicit Fixture(const Part& selected = PARTS[4]) : part(selected) {
    config.variant = part.variant;
    config.address = static_cast<t::DeviceAddress>(part.address);
    config.i2cWrite = write;
    config.i2cWriteRead = read;
    config.i2cUser = this;
    CHECK(device.bind(config).ok());
  }

  void clear() {
    fake = Fake{};
    txBytes = 0;
    rxBytes = 0;
    watchAck = false;
  }

  void profile(bool identity = true, uint8_t status08 = 0, uint8_t status09 = 0x14) {
    if (identity) {
      auto& id = fake.expectRead(part.address, 0, 8);
      std::memcpy(id.rx, part.is322 ? ID322 : ID320, 8);
      if (!part.base) fake.expectRead(part.address, 0xA0, 1).rx[0] = part.revision;
    }
    fake.expectRead(part.address, 8, 1).rx[0] = status08;
    fake.expectRead(part.address, 9, 1).rx[0] = status09;
    fake.expectRead(part.address, 0x0A, 1).rx[0] = 0x30;
    if (!part.is322) fake.expectRead(part.address, 0x45, 1).rx[0] = 0;
  }

  t::Status poll(t::OperationToken token, uint64_t now = 100, uint8_t budget = 255) {
    const auto before = fake.calls;
    const bool active = device.progress().state == t::OperationState::ACTIVE;
    const auto status = device.poll(token, now, budget);
    CHECK(fake.calls - before <= 1);
    if (active && !status.is(t::Err::INVALID_PARAM))
      CHECK(device.progress().lastPollTransfers == fake.calls - before);
    return status;
  }

  t::Status finish(t::OperationToken token, uint64_t now = 100, uint8_t budget = 255) {
    for (unsigned turn = 0; turn < 20; ++turn) {
      const auto status = poll(token, now, budget);
      if (!status.inProgress()) return status;
    }
    CHECK(false);
    return t::Status::Error(t::Err::I2C_ERROR, "fixed IRQ script did not finish");
  }

  t::OperationResult take(t::OperationToken token) {
    t::OperationResult result{};
    CHECK(device.takeResult(token, result).ok());
    CHECK(result.token.value == token.value);
    return result;
  }

  void initialize() {
    profile();
    t::OperationToken token{};
    CHECK(device.startInitialize({10, 1000}, token).inProgress());
    CHECK(finish(token, 10).ok() && fake.complete());
    CHECK(take(token).configuration.state == t::ConfigurationState::KNOWN);
    clear();
  }

  t::OperationToken start(bool service, uint64_t now = 100, uint64_t deadline = 1000) {
    t::OperationToken token{999};
    const auto before = fake.calls;
    const auto status = service ? device.startServiceInterrupt({now, deadline}, token)
                                : device.startAcknowledgeInterrupt({now, deadline}, token);
    CHECK(status.inProgress() && token.valid() && token.value != 999);
    CHECK(fake.calls == before);
    CHECK(device.progress().kind == (service ? t::OperationKind::SERVICE_INTERRUPT
                                            : t::OperationKind::ACKNOWLEDGE_INTERRUPT));
    return token;
  }

  void script(bool service, uint8_t first = 0xB4, uint8_t final = 0xB4,
              uint8_t after = 0xA4, uint8_t ack = 0x14) {
    fake.expectRead(part.address, 9, 1).rx[0] = first;
    if (service) {
      fake.expectRead(part.address, 8, 1).rx[0] = 0x90;
      fake.expectRead(part.address, 9, 1).rx[0] = final;
    }
    fake.expectWrite(part.address, 9, ack);
    if (service) fake.expectRead(part.address, 9, 1).rx[0] = after;
  }

  t::PortSnapshot refresh(uint64_t now = 50) {
    fake.expectRead(part.address, 9, 1).rx[0] = 0x94;
    fake.expectRead(part.address, 8, 1).rx[0] = 0x10;
    fake.expectRead(part.address, 9, 1).rx[0] = 0x94;
    t::OperationToken token{};
    CHECK(device.startRefreshPort({now, 1000}, token).inProgress());
    CHECK(finish(token, now).ok() && fake.complete());
    const auto image = take(token).port;
    clear();
    return image;
  }

  void stageDesired(t::DrpDutyCycle duty = t::DrpDutyCycle::PERCENT_30) {
    auto desired = device.configurationSnapshot().verifiedProfile;
    desired.drpDutyCycle = duty;
    t::OperationToken token{};
    CHECK(device.startApplyProfile(desired, {30, 1000}, token).inProgress());
    CHECK(device.cancel(token).ok());
    CHECK(take(token).hardwareEffect == t::HardwareEffect::NONE);
    CHECK(device.configurationSnapshot().desiredProfile.drpDutyCycle == duty);
    CHECK(fake.calls == 0);
  }
};

void admission() {
  Driver unbound;
  t::OperationToken token{77};
  for (const Start method : {&Driver::startAcknowledgeInterrupt, &Driver::startServiceInterrupt}) {
    CHECK((unbound.*method)({100, 1000}, token).is(t::Err::NOT_BOUND));
    CHECK((unbound.*method)({100, 100}, token).is(t::Err::INVALID_PARAM));
    CHECK((unbound.*method)({0, HALF_RANGE}, token).is(t::Err::INVALID_PARAM));
    CHECK(token.value == 77);
    for (const auto& part : PARTS) {
      Fixture f(part);
      CHECK((f.device.*method)({100, 1000}, token).is(part.is322 ? t::Err::UNSUPPORTED_FEATURE
                                                                            : t::Err::NOT_INITIALIZED));
      CHECK(f.fake.calls == 0 && token.value == 77);
      f.initialize();
      if (part.is322) {
        CHECK((f.device.*method)({100, 1000}, token).is(t::Err::UNSUPPORTED_FEATURE));
        CHECK(f.fake.calls == 0 && token.value == 77);
        f.fake.expectRead(part.address, 9, 1).rx[0] = 0x08;
        f.fake.expectRead(part.address, 8, 1).rx[0] = 0;
        f.fake.expectRead(part.address, 9, 1).rx[0] = 0x08;
        t::OperationToken refresh{};
        CHECK(f.device.startRefreshPort({100, 1000}, refresh).inProgress());
        CHECK(f.finish(refresh).ok() && f.fake.complete());
        const auto result = f.take(refresh);
        CHECK(result.port.rawStatus09 == 0x08 && !result.port.interruptPending);
        CHECK(!result.interrupt.observationValid && !result.interrupt.acknowledgeAttempted);
      } else {
        CHECK(f.device.end().ok());
        CHECK(f.device.bind(f.config).ok());
        CHECK((f.device.*method)({100, 1000}, token).is(t::Err::NOT_INITIALIZED));
        CHECK(f.fake.calls == 0);
      }
    }
  }
}

void exactWritesAndBounds() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (const bool service : {false, true}) {
      for (const uint8_t budget : {uint8_t{1}, uint8_t{2}, uint8_t{255}}) {
        Fixture f(part);
        f.initialize();
        f.stageDesired();
        const auto historical = f.refresh();
        const uint8_t actual = part.base ? 0xB4 : 0xB5;
        const uint8_t ack = part.base ? 0x14 : 0x15;
        f.script(service, actual, actual, 0x64, ack);
        const auto token = f.start(service);
        t::OperationToken rejected{77};
        CHECK(f.device.startRefreshPort({100, 1000}, rejected).is(t::Err::BUSY));
        CHECK(f.poll(token, 100, 0).inProgress() && f.fake.calls == 0);
        const std::size_t count = service ? 5 : 2;
        for (std::size_t index = 0; index < count; ++index) {
          // Publication precedes the ACK callback and uses the third status byte.
          if (index == (service ? 3U : 1U)) {
            if (service) {
              // A zero-budget turn may publish complete observation, but cannot ACK.
              CHECK(f.poll(token, 130, 0).inProgress());
              CHECK(f.fake.calls == 3);
              const auto image = f.device.portSnapshot();
              CHECK(image.sequence == historical.sequence + 1 && image.rawStatus09 == actual);
              CHECK(image.observedUptimeMs == 100 && image.interruptPending && !image.invalidated);
            }
            f.watchAck = true;
            f.beforeAck = f.device.portSnapshot();
            // Bit0 differs from earlier initialization on extended parts.
            f.expectedTrust = !service && part.base ? t::ConfigurationState::KNOWN
                                                    : t::ConfigurationState::UNKNOWN;
          }
          CHECK(f.poll(token, 100 + index * 10, budget).inProgress());
          CHECK(f.fake.calls == index + 1);
        }
        CHECK(f.fake.complete());
        CHECK(f.txBytes == (service ? 6U : 3U) && f.rxBytes == (service ? 4U : 1U));
        t::OperationResult unavailable{};
        CHECK(f.device.resultSnapshot(token, unavailable).is(t::Err::RESULT_NOT_AVAILABLE));
        CHECK(f.poll(token, 200, 0).ok());
        t::OperationResult retained{};
        CHECK(f.device.resultSnapshot(token, retained).ok());
        CHECK(retained.hardwareEffect == t::HardwareEffect::CONFIRMED && retained.transfers == count);
        CHECK(retained.interrupt.observationValid && retained.interrupt.pendingBefore);
        CHECK(retained.interrupt.acknowledgeAttempted && retained.interrupt.acknowledgeConfirmed);
        CHECK(retained.interrupt.pendingAfterValid == service && !retained.interrupt.pendingAfter);
        CHECK(retained.configuration.desiredProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_30);
        CHECK(retained.completedTimeValid && retained.completedUptimeMs == 200);
        CHECK(f.device.startRefreshPort({200, 1000}, rejected).is(t::Err::RESULT_PENDING));
        CHECK(rejected.value == 77);
        if (!service) CHECK(samePort(retained.port, historical));
        else {
          CHECK(retained.port.rawStatus09 == actual && retained.port.interruptPending);
          CHECK(retained.port.attachment == t::Attachment::LOCAL_SINK);
          CHECK(retained.port.orientation == t::Orientation::CC2);
          // Recheck64 is now LOCAL_SOURCE/CC2; one09 byte cannot replace the
          // complete pre-ACK LOCAL_SINK observation or manufacture a new sequence.
          CHECK(retained.port.sequence == historical.sequence + 1);
        }
        CHECK(sameIrq(f.device.diagnostics().interrupt, retained.interrupt));
        CHECK(f.poll(token, 900).ok() && f.fake.calls == count);
        CHECK(sameIrq(f.take(token).interrupt, retained.interrupt));
        CHECK(sameIrq(f.device.diagnostics().interrupt, retained.interrupt));
      }
    }
  }
}

void actualControlsWithoutTrust() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (const bool service : {false, true}) {
      Fixture f(part);
      f.initialize();
      f.stageDesired();
      const auto historical = f.refresh();
      // Fresh actual duty60 differs from both verified50 and staged30. The
      // observed low bits alone establish the safe image even after trust drops.
      f.script(service, 0x06, 0x06, 0, 0x16);
      const auto token = f.start(service);
      CHECK(f.finish(token).ok() && f.fake.complete());
      const auto result = f.take(token);
      CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
      CHECK(!result.interrupt.pendingBefore && result.interrupt.acknowledgeConfirmed);
      CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED);
      if (!service) CHECK(samePort(result.port, historical));
      // Start a second ACK with already-UNKNOWN applied evidence and fresh controls.
      f.clear();
      f.script(service, 0xB4, 0xB4);
      const auto second = f.start(service);
      CHECK(f.finish(second).ok() && f.fake.complete());
      CHECK(f.take(second).configuration.state == t::ConfigurationState::UNKNOWN);
    }
  }
}

void serviceUsesFinalControlsAndBookends() {
  // The first09 is neither the ACK control source nor the final latch sample.
  // Change RW controls independently, then change attachment/direction and IRQ
  // between bookends. Literal ACK frames must preserve the final actual controls.
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (unsigned vector = 0; vector < 3; ++vector) {
      for (unsigned failure = 0; failure < 3; ++failure) {
        Fixture f(part);
        f.initialize();
        f.stageDesired();
        const auto old = f.refresh();
        const uint8_t first = vector == 0 ? 0x94 : vector == 1 ? 0x84 : 0x74;
        const uint8_t last = static_cast<uint8_t>((vector == 0 ? 0x90U :
            vector == 1 ? 0x70U : 0x80U) | (part.base ? 0x06U : 0x07U));
        const uint8_t after = vector == 2 ? 0x96 : 0x86;
        f.script(true, first, last, after, part.base ? 0x16 : 0x17);
        if (failure != 0) f.fake.script[failure == 1 ? 3 : 4].status =
            t::Status::Error(t::Err::I2C_BUS, "changed-bookend service failure", -1609);
        const auto token = f.start(true);
        for (unsigned read = 0; read < 3; ++read) CHECK(f.poll(token, 100 + read).inProgress());
        CHECK(samePort(f.device.portSnapshot(), old));
        CHECK(f.poll(token, 103, 0).inProgress() && f.fake.calls == 3);
        const auto complete = f.device.portSnapshot();
        CHECK(complete.rawStatus09 == last && complete.rawStatus08 == 0x90);
        CHECK(complete.rawValidMask == 3 && !complete.invalidated);
        CHECK(complete.sequence == old.sequence + 1 && complete.observedUptimeMs == 100);
        CHECK(complete.observedTimeValid && complete.configGeneration == old.configGeneration);
        CHECK(complete.configurationState == t::ConfigurationState::KNOWN);
        CHECK(complete.interruptPending == (vector != 2));
        CHECK(complete.coherence == (vector == 0 ? t::SnapshotCoherence::BOOKENDS_MATCH
                                                 : t::SnapshotCoherence::CHANGED_DURING_READ));
        CHECK(complete.attachment == (vector == 1 ? t::Attachment::LOCAL_SOURCE
                                                 : t::Attachment::LOCAL_SINK));
        CHECK(complete.localRole == (vector == 1 ? t::PortRole::SOURCE : t::PortRole::SINK));
        CHECK(complete.peerRole == (vector == 1 ? t::PortRole::SINK : t::PortRole::SOURCE));
        CHECK(complete.orientation == (vector == 1 ? t::Orientation::CC2 : t::Orientation::CC1));
        const uint16_t dependent = static_cast<uint16_t>(t::PortField::DETECTED_CURRENT) |
            static_cast<uint16_t>(t::PortField::ACCESSORY) |
            static_cast<uint16_t>(t::PortField::ACTIVE_CABLE);
        if (vector != 0) {
          CHECK((complete.validFields & dependent) == 0);
          CHECK(complete.detectedCurrent == t::DetectedCurrent::UNKNOWN);
          CHECK(complete.accessory == t::Accessory::UNKNOWN);
        } else {
          CHECK((complete.validFields & static_cast<uint16_t>(t::PortField::DETECTED_CURRENT)) != 0);
          CHECK((complete.validFields & static_cast<uint16_t>(t::PortField::ACCESSORY)) != 0);
        }
        CHECK((complete.validFields & static_cast<uint16_t>(t::PortField::ADVERTISED_CURRENT)) != 0);
        CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
        const auto status = f.finish(token, 104);
        CHECK(failure == 0 ? status.ok() : status.is(t::Err::I2C_BUS));
        CHECK(f.fake.errors == 0 && f.fake.calls == (failure == 1 ? 4U : 5U));
        const auto result = f.take(token);
        CHECK(samePort(result.port, complete) && samePort(f.device.portSnapshot(), complete));
        CHECK(result.interrupt.observationValid && result.interrupt.pendingBefore == (vector != 2));
        CHECK(result.interrupt.acknowledgeAttempted && result.interrupt.acknowledgeConfirmed == (failure != 1));
        CHECK(result.interrupt.pendingAfterValid == (failure == 0));
        if (failure == 0) CHECK(result.interrupt.pendingAfter == (vector == 2));
        else CHECK(result.status.detail == -1609);
        CHECK(result.hardwareEffect == (failure == 0 ? t::HardwareEffect::CONFIRMED :
            failure == 1 ? t::HardwareEffect::INDETERMINATE : t::HardwareEffect::PARTIAL));
      }
    }
  }
}

void everyAckByteAndStaged02() {
  // Independent literal low-bit tables: base bit0 is reserved; extended bit0
  // is the persistent accessory control. All RO/reserved high bits are omitted.
  constexpr uint8_t BASE_ACK[] = {0x10, 0x10, 0x12, 0x12, 0x14, 0x14, 0x16, 0x16};
  constexpr uint8_t EXTENDED_ACK[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    Fixture f(part);
    f.initialize();
    f.stageDesired(t::DrpDutyCycle::PERCENT_40); // Staged duty encoding02.
    f.script(false, part.base ? 0xB4 : 0xB5, 0, 0, part.base ? 0x14 : 0x15);
    const auto staged = f.start(false);
    CHECK(f.finish(staged).ok() && f.fake.complete());
    CHECK(f.take(staged).configuration.desiredProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_40);
    for (unsigned raw = 0; raw < 256; ++raw) {
      f.clear();
      const auto byte = static_cast<uint8_t>(raw);
      const auto expected = part.base ? BASE_ACK[raw % 8] : EXTENDED_ACK[raw % 8];
      f.script(false, byte, 0, 0, expected);
      const auto token = f.start(false);
      CHECK(f.finish(token).ok() && f.fake.complete());
      const auto result = f.take(token);
      CHECK(result.interrupt.pendingBefore == ((raw & 0x10U) != 0));
      CHECK(result.interrupt.observationValid && result.interrupt.acknowledgeConfirmed);
      CHECK(!result.interrupt.pendingAfterValid && result.hardwareEffect == t::HardwareEffect::CONFIRMED);
      CHECK(result.transfers == 2 && f.txBytes == 3 && f.rxBytes == 1);
    }
  }
}

void negativeControlEvidence() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (const bool service : {false, true}) {
      for (const std::size_t changed : {std::size_t{0}, std::size_t{2}, std::size_t{4}}) {
        if (!service && changed != 0) continue;
        Fixture f(part);
        f.initialize();
        const auto old = f.refresh();
        f.script(service, 0xB4, changed == 2 ? 0xB6 : 0xB4, changed == 4 ? 0xA6 : 0xA4,
                 changed == 2 ? 0x16 : 0x14);
        if (service) f.fake.script[1].rx[0] = 0; // Keep advertisement identical to initialization.
        if (changed == 0) f.fake.script[0].rx[0] = 0xB6;
        const auto token = f.start(service);
        for (std::size_t index = 0; index <= changed; ++index) {
          CHECK(f.poll(token).inProgress());
          CHECK(f.device.configurationSnapshot().state == (index == changed
              ? t::ConfigurationState::UNKNOWN : t::ConfigurationState::KNOWN));
        }
        const auto image = f.device.portSnapshot();
        CHECK(!image.invalidated);
        if (changed < 3) CHECK(samePort(image, old));
        else {
          CHECK(image.rawStatus09 == 0xB4 && image.sequence == old.sequence + 1);
          CHECK(image.configurationState == t::ConfigurationState::KNOWN && image.interruptPending);
        }
        CHECK(f.device.cancel(token).ok());
        const auto result = f.take(token);
        CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
        CHECK(samePort(result.port, image));
        CHECK(!result.interrupt.pendingAfterValid);
        CHECK(f.fake.calls == changed + 1 && f.fake.errors == 0);
      }
    }
  }
}

void unrelatedProfileWritesPreserveLatch() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (unsigned field = 0; field < 8; ++field) {
      if (part.base && (field == 2 || field == 3 || field == 5)) continue;
      Fixture f(part);
      f.initialize();
      auto desired = f.device.configurationSnapshot().verifiedProfile;
      f.latchModel = true;
      f.latch = true;
      switch (field) {
        case 0:
          desired.sourceCurrent = t::SourceCurrent::MA_1500;
          f.fake.expectWrite(part.address, 8, 0x40);
          break;
        case 1:
          desired.drpDutyCycle = t::DrpDutyCycle::PERCENT_30;
          f.fake.expectWrite(part.address, 9, 0x00); // IRQ10 must remain zero in the frame.
          break;
        case 2:
          desired.disableUfpAccessory = true;
          f.fake.expectWrite(part.address, 9, 0x05);
          break;
        case 3:
          desired.sourcePreference = t::SourcePreference::TRY_SNK;
          f.fake.expectWrite(part.address, 0x0A, 0x32);
          break;
        case 4:
          desired.ccDebounce = part.base ? t::CcDebounce::MS_116 : t::CcDebounce::MS_118;
          f.fake.expectWrite(part.address, 0x0A, 0x70);
          break;
        case 5:
          desired.disableTerm = true;
          f.fake.expectWrite(part.address, 0x0A, 0x31);
          break;
        case 6:
          desired.disableRdRp = true;
          f.fake.expectWrite(part.address, 0x45, 0x04);
          break;
        default:
          desired.portMode = t::PortMode::UFP;
          if (part.base) {
            f.fake.expectRead(part.address, 9, 1).rx[0] = 0x14;
            f.fake.expectWrite(part.address, 0x0A, 0x10);
          } else {
            f.fake.expectWrite(part.address, 0x0A, 0x31);
            f.fake.expectWrite(part.address, 0x0A, 0x11);
            f.fake.expectWrite(part.address, 0x0A, 0x10);
          }
          break;
      }
      t::OperationToken token{};
      CHECK(f.device.startApplyProfile(desired, {100, 1000}, token).inProgress());
      uint64_t now = 100;
      for (unsigned turn = 0; turn < 20 && f.fake.calls < f.fake.stepCount; ++turn) {
        CHECK(f.poll(token, now).inProgress());
        if (f.device.progress().readyAtValid) now = f.device.progress().readyAtMs;
        CHECK(f.latch);
      }
      CHECK(f.fake.complete() && f.latch);
      // Cancel after the setting frames: complete F behavior is covered by the
      // application suite. This test isolates pending-latch preservation across
      // every individual public profile field, including all mode-stage writes.
      CHECK(f.device.cancel(token).ok());
      const auto result = f.take(token);
      CHECK(result.hardwareEffect == t::HardwareEffect::PARTIAL);
      CHECK(!result.interrupt.observationValid && !result.interrupt.acknowledgeAttempted);
      f.clear();
      CHECK(f.refresh(200).interruptPending && f.latch);
      CHECK(!f.device.diagnostics().interrupt.acknowledgeAttempted);
    }
  }
}

void recheckContradictionRetainsPublication() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (unsigned action = 0; action < 4; ++action) {
      Fixture f(part);
      f.initialize();
      const auto old = f.refresh();
      f.script(true, 0x94, 0x94, 0x86);
      f.fake.script[1].rx[0] = 0x10; // Advertisement agrees with verified00.
      const auto token = f.start(true);
      for (unsigned index = 0; index < 5; ++index) {
        CHECK(f.poll(token, 100 + index).inProgress());
        CHECK(f.device.configurationSnapshot().state == (index == 4
            ? t::ConfigurationState::UNKNOWN : t::ConfigurationState::KNOWN));
      }
      const auto complete = f.device.portSnapshot();
      CHECK(complete.sequence == old.sequence + 1 && complete.rawStatus09 == 0x94);
      CHECK(complete.configurationState == t::ConfigurationState::KNOWN && !complete.invalidated);
      CHECK(complete.observedUptimeMs == 100 && complete.interruptPending);
      if (action == 1) CHECK(f.device.cancel(token).ok());
      else if (action == 2) CHECK(f.poll(token, 1000).is(t::Err::TIMEOUT));
      else {
        if (action == 3) {
          CHECK(f.poll(token, 103).is(t::Err::INVALID_PARAM));
          CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
          CHECK(samePort(f.device.portSnapshot(), complete));
        }
        CHECK(f.poll(token, 105, 0).ok());
      }
      const auto result = f.take(token);
      CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
      CHECK(samePort(result.port, complete) && samePort(f.device.portSnapshot(), complete));
      CHECK(result.interrupt.pendingBefore && result.interrupt.acknowledgeConfirmed);
      const bool success = action == 0 || action == 3;
      CHECK(result.interrupt.pendingAfterValid == success && !result.interrupt.pendingAfter);
      CHECK(result.hardwareEffect == (success ? t::HardwareEffect::CONFIRMED : t::HardwareEffect::PARTIAL));
      CHECK(f.fake.complete() && f.fake.calls == 5);
    }
  }
}

void everyTransportFailure() {
  constexpr t::Err ERRORS[] = {t::Err::I2C_ERROR, t::Err::I2C_NACK_ADDR, t::Err::I2C_NACK_DATA,
      t::Err::I2C_TIMEOUT, t::Err::I2C_BUS, t::Err::I2C_BUSY};
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (const bool service : {false, true}) {
      const std::size_t count = service ? 5 : 2;
      const std::size_t ackIndex = service ? 3 : 1;
      for (std::size_t boundary = 0; boundary < count; ++boundary) {
        for (const auto code : ERRORS) {
          for (const bool accepted : {false, true}) {
            if (boundary != ackIndex && accepted) continue;
            Fixture f(part);
            f.initialize();
            const auto old = f.refresh();
            f.script(service);
            auto& failed = f.fake.script[boundary];
            failed.status = t::Status::Error(code, "IRQ injected transport failure", -2147483000);
            failed.fillLength = 0; // A failed/truncated read cannot create a complete sample.
            f.latchModel = true;
            f.clearOnFailedWrite = accepted;
            const auto token = f.start(service);
            CHECK(f.finish(token).is(code));
            CHECK(f.fake.calls == boundary + 1 && f.fake.errors == 0);
            const auto result = f.take(token);
            CHECK(result.status.is(code) && result.status.detail == -2147483000);
            CHECK(!result.completedTimeValid && result.transfers == boundary + 1);
            CHECK(result.hardwareEffect == (boundary < ackIndex ? t::HardwareEffect::NONE
                : boundary == ackIndex ? t::HardwareEffect::INDETERMINATE : t::HardwareEffect::PARTIAL));
            CHECK(result.interrupt.observationValid == (boundary >= ackIndex));
            CHECK(result.interrupt.acknowledgeAttempted == (boundary >= ackIndex));
            CHECK(result.interrupt.acknowledgeConfirmed == (boundary > ackIndex));
            CHECK(!result.interrupt.pendingAfterValid);
            if (service && boundary >= ackIndex) {
              CHECK(result.port.sequence == old.sequence + 1 && result.port.rawStatus09 == 0xB4);
              CHECK(result.port.observedUptimeMs == 100 && result.port.interruptPending);
              CHECK(!result.port.invalidated);
            } else CHECK(samePort(result.port, old));
            if (boundary == ackIndex) CHECK(f.latch == !accepted);
            CHECK(sameIrq(f.device.diagnostics().interrupt, result.interrupt));
            CHECK(f.device.transportStats().lastStatus.is(code));
          }
        }
      }
    }
  }
}

void interruptedBoundaries() {
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (const bool service : {false, true}) {
      const std::size_t count = service ? 5 : 2;
      const std::size_t ackIndex = service ? 3 : 1;
      for (std::size_t completed = 0; completed <= count; ++completed) {
        for (unsigned action = 0; action < 4; ++action) {
          Fixture f(part);
          f.initialize();
          const auto old = f.refresh();
          f.script(service);
          const auto token = f.start(service);
          for (std::size_t index = 0; index < completed; ++index)
            CHECK(f.poll(token).inProgress());
          const auto calls = f.fake.calls;
          if (action == 0) CHECK(f.device.cancel(token).ok());
          else if (action == 1) CHECK(f.device.end().ok());
          else if (action == 2) CHECK(f.poll(token, 1000).is(t::Err::TIMEOUT));
          else CHECK(f.device.invalidateDeviceState(t::Status::Error(
              t::Err::CONFIGURATION_UNKNOWN, "external IRQ test change", -97)).ok());
          CHECK(f.fake.calls == calls && calls == completed && f.fake.errors == 0);
          t::OperationResult saved{};
          CHECK(f.device.resultSnapshot(token, saved).ok());
          CHECK(saved.status.is(action == 2 ? t::Err::TIMEOUT : t::Err::CANCELLED));
          CHECK(saved.completedTimeValid == (action == 2));
          CHECK(saved.transfers == completed);
          CHECK(saved.hardwareEffect == (completed <= ackIndex ? t::HardwareEffect::NONE
                                                              : t::HardwareEffect::PARTIAL));
          CHECK(saved.interrupt.observationValid == (completed > ackIndex));
          CHECK(saved.interrupt.acknowledgeAttempted == (completed > ackIndex));
          CHECK(saved.interrupt.acknowledgeConfirmed == (completed > ackIndex));
          // The status-only successful recheck needs a fresh accepted owner turn.
          // Cancellation or deadline equality before that turn cannot publish it.
          CHECK(!saved.interrupt.pendingAfterValid);
          // Three successful callbacks still need a fresh owner turn to publish.
          const bool published = service && completed > 3;
          if (published) {
            CHECK(saved.port.sequence == old.sequence + 1 && saved.port.observedUptimeMs == 100);
            CHECK(saved.port.interruptPending && saved.port.rawStatus09 == 0xB4);
          } else {
            CHECK(saved.port.sequence == old.sequence && saved.port.observedUptimeMs == old.observedUptimeMs);
            CHECK(saved.port.rawStatus09 == old.rawStatus09);
          }
          CHECK(saved.port.invalidated == (action == 1 || action == 3));
          CHECK(f.device.cancel(token).ok());
          CHECK(sameIrq(f.take(token).interrupt, saved.interrupt));
          CHECK(sameIrq(f.device.diagnostics().interrupt, saved.interrupt));
          CHECK(f.fake.calls == calls);
        }
      }
    }
  }
}

void finiteLatchRaces() {
  // Event scheduling is an explicit test model: reads do not consume a latch,
  // events before W1C can coalesce, events after it reassert. No GPIO callback is
  // configured: bounded service must work entirely from register polling.
  for (const auto& part : PARTS) {
    if (part.is322) continue;
    for (unsigned race = 0; race < 7; ++race) {
      Fixture f(part);
      f.initialize();
      f.script(true);
      f.latchModel = true;
      f.latch = race != 0;
      if (race == 0) f.assertBefore = 2; // Between observation bookends.
      if (race == 1) f.assertBefore = 3; // Before ACK: second event coalesces.
      if (race == 2) f.assertAfter = 3;  // During the ACK callback after clear.
      if (race == 3) f.assertBefore = 4; // Before recheck.
      if (race == 4) f.assertAfter = 4;  // After sampled recheck: unseen later event.
      if (race == 5) f.stuck = true;
      if (race == 6) { f.event(); f.event(); f.event(); } // Four events, one level.
      const auto token = f.start(true);
      CHECK(f.finish(token).ok() && f.fake.complete());
      const auto result = f.take(token);
      const bool pendingAfter = race == 2 || race == 3 || race == 5;
      CHECK(result.interrupt.observationValid && result.interrupt.pendingBefore);
      CHECK(result.interrupt.acknowledgeConfirmed && result.interrupt.pendingAfterValid);
      CHECK(result.interrupt.pendingAfter == pendingAfter);
      CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED);
      CHECK(result.port.rawStatus09 == 0xB4 && result.port.interruptPending);
      CHECK(result.port.rawStatus08 == 0x90 && result.port.sequence == 1);
      CHECK(f.fake.calls == 5 && f.txBytes == 6 && f.rxBytes == 4);
      if (race == 4) CHECK(f.latch && !result.interrupt.pendingAfter);
      if (race == 6) CHECK(f.simulatedEvents == 3 && !result.interrupt.pendingAfter);
    }
  }
}

void timeAndHistory() {
  Fixture f;
  f.initialize();
  f.script(true, 0x94, 0x94, 0x84);
  const auto token = f.start(true, std::numeric_limits<uint64_t>::max() - 10, 20);
  CHECK(f.poll(token, std::numeric_limits<uint64_t>::max() - 10).inProgress());
  CHECK(f.poll(token, std::numeric_limits<uint64_t>::max() - 11).is(t::Err::INVALID_PARAM));
  CHECK(f.fake.calls == 1);
  CHECK(f.finish(token, 0).ok() && f.fake.complete());
  const auto result = f.take(token);
  CHECK(result.port.observedUptimeMs == std::numeric_limits<uint64_t>::max() - 10);
  CHECK(result.interrupt.pendingBefore && !result.interrupt.pendingAfter);
  CHECK(sameIrq(f.device.diagnostics().interrupt, result.interrupt));
  f.clear();
  f.refresh(200);
  CHECK(sameIrq(f.device.diagnostics().interrupt, result.interrupt));
  f.device.clearTransportStats();
  CHECK(sameIrq(f.device.diagnostics().interrupt, result.interrupt));
  f.clear();
  f.script(false, 0x84);
  const auto second = f.start(false, 300, 305);
  for (auto& step : f.fake.script) step.timeoutMs = 5;
  CHECK(f.poll(second, 300).inProgress());
  CHECK(f.poll(second, 300).inProgress());
  CHECK(f.poll(second, 305, 0).is(t::Err::TIMEOUT));
  const auto late = f.take(second);
  CHECK(late.hardwareEffect == t::HardwareEffect::PARTIAL && late.interrupt.acknowledgeConfirmed);
  CHECK(!late.interrupt.pendingBefore && !late.interrupt.pendingAfterValid);
  CHECK(sameIrq(f.device.diagnostics().interrupt, late.interrupt));
  CHECK(f.fake.complete());
}

void incompleteWorkRetainsIrqHistory() {
  Fixture f;
  f.initialize();
  f.script(true);
  const auto first = f.start(true);
  CHECK(f.finish(first).ok() && f.fake.complete());
  const auto successful = f.take(first);
  for (const bool service : {false, true}) {
    for (const unsigned stop : {0U, 1U, 2U}) {
      if (!service && stop == 2) continue;
      f.clear();
      f.script(service);
      const auto token = f.start(service, 200, 1000);
      CHECK(sameIrq(f.device.diagnostics().interrupt, successful.interrupt));
      for (unsigned index = 0; index < stop; ++index)
        CHECK(f.poll(token, 200).inProgress());
      CHECK(f.device.cancel(token).ok());
      const auto incomplete = f.take(token);
      CHECK(!incomplete.interrupt.observationValid && !incomplete.interrupt.acknowledgeAttempted);
      CHECK(sameIrq(f.device.diagnostics().interrupt, successful.interrupt));
    }
  }
  // A fresh zero-budget turn can establish the new ACK observation without
  // attempting the write; that new evidence then replaces diagnostic history.
  f.clear();
  f.script(false, 0x84);
  const auto token = f.start(false, 300, 1000);
  CHECK(f.poll(token, 300).inProgress());
  CHECK(f.poll(token, 301, 0).inProgress() && f.fake.calls == 1);
  CHECK(f.device.cancel(token).ok());
  const auto cancelled = f.take(token);
  CHECK(cancelled.interrupt.observationValid && !cancelled.interrupt.pendingBefore);
  CHECK(!cancelled.interrupt.acknowledgeAttempted && cancelled.hardwareEffect == t::HardwareEffect::NONE);
  CHECK(sameIrq(f.device.diagnostics().interrupt, cancelled.interrupt));
}

} // namespace

int runInterruptTests() {
  admission();
  exactWritesAndBounds();
  actualControlsWithoutTrust();
  serviceUsesFinalControlsAndBookends();
  everyAckByteAndStaged02();
  negativeControlEvidence();
  unrelatedProfileWritesPreserveLatch();
  recheckContradictionRetainsPublication();
  everyTransportFailure();
  interruptedBoundaries();
  finiteLatchRaces();
  timeAndHistory();
  incompleteWorkRetainsIrqHistory();
  if (failures == 0) std::printf("PASS %zu interrupt checks; bounded ACK/service and retained IRQ facts\n", checks);
  else std::printf("FAIL %d of %zu interrupt checks\n", failures, checks);
  return failures;
}
