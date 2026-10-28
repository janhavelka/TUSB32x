/// @file main.cpp
/// @brief Native pinned main-task entry; all commands use the shared CLI and core engine.
#include "ExampleApp.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sdkconfig.h>

#if !CONFIG_ESP_MAIN_TASK_AFFINITY_CPU0 || !CONFIG_ESP_CONSOLE_NONE || CONFIG_ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG
#error "Native example requires a pinned core0 owner and exclusive application UART0"
#endif

/// @brief Own application setup, finite turns and shutdown on IDF's configured pinned main task.
/// @note No bus callbacks run before board readiness or after teardown. The SDK deletes this
/// task when it returns; static contexts survive failed cleanup without dangling borrowed storage.
extern "C" void app_main() {
  if (!tusb32x_idf::setupExample()) return;
  while (tusb32x_idf::loopExample()) vTaskDelay(1);
}
