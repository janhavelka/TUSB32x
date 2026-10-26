/// @file test_profile_observation.cpp
/// @brief Public read-only profile and reconciliation proof/fault regressions.
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
static_assert(std::is_same_v<decltype(&Driver::startReadProfile), Start>);
static_assert(std::is_same_v<decltype(&Driver::startReconcile), Start>);

int failures = 0;
std::size_t checks = 0;

void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    std::printf("FAIL profile line %d: %s\n", line, expression);
    ++failures;
  }
}

#define CHECK(expression) check((expression), #expression, __LINE__)

constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
constexpr uint64_t HALF_RANGE = uint64_t{1} << 63;
constexpr uint64_t MAX_TIME = std::numeric_limits<uint64_t>::max();

struct Part {
  t::Variant variant;
  uint8_t address;
  bool base;
  bool is322;
  uint8_t revision;
};

// Independent plan I/P/F from the TI-backed device guide; no codec-derived oracle.
constexpr Part PARTS[] = {
    {t::Variant::TUSB320, 0x60, true, false, 0},
    {t::Variant::TUSB320I, 0x60, true, false, 0},
    {t::Variant::TUSB320LI, 0x47, false, false, 1},
    {t::Variant::TUSB320HI, 0x47, false, false, 1},
    {t::Variant::TUSB320LAI, 0x47, false, false, 2},
    {t::Variant::TUSB320HAI, 0x47, false, false, 2},
    {t::Variant::TUSB322I, 0x47, false, true, 2},
};
constexpr Start STARTS[] = {&Driver::startReadProfile, &Driver::startReconcile};

bool sameProfile(const t::DeviceProfile& a, const t::DeviceProfile& b) {
  return a.portMode == b.portMode && a.sourceCurrent == b.sourceCurrent &&
      a.sourcePreference == b.sourcePreference && a.drpDutyCycle == b.drpDutyCycle &&
      a.ccDebounce == b.ccDebounce && a.disableUfpAccessory == b.disableUfpAccessory &&
      a.disableTerm == b.disableTerm && a.disableRdRp == b.disableRdRp;
}

bool sameObservation(const t::ConfigurationSnapshot& a, const t::ConfigurationSnapshot& b) {
  return a.hasObservedProfile == b.hasObservedProfile &&
      sameProfile(a.observedProfile, b.observedProfile) &&
      a.rawConfig08 == b.rawConfig08 && a.rawConfig09 == b.rawConfig09 &&
      a.rawConfig0A == b.rawConfig0A && a.rawConfig45 == b.rawConfig45 &&
      a.rawConfigValidMask == b.rawConfigValidMask &&
      a.observedValidFields == b.observedValidFields &&
      a.observedTimeValid == b.observedTimeValid && a.observedUptimeMs == b.observedUptimeMs;
}

bool sameStatus(t::Status a, t::Status b) {
  return a.code == b.code && a.detail == b.detail && a.msg == b.msg;
}

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
    config.address = static_cast<t::DeviceAddress>(part.address);
    CHECK(device.bind(config).ok());
  }

  std::size_t reads(Start start) const {
    const std::size_t profile = part.is322 ? 3U : 4U;
    return profile + (start == &Driver::startReadProfile ? 0U : part.base ? 1U : 2U);
  }

  void script(Start start, uint8_t byte08 = 0x80, uint8_t byte09 = 0x14,
              uint8_t byte0A = 0x30, uint8_t byte45 = 0) {
    fake = Fake{};
    if (start != &Driver::startReadProfile) {
      auto& id = fake.expectRead(part.address, 0, 8);
      std::memcpy(id.rx, part.is322 ? ID322 : ID320, 8);
      if (!part.base) fake.expectRead(part.address, 0xA0, 1).rx[0] = part.revision;
    }
    fake.expectRead(part.address, 0x08, 1).rx[0] = byte08;
    fake.expectRead(part.address, 0x09, 1).rx[0] = byte09;
    fake.expectRead(part.address, 0x0A, 1).rx[0] = byte0A;
    if (!part.is322) fake.expectRead(part.address, 0x45, 1).rx[0] = byte45;
  }

  t::OperationToken start(Start method, uint64_t now = 100, uint64_t deadline = 1000) {
    t::OperationToken token{777};
    CHECK((device.*method)({now, deadline}, token).inProgress());
    CHECK(token.valid() && token.value != 777 && fake.calls == 0);
    return token;
  }

  t::Status finish(t::OperationToken token, std::size_t count, uint64_t now = 100,
                   uint8_t budget = 255) {
    for (std::size_t turn = 0; turn <= count + 1; ++turn) {
      const auto previous = fake.calls;
      const auto status = device.poll(token, now, budget);
      CHECK(fake.calls - previous <= 1 && device.progress().lastPollTransfers <= 1);
      if (!status.inProgress()) return status;
    }
    CHECK(false); // No retries or unbounded proof loop.
    return t::Status::Error(t::Err::I2C_ERROR, "test operation did not finish");
  }

  t::OperationResult take(t::OperationToken token) {
    t::OperationResult out{};
    CHECK(device.takeResult(token, out).ok());
    CHECK(out.token.value == token.value && out.hardwareEffect == t::HardwareEffect::NONE);
    return out;
  }

  void initialize() {
    script(&Driver::startInitialize);
    const auto token = start(&Driver::startInitialize);
    CHECK(finish(token, reads(&Driver::startInitialize)).ok());
    CHECK(fake.complete());
    CHECK(take(token).configuration.verifiedGeneration == 1);
  }
};

