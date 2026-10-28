/// @file TUSB32xCodecs.h
/// @brief Private pure field encoding, semantic comparison and qualified write composition.
/// @details No function invokes transport, allocates, retains arguments or mutates device/owner
/// state. These helpers neither perform nor authorize skipping the later owner procedures.
/// All loops traverse at most seven variant entries or four debounce choices.
#pragma once

#include "TUSB32x/CommandTable.h"

namespace TUSB32x {
/// @cond TUSB32X_INTERNAL
namespace detail {

/// @brief Interpret one complete 09/08/09 observation using supplied bytes only.
/// @param[in] variant Exact part; malformed variants return INVALID_PARAM.
/// @param[in] first09 First attachment/direction bookend.
/// @param[in] status08 Advertisement/current/accessory/cable sample.
/// @param[in] last09 Final published attachment/direction/IRQ sample.
/// @param[out] out Decoded fields/raw validity/coherence; unchanged on error. Owner provenance,
/// sequence and freshness remain default until the operation publishes the complete observation.
/// @return OK even when reserved/incoherent fields lack validity; no callbacks or policy actions.
Status decodePort(Variant variant, uint8_t first09, uint8_t status08, uint8_t last09,
                  PortSnapshot& out);

/// @brief One explicit purpose within the verified owner procedure; never a raw byte command.
enum class WriteIntent : uint8_t {
  SETTINGS,          ///< Ordinary requested fields, with no actual mode change.
  ACK_IRQ,           ///< Preserve actual 09 controls and assert only IRQ W1C.
  RESET,             ///< Preserve actual 0A controls and assert only reset.
  BASE_MODE_CHANGE,  ///< Base 320/I mode write after the detached precheck.
  MODE_DISABLE,      ///< Extended mode procedure's separate disableTerm write.
  MODE_CHANGE,       ///< Extended mode/control write after a successful separate disable.
  MODE_ENABLE,       ///< Extended procedure's separate enable after mode/wait proof.
  RECOVERY_DISABLE,  ///< Qualified original-device literal 45=04 step.
  RECOVERY_ENABLE    ///< Qualified original-device literal 45=00 step after its wait.
};

/// @brief Typed request; unrequested fields can never replace actual register controls.
struct WriteRequest {
  Variant variant = Variant::UNSPECIFIED; ///< Exact configured part; no identity inference.
  uint8_t reg = 0; ///< One writable register, not an I2C address.
  WriteIntent intent = WriteIntent::SETTINGS; ///< Explicit purpose, validated before composition.
  DeviceProfile desired{}; ///< Complete validated candidate; ignored by ACK/reset/fixed stages.
  uint16_t fields = 0; ///< Whole ProfileField bits for SETTINGS/base-mode/mode-change only.
};

/// @brief Required actual evidence supplied by the serialized owner, never staged desired state.
/// @details Flags default false. The owner must establish freshness, identity and association;
/// this pure function cannot prove them. Sequence proofs refer to this same owned procedure.
struct WriteEvidence {
  uint8_t actual = 0; ///< Observed/trusted applied byte for request.reg, including raw command bits.
  bool actualValid = false; ///< Complete byte available; every applicable field is also checked.
  /// @brief SETTINGS: all effect obligations resolved; mode: only this procedure exempted.
  /// Recovery requires no different pending reset/mode; its old recovery history and ordinary
  /// configuration uncertainty are allowed. ACK/reset ignore this flag and retain older history.
  bool effectsResolved = false;
  bool detached = false; ///< Base mode precheck and application unattached policy are satisfied.
  bool disableWritten = false; ///< This procedure's distinct disable write completed successfully.
  bool modeWritten = false; ///< This procedure's distinct mode write completed successfully.
  bool minimumWaitElapsed = false; ///< Owner proved the required TI wait plus clock margin.
  bool recoveryQualified = false; ///< Base recovery explicitly selected with stable VDD readiness.
};

/// @brief Encode a complete validated candidate's persistent fields for one register.
/// @param[in] variant Exact part; malformed variants/profiles return INVALID_CONFIG.
/// @param[in] reg One of 08/09/0A or present 45; other offsets return INVALID_PARAM.
/// @param[in] profile Complete candidate; unsupported settings/registers return UNSUPPORTED_FEATURE.
/// @param[out] out Persistent field bits only; unchanged on error. No live/reserved/command bits.
/// @return OK on representation success, with zero callbacks/waits. In particular, encoding 322I
/// 08/09 is not permission to write them. This encoder supplies no actual-state/sequence proof.
Status encodeProfileRegister(Variant variant, uint8_t reg, const DeviceProfile& profile,
                             uint8_t& out);

/// @brief Compare all persistent fields in two supplied bytes, with 322I DRP 00/11 equivalence.
/// @param[in] variant Exact part; invalid variants return INVALID_PARAM.
/// @param[in] reg Persistent register 08/09/0A or present 45; absent returns UNSUPPORTED_FEATURE.
/// @param[in] left First raw byte; live/reserved/command bits do not participate.
/// @param[in] right Second raw byte; reserved persistent codes give CONFIGURATION_UNKNOWN.
/// @param[out] equal Unchanged on error; true only when every applicable persistent field matches.
/// @return OK or the decode/access error, with zero callbacks/allocation. Equality proves no
/// identity, reset completion, write permission, protected-control qualification or port state.
Status comparePersistentSettings(Variant variant, uint8_t reg, uint8_t left, uint8_t right,
                                 bool& equal);

/// @brief Compose one safe byte from typed requested fields and proved actual controls.
/// @param[in] request Explicit intent/register/whole fields; malformed combinations return
/// INVALID_PARAM, invalid variant/candidate INVALID_CONFIG, absent/gated features UNSUPPORTED_FEATURE.
/// @param[in] evidence Required actual byte and owner stage proofs. Missing/unsafe evidence,
/// including read reset=1 or protected 45 internal bits nonzero, gives CONFIGURATION_UNKNOWN.
/// @param[out] out Composed data byte, unchanged on error; caller supplies the register pointer.
/// @return OK with no I2C, allocation, waits or state mutation. SETTINGS cannot change mode;
/// dedicated mode/recovery intents require their procedure proofs. ACK/reset ignore desired.
/// Ordinary outputs exclude IRQ/reset, RO and reserved bits; 45 preserves protected internal bits
/// after qualification. The later engine still owns timing, ordering, admission and verification.
Status composeWrite(const WriteRequest& request, const WriteEvidence& evidence, uint8_t& out);

}  // namespace detail
/// @endcond
}  // namespace TUSB32x
