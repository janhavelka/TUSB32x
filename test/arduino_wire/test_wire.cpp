/// @file test_wire.cpp
/// @brief Host behavioral checks of the real adapter against fixed recording SDK calls.
#include "Tusb32xWire.h"
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <limits>

namespace {
using TUSB32x::Err;
namespace adapter = tusb32x_arduino;
using tusb32x_arduino::Context;
unsigned checks = 0;
unsigned failures = 0;
#define CHECK(condition) do { ++checks; if (!(condition)) { \
  ++failures; std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
} } while (false)

/// @brief One terminal transaction script; counts are observable API calls, not wire events.
struct RecordingSdk {
  uint64_t physicalUs = 123456;
  int core = 0;
  bool isr = false;
  Context* context = nullptr;
  uint16_t timeout = 71;
  std::array<uint16_t, 8> assignedTimeouts{};
  size_t timeoutAssignments = 0;
  unsigned getters = 0;
  unsigned begins = 0;
  unsigned writes = 0;
  unsigned transmits = 0;
  unsigned receives = 0;
  unsigned reads = 0;
  unsigned flushes = 0;
  unsigned busBegins = 0;
  unsigned busEnds = 0;
  uint8_t txAddress = 0;
  uint8_t rxAddress = 0;
  std::array<uint8_t, 8> tx{};
  size_t txLength = 0;
  size_t rxLength = 0;
  bool txStop = false;
  bool rxStop = false;
  size_t accepted = 99; // 99 selects exact acceptance; short acceptance is explicit.
  size_t received = 99;
  uint8_t transmitCode = 0;
  std::array<int, 8> rx{{0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0}};
  uint64_t txDurationUs = 37;
  uint64_t rxDurationUs = 43;
  uint64_t pointerFinishedUs = 0;
  uint64_t receiveStartedUs = 0;
  uint64_t lastReadTimeUs = 0; // Optional deliberate qualification violation at buffer completion.
  uint64_t clockStepAfterReadUs = 0; // Clock can advance between post-read observations.
  uint32_t guardUs = 0;
  bool worstWaits = false;
  bool gateHeldForEveryCall = true;
};
RecordingSdk sdk;

/// @brief Every backend call consumes a microsecond and observes shared ownership.
void backendCall() {
  ++sdk.physicalUs;
  if (!sdk.context || !sdk.context->gate.test_and_set(std::memory_order_acquire)) {
    sdk.gateHeldForEveryCall = false;
    if (sdk.context) sdk.context->gate.clear(std::memory_order_release);
  }
}

/// @brief Independent envelope from the audited SDK's wait sites, not adapter helpers.
uint64_t physicalWaitEnvelope(uint16_t timeoutMs) {
  const uint64_t tickUs = (1000000ULL + configTICK_RATE_HZ - 1) / configTICK_RATE_HZ;
  // Four command waits, bus ownership, final completion and NACK-busy wait.
  // Each may cross one tick; two controller clears may each cross one tick.
  return 7 * (static_cast<uint64_t>(timeoutMs) * 1000 + tickUs) +
         2 * (50000 + tickUs);
}

/// @brief Reset only the fixture; application initialization is outside callbacks.
void prepare(Context& context, TwoWire& wire) {
  sdk = RecordingSdk{};
  context.wire = &wire;
  context.ownerCore = 0;
  context.preparedAddresses = 0x0F;
  context.busHz = 400000;
  context.wireTimeoutMs = 100;
  context.maxOverheadUs = 1000;
  context.acceptSdkCleanup = true;
  context.retired = false;
  context.stats = {};
  context.gate.clear();
  sdk.context = &context;
}

/// @brief Check return cleanup and borrowed ownership independently of transfer result.
void checkReleased(Context& context) {
  CHECK(sdk.timeout == 71);
  CHECK(sdk.busBegins == 0 && sdk.busEnds == 0);
  CHECK(sdk.gateHeldForEveryCall);
  CHECK(!context.gate.test_and_set());
  context.gate.clear();
}