void admissionAndExactPlans() {
  for (const auto method : STARTS) {
    Driver unbound;
    t::OperationToken rejected{99};
    CHECK((unbound.*method)({100, 1000}, rejected).is(t::Err::NOT_BOUND));
    CHECK((unbound.*method)({100, 100}, rejected).is(t::Err::INVALID_PARAM));
    CHECK((unbound.*method)({0, HALF_RANGE}, rejected).is(t::Err::INVALID_PARAM));
    CHECK(rejected.value == 99);
    for (const auto& part : PARTS) {
      for (const uint8_t budget : {uint8_t{1}, uint8_t{2}, uint8_t{255}}) {
        Fixture f(part);
        f.script(method);
        auto timing = t::OperationTiming{100, 1000};
        t::OperationToken token{};
        CHECK((f.device.*method)(timing, token).inProgress());
        CHECK(f.fake.calls == 0);
        timing.nowMs = 500;
        timing.deadlineMs = 101;
        CHECK(f.device.progress().deadlineMs == 1000);
        CHECK(f.device.progress().kind == (method == &Driver::startReadProfile ?
            t::OperationKind::READ_PROFILE : t::OperationKind::RECONCILE));
        CHECK((f.device.*method)({100, 1000}, rejected).is(t::Err::BUSY));
        CHECK(rejected.value == 99);
        CHECK(f.device.poll(token, 100, 0).inProgress());
        CHECK(f.fake.calls == 0 && f.device.progress().lastPollTransfers == 0);
        for (std::size_t index = 0; index < f.reads(method); ++index) {
          CHECK(f.device.poll(token, 100, budget).inProgress());
          CHECK(f.fake.calls == index + 1 && f.device.progress().lastPollTransfers == 1);
          CHECK(f.device.configurationSnapshot().rawConfigValidMask == 0);
        }
        CHECK(f.fake.complete());
        t::OperationResult unchanged{};
        unchanged.token.value = 555;
        CHECK(f.device.resultSnapshot(token, unchanged).is(t::Err::RESULT_NOT_AVAILABLE));
        CHECK(unchanged.token.value == 555);
        CHECK(f.device.poll(token, 101, 0).ok());
        CHECK(f.fake.calls == f.reads(method));
        CHECK((f.device.*method)({101, 1000}, rejected).is(t::Err::RESULT_PENDING));
        const auto result = f.take(token);
        const auto& image = result.configuration;
        CHECK(result.kind == (method == &Driver::startReadProfile ?
            t::OperationKind::READ_PROFILE : t::OperationKind::RECONCILE));
        CHECK(result.startedUptimeMs == 100);
        CHECK(result.status.ok() && result.completedTimeValid && result.completedUptimeMs == 101);
        CHECK(result.transfers == f.reads(method) && result.port.invalidated);
        CHECK(image.hasObservedProfile && !image.hasDesiredProfile && image.desiredGeneration == 0);
        CHECK(image.rawConfig08 == 0x80 && image.rawConfig09 == 0x14 && image.rawConfig0A == 0x30);
        CHECK(image.rawConfigValidMask == (part.is322 ? 7 : 15));
        CHECK(image.observedValidFields == (part.base ? 0x9B : part.is322 ? 0x7F : 0xFF));
        CHECK(image.observedTimeValid && image.observedUptimeMs == 100);
        CHECK(image.observedProfile.portMode == t::PortMode::DRP);
        CHECK(image.observedProfile.sourceCurrent == t::SourceCurrent::MA_3000);
        if (method == &Driver::startReadProfile) {
          CHECK(!f.device.diagnostics().initialized && !image.hasVerifiedProfile);
          CHECK(image.verifiedGeneration == 0 && image.state != t::ConfigurationState::KNOWN);
          CHECK(!result.identity.identityValid);
        } else {
          CHECK(f.device.diagnostics().initialized && image.hasVerifiedProfile);
          CHECK(image.verifiedGeneration == 1 && image.state == t::ConfigurationState::KNOWN);
          CHECK(image.terminationState == t::TerminationState::ENABLED);
          CHECK(result.identity.identityValid && result.identity.identityMatches);
          CHECK(result.identity.revisionValid == !part.base);
        }
        CHECK(f.device.transportStats().totalAttempts == f.reads(method));
        CHECK(f.device.transportStats().totalFailures == 0);
        CHECK(f.device.portSnapshot().invalidated && f.device.portSnapshot().sequence == 0);
      }
    }
  }
}

