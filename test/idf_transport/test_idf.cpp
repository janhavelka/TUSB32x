/// @file test_idf.cpp
/// @brief Behavioral contracts of the actual native adapter against recording SDK calls.
#include "Tusb32xIdf.h"
#include <TUSB32x/TUSB32x.h>
#include <freertos/FreeRTOS.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <limits>

/// @brief Concrete identities exist only in the host fixture; the adapter sees opaque handles.
struct i2c_master_bus_t { unsigned identity; };
/// @brief Two native targets deliberately share one serialized physical bus context.
struct i2c_master_dev_t { unsigned identity; };

namespace {
using TUSB32x::Err;
namespace adapter = tusb32x_idf;
using adapter::BusContext;
using adapter::Context;
unsigned checks = 0;
unsigned failures = 0;
#define CHECK(condition) do { ++checks; if (!(condition)) { \
  ++failures; std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
} } while (false)

i2c_master_bus_t nativeBus{1};
i2c_master_dev_t nativeDevice{2};
i2c_master_dev_t secondDevice{3};

/// @brief Script finite synchronous calls and independent clock movement, without physical I2C.
struct RecordingSdk {
  uint64_t physicalUs = 123456;
  int core = 0;
  bool isr = false;
  BusContext* bus = nullptr;
  unsigned transmits = 0;
  unsigned receives = 0;
  i2c_master_dev_handle_t txDevice = nullptr;
  i2c_master_dev_handle_t rxDevice = nullptr;
  std::array<uint8_t, 8> tx{};
  size_t txLength = 0;
  size_t rxLength = 0;
  int txTimeoutMs = 0;
  int rxTimeoutMs = 0;
  esp_err_t txError = ESP_OK;
  esp_err_t rxError = ESP_OK;
  std::array<uint8_t, 8> rx{{0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0}};
  uint64_t txDurationUs = 37;
  uint64_t rxDurationUs = 43;
  uint64_t pointerFinishedUs = 0;
  uint64_t receiveStartedUs = 0;
  uint64_t receiveFinishedUs = 0;
  uint64_t clockStepAfterReceiveUs = 0;
  uint32_t guardUs = 0;
  bool worstWaits = false;
  bool gateHeldForEveryCall = true;
};
RecordingSdk sdk;

/// @brief Independent SDK wait-site envelope; not an adapter implementation helper.
uint64_t physicalWaitEnvelope(int timeoutMs) {
  CHECK(timeoutMs > 0);
  const uint64_t tickUs = (1000000ULL + configTICK_RATE_HZ - 1) / configTICK_RATE_HZ;
  // Bus mutex, up to four command waits, final event, NACK busy wait; two clears.
  return 7 * (static_cast<uint64_t>(timeoutMs) * 1000 + tickUs) +
         2 * (50000 + tickUs);
}

/// @brief Every SDK phase must observe the same gate held throughout terminal completion.
void backendCall() {
  ++sdk.physicalUs;
  if (!sdk.bus || !sdk.bus->gate.test_and_set(std::memory_order_acquire)) {
    sdk.gateHeldForEveryCall = false;
    if (sdk.bus) sdk.bus->gate.clear(std::memory_order_release);
  }
}

/// @brief Model application creation before binding; callbacks may only borrow these resources.
void prepare(BusContext& bus, Context& context) {
  sdk = RecordingSdk{};
  bus.handle = &nativeBus;
  bus.config = {};
  bus.config.i2c_port = I2C_NUM_0;
  bus.config.sda_io_num = GPIO_NUM_8;
  bus.config.scl_io_num = GPIO_NUM_9;
  bus.config.clk_source = I2C_CLK_SRC_DEFAULT;
  bus.config.glitch_ignore_cnt = 7;
  bus.ownerCore = 0;
  bus.maxOverheadUs = 1000;
  bus.acceptSdkCleanup = true;
  bus.eventCallbacksRegistered = false;
  bus.retired = false;
  bus.stats = {};
  bus.gate.clear();
  context.bus = &bus;
  context.device = &nativeDevice;
  context.config = {};
  context.config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  context.config.device_address = 0x47;
  context.config.scl_speed_hz = 400000;
  context.sdkTimeoutMs = 100;
  sdk.bus = &bus;
}