/// @brief Exact buffers, STOP framing, native addressing and honest counts on success.
void exactTransfers() {
  for (const uint8_t address : {uint8_t{0x47}, uint8_t{0x67}, uint8_t{0x60}, uint8_t{0x61}}) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    const uint8_t bytes[]{0x09, 0x15};
    CHECK(adapter::validateContext(context).ok());
    CHECK(sdk.getters == 0);
    CHECK(adapter::write(address, bytes, 2, 1000, &context).ok());
    CHECK(sdk.begins == 1 && sdk.writes == 1 && sdk.transmits == 1 && sdk.receives == 0);
    CHECK(sdk.txAddress == address && sdk.txLength == 2);
    CHECK(sdk.tx[0] == 0x09 && sdk.tx[1] == 0x15 && sdk.txStop);
    CHECK(context.stats.transmitCalls == 1 && context.stats.bufferedTxBytes == 2);
    CHECK(context.stats.receiveCalls == 0 && context.stats.returnedRxBytes == 0);
    checkReleased(context);

    prepare(context, wire);
    const uint8_t pointer = 0;
    uint8_t output[8]{};
    CHECK(adapter::writeRead(address, &pointer, 1, output, 8, 1000, &context).ok());
    const uint8_t expected[]{0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
    CHECK(std::memcmp(output, expected, 8) == 0);
    CHECK(sdk.txAddress == address && sdk.rxAddress == address);
    CHECK(sdk.txLength == 1 && sdk.tx[0] == 0 && sdk.rxLength == 8);
    CHECK(sdk.txStop && sdk.rxStop);
    CHECK(sdk.guardUs == 2 && sdk.receiveStartedUs >= sdk.pointerFinishedUs + 2);
    CHECK(context.stats.transmitCalls == 1 && context.stats.receiveCalls == 1);
    CHECK(context.stats.bufferedTxBytes == 1 && context.stats.returnedRxBytes == 8);
    checkReleased(context);
  }
  for (size_t length = 1; length <= 8; ++length) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    const uint8_t pointer = 0;
    uint8_t output[8]{};
    CHECK(adapter::writeRead(0x47, &pointer, 1, output, length, 1000, &context).ok());
    CHECK(sdk.rxLength == length && sdk.reads == length);
    CHECK(context.stats.returnedRxBytes == length);
    checkReleased(context);
  }
}

/// @brief Invalid setup, callback arguments and gates never enter Wire or alter outputs.
void localRejections() {
  for (unsigned scenario = 0; scenario < 25; ++scenario) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    const uint8_t data[]{0x08, 0x80};
    uint8_t output[9];
    std::memset(output, 0xA5, sizeof(output));
    auto status = TUSB32x::Status::Ok();
    switch (scenario) {
      case 0: context.wire = nullptr; break;
      case 1: context.ownerCore = 1; break;
      case 2: sdk.isr = true; break;
      case 3: context.preparedAddresses = 0; break;
      case 4: context.preparedAddresses = 0x10; break;
      case 5: context.wireTimeoutMs = 0; break;
      case 6: context.maxOverheadUs = 0; break;
      case 7: context.acceptSdkCleanup = false; break;
      case 8: context.retired = true; break;
      case 9: context.preparedAddresses = 2; break;
      case 10: context.gate.test_and_set(); break;
      default: break;
    }
    if (scenario < 11) status = adapter::write(0x47, data, 2, 1000, &context);
    else if (scenario == 11) status = adapter::write(0x47, data, 2, 1000, nullptr);
    else if (scenario == 12) status = adapter::write(0x47, nullptr, 2, 1000, &context);
    else if (scenario == 13) status = adapter::write(0x47, data, 0, 1000, &context);
    else if (scenario == 14) status = adapter::write(0x47, data, 1, 1000, &context);
    else if (scenario == 15) status = adapter::write(0x47, data, 2, 0, &context);
    else if (scenario == 16) status = adapter::write(0x8E, data, 2, 1000, &context);
    else if (scenario == 17) status = adapter::write(0x8F, data, 2, 1000, &context);
    else if (scenario == 18) status = adapter::write(0x48, data, 2, 1000, &context);
    else if (scenario == 19) status = adapter::writeRead(0x47, data, 1, output, 0, 1000, &context);
    else if (scenario == 20) status = adapter::writeRead(0x47, data, 1, output, 9, 1000, &context);
    else if (scenario == 21) status = adapter::writeRead(0x47, data, 2, output, 1, 1000, &context);
    else if (scenario == 22) status = adapter::writeRead(0x47, data, 1, nullptr, 1, 1000, &context);
    else if (scenario == 23) status = adapter::write(0x47, data, 3, 1000, &context);
    else status = adapter::writeRead(0x47, data, 1, output, 1, 0, &context);
    const auto expected = scenario == 8 ? Err::I2C_BUS :
                          scenario == 10 ? Err::I2C_BUSY :
                          scenario == 15 || scenario == 24 ? Err::I2C_TIMEOUT : Err::I2C_ERROR;
    CHECK(status.is(expected));
    CHECK(sdk.getters == 0 && sdk.timeoutAssignments == 0 && sdk.begins == 0);
    CHECK(sdk.transmits == 0 && sdk.receives == 0 && sdk.flushes == 0);
    CHECK(context.stats.bufferedTxBytes == 0);
    for (const auto byte : output) CHECK(byte == 0xA5);
    if (scenario == 10) {
      CHECK(context.gate.test_and_set()); // Rejection must not release another user's gate.
      context.gate.clear();
    }
    checkReleased(context);
  }
}

