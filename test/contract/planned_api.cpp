// Compile-only signature inventory. No driver is constructed or linked here.
#include "PlannedApi.h"

#include <type_traits>

using Driver = TUSB32x::planned::Driver;
using ImplementedDriver = TUSB32x::TUSB32x;
using Start = TUSB32x::Status (Driver::*)(const TUSB32x::OperationTiming&,
                                       TUSB32x::OperationToken&);
static_assert(!std::is_copy_constructible_v<Driver>);
static_assert(!std::is_copy_assignable_v<Driver>);
static_assert(!std::is_move_constructible_v<Driver>);
static_assert(!std::is_move_assignable_v<Driver>);
static_assert(std::is_same_v<decltype(&ImplementedDriver::startInitialize),
              TUSB32x::Status (ImplementedDriver::*)(const TUSB32x::OperationTiming&,
                                                   TUSB32x::OperationToken&)>);
static_assert(std::is_same_v<decltype(&Driver::startReadProfile), Start>);
static_assert(std::is_same_v<decltype(&Driver::startVerifyProfile), Start>);
static_assert(std::is_same_v<decltype(&Driver::startReconcile), Start>);
static_assert(std::is_same_v<decltype(&Driver::startRefreshPort), Start>);
static_assert(std::is_same_v<decltype(&Driver::startAcknowledgeInterrupt), Start>);
static_assert(std::is_same_v<decltype(&Driver::startServiceInterrupt), Start>);
static_assert(std::is_same_v<decltype(&Driver::startReset), Start>);
static_assert(std::is_same_v<decltype(&Driver::startRecoverDeadBattery), Start>);

// Type-check every remaining planned method and the default poll budget. These
// calls generate no runtime claims: the file is checked with -fsyntax-only.
void checkPlannedSignatures(Driver& driver, const Driver& cached,
                            const TUSB32x::DriverConfig& config,
                            const TUSB32x::DeviceProfile& profile,
                            const TUSB32x::OperationTiming& timing,
                            TUSB32x::OperationToken& token,
                            TUSB32x::OperationResult& result,
                            TUSB32x::RegisterDecode& decoded,
                            const TUSB32x::HealthPolicy& policy,
                            const TUSB32x::ManagedTransportEvidence& evidence,
                            TUSB32x::HealthSnapshot& health, uint8_t& byte) {
  (void)TUSB32x::validateDriverConfig(config);
  (void)TUSB32x::validateProfile(config.variant, profile);
  (void)TUSB32x::validateHealthPolicy(policy);
  (void)TUSB32x::projectHealth(evidence, policy, health);
  (void)TUSB32x::decodeRegister(config.variant, 0x08, byte, decoded);
  (void)driver.bind(config);
  (void)driver.end();
  (void)driver.invalidateDeviceState(TUSB32x::Status::Error(
      TUSB32x::Err::CONFIGURATION_UNKNOWN, "external change"));
  (void)driver.forgetDeviceEvidence();
  (void)driver.startApplyProfile(profile, timing, token);
  (void)driver.poll(token, timing.nowMs);
  (void)driver.poll(token, timing.nowMs, 0);
  (void)driver.cancel(token);
  (void)cached.resultSnapshot(token, result);
  (void)driver.takeResult(token, result);
  (void)cached.progress();
  (void)cached.portSnapshot();
  (void)cached.configurationSnapshot();
  (void)cached.transportStats();
  (void)cached.diagnostics();
  driver.clearTransportStats();
  (void)driver.readRegisterSync(0x08, byte);
  (void)driver.writeRegisterSync(0x08, byte);
}
