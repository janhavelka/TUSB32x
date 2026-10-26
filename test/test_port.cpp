/// @file test_port.cpp
/// @brief Public fixed port acquisitions, independent status vectors and historical invalidation.
#include <TUSB32x/TUSB32x.h>

#include "support/FakeTusb32xDevice.h"
#include "TUSB32xOwnerTime.h"

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
static_assert(std::is_same_v<decltype(&Driver::startRefreshPort), Start>);

int failures = 0;
std::size_t checks = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    if (failures < 40) std::printf("FAIL port line %d: %s\n", line, expression);
    ++failures;
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr uint64_t HALF_RANGE = uint64_t{1} << 63;
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

bool has(const t::PortSnapshot& image, t::PortField field) {
  return (image.validFields & static_cast<uint16_t>(field)) != 0;
}

bool sameHistory(const t::PortSnapshot& a, const t::PortSnapshot& b) {
  return a.rawStatus08 == b.rawStatus08 && a.rawStatus09 == b.rawStatus09 &&
      a.rawValidMask == b.rawValidMask && a.validFields == b.validFields &&
      a.attachment == b.attachment && a.localRole == b.localRole &&
      a.peerRole == b.peerRole && a.orientation == b.orientation &&
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
  bool checkWriteInvalidation = false;
  t::PortSnapshot historical{};

  static t::Status write(uint8_t address, const uint8_t* tx, std::size_t length,
                         uint32_t timeout, void* user) {
    auto& fixture = *static_cast<Fixture*>(user);
    if (fixture.checkWriteInvalidation) {
      CHECK(fixture.device.portSnapshot().invalidated);
      CHECK(sameHistory(fixture.device.portSnapshot(), fixture.historical));
    }
    return Fake::write(address, tx, length, timeout, &fixture.fake);
  }

  static t::Status writeRead(uint8_t address, const uint8_t* tx, std::size_t txLength,
                             uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* user) {
    auto& fixture = *static_cast<Fixture*>(user);
    return Fake::writeRead(address, tx, txLength, rx, rxLength, timeout, &fixture.fake);
  }

  explicit Fixture(const Part& selected = PARTS[4]) : part(selected) {
    config.variant = part.variant;
    config.address = static_cast<t::DeviceAddress>(part.address);
    config.i2cWrite = write;
    config.i2cWriteRead = writeRead;
    config.i2cUser = this;
    CHECK(device.bind(config).ok());
  }

  void port(uint8_t first09 = 0x94, uint8_t byte08 = 0x90, uint8_t final09 = 0x94) {
    fake = Fake{};
    fake.expectRead(part.address, 9, 1).rx[0] = first09;
    fake.expectRead(part.address, 8, 1).rx[0] = byte08;
    fake.expectRead(part.address, 9, 1).rx[0] = final09;
  }

  void profile(bool identity = true, uint8_t byte08 = 0x80, uint8_t byte09 = 0x14,
               uint8_t byte0A = 0x30, uint8_t byte45 = 0) {
    if (identity) {
      auto& id = fake.expectRead(part.address, 0, 8);
      std::memcpy(id.rx, part.is322 ? ID322 : ID320, 8);
      if (!part.base) fake.expectRead(part.address, 0xA0, 1).rx[0] = part.revision;
    }
    fake.expectRead(part.address, 8, 1).rx[0] = byte08;
    fake.expectRead(part.address, 9, 1).rx[0] = byte09;
    fake.expectRead(part.address, 0x0A, 1).rx[0] = byte0A;
    if (!part.is322) fake.expectRead(part.address, 0x45, 1).rx[0] = byte45;
  }

  t::OperationToken start(Start method = &Driver::startRefreshPort,
                          uint64_t now = 100, uint64_t deadline = 1000) {
    t::OperationToken token{std::numeric_limits<uint64_t>::max()};
    const auto before = fake.calls;
    CHECK((device.*method)({now, deadline}, token).inProgress());
    CHECK(token.valid() && token.value != std::numeric_limits<uint64_t>::max() && fake.calls == before);
    return token;
  }

  t::Status poll(t::OperationToken token, uint64_t now = 100, uint8_t budget = 255) {
    const auto before = fake.calls;
    const bool active = device.progress().state == t::OperationState::ACTIVE;
    const auto status = device.poll(token, now, budget);
    CHECK(fake.calls - before <= 1);
    if (active) CHECK(device.progress().lastPollTransfers == fake.calls - before);
    else CHECK(fake.calls == before); // A retained result freezes its previous-turn counters.
    return status;
  }

  t::Status finish(t::OperationToken token, uint64_t now = 100) {
    for (unsigned turn = 0; turn < 24; ++turn) {
      const auto status = poll(token, now);
      if (!status.inProgress()) return status;
      const auto progress = device.progress();
      if (progress.waiting && progress.readyAtValid) now = progress.readyAtMs;
    }
    CHECK(false);
    return t::Status::Error(t::Err::I2C_ERROR, "bounded test script did not finish");
  }

  t::OperationResult take(t::OperationToken token) {
    t::OperationResult result{};
    CHECK(device.takeResult(token, result).ok());
    CHECK(result.token.value == token.value);
    return result;
  }

  t::PortSnapshot refresh(uint64_t now = 100) {
    port();
    const auto token = start(&Driver::startRefreshPort, now, now + 1000);
    CHECK(finish(token, now).ok());
    CHECK(fake.complete());
    const auto result = take(token);
    CHECK(result.hardwareEffect == t::HardwareEffect::NONE);
    CHECK(result.kind == t::OperationKind::REFRESH_PORT && result.transfers == 3);
    return result.port;
  }

  void initialize() {
    fake = Fake{};
    profile();
    const auto token = start(&Driver::startInitialize);
    CHECK(finish(token).ok() && fake.complete());
    CHECK(take(token).configuration.state == t::ConfigurationState::KNOWN);
  }
};

