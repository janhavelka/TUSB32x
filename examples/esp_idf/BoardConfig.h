/// @file BoardConfig.h
/// @brief Explicit single-target native example wiring and qualification; no automatic power policy.
#pragma once
#include <TUSB32x/Config.h>

#ifndef TUSB32X_VARIANT
/// @brief Exact strapped part, not automatically detected from the shared signature.
#define TUSB32X_VARIANT TUSB320LAI
#endif
#ifndef TUSB32X_ADDRESS
/// @brief Seven-bit ADDR-low address for the default LAI.
#define TUSB32X_ADDRESS 0x47
#endif
#ifndef TUSB32X_SDA
/// @brief Example SDA GPIO, subject to board qualification.
#define TUSB32X_SDA 4
#endif
#ifndef TUSB32X_SCL
/// @brief Example SCL GPIO, subject to board qualification.
#define TUSB32X_SCL 5
#endif
#ifndef TUSB32X_IRQ
/// @brief Optional input only; -1 means absent, no automatic ACK or service.
#define TUSB32X_IRQ -1
#endif
#ifndef TUSB32X_ENABLE
/// @brief Report-only externally controlled enable pin; -1 means unspecified.
#define TUSB32X_ENABLE -1
#endif
#ifndef TUSB32X_UART_TX
/// @brief UART0 TX on both example targets; connect to a 3.3V receiver.
#define TUSB32X_UART_TX 43
#endif
#ifndef TUSB32X_UART_RX
/// @brief UART0 RX on both example targets; connect to a 3.3V transmitter.
#define TUSB32X_UART_RX 44
#endif
#ifndef TUSB32X_BUS_HZ
/// @brief Audited native adapter SCL range is 100000..400000Hz.
#define TUSB32X_BUS_HZ 100000
#endif
#ifndef TUSB32X_CALLBACK_MS
/// @brief Whole callback milliseconds, including SDK cleanup and qualified overhead.
#define TUSB32X_CALLBACK_MS 600
#endif
#ifndef TUSB32X_OPERATION_MS
/// @brief Immutable application startup-operation deadline interval, in milliseconds.
#define TUSB32X_OPERATION_MS 10000
#endif
#ifndef TUSB32X_BOARD_READY
/// @brief Default zero leaves I2C disabled until the board prerequisites are explicitly accepted.
#define TUSB32X_BOARD_READY 0
#endif
#ifndef TUSB32X_MAX_OVERHEAD_US
/// @brief Qualified whole-callback CPU/preemption microseconds; zero rejects I2C setup.
#define TUSB32X_MAX_OVERHEAD_US 0
#endif
#ifndef TUSB32X_ACCEPT_SDK_CLEANUP
/// @brief Explicit acceptance of native SDK controller reset/bus-clear effects on failure.
#define TUSB32X_ACCEPT_SDK_CLEANUP 0
#endif
#ifndef TUSB32X_RUN_MS
/// @brief Zero runs until requestStop; positive milliseconds request finite application lifetime.
#define TUSB32X_RUN_MS 0
#endif

namespace tusb32x_idf {
static_assert(TUSB32X_ADDRESS >= 0 && TUSB32X_ADDRESS <= 0x7F, "Address must fit seven bits");
static_assert(TUSB32X_BUS_HZ >= 100000 && TUSB32X_BUS_HZ <= 400000, "Audited SCL range is 100..400kHz");
static_assert(TUSB32X_CALLBACK_MS > 0 && TUSB32X_CALLBACK_MS <= UINT32_MAX, "Callback milliseconds must fit uint32");
static_assert(TUSB32X_OPERATION_MS > 0 && TUSB32X_OPERATION_MS <= UINT32_MAX, "Operation milliseconds must fit uint32");
static_assert(TUSB32X_MAX_OVERHEAD_US >= 0 && TUSB32X_MAX_OVERHEAD_US <= UINT32_MAX, "Overhead microseconds must fit uint32");
static_assert(TUSB32X_RUN_MS >= 0 && TUSB32X_RUN_MS < (UINT64_C(1) << 63), "Run interval must be below half range");
static_assert(TUSB32X_SDA >= 0 && TUSB32X_SDA <= INT32_MAX &&
    TUSB32X_SCL >= 0 && TUSB32X_SCL <= INT32_MAX, "I2C pins must fit signed GPIO numbers");
static_assert(TUSB32X_UART_TX >= 0 && TUSB32X_UART_TX <= INT32_MAX &&
    TUSB32X_UART_RX >= 0 && TUSB32X_UART_RX <= INT32_MAX, "UART pins must fit signed GPIO numbers");
static_assert(TUSB32X_IRQ >= -1 && TUSB32X_IRQ <= INT32_MAX &&
    TUSB32X_ENABLE >= -1 && TUSB32X_ENABLE <= INT32_MAX, "Absent optional pins use -1");
/// @brief Compile-time choices; pin numbers and positive overhead are not board qualification.
struct BoardConfig {
  static constexpr TUSB32x::Variant variant = TUSB32x::Variant::TUSB32X_VARIANT; ///< Exact configured model.
  static constexpr uint8_t address = TUSB32X_ADDRESS; ///< Strapped seven-bit target.
  static constexpr int sda = TUSB32X_SDA; ///< Owned SDA GPIO.
  static constexpr int scl = TUSB32X_SCL; ///< Owned SCL GPIO.
  static constexpr int irq = TUSB32X_IRQ; ///< Optional owned input, absent at -1.
  static constexpr int enable = TUSB32X_ENABLE; ///< External enable, never configured or driven here.
  static constexpr int uartTx = TUSB32X_UART_TX; ///< Owned UART0 TX GPIO.
  static constexpr int uartRx = TUSB32X_UART_RX; ///< Owned UART0 RX GPIO.
  static constexpr uint32_t busHz = TUSB32X_BUS_HZ; ///< Native SCL hertz.
  static constexpr uint32_t callbackMs = TUSB32X_CALLBACK_MS; ///< Aggregate terminal callback cap.
  static constexpr uint32_t operationMs = TUSB32X_OPERATION_MS; ///< Startup interval, milliseconds.
  static constexpr uint32_t maxOverheadUs = TUSB32X_MAX_OVERHEAD_US; ///< Qualified CPU/preemption bound.
  static constexpr bool boardReady = TUSB32X_BOARD_READY != 0; ///< Explicit wiring/supply readiness.
  static constexpr bool acceptSdkCleanup = TUSB32X_ACCEPT_SDK_CLEANUP != 0; ///< SDK cleanup acceptance.
  static constexpr uint64_t runMs = TUSB32X_RUN_MS; ///< Finite lifetime or zero for explicit stop.
  static constexpr bool enableActiveHigh = variant == TUSB32x::Variant::TUSB320HI ||
      variant == TUSB32x::Variant::TUSB320HAI; ///< Exact HI/HAI polarity; all other supported parts active low.
};
static_assert(BoardConfig::variant >= TUSB32x::Variant::TUSB320 &&
    BoardConfig::variant <= TUSB32x::Variant::TUSB322I, "Select an explicit supported variant");
static_assert((BoardConfig::variant == TUSB32x::Variant::TUSB320 ||
    BoardConfig::variant == TUSB32x::Variant::TUSB320I)
    ? (BoardConfig::address == 0x60 || BoardConfig::address == 0x61)
    : (BoardConfig::address == 0x47 || BoardConfig::address == 0x67), "Variant/address mismatch");
} // namespace tusb32x_idf
