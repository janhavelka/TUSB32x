/// @file TUSB32xProtocol.h
/// @brief Private terminal register transport and pure exact-part identity checks.
/// @details This is not a public API or an operation engine. The serialized owner must enforce
/// identity, safe byte composition and silicon sequences before requesting a write. No allocation,
/// retry, clock, device lifecycle policy or framework dependency belongs at this boundary.
#pragma once

#include <cstddef>
#include <cstdint>

#include "TUSB32x/TUSB32x.h"

namespace TUSB32x {
/// @cond TUSB32X_INTERNAL
namespace detail {

/// @brief Closed exact-part capabilities shared by transport and pure codecs.
struct VariantInfo {
  Variant variant; ///< Application-selected exact part, never inferred from shared identity.
  bool hasRevision; ///< Documented A0 register is present.
  bool hasRdRp; ///< Documented 45 register is present.
  bool hasExtendedControls; ///< 09 accessory-disable and 0A preference/disableTerm fields exist.
  uint8_t revision; ///< Exact accepted A0 byte; unused when hasRevision is false.
};

/// @brief Look up the seven-entry closed capability table without I2C or allocation.
/// @param[in] variant Exact part, including an invalid/unspecified cast.
/// @return Static table entry, or nullptr for an invalid variant; no capability inference.
const VariantInfo* findVariant(Variant variant);

/// @brief Validate scalar raw access without invoking transport or changing state.
/// @param[in] variant Bound exact part, or UNSPECIFIED for variant-independent prebinding checks.
/// @param[in] reg Scalar mapped register; holes are INVALID_PARAM and RO writes UNSUPPORTED_FEATURE.
/// @param[in] write True for raw write access; false for diagnostic reads.
/// @return OK or local access error. Absent/gated bound-part access is UNSUPPORTED_FEATURE;
/// malformed specified variants are INVALID_CONFIG. No byte encoding or owner proof is checked.
Status validateRegisterAccess(Variant variant, uint8_t reg, bool write);

/// @brief Explicit accounting origin, independent of aliases or callback outcome.
enum class TrafficSource : uint8_t {
  MANAGED, ///< Production operation, counted in all-source and managed history.
  RAW      ///< Explicit diagnostics, counted only in all-source history.
};

/// @brief Evidence from this one write attempt; never final-operation postcondition proof.
enum class WriteAcceptance : uint8_t {
  NOT_ATTEMPTED,      ///< No data-changing callback invoked, including every read.
  CALLBACK_SUCCEEDED, ///< Complete write callback returned OK; state still needs verification.
  POSSIBLE           ///< Invoked write failed; Status alone cannot exclude prior acceptance.
};

/// @brief Borrowed owner storage, valid only during a synchronous helper call.
struct ProtocolContext {
  const DriverConfig& config; ///< Validated per request; callbacks/context are application-owned.
  TransportStats& stats; ///< Instance-local all-source counters updated only at actual invocation.
  DriverDiagnostics& diagnostics; ///< Only last-failed transport and managed history fields change.
};

/// @brief Separate normalized outcome, callback count evidence and possible write acceptance.
struct TransferResult {
  Status status{}; ///< Original legal callback result or local/normalized failure with full detail.
  bool callbackInvoked = false; ///< Exactly one callback ran; useful even when counters saturate.
  /// @brief Independent evidence of data-changing callback acceptance.
  WriteAcceptance writeAcceptance = WriteAcceptance::NOT_ATTEMPTED;
};

/// @brief Read one mapped contiguous range with a fresh pointer and fixed 11-byte scratch buffer.
/// @param[in] context Borrowed serialized owner/binding storage; never retained.
/// @param[in] source Explicit managed/raw origin; invalid casts are rejected locally.
/// @param[in] reg First offset; range stays within 00..0A or one present 45/A0.
/// @param[out] out Nonnull storage for length bytes; unchanged on rejection or callback error.
/// @param[in] length Exact receive count, 1..11; adapter must reject short transfers.
/// @param[in] remainingMs Positive available owner budget in milliseconds; clipped to config cap.
/// @return At most one terminal callback, TX 1 byte/RX length bytes; no retries. Local
/// INVALID_CONFIG/INVALID_PARAM/UNSUPPORTED_FEATURE failures do not affect accounting.
/// Callback errors preserve legal I2C_* status/detail. Pointer-write STOP/read framing and
/// aggregate lock/time bounds are application obligations. No callback completion timestamp
/// or physical transaction count is known.
TransferResult readRegisters(const ProtocolContext& context, TrafficSource source, uint8_t reg,
                             uint8_t* out, size_t length, uint64_t remainingMs);

/// @brief Send one precomposed [register,value] write to a mapped writable register.
/// @param[in] context Borrowed serialized owner/binding storage; never retained.
/// @param[in] source Explicit managed/raw accounting origin.
/// @param[in] reg Writable offset 08/09/0A/45 as present; 322I 08/09 always gated.
/// @param[in] value Already safely composed byte; does not grant sequence permission.
/// @param[in] remainingMs Positive available milliseconds; clipped to config cap.
/// @return At most one callback, TX 2 bytes/RX 0; same local/error rules as readRegisters.
/// Every invoked failed write retains POSSIBLE acceptance, even NACK/BUSY, because callbacks
/// have no separate phase/byte evidence. No lifecycle, trust, reset or timing state changes here.
TransferResult writeRegister(const ProtocolContext& context, TrafficSource source, uint8_t reg,
                             uint8_t value, uint64_t remainingMs);

/// @brief Compare all eight ascending identity bytes, including NUL, without I2C or mutation.
/// @param[in] variant Explicit exact-part expectation; no inference of model or enable polarity.
/// @param[in] bytes Borrowed complete signature, never retained or modified.
/// @param[in] length Must equal eight; a partial observation cannot establish a match.
/// @retval OK Complete signature matches the configured family.
/// @retval CHIP_ID_MISMATCH Complete signature differs; caller retains raw evidence separately.
/// @retval INVALID_CONFIG Invalid variant.
/// @retval INVALID_PARAM Null signature or length other than eight; no I2C or state mutation.
Status checkIdentity(Variant variant, const uint8_t* bytes, size_t length);

/// @brief Compare one observed A0 byte to the exact configured revision; no I2C or mutation.
/// @param[in] variant Explicit exact-part expectation; base parts have no revision register.
/// @param[in] revision Successfully read raw byte; retained by the caller even on mismatch.
/// @retval OK LI/HI=01, LAI/HAI/322I=02, without capability expansion.
/// @retval CHIP_ID_MISMATCH Unknown or different revision, including 06; no numeric fallback.
/// @retval UNSUPPORTED_FEATURE Base 320/I has no documented A0; do not read it.
/// @retval INVALID_CONFIG Invalid variant; zero callbacks/allocation and unchanged caller state.
Status checkRevision(Variant variant, uint8_t revision);

}  // namespace detail
/// @endcond
}  // namespace TUSB32x
