/// @file TUSB32xOwnerTime.h
/// @brief Private unsigned owner arithmetic; no clock or mutable global state.
#pragma once

#include <cstdint>
#include <limits>

namespace TUSB32x {
namespace detail {

inline constexpr uint64_t HALF_RANGE = uint64_t{1} << 63; ///< Ambiguous uint64 clock distance.

/// @brief Advance a publication count without wrapping; saturation does not identify an event.
/// @param[in] previous Previous publication count, initially zero.
/// @return Previous plus one, or UINT64_MAX unchanged; no callbacks or mutable state.
constexpr uint64_t nextSequence(uint64_t previous) {
  return previous == std::numeric_limits<uint64_t>::max() ? previous : previous + 1;
}

/// @brief Allocate the next nonzero token without wrapping or resetting instance history.
/// @param[in] previous Last admitted token; zero before the first admission.
/// @param[out] out Next token on true; unchanged on exhaustion.
/// @return False only when uint64 token space is exhausted; performs no callbacks.
constexpr bool nextToken(uint64_t previous, uint64_t& out) {
  if (previous == std::numeric_limits<uint64_t>::max()) return false;
  out = previous + 1;
  return true;
}

}  // namespace detail
}  // namespace TUSB32x
