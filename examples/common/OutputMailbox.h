/// @file OutputMailbox.h
/// @brief One copied output chunk separates owner progress from a blocking USB worker.
#pragma once

#include <atomic>
#include <cstddef>
#include <cstring>

namespace tusb32x_cli {

/// @brief Fixed single-producer/single-consumer output handoff; owns no task or peripheral.
/// @details Only the owner calls offer(), and only one USB worker calls drain(). The worker
/// may block inside its backend without delaying offer(): the occupied slot rejects new data.
/// Use static lifetime in the board examples. Never destroy this object or its backend context
/// while either task can use it. Do not re-enter drain(), and do not share the producer role.
/// Acceptance means copied or accepted by the SDK, not received by the USB host.
class OutputMailbox {
 public:
  static constexpr std::size_t CAPACITY = 256; ///< Maximum copied bytes; one slot, initially empty.

  /// @brief Worker backend callback; unlike the owner's offer(), it may block in the SDK.
  /// @param[in] data Borrowed remaining bytes, valid until this callback returns.
  /// @param[in] length Offered bytes, from one through CAPACITY.
  /// @param[in,out] context Worker-owned backend context, borrowed through return.
  /// @return Accepted bytes; zero retains everything, and values above length are clamped.
  /// @note Do not retain data or call drain() recursively. No core or I2C work belongs here.
  using WriteFn = std::size_t (*)(const char* data, std::size_t length, void* context);

  /// @brief Create an empty slot before starting either task; no allocation or backend calls.
  /// @note C++17 atomic_flag is guaranteed lock-free, unlike atomic<bool> on ESP32-S2.
  OutputMailbox() { _ready.test_and_set(std::memory_order_relaxed); }

  /// @brief Copy a bounded prefix into an empty slot without waiting for the worker.
  /// @param[in] data Borrowed bytes, copied before return; null is rejected.
  /// @param[in] length Available bytes; zero is rejected and larger values are clipped.
  /// @return Copied bytes, at most CAPACITY, or zero if occupied or input is invalid.
  /// @note Single producer only. No callback, allocation, wait or retry occurs here.
  std::size_t offer(const char* data, std::size_t length) {
    if (!data || length == 0 || _occupied.test_and_set(std::memory_order_acquire)) return 0;
    const std::size_t count = length < CAPACITY ? length : CAPACITY;
    std::memcpy(_data, data, count);
    _length = count;
    _ready.clear(std::memory_order_release);
    return count;
  }

  /// @brief Offer remaining bytes to the worker backend at most once; retain short writes.
  /// @param[in] write Backend callback; null leaves the slot unchanged.
  /// @param[in,out] context Borrowed callback context; never used by the producer.
  /// @return Accepted bytes, at most CAPACITY; zero when empty, null or stalled.
  /// @note Single consumer only. No lock is held across the callback. A blocked backend keeps
  /// this slot occupied but cannot block the producer. The slot is released only after all
  /// its bytes are accepted; it is never overwritten or discarded to admit newer output.
  std::size_t drain(WriteFn write, void* context) {
    if (!write) return 0;
    if (!_reading) {
      if (_ready.test_and_set(std::memory_order_acquire)) return 0;
      _reading = true;
      _offset = 0;
    }
    const std::size_t remaining = _length - _offset;
    std::size_t accepted = write(_data + _offset, remaining, context);
    if (accepted > remaining) accepted = remaining;
    _offset += accepted;
    if (_offset == _length) {
      _reading = false;
      _occupied.clear(std::memory_order_release);
    }
    return accepted;
  }

 private:
  // Each flag is tested once, never spun on or waited for. The first reserves storage;
  // the second publishes the completed copy. The consumer releases storage only after
  // its callback returns, and then touches no shared bytes until the next publication.
  // Separate words avoid false sharing in Xtensa's word-sized byte-flag exchange helper.
  alignas(4) std::atomic_flag _occupied = ATOMIC_FLAG_INIT;
  alignas(4) std::atomic_flag _ready = ATOMIC_FLAG_INIT;
  char _data[CAPACITY]{};
  std::size_t _length = 0;
  std::size_t _offset = 0; ///< Consumer-only offset; never accessed by the producer.
  bool _reading = false; ///< Consumer owns one published chunk until it is completely accepted.
};

} // namespace tusb32x_cli
