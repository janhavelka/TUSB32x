/// @file test_app.cpp
/// @brief Execute the actual native application, adapter and CLI with recording SDK calls.
/// @details One scenario per process preserves real static storage lifetimes. Including the
/// application permits checking exact token retention and ownership at resource deletion;
/// all application admission, input, startup, polling and cleanup use production functions.
#include "../../examples/esp_idf/ExampleApp.cpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iterator>
#include <string>
#include <vector>

/// @brief Opaque SDK identities are concrete only in this recording host fixture.
struct i2c_master_bus_t { unsigned identity = 1; };
/// @brief Exactly one owned device belongs to the owned controller.
struct i2c_master_dev_t { unsigned identity = 2; };
/// @brief Event-queue identity outlives every application callback.
struct RecordingQueue { unsigned identity = 3; };

namespace {
unsigned checks = 0;
unsigned failures = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    ++failures;
    std::printf("FAIL native app %d: %s\n", line, expression);
  }
}
#define CHECK(value) check((value), #value, __LINE__)

/// @brief Finite terminal SDK outcomes and independent clock movement, with no physical bus.
struct Backend {
  uint64_t microseconds = 100000;
  bool busAlive = false;
  bool externalBus = false;
  bool deviceAlive = false;
  bool uartAlive = false;
  bool stalled = false;
  bool createBusFails = false;
  bool createDeviceFails = false;
  bool removeDeviceFails = false;
  bool deleteBusFails = false;
  bool deleteUartFails = false;
  bool uartConfigFails = false;
  bool uartPinFails = false;
  bool uartInstallFails = false;
  bool identityFails = false;
  bool gpioConfigFails = false;
  bool overflowAfterRead = false;
  esp_err_t txError = ESP_OK;
  esp_err_t rxError = ESP_OK;
  unsigned newBusCalls = 0;
  unsigned addDeviceCalls = 0;
  unsigned removeDeviceCalls = 0;
  unsigned deleteBusCalls = 0;
  unsigned uartConfigCalls = 0;
  unsigned uartPinCalls = 0;
  unsigned uartInstallCalls = 0;
  unsigned uartDeleteCalls = 0;
  unsigned uartReads = 0;
  unsigned uartWrites = 0;
  unsigned uartWaits = 0;
  unsigned txPendingPolls = 0;
  unsigned transmits = 0;
  unsigned receives = 0;
  unsigned writes = 0;
  unsigned gpioCalls = 0;
  unsigned gpioReads = 0;
  uint8_t pointer = 0;
  std::size_t txCapacity = 256;
  std::string input;
  std::string output;
  std::deque<uart_event_t> events;
  std::vector<char> cleanup;
  i2c_master_bus_config_t busConfig{};
  i2c_device_config_t deviceConfig{};
} sdk;
i2c_master_bus_t nativeBus;
i2c_master_dev_t nativeDevice;
RecordingQueue uartQueue;

/// @brief Assert the real owner never performs two core callbacks in one finite turn.
bool turn() {
  const unsigned beforeTx = sdk.transmits;
  const unsigned beforeReads = sdk.uartReads;
  const unsigned beforeWrites = sdk.uartWrites;
  const unsigned beforeWaits = sdk.uartWaits;
  const bool running = tusb32x_idf::loopExample();
  CHECK(sdk.transmits - beforeTx <= 1);
  CHECK(sdk.uartReads - beforeReads <= 1);
  CHECK(sdk.uartWrites - beforeWrites <= 1);
  CHECK(sdk.uartWaits - beforeWaits <= 1);
  return running;
}

/// @brief Drain finite CLI output through the actual owner loop.
void drain() { for (unsigned i = 0; i < 96; ++i) (void)turn(); }

/// @brief Submit one fresh real UART command and preserve all unconsumed bytes.
std::string command(const std::string& input) {
  const auto offset = sdk.output.size();
  CHECK(sdk.input.empty());
  sdk.input = input + "\n";
  drain();
  CHECK(sdk.input.empty());
  return sdk.output.substr(offset);
}