/// @brief Only audited SCL rates enter Wire; callbacks preserve the application setting.
void supportedBusRates() {
  for (const uint32_t busHz : {uint32_t{0}, uint32_t{1}, uint32_t{99999},
                              uint32_t{100000}, uint32_t{400000}, uint32_t{400001}}) {
    for (const bool receiving : {false, true}) {
      Context context;
      TwoWire wire;
      prepare(context, wire);
      context.busHz = busHz;
      const bool supported = busHz == 100000 || busHz == 400000;
      const auto validated = adapter::validateContext(context);
      CHECK(supported ? validated.ok() : validated.is(Err::I2C_ERROR));
      CHECK(sdk.getters == 0 && sdk.begins == 0);
      const uint8_t data[]{0x09, 0x15};
      uint8_t output = 0xA5;
      const auto status = receiving ?
          adapter::writeRead(0x47, data, 1, &output, 1, 1000, &context) :
          adapter::write(0x47, data, 2, 1000, &context);
      CHECK(supported ? status.ok() : status.is(Err::I2C_ERROR));
      CHECK(context.busHz == busHz);
      CHECK(sdk.transmits == (supported ? 1U : 0U));
      CHECK(sdk.receives == (supported && receiving ? 1U : 0U));
      if (!supported) {
        CHECK(sdk.getters == 0 && sdk.timeoutAssignments == 0 && sdk.begins == 0);
        CHECK(sdk.flushes == 0 && output == 0xA5);
      }
      checkReleased(context);
    }
  }
}

