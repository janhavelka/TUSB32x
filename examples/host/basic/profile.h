/// @file profile.h
/// @brief Shared candidate declaration for the two-translation-unit host consumer.
#pragma once

#include <TUSB32x/Config.h>

/// @brief Construct candidate settings without observing or changing hardware.
/// @return A semantically valid TUSB320LAI candidate; not an observed or applied profile.
TUSB32x::DeviceProfile makeExampleProfile();