/// @brief Stop once and prove finite completion even when the output sink stays full.
void stop(bool request = true) {
  if (request) tusb32x_idf::requestStop();
  const auto beforeTx = sdk.transmits;
  bool running = true;
  unsigned turns = 0;
  while (running && turns < 150) { running = turn(); ++turns; }
  CHECK(!running && turns <= 134);
  CHECK(!tusb32x_idf::primary.diagnostics().bound);
  CHECK(sdk.txPendingPolls == 0 || tusb32x_idf::shutdownOutputAbandoned);
  CHECK(sdk.transmits == beforeTx);
  const auto cleanups = sdk.cleanup.size();
  for (unsigned i = 0; i < 4; ++i) CHECK(!turn());
  CHECK(sdk.cleanup.size() == cleanups); // No teardown retry after terminal failure.
}
} // namespace

bool uart_is_driver_installed(uart_port_t port) {
  CHECK(port == UART_NUM_0);
  return sdk.uartAlive;
}
esp_err_t uart_param_config(uart_port_t port, const uart_config_t* config) {
  ++sdk.uartConfigCalls;
  CHECK(port == UART_NUM_0 && config != nullptr);
  CHECK(config->baud_rate == 115200 && config->data_bits == UART_DATA_8_BITS);
  CHECK(config->flow_ctrl == UART_HW_FLOWCTRL_DISABLE);
  return sdk.uartConfigFails ? ESP_ERR_INVALID_ARG : ESP_OK;
}
esp_err_t uart_set_pin(uart_port_t port, int tx, int rx, int rts, int cts) {
  ++sdk.uartPinCalls;
  CHECK(port == UART_NUM_0 && tx == TUSB32X_UART_TX && rx == TUSB32X_UART_RX);
  CHECK(rts == UART_PIN_NO_CHANGE && cts == UART_PIN_NO_CHANGE);
  return sdk.uartPinFails ? ESP_ERR_INVALID_ARG : ESP_OK;
}
esp_err_t uart_driver_install(uart_port_t port, int rxBytes, int txBytes,
    int queueSize, QueueHandle_t* queue, int flags) {
  ++sdk.uartInstallCalls;
  CHECK(port == UART_NUM_0 && !sdk.uartAlive);
  CHECK(rxBytes == 256 && txBytes == 0 && queueSize == 16 && queue != nullptr && flags == 0);
  if (sdk.uartInstallFails) return ESP_ERR_NO_MEM;
  sdk.uartAlive = true;
  *queue = &uartQueue;
  return ESP_OK;
}
esp_err_t uart_driver_delete(uart_port_t port) {
  ++sdk.uartDeleteCalls;
  CHECK(port == UART_NUM_0 && sdk.uartAlive);
  CHECK(!tusb32x_idf::primary.diagnostics().bound);
  sdk.cleanup.push_back('u');
  if (sdk.deleteUartFails) return ESP_FAIL;
  sdk.uartAlive = false;
  return ESP_OK;
}
int uart_read_bytes(uart_port_t port, void* destination, uint32_t length, TickType_t ticks) {
  ++sdk.uartReads;
  CHECK(port == UART_NUM_0 && sdk.uartAlive && destination != nullptr);
  CHECK(ticks == 0 && length > 0 && length <= 64);
  const auto count = std::min<std::size_t>(length, sdk.input.size());
  std::memcpy(destination, sdk.input.data(), count);
  sdk.input.erase(0, count);
  if (sdk.overflowAfterRead) {
    sdk.events.push_back({UART_FIFO_OVF, 0, false});
    sdk.overflowAfterRead = false;
  }
  return static_cast<int>(count);
}
int uart_tx_chars(uart_port_t port, const char* data, uint32_t length) {
  ++sdk.uartWrites;
  CHECK(port == UART_NUM_0 && sdk.uartAlive && data != nullptr && length <= 256);
  if (sdk.stalled) return 0;
  const auto count = std::min<std::size_t>(length, sdk.txCapacity);
  sdk.output.append(data, count);
  return static_cast<int>(count);
}
esp_err_t uart_wait_tx_done(uart_port_t port, TickType_t ticks) {
  ++sdk.uartWaits;
  CHECK(port == UART_NUM_0 && sdk.uartAlive && ticks == 0);
  CHECK(!tusb32x_idf::primary.diagnostics().bound);
  if (sdk.txPendingPolls) { --sdk.txPendingPolls; return ESP_ERR_TIMEOUT; }
  return ESP_OK;
}
BaseType_t xQueueReceive(QueueHandle_t queue, void* destination, TickType_t ticks) {
  CHECK(queue == &uartQueue && destination != nullptr && ticks == 0);
  if (sdk.events.empty()) return 0;
  *static_cast<uart_event_t*>(destination) = sdk.events.front();
  sdk.events.pop_front();
  return pdTRUE;
}
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t queue) {
  CHECK(queue == &uartQueue);
  return static_cast<UBaseType_t>(sdk.events.size());
}
esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t* config,
    i2c_master_bus_handle_t* bus) {
  ++sdk.newBusCalls;
  CHECK(config != nullptr && bus != nullptr && (!sdk.busAlive || sdk.externalBus));
  CHECK(config->i2c_port == I2C_NUM_0 && config->sda_io_num == TUSB32X_SDA);
  CHECK(config->scl_io_num == TUSB32X_SCL && config->trans_queue_depth == 0);
  sdk.busConfig = *config;
  if (sdk.createBusFails) return sdk.externalBus ? ESP_ERR_INVALID_STATE : ESP_ERR_NO_MEM;
  sdk.busAlive = true;
  *bus = &nativeBus;
  return ESP_OK;
}
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus,
    const i2c_device_config_t* config, i2c_master_dev_handle_t* device) {
  ++sdk.addDeviceCalls;
  CHECK(bus == &nativeBus && sdk.busAlive && config != nullptr && device != nullptr);
  CHECK(config->device_address == TUSB32X_ADDRESS && config->dev_addr_length == I2C_ADDR_BIT_LEN_7);
  CHECK(config->scl_speed_hz == TUSB32X_BUS_HZ && !config->flags.disable_ack_check);
  sdk.deviceConfig = *config;
  if (sdk.createDeviceFails) return ESP_ERR_NO_MEM;
  sdk.deviceAlive = true;
  *device = &nativeDevice;
  return ESP_OK;
}
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t device) {
  ++sdk.removeDeviceCalls;
  CHECK(device == &nativeDevice && sdk.deviceAlive && sdk.busAlive);
  CHECK(!tusb32x_idf::primary.diagnostics().bound);
  sdk.cleanup.push_back('d');
  if (sdk.removeDeviceFails) return ESP_FAIL;
  sdk.deviceAlive = false;
  return ESP_OK;
}
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus) {
  ++sdk.deleteBusCalls;
  CHECK(bus == &nativeBus && sdk.busAlive && !sdk.deviceAlive);
  CHECK(!tusb32x_idf::primary.diagnostics().bound);
  sdk.cleanup.push_back('b');
  if (sdk.deleteBusFails) return ESP_FAIL;
  sdk.busAlive = false;
  return ESP_OK;
}
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t device, const uint8_t* data,
    size_t length, int timeoutMs) {
  ++sdk.transmits;
  CHECK(device == &nativeDevice && sdk.deviceAlive && sdk.busAlive);
  CHECK(data != nullptr && (length == 1 || length == 2) && timeoutMs > 0);
  CHECK(tusb32x_idf::busContext.gate.test_and_set());
  sdk.pointer = data[0];
  if (length == 2) ++sdk.writes;
  sdk.microseconds += 50;
  return sdk.txError;
}
esp_err_t i2c_master_receive(i2c_master_dev_handle_t device, uint8_t* data,
    size_t length, int timeoutMs) {
  ++sdk.receives;
  CHECK(device == &nativeDevice && sdk.deviceAlive && sdk.busAlive);
  CHECK(data != nullptr && length > 0 && length <= 8 && timeoutMs > 0);
  CHECK(tusb32x_idf::busContext.gate.test_and_set());
  constexpr uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
  for (size_t i = 0; i < length; ++i)
    data[i] = sdk.pointer == 0 ? identity[i] : sdk.pointer == 0xA0 ? 2 :
              sdk.pointer == 9 ? 0x94 : 0;
  sdk.microseconds += 50;
  if (sdk.identityFails && sdk.pointer == 0 && length == 8) return ESP_ERR_TIMEOUT;
  return sdk.rxError;
}
esp_err_t gpio_config(const gpio_config_t* config) {
  ++sdk.gpioCalls;
  CHECK(config && config->mode == GPIO_MODE_INPUT && config->intr_type == GPIO_INTR_DISABLE);
  CHECK(tusb32x_idf::BoardConfig::irq >= 0);
  CHECK(config->pin_bit_mask == (UINT64_C(1) <<
      (static_cast<unsigned>(tusb32x_idf::BoardConfig::irq) & 63U)));
  return sdk.gpioConfigFails ? ESP_ERR_INVALID_ARG : ESP_OK;
}
int gpio_get_level(gpio_num_t pin) { ++sdk.gpioReads; CHECK(pin >= 0); return 1; }
esp_err_t gpio_reset_pin(gpio_num_t pin) { ++sdk.gpioCalls; CHECK(pin >= 0); return ESP_OK; }
int xPortGetCoreID() { return 0; }
bool xPortInIsrContext() { return false; }
int64_t esp_timer_get_time() { return static_cast<int64_t>(sdk.microseconds); }
const char* esp_get_idf_version() { return "5.5.5"; }
void esp_rom_delay_us(uint32_t duration) { sdk.microseconds += duration; }
void vTaskDelay(TickType_t ticks) {
  CHECK(ticks > 0);
  sdk.microseconds += static_cast<uint64_t>(ticks) * 1000000 / configTICK_RATE_HZ;
}

