/// @file ExampleApp.cpp
/// @brief Single native owner, exclusive UART FIFO console, read-only startup and checked teardown.
#include "ExampleApp.h"
#include "BoardConfig.h"
#include "Tusb32xIdf.h"
#include "../common/Tusb32xCli.h"
#include "../common/ExampleOwner.h"
#include "FirmwareBuild.h"

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_idf_version.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <atomic>
#include <cinttypes>
#include <cstdarg>
#include <cstdio>

namespace tusb32x_idf {
namespace {
namespace t = TUSB32x;
constexpr uart_port_t CONSOLE = UART_NUM_0;
constexpr unsigned EVENT_BUDGET = 16;
constexpr unsigned SHUTDOWN_TURNS = 64;
BusContext busContext;
Context context;
t::TUSB32x primary;
tusb32x_example::ExampleOwner startupOwner(primary, BoardConfig::operationMs);
QueueHandle_t consoleEvents = nullptr;
bool setupAttempted = false;
bool ownsConsole = false;
bool ownsBus = false;
bool ownsDevice = false;
bool deviceReleaseAttempted = false;
bool consoleReleaseAttempted = false;
bool inputConfigured = false;
bool inputFailed = false;
bool startupStarted = false;
bool stopping = false;
bool shutdownOutputAbandoned = false;
bool cliReady = false;
unsigned shutdownTurns = 0;
uint64_t startedAt = 0;
uint64_t startupReport = 0;
char input[64];
size_t inputSize = 0;
size_t inputOffset = 0;
std::atomic<bool> stopRequested{false};

/// @brief The only UART transmitter; SDK mutex is uncontended under this application's ownership.
size_t consoleWrite(const char* data, size_t length, void*) {
  // IDF5.5.5 uart.c:1548-1560 takes tx_mux with portMAX_DELAY. No VFS, SDK logging,
  // other task or TX API uses this owned driver; uart_tx_chars then writes only
  // available hardware FIFO space (uart.c:1131), never waits for wire completion.
  if (!ownsConsole || length > tusb32x_cli::ConsoleBuffer::DRAIN_BUDGET) return 0;
  const int written = uart_tx_chars(CONSOLE, data, static_cast<uint32_t>(length));
  return written > 0 && static_cast<size_t>(written) <= length ? static_cast<size_t>(written) : 0;
}
/// @brief Sample native monotonic time afresh; no 32-bit intermediate or platform sleep.
uint64_t nowMs(void*) { return ownerNowMs(); }
/// @brief Sample only the optional configured application input, never set it up in a CLI handler.
bool readInput(bool& value, void*) {
  if (!inputConfigured) return false;
  value = gpio_get_level(static_cast<gpio_num_t>(BoardConfig::irq)) != 0;
  return true;
}
/// @brief Restrict binding/edits to this application's explicitly created synchronous device.
t::Status validateTarget(const t::DriverConfig& config, void*) {
  if (!ownsDevice || !ownsBus || config.variant != BoardConfig::variant ||
      static_cast<uint8_t>(config.address) != context.config.device_address)
    return t::Status::Error(t::Err::INVALID_CONFIG, "End and explicitly replace native handles before selecting another target");
  return validateContext(context);
}
/// @brief Borrow the one device context; resource ownership stays in the pinned application.
t::DriverConfig configuration() {
  t::DriverConfig config;
  config.variant = BoardConfig::variant;
  config.address = static_cast<t::DeviceAddress>(BoardConfig::address);
  config.i2cWrite = write;
  config.i2cWriteRead = writeRead;
  config.i2cUser = &context;
  config.i2cTimeoutMs = BoardConfig::callbackMs;
  return config;
}
/// @brief Copy example-only firmware identity and actual UART/I2C route metadata.
tusb32x_cli::Platform platform() {
  tusb32x_cli::Platform result;
  result.nowMs = nowMs;
  result.write = consoleWrite;
  if (BoardConfig::irq >= 0) result.readInput = readInput;
  result.validateTarget = validateTarget;
  result.identity.git = TUSB32X_BUILD_GIT;
  result.identity.tree = TUSB32X_BUILD_TREE;
  result.identity.build = TUSB32X_BUILD_UTC;
  result.identity.framework = "ESP-IDF";
  result.identity.frameworkVersion = esp_get_idf_version();
  result.identity.target = TUSB32X_BUILD_TARGET;
  result.bus = "I2C0";
  result.framing = "pointer-STOP-read-STOP";
  result.busHz = BoardConfig::busHz;
  result.sdaPin = BoardConfig::sda;
  result.sclPin = BoardConfig::scl;
  result.irqPin = BoardConfig::irq;
  // Native transmit/receive call totals are different units from callback kinds;
  // do not mislabel them as the optional CLI adapter callback counters.
  return result;
}
tusb32x_cli::Cli processor(primary, configuration(), platform());

/// @brief Queue one whole application record; loss never changes ownership or retries a transfer.
void appRecord(const char* format, ...) {
  char record[tusb32x_cli::ConsoleBuffer::RECORD_CAPACITY];
  va_list args;
  va_start(args, format);
  const int length = std::vsnprintf(record, sizeof(record), format, args);
  va_end(args);
  if (length > 0 && static_cast<size_t>(length) < sizeof(record))
    (void)processor.enqueueApplicationRecord(record, static_cast<size_t>(length));
}
/// @brief UART pins are validated before acquisition; no native USB/GPIO route is borrowed.
bool validConsolePins() {
  return GPIO_IS_VALID_OUTPUT_GPIO(BoardConfig::uartTx) && GPIO_IS_VALID_GPIO(BoardConfig::uartRx) &&
      BoardConfig::uartTx != BoardConfig::uartRx;
}
/// @brief Avoid I2C/input/external-enable collisions with the live UART route.
bool validDevicePins() {
  const auto consolePin = [](int pin) { return pin == BoardConfig::uartTx || pin == BoardConfig::uartRx; };
  if (!GPIO_IS_VALID_OUTPUT_GPIO(BoardConfig::sda) || !GPIO_IS_VALID_OUTPUT_GPIO(BoardConfig::scl) ||
      BoardConfig::sda == BoardConfig::scl || consolePin(BoardConfig::sda) || consolePin(BoardConfig::scl)) return false;
  const int optionalPins[] = {BoardConfig::irq, BoardConfig::enable};
  for (int pin : optionalPins) {
    if (pin >= 0 && (!GPIO_IS_VALID_GPIO(pin) || consolePin(pin) ||
        pin == BoardConfig::sda || pin == BoardConfig::scl)) return false;
  }
  return BoardConfig::irq < 0 || BoardConfig::enable < 0 || BoardConfig::irq != BoardConfig::enable;
}
/// @brief Delete only successfully acquired resources once, after all driver borrowing has ended.
void releaseDevice() {
  if (deviceReleaseAttempted) return;
  deviceReleaseAttempted = true;
  busContext.retired = true;
  if (inputConfigured) {
    const auto code = gpio_reset_pin(static_cast<gpio_num_t>(BoardConfig::irq));
    if (code == ESP_OK) inputConfigured = false;
    else appRecord("APP release=irq_failed detail=%" PRId32 "\n", static_cast<int32_t>(code));
  }
  if (ownsDevice) {
    const auto code = i2c_master_bus_rm_device(context.device);
    if (code != ESP_OK) {
      appRecord("APP release=device_failed detail=%" PRId32 " ownership=retained\n", static_cast<int32_t>(code));
      return; // A live device must keep its parent bus and static context storage.
    }
    context.device = nullptr;
    ownsDevice = false;
  }
  if (ownsBus) {
    const auto code = i2c_del_master_bus(busContext.handle);
    if (code != ESP_OK) {
      appRecord("APP release=bus_failed detail=%" PRId32 " ownership=retained\n", static_cast<int32_t>(code));
      return;
    }
    busContext.handle = nullptr;
    ownsBus = false;
  }
}
/// @brief UART delete failure retains the static ownership evidence; never retry or claim release.
bool releaseConsole() {
  if (!ownsConsole) return true;
  if (consoleReleaseAttempted) return false;
  consoleReleaseAttempted = true;
  const auto code = uart_driver_delete(CONSOLE);
  if (code == ESP_OK) {
    ownsConsole = false;
    consoleEvents = nullptr;
    return true;
  }
  appRecord("APP release=console_failed detail=%" PRId32 " ownership=retained\n", static_cast<int32_t>(code));
  return false;
}
/// @brief Report each consumed startup result, including failure, independently of output capacity.
void reportStartup() {
  if (startupReport == startupOwner.reportSequence()) return;
  startupReport = startupOwner.reportSequence();
  const auto status = startupOwner.lastStatus();
  const auto& result = startupOwner.lastResult();
  appRecord("APP owner=startup report=%" PRIu64 " code=%s detail=%" PRId32
      " result_valid=%u result_code=%s result_detail=%" PRId32 " token=%" PRIu64 " kind=%u effect=%u callbacks=%" PRIu32 "\n",
      startupReport, t::errorName(status.code), status.detail, startupOwner.lastResultValid() ? 1U : 0U,
      startupOwner.lastResultValid() ? t::errorName(result.status.code) : "UNAVAILABLE",
      startupOwner.lastResultValid() ? result.status.detail : 0, result.token.value,
      static_cast<unsigned>(result.kind), static_cast<unsigned>(result.hardwareEffect), result.transfers);
}
/// @brief Drain finite SDK events; input errors permanently stop admissions rather than splice bytes.
bool inputEvents() {
  uart_event_t event{};
  // The SDK merely logs a failed event enqueue (uart.c:1470). A full queue or
  // full drain budget could hide an error; refuse input even if all visible
  // events say DATA. Quiet polling/output and already-admitted jobs still run.
  bool lost = uxQueueMessagesWaiting(consoleEvents) >= EVENT_BUDGET;
  unsigned count = 0;
  for (; count < EVENT_BUDGET; ++count) {
    if (xQueueReceive(consoleEvents, &event, 0) != pdTRUE) break;
    if (event.type == UART_FIFO_OVF || event.type == UART_BUFFER_FULL || event.type == UART_BREAK ||
        event.type == UART_PARITY_ERR || event.type == UART_FRAME_ERR) lost = true;
  }
  if (lost || count == EVENT_BUDGET) {
    if (!inputFailed) appRecord("APP console=input_failed reason=uart_stream_loss commands=disabled\n");
    inputFailed = true;
    inputSize = inputOffset = 0;
  }
  return !inputFailed && uxQueueMessagesWaiting(consoleEvents) == 0;
}
/// @brief Retain the exact unconsumed suffix when the shared console accepts only one command.
void readConsole() {
  if (!inputEvents()) return;
  if (inputOffset == inputSize) {
    inputSize = inputOffset = 0;
    const int count = uart_read_bytes(CONSOLE, input, sizeof(input), 0);
    if (count < 0 || count > static_cast<int>(sizeof(input))) {
      inputFailed = true;
      appRecord("APP console=input_failed reason=uart_read commands=disabled\n");
      return;
    }
    inputSize = static_cast<size_t>(count);
  }
  if (!inputEvents()) return;
  inputOffset += processor.feed(input + inputOffset, inputSize - inputOffset);
}
/// @brief Stop admissions, retain CLI results, end borrowing and only then remove owned handles.
void shutdown() {
  stopping = true;
  const auto ended = startupOwner.stop(); // Cancels/takes only startup's token; ends other work locally.
  reportStartup();
  const auto progress = primary.progress();
  t::OperationResult result;
  if (progress.resultPending && primary.resultSnapshot(progress.token, result).ok())
    appRecord("APP shutdown=retained_cli_result token=%" PRIu64 " code=%s detail=%" PRId32 " effect=%u\n",
        result.token.value, t::errorName(result.status.code), result.status.detail,
        static_cast<unsigned>(result.hardwareEffect));
  if (ended.ok() && !primary.diagnostics().bound && progress.state != t::OperationState::ACTIVE)
    releaseDevice();
  appRecord("APP shutdown=ended code=%s detail=%" PRId32 " device_owned=%u bus_owned=%u\n",
      t::errorName(ended.code), ended.detail, ownsDevice ? 1U : 0U, ownsBus ? 1U : 0U);
}
} // namespace

bool setupExample() {
  if (setupAttempted) return false;
  setupAttempted = true;
  startedAt = ownerNowMs();
  // IDF main task is pinned to core0 by sdkconfig; driver interrupts are allocated here too.
  if (xPortGetCoreID() != 0 || !validConsolePins() || uart_is_driver_installed(CONSOLE)) return false;
  auto code = uart_driver_install(CONSOLE, 256, 0, EVENT_BUDGET, &consoleEvents, 0);
  if (code != ESP_OK) return false;
  ownsConsole = true;
  uart_config_t uart{};
  uart.baud_rate = 115200;
  uart.data_bits = UART_DATA_8_BITS;
  uart.parity = UART_PARITY_DISABLE;
  uart.stop_bits = UART_STOP_BITS_1;
  uart.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  uart.source_clk = UART_SCLK_DEFAULT;
  code = uart_param_config(CONSOLE, &uart);
  if (code == ESP_OK) code = uart_set_pin(CONSOLE, BoardConfig::uartTx, BoardConfig::uartRx,
      UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  if (code != ESP_OK || consoleEvents == nullptr) { releaseConsole(); return false; }
  cliReady = processor.setup().ok();
  if (!cliReady) { releaseConsole(); return false; }
  appRecord("APP setup=console framework=ESP-IDF target=%s uart=0 tx=%d rx=%d baud=115200"
      " variant=%u address=%u sda=%d scl=%d irq=%d enable_external=%d enable_active_high=%u ready=%u cleanup=%u\n",
      TUSB32X_BUILD_TARGET, BoardConfig::uartTx, BoardConfig::uartRx,
      static_cast<unsigned>(BoardConfig::variant), static_cast<unsigned>(BoardConfig::address),
      BoardConfig::sda, BoardConfig::scl, BoardConfig::irq, BoardConfig::enable,
      BoardConfig::enableActiveHigh ? 1U : 0U, BoardConfig::boardReady ? 1U : 0U,
      BoardConfig::acceptSdkCleanup ? 1U : 0U);
  if (!BoardConfig::boardReady || !BoardConfig::maxOverheadUs || !BoardConfig::acceptSdkCleanup || !validDevicePins()) {
    appRecord("APP setup=diagnostic_only reason=unqualified_board_or_timing_or_pins\n");
    return true;
  }
  busContext.ownerCore = xPortGetCoreID();
  busContext.maxOverheadUs = BoardConfig::maxOverheadUs;
  busContext.acceptSdkCleanup = BoardConfig::acceptSdkCleanup;
  busContext.config.i2c_port = I2C_NUM_0;
  busContext.config.sda_io_num = static_cast<gpio_num_t>(BoardConfig::sda);
  busContext.config.scl_io_num = static_cast<gpio_num_t>(BoardConfig::scl);
  busContext.config.clk_source = I2C_CLK_SRC_DEFAULT;
  busContext.config.glitch_ignore_cnt = 7;
  busContext.config.trans_queue_depth = 0;
  code = i2c_new_master_bus(&busContext.config, &busContext.handle);
  if (code != ESP_OK) {
    appRecord("APP setup=diagnostic_only reason=bus_create detail=%" PRId32 "\n", static_cast<int32_t>(code));
    return true;
  }
  ownsBus = true;
  context.bus = &busContext;
  context.config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  context.config.device_address = BoardConfig::address;
  context.config.scl_speed_hz = BoardConfig::busHz;
  code = i2c_master_bus_add_device(busContext.handle, &context.config, &context.device);
  if (code != ESP_OK) {
    appRecord("APP setup=diagnostic_only reason=device_create detail=%" PRId32 "\n", static_cast<int32_t>(code));
    releaseDevice();
    return true;
  }
  ownsDevice = true;
  auto status = validateContext(context);
  if constexpr (BoardConfig::irq >= 0) {
    if (status.ok()) {
      gpio_config_t pin{};
      pin.pin_bit_mask = UINT64_C(1) << BoardConfig::irq;
      pin.mode = GPIO_MODE_INPUT;
      pin.pull_up_en = GPIO_PULLUP_DISABLE;
      pin.pull_down_en = GPIO_PULLDOWN_DISABLE;
      pin.intr_type = GPIO_INTR_DISABLE;
      code = gpio_config(&pin);
      inputConfigured = code == ESP_OK;
      if (!inputConfigured) status = t::Status::Error(t::Err::INVALID_CONFIG, "IRQ input configuration failed", code);
    }
  }
  if (status.ok()) status = primary.bind(configuration());
  if (!status.ok()) {
    appRecord("APP setup=diagnostic_only code=%s detail=%" PRId32 "\n", t::errorName(status.code), status.detail);
    releaseDevice();
    return true;
  }
  startupStarted = true;
  (void)startupOwner.start(ownerNowMs());
  reportStartup();
  return true;
}

void requestStop() { stopRequested.store(true, std::memory_order_release); }

bool loopExample() {
  if (!cliReady || !ownsConsole) return false;
  if (!stopping && (stopRequested.load(std::memory_order_acquire) ||
      (BoardConfig::runMs && ownerNowMs() - startedAt >= BoardConfig::runMs))) shutdown();
  if (stopping) {
    processor.tick(); // end has cancelled work; no managed callback can follow teardown.
    // FIFO acceptance is not wire completion. IDF uart_wait_tx_done(...,0) has
    // zero semaphore waits (and at most its 2us TX_DONE setup guard).
    const bool drained = processor.console().queued() == 0 && uart_wait_tx_done(CONSOLE, 0) == ESP_OK;
    if (++shutdownTurns >= SHUTDOWN_TURNS || drained) {
      if (!drained) shutdownOutputAbandoned = true; // Preserve loss evidence even without a terminal.
      if (consoleReleaseAttempted || releaseConsole()) return false;
      shutdownTurns = 0; // One finite additional drain for the failed-delete record, no retry.
    }
    return true;
  }
  if (startupStarted && !startupOwner.startupDone()) {
    startupOwner.tick(ownerNowMs(), 1);
    reportStartup();
    // No CLI input was admitted. Its tick drains only, never polls startup's token.
  } else readConsole();
  processor.tick();
  return true;
}
} // namespace tusb32x_idf
