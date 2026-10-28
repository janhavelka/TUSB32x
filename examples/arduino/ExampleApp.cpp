/// @file ExampleApp.cpp
/// @brief Pinned Arduino owner, explicit startup, shared CLI and USB worker isolation.
#include "ExampleApp.h"
#include "BoardConfig.h"
#include "Tusb32xWire.h"
#include "../common/Tusb32xCli.h"
#include "../common/ExampleOwner.h"
#include "../common/OutputMailbox.h"
#include "FirmwareBuild.h"

#include <Arduino.h>
#include <Wire.h>
#include <esp_arduino_version.h>
#include <esp_rom_sys.h>
#include <esp32-hal-i2c.h>
#include <driver/gpio.h>
#include <soc/usb_pins.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <atomic>
#include <cstdarg>
#include <cstdio>

// These Arduino macros collide with portable enum members; keep the boundary local.
#undef DEFAULT
#undef INTERRUPT
#if ARDUINO_RUNNING_CORE < 0
#error "Wire owner must use a pinned Arduino loop core"
#endif

namespace tusb32x_arduino {
namespace {
namespace t = TUSB32x;
Context context;
t::TUSB32x primary;
t::TUSB32x secondary;
tusb32x_example::ExampleOwner primaryOwner(primary, BoardConfig::operationMs);
tusb32x_example::ExampleOwner secondaryOwner(secondary, BoardConfig::operationMs, 1000);
tusb32x_cli::OutputMailbox output;
StaticTask_t usbTaskStorage;
StackType_t usbStack[2048];
bool twoDevices = false;
bool ownsWire = false;
bool wireReleaseAttempted = false;
bool primaryStarted = false;
bool secondaryStarted = false;
bool primaryTurn = true;
bool stopping = false;
bool cliReady = false;
bool irqAttached = false;
bool primaryInputConfigured = false;
int heldInput = -1;
uint64_t startedAt = 0;
uint64_t primaryReport = 0;
uint64_t secondaryReport = 0;
std::atomic<bool> stopRequested{false};
portMUX_TYPE irqMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool irqSignalled = false;

/// @brief Copy one bounded output chunk; never call the USB SDK in the owner task.
size_t queueOutput(const char* data, size_t length, void*) { return output.offer(data, length); }
/// @brief Fresh native 64-bit milliseconds, independent of wrapping Arduino millis().
uint64_t ownerTime(void*) { return ownerNowMs(); }
/// @brief Optional input observation; pins are configured only after board readiness opt-in.
bool inputLevel(bool& value, void*) {
  if (!primaryInputConfigured) return false;
  value = digitalRead(BoardConfig::irq) != LOW;
  return true;
}
/// @brief Reject unprepared targets and the secondary owner's physical device without I2C.
t::Status validateTarget(const t::DriverConfig& config, void*) {
  if (!ownsWire || config.variant != BoardConfig::variant ||
      static_cast<uint8_t>(config.address) != BoardConfig::address)
    return t::Status::Error(t::Err::INVALID_CONFIG, "Only the prepared primary fixture is selectable");
  return validateContext(context);
}
/// @brief Build one borrowed terminal callback configuration for the selected physical target.
t::DriverConfig driverConfig(t::Variant variant, uint8_t address) {
  t::DriverConfig config;
  config.variant = variant;
  config.address = static_cast<t::DeviceAddress>(address);
  config.i2cWrite = write;
  config.i2cWriteRead = writeRead;
  config.i2cUser = &context;
  config.i2cTimeoutMs = BoardConfig::callbackMs;
  return config;
}
/// @brief Populate actual firmware identity and explicit runtime wiring metadata.
tusb32x_cli::Platform platform() {
  tusb32x_cli::Platform value;
  value.nowMs = ownerTime;
  value.write = queueOutput;
  value.readInput = inputLevel;
  value.validateTarget = validateTarget;
  value.identity.git = TUSB32X_BUILD_GIT;
  value.identity.tree = TUSB32X_BUILD_TREE;
  value.identity.build = TUSB32X_BUILD_UTC;
  value.identity.framework = "Arduino-ESP32";
  value.identity.frameworkVersion = ESP_ARDUINO_VERSION_STR;
  value.identity.target = TUSB32X_BUILD_TARGET;
  value.bus = "Wire0";
  value.framing = "pointer-STOP-read-STOP";
  value.busHz = BoardConfig::busHz;
  value.sdaPin = BoardConfig::sda;
  value.sclPin = BoardConfig::scl;
  value.irqPin = BoardConfig::irq;
  return value;
}
tusb32x_cli::Cli processor(primary, driverConfig(BoardConfig::variant, BoardConfig::address), platform());

/// @brief Append a complete bounded application record to the same ordered CLI output queue.
void appRecord(const char* format, ...) {
  char record[tusb32x_cli::ConsoleBuffer::RECORD_CAPACITY];
  va_list arguments;
  va_start(arguments, format);
  const int length = vsnprintf(record, sizeof(record), format, arguments);
  va_end(arguments);
  if (length > 0 && static_cast<size_t>(length) < sizeof(record))
    processor.enqueueApplicationRecord(record, static_cast<size_t>(length));
}
/// @brief Only the worker enters USB transmit code; an SDK lock cannot block owner progress.
size_t usbWrite(const char* data, size_t length, void*) {
#if ARDUINO_USB_MODE
  if (!Serial) return 0; // HWCDC otherwise may replace old FIFO data while disconnected.
#endif
  const int available = Serial.availableForWrite();
  if (available <= 0) return 0;
  if (length > static_cast<size_t>(available)) length = static_cast<size_t>(available);
  return Serial.write(reinterpret_cast<const uint8_t*>(data), length);
}
/// @brief Static worker retains mailbox data during USB stalls and never accesses Wire/core state.
void usbWorker(void*) {
  for (;;) {
    output.drain(usbWrite, nullptr);
    vTaskDelay(1);
  }
}
/// @brief ISR signals a coalesced request only; the owner later performs any explicit service.
void ARDUINO_ISR_ATTR interruptSignal() {
  portENTER_CRITICAL_ISR(&irqMux);
  irqSignalled = true;
  portEXIT_CRITICAL_ISR(&irqMux);
}
/// @brief Consume the ISR signal under the matching platform synchronization boundary.
bool takeInterrupt() {
  portENTER_CRITICAL(&irqMux);
  const bool signalled = irqSignalled;
  irqSignalled = false;
  portEXIT_CRITICAL(&irqMux);
  return signalled;
}
/// @brief Validate pin choices before any GPIO or bus setup; optional enable stays externally owned.
bool validPins() {
  // These pins already belong to this application's native USB console. The SDK
  // peripheral manager would detach USB if Wire or pinMode acquired either pin.
  const auto consolePin = [](int pin) {
    return pin == USBPHY_DM_NUM || pin == USBPHY_DP_NUM;
  };
  if (!GPIO_IS_VALID_OUTPUT_GPIO(BoardConfig::sda) || !GPIO_IS_VALID_OUTPUT_GPIO(BoardConfig::scl) ||
      BoardConfig::sda == BoardConfig::scl || consolePin(BoardConfig::sda) ||
      consolePin(BoardConfig::scl)) return false;
  const int pins[] = {BoardConfig::irq, twoDevices ? BoardConfig::secondIrq : -1, BoardConfig::enable};
  for (int pin : pins) {
    if (pin >= 0 && (!GPIO_IS_VALID_GPIO(pin) || pin == BoardConfig::sda || pin == BoardConfig::scl)) return false;
  }
  if (consolePin(BoardConfig::irq) || (twoDevices && consolePin(BoardConfig::secondIrq))) return false;
  return BoardConfig::irq < 0 || !twoDevices || BoardConfig::irq != BoardConfig::secondIrq;
}
/// @brief Attempt owned controller release once; failed teardown keeps ownership and blocks reuse.
bool releaseWire() {
  if (!ownsWire) return true;
  if (wireReleaseAttempted) return false;
  wireReleaseAttempted = true;
  context.retired = true;
  if (!Wire.end()) {
    appRecord("APP bus=release_failed ownership=retained controller=retired\n");
    return false;
  }
  ownsWire = false;
  return true;
}
/// @brief Prepare the SDK handle with checked read-only identity traffic, outside bounded callbacks.
/// @details Exclusive setup runs before either binding. No identity match or board qualification is inferred.
t::Status prepare(uint8_t address) {
  if (context.gate.test_and_set(std::memory_order_acquire))
    return t::Status::Error(t::Err::I2C_BUSY, "Setup gate busy");
  const uint16_t saved = Wire.getTimeOut();
  Wire.setTimeOut(context.wireTimeoutMs);
  Wire.beginTransmission(address);
  const size_t accepted = Wire.write(static_cast<uint8_t>(0));
  t::Status status;
  if (accepted != 1) {
    Wire.flush();
    status = t::Status::Error(t::Err::I2C_ERROR, "Setup pointer acceptance failed", static_cast<int32_t>(accepted));
  } else {
    const uint8_t code = Wire.endTransmission(true);
    if (code) status = t::Status::Error(code == 5 ? t::Err::I2C_TIMEOUT : t::Err::I2C_ERROR,
                                      "Setup pointer failed", code);
    else {
      esp_rom_delay_us(2);
      const size_t received = Wire.requestFrom(address, static_cast<size_t>(1), true);
      const int byte = received == 1 ? Wire.read() : -1;
      if (received != 1 || byte < 0 || byte > 255) {
        context.retired = true;
        status = t::Status::Error(t::Err::I2C_ERROR, "Setup receive failed", static_cast<int32_t>(received));
      }
    }
  }
  Wire.setTimeOut(saved);
  if (status.ok()) {
    const uint8_t bit = address == 0x47 ? 1 : address == 0x67 ? 2 : address == 0x60 ? 4 : 8;
    context.preparedAddresses |= bit;
  }
  context.gate.clear(std::memory_order_release);
  return status;
}
/// @brief Report the owner's consumed result independently of whether a USB host is reading.
void reportOwner(const char* name, const tusb32x_example::ExampleOwner& owner, uint64_t& seen) {
  if (seen == owner.reportSequence()) return;
  seen = owner.reportSequence();
  const auto status = owner.lastStatus();
  const auto& result = owner.lastResult();
  appRecord("APP owner=%s report=%llu latest_code=%s latest_detail=%ld result_valid=%u result_code=%s result_detail=%ld token=%llu kind=%u effect=%u callbacks=%lu\n",
      name, static_cast<unsigned long long>(seen), t::errorName(status.code), static_cast<long>(status.detail),
      owner.lastResultValid(), owner.lastResultValid() ? t::errorName(result.status.code) : "UNAVAILABLE",
      owner.lastResultValid() ? static_cast<long>(result.status.detail) : 0L,
      static_cast<unsigned long long>(result.token.value),
      static_cast<unsigned>(result.kind), static_cast<unsigned>(result.hardwareEffect),
      static_cast<unsigned long>(result.transfers));
}
/// @brief Stop admission, end both bindings and only then release this application's Wire bus.
void shutdown() {
  stopping = true;
  if (irqAttached) { detachInterrupt(BoardConfig::secondIrq); irqAttached = false; }
  primaryOwner.stop();
  if (twoDevices) secondaryOwner.stop();
  reportOwner("primary", primaryOwner, primaryReport);
  if (twoDevices) reportOwner("secondary", secondaryOwner, secondaryReport);
  const auto progress = primary.progress();
  t::OperationResult result;
  if (progress.resultPending && primary.resultSnapshot(progress.token, result).ok()) {
    appRecord("APP shutdown=retained_cli_result token=%llu code=%s detail=%ld effect=%u\n",
        static_cast<unsigned long long>(result.token.value), t::errorName(result.status.code),
        static_cast<long>(result.status.detail), static_cast<unsigned>(result.hardwareEffect));
  }
  const bool hadWire = ownsWire || wireReleaseAttempted;
  const bool released = releaseWire();
  appRecord("APP shutdown=complete results=retained_or_reported bus=%s usb=alive\n",
      !hadWire ? "not_owned" : released ? "released" : "release_failed");
}
} // namespace

void setupExample(bool useTwoDevices) {
  twoDevices = useTwoDevices;
  startedAt = ownerNowMs();
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);
#if !ARDUINO_USB_MODE
  Serial.enableReboot(false); // A monitor's 1200-baud/DTR/RTS dance must not own the service lifetime.
#endif
  cliReady = processor.setup().ok();
  const auto worker = xTaskCreateStatic(usbWorker, "tusb-usb", sizeof(usbStack), nullptr, 1,
                                      usbStack, &usbTaskStorage);
  if (!worker || !cliReady) {
    if (worker) {
      constexpr char error[] = "APP setup=failed reason=invalid_cli_configuration\n";
      output.offer(error, sizeof(error) - 1);
    }
    return;
  }
  appRecord("APP setup=console variant=%u address=0x%02X second=%u second_address=0x%02X sda=%d scl=%d irq=%d second_irq=%d enable_external=%d hz=%lu callback_ms=%lu operation_ms=%lu overhead_us=%lu ready=%u cleanup=%u\n",
      static_cast<unsigned>(BoardConfig::variant), BoardConfig::address, twoDevices,
      BoardConfig::secondAddress, BoardConfig::sda, BoardConfig::scl, BoardConfig::irq,
      BoardConfig::secondIrq, BoardConfig::enable, static_cast<unsigned long>(BoardConfig::busHz),
      static_cast<unsigned long>(BoardConfig::callbackMs), static_cast<unsigned long>(BoardConfig::operationMs),
      static_cast<unsigned long>(BoardConfig::maxOverheadUs), BoardConfig::boardReady, BoardConfig::acceptSdkCleanup);
  if (!BoardConfig::boardReady || !BoardConfig::maxOverheadUs || !BoardConfig::acceptSdkCleanup || !validPins()) {
    appRecord("APP setup=diagnostic_only reason=unqualified_board_or_timing_or_pins\n");
    return;
  }
  const auto first = driverConfig(BoardConfig::variant, BoardConfig::address);
  const auto second = driverConfig(BoardConfig::secondVariant, BoardConfig::secondAddress);
  if (!t::validateDriverConfig(first).ok() || (twoDevices &&
      (!t::validateDriverConfig(second).ok() || BoardConfig::address == BoardConfig::secondAddress))) {
    appRecord("APP setup=diagnostic_only reason=invalid_or_duplicate_target\n");
    return;
  }
  context.wire = &Wire;
  context.ownerCore = xPortGetCoreID();
  context.maxOverheadUs = BoardConfig::maxOverheadUs;
  context.busHz = BoardConfig::busHz;
  context.acceptSdkCleanup = BoardConfig::acceptSdkCleanup;
  // Wire.begin reports success for an existing controller without checking its
  // pins, speed or initialization core. This example must create its own bus.
  if (i2cIsInit(0)) {
    appRecord("APP setup=diagnostic_only reason=Wire_controller_already_owned\n");
    return;
  }
  ownsWire = Wire.begin(BoardConfig::sda, BoardConfig::scl, BoardConfig::busHz);
  if (!ownsWire) { appRecord("APP setup=diagnostic_only reason=Wire_begin_failed\n"); return; }
  auto prepared = prepare(BoardConfig::address);
  if (prepared.ok() && twoDevices) prepared = prepare(BoardConfig::secondAddress);
  if (prepared.ok()) prepared = validateContext(context);
  if (!prepared.ok()) {
    appRecord("APP setup=diagnostic_only code=%s detail=%ld reason=preparation_failed\n",
        t::errorName(prepared.code), static_cast<long>(prepared.detail));
    (void)releaseWire(); return;
  }
  if (BoardConfig::irq >= 0) { pinMode(BoardConfig::irq, INPUT); primaryInputConfigured = true; }
  if (twoDevices && BoardConfig::secondIrq >= 0) {
    pinMode(BoardConfig::secondIrq, INPUT);
    attachInterrupt(BoardConfig::secondIrq, interruptSignal, FALLING);
    irqAttached = true;
    if (digitalRead(BoardConfig::secondIrq) == LOW) {
      portENTER_CRITICAL(&irqMux);
      irqSignalled = true; // Retain an already asserted level until startup completes.
      portEXIT_CRITICAL(&irqMux);
    }
  }
  const auto primaryBind = primary.bind(first);
  if (primaryBind.ok()) { primaryStarted = true; primaryOwner.start(ownerNowMs()); }
  else appRecord("APP owner=primary bind_code=%s detail=%ld\n", t::errorName(primaryBind.code),
                 static_cast<long>(primaryBind.detail));
  if (twoDevices) {
    const auto secondaryBind = secondary.bind(second);
    if (secondaryBind.ok()) { secondaryStarted = true; secondaryOwner.start(ownerNowMs()); }
    else appRecord("APP owner=secondary bind_code=%s detail=%ld\n", t::errorName(secondaryBind.code),
                   static_cast<long>(secondaryBind.detail));
  }
  reportOwner("primary", primaryOwner, primaryReport);
  if (twoDevices) reportOwner("secondary", secondaryOwner, secondaryReport);
  appRecord("APP setup=prepared primary_start=%u secondary_start=%u preparation=setup_only_pointer_reads\n",
      primaryStarted, secondaryStarted);
}