/// @brief Execute a selected independent static-lifetime scenario; never qualify hardware.
int main(int argc, char** argv) {
  CHECK(argc == 2);
  const std::string scenario = argc == 2 ? argv[1] : "invalid";
  constexpr const char* scenarios[] = {"startup", "startup-failure", "shutdown-active",
      "shutdown-startup", "backpressure", "bus-failure", "device-failure",
      "uart-config-failure", "uart-pin-failure", "uart-install-failure",
      "console-already-owned", "device-remove-failure", "bus-delete-failure",
      "uart-delete-failure", "rx-overflow", "rx-overflow-after-read", "rx-event-flood",
      "device-cleanup-failure", "existing-bus", "teardown-fifo-pending", "teardown-fifo-stalled",
      "irq-configured", "irq-config-failure", "irq-cleanup-failure", "run-timeout",
      "raw-error", "diagnostic", "pin-conflict"};
  CHECK(std::any_of(std::begin(scenarios), std::end(scenarios),
                   [&](const char* name) { return scenario == name; }));
  sdk.externalBus = scenario == "existing-bus";
  sdk.busAlive = sdk.externalBus;
  sdk.createBusFails = scenario == "bus-failure" || sdk.externalBus;
  sdk.createDeviceFails = scenario == "device-failure" || scenario == "device-cleanup-failure";
  sdk.removeDeviceFails = scenario == "device-remove-failure" || scenario == "irq-cleanup-failure";
  sdk.deleteBusFails = scenario == "bus-delete-failure" || scenario == "device-cleanup-failure";
  sdk.deleteUartFails = scenario == "uart-delete-failure";
  sdk.uartConfigFails = scenario == "uart-config-failure";
  sdk.uartPinFails = scenario == "uart-pin-failure";
  sdk.uartInstallFails = scenario == "uart-install-failure";
  sdk.uartAlive = scenario == "console-already-owned";
  sdk.identityFails = scenario == "startup-failure";
  sdk.gpioConfigFails = scenario == "irq-config-failure" || scenario == "irq-cleanup-failure";
  const bool consoleFailure = sdk.uartConfigFails || sdk.uartPinFails || sdk.uartInstallFails || sdk.uartAlive;
  const bool ready = tusb32x_idf::setupExample();
  CHECK(ready == !consoleFailure);
  CHECK(sdk.transmits == 0 && sdk.receives == 0); // Admission and bind are bus-silent.
  const auto installs = sdk.uartInstallCalls;
  const auto creates = sdk.newBusCalls;
  CHECK(!tusb32x_idf::setupExample());
  CHECK(sdk.uartInstallCalls == installs && sdk.newBusCalls == creates);
  if (consoleFailure) {
    CHECK(!sdk.busAlive && sdk.newBusCalls == 0 && sdk.transmits == 0);
    CHECK(!tusb32x_idf::primary.diagnostics().bound);
    stop();
    CHECK(sdk.uartDeleteCalls == ((sdk.uartConfigFails || sdk.uartPinFails) ? 1U : 0U));
  } else {
    if (scenario == "shutdown-startup") {
      CHECK(tusb32x_idf::primary.progress().state == TUSB32x::OperationState::ACTIVE);
      stop();
      CHECK(tusb32x_idf::startupOwner.lastResultValid());
      CHECK(tusb32x_idf::startupOwner.lastResult().status.is(TUSB32x::Err::CANCELLED));
      CHECK(!tusb32x_idf::primary.progress().resultPending);
    } else {
      sdk.stalled = scenario == "backpressure";
      if (scenario == "startup") {
        sdk.input = "version\n";
        CHECK(tusb32x_idf::primary.progress().state == TUSB32x::OperationState::ACTIVE);
        (void)turn();
        CHECK(sdk.input == "version\n"); // Startup owns admission until it takes its result.
      }
      for (unsigned i = 0; i < 64; ++i) (void)turn();
      const bool diagnostic = scenario == "diagnostic" || scenario == "pin-conflict" ||
                              sdk.createBusFails || sdk.createDeviceFails || sdk.gpioConfigFails;
      if (diagnostic) {
        CHECK(!tusb32x_idf::primary.diagnostics().bound);
        CHECK(sdk.transmits == 0 && sdk.receives == 0);
        if (scenario == "diagnostic" || scenario == "pin-conflict") CHECK(sdk.newBusCalls == 0);
      } else {
        CHECK(tusb32x_idf::startupOwner.startupDone());
        CHECK(tusb32x_idf::startupOwner.reportSequence() == (sdk.identityFails ? 1 : 2));
        CHECK(tusb32x_idf::primary.progress().state == TUSB32x::OperationState::IDLE);
        CHECK(!tusb32x_idf::primary.progress().resultPending);
        CHECK(sdk.busConfig.sda_io_num == tusb32x_idf::busContext.config.sda_io_num);
        CHECK(sdk.deviceConfig.device_address == tusb32x_idf::context.config.device_address);
        if (sdk.identityFails) {
          CHECK(tusb32x_idf::startupOwner.lastResultValid());
          CHECK(tusb32x_idf::startupOwner.lastResult().status.is(TUSB32x::Err::I2C_TIMEOUT));
          CHECK(tusb32x_idf::startupOwner.lastResult().status.detail == ESP_ERR_TIMEOUT);
          CHECK(tusb32x_idf::busContext.retired);
        }
      }
      CHECK(sdk.writes == 0); // Startup never applies, resets, recovers or acknowledges IRQ.
      CHECK(tusb32x_idf::callbackCounters(nullptr).writeReads ==
            tusb32x_idf::primary.transportStats().totalAttempts); // Include failed callback attempts.
      sdk.stalled = false;
      drain();
      CHECK(command("version").find("TUSB VERSION") != std::string::npos);
      CHECK(sdk.output.find("framework=ESP-IDF") != std::string::npos);
      CHECK(command("diag").find("TUSB DIAG") != std::string::npos);
      if (diagnostic) {
        CHECK(command("bind").find(scenario == "irq-cleanup-failure" ?
              "code=I2C_BUS" : "code=INVALID_CONFIG") != std::string::npos);
        CHECK(!tusb32x_idf::primary.diagnostics().bound && sdk.transmits == 0);
      }
      if (scenario == "startup") {
        const auto counters = tusb32x_idf::platform().counters(nullptr);
        CHECK(counters.available && counters.writes == 0 && counters.writeReads > 0);
        CHECK(counters.writeReads == tusb32x_idf::primary.transportStats().totalAttempts);
        CHECK(command("bus").find("adapter_available=1 adapter_writes=0 adapter_write_reads=" +
              std::to_string(counters.writeReads)) != std::string::npos);
        const auto transmits = sdk.transmits;
        CHECK(command("reg 8").find("code=OK") != std::string::npos);
        CHECK(sdk.transmits == transmits + 1);
        CHECK(tusb32x_idf::callbackCounters(nullptr).writes == 0);
        CHECK(tusb32x_idf::callbackCounters(nullptr).writeReads == counters.writeReads + 1);
        (void)command("end");
        (void)command("bind");
        CHECK(tusb32x_idf::callbackCounters(nullptr).writeReads == counters.writeReads + 1);
        const uint8_t bytes[] = {8, 0};
        CHECK(tusb32x_idf::primaryWrite(tusb32x_idf::BoardConfig::address, bytes,
              sizeof(bytes), tusb32x_idf::BoardConfig::callbackMs, &tusb32x_idf::context).ok());
        CHECK(tusb32x_idf::callbackCounters(nullptr).writes == 1 && sdk.writes == 1);
        const auto offset = sdk.output.size();
        sdk.txCapacity = 7; // Partial FIFO acceptance must preserve exact output suffixes.
        sdk.input = "version\nbus\nversion\n";
        drain();
        CHECK(sdk.input.empty());
        sdk.txCapacity = 256;
        drain();
        const auto output = sdk.output.substr(offset);
        CHECK(output.find("TUSB BUS") != std::string::npos);
        const auto first = output.find("TUSB VERSION");
        CHECK(first != std::string::npos && output.find("TUSB VERSION", first + 1) != std::string::npos);
      }
      const auto gpioBefore = sdk.gpioReads;
      CHECK(command("intpin").find("TUSB PIN") != std::string::npos);
      CHECK(sdk.gpioReads == gpioBefore + (tusb32x_idf::inputConfigured ? 1U : 0U));
      if (scenario == "rx-overflow" || scenario == "rx-overflow-after-read" || scenario == "rx-event-flood") {
        sdk.input = "wreg 8 ";
        (void)turn();
        if (scenario == "rx-overflow-after-read") sdk.overflowAfterRead = true;
        else if (scenario == "rx-event-flood") {
          for (unsigned i = 0; i < 16; ++i) sdk.events.push_back({UART_DATA, 1, false});
        } else sdk.events.push_back({UART_FIFO_OVF, 0, false});
        sdk.input = "0\nversion\n";
        const auto reads = sdk.uartReads;
        drain();
        CHECK(sdk.writes == 0);
        CHECK(sdk.uartReads == reads + (scenario == "rx-overflow-after-read" ? 1U : 0U));
        CHECK(sdk.output.find("console=input_failed") != std::string::npos);
      } else if (!diagnostic && !sdk.identityFails) {
        const auto before = sdk.transmits;
        if (scenario == "shutdown-active") {
          sdk.input = "read\n";
          (void)turn();
          CHECK(tusb32x_idf::primary.progress().state == TUSB32x::OperationState::ACTIVE);
        } else if (scenario == "raw-error") {
          sdk.txError = ESP_ERR_TIMEOUT;
          const auto counts = tusb32x_idf::callbackCounters(nullptr);
          const auto output = command("reg 8");
          CHECK(output.find("code=I2C_TIMEOUT") != std::string::npos);
          CHECK(output.find("detail=263") != std::string::npos);
          CHECK(sdk.transmits == before + 1);
          CHECK(tusb32x_idf::callbackCounters(nullptr).writeReads == counts.writeReads + 1);
          CHECK(tusb32x_idf::callbackCounters(nullptr).writes == counts.writes);
        } else {
          (void)command("read");
          const auto progress = tusb32x_idf::primary.progress();
          CHECK(progress.resultPending && progress.token.valid());
          CHECK(tusb32x_idf::startupOwner.reportSequence() == 2);
          if (scenario == "startup") {
            CHECK(command("read").find("code=RESULT_PENDING") != std::string::npos);
            (void)command("result take " + std::to_string(progress.token.value));
            CHECK(!tusb32x_idf::primary.progress().resultPending);
            (void)command("read");
            CHECK(tusb32x_idf::primary.progress().resultPending);
          }
        }
      }
      const auto progress = tusb32x_idf::primary.progress();
      sdk.stalled = scenario == "backpressure";
      if (scenario == "teardown-fifo-pending") sdk.txPendingPolls = 6;
      if (scenario == "teardown-fifo-stalled") sdk.txPendingPolls = 1000;
      if (scenario == "run-timeout") {
        CHECK(tusb32x_idf::BoardConfig::runMs > 0);
        sdk.microseconds += tusb32x_idf::BoardConfig::runMs * 1000;
      }
      stop(scenario != "run-timeout");
      if (scenario == "teardown-fifo-pending") CHECK(sdk.uartWaits >= 7 && sdk.txPendingPolls == 0);
      if (scenario == "teardown-fifo-stalled") CHECK(sdk.uartWaits > 0 && sdk.txPendingPolls > 0);
      if (scenario == "teardown-fifo-stalled" || scenario == "backpressure")
        CHECK(tusb32x_idf::shutdownOutputAbandoned);
      if (scenario == "teardown-fifo-pending") CHECK(!tusb32x_idf::shutdownOutputAbandoned);
      if (progress.token.valid()) {
        TUSB32x::OperationResult result;
        CHECK(tusb32x_idf::primary.resultSnapshot(progress.token, result).ok());
        if (scenario == "shutdown-active") CHECK(result.status.is(TUSB32x::Err::CANCELLED));
        if (scenario != "backpressure")
          CHECK(sdk.output.find("shutdown=retained_cli_result") != std::string::npos);
      }
    }
    CHECK(sdk.removeDeviceCalls <= 1 && sdk.deleteBusCalls <= 1 && sdk.uartDeleteCalls == 1);
    if (sdk.removeDeviceFails) {
      CHECK(sdk.deviceAlive && sdk.busAlive && sdk.deleteBusCalls == 0);
      CHECK(tusb32x_idf::ownsDevice && tusb32x_idf::ownsBus);
    } else if (sdk.deleteBusFails) {
      CHECK(!sdk.deviceAlive && sdk.busAlive && tusb32x_idf::ownsBus);
    } else if (sdk.externalBus) {
      CHECK(sdk.busAlive && !tusb32x_idf::ownsBus && sdk.deleteBusCalls == 0);
      CHECK(!sdk.deviceAlive && sdk.removeDeviceCalls == 0);
    } else CHECK(!sdk.deviceAlive && !sdk.busAlive);
    CHECK(tusb32x_idf::ownsConsole == sdk.deleteUartFails);
    CHECK(sdk.uartAlive == sdk.deleteUartFails);
    if (sdk.removeDeviceCalls && !sdk.removeDeviceFails) {
      CHECK(sdk.cleanup.size() >= 3 && sdk.cleanup[0] == 'd' && sdk.cleanup[1] == 'b');
    }
    CHECK(!sdk.cleanup.empty() && sdk.cleanup.back() == 'u');
  }
  if (scenario == "startup") {
    const auto transmits = sdk.transmits;
    const uint8_t bytes[] = {8, 0};
    uint8_t received = 0xA5;
    tusb32x_idf::primaryCounters.writes = UINT32_MAX - 1;
    tusb32x_idf::primaryCounters.writeReads = UINT32_MAX - 1;
    for (unsigned i = 0; i < 2; ++i) {
      CHECK(tusb32x_idf::primaryWrite(0x47, bytes, 2, 10, nullptr).is(TUSB32x::Err::I2C_ERROR));
      CHECK(tusb32x_idf::primaryWriteRead(0x47, bytes, 1, &received, 1, 10, nullptr)
            .is(TUSB32x::Err::I2C_ERROR));
    }
    CHECK(tusb32x_idf::callbackCounters(nullptr).writes == UINT32_MAX);
    CHECK(tusb32x_idf::callbackCounters(nullptr).writeReads == UINT32_MAX);
    CHECK(received == 0xA5 && sdk.transmits == transmits);
  }
  std::printf("Native app %s: %u checks, %u failures\n", scenario.c_str(), checks, failures);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