void admissionAndPublication() {
  Driver unbound;
  t::OperationToken rejected{77};
  CHECK(unbound.startRefreshPort({100, 1000}, rejected).is(t::Err::NOT_BOUND));
  CHECK(unbound.startRefreshPort({100, 100}, rejected).is(t::Err::INVALID_PARAM));
  CHECK(unbound.startRefreshPort({0, HALF_RANGE}, rejected).is(t::Err::INVALID_PARAM));
  CHECK(rejected.value == 77);
  for (const auto& part : PARTS) {
    for (const uint8_t budget : {uint8_t{1}, uint8_t{2}, uint8_t{255}}) {
      Fixture f(part);
      const auto empty = f.device.portSnapshot();
      CHECK(empty.invalidated && empty.sequence == 0 && empty.rawValidMask == 0);
      CHECK(empty.attachment == t::Attachment::UNKNOWN && !empty.observedTimeValid);
      f.port(0x90, 0x90, 0x9F); // Low bits, including raw 322 fault, do not break E0 match.
      const auto token = f.start();
      CHECK(f.device.progress().kind == t::OperationKind::REFRESH_PORT);
      CHECK(f.device.startRefreshPort({100, 1000}, rejected).is(t::Err::BUSY));
      CHECK(f.poll(token, 110, 0).inProgress() && f.fake.calls == 0);
      for (unsigned index = 0; index < 3; ++index) {
        CHECK(f.poll(token, 120 + index * 10, budget).inProgress());
        CHECK(f.fake.calls == index + 1);
        CHECK(sameHistory(f.device.portSnapshot(), empty));
      }
      CHECK(f.fake.complete());
      t::OperationResult unchanged{};
      unchanged.token.value = 88;
      CHECK(f.device.resultSnapshot(token, unchanged).is(t::Err::RESULT_NOT_AVAILABLE));
      CHECK(unchanged.token.value == 88);
      CHECK(f.poll(token, 900, 0).ok());
      CHECK(f.fake.calls == 3);
      const auto image = f.device.portSnapshot();
      CHECK(!image.invalidated && image.rawValidMask == 3 && image.sequence == 1);
      CHECK(image.rawStatus08 == 0x90 && image.rawStatus09 == 0x9F);
      CHECK(image.coherence == t::SnapshotCoherence::BOOKENDS_MATCH);
      CHECK(image.observedTimeValid && image.observedUptimeMs == 120);
      CHECK(image.configGeneration == 0 && image.configurationState == t::ConfigurationState::UNCONFIGURED);
      CHECK(!f.device.diagnostics().initialized && image.interruptPending);
      CHECK(f.device.startRefreshPort({900, 1000}, rejected).is(t::Err::RESULT_PENDING));
      t::OperationResult retained{};
      CHECK(f.device.resultSnapshot(token, retained).ok());
      CHECK(sameHistory(retained.port, image) && retained.completedUptimeMs == 900);
      CHECK(retained.status.ok() && retained.completedTimeValid && retained.transfers == 3);
      CHECK(f.device.end().ok());
      CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), image));
      CHECK(sameHistory(f.take(token).port, image));
      CHECK(f.device.takeResult(token, retained).is(t::Err::RESULT_NOT_AVAILABLE));
      CHECK(f.fake.calls == 3 && rejected.value == 77);
    }
  }
}

