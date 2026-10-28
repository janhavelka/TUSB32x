/// @file BoardConfig.h
/// @brief Explicit example wiring and qualification switches; no board power policy.
#pragma once
#include <TUSB32x/Config.h>

#ifndef TUSB32X_VARIANT
/// @brief Exact primary variant enum name; default TUSB320LAI.
#define TUSB32X_VARIANT TUSB320LAI
#endif
#ifndef TUSB32X_ADDRESS
/// @brief Primary seven-bit ADDR strap; default 0x47.
#define TUSB32X_ADDRESS 0x47
#endif
#ifndef TUSB32X_SECOND_VARIANT
/// @brief Exact secondary variant enum name; default TUSB320LAI.
#define TUSB32X_SECOND_VARIANT TUSB320LAI
#endif
#ifndef TUSB32X_SECOND_ADDRESS
/// @brief Secondary seven-bit ADDR strap; default 0x67.
#define TUSB32X_SECOND_ADDRESS 0x67
#endif
#ifndef TUSB32X_SDA
/// @brief Example SDA GPIO; default 4, verify board wiring.
#define TUSB32X_SDA 4
#endif
#ifndef TUSB32X_SCL
/// @brief Example SCL GPIO; default 5, verify board wiring.
#define TUSB32X_SCL 5
#endif
#ifndef TUSB32X_IRQ
/// @brief Optional primary IRQ input GPIO; -1 means absent.
#define TUSB32X_IRQ -1
#endif
#ifndef TUSB32X_SECOND_IRQ
/// @brief Optional secondary IRQ input GPIO; -1 means absent.
#define TUSB32X_SECOND_IRQ -1
#endif
#ifndef TUSB32X_ENABLE
/// @brief Report-only externally controlled enable GPIO; -1 means unspecified.
#define TUSB32X_ENABLE -1
#endif
#ifndef TUSB32X_BUS_HZ
/// @brief Requested SCL frequency in Hz; default 100000, maximum 400000.
#define TUSB32X_BUS_HZ 100000
#endif
#ifndef TUSB32X_CALLBACK_MS
/// @brief Aggregate adapter callback budget in milliseconds; default 400.
#define TUSB32X_CALLBACK_MS 400
#endif
#ifndef TUSB32X_OPERATION_MS
/// @brief Immutable core operation interval in milliseconds; default 5000.
#define TUSB32X_OPERATION_MS 5000
#endif
#ifndef TUSB32X_BOARD_READY
/// @brief Explicit qualified board-readiness opt-in; default zero disables bus setup.
#define TUSB32X_BOARD_READY 0
#endif
#ifndef TUSB32X_MAX_OVERHEAD_US
/// @brief Qualified whole-callback CPU overhead in microseconds; default zero is invalid.
#define TUSB32X_MAX_OVERHEAD_US 0
#endif
#ifndef TUSB32X_ACCEPT_SDK_CLEANUP
/// @brief Explicit SDK controller reset/bus-clear acceptance; default zero.
#define TUSB32X_ACCEPT_SDK_CLEANUP 0
#endif
#ifndef TUSB32X_RUN_MS
/// @brief Finite runtime before shutdown in milliseconds; zero disables automatic shutdown.
#define TUSB32X_RUN_MS 0
#endif