/// @brief Preserve actual Wire return detail; generic code 2 is not proved address NACK.
void failuresAndRetirement() {
  constexpr uint8_t codes[]{1, 2, 3, 4, 5, 255};
  for (const uint8_t code : codes) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    sdk.transmitCode = code;
    sdk.txDurationUs = 2000000; // Actual backend failure remains primary even when late.
    const uint8_t pointer = 9;
    uint8_t result = 0xA5;
    const auto status = adapter::writeRead(0x47, &pointer, 1, &result, 1, 1000, &context);
    CHECK(status.is(code == 5 ? Err::I2C_TIMEOUT : Err::I2C_ERROR));
    CHECK(status.detail == code && status.msg != nullptr && status.msg[0] != '\0');
    CHECK(sdk.transmits == 1 && sdk.receives == 0 && result == 0xA5);
    CHECK(!context.retired);
    checkReleased(context);
  }
  for (const size_t accepted : {size_t{0}, size_t{1}}) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    sdk.accepted = accepted;
    const uint8_t bytes[]{9, 0x15};
    const auto status = adapter::write(0x47, bytes, 2, 1000, &context);
    CHECK(status.is(Err::I2C_ERROR) && status.detail == static_cast<int32_t>(accepted));
    CHECK(sdk.transmits == 0 && sdk.flushes == 1);
    CHECK(context.stats.bufferedTxBytes == accepted && context.stats.transmitCalls == 0);
    CHECK(!context.retired);
    checkReleased(context);
  }
  {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    sdk.accepted = 0;
    const uint8_t pointer = 0;
    uint8_t output = 0xA5;
    CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context)
              .is(Err::I2C_ERROR));
    CHECK(sdk.transmits == 0 && sdk.receives == 0 && sdk.flushes == 1 && output == 0xA5);
    CHECK(context.stats.bufferedTxBytes == 0);
    checkReleased(context);
  }
  for (unsigned scenario = 0; scenario < 4; ++scenario) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    if (scenario == 0) sdk.received = 0;
    if (scenario == 1) sdk.received = 3;
    if (scenario == 2) sdk.rx[2] = -123456789;
    if (scenario == 3) sdk.rx[2] = 256;
    const uint8_t pointer = 0;
    uint8_t output[8];
    std::memset(output, 0xA5, 8);
    const auto status = adapter::writeRead(0x47, &pointer, 1, output, 8, 1000, &context);
    CHECK(status.is(Err::I2C_ERROR));
    const int32_t detail = scenario == 0 ? 0 : scenario == 1 ? 3 : scenario == 2 ? -123456789 : 256;
    CHECK(status.detail == detail && context.retired);
    for (const auto byte : output) CHECK(byte == 0xA5);
    CHECK(context.stats.returnedRxBytes == (scenario < 2 ? 0 : 2));
    CHECK(sdk.transmits == 1 && sdk.receives == 1 && sdk.flushes == 0);
    checkReleased(context);
    // A different address shares the same retired physical controller and cannot proceed.
    const unsigned calls = sdk.begins;
    const uint8_t bytes[]{9, 0x14};
    CHECK(adapter::write(0x61, bytes, 2, 1000, &context).is(Err::I2C_BUS));
    CHECK(sdk.begins == calls && sdk.transmits == 1 && sdk.receives == 1);
    checkReleased(context);
  }
}

/// @brief Aggregate clipping includes independently consumed SDK waits and coarse tick bounds.
void aggregateTime() {
  for (const uint32_t budgetMs : {uint32_t{1}, uint32_t{100}, uint32_t{200}}) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    const uint8_t pointer = 8;
    uint8_t output = 0xA5;
    CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, budgetMs, &context)
              .is(Err::I2C_TIMEOUT));
    CHECK(sdk.getters == 0 && sdk.begins == 0 && sdk.timeoutAssignments == 0);
    CHECK(output == 0xA5);
    checkReleased(context);
  }
  for (const uint32_t budgetMs : {uint32_t{300}, uint32_t{500}, uint32_t{1000}, uint32_t{2000}}) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    sdk.worstWaits = true;
    const uint64_t started = sdk.physicalUs;
    const uint8_t pointer = 8;
    uint8_t output = 0xA5;
    const auto status = adapter::writeRead(0x47, &pointer, 1, &output, 1, budgetMs, &context);
    CHECK(status.ok() || status.is(Err::I2C_TIMEOUT));
    CHECK(sdk.physicalUs - started <= static_cast<uint64_t>(budgetMs) * 1000);
    if (sdk.receives) {
      CHECK(status.ok() && sdk.timeoutAssignments == 3);
      CHECK(sdk.assignedTimeouts[0] <= 100 && sdk.assignedTimeouts[1] <= 100);
      CHECK(sdk.assignedTimeouts[0] * uint64_t{1000} >= 1000000ULL / configTICK_RATE_HZ);
      CHECK(sdk.assignedTimeouts[1] * uint64_t{1000} >= 1000000ULL / configTICK_RATE_HZ);
      CHECK(output == 0x30);
    } else CHECK(output == 0xA5);
    checkReleased(context);
  }
  Context context;
  TwoWire wire;
  prepare(context, wire);
  context.wireTimeoutMs = 65535;
  sdk.worstWaits = true;
  const uint8_t pointer = 8;
  uint8_t output = 0xA5;
  CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context).ok());
  CHECK(sdk.assignedTimeouts[0] < 100 && sdk.assignedTimeouts[1] < 100);
  checkReleased(context);

  // A backend breaking its qualified overhead promise cannot gain a new receive budget.
  prepare(context, wire);
  sdk.txDurationUs = 950000;
  output = 0xA5;
  CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context).is(Err::I2C_TIMEOUT));
  CHECK(sdk.transmits == 1 && sdk.receives == 0 && output == 0xA5);
  checkReleased(context);

  prepare(context, wire);
  sdk.rxDurationUs = 1000000;
  output = 0xA5;
  CHECK(adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context).is(Err::I2C_TIMEOUT));
  CHECK(output == 0xA5 && !context.retired);
  checkReleased(context);

  // A successful write which violated the qualified bound must not report on-time success.
  prepare(context, wire);
  sdk.txDurationUs = 1000000;
  const uint8_t bytes[]{0x09, 0x15};
  CHECK(adapter::write(0x47, bytes, 2, 1000, &context).is(Err::I2C_TIMEOUT));
  CHECK(sdk.transmits == 1 && sdk.receives == 0 && !context.retired);
  checkReleased(context);

  for (const uint32_t budgetMs : {uint32_t{1}, uint32_t{150}, uint32_t{300},
                                 uint32_t{1000}, std::numeric_limits<uint32_t>::max()}) {
    prepare(context, wire);
    context.wireTimeoutMs = std::numeric_limits<uint16_t>::max();
    sdk.worstWaits = true;
    const uint64_t started = sdk.physicalUs;
    const auto status = adapter::write(0x47, bytes, 2, budgetMs, &context);
    CHECK(status.ok() || status.is(Err::I2C_TIMEOUT));
    CHECK(sdk.physicalUs - started <= static_cast<uint64_t>(budgetMs) * 1000);
    if (sdk.transmits) {
      CHECK(status.ok() && sdk.transmits == 1 && sdk.receives == 0);
      CHECK(sdk.timeoutAssignments == 2);
      CHECK(sdk.assignedTimeouts[0] * uint64_t{1000} >= 1000000ULL / configTICK_RATE_HZ);
      if (budgetMs == std::numeric_limits<uint32_t>::max()) {
        CHECK(sdk.assignedTimeouts[0] == std::numeric_limits<uint16_t>::max());
      }
    } else {
      CHECK(status.is(Err::I2C_TIMEOUT) && sdk.getters == 0);
    }
    checkReleased(context);
  }
}

