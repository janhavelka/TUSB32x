/// @file test_app.cpp
/// @brief Execute real Arduino setup/loop and adapter with recording SDK calls.
/// @details One scenario per process preserves static application lifetimes. The actual
/// application source is included so the test can pump its private mailbox in place of
/// creating an unbounded USB worker. Scheduling, ownership and CLI code are unchanged.
#include "../../examples/arduino/ExampleApp.cpp"

#include <cstdlib>
#include <string>

namespace {
unsigned checks = 0;
unsigned failures = 0;
void check(bool value, const char* expression, int line) {
  ++checks;
  if (!value) { ++failures; std::printf("FAIL Arduino app %d: %s\n", line, expression); }
}
#define CHECK(value) check((value), #value, __LINE__)
struct Backend {
  bool initialized = false;
  bool beginSucceeds = true;
  bool endSucceeds = true;
  bool prepareFails = false;
  bool identityFails = false;
  unsigned beginCalls = 0;
  unsigned endCalls = 0;
  unsigned transmitCalls = 0;
  unsigned receiveCalls = 0;
  unsigned gpioCalls = 0;
  unsigned taskCalls = 0;
  unsigned writes = 0;
  unsigned reads = 0;
  uint8_t pointer = 0;
  std::size_t remaining = 0;
  std::size_t offset = 0;
  uint16_t timeout = 50;
  uint64_t microseconds = 100000;
} backend;

/// @brief Perform a finite turn and the same single mailbox drain as the USB worker.
void turn(bool usb = true) {
  tusb32x_arduino::loopExample();
  if (usb) tusb32x_arduino::output.drain(tusb32x_arduino::usbWrite, nullptr);
}
/// @brief Drain bounded fixture output; never advances an application operation.
void drain() {
  for (unsigned i = 0; i < 64; ++i) {
    tusb32x_arduino::processor.tick();
    tusb32x_arduino::output.drain(tusb32x_arduino::usbWrite, nullptr);
  }
}
/// @brief Execute one fresh serial command through the real bounded application loop.
void command(const std::string& text) {
  Serial.input = text + "\n";
  for (unsigned i = 0; i < 16 && !Serial.input.empty(); ++i) turn();
  drain();
  CHECK(Serial.input.empty());
}
}

