/// @file ConsoleBuffer.h
/// @brief Fixed-storage input lines and whole-record output for the example CLI.
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

/// @brief Portable, application-owned CLI helpers; these are not core library APIs.
namespace tusb32x_cli {
namespace console_detail {
/// @brief Add a bounded console count without wrapping or accessing mutable state.
/// @param[in] value Current uint32 byte/record count.
/// @param[in] increment Requested increase; SIZE_MAX is supported without narrowing first.
/// @return Mathematical sum capped at UINT32_MAX; no allocation, callbacks or mutation.
constexpr uint32_t saturatingAdd(uint32_t value, std::size_t increment) {
  return increment > std::numeric_limits<uint32_t>::max() - value
      ? std::numeric_limits<uint32_t>::max() : value + static_cast<uint32_t>(increment);
}
}  // namespace console_detail

/// @brief Single-owner bounded console; performs no allocation, command dispatch or bus work.
/// @note The application serializes all access. Backend callbacks must return promptly and
/// must not retain the borrowed bytes or re-enter this object. Input and output are independent.
class ConsoleBuffer {
 public:
  static constexpr std::size_t LINE_CAPACITY = 192;  ///< Input storage, including the final NUL.
  static constexpr std::size_t INPUT_BUDGET = 64;  ///< Maximum input bytes consumed by one feed.
  static constexpr std::size_t OUTPUT_CAPACITY = 8192;  ///< Fixed queued-output storage in bytes.
  static constexpr std::size_t RECORD_CAPACITY = 512;  ///< Record storage including its final NUL.
  static constexpr std::size_t DRAIN_BUDGET = 256;  ///< Maximum bytes offered in one backend call.

  /// @brief Nonblocking write; returns accepted bytes, with zero meaning no progress.
  /// @param[in] data Borrowed contiguous output bytes, valid only during the call.
  /// @param[in] length Offered byte count, at most DRAIN_BUDGET.
  /// @param[in] context Application-owned callback context.
  /// @return Accepted bytes. Invalid values above length are clamped to length.
  using WriteFn = std::size_t (*)(const char* data, std::size_t length, void* context);

  /// @brief Consume input until a complete line or the per-call byte budget is reached.
  /// @param[in] data Borrowed input; null consumes no bytes.
  /// @param[in] length Available bytes; unread bytes remain the caller's responsibility.
  /// @return Consumed bytes, at most INPUT_BUDGET; zero while a line awaits consumeLine().
  /// @note CR, LF and CRLF terminate lines, including split CRLF. ASCII space through '~'
  /// and tab are accepted. Backspace/DEL edit valid bytes. Other bytes and overflow poison
  /// the complete line until its terminator; editing cannot repair poisoned input.
  std::size_t feed(const char* data, std::size_t length) {
    if (_lineReady || !data) return 0;
    const std::size_t limit = length < INPUT_BUDGET ? length : INPUT_BUDGET;
    std::size_t consumed = 0;
    while (consumed < limit) {
      const unsigned char byte = static_cast<unsigned char>(data[consumed++]);
      if (_skipLf) {
        _skipLf = false;
        if (byte == '\n') continue;
      }
      if (byte == '\r' || byte == '\n') {
        _skipLf = byte == '\r';
        _lineReady = true;
        break;
      }
      if (_linePoisoned) continue;
      if (byte == 8 || byte == 127) {
        if (_lineLength != 0) _line[--_lineLength] = '\0';
      } else if ((byte >= 32 && byte <= 126) || byte == '\t') {
        if (_lineLength == LINE_CAPACITY - 1) _linePoisoned = true;
        else {
          _line[_lineLength++] = static_cast<char>(byte);
          _line[_lineLength] = '\0';
        }
      } else {
        _linePoisoned = true;
      }
    }
    return consumed;
  }

  /// @brief Return whether one complete line awaits consumption; performs no callbacks.
  /// @return True only until consumeLine releases the complete line.
  bool lineReady() const { return _lineReady; }
  /// @brief Return sticky malformed-input evidence for the current line, complete or partial.
  /// @return True after an illegal byte or overflow, until a complete line is consumed.
  bool linePoisoned() const { return _linePoisoned; }
  /// @brief Borrow the NUL-terminated current payload until the next feed() or consumeLine().
  /// @note Dispatch only when lineReady() is true and linePoisoned() is false.
  /// @return Borrowed fixed input storage; never null.
  const char* line() const { return _line; }
  /// @brief Release a complete line and its poison evidence; incomplete lines stay unchanged.
  /// @note A pending CRLF suffix remains recognized after consuming the CR-terminated line.
  void consumeLine() {
    if (!_lineReady) return;
    _lineReady = false;
    _linePoisoned = false;
    _lineLength = 0;
    _line[0] = '\0';
  }