// Literal status tables derive from TI B p17/18, L/A Tables8/9 and V Tables6-8/9.
// They deliberately do not call decodeRegister or any private production codec.
constexpr t::Attachment ATTACHMENTS[] = {t::Attachment::DETACHED, t::Attachment::LOCAL_SOURCE,
    t::Attachment::LOCAL_SINK, t::Attachment::ACCESSORY};
constexpr t::SourceCurrent ADVERTISEMENTS[] = {t::SourceCurrent::DEFAULT,
    t::SourceCurrent::MA_1500, t::SourceCurrent::MA_3000};
constexpr t::DetectedCurrent CURRENTS[] = {t::DetectedCurrent::DEFAULT,
    t::DetectedCurrent::MA_1500, t::DetectedCurrent::CHARGE_THROUGH_500_MA,
    t::DetectedCurrent::MA_3000};
constexpr t::Accessory BASE_ACCESSORIES[] = {t::Accessory::NONE, t::Accessory::UNKNOWN,
    t::Accessory::UNKNOWN, t::Accessory::UNKNOWN, t::Accessory::AUDIO,
    t::Accessory::CHARGE_THROUGH, t::Accessory::DEBUG, t::Accessory::UNKNOWN};
constexpr t::Accessory EXTENDED_ACCESSORIES[] = {t::Accessory::NONE, t::Accessory::UNKNOWN,
    t::Accessory::UNKNOWN, t::Accessory::UNKNOWN, t::Accessory::AUDIO,
    t::Accessory::CHARGE_THROUGH, t::Accessory::DEBUG_LOCAL_DFP, t::Accessory::DEBUG_LOCAL_UFP};

void independentStatusEncodings() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    uint64_t publications = 0;
    for (unsigned attachment = 0; attachment < 4; ++attachment) {
      for (unsigned direction = 0; direction < 2; ++direction) {
        for (unsigned raw08 = 0; raw08 < 256; ++raw08) {
          const auto raw09 = static_cast<uint8_t>(attachment * 64 + direction * 32 + 0x1F);
          f.port(raw09, static_cast<uint8_t>(raw08), raw09);
          const auto token = f.start();
          CHECK(f.finish(token).ok() && f.fake.complete());
          const auto image = f.take(token).port;
          const unsigned accessoryCode = (raw08 / 2) % 8;
          const unsigned currentCode = (raw08 / 16) % 4;
          const auto accessoryCodeValue = part.base ? BASE_ACCESSORIES[accessoryCode]
                                                   : EXTENDED_ACCESSORIES[accessoryCode];
          CHECK(image.sequence == ++publications && !image.invalidated);
          CHECK(image.rawStatus08 == raw08 && image.rawStatus09 == raw09 && image.rawValidMask == 3);
          CHECK(image.attachment == ATTACHMENTS[attachment] && has(image, t::PortField::ATTACHMENT));
          CHECK(image.interruptPending && has(image, t::PortField::INTERRUPT_PENDING));
          CHECK(has(image, t::PortField::ADVERTISED_CURRENT) == (raw08 < 192));
          if (raw08 < 192) CHECK(image.advertisedCurrent == ADVERTISEMENTS[raw08 / 64]);
          CHECK(image.coherence == t::SnapshotCoherence::BOOKENDS_MATCH);
          // The paired applicability assertions are below; raw field codes alone
          // must never be promoted to usable current, orientation or VCONN.
          if (attachment == 0) {
            CHECK(image.localRole == t::PortRole::NOT_APPLICABLE);
            CHECK(image.peerRole == t::PortRole::NOT_APPLICABLE);
            CHECK(image.orientation == t::Orientation::NOT_APPLICABLE);
            CHECK(image.detectedCurrent == t::DetectedCurrent::NOT_APPLICABLE);
          } else if (attachment == 1 || attachment == 2) {
            CHECK(image.localRole == (attachment == 1 ? t::PortRole::SOURCE : t::PortRole::SINK));
            CHECK(image.peerRole == (attachment == 1 ? t::PortRole::SINK : t::PortRole::SOURCE));
            CHECK(image.orientation == (direction == 0 ? t::Orientation::CC1 : t::Orientation::CC2));
          } else {
            CHECK(image.localRole == t::PortRole::NOT_APPLICABLE);
            CHECK(image.peerRole == t::PortRole::NOT_APPLICABLE);
            CHECK(image.orientation == t::Orientation::UNKNOWN);
          }
          CHECK(has(image, t::PortField::LOCAL_ROLE) && has(image, t::PortField::PEER_ROLE));
          CHECK(has(image, t::PortField::ORIENTATION) == (attachment != 3));
          const bool validAccessory = attachment == 3
              ? accessoryCodeValue != t::Accessory::UNKNOWN && accessoryCode != 0
              : accessoryCode == 0;
          CHECK(has(image, t::PortField::ACCESSORY) == validAccessory);
          CHECK(image.accessory == (!validAccessory ? t::Accessory::UNKNOWN : attachment == 0
              ? t::Accessory::NOT_APPLICABLE : attachment == 3 ? accessoryCodeValue : t::Accessory::NONE));
          const bool validCable = attachment == 1 && accessoryCode == 0;
          CHECK(has(image, t::PortField::ACTIVE_CABLE) == validCable);
          if (validCable) CHECK(image.activeCable == ((raw08 % 2) != 0));
          bool validCurrent = false;
          auto current = t::DetectedCurrent::UNKNOWN;
          if (attachment == 0 || attachment == 1) {
            validCurrent = true;
            current = t::DetectedCurrent::NOT_APPLICABLE;
          } else if (attachment == 2 && accessoryCode == 0 && currentCode != 2) {
            validCurrent = true;
            current = CURRENTS[currentCode];
          } else if (attachment == 3 && validAccessory) {
            if (accessoryCode == 4 || accessoryCode == 6) {
              validCurrent = true;
              current = t::DetectedCurrent::NOT_APPLICABLE;
            } else if (accessoryCode == 5 && currentCode == 2) {
              validCurrent = true;
              current = t::DetectedCurrent::CHARGE_THROUGH_500_MA;
            }
          }
          CHECK(has(image, t::PortField::DETECTED_CURRENT) == validCurrent);
          CHECK(image.detectedCurrent == current);
        }
      }
    }
  }
}

