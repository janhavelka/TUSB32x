/// @file CommandTable.h
/// @brief Pure, variant-specific decoding of one supplied register byte and its raw evidence.
/// @details No device, binding, transport callback or private source include is required.
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

/// @brief Decode one caller-supplied byte without I2C, allocation or device/owner mutation.
/// @param[in] variant Explicit exact part; no model or polarity is inferred from register contents.
/// @param[in] reg Documented 00..0A, present 45 or present A0; not a target I2C address.
/// @param[in] value Unmodified raw byte, including reserved and unknown evidence.
/// @param[out] out Replaced on success with this byte's codes/validity; completely unchanged on error.
/// @retval OK Raw byte retained; reserved field codes have clear validity, not an invented value.
/// @retval INVALID_PARAM Invalid/unspecified variant, then unmapped register, in that order.
/// @retval UNSUPPORTED_FEATURE Family register absent on this part (base A0 or TUSB322I 45).
/// @note Work is a bounded table lookup and fixed branches, zero callbacks/waits or retained storage.
/// Raw direction remains CC1/CC2 even for detached 09; current/accessory/cable codes also need paired
/// port context for applicability. CC indication is neither measured current, a PD contract nor
/// permission to draw power. A reset bit is not completion proof; IRQ decoding does not acknowledge.
/// TUSB322I fault bit stays raw, with no typed fault-clear API or permission for gated 08/09 writes.
/// All returned messages have static lifetime and detail zero.
Status decodeRegister(Variant variant, uint8_t reg, uint8_t value, RegisterDecode& out);

}  // namespace TUSB32x