/// @brief Ensure callback exit releases its own gate and preserves borrowed identities/settings.
void checkReleased(BusContext& bus, const Context& context) {
  CHECK(bus.handle == &nativeBus && context.device == &nativeDevice);
  CHECK(context.sdkTimeoutMs != 0); // A borrowed configuration is never changed temporarily.
  CHECK(sdk.gateHeldForEveryCall);
  CHECK(!bus.gate.test_and_set());
  bus.gate.clear();
}

/// @brief Seven-bit targets and exact fixed frames, terminal STOP-separated pointer/read phases.
void exactTransfers() {
  for (const uint8_t address : {uint8_t{0x47}, uint8_t{0x67}, uint8_t{0x60}, uint8_t{0x61}}) {
    BusContext bus;
    Context context;
    prepare(bus, context);
    context.config.device_address = address;
    const uint8_t data[]{0x09, 0x15};
    CHECK(adapter::validateContext(context).ok());
    CHECK(sdk.transmits == 0 && sdk.receives == 0);
    CHECK(adapter::write(address, data, 2, 1000, &context).ok());
    CHECK(sdk.transmits == 1 && sdk.receives == 0);
    CHECK(sdk.txDevice == &nativeDevice && sdk.txLength == 2);
    CHECK(sdk.tx[0] == 0x09 && sdk.tx[1] == 0x15);
    CHECK(bus.stats.transmitCalls == 1 && bus.stats.successfulTxBytes == 2);
    CHECK(bus.stats.receiveCalls == 0 && bus.stats.successfulRxBytes == 0);
    CHECK(context.sdkTimeoutMs == 100);
    checkReleased(bus, context);

    prepare(bus, context);
    context.config.device_address = address;
    const uint8_t pointer = 0x00;
    uint8_t output[8]{};
    CHECK(adapter::writeRead(address, &pointer, 1, output, 8, 1000, &context).ok());
    CHECK(std::memcmp(output, sdk.rx.data(), 8) == 0);
    CHECK(sdk.txDevice == &nativeDevice && sdk.rxDevice == &nativeDevice);
    CHECK(sdk.txLength == 1 && sdk.tx[0] == 0x00 && sdk.rxLength == 8);
    CHECK(sdk.transmits == 1 && sdk.receives == 1);
    CHECK(sdk.guardUs == 2 && sdk.receiveStartedUs >= sdk.pointerFinishedUs + 2);
    CHECK(bus.stats.transmitCalls == 1 && bus.stats.receiveCalls == 1);
    CHECK(bus.stats.successfulTxBytes == 1 && bus.stats.successfulRxBytes == 8);
    CHECK(context.sdkTimeoutMs == 100);
    checkReleased(bus, context);
  }
  for (size_t length = 1; length <= 8; ++length) {
    BusContext bus;
    Context context;
    prepare(bus, context);
    const uint8_t pointer = 0;
    std::array<uint8_t, 9> output{};
    output.fill(0xA5);
    CHECK(adapter::writeRead(0x47, &pointer, 1, output.data(), length, 1000, &context).ok());
    CHECK(sdk.rxLength == length && std::memcmp(output.data(), sdk.rx.data(), length) == 0);
    CHECK(output[length] == 0xA5 && bus.stats.successfulRxBytes == length);
    checkReleased(bus, context);
  }
  BusContext bus;
  Context context;
  prepare(bus, context);
  const uint8_t pointer = 0x45;
  uint8_t output = 0;
  CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context).ok());
  CHECK(sdk.tx[0] == 0x45 && sdk.txLength == 1 && sdk.rxLength == 1);
  CHECK(context.config.device_address == 0x47 && sdk.txDevice == &nativeDevice);
  checkReleased(bus, context);
}