void changedBookends() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    uint64_t sequence = 0;
    for (unsigned first = 0; first < 8; ++first) {
      for (unsigned last = 0; last < 8; ++last) {
        if (first == last) continue;
        f.port(static_cast<uint8_t>(first * 32), 0xBB, static_cast<uint8_t>(last * 32 + 0x18));
        const auto token = f.start();
        CHECK(f.finish(token).ok() && f.fake.complete());
        const auto image = f.take(token).port;
        CHECK(!image.invalidated && image.sequence == ++sequence && image.rawValidMask == 3);
        CHECK(image.rawStatus08 == 0xBB && image.rawStatus09 == last * 32 + 0x18);
        CHECK(image.coherence == t::SnapshotCoherence::CHANGED_DURING_READ);
        CHECK(!has(image, t::PortField::DETECTED_CURRENT) && !has(image, t::PortField::ACCESSORY));
        CHECK(!has(image, t::PortField::ACTIVE_CABLE));
        CHECK(image.detectedCurrent == t::DetectedCurrent::UNKNOWN && image.accessory == t::Accessory::UNKNOWN);
        CHECK(image.attachment == ATTACHMENTS[last / 2] && has(image, t::PortField::ATTACHMENT));
        CHECK(has(image, t::PortField::LOCAL_ROLE) && has(image, t::PortField::PEER_ROLE));
        const unsigned finalAttachment = last / 2;
        CHECK(image.localRole == (finalAttachment == 1 ? t::PortRole::SOURCE : finalAttachment == 2
            ? t::PortRole::SINK : t::PortRole::NOT_APPLICABLE));
        CHECK(image.peerRole == (finalAttachment == 1 ? t::PortRole::SINK : finalAttachment == 2
            ? t::PortRole::SOURCE : t::PortRole::NOT_APPLICABLE));
        CHECK(has(image, t::PortField::ORIENTATION) == (finalAttachment != 3));
        CHECK(image.orientation == (finalAttachment == 0 ? t::Orientation::NOT_APPLICABLE
            : finalAttachment == 3 ? t::Orientation::UNKNOWN
            : last % 2 == 0 ? t::Orientation::CC1 : t::Orientation::CC2));
        CHECK(has(image, t::PortField::ADVERTISED_CURRENT) && image.advertisedCurrent == t::SourceCurrent::MA_3000);
        CHECK(image.interruptPending && has(image, t::PortField::INTERRUPT_PENDING));
        CHECK(f.fake.calls == 3); // No restart or retry until stable.
      }
    }
  }
}

void everyFinalStatusByte() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    for (unsigned raw09 = 0; raw09 < 256; ++raw09) {
      // Change every control/IRQ/fault/reserved low bit without changing E0.
      f.port(static_cast<uint8_t>(raw09 ^ 0x1F), 0x00, static_cast<uint8_t>(raw09));
      const auto token = f.start();
      CHECK(f.finish(token).ok() && f.fake.complete());
      const auto image = f.take(token).port;
      CHECK(image.rawStatus09 == raw09 && image.coherence == t::SnapshotCoherence::BOOKENDS_MATCH);
      CHECK(image.interruptPending == ((raw09 & 0x10U) != 0));
      CHECK(has(image, t::PortField::INTERRUPT_PENDING));
      CHECK(image.attachment == ATTACHMENTS[raw09 / 64]);
      CHECK(has(image, t::PortField::ADVERTISED_CURRENT));
      // Fault bit3 remains raw, with no typed fault/clear inference.
      CHECK((image.validFields & static_cast<uint16_t>(~uint16_t{0x1FF})) == 0);
    }
  }
}

