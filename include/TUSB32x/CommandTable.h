/// @file CommandTable.h
/// @brief Fixed passive result contract for future pure decoding of a supplied register byte.
/// @details The decoder and verified register-table implementation are planned for prompt 10.
/// This header publishes no unimplemented callable declaration or register encoder.
#pragma once

#include <cstdint>

#include "TUSB32x/TUSB32x.h"

namespace TUSB32x {

/// @brief Supplied raw register byte plus independently valid field codes, without port context.
/// @details This is not a port snapshot or configuration proof. Validity establishes only a documented
/// field code for the selected variant. Reserved/unsupported field encodings keep their validity bit
/// clear and retain the raw byte. Default typed values are unusable placeholders until flagged.
/// Remote-current, accessory and active-cable applicability requires a complete paired port
/// observation. No supplied-byte decode establishes local/peer role, time, attachment coherence,
/// reset completion, IRQ acknowledgement, advertised-power authorization or device identity.
struct RegisterDecode {
  Variant variant = Variant::UNSPECIFIED; ///< Explicit variant used to interpret supplied codes.
  uint8_t reg = 0; ///< Supplied documented register address, not a target I2C address.
  uint8_t value = 0; ///< Unmodified supplied byte, including unknown/reserved evidence.
  DeviceProfile profileCodes{}; ///< Supplied-byte setting codes, only valid under profileValidFields.
  uint16_t profileValidFields = 0; ///< ProfileField bits for documented codes available in this byte.
  Attachment attachment = Attachment::UNKNOWN; ///< Attachment code; not a complete port observation.
  Orientation orientation = Orientation::UNKNOWN; ///< Raw direction code without paired applicability.
  DetectedCurrent detectedCurrent = DetectedCurrent::UNKNOWN; ///< Remote code without applicability.
  Accessory accessory = Accessory::UNKNOWN; ///< Accessory code without paired applicability.
  bool activeCable = false; ///< Active-cable code only; invalid until its status field bit is set.
  bool interruptPending = false; ///< Supplied latch bit only; no acknowledgement or later-state proof.
  uint16_t statusValidFields = 0; ///< PortField code bits; LOCAL_ROLE/PEER_ROLE/ADVERTISED_CURRENT clear.
  bool resetRequested = false; ///< Supplied 0x0A command bit, only when resetValid; not effect proof.
  bool resetValid = false; ///< Register/variant defines the supplied reset-command bit.
};

}  // namespace TUSB32x