void requestStop() { stopRequested.store(true, std::memory_order_release); }

void loopExample() {
  if (!cliReady) { vTaskDelay(1); return; }
  if (!stopping && (stopRequested.load(std::memory_order_acquire) ||
      (BoardConfig::runMs && ownerNowMs() - startedAt >= BoardConfig::runMs))) shutdown();
  if (stopping) { processor.tick(); vTaskDelay(1); return; }
  if (twoDevices && !primaryTurn) {
    if (secondaryStarted) {
      if (secondaryOwner.startupDone() && !secondaryOwner.stopped() && takeInterrupt())
        secondaryOwner.requestService();
      secondaryOwner.tick(ownerNowMs(), 1);
      reportOwner("secondary", secondaryOwner, secondaryReport);
    }
  } else {
    if (primaryStarted && !primaryOwner.startupDone()) {
      primaryOwner.tick(ownerNowMs(), 1);
      reportOwner("primary", primaryOwner, primaryReport);
      // No CLI command was admitted: tick only drains here, never polls the startup token.
    } else {
      for (size_t count = 0; count < tusb32x_cli::ConsoleBuffer::INPUT_BUDGET; ++count) {
        if (heldInput < 0) {
          if (Serial.available() <= 0) break;
          heldInput = Serial.read();
          if (heldInput < 0) break;
        }
        const char byte = static_cast<char>(heldInput);
        if (processor.feed(&byte, 1) == 0) break;
        heldInput = -1;
      }
    }
    processor.tick();
  }
  if (twoDevices) primaryTurn = !primaryTurn;
  vTaskDelay(1); // Yield to USB/SDK tasks; no busy wait and no lock across turns.
}
} // namespace tusb32x_arduino
