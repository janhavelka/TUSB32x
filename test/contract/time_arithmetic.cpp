/**
 * @file time_arithmetic.cpp
 * @brief Compile-only specification vectors for unsigned owner-clock arithmetic.
 *
 * These independent constexpr helpers describe the planned arithmetic contract.
 * They are not production helpers or an operation engine. Static assertions prove
 * the listed integer relationships, not callback timing, fresh clock sampling,
 * engine transitions, adapter bounds or physical hardware waits.
 */
#include <TUSB32x/TUSB32x.h>

#include <cstdint>
#include <limits>

namespace TimeArithmetic {

constexpr uint64_t HALF_RANGE = uint64_t{1} << 63;
constexpr uint64_t MAX_TIME = std::numeric_limits<uint64_t>::max();
constexpr uint32_t HALF_RANGE_32 = uint32_t{1} << 31;
constexpr uint32_t MAX_CALLBACK_CAP = std::numeric_limits<uint32_t>::max();

// Admission requires a positive interval strictly below the unsigned half range.
constexpr bool validInterval(uint64_t nowMs, uint64_t deadlineMs) {
  const uint64_t intervalMs = deadlineMs - nowMs;
  return intervalMs != 0 && intervalMs < HALF_RANGE;
}

// Equal ticks are forward-valid but do not create elapsed time.
constexpr bool forwardSample(uint64_t previousMs, uint64_t nowMs) {
  return nowMs - previousMs < HALF_RANGE;
}

// Use after sample validation. Equality expires the exclusive deadline; an
// exactly half-range comparison is ambiguous and cannot prove expiry.
constexpr bool deadlineReached(uint64_t nowMs, uint64_t deadlineMs) {
  return nowMs - deadlineMs < HALF_RANGE;
}

// Zero means no positive unambiguous time remains, so no callback is admissible.
constexpr uint64_t remainingMs(uint64_t nowMs, uint64_t deadlineMs) {
  return validInterval(nowMs, deadlineMs) ? deadlineMs - nowMs : 0;
}

// Clamp in the wide domain before narrowing. Zero is a no-callback decision,
// never a request to pass a backend its possibly unbounded zero-timeout value.
constexpr uint32_t callbackCap(uint64_t nowMs, uint64_t deadlineMs, uint32_t capMs) {
  const uint64_t remaining = remainingMs(nowMs, deadlineMs);
  return remaining < capMs ? static_cast<uint32_t>(remaining) : capMs;
}

// The engine must separately validate samples against its last accepted time.
// Wait completion does not authorize a callback at or after the owner deadline.
constexpr bool waitReached(uint64_t anchorMs, uint64_t nowMs, uint64_t waitTicks) {
  const uint64_t elapsed = nowMs - anchorMs;
  return waitTicks != 0 && waitTicks < HALF_RANGE && elapsed < HALF_RANGE &&
         elapsed >= waitTicks;
}

// Same extension rule as owner_examples.cpp: first sample starts the domain;
// later samples need delta < 2^31 with no reset and sufficiently frequent reads.
// Rejection preserves the extension, prior sample, initialization flag and out.
constexpr bool extendClock32(uint32_t rawTickMs, uint32_t& previousTickMs,
                             uint64_t& extendedMs, bool& started, uint64_t& out) {
  if (!started) {
    previousTickMs = rawTickMs;
    extendedMs = rawTickMs;
    started = true;
  } else {
    const uint32_t elapsedMs = rawTickMs - previousTickMs;
    if (elapsedMs >= HALF_RANGE_32) {
      return false;
    }
    extendedMs += elapsedMs;
    previousTickMs = rawTickMs;
  }
  out = extendedMs;
  return true;
}

constexpr bool extendedSampleMatches(uint32_t previous, uint64_t extended, bool started,
                                     uint32_t raw, uint64_t expected) {
  uint64_t out = 99;
  return extendClock32(raw, previous, extended, started, out) && started &&
         previous == raw && extended == expected && out == expected;
}

constexpr bool rejectedSamplePreserves(uint32_t previous, uint32_t raw) {
  const uint32_t originalPrevious = previous;
  uint64_t extended = 0x100000010ULL;
  bool started = true;
  uint64_t out = 99;
  return !extendClock32(raw, previous, extended, started, out) &&
         previous == originalPrevious && extended == 0x100000010ULL && started && out == 99;
}

// Admission and forward-sample boundaries, including natural uint64 rollover.
static_assert(!validInterval(100, 100));
static_assert(validInterval(100, 101));
static_assert(validInterval(0, HALF_RANGE - 1));
static_assert(!validInterval(0, HALF_RANGE));
static_assert(!validInterval(0, HALF_RANGE + 1));
static_assert(!validInterval(100, 99));
static_assert(validInterval(MAX_TIME - 2, 3));
static_assert(forwardSample(100, 100));
static_assert(forwardSample(100, 101));
static_assert(forwardSample(0, HALF_RANGE - 1));
static_assert(!forwardSample(0, HALF_RANGE));
static_assert(!forwardSample(100, 99));
static_assert(forwardSample(MAX_TIME - 2, 3));

// Exclusive deadline and remaining time; no renewed interval after expiry.
static_assert(!deadlineReached(113, 114));
static_assert(deadlineReached(114, 114));
static_assert(deadlineReached(115, 114));
static_assert(!deadlineReached(2, 3));
static_assert(deadlineReached(3, 3));
static_assert(!deadlineReached(MAX_TIME - 2, 3));
static_assert(!deadlineReached(HALF_RANGE, 0));
static_assert(remainingMs(100, 114) == 14);
static_assert(remainingMs(114, 114) == 0);
static_assert(remainingMs(115, 114) == 0);
static_assert(remainingMs(MAX_TIME - 2, 3) == 6);
static_assert(remainingMs(0, HALF_RANGE) == 0);

// A wide remaining interval cannot truncate into a smaller accidental timeout.
static_assert(callbackCap(100, 114, 20) == 14);
static_assert(callbackCap(100, 130, 20) == 20);
static_assert(callbackCap(114, 114, 20) == 0);
static_assert(callbackCap(100, 114, 0) == 0);
static_assert(callbackCap(0, 0x100000001ULL, 20) == 20);
static_assert(callbackCap(0, HALF_RANGE - 1, MAX_CALLBACK_CAP) == 0xFFFFFFFFULL);
static_assert(callbackCap(MAX_TIME - 2, 3, 20) == 6);

// TI minimum intervals and the separate one-tick quantization margin. These
// arithmetic vectors do not prove that a callback's actual completion was fresh.
static_assert(TUSB32x::OWNER_CLOCK_MARGIN_MS == 1);
static_assert(5 + TUSB32x::OWNER_CLOCK_MARGIN_MS == 6);
static_assert(30 + TUSB32x::OWNER_CLOCK_MARGIN_MS == 31);
static_assert(95 + TUSB32x::OWNER_CLOCK_MARGIN_MS == 96);
static_assert(!waitReached(108, 108, 6));
static_assert(!waitReached(108, 113, 6));
static_assert(waitReached(108, 114, 6));
static_assert(!waitReached(200, 230, 31));
static_assert(waitReached(200, 231, 31));
static_assert(!waitReached(300, 395, 96));
static_assert(waitReached(300, 396, 96));
static_assert(!waitReached(MAX_TIME - 2, 2, 6));
static_assert(waitReached(MAX_TIME - 2, 3, 6));
static_assert(!waitReached(100, 99, 6));
static_assert(!waitReached(0, HALF_RANGE, 6));
static_assert(!waitReached(0, 6, 0));
static_assert(!waitReached(0, 6, HALF_RANGE));
static_assert(waitReached(108, 114, 6) && deadlineReached(114, 114));

// The fake owner's extension accepts equal ticks, handles 32-bit rollover and
// preserves all state/output on half-range or backward input.
static_assert(extendedSampleMatches(0, 0, false, 123, 123));
static_assert(extendedSampleMatches(123, 123, true, 123, 123));
static_assert(extendedSampleMatches(123, 123, true, 124, 124));
static_assert(extendedSampleMatches(0xFFFFFFFEU, 0xFFFFFFFEULL, true, 1, 0x100000001ULL));
static_assert(extendedSampleMatches(0, 0, true, HALF_RANGE_32 - 1, 0x7FFFFFFFULL));
static_assert(extendedSampleMatches(10, MAX_TIME - 1, true, 13, 1));
static_assert(rejectedSamplePreserves(0, HALF_RANGE_32));
static_assert(rejectedSamplePreserves(0, HALF_RANGE_32 + 1));
static_assert(rejectedSamplePreserves(100, 99));
static_assert(rejectedSamplePreserves(0xFFFFFFF0U, 0x7FFFFFF0U));

}  // namespace TimeArithmetic