void everyReadFailureAndInterruption() {
  constexpr t::Err ERRORS[] = {t::Err::I2C_ERROR, t::Err::I2C_NACK_ADDR, t::Err::I2C_NACK_DATA,
      t::Err::I2C_TIMEOUT, t::Err::I2C_BUS, t::Err::I2C_BUSY};
  for (const auto& part : PARTS) {
    for (unsigned boundary = 0; boundary < 3; ++boundary) {
      for (const auto code : ERRORS) {
        Fixture f(part);
        const auto old = f.refresh();
        const auto before = f.device.transportStats();
        f.port(0, 0, 0); // Partial receive bytes must not fabricate a new detach.
        f.fake.script[boundary].status = t::Status::Error(code, "port read failed", -123456789);
        const auto token = f.start(&Driver::startRefreshPort, 200, 1000);
        for (unsigned index = 0; index < boundary; ++index) CHECK(f.poll(token, 200).inProgress());
        CHECK(f.poll(token, 200).is(code));
        CHECK(sameHistory(f.device.portSnapshot(), old) && !f.device.portSnapshot().invalidated);
        CHECK(f.poll(token, 1000, 0).is(code)); // Backend error remains primary after late return.
        const auto result = f.take(token);
        CHECK(result.status.is(code) && result.status.detail == -123456789);
        CHECK(result.transfers == boundary + 1 && result.hardwareEffect == t::HardwareEffect::NONE);
        CHECK(sameHistory(result.port, old) && !result.port.invalidated);
        CHECK(f.fake.calls == boundary + 1 && f.fake.errors == 0);
        const auto totals = f.device.transportStats();
        CHECK(totals.totalAttempts - before.totalAttempts == boundary + 1);
        CHECK(totals.totalFailures - before.totalFailures == 1);
      }
    }
    for (unsigned boundary = 0; boundary <= 3; ++boundary) {
      for (unsigned action = 0; action < 4; ++action) {
        Fixture f(part);
        const auto old = f.refresh();
        f.port(0, 0, 0);
        const auto token = f.start(&Driver::startRefreshPort, 200, 300);
        for (unsigned index = 0; index < boundary; ++index) CHECK(f.poll(token, 200).inProgress());
        CHECK(sameHistory(f.device.portSnapshot(), old));
        if (action == 0) CHECK(f.device.cancel(token).ok());
        if (action == 1) CHECK(f.device.end().ok());
        if (action == 2) CHECK(f.device.invalidateDeviceState(t::Status::Error(t::Err::I2C_BUS, "external", 91)).ok());
        if (action == 3) CHECK(f.poll(token, 300, 255).is(t::Err::TIMEOUT));
        const auto result = f.take(token);
        CHECK(result.status.is(action == 3 ? t::Err::TIMEOUT : t::Err::CANCELLED));
        CHECK(result.transfers == boundary && result.hardwareEffect == t::HardwareEffect::NONE);
        CHECK(sameHistory(f.device.portSnapshot(), old) && sameHistory(result.port, old));
        CHECK(result.port.invalidated == (action == 1 || action == 2));
        CHECK(f.fake.calls == boundary && f.fake.errors == 0);
      }
    }
    for (unsigned boundary = 0; boundary < 3; ++boundary) {
      Fixture f(part);
      const auto historical = f.refresh();
      CHECK(f.device.invalidateDeviceState(t::Status::Ok()).ok());
      f.port(0, 0, 0);
      f.fake.script[boundary].status = t::Status::Error(t::Err::I2C_BUS, "invalidated old image");
      const auto token = f.start(&Driver::startRefreshPort, 200, 1000);
      CHECK(f.finish(token, 200).is(t::Err::I2C_BUS));
      const auto result = f.take(token);
      CHECK(result.port.invalidated && sameHistory(result.port, historical));
      CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
      CHECK(f.fake.calls == boundary + 1 && f.fake.errors == 0);
    }
  }
}