RecordingSerial Serial;
TwoWire Wire;
bool i2cIsInit(uint8_t controller) { CHECK(controller == 0); return backend.initialized; }
bool TwoWire::begin(int sda, int scl, uint32_t frequency) {
  ++backend.beginCalls;
  CHECK(sda != USBPHY_DM_NUM && sda != USBPHY_DP_NUM);
  CHECK(scl != USBPHY_DM_NUM && scl != USBPHY_DP_NUM);
  CHECK(frequency > 0 && frequency <= 400000);
  if (backend.beginSucceeds) backend.initialized = true;
  return backend.beginSucceeds;
}
bool TwoWire::end() {
  ++backend.endCalls;
  CHECK(!tusb32x_arduino::primary.diagnostics().bound);
  CHECK(!tusb32x_arduino::secondary.diagnostics().bound);
  if (backend.endSucceeds) backend.initialized = false;
  return backend.endSucceeds;
}
uint16_t TwoWire::getTimeOut() { return backend.timeout; }
void TwoWire::setTimeOut(uint16_t value) { backend.timeout = value; }
void TwoWire::beginTransmission(uint8_t) {}
std::size_t TwoWire::write(uint8_t value) { backend.pointer = value; return 1; }
std::size_t TwoWire::write(const uint8_t* bytes, std::size_t length) {
  backend.pointer = bytes[0];
  if (length == 2) ++backend.writes;
  return length;
}
uint8_t TwoWire::endTransmission(bool stop) {
  CHECK(stop && backend.initialized);
  ++backend.transmitCalls;
  backend.microseconds += 100;
  return 0;
}
std::size_t TwoWire::requestFrom(uint8_t, std::size_t length, bool stop) {
  CHECK(stop && backend.initialized);
  ++backend.receiveCalls;
  backend.microseconds += 100;
  backend.offset = 0;
  backend.remaining = length;
  if ((backend.prepareFails && length == 1 && backend.pointer == 0) ||
      (backend.identityFails && length == 8)) {
    backend.remaining = 0;
    return 0;
  }
  return length;
}
int TwoWire::read() {
  if (backend.remaining == 0) return -1;
  --backend.remaining;
  ++backend.reads;
  constexpr uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
  if (backend.pointer == 0) return identity[backend.offset++];
  return backend.pointer == 0xA0 ? 2 : backend.pointer == 9 ? 0x94 : 0;
}
void TwoWire::flush() { backend.remaining = 0; }
int xPortGetCoreID() { return 0; }
bool xPortInIsrContext() { return false; }
void vTaskDelay(unsigned ticks) { backend.microseconds += static_cast<uint64_t>(ticks) * 1000; }
StaticTask_t* xTaskCreateStatic(void (*entry)(void*), const char*, std::size_t bytes,
    void*, unsigned, StackType_t* stack, StaticTask_t* task) {
  ++backend.taskCalls;
  CHECK(entry != nullptr && bytes > 0 && stack != nullptr && task != nullptr);
  return task;
}
void pinMode(int pin, int) {
  ++backend.gpioCalls;
  CHECK(pin != USBPHY_DM_NUM && pin != USBPHY_DP_NUM);
}
int digitalRead(int) { return 1; }
void attachInterrupt(int, void (*)(), int) { ++backend.gpioCalls; }
void detachInterrupt(int) { ++backend.gpioCalls; }
void esp_rom_delay_us(uint32_t duration) { backend.microseconds += duration; }
int64_t esp_timer_get_time() { return static_cast<int64_t>(backend.microseconds); }