void observationAndGenerations() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.initialize();
    const auto original = f.device.configurationSnapshot();
    // Status and W1C bits are outside persistent semantic comparison.
    f.script(&Driver::startReadProfile, 0xBF, 0xF4, 0x30);
    auto token = f.start(&Driver::startReadProfile);
    CHECK(f.finish(token, f.reads(&Driver::startReadProfile)).ok());
    auto image = f.take(token).configuration;
    CHECK(image.state == t::ConfigurationState::KNOWN && image.verifiedGeneration == 1);
    CHECK(image.rawConfig08 == 0xBF && image.rawConfig09 == 0xF4);
    CHECK(sameProfile(image.verifiedProfile, original.verifiedProfile));
    CHECK(!image.hasDesiredProfile && image.desiredGeneration == 0);

    f.script(&Driver::startReadProfile, 0x40);
    token = f.start(&Driver::startReadProfile);
    CHECK(f.finish(token, f.reads(&Driver::startReadProfile)).ok());
    image = f.take(token).configuration;
    CHECK(image.state == t::ConfigurationState::UNKNOWN && image.hasObservedProfile);
    CHECK(image.observedProfile.sourceCurrent == t::SourceCurrent::MA_1500);
    CHECK(image.verifiedGeneration == 1 && image.hasVerifiedProfile);
    CHECK(sameProfile(image.verifiedProfile, original.verifiedProfile));
    CHECK(!image.hasDesiredProfile && image.desiredGeneration == 0);
    CHECK(f.device.portSnapshot().invalidated);

    CHECK(f.device.invalidateDeviceState(t::Status::Ok()).ok());
    CHECK(!f.device.diagnostics().initialized);
    f.script(&Driver::startReadProfile, 0x40);
    token = f.start(&Driver::startReadProfile, 200);
    CHECK(f.finish(token, f.reads(&Driver::startReadProfile), 200).ok());
    CHECK(!f.device.diagnostics().initialized);
    CHECK(f.take(token).configuration.verifiedGeneration == 1);

    f.script(&Driver::startReconcile, 0x40);
    token = f.start(&Driver::startReconcile, 300);
    CHECK(f.finish(token, f.reads(&Driver::startReconcile), 300).ok());
    image = f.take(token).configuration;
    CHECK(image.state == t::ConfigurationState::KNOWN && image.verifiedGeneration == 2);
    CHECK(image.verifiedProfile.sourceCurrent == t::SourceCurrent::MA_1500);
    CHECK(f.device.diagnostics().initialized && !image.hasDesiredProfile);
    CHECK(f.device.portSnapshot().invalidated);

    f.script(&Driver::startReconcile, 0x40);
    token = f.start(&Driver::startReconcile, 400);
    CHECK(f.finish(token, f.reads(&Driver::startReconcile), 400).ok());
    CHECK(f.take(token).configuration.verifiedGeneration == 3);
    CHECK(image.verifiedGeneration == 2); // A copied old result is immutable.
  }
  // TUSB322I DRP bits 00 and 11 are equivalent despite different raw bytes.
  Fixture equivalent(PARTS[6]);
  equivalent.initialize();
  equivalent.script(&Driver::startReadProfile, 0x80, 0x14, 0x00);
  const auto token = equivalent.start(&Driver::startReadProfile);
  CHECK(equivalent.finish(token, 3).ok());
  const auto image = equivalent.take(token).configuration;
  CHECK(image.rawConfig0A == 0 && image.observedProfile.portMode == t::PortMode::DRP);
  CHECK(image.state == t::ConfigurationState::KNOWN && image.verifiedGeneration == 1);
  CHECK(!image.mismatchValid && equivalent.fake.complete());
}

void completeProfilePublication() {
  struct Vector {
    uint8_t byte08;
    uint8_t byte09;
    uint8_t byte0A;
    uint8_t byte45;
    t::SourceCurrent current;
    t::DrpDutyCycle duty;
    t::PortMode mode;
    t::SourcePreference preference;
    t::CcDebounce baseDebounce;
    t::CcDebounce extendedDebounce;
    bool accessory;
    bool term;
    bool rdRp;
  };
  // Literal received bytes and semantic expectations from register-map.md's TI
  // tables. Exercise every valid small field value through the public engine;
  // neither decoding nor encoding helpers supply this publication oracle.
  constexpr Vector VECTORS[] = {
      {0x3F, 0xF8, 0x00, 0xFB, t::SourceCurrent::DEFAULT, t::DrpDutyCycle::PERCENT_30,
       t::PortMode::FOLLOW_PORT_PIN, t::SourcePreference::NORMAL,
       t::CcDebounce::MS_133, t::CcDebounce::MS_168, false, false, false},
      {0x7F, 0xFB, 0x53, 0xFF, t::SourceCurrent::MA_1500, t::DrpDutyCycle::PERCENT_40,
       t::PortMode::UFP, t::SourcePreference::TRY_SNK,
       t::CcDebounce::MS_116, t::CcDebounce::MS_118, true, true, true},
      {0xBF, 0xFC, 0xA6, 0xFF, t::SourceCurrent::MA_3000, t::DrpDutyCycle::PERCENT_50,
       t::PortMode::DFP, t::SourcePreference::TRY_SRC,
       t::CcDebounce::MS_151, t::CcDebounce::MS_134, false, false, true},
      {0x3F, 0xFF, 0xF1, 0xFB, t::SourceCurrent::DEFAULT, t::DrpDutyCycle::PERCENT_60,
       t::PortMode::DRP, t::SourcePreference::NORMAL,
       t::CcDebounce::MS_168, t::CcDebounce::MS_152, true, true, false},
  };
  for (const auto method : STARTS) {
    for (const auto& part : PARTS) {
      for (const auto& vector : VECTORS) {
        Fixture f(part);
        f.script(method, vector.byte08, vector.byte09, vector.byte0A, vector.byte45);
        const auto token = f.start(method);
        CHECK(f.finish(token, f.reads(method)).ok());
        const auto result = f.take(token);
        const auto& image = result.configuration;
        t::DeviceProfile expected{};
        expected.sourceCurrent = vector.current;
        expected.drpDutyCycle = vector.duty;
        expected.portMode = part.is322 && vector.mode == t::PortMode::FOLLOW_PORT_PIN ?
            t::PortMode::DRP : vector.mode;
        expected.sourcePreference = part.base ? t::SourcePreference::NORMAL : vector.preference;
        expected.ccDebounce = part.base ? vector.baseDebounce : vector.extendedDebounce;
        expected.disableUfpAccessory = !part.base && vector.accessory;
        expected.disableTerm = !part.base && vector.term;
        expected.disableRdRp = !part.is322 && vector.rdRp;
        CHECK(sameProfile(image.observedProfile, expected));
        CHECK(image.hasObservedProfile && !image.hasDesiredProfile && image.desiredGeneration == 0);
        CHECK(image.observedValidFields == (part.base ? 0x9B : part.is322 ? 0x7F : 0xFF));
        CHECK(image.rawConfig08 == vector.byte08 && image.rawConfig09 == vector.byte09);
        CHECK(image.rawConfig0A == vector.byte0A);
        CHECK(image.rawConfig45 == (part.is322 ? 0 : vector.byte45));
        CHECK(image.rawConfigValidMask == (part.is322 ? 7 : 15));
        CHECK(image.terminationState == (expected.disableTerm || expected.disableRdRp ?
            t::TerminationState::DISABLED : t::TerminationState::ENABLED));
        CHECK(image.resetState == t::ResetState::NONE);
        if (method == &Driver::startReadProfile) {
          CHECK(image.state == t::ConfigurationState::UNCONFIGURED);
          CHECK(!image.hasVerifiedProfile && image.verifiedGeneration == 0);
          CHECK(!f.device.diagnostics().initialized);
        } else {
          CHECK(image.state == t::ConfigurationState::KNOWN);
          CHECK(image.hasVerifiedProfile && image.verifiedGeneration == 1);
          CHECK(sameProfile(image.verifiedProfile, expected));
          CHECK(f.device.diagnostics().initialized);
        }
        CHECK(result.transfers == f.reads(method) && f.fake.complete());
      }
    }
  }
}

