/**
 * @file PlannedApi.h
 * @brief Compile-only sketch of future callable contracts; never a delivered header.
 *
 * No method here has an implementation. Compile its consumer without linking it.
 * Replace this sketch with real public API checks as the owning prompts implement
 * each method. Do not include this directory in library include paths or packages.
 */
#pragma once

#include <TUSB32x/CommandTable.h>
#include <TUSB32x/Config.h>
#include <TUSB32x/Status.h>
#include <TUSB32x/TUSB32x.h>

namespace TUSB32x {

// Binding/profile validators: prompt 7. Health validation/projection: 17. Decoder: 10.
Status validateDriverConfig(const DriverConfig& config);
Status validateProfile(Variant variant, const DeviceProfile& profile);
Status validateHealthPolicy(const HealthPolicy& policy);
Status projectHealth(const ManagedTransportEvidence& evidence,
                     const HealthPolicy& policy, HealthSnapshot& out);
Status decodeRegister(Variant variant, uint8_t reg, uint8_t value, RegisterDecode& out);

// Planned facade only. There are deliberately no successful or unsupported stubs.
class TUSB32x {
 public:
  TUSB32x();
  ~TUSB32x();
  TUSB32x(const TUSB32x&) = delete;
  TUSB32x& operator=(const TUSB32x&) = delete;
  TUSB32x(TUSB32x&&) = delete;
  TUSB32x& operator=(TUSB32x&&) = delete;

  Status bind(const DriverConfig& config);
  Status end();
  Status invalidateDeviceState(const Status& reason);
  Status forgetDeviceEvidence();

  Status startInitialize(const OperationTiming& timing, OperationToken& token);
  Status startReadProfile(const OperationTiming& timing, OperationToken& token);
  Status startApplyProfile(const DeviceProfile& profile,
                          const OperationTiming& timing, OperationToken& token);
  Status startVerifyProfile(const OperationTiming& timing, OperationToken& token);
  Status startReconcile(const OperationTiming& timing, OperationToken& token);
  Status startRefreshPort(const OperationTiming& timing, OperationToken& token);
  Status startAcknowledgeInterrupt(const OperationTiming& timing, OperationToken& token);
  Status startServiceInterrupt(const OperationTiming& timing, OperationToken& token);
  Status startReset(const OperationTiming& timing, OperationToken& token);
  Status startRecoverDeadBattery(const OperationTiming& timing, OperationToken& token);
  Status poll(OperationToken token, uint64_t nowMs, uint8_t maxTransfers = 1);
  Status cancel(OperationToken token);
  Status resultSnapshot(OperationToken token, OperationResult& out) const;
  Status takeResult(OperationToken token, OperationResult& out);

  OperationProgress progress() const;
  PortSnapshot portSnapshot() const;
  ConfigurationSnapshot configurationSnapshot() const;
  TransportStats transportStats() const;
  DriverDiagnostics diagnostics() const;
  void clearTransportStats();

  Status readRegisterSync(uint8_t reg, uint8_t& out);
  Status writeRegisterSync(uint8_t reg, uint8_t value);
};

}  // namespace TUSB32x