void ownerTimeAndProvenance() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    auto image = f.refresh();
    CHECK(image.configGeneration == 1 && image.configurationState == t::ConfigurationState::KNOWN);
    f.fake = Fake{};
    f.profile();
    auto token = f.start(&Driver::startReconcile, 200, 1000);
    CHECK(f.finish(token, 200).ok() && f.fake.complete());
    CHECK(f.take(token).configuration.verifiedGeneration == 2);
    CHECK(sameHistory(f.device.portSnapshot(), image) && !f.device.portSnapshot().invalidated);
    image = f.refresh(300);
    CHECK(image.sequence == 2 && image.configGeneration == 2 && image.configurationState == t::ConfigurationState::KNOWN);
    CHECK(f.device.invalidateDeviceState(t::Status::Ok()).ok());
    CHECK(sameHistory(f.device.portSnapshot(), image) && f.device.portSnapshot().invalidated);
    f.fake = Fake{};
    f.profile();
    token = f.start(&Driver::startReconcile, 400, 1000);
    CHECK(f.finish(token, 400).ok() && f.fake.complete());
    (void)f.take(token);
    CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), image));
    image = f.refresh(500);
    CHECK(!image.invalidated && image.sequence == 3 && image.configGeneration == 3);
    CHECK(f.device.bind(f.config).ok());
    CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), image));
    CHECK(f.device.end().ok() && f.device.forgetDeviceEvidence().ok());
    CHECK(f.device.portSnapshot().sequence == 0 && f.device.portSnapshot().rawValidMask == 0);
  }
  Fixture f;
  const auto maximum = std::numeric_limits<uint64_t>::max();
  f.port();
  const auto token = f.start(&Driver::startRefreshPort, maximum - 5, 20);
  CHECK(f.poll(token, maximum - 6).is(t::Err::INVALID_PARAM) && f.fake.calls == 0);
  CHECK(f.poll(token, maximum - 4).inProgress());
  CHECK(f.poll(token, maximum - 3).inProgress());
  CHECK(f.poll(token, 0).inProgress());
  CHECK(f.poll(token, 1, 0).ok());
  CHECK(f.fake.complete());
  CHECK(f.take(token).port.observedUptimeMs == maximum - 4);
  Fixture clipped;
  clipped.port();
  clipped.fake.script[0].timeoutMs = 7;
  const auto pending = clipped.start(&Driver::startRefreshPort, 100, 107);
  CHECK(clipped.poll(pending, 100).inProgress());
  CHECK(clipped.poll(pending, 107, 0).is(t::Err::TIMEOUT));
  CHECK(clipped.fake.calls == 1 && clipped.fake.errors == 0);
  CHECK(clipped.take(pending).port.rawValidMask == 0);
}

