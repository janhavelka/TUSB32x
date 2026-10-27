/// @file test_protocol_conformance.cpp
/// @brief Public operation address propagation across the evidenced family maps.
#include <TUSB32x/TUSB32x.h>

#include "support/FakeTusb32xDevice.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
namespace t = TUSB32x;
using Driver = t::TUSB32x;
using Fake = tusb32x_test::FakeTusb32xDevice;

int failures = 0;
std::size_t checks = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    std::printf("FAIL conformance line %d: %s\n", line, expression);
    ++failures;
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

struct Part {
  t::Variant variant;
  uint8_t low;
  uint8_t high;
  bool base;
  bool is322;
  uint8_t revision;
};

// B SLLSEN9F Tables7-5/7-7 pp15-19; L SLLSEP2D/A SLLSEQ8D
// Tables5/7-12 pp15-20; V SLLSEO5D Tables6-5/6-7..6-11 pp15-20.
// Seven-bit addresses and ID bytes are independent literals from those tables.
constexpr Part PARTS[] = {
    {t::Variant::TUSB320, 0x60, 0x61, true, false, 0},
    {t::Variant::TUSB320I, 0x60, 0x61, true, false, 0},
    {t::Variant::TUSB320LI, 0x47, 0x67, false, false, 1},
    {t::Variant::TUSB320HI, 0x47, 0x67, false, false, 1},
    {t::Variant::TUSB320LAI, 0x47, 0x67, false, false, 2},
    {t::Variant::TUSB320HAI, 0x47, 0x67, false, false, 2},
    {t::Variant::TUSB322I, 0x47, 0x67, false, true, 2},
};
constexpr uint8_t ID320[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
constexpr uint8_t ID322[] = {0x32, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};

t::OperationResult finish(Driver& device, Fake& fake, t::OperationToken token,
                           uint64_t now) {
  for (unsigned turn = 0; turn < 8; ++turn) {
    const auto before = fake.calls;
    const auto status = device.poll(token, now, 255);
    CHECK(fake.calls - before <= 1);
    if (status.inProgress()) continue;
    CHECK(status.ok());
    t::OperationResult result{};
    CHECK(device.takeResult(token, result).ok());
    CHECK(result.status.ok() && result.token.value == token.value);
    CHECK(result.transfers == fake.calls && fake.complete());
    return result;
  }
  CHECK(false);
  return {};
}

void publicOperationsUseBothLegalAddresses() {
  for (const auto& part : PARTS) {
    const uint8_t addresses[] = {part.low, part.high};
    for (const auto address : addresses) {
      Fake fake;
      Driver device;
      t::DriverConfig config{};
      config.variant = part.variant;
      config.address = static_cast<t::DeviceAddress>(address);
      config.i2cWrite = Fake::write;
      config.i2cWriteRead = Fake::writeRead;
      config.i2cUser = &fake;
      CHECK(device.bind(config).ok() && fake.calls == 0);

      auto& id = fake.expectRead(address, 0, 8);
      std::memcpy(id.rx, part.is322 ? ID322 : ID320, 8);
      if (!part.base) fake.expectRead(address, 0xA0, 1).rx[0] = part.revision;
      fake.expectRead(address, 8, 1).rx[0] = 0x80;
      fake.expectRead(address, 9, 1).rx[0] = part.base ? 0x14 : 0x15;
      fake.expectRead(address, 0x0A, 1).rx[0] = 0x30;
      if (!part.is322) fake.expectRead(address, 0x45, 1).rx[0] = 0;
      t::OperationToken token{};
      CHECK(device.startInitialize({100, 1000}, token).inProgress() && fake.calls == 0);
      auto result = finish(device, fake, token, 100);
      CHECK(result.identity.variant == part.variant);
      CHECK(result.identity.address == config.address);
      CHECK(result.identity.identityValid && result.identity.identityMatches);
      CHECK(result.configuration.state == t::ConfigurationState::KNOWN);
      CHECK(result.hardwareEffect == t::HardwareEffect::NONE);

      fake = Fake{};
      const uint8_t status09 = part.base ? 0xB4 : 0xB5;
      fake.expectRead(address, 9, 1).rx[0] = status09;
      fake.expectRead(address, 8, 1).rx[0] = 0x90;
      fake.expectRead(address, 9, 1).rx[0] = status09;
      CHECK(device.startRefreshPort({200, 1000}, token).inProgress() && fake.calls == 0);
      result = finish(device, fake, token, 200);
      CHECK(result.port.rawStatus08 == 0x90 && result.port.rawStatus09 == status09);
      CHECK(result.port.observedTimeValid && result.port.observedUptimeMs == 200);
      CHECK(result.port.interruptPending && !result.port.invalidated);
      CHECK(result.hardwareEffect == t::HardwareEffect::NONE);

      fake = Fake{};
      if (part.is322) {
        token.value = 777;
        CHECK(device.startAcknowledgeInterrupt({300, 1000}, token)
                  .is(t::Err::UNSUPPORTED_FEATURE));
        CHECK(token.value == 777 && fake.calls == 0);
        CHECK(device.startServiceInterrupt({300, 1000}, token)
                  .is(t::Err::UNSUPPORTED_FEATURE));
        CHECK(token.value == 777 && fake.calls == 0);
      } else {
        fake.expectRead(address, 9, 1).rx[0] = status09;
        fake.expectWrite(address, 9, part.base ? 0x14 : 0x15);
        CHECK(device.startAcknowledgeInterrupt({300, 1000}, token).inProgress());
        CHECK(fake.calls == 0);
        result = finish(device, fake, token, 300);
        CHECK(result.interrupt.pendingBefore && result.interrupt.acknowledgeConfirmed);
        CHECK(!result.interrupt.pendingAfterValid);
        CHECK(result.hardwareEffect == t::HardwareEffect::CONFIRMED);
      }

      // The public raw path also forwards the selected seven-bit address and
      // returns TUSB322I's independent raw fault and IRQ bits without an explicit
      // clear write. Fault read effects remain unresolved.
      fake = Fake{};
      const uint8_t raw09 = part.is322 ? 0xBD : status09;
      fake.expectRead(address, 9, 1).rx[0] = raw09;
      uint8_t output = 0xD3;
      CHECK(device.readRegisterSync(9, output).ok());
      CHECK(output == raw09 && fake.calls == 1 && fake.complete());
      CHECK(device.end().ok() && fake.calls == 1);
    }
  }
}

#undef CHECK
} // namespace

int runProtocolConformanceChecks() {
  publicOperationsUseBothLegalAddresses();
  std::printf("%s %zu public protocol conformance checks\n",
              failures == 0 ? "PASS" : "FAIL", checks);
  return failures;
}