/// @brief Successful users reuse one physical gate without altering borrowed setup or lifecycle.
void borrowedLifecycleAndSharing() {
  BusContext bus;
  Context first;
  prepare(bus, first);
  const Context original = first;
  const auto originalBusConfig = bus.config;
  Context second = first;
  second.device = &secondDevice;
  second.config.device_address = 0x67;
  const uint8_t data[]{9, 0x15};
  const uint8_t pointer = 0;
  uint8_t output = 0;
  CHECK(adapter::write(0x47, data, 2, 1000, &first).ok());
  checkReleased(bus, first);
  CHECK(adapter::write(0x67, data, 2, 1000, &second).ok());
  CHECK(sdk.txDevice == &secondDevice && sdk.transmits == 2);
  CHECK(!bus.gate.test_and_set());
  bus.gate.clear();
  CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &first).ok());
  CHECK(sdk.rxDevice == &nativeDevice && sdk.transmits == 3 && sdk.receives == 1);
  CHECK(bus.stats.successfulTxBytes == 5 && bus.stats.successfulRxBytes == 1);
  checkReleased(bus, first);

  CHECK(first.bus == original.bus && first.device == original.device);
  CHECK(first.sdkTimeoutMs == original.sdkTimeoutMs);
  CHECK(first.config.dev_addr_length == original.config.dev_addr_length);
  CHECK(first.config.device_address == original.config.device_address);
  CHECK(first.config.scl_speed_hz == original.config.scl_speed_hz);
  CHECK(first.config.scl_wait_us == original.config.scl_wait_us);
  CHECK(first.config.flags.disable_ack_check == original.config.flags.disable_ack_check);
  CHECK(bus.config.i2c_port == originalBusConfig.i2c_port);
  CHECK(bus.config.sda_io_num == originalBusConfig.sda_io_num);
  CHECK(bus.config.scl_io_num == originalBusConfig.scl_io_num);
  CHECK(bus.config.clk_source == originalBusConfig.clk_source);
  CHECK(bus.config.glitch_ignore_cnt == originalBusConfig.glitch_ignore_cnt);
  CHECK(bus.config.intr_priority == originalBusConfig.intr_priority);
  CHECK(bus.config.trans_queue_depth == originalBusConfig.trans_queue_depth);
  CHECK(bus.config.flags.enable_internal_pullup == originalBusConfig.flags.enable_internal_pullup);
  CHECK(bus.config.flags.allow_pd == originalBusConfig.flags.allow_pd);
  CHECK(bus.ownerCore == 0 && bus.maxOverheadUs == 1000);
  CHECK(bus.acceptSdkCleanup && !bus.eventCallbacksRegistered && !bus.retired);

  // The core borrows the context until end; neither lifecycle action configures the SDK.
  sdk = RecordingSdk{};
  sdk.bus = &bus;
  TUSB32x::DriverConfig config;
  config.i2cWrite = adapter::write;
  config.i2cWriteRead = adapter::writeRead;
  config.i2cUser = &first;
  config.variant = TUSB32x::Variant::TUSB320LAI;
  config.address = TUSB32x::DeviceAddress::ADDR_LOW_0X47;
  config.i2cTimeoutMs = 1000;
  {
    TUSB32x::TUSB32x device;
    CHECK(device.bind(config).ok());
    CHECK(sdk.transmits == 0 && sdk.receives == 0);
    CHECK(device.end().ok());
    CHECK(sdk.transmits == 0 && sdk.receives == 0);
  }
  CHECK(sdk.transmits == 0 && sdk.receives == 0);
  CHECK(bus.handle == &nativeBus && first.device == &nativeDevice);
  CHECK(bus.stats.transmitCalls == 3 && bus.stats.receiveCalls == 1);
  checkReleased(bus, first);
}