  /// @brief Copy one whole newline-terminated record into the fixed output queue.
  /// @param[in] data Borrowed record bytes, copied before return; null is rejected.
  /// @param[in] length Payload bytes including newline, from 1 through RECORD_CAPACITY - 1.
  /// No trailing C terminator is queued; embedded NUL bytes are rejected.
  /// @return True when copied in full; false on invalid framing, size or insufficient space.
  /// @note Rejection preserves queued bytes, increments saturating loss totals and retains
  /// a pending loss marker. Records may include multiple human-output lines.
  bool enqueue(const char* data, std::size_t length) {
    if (!data || length == 0 || length >= RECORD_CAPACITY || length > available() ||
        data[length - 1] != '\n' || std::memchr(data, '\0', length)) {
      _droppedRecords = console_detail::saturatingAdd(_droppedRecords, 1);
      _droppedBytes = console_detail::saturatingAdd(_droppedBytes, length);
      _lossPending = true;
      return false;
    }
    append(data, length);
    return true;
  }

  /// @brief Return queued bytes, including any already queued loss marker; no callbacks.
  /// @return Current output occupancy, in 0..OUTPUT_CAPACITY bytes.
  std::size_t queued() const { return _used; }
  /// @brief Return currently unused queue bytes; this does not reserve space.
  /// @return OUTPUT_CAPACITY minus queued bytes.
  std::size_t available() const { return OUTPUT_CAPACITY - _used; }
  /// @brief Return cumulative rejected whole records, saturated at UINT32_MAX.
  /// @return Retained record-loss count; draining does not clear it.
  uint32_t droppedRecords() const { return _droppedRecords; }
  /// @brief Return cumulative rejected payload bytes, saturated at UINT32_MAX.
  /// @return Retained byte-loss count; draining does not clear it.
  uint32_t droppedBytes() const { return _droppedBytes; }
  /// @brief Return whether loss evidence still needs to be queued as a marker.
  /// @note False means the marker was queued, not that a backend accepted or delivered it.
  /// @return True while a marker still awaits sufficient queue capacity.
  bool lossPending() const { return _lossPending; }

  /// @brief Attempt pending loss-marker insertion and at most one bounded backend write.
  /// @param[in] write Nonblocking callback; null performs no backend call.
  /// @param[in] context Borrowed callback context, passed through unchanged.
  /// @return Accepted bytes, at most DRAIN_BUDGET; short/zero writes retain the remainder.
  /// @note Marker insertion is attempted before and after the single write. A wrapped queue
  /// offers only its first contiguous span. A marker never increases loss counters itself.
  std::size_t drain(WriteFn write, void* context) {
    queueLossMarker();
    if (!write || _used == 0) return 0;
    std::size_t count = _used < OUTPUT_CAPACITY - _head ? _used : OUTPUT_CAPACITY - _head;
    if (count > DRAIN_BUDGET) count = DRAIN_BUDGET;
    std::size_t accepted = write(_output + _head, count, context);
    if (accepted > count) accepted = count;
    _head = (_head + accepted) % OUTPUT_CAPACITY;
    _used -= accepted;
    queueLossMarker();
    return accepted;
  }

 private:
  void append(const char* data, std::size_t length) {
    const std::size_t tail = (_head + _used) % OUTPUT_CAPACITY;
    const std::size_t first = length < OUTPUT_CAPACITY - tail ? length : OUTPUT_CAPACITY - tail;
    std::memcpy(_output + tail, data, first);
    if (length != first) std::memcpy(_output, data + first, length - first);
    _used += length;
  }

  void queueLossMarker() {
    if (!_lossPending) return;
    // Counts are cumulative; this marker is deliberately independent of command correlation.
    char marker[128];
    const int length = std::snprintf(marker, sizeof(marker),
        "TUSB LOSS schema=1 dropped_records=%lu dropped_bytes=%lu\n",
        static_cast<unsigned long>(_droppedRecords), static_cast<unsigned long>(_droppedBytes));
    if (length > 0 && static_cast<std::size_t>(length) < sizeof(marker) &&
        static_cast<std::size_t>(length) <= available()) {
      append(marker, static_cast<std::size_t>(length));
      _lossPending = false;
    }
  }

  char _line[LINE_CAPACITY]{};
  std::size_t _lineLength = 0;
  bool _lineReady = false;
  bool _linePoisoned = false;
  bool _skipLf = false;
  char _output[OUTPUT_CAPACITY]{};
  std::size_t _head = 0;
  std::size_t _used = 0;
  uint32_t _droppedRecords = 0;
  uint32_t _droppedBytes = 0;
  bool _lossPending = false;
};
}  // namespace tusb32x_cli