/// @brief A late supplemental check cannot reject data already published to the caller.
void publicationAtDeadline() {
  for (const uint64_t lateUs : {uint64_t{0}, uint64_t{1}}) {
    Context context;
    TwoWire wire;
    prepare(context, wire);
    // Deliberately exceed the qualified callback envelope so the supplemental
    // violation path is exercised. At receive completion the next time sample
    // is exactly at (or just after) the deadline; subsequent samples advance.
    // This describes observable backend events, not private adapter call counts.
    sdk.lastReadTimeUs = sdk.physicalUs + 1000000 + lateUs;
    sdk.clockStepAfterReadUs = 2;
    const uint8_t pointer = 0;
    uint8_t output = 0xA5;
    const auto status = adapter::writeRead(0x47, &pointer, 1, &output, 1, 1000, &context);
    CHECK(status.ok() || output == 0xA5);
    if (lateUs == 0) CHECK(status.ok() && output == 0x30);
    else CHECK(status.is(Err::I2C_TIMEOUT) && output == 0xA5);
    CHECK(sdk.transmits == 1 && sdk.receives == 1 && sdk.reads == 1);
    checkReleased(context);
  }
}

/// @brief Native clock and counters retain full width, with bounded saturation fixtures.
void clockAndSaturation() {
  Context context;
  TwoWire wire;
  prepare(context, wire);
  sdk.physicalUs = (uint64_t{1} << 32) * 1000 - 1;
  CHECK(adapter::ownerNowMs() == (uint64_t{1} << 32) - 1);
  sdk.physicalUs += 2;
  CHECK(adapter::ownerNowMs() == (uint64_t{1} << 32));
  sdk.physicalUs = (uint64_t{1} << 40) * 1000 + 999;
  CHECK(adapter::ownerNowMs() == (uint64_t{1} << 40));
  const auto maximum = std::numeric_limits<uint64_t>::max();
  context.stats = {maximum, maximum, maximum - 1, maximum - 1};
  const uint8_t pointer = 0;
  uint8_t output[8]{};
  CHECK(adapter::writeRead(0x47, &pointer, 1, output, 8, 1000, &context).ok());
  CHECK(context.stats.transmitCalls == maximum && context.stats.receiveCalls == maximum);
  CHECK(context.stats.bufferedTxBytes == maximum && context.stats.returnedRxBytes == maximum);
  const uint8_t bytes[]{8, 0};
  CHECK(adapter::write(0x47, bytes, 2, std::numeric_limits<uint32_t>::max(), &context).ok());
  CHECK(context.stats.bufferedTxBytes == maximum && context.stats.transmitCalls == maximum);
  checkReleased(context);
}
} // namespace