void reservedAndTerminationProof() {
  for (const auto method : STARTS) {
    for (const auto& part : PARTS) {
      for (unsigned controls = 0; controls < 4; ++controls) {
        if ((part.base && (controls & 1U) != 0) || (part.is322 && (controls & 2U) != 0)) continue;
        Fixture f(part);
        f.script(method, 0x80, 0x14, (controls & 1U) != 0 ? 0x31 : 0x30,
                 (controls & 2U) != 0 ? 4 : 0);
        const auto token = f.start(method);
        CHECK(f.finish(token, f.reads(method)).ok());
        const auto image = f.take(token).configuration;
        CHECK(image.hasObservedProfile);
        CHECK(image.observedProfile.disableTerm == ((controls & 1U) != 0));
        CHECK(image.observedProfile.disableRdRp == ((controls & 2U) != 0));
        if (method == &Driver::startReconcile) {
          CHECK(image.state == t::ConfigurationState::KNOWN);
        } else {
          CHECK(image.state != t::ConfigurationState::KNOWN && !image.hasVerifiedProfile);
        }
        CHECK(image.terminationState == (controls == 0 ? t::TerminationState::ENABLED :
                                                                    t::TerminationState::DISABLED));
        CHECK(image.resetState == t::ResetState::NONE && f.fake.complete());
      }
      for (unsigned reserved = 0; reserved < (part.base ? 1U : 2U); ++reserved) {
        Fixture f(part);
        f.initialize();
        const auto old = f.device.configurationSnapshot();
        f.script(method, reserved == 0 ? 0xC0 : 0x80, 0x14, reserved == 1 ? 0x34 : 0x30);
        const auto token = f.start(method);
        CHECK(f.finish(token, f.reads(method)).is(t::Err::CONFIGURATION_UNKNOWN));
        const auto image = f.take(token).configuration;
        CHECK(image.rawConfigValidMask == (part.is322 ? 7 : 15));
        CHECK(image.rawConfig08 == (reserved == 0 ? 0xC0 : 0x80));
        CHECK(image.rawConfig0A == (reserved == 1 ? 0x34 : 0x30));
        const auto invalid = reserved == 0 ? t::ProfileField::SOURCE_CURRENT :
                                            t::ProfileField::SOURCE_PREFERENCE;
        CHECK((image.observedValidFields & static_cast<uint16_t>(invalid)) == 0);
        CHECK(!image.hasObservedProfile && image.state == t::ConfigurationState::UNKNOWN);
        CHECK(image.hasVerifiedProfile && image.verifiedGeneration == old.verifiedGeneration);
        CHECK(sameProfile(image.verifiedProfile, old.verifiedProfile));
        CHECK(!image.hasDesiredProfile && image.desiredGeneration == 0);
        CHECK(f.device.portSnapshot().invalidated && f.fake.complete());
      }
    }
  }
}

void everyReadFailureAndInterruption() {
  constexpr t::Err ERRORS[] = {t::Err::I2C_ERROR, t::Err::I2C_NACK_ADDR, t::Err::I2C_NACK_DATA,
      t::Err::I2C_TIMEOUT, t::Err::I2C_BUS, t::Err::I2C_BUSY};
  for (const auto method : STARTS) {
    for (const auto& part : PARTS) {
      for (std::size_t boundary = 0; boundary < (part.is322 ? 3U : 4U) +
          (method == &Driver::startReadProfile ? 0U : part.base ? 1U : 2U); ++boundary) {
        for (const auto error : ERRORS) {
          Fixture f(part);
          f.initialize();
          const auto old = f.device.configurationSnapshot();
          f.script(method);
          auto& failed = f.fake.script[boundary];
          failed.fillLength = boundary % 2 == 0 ? 0 : failed.rxLength;
          failed.rx[0] = 0xFF; // Error-path receive bytes cannot become adverse evidence.
          failed.status = {error, -129876543, "profile backend failure"};
          const auto token = f.start(method);
          for (std::size_t index = 0; index < boundary; ++index)
            CHECK(f.device.poll(token, 100).inProgress());
          CHECK(sameStatus(f.device.poll(token, 100), failed.status));
          CHECK(f.fake.calls == boundary + 1 && f.fake.errors == 0);
          CHECK(sameObservation(f.device.configurationSnapshot(), old));
          CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::KNOWN);
          CHECK(f.device.diagnostics().initialized);
          CHECK(f.device.configurationSnapshot().verifiedGeneration == old.verifiedGeneration);
          CHECK(sameStatus(f.device.poll(token, 2000), failed.status));
          CHECK(f.device.cancel(token).ok());
          const auto result = f.take(token);
          CHECK(sameStatus(result.status, failed.status));
          CHECK(result.transfers == boundary + 1 && !result.completedTimeValid);
          CHECK(sameObservation(result.configuration, old));
          CHECK(f.device.transportStats().totalFailures == 1);
        }
      }
      Fixture count(part);
      for (std::size_t boundary = 0; boundary <= count.reads(method); ++boundary) {
        // Deadline and cancellation include the final successful callback/fresh-time gap.
        for (unsigned action = 0; action < 3; ++action) {
          Fixture f(part);
          f.initialize();
          const auto old = f.device.configurationSnapshot();
          f.script(method);
          const auto token = f.start(method);
          for (std::size_t index = 0; index < boundary; ++index)
            CHECK(f.device.poll(token, 100, 255).inProgress());
          if (action == 0) CHECK(f.device.cancel(token).ok());
          if (action == 1) CHECK(f.device.poll(token, 1000, 255).is(t::Err::TIMEOUT));
          if (action == 2) CHECK(f.device.end().ok());
          const auto result = f.take(token);
          CHECK(result.status.code == (action == 1 ? t::Err::TIMEOUT : t::Err::CANCELLED));
          CHECK(result.transfers == boundary && f.fake.calls == boundary && f.fake.errors == 0);
          CHECK(result.completedTimeValid == (action == 1));
          CHECK(sameObservation(result.configuration, old));
          CHECK(result.configuration.verifiedGeneration == old.verifiedGeneration);
          CHECK(f.device.transportStats().totalFailures == 0);
        }
      }
    }
  }
}

