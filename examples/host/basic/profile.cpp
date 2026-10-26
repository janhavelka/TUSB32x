/// @file profile.cpp
/// @brief Construct a silicon-settings candidate in a separate consumer translation unit.
#include <TUSB32x/Config.h>

#include "profile.h"

TUSB32x::DeviceProfile makeExampleProfile() {
  TUSB32x::DeviceProfile profile{};
  profile.portMode = TUSB32x::PortMode::DRP;
  profile.sourcePreference = TUSB32x::SourcePreference::TRY_SNK;
  return profile;
}
