/// @file secondary.cpp
/// @brief Public helper and facade linkage independent of the main translation unit.
#include <TUSB32x/CommandTable.h>
#include <TUSB32x/Config.h>
#include <TUSB32x/Status.h>
#include <TUSB32x/TUSB32x.h>
#include <TUSB32x/Version.h>

#include "consumer.h"

#include <cstring>

bool inspectFromOtherTranslationUnit(const TUSB32x::TUSB32x& driver) {
  namespace t = TUSB32x;
  const t::Status ok = t::Status::Ok();
  const t::Status error = t::Status::Error(t::Err::I2C_BUS, "static consumer message", -70000);
  const t::Status progress{t::Err::IN_PROGRESS, 0, "static progress"};
  t::RegisterDecode decoded{};
  t::HealthSnapshot health{};
  const auto diagnostics = driver.diagnostics();
  static_cast<void>(t::VERSION_MAJOR);
  static_cast<void>(t::VERSION_MINOR);
  static_cast<void>(t::VERSION_PATCH);
  return ok.ok() && static_cast<bool>(ok) && !static_cast<bool>(error) &&
      error.is(t::Err::I2C_BUS) && error.detail == -70000 && progress.inProgress() &&
      std::strcmp(t::errorName(static_cast<t::Err>(255)), "UNKNOWN") == 0 &&
      t::VERSION[0] != '\0' &&
      t::validateProfile(t::Variant::TUSB320LAI, {}).ok() &&
      t::validateHealthPolicy({}).ok() &&
      t::decodeRegister(t::Variant::TUSB320LAI, 0x08, 0x80, decoded).ok() &&
      decoded.profileCodes.sourceCurrent == t::SourceCurrent::MA_3000 &&
      t::projectHealth(diagnostics.managedTransport, {}, health).ok() &&
      driver.progress().state == t::OperationState::IDLE &&
      driver.transportStats().totalAttempts >= driver.transportStats().totalFailures &&
      driver.configurationSnapshot().state == diagnostics.configurationState &&
      driver.portSnapshot().rawValidMask <= (t::STATUS_08_VALID | t::STATUS_09_VALID);
}
