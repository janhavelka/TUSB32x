/**
 * @file PlannedApi.h
 * @brief Compile-only sketch of future callable contracts; never a delivered header.
 *
 * Methods declared here remain unimplemented; inherited owner methods are real.
 * Compile future consumers without linking them. Runtime suites separately call
 * the delivered facade. Do not expose this directory to consumers or packages.
 */
#pragma once

#include <TUSB32x/CommandTable.h>
#include <TUSB32x/Config.h>
#include <TUSB32x/Status.h>
#include <TUSB32x/TUSB32x.h>

namespace TUSB32x {

// Validators and decoder are supplied by public headers. Health validation/projection: 17.
Status validateHealthPolicy(const HealthPolicy& policy);
Status projectHealth(const ManagedTransportEvidence& evidence,
                     const HealthPolicy& policy, HealthSnapshot& out);

namespace planned {

// Future families inherit the delivered owner API. This distinct fixture type
// cannot redefine the public facade or claim implementations for future jobs.
class Driver : public ::TUSB32x::TUSB32x {
 public:
  Status readRegisterSync(uint8_t reg, uint8_t& out);
  Status writeRegisterSync(uint8_t reg, uint8_t value);
};

}  // namespace planned

}  // namespace TUSB32x
