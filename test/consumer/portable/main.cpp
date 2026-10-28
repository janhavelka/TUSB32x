/// @file main.cpp
/// @brief Standalone public owner consumer, fixed register model and armed allocation sentinels.
#include <TUSB32x/CommandTable.h>
#include <TUSB32x/TUSB32x.h>

#include "consumer.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#if defined(_WIN32)
#include <malloc.h>
#endif

namespace {
bool allocationGuard = false;
int failures = 0;
unsigned checks = 0;

// Exit without diagnostics/runtime cleanup while armed, since those can allocate.
void rejectAllocation() {
  if (allocationGuard) std::_Exit(86);
}

void* allocate(std::size_t bytes) {
  rejectAllocation();
  void* result = std::malloc(bytes == 0 ? 1 : bytes);
  if (result == nullptr) throw std::bad_alloc{};
  return result;
}

void* allocateAligned(std::size_t bytes, std::size_t alignment) {
  rejectAllocation();
#if defined(_WIN32)
  void* result = _aligned_malloc(bytes == 0 ? 1 : bytes, alignment);
#else
  void* result = nullptr;
  if (posix_memalign(&result, alignment, bytes == 0 ? 1 : bytes) != 0) result = nullptr;
#endif
  if (result == nullptr) throw std::bad_alloc{};
  return result;
}

void releaseAligned(void* pointer) noexcept {
  rejectAllocation();
#if defined(_WIN32)
  _aligned_free(pointer);
#else
  std::free(pointer);
#endif
}

void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    ++failures;
    // Leave the trap armed around core calls, but exclude test diagnostics.
    allocationGuard = false;
    std::printf("FAIL portable consumer line %d: %s\n", line, expression);
    allocationGuard = true;
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

namespace t = TUSB32x;
using Driver = t::TUSB32x;

// Explicit software model only. It implements just the registers used below;
// unknown framing/registers fail, and no production or hardware backend is stubbed.
struct Model {
  uint8_t reg08 = 0x00;
  uint8_t reg09 = 0x04;
  uint8_t reg0A = 0x30;
  uint8_t reg45 = 0;
  uint8_t address = 0x47;
  unsigned calls = 0;
  unsigned writes = 0;
  t::Status nextFailure{};

  static t::Status write(uint8_t address, const uint8_t* data, std::size_t length,
                         uint32_t timeout, void* context) {
    auto& model = *static_cast<Model*>(context);
    ++model.calls;
    if (!model.nextFailure.ok()) return model.takeFailure();
    if (address != model.address || length != 2 || timeout == 0)
      return t::Status::Error(t::Err::I2C_ERROR, "model write framing");
    ++model.writes;
    switch (data[0]) {
      case 0x08: model.reg08 = static_cast<uint8_t>((model.reg08 & 0x3FU) | data[1]); break;
      case 0x09:
        model.reg09 = static_cast<uint8_t>((model.reg09 & 0xF8U) | (data[1] & 0x07U));
        if ((data[1] & 0x10U) != 0) model.reg09 &= 0xEF;
        break;
      case 0x0A:
        // Command auto-clears; the owner still must observe the complete reset wait.
        model.reg0A = static_cast<uint8_t>(data[1] & 0xF7U);
        break;
      case 0x45: model.reg45 = data[1]; break;
      default: return t::Status::Error(t::Err::I2C_ERROR, "model unmapped write");
    }
    return t::Status::Ok();
  }