int main(int argc, char** argv) {
  CHECK(argc == 2);
  const std::string scenario = argc == 2 ? argv[1] : "invalid";
  backend.initialized = scenario == "existing-bus";
  backend.beginSucceeds = scenario != "begin-failure";
  backend.endSucceeds = scenario != "end-failure" && scenario != "cleanup-failure";
  backend.prepareFails = scenario == "preparation-failure" || scenario == "cleanup-failure";
  backend.identityFails = scenario == "startup-failure";
  const bool two = scenario == "startup";
  tusb32x_arduino::setupExample(two);
  CHECK(Serial.beginCalls == 1 && backend.taskCalls == 1);
  const bool pinConflict = scenario == "usb-pins";
  const bool diagnostic = pinConflict || scenario == "existing-bus" ||
      scenario == "begin-failure" || backend.prepareFails;
  if (diagnostic) {
    CHECK(backend.gpioCalls == 0);
    CHECK(!tusb32x_arduino::primary.diagnostics().bound);
    CHECK(!tusb32x_arduino::secondary.diagnostics().bound);
    if (pinConflict || scenario == "existing-bus") CHECK(backend.beginCalls == 0);
    command("version");
    CHECK(Serial.output.find("TUSB VERSION") != std::string::npos);
    if (scenario == "existing-bus") CHECK(backend.transmitCalls == 0 && backend.endCalls == 0);
    if (pinConflict || scenario == "begin-failure") CHECK(backend.endCalls == 0);
    if (scenario == "preparation-failure") {
      CHECK(backend.endCalls == 1 && !backend.initialized);
      CHECK(!tusb32x_arduino::ownsWire);
    }
    if (scenario == "cleanup-failure") {
      CHECK(backend.endCalls == 1 && backend.initialized);
      CHECK(tusb32x_arduino::ownsWire);
      CHECK(tusb32x_arduino::context.retired);
      command("bind");
      CHECK(!tusb32x_arduino::primary.diagnostics().bound);
    }
  } else {
    // USB can stop consuming entirely while startup still takes both exact results.
    Serial.stalled = true;
    for (unsigned i = 0; i < 64; ++i) {
      const auto before = backend.transmitCalls;
      turn();
      CHECK(backend.transmitCalls - before <= 1);
    }
    CHECK(tusb32x_arduino::primaryOwner.startupDone());
    CHECK(tusb32x_arduino::primary.progress().state == TUSB32x::OperationState::IDLE);
    CHECK(tusb32x_arduino::primaryOwner.reportSequence() == (backend.identityFails ? 1 : 2));
    if (two) {
      CHECK(tusb32x_arduino::secondaryOwner.startupDone());
      CHECK(tusb32x_arduino::secondaryOwner.reportSequence() == 2);
    }
    Serial.stalled = false;
    drain();
    command("diag");
    CHECK(Serial.output.find("TUSB DIAG") != std::string::npos);
    CHECK(backend.writes == 0); // Startup never applies, resets or acknowledges IRQ.
    CHECK(tusb32x_arduino::callbackCounters(nullptr).writeReads ==
          tusb32x_arduino::primary.transportStats().totalAttempts); // Include failed callback attempts.
    if (scenario == "startup") {
      const auto counters = tusb32x_arduino::platform().counters(nullptr);
      CHECK(counters.available && counters.writes == 0 && counters.writeReads > 0);
      CHECK(counters.writeReads == tusb32x_arduino::primary.transportStats().totalAttempts);
      CHECK(tusb32x_arduino::secondary.transportStats().totalAttempts > 0);
      CHECK(counters.writeReads < backend.transmitCalls); // Exclude prewarming and the secondary owner.
      command("bus");
      CHECK(Serial.output.find("adapter_available=1 adapter_writes=0 adapter_write_reads=" +
            std::to_string(counters.writeReads)) != std::string::npos);
      command("reg 8");
      CHECK(tusb32x_arduino::callbackCounters(nullptr).writes == 0);
      CHECK(tusb32x_arduino::callbackCounters(nullptr).writeReads == counters.writeReads + 1);
      command("end");
      command("bind");
      CHECK(tusb32x_arduino::callbackCounters(nullptr).writeReads == counters.writeReads + 1);
      const uint8_t bytes[] = {8, 0};
      CHECK(tusb32x_arduino::primaryWrite(tusb32x_arduino::BoardConfig::address, bytes,
            sizeof(bytes), tusb32x_arduino::BoardConfig::callbackMs, &tusb32x_arduino::context).ok());
      CHECK(tusb32x_arduino::callbackCounters(nullptr).writes == 1 && backend.writes == 1);
    }
    if (scenario == "rx-overflow" || scenario == "rx-overflow-after-read") {
      // The hardware queue can lose the invalid suffix of a valid command.
      // The read-side race fills it after available(), immediately before dequeue.
      const bool duringRead = scenario == "rx-overflow-after-read";
      if (duringRead) {
        Serial.receive("read\n");
        turn();
        CHECK(tusb32x_arduino::primary.progress().state == TUSB32x::OperationState::ACTIVE);
        Serial.receive("\n");
        Serial.arrivalBeforeRead = std::string(250, '\n') + "reset bogus";
      } else {
        Serial.receive("version\nr");
        turn();
        CHECK(tusb32x_arduino::heldInput == 'r'); // CLI retained this byte after its completed line.
        Serial.receive(std::string(251, '\n') + "reset bogus");
      }
      CHECK(Serial.input.size() == (duringRead ? 1U : RecordingSerial::RX_CAPACITY));
      for (unsigned i = 0; i < 320; ++i) turn();
      CHECK(Serial.droppedInput == 6);
      CHECK(Serial.output.find("APP console=input_failed reason=usb_rx_capacity commands=disabled") != std::string::npos);
      CHECK(Serial.input.size() == RecordingSerial::RX_CAPACITY - (duringRead ? 1U : 0U));
      CHECK(tusb32x_arduino::heldInput == -1);
      if (duringRead) {
        const auto progress = tusb32x_arduino::primary.progress();
        CHECK(progress.resultPending);
        TUSB32x::OperationResult result;
        CHECK(tusb32x_arduino::primary.resultSnapshot(progress.token, result).ok());
        CHECK(result.status.ok()); // Input failure must not stop admitted work.
      }
      // Even if a fixture clears the SDK queue and later delivers a terminator,
      // the truncated reset prefix may never become an admitted command.
      Serial.input.clear();
      Serial.receive("\n");
      for (unsigned i = 0; i < 160; ++i) turn();
      CHECK(backend.writes == 0);
      CHECK(Serial.input == "\n");
      CHECK(Serial.output.find(" command=reset ") == std::string::npos);
      Serial.receive("reset\n");
      for (unsigned i = 0; i < 32; ++i) turn();
      CHECK(Serial.input == "\nreset\n" && backend.writes == 0);
    } else if (!backend.identityFails) {
      if (scenario == "shutdown") {
        Serial.input = "read\n";
        turn();
        CHECK(tusb32x_arduino::primary.progress().state == TUSB32x::OperationState::ACTIVE);
      } else {
        command("read");
        CHECK(tusb32x_arduino::primary.progress().resultPending);
      }
      CHECK(tusb32x_arduino::primaryOwner.reportSequence() == 2);
    }
  }
  const auto pending = tusb32x_arduino::primary.progress();
  const auto beforeStop = backend.transmitCalls;
  const auto ends = backend.endCalls;
  tusb32x_arduino::requestStop();
  turn();
  drain();
  CHECK(!tusb32x_arduino::primary.diagnostics().bound);
  CHECK(!tusb32x_arduino::secondary.diagnostics().bound);
  CHECK(backend.transmitCalls == beforeStop);
  if (pending.token.valid()) {
    TUSB32x::OperationResult result;
    CHECK(tusb32x_arduino::primary.resultSnapshot(pending.token, result).ok());
    if (scenario == "shutdown") CHECK(result.status.is(TUSB32x::Err::CANCELLED));
  }
  for (unsigned i = 0; i < 8; ++i) turn();
  if (scenario == "existing-bus") {
    CHECK(backend.initialized && backend.endCalls == 0);
    CHECK(Serial.output.find("bus=not_owned") != std::string::npos);
    CHECK(Serial.output.find("bus=released") == std::string::npos);
  }
  else if (scenario == "cleanup-failure") CHECK(backend.endCalls == ends);
  else if (!diagnostic) CHECK(backend.endCalls == 1);
  if (!backend.endSucceeds) {
    CHECK(backend.initialized && tusb32x_arduino::ownsWire);
    CHECK(Serial.output.find("bus=release_failed") != std::string::npos);
    CHECK(Serial.output.find("bus=released") == std::string::npos);
  }
  CHECK(backend.timeout == 50);
  if (scenario == "startup") {
    const auto calls = backend.transmitCalls;
    const uint8_t bytes[] = {8, 0};
    uint8_t received = 0xA5;
    tusb32x_arduino::primaryCounters.writes = UINT32_MAX - 1;
    tusb32x_arduino::primaryCounters.writeReads = UINT32_MAX - 1;
    for (unsigned i = 0; i < 2; ++i) {
      CHECK(tusb32x_arduino::primaryWrite(0x47, bytes, 2, 10, nullptr).is(TUSB32x::Err::I2C_ERROR));
      CHECK(tusb32x_arduino::primaryWriteRead(0x47, bytes, 1, &received, 1, 10, nullptr)
            .is(TUSB32x::Err::I2C_ERROR));
    }
    CHECK(tusb32x_arduino::callbackCounters(nullptr).writes == UINT32_MAX);
    CHECK(tusb32x_arduino::callbackCounters(nullptr).writeReads == UINT32_MAX);
    CHECK(received == 0xA5 && backend.transmitCalls == calls);
  }
  std::printf("Arduino app %s: %u checks, %u failures\n", scenario.c_str(), checks, failures);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