/// @brief Setup, address and argument rejection do no SDK work and publish no output.
void localRejections() {
  for (unsigned scenario = 0; scenario < 35; ++scenario) {
    BusContext bus;
    Context context;
    prepare(bus, context);
    const uint8_t data[]{8, 0x80};
    std::array<uint8_t, 9> output{};
    output.fill(0xA5);
    switch (scenario) {
      case 0: context.bus = nullptr; break;
      case 1: bus.handle = nullptr; break;
      case 2: context.device = nullptr; break;
      case 3: context.sdkTimeoutMs = 0; break;
      case 4: bus.maxOverheadUs = 0; break;
      case 5: bus.acceptSdkCleanup = false; break;
      case 6: bus.ownerCore = 1; break;
      case 7: bus.ownerCore = -1; break;
      case 8: sdk.isr = true; break;
      case 9: bus.config.trans_queue_depth = 1; break;
      case 10: bus.eventCallbacksRegistered = true; break;
      case 11: context.config.dev_addr_length = I2C_ADDR_BIT_LEN_10; break;
      case 12: context.config.device_address = I2C_DEVICE_ADDRESS_NOT_USED; break;
      case 13: context.config.device_address = 0x8E; break;
      case 14: context.config.scl_speed_hz = 0; break;
      case 15: context.config.scl_speed_hz = 400001; break;
      case 16: context.config.flags.disable_ack_check = 1; break;
      case 17: bus.retired = true; break;
      case 18: bus.gate.test_and_set(); break;
      default: break;
    }
    if (scenario <= 17) {
      const auto validated = adapter::validateContext(context);
      CHECK(validated.is(scenario == 17 ? Err::I2C_BUS : Err::I2C_ERROR));
      CHECK(sdk.transmits == 0 && sdk.receives == 0);
    }
    auto status = TUSB32x::Status::Ok();
    if (scenario <= 18) status = adapter::write(0x47, data, 2, 1000, &context);
    else if (scenario == 19) status = adapter::write(0x47, data, 2, 1000, nullptr);
    else if (scenario == 20) status = adapter::write(0x47, nullptr, 2, 1000, &context);
    else if (scenario == 21) status = adapter::write(0x47, data, 0, 1000, &context);
    else if (scenario == 22) status = adapter::write(0x47, data, 1, 1000, &context);
    else if (scenario == 23) status = adapter::write(0x47, data, 3, 1000, &context);
    else if (scenario == 24) status = adapter::write(0x8E, data, 2, 1000, &context);
    else if (scenario == 25) status = adapter::write(0x8F, data, 2, 1000, &context);
    else if (scenario == 26) status = adapter::write(0x67, data, 2, 1000, &context);
    else if (scenario == 27) status = adapter::writeRead(0x47, data, 1, output.data(), 0, 1000, &context);
    else if (scenario == 28) status = adapter::writeRead(0x47, data, 1, output.data(), 9, 1000, &context);
    else if (scenario == 29) status = adapter::writeRead(0x47, data, 2, output.data(), 1, 1000, &context);
    else if (scenario == 30) status = adapter::writeRead(0x47, data, 0, output.data(), 1, 1000, &context);
    else if (scenario == 31) status = adapter::writeRead(0x47, nullptr, 1, output.data(), 1, 1000, &context);
    else if (scenario == 32) status = adapter::writeRead(0x47, data, 1, nullptr, 1, 1000, &context);
    else if (scenario == 33) status = adapter::write(0x47, data, 2, 0, &context);
    else status = adapter::writeRead(0x47, data, 1, output.data(), 1, 0, &context);
    const auto expected = scenario == 17 ? Err::I2C_BUS : scenario == 18 ? Err::I2C_BUSY :
                          scenario >= 33 ? Err::I2C_TIMEOUT : Err::I2C_ERROR;
    CHECK(status.is(expected));
    CHECK(sdk.transmits == 0 && sdk.receives == 0 && sdk.guardUs == 0);
    CHECK(bus.stats.transmitCalls == 0 && bus.stats.receiveCalls == 0);
    CHECK(bus.stats.successfulTxBytes == 0 && bus.stats.successfulRxBytes == 0);
    for (const auto byte : output) CHECK(byte == 0xA5);
    if (scenario == 18) CHECK(bus.gate.test_and_set()); // Preserve another user's ownership.
    else CHECK(!bus.gate.test_and_set());
    bus.gate.clear();
  }
}