  static t::Status read(uint8_t address, const uint8_t* tx, std::size_t txLength,
                        uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* context) {
    auto& model = *static_cast<Model*>(context);
    ++model.calls;
    if (!model.nextFailure.ok()) return model.takeFailure();
    if (address != model.address || txLength != 1 || timeout == 0)
      return t::Status::Error(t::Err::I2C_ERROR, "model read framing");
    constexpr uint8_t ID[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
    if (tx[0] == 0 && rxLength == sizeof(ID)) {
      std::memcpy(rx, ID, sizeof(ID));
      return t::Status::Ok();
    }
    if (rxLength != 1) return t::Status::Error(t::Err::I2C_ERROR, "model read length");
    switch (tx[0]) {
      case 0: rx[0] = ID[0]; break;
      case 0x08: rx[0] = model.reg08; break;
      case 0x09: rx[0] = model.reg09; break;
      case 0x0A: rx[0] = model.reg0A; break;
      case 0x45: rx[0] = model.reg45; break;
      case 0xA0: rx[0] = 2; break;
      default: return t::Status::Error(t::Err::I2C_ERROR, "model unmapped read");
    }
    return t::Status::Ok();
  }

  t::Status takeFailure() {
    const auto status = nextFailure;
    nextFailure = {};
    return status;
  }
};

t::DriverConfig configuration(Model& model, bool base = false) {
  t::DriverConfig config{};
  config.i2cWrite = Model::write;
  config.i2cWriteRead = Model::read;
  config.i2cUser = &model;
  config.variant = base ? t::Variant::TUSB320 : t::Variant::TUSB320LAI;
  config.address = base ? t::DeviceAddress::ADDR_LOW_0X60 : t::DeviceAddress::ADDR_LOW_0X47;
  model.address = static_cast<uint8_t>(config.address);
  return config;
}

// Bound the consumer scheduler independently of the core. Every callback is
// followed by another owner sample; each poll receives the next owner tick.
t::OperationResult complete(Driver& driver, Model& model, t::OperationToken token, uint64_t& now) {
  t::Status status{t::Err::IN_PROGRESS, 0, "consumer active"};
  for (unsigned turn = 0; turn < 160 && status.inProgress(); ++turn) {
    const auto before = model.calls;
    status = driver.poll(token, now++, 8);
    CHECK(model.calls - before <= 1);
  }
  CHECK(status.ok());
  t::OperationResult result{};
  CHECK(driver.resultSnapshot(token, result).ok());
  CHECK(result.token.value == token.value && result.status.ok());
  CHECK(driver.takeResult(token, result).ok());
  CHECK(driver.progress().state == t::OperationState::IDLE);
  return result;
}

using Start = t::Status (Driver::*)(const t::OperationTiming&, t::OperationToken&);
t::OperationResult run(Driver& driver, Model& model, uint64_t& now, Start start,
                       t::OperationKind kind) {
  t::OperationToken token{};
  const auto before = model.calls;
  CHECK((driver.*start)({now, now + 1000}, token).inProgress());
  CHECK(token.valid() && model.calls == before);
  const auto result = complete(driver, model, token, now);
  CHECK(result.kind == kind);
  return result;
}

void exercise() {
  Model model{};
  Driver driver{};
  auto config = configuration(model);
  uint64_t now = 100;
  CHECK(t::validateDriverConfig(config).ok());
  const auto invalidBinding = t::validateDriverConfig({});
  const char* retainedMessage = invalidBinding.msg;
  CHECK(invalidBinding.code == t::Err::INVALID_CONFIG && retainedMessage[0] != '\0');
  char savedMessage[128]{};
  const auto messageLength = std::strlen(retainedMessage);
  CHECK(messageLength < sizeof(savedMessage));
  if (messageLength < sizeof(savedMessage)) std::memcpy(savedMessage, retainedMessage, messageLength);
  CHECK(driver.bind(config).ok() && model.calls == 0);
  run(driver, model, now, &Driver::startInitialize, t::OperationKind::INITIALIZE);
  CHECK(driver.configurationSnapshot().state == t::ConfigurationState::KNOWN);
  CHECK(model.writes == 0);
  CHECK(inspectFromOtherTranslationUnit(driver));

  t::OperationResult preserved{};
  preserved.token.value = 991;
  preserved.status = {t::Err::I2C_BUS, -70000, "caller result sentinel"};
  unsigned char original[sizeof(preserved)];
  std::memcpy(original, &preserved, sizeof(preserved));
  CHECK(driver.resultSnapshot({}, preserved).code == t::Err::INVALID_PARAM);
  CHECK(std::memcmp(original, &preserved, sizeof(preserved)) == 0);
  CHECK(driver.takeResult({}, preserved).code == t::Err::INVALID_PARAM);
  CHECK(std::memcmp(original, &preserved, sizeof(preserved)) == 0);
  uint8_t raw = 0xA5;
  CHECK(driver.readRegisterSync(0x7F, raw).code == t::Err::INVALID_PARAM && raw == 0xA5);
  t::RegisterDecode decoded{};
  decoded.value = 0xD3;
  unsigned char originalDecode[sizeof(decoded)];
  std::memcpy(originalDecode, &decoded, sizeof(decoded));
  CHECK(t::decodeRegister(t::Variant::UNSPECIFIED, 8, 0, decoded).code == t::Err::INVALID_PARAM);
  CHECK(std::memcmp(originalDecode, &decoded, sizeof(decoded)) == 0);
  t::HealthSnapshot health{};
  health.offlineThreshold = 123;
  unsigned char originalHealth[sizeof(health)];
  std::memcpy(originalHealth, &health, sizeof(health));
  CHECK(t::projectHealth({}, {0}, health).code == t::Err::INVALID_CONFIG);
  CHECK(std::memcmp(originalHealth, &health, sizeof(health)) == 0);

  run(driver, model, now, &Driver::startReadProfile, t::OperationKind::READ_PROFILE);
  t::DeviceProfile profile = driver.configurationSnapshot().observedProfile;
  profile.sourceCurrent = t::SourceCurrent::MA_1500;
  t::OperationToken token{};
  CHECK(driver.startApplyProfile(profile, {now, now + 1000}, token).inProgress());
  CHECK(complete(driver, model, token, now).hardwareEffect == t::HardwareEffect::CONFIRMED);
  CHECK(model.reg08 == 0x40 && model.writes == 1);
  // Also exercise the extended role-change write/wait path through the public API.
  profile.portMode = t::PortMode::DFP;
  CHECK(driver.startApplyProfile(profile, {now, now + 1000}, token).inProgress());
  CHECK(complete(driver, model, token, now).hardwareEffect == t::HardwareEffect::CONFIRMED);
  CHECK((model.reg0A & 0x31U) == 0x20);
  run(driver, model, now, &Driver::startVerifyProfile, t::OperationKind::VERIFY_PROFILE);
  run(driver, model, now, &Driver::startReconcile, t::OperationKind::RECONCILE);
  model.reg09 |= 0x50; // Detached controls become attached local source with IRQ pending.
  run(driver, model, now, &Driver::startRefreshPort, t::OperationKind::REFRESH_PORT);
  CHECK(driver.portSnapshot().attachment == t::Attachment::LOCAL_SOURCE);
  run(driver, model, now, &Driver::startAcknowledgeInterrupt,
      t::OperationKind::ACKNOWLEDGE_INTERRUPT);
  CHECK((model.reg09 & 0x10U) == 0);
  model.reg09 |= 0x10;
  run(driver, model, now, &Driver::startServiceInterrupt, t::OperationKind::SERVICE_INTERRUPT);
  CHECK(driver.diagnostics().interrupt.pendingAfterValid);
  CHECK(!driver.diagnostics().interrupt.pendingAfter);
  run(driver, model, now, &Driver::startReset, t::OperationKind::RESET);
  CHECK(driver.configurationSnapshot().resetState == t::ResetState::COMPLETED);
  CHECK(driver.readRegisterSync(0x08, raw).ok() && raw == model.reg08);
  CHECK(driver.writeRegisterSync(0x09, static_cast<uint8_t>((model.reg09 & 7U) | 0x10U)).ok());

  // A transport failure is preserved through retained delivery, with full int32 detail.
  model.nextFailure = {t::Err::I2C_BUS, std::numeric_limits<int32_t>::min(), "static bus fault"};
  const auto managedBefore = driver.diagnostics().managedTransport.totalFailures;
  raw = 0xA5;
  CHECK(driver.readRegisterSync(0x08, raw).detail == std::numeric_limits<int32_t>::min());
  CHECK(raw == 0xA5);
  CHECK(driver.diagnostics().managedTransport.totalFailures == managedBefore);
  CHECK(driver.startReadProfile({now, now + 1000}, token).inProgress());
  model.nextFailure = {t::Err::I2C_TIMEOUT, std::numeric_limits<int32_t>::max(), "static timeout"};
  CHECK(driver.poll(token, now++).code == t::Err::I2C_TIMEOUT);
  CHECK(driver.takeResult(token, preserved).ok());
  CHECK(preserved.status.detail == std::numeric_limits<int32_t>::max());
  const char* failureMessage = preserved.status.msg;
  CHECK(std::strcmp(failureMessage, "static timeout") == 0);
  CHECK(t::projectHealth(driver.diagnostics().managedTransport, {1}, health).ok());
  CHECK(health.state == t::DriverState::OFFLINE);
  run(driver, model, now, &Driver::startRefreshPort, t::OperationKind::REFRESH_PORT);

  const auto beforeSilent = model.calls;
  CHECK(driver.startReadProfile({now, now + 1000}, token).inProgress());
  t::OperationToken unchanged{812};
  CHECK(driver.startReadProfile({now, now + 1000}, unchanged).code == t::Err::BUSY);
  CHECK(unchanged.value == 812);
  CHECK(driver.poll(token, now, 0).inProgress());
  CHECK(driver.cancel(token).ok());
  CHECK(driver.takeResult(token, preserved).ok() && preserved.status.code == t::Err::CANCELLED);
  CHECK(driver.invalidateDeviceState(t::Status::Error(t::Err::CONFIGURATION_UNKNOWN,
                                                    "external change")).ok());
  driver.clearTransportStats();
  CHECK(driver.transportStats().totalAttempts == 0);
  CHECK(driver.end().ok());
  CHECK(driver.forgetDeviceEvidence().ok());
  CHECK(model.calls == beforeSilent);
  CHECK(inspectFromOtherTranslationUnit(driver));
  CHECK(std::strcmp(retainedMessage, savedMessage) == 0);
  CHECK(std::strcmp(failureMessage, "static timeout") == 0);

  // Base recovery is a different exact variant and requires its own physical target model.
  Model base{};
  Driver originalDevice{};
  config = configuration(base, true);
  CHECK(originalDevice.bind(config).ok());
  run(originalDevice, base, now, &Driver::startInitialize, t::OperationKind::INITIALIZE);
  CHECK(run(originalDevice, base, now, &Driver::startRecoverDeadBattery,
            t::OperationKind::RECOVER_DEAD_BATTERY).hardwareEffect == t::HardwareEffect::CONFIRMED);
  CHECK(base.reg45 == 0 && base.writes == 2);
  CHECK(originalDevice.end().ok());
}
} // namespace

// All replaceable C++17 allocating/deallocating forms are armed around exercised core work.
void* operator new(std::size_t bytes) { return allocate(bytes); }
void* operator new[](std::size_t bytes) { return allocate(bytes); }
void operator delete(void* pointer) noexcept { rejectAllocation(); std::free(pointer); }
void operator delete[](void* pointer) noexcept { rejectAllocation(); std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { ::operator delete(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { ::operator delete[](pointer); }
void* operator new(std::size_t bytes, const std::nothrow_t&) noexcept {
  try { return allocate(bytes); } catch (...) { return nullptr; }
}
void* operator new[](std::size_t bytes, const std::nothrow_t& tag) noexcept {
  return ::operator new(bytes, tag);
}
void operator delete(void* pointer, const std::nothrow_t&) noexcept { ::operator delete(pointer); }
void operator delete[](void* pointer, const std::nothrow_t&) noexcept { ::operator delete[](pointer); }
void* operator new(std::size_t bytes, std::align_val_t alignment) {
  return allocateAligned(bytes, static_cast<std::size_t>(alignment));
}
void* operator new[](std::size_t bytes, std::align_val_t alignment) {
  return ::operator new(bytes, alignment);
}
void operator delete(void* pointer, std::align_val_t) noexcept { releaseAligned(pointer); }
void operator delete[](void* pointer, std::align_val_t) noexcept { releaseAligned(pointer); }
void operator delete(void* pointer, std::size_t, std::align_val_t) noexcept { releaseAligned(pointer); }
void operator delete[](void* pointer, std::size_t, std::align_val_t) noexcept { releaseAligned(pointer); }
void* operator new(std::size_t bytes, std::align_val_t alignment, const std::nothrow_t&) noexcept {
  try { return allocateAligned(bytes, static_cast<std::size_t>(alignment)); }
  catch (...) { return nullptr; }
}
void* operator new[](std::size_t bytes, std::align_val_t alignment,
                     const std::nothrow_t& tag) noexcept {
  return ::operator new(bytes, alignment, tag);
}
void operator delete(void* pointer, std::align_val_t, const std::nothrow_t&) noexcept {
  releaseAligned(pointer);
}
void operator delete[](void* pointer, std::align_val_t, const std::nothrow_t&) noexcept {
  releaseAligned(pointer);
}

#if defined(TUSB32X_CONSUMER_C_WRAP)
extern "C" {
void* __real_malloc(std::size_t);
void* __real_calloc(std::size_t, std::size_t);
void* __real_realloc(void*, std::size_t);
void __real_free(void*);
void* __wrap_malloc(std::size_t bytes) { rejectAllocation(); return __real_malloc(bytes); }
void* __wrap_calloc(std::size_t count, std::size_t bytes) {
  rejectAllocation(); return __real_calloc(count, bytes);
}
void* __wrap_realloc(void* pointer, std::size_t bytes) {
  rejectAllocation(); return __real_realloc(pointer, bytes);
}
void __wrap_free(void* pointer) { rejectAllocation(); __real_free(pointer); }
}
#endif

/// @brief Run fixed public scenarios or a deliberate failure sentinel.
/// @param[in] argc Process argument count; accepts zero or one test mode argument.
/// @param[in] argv --fail-check returns one; allocation probes return 86 when trapped.
/// @return Zero for successful checks, one for failures/invalid arguments, 77 for absent C wrapping.
int main(int argc, char** argv) {
  if (argc == 2 && std::strcmp(argv[1], "--fail-check") == 0) {
    CHECK(false);
    return failures == 0 ? 0 : 1;
  }
  if (argc == 2 && std::strcmp(argv[1], "--allocate-probe") == 0) {
    allocationGuard = true;
    void* (*volatile allocateFunction)(std::size_t) = ::operator new;
    static_cast<void>(allocateFunction(1));
    return 1;
  }
  if (argc == 2 && std::strcmp(argv[1], "--malloc-probe") == 0) {
#if defined(TUSB32X_CONSUMER_C_WRAP)
    allocationGuard = true;
    void* (*volatile allocateFunction)(std::size_t) = std::malloc;
    static_cast<void>(allocateFunction(1));
    return 1;
#else
    return 77;
#endif
  }
  if (argc != 1) return 1;
  allocationGuard = true;
  exercise();
  allocationGuard = false;
  std::printf("Portable consumer: %u checks, %d failures; C allocation wrapping: %s\n",
              checks, failures,
#if defined(TUSB32X_CONSUMER_C_WRAP)
              "enabled"
#else
              "unavailable (C++ allocation trap active)"
#endif
  );
  return failures == 0 ? 0 : 1;
}