void earlierOperationsAndWrites() {
  for (const auto& part : PARTS) {
    // Read-only proof and successful no-op apply are not port acquisitions.
    Fixture f(part);
    f.initialize();
    auto historical = f.refresh();
    const auto desired = f.device.configurationSnapshot().verifiedProfile;
    f.fake = Fake{};
    f.profile();
    t::OperationToken token{};
    CHECK(f.device.startApplyProfile(desired, {200, 1000}, token).inProgress());
    CHECK(!f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
    CHECK(f.finish(token, 200).ok() && f.fake.complete());
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::NONE);
    CHECK(!f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
    for (const auto method : {&Driver::startReadProfile, &Driver::startVerifyProfile, &Driver::startReconcile}) {
      f.fake = Fake{};
      f.profile(method != &Driver::startReadProfile);
      token = f.start(method, 300, 1000);
      CHECK(f.finish(token, 300).ok() && f.fake.complete());
      CHECK(sameHistory(f.take(token).port, historical));
      CHECK(!f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
    }
    // Admission/cancellation before a possible write retains otherwise usable history.
    auto changed = f.device.configurationSnapshot().verifiedProfile;
    if (part.is322) changed.ccDebounce = t::CcDebounce::MS_118;
    else changed.sourceCurrent = t::SourceCurrent::MA_1500;
    f.fake = Fake{};
    CHECK(f.device.startApplyProfile(changed, {400, 1000}, token).inProgress());
    CHECK(!f.device.portSnapshot().invalidated && f.fake.calls == 0);
    CHECK(f.device.cancel(token).ok());
    (void)f.take(token);
    CHECK(!f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
    // Every possible configuration write invalidates before entering the callback.
    f.fake = Fake{};
    f.fake.expectWrite(part.address, part.is322 ? 0x0A : 0x08, part.is322 ? 0x70 : 0x40);
    f.profile(true, part.is322 ? 0x80 : 0x40, 0x14, part.is322 ? 0x70 : 0x30);
    f.historical = historical;
    f.checkWriteInvalidation = true;
    CHECK(f.device.startApplyProfile(changed, {500, 1000}, token).inProgress());
    CHECK(!f.device.portSnapshot().invalidated);
    CHECK(f.finish(token, 500).ok() && f.fake.complete());
    CHECK(f.take(token).hardwareEffect == t::HardwareEffect::CONFIRMED);
    CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
    historical = f.refresh(600);
    CHECK(!historical.invalidated && historical.sequence == 2);
    // Re-initialization invalidates history but its profile reads never refresh it.
    f.fake = Fake{};
    f.profile();
    token = f.start(&Driver::startInitialize, 700, 1000);
    CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
    CHECK(f.finish(token, 700).ok() && f.fake.complete());
    (void)f.take(token);
    CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
  }
  for (const auto& part : PARTS) {
    // Reset and recovery proof must preserve an existing port image, not only an empty one.
    for (unsigned procedure = 0; procedure < (part.base ? 2U : 1U); ++procedure) {
      Fixture f(part);
      f.initialize();
      const auto historical = f.refresh();
      f.historical = historical;
      f.checkWriteInvalidation = true;
      f.fake = Fake{};
      if (procedure == 0) {
        f.fake.expectRead(part.address, 0x0A, 1).rx[0] = 0x30;
        f.fake.expectWrite(part.address, 0x0A, 0x38);
        f.profile();
      } else {
        f.fake.expectRead(part.address, 0x45, 1).rx[0] = 0;
        f.fake.expectWrite(part.address, 0x45, 4);
        f.fake.expectWrite(part.address, 0x45, 0);
        f.fake.expectRead(part.address, 0x45, 1).rx[0] = 0;
      }
      const auto token = f.start(procedure == 0 ? &Driver::startReset : &Driver::startRecoverDeadBattery,
                                 200, 1000);
      CHECK(!f.device.portSnapshot().invalidated);
      CHECK(f.finish(token, 200).ok() && f.fake.complete());
      const auto result = f.take(token);
      CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED);
      CHECK(result.port.invalidated && sameHistory(result.port, historical));
      CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
    }
    for (unsigned procedure = 0; procedure < (part.base ? 3U : 2U); ++procedure) {
      for (const bool accepted : {false, true}) {
        Fixture f(part);
        f.initialize();
        const auto historical = f.refresh();
        f.historical = historical;
        f.checkWriteInvalidation = true;
        f.fake = Fake{};
        t::OperationToken token{};
        if (procedure == 0) {
          auto desired = f.device.configurationSnapshot().verifiedProfile;
          if (part.is322) desired.ccDebounce = t::CcDebounce::MS_118;
          else desired.sourceCurrent = t::SourceCurrent::MA_1500;
          f.fake.expectWrite(part.address, part.is322 ? 0x0A : 8, part.is322 ? 0x70 : 0x40);
          CHECK(f.device.startApplyProfile(desired, {200, 1000}, token).inProgress());
        } else if (procedure == 1) {
          f.fake.expectRead(part.address, 0x0A, 1).rx[0] = 0x30;
          f.fake.expectWrite(part.address, 0x0A, 0x38);
          token = f.start(&Driver::startReset, 200, 1000);
        } else {
          f.fake.expectRead(part.address, 0x45, 1).rx[0] = 0;
          f.fake.expectWrite(part.address, 0x45, 4);
          token = f.start(&Driver::startRecoverDeadBattery, 200, 1000);
        }
        auto& write = f.fake.script[f.fake.stepCount - 1];
        write.applyWrite = accepted;
        write.status = t::Status::Error(t::Err::I2C_BUS, "uncertain write", -271828);
        CHECK(f.finish(token, 200).is(t::Err::I2C_BUS) && f.fake.complete());
        const auto result = f.take(token);
        CHECK(result.status.detail == -271828 && result.hardwareEffect == t::HardwareEffect::INDETERMINATE);
        CHECK(result.port.invalidated && sameHistory(result.port, historical));
        CHECK(f.device.portSnapshot().invalidated && sameHistory(f.device.portSnapshot(), historical));
        // A later complete port acquisition can clear only port invalidation;
        // independent configuration uncertainty must survive successful communication.
        const auto refreshed = f.refresh(300);
        CHECK(!refreshed.invalidated && refreshed.sequence == historical.sequence + 1);
        CHECK(refreshed.configurationState == t::ConfigurationState::UNKNOWN);
        CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
      }
    }
  }
}

void negativeConfigurationEvidence() {
  for (const auto& part : PARTS) {
    for (unsigned changedRead = 0; changedRead < 3; ++changedRead) {
      // A contrary final09 has no subsequent read that could fail. It must
      // revoke configuration trust before fresh publication, cancel or expiry.
      for (unsigned termination = changedRead == 2 ? 1U : 0U; termination < 4; ++termination) {
        Fixture f(part);
        f.initialize();
        const auto historical = f.refresh();
        CHECK(historical.configurationState == t::ConfigurationState::KNOWN);
        f.port(changedRead == 0 ? 0x96 : 0x94, changedRead == 1 ? 0x50 : 0x90,
               changedRead == 2 ? 0x96 : 0x94);
        if (termination == 0) {
          f.fake.script[changedRead + 1].status =
              t::Status::Error(t::Err::I2C_TIMEOUT, "after contrary control", -4545);
        }
        const auto token = f.start(&Driver::startRefreshPort, 200, 300);
        for (unsigned read = 0; read <= changedRead; ++read) CHECK(f.poll(token, 200).inProgress());
        // Negative control evidence survives before completion-time publication.
        CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
        CHECK(sameHistory(f.device.portSnapshot(), historical) && !f.device.portSnapshot().invalidated);
        const auto beforeInvalid = f.device.progress();
        const auto callsBeforeInvalid = f.fake.calls;
        CHECK(f.device.poll(token, 199, 0).is(t::Err::INVALID_PARAM));
        CHECK(f.fake.calls == callsBeforeInvalid &&
              f.device.progress().lastPollTransfers == beforeInvalid.lastPollTransfers);
        if (changedRead != 2) CHECK(f.poll(token, 200, 0).inProgress());
        CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
        CHECK(sameHistory(f.device.portSnapshot(), historical) && !f.device.portSnapshot().invalidated);
        if (termination == 0) CHECK(f.finish(token, 200).is(t::Err::I2C_TIMEOUT));
        if (termination == 1) CHECK(f.device.cancel(token).ok());
        if (termination == 2) CHECK(f.poll(token, 300, 0).is(t::Err::TIMEOUT));
        if (termination == 3) CHECK(f.finish(token, 200).ok() && f.fake.complete());
        const auto result = f.take(token);
        CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
        if (termination != 3) {
          CHECK(sameHistory(result.port, historical) && !result.port.invalidated);
          CHECK(sameHistory(f.device.portSnapshot(), historical) && !f.device.portSnapshot().invalidated);
        } else {
          CHECK(!result.port.invalidated && result.port.sequence == historical.sequence + 1);
          CHECK(result.port.configGeneration == historical.configGeneration);
          CHECK(result.port.configurationState == t::ConfigurationState::KNOWN);
        }
      }
    }
  }
}

void saturatingSequenceBoundary() {
  const auto maximum = std::numeric_limits<uint64_t>::max();
  CHECK(t::detail::nextSequence(0) == 1);
  CHECK(t::detail::nextSequence(123) == 124);
  CHECK(t::detail::nextSequence(maximum - 1) == maximum);
  CHECK(t::detail::nextSequence(maximum) == maximum);
}

void ccAndVbusEvidence() {
  // TI B 7.3.1.2 p11 requires CC and VBUS for Attached.SNK. This explicit
  // fixture assumption is not a physical Type-C simulator or qualification.
  // The owner sees register evidence only, never an inferred GPIO/power policy.
  for (const auto& part : PARTS) {
    Fixture f(part);
    for (const bool ccPresent : {false, true}) {
      for (const bool vbusPresent : {false, true}) {
        const bool attached = ccPresent && vbusPresent;
        const uint8_t status09 = attached ? 0x80 : 0x00;
        f.port(status09, 0x31, status09); // High current and raw cable flag alone prove no attach.
        const auto token = f.start();
        CHECK(f.finish(token).ok() && f.fake.complete());
        const auto image = f.take(token).port;
        CHECK(image.attachment == (attached ? t::Attachment::LOCAL_SINK : t::Attachment::DETACHED));
        CHECK(image.detectedCurrent == (attached ? t::DetectedCurrent::MA_3000
                                                : t::DetectedCurrent::NOT_APPLICABLE));
        CHECK(image.activeCable && !has(image, t::PortField::ACTIVE_CABLE));
        CHECK(f.fake.calls == 3); // No reset to update current, ACK or external power write.
      }
    }
  }
}

} // namespace

int runInterruptTests();

int main() {
  failures += runInterruptTests();
  admissionAndPublication();
  independentStatusEncodings();
  everyFinalStatusByte();
  changedBookends();
  everyReadFailureAndInterruption();
  ownerTimeAndProvenance();
  earlierOperationsAndWrites();
  negativeConfigurationEvidence();
  saturatingSequenceBoundary();
  ccAndVbusEvidence();
  if (failures != 0) {
    std::printf("FAIL %d of %zu port checks\n", failures, checks);
    return 1;
  }
  std::printf("PASS %zu port checks; seven variants, fixed bookends and historical freshness\n", checks);
  return 0;
}