/// @brief Reject unaudited SCL/clock settings without SDK work or changed caller data.
void supportedBusConfiguration() {
  for (const uint32_t busHz : {uint32_t{0}, uint32_t{1}, uint32_t{99999},
                              uint32_t{100000}, uint32_t{400000}, uint32_t{400001}}) {
    for (const bool receiving : {false, true}) {
      BusContext bus;
      Context context;
      prepare(bus, context);
      context.config.scl_speed_hz = busHz;
      const bool supported = busHz == 100000 || busHz == 400000;
      const auto validated = adapter::validateContext(context);
      CHECK(supported ? validated.ok() : validated.is(Err::I2C_ERROR));
      CHECK(sdk.transmits == 0 && sdk.receives == 0);
      const uint8_t data[]{0x09, 0x15};
      uint8_t output = 0xA5;
      const auto status = receiving ?
          adapter::writeRead(0x47, data, 1, &output, 1, 1000, &context) :
          adapter::write(0x47, data, 2, 1000, &context);
      CHECK(supported ? status.ok() : status.is(Err::I2C_ERROR));
      CHECK(context.config.scl_speed_hz == busHz);
      CHECK(bus.config.clk_source == I2C_CLK_SRC_DEFAULT);
      CHECK(sdk.transmits == (supported ? 1U : 0U));
      CHECK(sdk.receives == (supported && receiving ? 1U : 0U));
      if (!supported) CHECK(sdk.guardUs == 0 && output == 0xA5);
      checkReleased(bus, context);
    }
  }
  for (const bool receiving : {false, true}) {
    BusContext bus;
    Context context;
    prepare(bus, context);
    const auto unsupportedClock = static_cast<i2c_clock_source_t>(I2C_CLK_SRC_DEFAULT + 1);
    bus.config.clk_source = unsupportedClock;
    CHECK(adapter::validateContext(context).is(Err::I2C_ERROR));
    const uint8_t data[]{0x09, 0x15};
    uint8_t output = 0xA5;
    const auto status = receiving ?
        adapter::writeRead(0x47, data, 1, &output, 1, 1000, &context) :
        adapter::write(0x47, data, 2, 1000, &context);
    CHECK(status.is(Err::I2C_ERROR));
    CHECK(sdk.transmits == 0 && sdk.receives == 0 && sdk.guardUs == 0);
    CHECK(output == 0xA5 && bus.config.clk_source == unsupportedClock);
    checkReleased(bus, context);
  }
}

/// @brief Preserve full terminal backend detail; only the SDK's timeout code denotes timeout.
/// @details IDF6.1 uses INVALID_RESPONSE for every non-DONE transfer, including
/// some timeout paths. Exercise it at pointer and receive boundaries without
/// inferring a NACK; failed receives must still retire all controller users.
void failuresAndRetirement() {
  constexpr esp_err_t codes[]{ESP_FAIL, ESP_ERR_NO_MEM, ESP_ERR_INVALID_ARG,
    ESP_ERR_INVALID_STATE, ESP_ERR_NOT_FOUND, ESP_ERR_NOT_SUPPORTED, ESP_ERR_TIMEOUT,
    ESP_ERR_INVALID_RESPONSE,
    0x12345678, -123456789};
  for (const esp_err_t code : codes) {
    for (const bool receiving : {false, true}) {
      BusContext bus;
      Context context;
      prepare(bus, context);
      // Even a deliberately late backend error remains the primary error.
      if (receiving) { sdk.rxError = code; sdk.rxDurationUs = 2000000; }
      else { sdk.txError = code; sdk.txDurationUs = 2000000; }
      const uint8_t pointer = 9;
      uint8_t output = 0xA5;
      const auto status = adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context);
      CHECK(status.is(code == ESP_ERR_TIMEOUT ? Err::I2C_TIMEOUT : Err::I2C_ERROR));
      CHECK(status.detail == code && status.msg != nullptr && status.msg[0] != '\0');
      CHECK(sdk.transmits == 1 && sdk.receives == (receiving ? 1U : 0U));
      CHECK(bus.stats.transmitCalls == 1 && bus.stats.receiveCalls == (receiving ? 1U : 0U));
      CHECK(bus.stats.successfulTxBytes == (receiving ? 1U : 0U));
      CHECK(bus.stats.successfulRxBytes == 0 && output == 0xA5);
      CHECK(bus.retired == receiving);
      checkReleased(bus, context);
      if (receiving) {
        Context other = context;
        other.device = &secondDevice;
        other.config.device_address = 0x67;
        CHECK(adapter::validateContext(other).is(Err::I2C_BUS));
        const uint8_t data[]{9, 0x15};
        CHECK(adapter::write(0x67, data, 2, 1000, &other).is(Err::I2C_BUS));
        CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context).is(Err::I2C_BUS));
        CHECK(sdk.transmits == 1 && sdk.receives == 1 && output == 0xA5);
        checkReleased(bus, context);
      }
    }
    BusContext bus;
    Context context;
    prepare(bus, context);
    sdk.txError = code;
    const uint8_t data[]{9, 0x15};
    const auto status = adapter::write(0x47, data, 2, 1000, &context);
    CHECK(status.is(code == ESP_ERR_TIMEOUT ? Err::I2C_TIMEOUT : Err::I2C_ERROR));
    CHECK(status.detail == code && !bus.retired);
    CHECK(bus.stats.transmitCalls == 1 && bus.stats.successfulTxBytes == 0);
    checkReleased(bus, context);
  }
}

