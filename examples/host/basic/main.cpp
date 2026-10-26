/// @file main.cpp
/// @brief Link validators, byte decoding and passive health with fake callbacks; no device I/O.
#include <TUSB32x/CommandTable.h>
#include <TUSB32x/Config.h>
#include <TUSB32x/Status.h>
#include <TUSB32x/TUSB32x.h>
#include <TUSB32x/Version.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "profile.h"

namespace {

std::size_t callbackAttempts = 0;

// These deliberately failing callbacks identify accidental I/O; no real bus is present.
TUSB32x::Status fakeWrite(uint8_t, const uint8_t*, std::size_t, uint32_t, void*) {
  ++callbackAttempts;
  return TUSB32x::Status::Error(TUSB32x::Err::I2C_ERROR, "fake transport has no device", -70000);
}

TUSB32x::Status fakeWriteRead(uint8_t, const uint8_t*, std::size_t, uint8_t*,
                            std::size_t, uint32_t, void*) {
  ++callbackAttempts;
  return TUSB32x::Status::Error(TUSB32x::Err::I2C_ERROR, "fake transport has no device", -70000);
}

}  // namespace

/// @brief Validate values, decode a supplied byte and project uninitialized health without a device.
/// @return Zero when pure helpers accept and no fake callback was attempted; one on failure.
int main() {
  std::printf("Package version: %s\n", TUSB32x::VERSION);
  TUSB32x::DriverConfig config{};
  config.i2cWrite = fakeWrite;
  config.i2cWriteRead = fakeWriteRead;
  config.variant = TUSB32x::Variant::TUSB320LAI;
  config.address = TUSB32x::DeviceAddress::ADDR_LOW_0X47;

  const TUSB32x::Status bindingStatus = TUSB32x::validateDriverConfig(config);
  const TUSB32x::Status profileStatus =
      TUSB32x::validateProfile(config.variant, makeExampleProfile());
  TUSB32x::RegisterDecode decoded{};
  const TUSB32x::Status decodeStatus = TUSB32x::decodeRegister(config.variant, 0x08, 0x90, decoded);
  TUSB32x::TUSB32x driver;
  TUSB32x::HealthSnapshot health{};
  const auto healthStatus = TUSB32x::projectHealth(
      driver.diagnostics().managedTransport, TUSB32x::HealthPolicy{}, health);
  std::printf("Binding validation: %s; profile validation: %s; fake callback attempts: %zu\n",
              TUSB32x::errorName(bindingStatus.code), TUSB32x::errorName(profileStatus.code),
              callbackAttempts);
  std::printf("Supplied 08=90 decode: %s; raw=%02X (no device observation)\n",
              TUSB32x::errorName(decodeStatus.code), static_cast<unsigned>(decoded.value));
  std::printf("Passive health: %s (unbound, no device observation)\n",
              health.state == TUSB32x::DriverState::UNINIT ? "UNINIT" : "unexpected");
  return bindingStatus.ok() && profileStatus.ok() && decodeStatus.ok() && healthStatus.ok() &&
      health.state == TUSB32x::DriverState::UNINIT &&
      decoded.profileCodes.sourceCurrent == TUSB32x::SourceCurrent::MA_3000 &&
      decoded.detectedCurrent == TUSB32x::DetectedCurrent::MA_1500 && callbackAttempts == 0 ? 0 : 1;
}
