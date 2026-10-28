/// @file main.cpp
/// @brief Independent native component and adapter link fixture; performs no I2C.
/// @details Default contexts deliberately lack handles and must reject locally.
/// No bus, device, task, asynchronous event or CLI frontend is created here.
/// This image proves linking against the selected SDK, not physical behavior.
#include <cstdint>

#include <TUSB32x/TUSB32x.h>

#include "Tusb32xIdf.h"

namespace {

/// @brief Keep each locally obtained result observable in the linked image.
volatile int32_t statusCodes[5]{};
/// @brief Link the native clock without creating or accessing a bus.
volatile uint64_t clockMs = 0;

} // namespace

/// @brief Link lifecycle APIs while rejecting the intentionally invalid default context.
extern "C" void app_main(void) {
  tusb32x_idf::Context context;
  const TUSB32x::Status contextStatus = tusb32x_idf::validateContext(context);
  statusCodes[0] = static_cast<int32_t>(contextStatus.code);

  const uint8_t writeBytes[]{0x0AU, 0x00U};
  uint8_t readByte = 0;
  statusCodes[1] = static_cast<int32_t>(
      tusb32x_idf::write(0x47U, writeBytes, sizeof(writeBytes), 20U, &context).code);
  statusCodes[2] = static_cast<int32_t>(
      tusb32x_idf::writeRead(0x47U, writeBytes, 1U, &readByte, 1U, 20U, &context).code);

  TUSB32x::DriverConfig config;
  config.i2cWrite = tusb32x_idf::write;
  config.i2cWriteRead = tusb32x_idf::writeRead;
  config.i2cUser = &context;
  config.variant = TUSB32x::Variant::TUSB320LAI;
  TUSB32x::TUSB32x device;
  // The default context intentionally rejects; application setup must pass first.
  statusCodes[3] = static_cast<int32_t>(contextStatus.code);
  if (contextStatus.ok()) {
    statusCodes[3] = static_cast<int32_t>(device.bind(config).code);
  }
  clockMs = tusb32x_idf::ownerNowMs();
  statusCodes[4] = static_cast<int32_t>(device.end().code);
}