/// @brief Aggregate deadline includes SDK locks/reused waits, cleanup, both phases and tick rounding.
void aggregateTime() {
  const uint8_t pointer = 8;
  const uint8_t data[]{9, 0x15};
  // A 400ms example budget fits 1ms ticks but cannot reserve two 10ms-tick
  // envelopes. A 600ms budget fits both without relaxing the SDK wait cap.
  for (const uint32_t budgetMs : {uint32_t{400}, uint32_t{600}}) {
    BusContext bus;
    Context context;
    prepare(bus, context);
    bus.maxOverheadUs = 2000;
    context.sdkTimeoutMs = 10;
    sdk.worstWaits = true;
    const auto started = sdk.physicalUs;
    uint8_t output = 0xA5;
    const auto status = adapter::writeRead(0x47, &pointer, 1, &output, 1, budgetMs, &context);
    const bool fits = configTICK_RATE_HZ == 1000 || budgetMs == 600;
    CHECK(fits ? status.ok() : status.is(Err::I2C_TIMEOUT));
    CHECK(sdk.physicalUs - started + bus.maxOverheadUs <= static_cast<uint64_t>(budgetMs) * 1000);
    if (fits) {
      CHECK(sdk.transmits == 1 && sdk.receives == 1 && output == 0x30);
      CHECK(sdk.txTimeoutMs == 10 && sdk.rxTimeoutMs == 10);
      CHECK(sdk.physicalUs - started >= 2 * physicalWaitEnvelope(10) + 2);
    } else {
      CHECK(sdk.transmits == 0 && sdk.receives == 0 && sdk.guardUs == 0 && output == 0xA5);
    }
    checkReleased(bus, context);
  }
  for (const bool receiving : {false, true}) {
    for (const uint32_t budgetMs : {uint32_t{0}, uint32_t{1}, uint32_t{100}, uint32_t{150},
                                   uint32_t{200}, uint32_t{300}, uint32_t{500}, uint32_t{1000},
                                   uint32_t{2000}, std::numeric_limits<uint32_t>::max()}) {
      BusContext bus;
      Context context;
      prepare(bus, context);
      context.sdkTimeoutMs = std::numeric_limits<uint16_t>::max();
      sdk.worstWaits = true;
      const auto started = sdk.physicalUs;
      uint8_t output = 0xA5;
      const auto status = receiving ?
        adapter::writeRead(0x47, &pointer, 1, &output, 1, budgetMs, &context) :
        adapter::write(0x47, data, 2, budgetMs, &context);
      CHECK(status.ok() || status.is(Err::I2C_TIMEOUT));
      CHECK(sdk.physicalUs - started <= static_cast<uint64_t>(budgetMs) * 1000);
      const int minimumTimeoutMs = (1000 + configTICK_RATE_HZ - 1) / configTICK_RATE_HZ;
      const uint64_t minimumUs = (receiving ? 2U : 1U) * physicalWaitEnvelope(minimumTimeoutMs) +
                                 bus.maxOverheadUs + 2;
      if (static_cast<uint64_t>(budgetMs) * 1000 < minimumUs)
        CHECK(sdk.transmits == 0 && sdk.receives == 0);
      if (sdk.transmits) {
        CHECK(sdk.txTimeoutMs > 0 && sdk.txTimeoutMs <= context.sdkTimeoutMs);
        CHECK(static_cast<uint64_t>(sdk.txTimeoutMs) * 1000 >= 1000000ULL / configTICK_RATE_HZ);
        if (budgetMs == 1000) CHECK(sdk.txTimeoutMs < (receiving ? 100 : 150));
        if (budgetMs == std::numeric_limits<uint32_t>::max())
          CHECK(sdk.txTimeoutMs == std::numeric_limits<uint16_t>::max());
        if (sdk.receives) {
          CHECK(receiving && status.ok() && output == 0x30);
          CHECK(sdk.rxTimeoutMs > 0 && sdk.rxTimeoutMs <= context.sdkTimeoutMs);
          CHECK(static_cast<uint64_t>(sdk.rxTimeoutMs) * 1000 >= 1000000ULL / configTICK_RATE_HZ);
        } else CHECK(output == 0xA5);
      } else CHECK(status.is(Err::I2C_TIMEOUT) && output == 0xA5);
      CHECK(!bus.retired);
      checkReleased(bus, context);
    }
  }
  // A user cap shorter than one native tick may not become a zero/infinite SDK wait.
  {
    BusContext bus;
    Context context;
    prepare(bus, context);
    context.sdkTimeoutMs = 1;
    const auto status = adapter::write(0x47, data, 2, 1000, &context);
    if (configTICK_RATE_HZ == 1000) CHECK(status.ok() && sdk.txTimeoutMs == 1);
    else CHECK(status.is(Err::I2C_TIMEOUT) && sdk.transmits == 0);
    checkReleased(bus, context);
  }
  // A successful but late pointer transfer cannot give receive a new full deadline.
  {
    BusContext bus;
    Context context;
    prepare(bus, context);
    sdk.txDurationUs = 950000;
    uint8_t output = 0xA5;
    CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context).is(Err::I2C_TIMEOUT));
    CHECK(sdk.transmits == 1 && sdk.receives == 0 && output == 0xA5 && !bus.retired);
    CHECK(bus.stats.successfulTxBytes == 1);
    checkReleased(bus, context);
  }
  // Qualified-bound violations are observable errors even after successful backend effects.
  for (const bool receiving : {false, true}) {
    BusContext bus;
    Context context;
    prepare(bus, context);
    if (receiving) sdk.rxDurationUs = 1000000;
    else sdk.txDurationUs = 1000000;
    uint8_t output = 0xA5;
    const auto status = receiving ?
      adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context) :
      adapter::write(0x47, data, 2, 1000, &context);
    CHECK(status.is(Err::I2C_TIMEOUT) && output == 0xA5 && !bus.retired);
    CHECK(bus.stats.successfulTxBytes == (receiving ? 1U : 2U));
    CHECK(bus.stats.successfulRxBytes == (receiving ? 1U : 0U));
    checkReleased(bus, context);
  }
  // Whole-callback overhead is reserved before the first backend call.
  {
    BusContext bus;
    Context context;
    prepare(bus, context);
    bus.maxOverheadUs = 1000000;
    CHECK(adapter::write(0x47, data, 2, 1000, &context).is(Err::I2C_TIMEOUT));
    CHECK(sdk.transmits == 0);
    checkReleased(bus, context);
  }
}