namespace tusb32x_arduino {
static_assert(TUSB32X_ADDRESS >= 0 && TUSB32X_ADDRESS <= 0x7F, "Primary address must be seven-bit before narrowing");
static_assert(TUSB32X_SECOND_ADDRESS >= 0 && TUSB32X_SECOND_ADDRESS <= 0x7F, "Second address must be seven-bit before narrowing");
static_assert(TUSB32X_BUS_HZ > 0 && TUSB32X_BUS_HZ <= 400000, "TI SCL limit is 400 kHz before narrowing");
static_assert(TUSB32X_CALLBACK_MS > 0 && TUSB32X_CALLBACK_MS <= UINT32_MAX, "Callback interval must fit uint32 milliseconds");
static_assert(TUSB32X_OPERATION_MS > 0 && TUSB32X_OPERATION_MS <= UINT32_MAX, "Operation interval must fit uint32 milliseconds");
static_assert(TUSB32X_MAX_OVERHEAD_US >= 0 && TUSB32X_MAX_OVERHEAD_US <= UINT32_MAX, "Overhead must fit uint32 microseconds");
static_assert(TUSB32X_RUN_MS >= 0 && TUSB32X_RUN_MS < (UINT64_C(1) << 63), "Run interval must be below half range");
static_assert(TUSB32X_IRQ >= -1 && TUSB32X_SECOND_IRQ >= -1 && TUSB32X_ENABLE >= -1, "Absent optional pins use -1");
static_assert(TUSB32X_SDA >= 0 && TUSB32X_SDA <= INT32_MAX && TUSB32X_SCL >= 0 && TUSB32X_SCL <= INT32_MAX,
              "Bus GPIO numbers must fit signed 32 bits before target validation");
static_assert(TUSB32X_IRQ <= INT32_MAX && TUSB32X_SECOND_IRQ <= INT32_MAX && TUSB32X_ENABLE <= INT32_MAX,
              "Optional GPIO numbers must fit signed 32 bits before target validation");
static_assert(TUSB32X_ENABLE < 0 || (TUSB32X_ENABLE != TUSB32X_IRQ && TUSB32X_ENABLE != TUSB32X_SECOND_IRQ),
              "Externally owned enable cannot share a configured IRQ input");
/// @brief Build-time application choices; GPIO4/5 are examples, not detected fixture wiring.
struct BoardConfig {
  static constexpr TUSB32x::Variant variant = TUSB32x::Variant::TUSB32X_VARIANT; ///< Exact primary part.
  static constexpr uint8_t address = TUSB32X_ADDRESS; ///< Primary seven-bit strapped address.
  static constexpr TUSB32x::Variant secondVariant = TUSB32x::Variant::TUSB32X_SECOND_VARIANT; ///< Exact second part.
  static constexpr uint8_t secondAddress = TUSB32X_SECOND_ADDRESS; ///< Distinct second target address.
  static constexpr int sda = TUSB32X_SDA; ///< Application SDA pin, default GPIO4.
  static constexpr int scl = TUSB32X_SCL; ///< Application SCL pin, default GPIO5.
  static constexpr int irq = TUSB32X_IRQ; ///< Optional primary input, -1 means absent.
  static constexpr int secondIrq = TUSB32X_SECOND_IRQ; ///< Optional secondary input/ISR signal.
  static constexpr int enable = TUSB32X_ENABLE; ///< Report-only externally controlled enable pin; never driven here.
  static constexpr uint32_t busHz = TUSB32X_BUS_HZ; ///< SCL hertz, at most 400000.
  static constexpr uint32_t callbackMs = TUSB32X_CALLBACK_MS; ///< Whole adapter callback budget.
  static constexpr uint32_t operationMs = TUSB32X_OPERATION_MS; ///< Immutable owner job interval.
  static constexpr uint32_t maxOverheadUs = TUSB32X_MAX_OVERHEAD_US; ///< Qualified whole-callback CPU bound; zero disables I2C.
  static constexpr bool boardReady = TUSB32X_BOARD_READY != 0; ///< Explicit stable supply/strap/enable/wiring qualification.
  static constexpr bool acceptSdkCleanup = TUSB32X_ACCEPT_SDK_CLEANUP != 0; ///< Accept SDK reset/bus-clear effects on all bus users.
  static constexpr uint64_t runMs = TUSB32X_RUN_MS; ///< Zero runs until requestStop; positive triggers orderly local shutdown.
};
static_assert(BoardConfig::busHz > 0 && BoardConfig::busHz <= 400000, "TI SCL limit is 400 kHz");
static_assert(BoardConfig::callbackMs > 0 && BoardConfig::operationMs > 0, "Finite positive timeouts required");
static_assert(BoardConfig::variant >= TUSB32x::Variant::TUSB320 && BoardConfig::variant <= TUSB32x::Variant::TUSB322I,
              "Select an explicit supported primary variant");
static_assert(BoardConfig::secondVariant >= TUSB32x::Variant::TUSB320 && BoardConfig::secondVariant <= TUSB32x::Variant::TUSB322I,
              "Select an explicit supported secondary variant");
static_assert((BoardConfig::variant == TUSB32x::Variant::TUSB320 || BoardConfig::variant == TUSB32x::Variant::TUSB320I)
    ? (BoardConfig::address == 0x60 || BoardConfig::address == 0x61)
    : (BoardConfig::address == 0x47 || BoardConfig::address == 0x67), "Primary variant/address mismatch");
static_assert((BoardConfig::secondVariant == TUSB32x::Variant::TUSB320 || BoardConfig::secondVariant == TUSB32x::Variant::TUSB320I)
    ? (BoardConfig::secondAddress == 0x60 || BoardConfig::secondAddress == 0x61)
    : (BoardConfig::secondAddress == 0x47 || BoardConfig::secondAddress == 0x67), "Secondary variant/address mismatch");
} // namespace tusb32x_arduino
