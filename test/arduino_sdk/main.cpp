/// @file main.cpp
/// @brief SDK link fixture for the core and Arduino adapter; performs no I2C.
/// @details This fixture never creates or starts a bus. Its default context and
/// unspecified variant deliberately exercise local rejection only. It is not a
/// board example and establishes no electrical or hardware qualification.
#include <TUSB32x/TUSB32x.h>

#include "Tusb32xWire.h"

namespace {

/// @brief Keep local validation outcomes observable in the linked firmware.
volatile int32_t contextCode = 0;
/// @brief Preserve the locally rejected binding outcome without logging or I2C.
volatile int32_t bindingCode = 0;
/// @brief Link the native monotonic clock; reading it does not access the bus.
volatile uint64_t clockMs = 0;

} // namespace

/// @brief Link every adapter entry point while executing only bus-silent checks.
void setup() {
  tusb32x_arduino::Context context;
  contextCode = static_cast<int32_t>(tusb32x_arduino::validateContext(context).code);
  TUSB32x::DriverConfig config;
  config.i2cWrite = tusb32x_arduino::write;
  config.i2cWriteRead = tusb32x_arduino::writeRead;
  config.i2cUser = &context;
  TUSB32x::TUSB32x device;
  bindingCode = static_cast<int32_t>(device.bind(config).code);
  clockMs = tusb32x_arduino::ownerNowMs();
}

/// @brief The build fixture has no owner loop or hardware activity.
void loop() {}