uint16_t TwoWire::getTimeOut() { backendCall(); ++sdk.getters; return sdk.timeout; }
void TwoWire::setTimeOut(uint16_t value) {
  backendCall();
  CHECK(sdk.timeoutAssignments < sdk.assignedTimeouts.size());
  if (sdk.timeoutAssignments < sdk.assignedTimeouts.size()) {
    sdk.assignedTimeouts[sdk.timeoutAssignments++] = value;
  }
  sdk.timeout = value;
}
void TwoWire::beginTransmission(uint8_t address) {
  backendCall();
  ++sdk.begins;
  sdk.txAddress = address;
}
size_t TwoWire::write(const uint8_t* data, size_t length) {
  backendCall();
  ++sdk.writes;
  CHECK(length <= sdk.tx.size());
  sdk.txLength = length;
  if (length <= sdk.tx.size()) std::memcpy(sdk.tx.data(), data, length);
  return sdk.accepted == 99 ? length : sdk.accepted;
}
uint8_t TwoWire::endTransmission(bool stop) {
  backendCall();
  ++sdk.transmits;
  sdk.txStop = stop;
  sdk.physicalUs += sdk.worstWaits ? physicalWaitEnvelope(sdk.timeout) : sdk.txDurationUs;
  sdk.pointerFinishedUs = sdk.physicalUs;
  return sdk.transmitCode;
}
size_t TwoWire::requestFrom(uint8_t address, size_t length, bool stop) {
  backendCall();
  ++sdk.receives;
  sdk.rxAddress = address;
  sdk.rxLength = length;
  sdk.rxStop = stop;
  sdk.receiveStartedUs = sdk.physicalUs;
  sdk.physicalUs += sdk.worstWaits ? physicalWaitEnvelope(sdk.timeout) : sdk.rxDurationUs;
  return sdk.received == 99 ? length : sdk.received;
}
int TwoWire::read() {
  backendCall();
  CHECK(sdk.reads < sdk.rx.size());
  const int value = sdk.reads < sdk.rx.size() ? sdk.rx[sdk.reads++] : -1;
  if (sdk.lastReadTimeUs && sdk.reads == sdk.rxLength) sdk.physicalUs = sdk.lastReadTimeUs;
  return value;
}
void TwoWire::flush() { backendCall(); ++sdk.flushes; }
bool TwoWire::begin() { backendCall(); ++sdk.busBegins; return true; }
bool TwoWire::end() { backendCall(); ++sdk.busEnds; return true; }
int64_t esp_timer_get_time() {
  const auto sampled = static_cast<int64_t>(sdk.physicalUs);
  if (sdk.rxLength && sdk.reads == sdk.rxLength) sdk.physicalUs += sdk.clockStepAfterReadUs;
  return sampled;
}
void esp_rom_delay_us(uint32_t duration) { sdk.guardUs += duration; sdk.physicalUs += duration; }
int xPortGetCoreID() { return sdk.core; }
bool xPortInIsrContext() { return sdk.isr; }

/// @brief Run observable adapter behavior without a physical controller or installed SDK.
int main() {
  exactTransfers();
  localRejections();
  supportedBusRates();
  failuresAndRetirement();
  aggregateTime();
  publicationAtDeadline();
  clockAndSaturation();
  std::printf("Wire contract (%d Hz): %u checks, %u failures\n",
              configTICK_RATE_HZ, checks, failures);
  return failures ? 1 : 0;
}