void immediateAdverseEvidence() {
  for (const auto method : STARTS) {
    for (const auto& part : PARTS) {
      // A successful contradictory byte is adverse evidence even if later cancelled.
      // It does not publish an incomplete new profile or discard historical proof.
      struct Contradiction {
        uint8_t bytes[4];
        std::size_t offset;
        bool reservedPreference;
      };
      constexpr Contradiction CONTRADICTIONS[] = {
          {{0x40, 0x14, 0x30, 0}, 0, false},
          {{0xC0, 0x14, 0x30, 0}, 0, false},
          {{0x80, 0x12, 0x30, 0}, 1, false},
          {{0x80, 0x14, 0x20, 0}, 2, false},
          {{0x80, 0x14, 0x34, 0}, 2, true},
          {{0x80, 0x14, 0x30, 4}, 3, false},
      };
      for (const auto& contrary : CONTRADICTIONS) {
        if ((part.is322 && contrary.offset == 3) ||
            (part.base && contrary.reservedPreference)) continue;
        Fixture changed(part);
        changed.initialize();
        const auto old = changed.device.configurationSnapshot();
        changed.script(method, contrary.bytes[0], contrary.bytes[1], contrary.bytes[2],
                       contrary.bytes[3]);
        const auto token = changed.start(method);
        const auto through08 = method == &Driver::startReadProfile ? 1U : part.base ? 2U : 3U;
        for (std::size_t index = 0; index < through08 + contrary.offset; ++index)
          CHECK(changed.device.poll(token, 100).inProgress());
        CHECK(changed.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
        CHECK(changed.device.configurationSnapshot().terminationState ==
            t::TerminationState::UNKNOWN);
        CHECK(changed.device.diagnostics().initialized);
        CHECK(changed.device.cancel(token).ok());
        const auto result = changed.take(token);
        CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
        CHECK(result.configuration.verifiedGeneration == old.verifiedGeneration);
        CHECK(sameObservation(result.configuration, old));
        CHECK(sameProfile(result.configuration.verifiedProfile, old.verifiedProfile));
      }
      Fixture f(part);
      f.initialize();
      const auto old = f.device.configurationSnapshot();
      f.script(method, 0x80, 0x14, 0x38);
      const auto token = f.start(method);
      const std::size_t through0A = f.reads(method) - (part.is322 ? 0U : 1U);
      for (std::size_t index = 0; index < through0A; ++index)
        CHECK(f.device.poll(token, 100).inProgress());
      CHECK(f.device.diagnostics().resetState == t::ResetState::MAY_BE_PENDING);
      CHECK(f.device.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
      CHECK(f.device.cancel(token).ok());
      const auto result = f.take(token);
      CHECK(result.configuration.resetState == t::ResetState::MAY_BE_PENDING);
      CHECK(sameObservation(result.configuration, old));
      CHECK(result.configuration.verifiedGeneration == old.verifiedGeneration);
      CHECK(result.configuration.state == t::ConfigurationState::UNKNOWN);
      CHECK(result.hardwareEffect == t::HardwareEffect::NONE);
    }
  }
}

void partialTerminationContradictionWithoutTrust() {
  for (const auto& part : PARTS) {
    for (const uint8_t reg : {uint8_t{0x0A}, uint8_t{0x45}}) {
      if ((reg == 0x0A && part.base) || (reg == 0x45 && part.is322)) continue;
      for (unsigned disabled = 0; disabled < 2; ++disabled) {
        for (unsigned reserved = 0; reserved < 2; ++reserved) {
          for (unsigned interruption = 0; interruption < 2; ++interruption) {
            const bool laterRead = reg == 0x0A && !part.is322;
            if (interruption == 1 && !laterRead) continue;
            Fixture f(part);
            const uint8_t byte08 = reserved == 0 ? 0x80 : 0xC0;
            const uint8_t byte0A = reg == 0x0A && disabled != 0 ? 0x31 : 0x30;
            const uint8_t byte45 = reg == 0x45 && disabled != 0 ? 4 : 0;
            f.script(&Driver::startReadProfile, byte08, 0x14, byte0A, byte45);
            auto token = f.start(&Driver::startReadProfile);
            CHECK(f.finish(token, f.reads(&Driver::startReadProfile)).code ==
                (reserved == 0 ? t::Err::OK : t::Err::CONFIGURATION_UNKNOWN));
            const auto old = f.take(token).configuration;
            CHECK(!old.hasVerifiedProfile && !f.device.diagnostics().initialized);
            CHECK(old.terminationState == (disabled == 0 ? t::TerminationState::ENABLED :
                                                               t::TerminationState::DISABLED));

            f.script(&Driver::startReadProfile, byte08, 0x14,
                     reg == 0x0A ? static_cast<uint8_t>(byte0A ^ 1U) : byte0A,
                     reg == 0x45 ? static_cast<uint8_t>(byte45 ^ 4U) : byte45);
            if (interruption == 1) {
              f.fake.script[3].fillLength = 0;
              f.fake.script[3].status = {
                  t::Err::I2C_ERROR, -664, "remaining termination read failed"};
            }
            token = f.start(&Driver::startReadProfile, 200);
            const std::size_t throughControl = reg == 0x0A ? 3U : 4U;
            for (std::size_t index = 0; index < throughControl; ++index)
              CHECK(f.device.poll(token, 200).inProgress());
            CHECK(f.device.configurationSnapshot().terminationState ==
                t::TerminationState::UNKNOWN);
            CHECK(f.device.diagnostics().terminationState == t::TerminationState::UNKNOWN);
            CHECK(sameObservation(f.device.configurationSnapshot(), old));
            CHECK(!f.device.diagnostics().initialized);
            if (interruption == 0) CHECK(f.device.cancel(token).ok());
            else CHECK(f.device.poll(token, 200).is(t::Err::I2C_ERROR));
            const auto result = f.take(token);
            CHECK(result.configuration.terminationState == t::TerminationState::UNKNOWN);
            CHECK(sameObservation(result.configuration, old));
            CHECK(result.configuration.verifiedGeneration == 0);
            CHECK(!result.configuration.hasDesiredProfile &&
                result.configuration.desiredGeneration == 0);
            CHECK(f.fake.errors == 0);
          }
        }
      }
    }
  }
}

void reconcileResetWait() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    // F first discovering an asserted reset is one finite proof, without a wait/retry.
    f.script(&Driver::startReconcile, 0x80, 0x14, 0x38);
    auto token = f.start(&Driver::startReconcile);
    CHECK(f.finish(token, f.reads(&Driver::startReconcile)).is(t::Err::CONFIGURATION_UNKNOWN));
    const auto interrupted = f.take(token);
    CHECK(interrupted.configuration.resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(interrupted.configuration.verifiedGeneration == 0 && f.fake.complete());
    CHECK(f.device.diagnostics().initialized);

    f.script(&Driver::startReconcile);
    token = f.start(&Driver::startReconcile, 150);
    CHECK(f.device.poll(token, 200, 255).inProgress());
    CHECK(f.fake.calls == 0);
    const auto waiting = f.device.progress();
    CHECK(waiting.waiting && waiting.readyAtValid && waiting.readyAtMs == 296);
    CHECK(waiting.deadlineMs == 1000);
    CHECK(f.device.poll(token, 295, 255).inProgress());
    CHECK(f.fake.calls == 0);
    CHECK(f.device.configurationSnapshot().resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(f.finish(token, f.reads(&Driver::startReconcile), 296).ok());
    const auto result = f.take(token);
    CHECK(result.configuration.resetState == t::ResetState::COMPLETED);
    CHECK(result.configuration.state == t::ConfigurationState::KNOWN);
    CHECK(result.configuration.terminationState == t::TerminationState::ENABLED);
    CHECK(result.configuration.verifiedGeneration == 1 && f.fake.complete());
    CHECK(interrupted.configuration.resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(interrupted.status.is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(result.hardwareEffect == t::HardwareEffect::NONE);

    // Wait expiry/cancel is bus-silent and cannot clear the retained obligation.
    for (unsigned action = 0; action < 2; ++action) {
      Fixture shortWait(part);
      shortWait.script(&Driver::startReadProfile, 0x80, 0x14, 0x38);
      auto pending = shortWait.start(&Driver::startReadProfile);
      CHECK(shortWait.finish(pending, shortWait.reads(&Driver::startReadProfile))
          .ok());
      CHECK(shortWait.take(pending).configuration.resetState == t::ResetState::MAY_BE_PENDING);
      shortWait.script(&Driver::startReconcile);
      pending = shortWait.start(&Driver::startReconcile, 200, 296);
      CHECK(shortWait.device.poll(pending, 200, 255).inProgress());
      CHECK(shortWait.fake.calls == 0 && shortWait.device.progress().readyAtMs == 296);
      if (action == 0) CHECK(shortWait.device.poll(pending, 296, 255).is(t::Err::TIMEOUT));
      else CHECK(shortWait.device.cancel(pending).ok());
      const auto stopped = shortWait.take(pending);
      CHECK(stopped.configuration.resetState == t::ResetState::MAY_BE_PENDING);
      CHECK(stopped.configuration.state == t::ConfigurationState::UNKNOWN);
      CHECK(stopped.transfers == 0 && shortWait.fake.calls == 0);
    }

    Fixture wrapped(part);
    wrapped.script(&Driver::startReadProfile, 0x80, 0x14, 0x38);
    auto pending = wrapped.start(&Driver::startReadProfile);
    CHECK(wrapped.finish(pending, wrapped.reads(&Driver::startReadProfile)).ok());
    CHECK(wrapped.take(pending).configuration.resetState == t::ResetState::MAY_BE_PENDING);
    wrapped.script(&Driver::startReconcile);
    pending = wrapped.start(&Driver::startReconcile, MAX_TIME - 50, 200);
    CHECK(wrapped.device.poll(pending, MAX_TIME - 20, 0).inProgress());
    CHECK(wrapped.device.progress().readyAtMs == 75);
    CHECK(wrapped.device.poll(pending, 74, 255).inProgress());
    CHECK(wrapped.fake.calls == 0);
    CHECK(wrapped.finish(pending, wrapped.reads(&Driver::startReconcile), 75).ok());
    CHECK(wrapped.take(pending).configuration.resetState == t::ResetState::COMPLETED);
    CHECK(wrapped.fake.complete());

    Fixture stillReset(part);
    stillReset.script(&Driver::startReadProfile, 0x80, 0x14, 0x38);
    pending = stillReset.start(&Driver::startReadProfile);
    CHECK(stillReset.finish(pending, stillReset.reads(&Driver::startReadProfile)).ok());
    CHECK(stillReset.take(pending).configuration.resetState == t::ResetState::MAY_BE_PENDING);
    stillReset.script(&Driver::startReconcile, 0x80, 0x14, 0x38);
    pending = stillReset.start(&Driver::startReconcile, 200);
    CHECK(stillReset.device.poll(pending, 200, 255).inProgress());
    CHECK(stillReset.finish(pending, stillReset.reads(&Driver::startReconcile), 296)
        .is(t::Err::CONFIGURATION_UNKNOWN));
    CHECK(stillReset.take(pending).configuration.resetState == t::ResetState::MAY_BE_PENDING);
    CHECK(stillReset.fake.complete());
  }
}

void incompletePostWaitProof() {
  for (const auto& part : PARTS) {
    Fixture count(part);
    for (std::size_t boundary = 0; boundary <= count.reads(&Driver::startReconcile); ++boundary) {
      for (unsigned action = 0; action < 3; ++action) {
        if (action == 2 && boundary == count.reads(&Driver::startReconcile)) continue;
        Fixture f(part);
        f.script(&Driver::startReadProfile, 0x80, 0x14, 0x38);
        auto token = f.start(&Driver::startReadProfile);
        CHECK(f.finish(token, f.reads(&Driver::startReadProfile)).ok());
        CHECK(f.take(token).configuration.resetState == t::ResetState::MAY_BE_PENDING);
        const auto old = f.device.configurationSnapshot();
        f.script(&Driver::startReconcile);
        if (action == 2) {
          f.fake.script[boundary].fillLength = 0;
          f.fake.script[boundary].status = {t::Err::I2C_BUS, -782, "post-wait proof failure"};
        }
        token = f.start(&Driver::startReconcile, 200);
        CHECK(f.device.poll(token, 200, 0).inProgress());
        CHECK(f.fake.calls == 0 && f.device.progress().readyAtMs == 296);
        for (std::size_t index = 0; index < boundary; ++index)
          CHECK(f.device.poll(token, 296, 255).inProgress());
        if (action == 0) CHECK(f.device.cancel(token).ok());
        if (action == 1) CHECK(f.device.poll(token, 1000, 255).is(t::Err::TIMEOUT));
        if (action == 2) CHECK(f.device.poll(token, 296).is(t::Err::I2C_BUS));
        const auto stopped = f.take(token);
        CHECK(stopped.configuration.resetState == t::ResetState::MAY_BE_PENDING);
        CHECK(stopped.configuration.state == t::ConfigurationState::UNKNOWN);
        CHECK(stopped.configuration.verifiedGeneration == 0);
        CHECK(sameObservation(stopped.configuration, old));
        CHECK(f.fake.calls == boundary + (action == 2 ? 1U : 0U));
        CHECK(f.fake.errors == 0);
        if (action == 2) CHECK(stopped.status.detail == -782);
      }
    }
  }
}

void resetCompletionWithReservedProfile() {
  for (const auto& part : PARTS) {
    for (unsigned reserved = 0; reserved < (part.base ? 1U : 2U); ++reserved) {
      Fixture f(part);
      f.script(&Driver::startReadProfile, 0x80, 0x14, 0x38);
      auto token = f.start(&Driver::startReadProfile);
      CHECK(f.finish(token, f.reads(&Driver::startReadProfile)).ok());
      CHECK(f.take(token).configuration.resetState == t::ResetState::MAY_BE_PENDING);
      const auto before = f.device.configurationSnapshot();

      // Clear reset plus the elapsed interval and complete F establish reset
      // completion independently of an unrelated reserved persistent field.
      f.script(&Driver::startReconcile, reserved == 0 ? 0xC0 : 0x80, 0x14,
               reserved == 1 ? 0x34 : 0x30);
      token = f.start(&Driver::startReconcile, 200);
      CHECK(f.device.poll(token, 200, 255).inProgress());
      CHECK(f.device.progress().waiting && f.device.progress().readyAtMs == 296);
      CHECK(f.fake.calls == 0);
      CHECK(f.finish(token, f.reads(&Driver::startReconcile), 296)
          .is(t::Err::CONFIGURATION_UNKNOWN));
      const auto result = f.take(token);
      const auto& image = result.configuration;
      CHECK(image.resetState == t::ResetState::COMPLETED);
      CHECK(image.terminationState == t::TerminationState::ENABLED);
      CHECK(image.state == t::ConfigurationState::UNKNOWN && !image.hasObservedProfile);
      CHECK(image.rawConfigValidMask == (part.is322 ? 7 : 15));
      const auto invalid = reserved == 0 ? t::ProfileField::SOURCE_CURRENT :
                                          t::ProfileField::SOURCE_PREFERENCE;
      CHECK((image.observedValidFields & static_cast<uint16_t>(invalid)) == 0);
      CHECK(!image.hasVerifiedProfile && image.verifiedGeneration == before.verifiedGeneration);
      CHECK(!image.hasDesiredProfile && image.desiredGeneration == 0);
      CHECK(f.device.diagnostics().initialized);
      CHECK(f.device.diagnostics().resetState == t::ResetState::COMPLETED);
      CHECK(f.device.diagnostics().terminationState == t::TerminationState::ENABLED);
      CHECK(result.transfers == f.reads(&Driver::startReconcile) && f.fake.complete());
      CHECK(result.port.invalidated);
    }
  }
}

void failedInitializationAndIdentity() {
  for (const auto& part : PARTS) {
    Fixture f(part);
    f.script(&Driver::startInitialize);
    f.fake.script[0].status = {t::Err::I2C_NACK_ADDR, -44, "initial identity failed"};
    auto token = f.start(&Driver::startInitialize);
    CHECK(f.device.poll(token, 100).is(t::Err::I2C_NACK_ADDR));
    CHECK(f.take(token).status.detail == -44);
    f.script(&Driver::startReconcile);
    token = f.start(&Driver::startReconcile, 200);
    CHECK(f.finish(token, f.reads(&Driver::startReconcile), 200).ok());
    CHECK(f.take(token).configuration.state == t::ConfigurationState::KNOWN);
    CHECK(f.device.diagnostics().initialized && f.fake.complete());

    // Receiving a matching signature must not erase previously proved revision
    // while this reconciliation's next callback fails before a new revision exists.
    if (!part.base) {
      const auto oldIdentity = f.device.diagnostics().identity;
      const auto oldConfiguration = f.device.configurationSnapshot();
      f.script(&Driver::startReconcile);
      f.fake.script[1].fillLength = 0;
      f.fake.script[1].status = {t::Err::I2C_BUS, -5007, "revision read failed"};
      token = f.start(&Driver::startReconcile, 250);
      CHECK(f.device.poll(token, 250).inProgress());
      CHECK(f.device.poll(token, 250).is(t::Err::I2C_BUS));
      const auto partial = f.take(token);
      CHECK(partial.identity.revisionValid && partial.identity.revisionMatches);
      CHECK(partial.identity.rawRevision == oldIdentity.rawRevision);
      CHECK(f.device.diagnostics().initialized);
      CHECK(partial.configuration.state == t::ConfigurationState::KNOWN);
      CHECK(sameObservation(partial.configuration, oldConfiguration));
      CHECK(partial.status.detail == -5007);
    }

    for (unsigned wrong = 0; wrong < (part.base ? 1U : 2U); ++wrong) {
      f.script(&Driver::startReconcile);
      f.fake.script[wrong].rx[0] ^= 0xFF;
      token = f.start(&Driver::startReconcile, 300);
      for (unsigned index = 0; index <= wrong; ++index)
        CHECK(f.device.poll(token, 300).inProgress());
      CHECK(!f.device.diagnostics().initialized);
      CHECK(f.device.poll(token, 301, 0).is(t::Err::CHIP_ID_MISMATCH));
      CHECK(f.take(token).status.is(t::Err::CHIP_ID_MISMATCH));
      CHECK(f.fake.calls == wrong + 1 && f.fake.errors == 0);
    }
  }
}

void ownerTimeBoundaries() {
  for (const auto method : STARTS) {
    Fixture f;
    f.script(method);
    const auto token = f.start(method, MAX_TIME - 2, 50);
    CHECK(f.device.poll(token, MAX_TIME - 3, 0).is(t::Err::INVALID_PARAM));
    CHECK(f.device.poll(token, MAX_TIME - 2 + HALF_RANGE, 0).is(t::Err::INVALID_PARAM));
    CHECK(f.fake.calls == 0);
    CHECK(f.device.poll(token, MAX_TIME - 2, 0).inProgress());
    CHECK(f.finish(token, f.reads(method), 1).ok());
    const auto result = f.take(token);
    CHECK(result.completedTimeValid && result.completedUptimeMs == 1);
    CHECK(result.configuration.observedTimeValid && result.configuration.observedUptimeMs == 1);

    Fixture clipped;
    clipped.script(method);
    for (std::size_t index = 0; index < clipped.fake.stepCount; ++index)
      clipped.fake.script[index].timeoutMs = 1;
    const auto shortToken = clipped.start(method, 100, 101);
    CHECK(clipped.finish(shortToken, clipped.reads(method), 100).ok());
    CHECK(clipped.take(shortToken).status.ok() && clipped.fake.complete());
  }
  for (const auto method : STARTS) {
    Fixture dated;
    dated.script(method);
    const auto token = dated.start(method);
    const std::size_t identityReads = method == &Driver::startReadProfile ? 0U : 2U;
    for (std::size_t index = 0; index < identityReads; ++index)
      CHECK(dated.device.poll(token, 101).inProgress());
    // No transfer budget consumes pending identity locally without timestamping P.
    CHECK(dated.device.poll(token, 110, 0).inProgress());
    CHECK(dated.fake.calls == identityReads);
    for (std::size_t index = identityReads; index < dated.reads(method); ++index)
      CHECK(dated.device.poll(token, 120).inProgress());
    CHECK(dated.device.poll(token, 130, 0).ok());
    t::OperationResult before{};
    CHECK(dated.device.resultSnapshot(token, before).ok());
    CHECK(before.completedUptimeMs == 130 && before.configuration.observedUptimeMs == 120);
    CHECK(dated.device.invalidateDeviceState(t::Status::Ok()).ok());
    const auto after = dated.take(token);
    CHECK(after.completedUptimeMs == before.completedUptimeMs &&
        sameStatus(after.status, before.status));
    CHECK(after.configuration.state == before.configuration.state);
    CHECK(sameObservation(after.configuration, before.configuration));
    CHECK(dated.device.configurationSnapshot().state != t::ConfigurationState::KNOWN);
    CHECK(dated.fake.complete());
  }
}

} // namespace

/// @brief Execute linked public observation tests and return failed check count.
int runProfileObservationChecks() {
  admissionAndExactPlans();
  observationAndGenerations();
  completeProfilePublication();
  reservedAndTerminationProof();
  everyReadFailureAndInterruption();
  immediateAdverseEvidence();
  partialTerminationContradictionWithoutTrust();
  reconcileResetWait();
  incompletePostWaitProof();
  resetCompletionWithReservedProfile();
  failedInitializationAndIdentity();
  ownerTimeBoundaries();
  std::printf("%s %zu profile observation checks\n", failures == 0 ? "PASS" : "FAIL", checks);
  return failures;
}