/// @brief Caller publication is all-or-nothing; no later clock check can reject copied bytes.
void publicationAtDeadline() {
  for (const uint64_t lateUs : {uint64_t{0}, uint64_t{1}}) {
    BusContext bus;
    Context context;
    prepare(bus, context);
    sdk.receiveFinishedUs = sdk.physicalUs + 1000000 + lateUs;
    sdk.clockStepAfterReceiveUs = 2;
    const uint8_t pointer = 0;
    uint8_t output = 0xA5;
    const auto status = adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context);
    CHECK(status.ok() || output == 0xA5);
    if (lateUs == 0) CHECK(status.ok() && output == 0x30);
    else CHECK(status.is(Err::I2C_TIMEOUT) && output == 0xA5);
    CHECK(sdk.transmits == 1 && sdk.receives == 1 && !bus.retired);
    checkReleased(bus, context);
  }
}

/// @brief Clock conversion preserves 64-bit uptime; backend counts saturate independently.
void clockAndSaturation() {
  BusContext bus;
  Context context;
  prepare(bus, context);
  sdk.physicalUs = (uint64_t{1} << 32) * 1000 - 1;
  CHECK(adapter::ownerNowMs() == (uint64_t{1} << 32) - 1);
  sdk.physicalUs += 2;
  CHECK(adapter::ownerNowMs() == (uint64_t{1} << 32));
  sdk.physicalUs = (uint64_t{1} << 40) * 1000 + 999;
  CHECK(adapter::ownerNowMs() == (uint64_t{1} << 40));
  const auto maximum = std::numeric_limits<uint64_t>::max();
  bus.stats = {maximum, maximum, maximum - 1, maximum - 1};
  const uint8_t pointer = 0;
  uint8_t output[8]{};
  CHECK(adapter::writeRead(0x47, &pointer, 1, output, 8, 1000, &context).ok());
  CHECK(bus.stats.transmitCalls == maximum && bus.stats.receiveCalls == maximum);
  CHECK(bus.stats.successfulTxBytes == maximum && bus.stats.successfulRxBytes == maximum);
  const uint8_t data[]{8, 0};
  CHECK(adapter::write(0x47, data, 2, std::numeric_limits<uint32_t>::max(), &context).ok());
  CHECK(bus.stats.successfulTxBytes == maximum && bus.stats.transmitCalls == maximum);
  checkReleased(bus, context);
}
} // namespace

