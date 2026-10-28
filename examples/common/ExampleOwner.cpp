/// @file ExampleOwner.cpp
/// @brief Single-token application scheduling with output-independent startup completion.
#include "ExampleOwner.h"

#include <limits>

namespace tusb32x_example {
namespace t = TUSB32x;

ExampleOwner::ExampleOwner(TUSB32x::TUSB32x& device, uint32_t operationTimeoutMs,
                           uint32_t refreshPeriodMs)
    : _device(device), _timeoutMs(operationTimeoutMs), _periodMs(refreshPeriodMs) {}

void ExampleOwner::report() {
  if (_reportSequence != std::numeric_limits<uint64_t>::max()) ++_reportSequence;
}

void ExampleOwner::fail(t::Status status) {
  _lastStatus = status;
  _stopped = true;
  _startupDone = true;
  _serviceRequested = false;
  report();
}

t::Status ExampleOwner::start(uint64_t nowMs) {
  if (_started || _stopped)
    return t::Status::Error(t::Err::INVALID_PARAM, "example startup already attempted");
  _started = true;
  if (_timeoutMs == 0) {
    fail(t::Status::Error(t::Err::INVALID_CONFIG, "example operation timeout is zero"));
    return _lastStatus;
  }
  _lastStatus = _device.startInitialize({nowMs, nowMs + _timeoutMs}, _token);
  if (!_lastStatus.inProgress()) fail(_lastStatus);
  return _lastStatus;
}

t::Status ExampleOwner::collect() {
  t::OperationResult result{};
  const auto status = _device.takeResult(_token, result);
  if (!status.ok()) return status;
  _token = {};
  _lastResult = result;
  _lastResultValid = true;
  _lastStatus = result.status;
  report();
  return status;
}

void ExampleOwner::tick(uint64_t nowMs, uint8_t maxTransfers) {
  if (!_started || _stopped) return;
  if (_token.valid()) {
    const auto progress = _device.progress();
    if (progress.token.value != _token.value) {
      fail(t::Status::Error(t::Err::TOKEN_MISMATCH, "example operation ownership changed"));
      return;
    }
    if (progress.state == t::OperationState::ACTIVE)
      (void)_device.poll(_token, nowMs, maxTransfers == 0 ? 0 : 1);
    if (!_device.progress().resultPending) return;
    const auto delivered = collect();
    if (!delivered.ok()) { fail(delivered); return; }
    if (!_lastStatus.ok()) {
      _stopped = true;
      _startupDone = true;
      _serviceRequested = false;
      return;
    }
    _completedAtMs = nowMs;
    if (_phase == Phase::INITIALIZE) {
      _phase = Phase::STARTUP_REFRESH;
      _lastStatus = _device.startRefreshPort({nowMs, nowMs + _timeoutMs}, _token);
      if (!_lastStatus.inProgress()) fail(_lastStatus);
      return;
    }
    _phase = Phase::READY;
    _startupDone = true;
    return;
  }
  if (_phase != Phase::READY || _periodMs == 0) return;
  // Another application owner may hold the slot; never poll, cancel or take its work.
  if (_device.progress().state != t::OperationState::IDLE) return;
  const uint64_t elapsed = nowMs - _completedAtMs;
  if (!_serviceRequested && (elapsed >= (uint64_t{1} << 63) || elapsed < _periodMs)) return;
  if (_serviceRequested) {
    _serviceRequested = false;
    _lastStatus = _device.startServiceInterrupt({nowMs, nowMs + _timeoutMs}, _token);
  } else {
    _lastStatus = _device.startRefreshPort({nowMs, nowMs + _timeoutMs}, _token);
  }
  if (!_lastStatus.inProgress()) fail(_lastStatus);
}

bool ExampleOwner::requestService() {
  if (!_startupDone || _stopped || _periodMs == 0) return false;
  _serviceRequested = true;
  return true;
}

t::Status ExampleOwner::stop() {
  _stopped = true;
  _startupDone = true;
  _serviceRequested = false;
  t::Status status{};
  const auto progress = _device.progress();
  if (_token.valid() && progress.token.value == _token.value) {
    if (progress.state == t::OperationState::ACTIVE) status = _device.cancel(_token);
    if (status.ok() && _device.progress().resultPending) status = collect();
  }
  const auto ended = _device.end();
  return status.ok() ? ended : status;
}

} // namespace tusb32x_example
