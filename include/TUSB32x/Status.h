/// @file Status.h
/// @brief Portable error values and allocation-free, constant-evaluable status values.
#pragma once

#include <cstdint>

/// @brief Framework-neutral values and operations for explicitly selected TUSB32x parts.
namespace TUSB32x {

/// @brief API outcomes; numeric order is part of the initial public contract.
/// @details Local rejections perform no callback and preserve caller outputs and represented work.
/// Terminal operation failures may retain hardware effects; inspect the separate effect evidence.
enum class Err : uint8_t {
  OK = 0,                    ///< Action succeeded; result delivery does not imply operation success.
  NOT_BOUND,                 ///< Required valid transport/device binding is absent; no callback.
  NOT_INITIALIZED,           ///< Required read-only identity/initialization proof is absent; no write.
  INVALID_CONFIG,            ///< Whole binding or silicon profile is invalid; local rejection.
  INVALID_PARAM,             ///< Argument, enum, register, time interval or budget is invalid; local.
  UNSUPPORTED_FEATURE,       ///< Selected variant does not support this valid request; no callback.
  BUSY,                      ///< An active operation owns the instance; existing work is preserved.
  IN_PROGRESS,               ///< Work was admitted or remains active; illegal as a callback result.
  RESULT_PENDING,            ///< Retained terminal result blocks replacement; take it first.
  RESULT_NOT_AVAILABLE,      ///< No terminal result is available; caller output stays unchanged.
  TOKEN_MISMATCH,            ///< Token differs from represented work; preserve work and outputs.
  TOKEN_EXHAUSTED,           ///< Every nonzero instance token was used; tokens never wrap or repeat.
  TIMEOUT,                   ///< Owner deadline expired; not a callback failure or effect rollback.
  CANCELLED,                 ///< Local cancellation of admitted work; possible effects remain.
  CONFIGURATION_UNKNOWN,     ///< Required verified configuration or effect evidence is unavailable.
  CONFIGURATION_MISMATCH,     ///< Successful persistent-bit readback differed; not a transport fault.
  CHIP_ID_MISMATCH,          ///< Complete successful identity reads contradicted the selected part.
  I2C_ERROR,                 ///< Generic terminal callback failure, including illegal callback status.
  I2C_NACK_ADDR,             ///< Backend positively identified address NACK; earlier effects remain.
  I2C_NACK_DATA,             ///< Backend positively identified data NACK; earlier bytes may act.
  I2C_TIMEOUT,               ///< Terminal callback timeout; one attempt, not proof of wire traffic.
  I2C_BUS,                   ///< Backend positively identified a bus fault; recovery is external.
  I2C_BUSY                   ///< Bounded callback could not acquire/use bus; counts as an attempt.
};

/// @brief Small copied status; owns no memory and performs no I2C.
/// @details Callback statuses are restricted to OK and I2C_* values. The future transport path
/// normalizes other callback codes to I2C_ERROR, preserving detail. Status text has static lifetime.
/// A status alone cannot prove hardware effects or callback completion time.
struct Status {
  Err code = Err::OK;     ///< Typed API outcome; default construction is successful.
  int32_t detail = 0;     ///< Full backend detail; zero when no separately documented detail applies.
  const char* msg = "";  ///< Borrowed static-lifetime text; never a temporary or owned buffer.

  /// @brief Construct OK with zero detail and empty static text; no callbacks.
  constexpr Status() = default;

  /// @brief Copy a complete status representation without validation or callbacks.
  /// @param[in] c Typed outcome.
  /// @param[in] d Full backend or documented implementation detail.
  /// @param[in] m Static-lifetime diagnostic text, borrowed for the status lifetime.
  constexpr Status(Err c, int32_t d, const char* m) : code(c), detail(d), msg(m) {}

  /// @brief Inspect the copied code without I2C.
  /// @return True exactly for Err::OK.
  constexpr bool ok() const { return code == Err::OK; }

  /// @brief Compare copied codes without I2C.
  /// @param[in] value Code to compare, including any explicitly cast value.
  /// @return True exactly when code equals value.
  constexpr bool is(Err value) const { return code == value; }

  /// @brief Inspect progress without I2C.
  /// @return True exactly for Err::IN_PROGRESS.
  constexpr bool inProgress() const { return is(Err::IN_PROGRESS); }

  /// @brief Explicit success conversion; performs no callbacks.
  /// @return True exactly for Err::OK, never for progress.
  explicit constexpr operator bool() const { return ok(); }

  /// @brief Construct canonical success without I2C or allocation.
  /// @return OK with zero detail and static text "OK".
  static constexpr Status Ok() { return {Err::OK, 0, "OK"}; }

  /// @brief Construct a copied status without validation, I2C or allocation.
  /// @param[in] code Typed outcome; this factory does not normalize callback codes.
  /// @param[in] msg Static-lifetime diagnostic text.
  /// @param[in] detail Full backend or documented implementation detail; defaults to zero.
  /// @return The supplied representation with detail and text preserved.
  static constexpr Status Error(Err code, const char* msg, int32_t detail = 0) {
    return {code, detail, msg};
  }
};

/// @brief Get a static diagnostic name without callbacks or allocation.
/// @param[in] code Typed outcome, including an invalid explicitly cast enum value.
/// @return The exact enum spelling, or "UNKNOWN" for an invalid value; static lifetime.
constexpr const char* errorName(Err code) {
  switch (code) {
    case Err::OK: return "OK";
    case Err::NOT_BOUND: return "NOT_BOUND";
    case Err::NOT_INITIALIZED: return "NOT_INITIALIZED";
    case Err::INVALID_CONFIG: return "INVALID_CONFIG";
    case Err::INVALID_PARAM: return "INVALID_PARAM";
    case Err::UNSUPPORTED_FEATURE: return "UNSUPPORTED_FEATURE";
    case Err::BUSY: return "BUSY";
    case Err::IN_PROGRESS: return "IN_PROGRESS";
    case Err::RESULT_PENDING: return "RESULT_PENDING";
    case Err::RESULT_NOT_AVAILABLE: return "RESULT_NOT_AVAILABLE";
    case Err::TOKEN_MISMATCH: return "TOKEN_MISMATCH";
    case Err::TOKEN_EXHAUSTED: return "TOKEN_EXHAUSTED";
    case Err::TIMEOUT: return "TIMEOUT";
    case Err::CANCELLED: return "CANCELLED";
    case Err::CONFIGURATION_UNKNOWN: return "CONFIGURATION_UNKNOWN";
    case Err::CONFIGURATION_MISMATCH: return "CONFIGURATION_MISMATCH";
    case Err::CHIP_ID_MISMATCH: return "CHIP_ID_MISMATCH";
    case Err::I2C_ERROR: return "I2C_ERROR";
    case Err::I2C_NACK_ADDR: return "I2C_NACK_ADDR";
    case Err::I2C_NACK_DATA: return "I2C_NACK_DATA";
    case Err::I2C_TIMEOUT: return "I2C_TIMEOUT";
    case Err::I2C_BUS: return "I2C_BUS";
    case Err::I2C_BUSY: return "I2C_BUSY";
  }
  return "UNKNOWN";
}

}  // namespace TUSB32x