esp_err_t i2c_master_transmit(i2c_master_dev_handle_t device, const uint8_t* data,
                              size_t length, int timeoutMs) {
  backendCall();
  ++sdk.transmits;
  sdk.txDevice = device;
  sdk.txLength = length;
  sdk.txTimeoutMs = timeoutMs;
  CHECK(length <= sdk.tx.size());
  CHECK(timeoutMs > 0);
  if (length <= sdk.tx.size()) std::memcpy(sdk.tx.data(), data, length);
  sdk.physicalUs += sdk.worstWaits ? physicalWaitEnvelope(timeoutMs) : sdk.txDurationUs;
  sdk.pointerFinishedUs = sdk.physicalUs;
  return sdk.txError;
}

esp_err_t i2c_master_receive(i2c_master_dev_handle_t device, uint8_t* data,
                             size_t length, int timeoutMs) {
  backendCall();
  ++sdk.receives;
  sdk.rxDevice = device;
  sdk.rxLength = length;
  sdk.rxTimeoutMs = timeoutMs;
  sdk.receiveStartedUs = sdk.physicalUs;
  CHECK(length <= sdk.rx.size());
  CHECK(timeoutMs > 0);
  // Deliberately mutate the SDK destination even on error: the caller must retain its old data.
  if (length <= sdk.rx.size()) std::memcpy(data, sdk.rx.data(), length);
  sdk.physicalUs += sdk.worstWaits ? physicalWaitEnvelope(timeoutMs) : sdk.rxDurationUs;
  if (sdk.receiveFinishedUs) sdk.physicalUs = sdk.receiveFinishedUs;
  return sdk.rxError;
}

int64_t esp_timer_get_time() {
  const auto sampled = static_cast<int64_t>(sdk.physicalUs);
  if (sdk.receives) sdk.physicalUs += sdk.clockStepAfterReceiveUs;
  return sampled;
}
void esp_rom_delay_us(uint32_t duration) {
  CHECK(sdk.bus && sdk.bus->gate.test_and_set());
  sdk.guardUs += duration;
  sdk.physicalUs += duration;
}
int xPortGetCoreID() { return sdk.core; }
bool xPortInIsrContext() { return sdk.isr; }

/// @brief Execute the real adapter against finite scripted SDK calls, with no board claims.
int main() {
  exactTransfers();
  borrowedLifecycleAndSharing();
  localRejections();
  supportedBusConfiguration();
  failuresAndRetirement();
  aggregateTime();
  publicationAtDeadline();
  clockAndSaturation();
  std::printf("Native IDF contract (%d Hz): %u checks, %u failures\n",
              configTICK_RATE_HZ, checks, failures);
  return failures ? 1 : 0;
}
