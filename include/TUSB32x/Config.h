/// @file Config.h
/// @brief Borrowed terminal transport callbacks, explicit target binding and silicon settings.
/// @details Semantic fields follow TI SLLSEN9F Table 7-7 pp16-19, SLLSEP2D/SLLSEQ8D
/// Tables 7-12 pp16-20 and SLLSEO5D Tables 6-7 through 6-11 pp17-20. Exact encoding,
/// capability and write-procedure limits are recorded in docs/register-map.md and
/// docs/device-behavior.md; these types do not validate or encode their own values.
#pragma once

#include <cstddef>
#include <cstdint>

#include "TUSB32x/Status.h"

namespace TUSB32x {

/// @brief One terminal, synchronous write callback; no retained buffers or instance reentry.
/// @param[in] address Seven-bit target address; adapter supplies the direction bit.
/// @param[in] data Fixed driver-owned bytes, including the register pointer; borrowed until return.
/// @param[in] length Number of bytes to transmit.
/// @param[in] timeoutMs Positive aggregate millisecond budget, including lock acquisition and cleanup.
/// @param[in] user Borrowed application context; null is permitted when unused.
/// @return OK only for complete success; otherwise a documented terminal I2C_* status with full detail.
/// @note Application owns the bus and serialization across every user. No core/adapter-added retry or
/// recovery is allowed. SDK internal waits/cleanup require an independent bound audit. No asynchronous
/// work may outlive return. Owner/validation codes and invalid enums are normalized to I2C_ERROR.
/// Status does not report elapsed time; the owner must obtain a fresh time sample after return.
/// Returned Status.msg must have static lifetime because results and diagnostics retain it.
using I2cWriteFn = Status (*)(uint8_t address, const uint8_t* data, size_t length,
                             uint32_t timeoutMs, void* user);

/// @brief One terminal pointer-write/read callback under one aggregate timeout and bus lock.
/// @param[in] address Seven-bit target address; adapter supplies direction bits.
/// @param[in] txData Driver-owned pointer bytes, borrowed only until terminal return.
/// @param[in] txLength Number of pointer bytes to transmit.
/// @param[out] rxData Driver-owned scratch buffer; only complete success permits publication.
/// @param[in] rxLength Exact receive count; a short read is failure.
/// @param[in] timeoutMs Positive aggregate milliseconds for lock, both phases, delays and cleanup.
/// @param[in] user Borrowed application context; null is permitted when unused.
/// @return OK only for both complete phases; otherwise a terminal I2C_* status with full detail.
/// @note Status carries no received byte count. The adapter must detect a short read and return
/// failure; the core cannot detect an incomplete receive falsely reported as OK.
/// @note Selected framing is pointer-write STOP then read, with serialization across the STOP.
/// Repeated START needs explicit adapter/part evidence. No core/adapter-added retry or recovery,
/// retained buffers, asynchronous completion or instance reentry. SDK internal waits/cleanup require
/// an independent bound audit. Status does not report elapsed time; the owner samples freshly after
/// return. A timeout parameter alone does not prove an SDK bound.
/// Returned Status.msg must have static lifetime because results and diagnostics retain it.
using I2cWriteReadFn = Status (*)(uint8_t address, const uint8_t* txData, size_t txLength,
                                 uint8_t* rxData, size_t rxLength, uint32_t timeoutMs, void* user);

/// @brief Exact application-selected silicon; an identity read never guesses the variant.
enum class Variant : uint8_t {
  UNSPECIFIED = 0, ///< Invalid binding sentinel; no part selected.
  TUSB320,        ///< Base commercial-temperature device; addresses 0x60/0x61.
  TUSB320I,       ///< Base industrial-temperature device; addresses 0x60/0x61.
  TUSB320LI,      ///< Extended active-low-enable device; revision 0x01.
  TUSB320HI,      ///< Extended active-high-enable device; revision 0x01.
  TUSB320LAI,     ///< Primary active-low-enable device; revision 0x02.
  TUSB320HAI,     ///< Active-high-enable counterpart; revision 0x02.
  TUSB322I        ///< Distinct 322 identity/map; unresolved writes remain capability-gated.
};

/// @brief Verified strap-selected seven-bit I2C addresses; floating ADDR is outside I2C use.
enum class DeviceAddress : uint8_t {
  ADDR_LOW_0X60 = 0x60,  ///< ADDR low on base TUSB320/TUSB320I.
  ADDR_HIGH_0X61 = 0x61, ///< ADDR high on base TUSB320/TUSB320I.
  ADDR_LOW_0X47 = 0x47,  ///< ADDR low on LI/HI/LAI/HAI/322I.
  ADDR_HIGH_0X67 = 0x67  ///< ADDR high on LI/HI/LAI/HAI/322I.
};

/// @brief Application-owned terminal transport binding; default is invalid until callbacks/part set.
/// @details Accepted binding copies this value and borrows i2cUser until end returns. No pins, clocks,
/// locks, power policy or bus ownership are transferred. Callbacks run only in a serialized owner
/// context, never an ISR; do not reenter the instance. Binding itself performs no I2C or settings write.
/// Target association includes variant, address, context and both callbacks; equal pointer values do
/// not prove physical identity. After end the association is retained without invoking the old bus.
struct DriverConfig {
  I2cWriteFn i2cWrite = nullptr;         ///< Required terminal write callback.
  I2cWriteReadFn i2cWriteRead = nullptr; ///< Required terminal pointer-write/read callback.
  void* i2cUser = nullptr;              ///< Borrowed context, forwarded unchanged; null may be valid.
  Variant variant = Variant::UNSPECIFIED; ///< Explicit exact part; UNSPECIFIED is invalid.
  DeviceAddress address = DeviceAddress::ADDR_LOW_0X47; ///< Strap address; must match selected part.
  uint32_t i2cTimeoutMs = 20; ///< Positive per-callback cap in milliseconds; not a TI timing constant.
};

/// @brief Requested local role mode; ordinals are not register encodings.
enum class PortMode : uint8_t {
  FOLLOW_PORT_PIN, ///< Follow sampled PORT strap on six 320 parts; unsupported on 322I.
  UFP,             ///< Request sink/UFP mode.
  DFP,             ///< Request source/DFP mode.
  DRP              ///< Request dual-role mode; two 322I encodings are semantically equivalent.
};

/// @brief CC advertisement category; neither measured load nor authorization to supply power.
enum class SourceCurrent : uint8_t {
  DEFAULT, ///< Default USB current advertisement; interpretation requires USB/board policy.
  MA_1500, ///< 1.5 A advertisement; board supply remains application-owned.
  MA_3000  ///< 3 A advertisement; exact-part voltage and board prerequisites still apply.
};

/// @brief Requested dual-role preference; supported combinations depend on the exact variant.
enum class SourcePreference : uint8_t {
  NORMAL,  ///< No Try preference; neutral value on variants without this control.
  TRY_SNK, ///< Prefer sink using the documented Try.SNK control.
  TRY_SRC  ///< Prefer source using the documented Try.SRC control.
};

/// @brief Semantic source share of the DRP cycle; use a variant-specific encoding table.
enum class DrpDutyCycle : uint8_t {
  PERCENT_30, ///< 30 percent source duty.
  PERCENT_40, ///< 40 percent source duty.
  PERCENT_50, ///< 50 percent source duty; selected candidate default.
  PERCENT_60  ///< 60 percent source duty.
};

/// @brief Nominal CC debounce settings in milliseconds; variant/value support must be validated.
enum class CcDebounce : uint8_t {
  MS_116, ///< 116 ms setting on its documented variants.
  MS_118, ///< 118 ms setting on its documented variants.
  MS_133, ///< 133 ms setting on its documented variants.
  MS_134, ///< 134 ms setting on its documented variants.
  MS_151, ///< 151 ms setting on its documented variants.
  MS_152, ///< 152 ms setting on its documented variants.
  MS_168  ///< 168 ms setting; selected candidate default, not a universal reset claim.
};

/// @brief Candidate silicon settings, separate from transport and all application policy.
/// @details Defaults do not initialize hardware, establish observed values, or authorize power.
/// Only an accepted explicit apply may write a validated candidate. Unsupported neutral placeholders
/// never authorize access to an absent register. Enum ordinals are not register bit patterns.
struct DeviceProfile {
  PortMode portMode = PortMode::DRP; ///< Requested local role mode.
  SourceCurrent sourceCurrent = SourceCurrent::DEFAULT; ///< Requested CC advertisement category.
  SourcePreference sourcePreference = SourcePreference::NORMAL; ///< Requested Try preference.
  DrpDutyCycle drpDutyCycle = DrpDutyCycle::PERCENT_50; ///< Requested source share of the DRP cycle.
  CcDebounce ccDebounce = CcDebounce::MS_168; ///< Requested nominal debounce in milliseconds.
  bool disableUfpAccessory = false; ///< Disable documented UFP accessory support where present.
  bool disableTerm = false; ///< Independent 0x0A state-machine/termination disable control.
  bool disableRdRp = false; ///< Independent 0x45 Rd/Rp disable control; absent on 322I.
};

/// @brief Check a complete binding value without I2C, allocation or mutation.
/// @param[in] config Candidate binding, borrowed only for this call. Both callbacks and an explicit
/// variant/address pair are required; i2cUser may be null and i2cTimeoutMs must be positive milliseconds.
/// @retval OK All binding fields are valid; no device identity or backend timing is proved.
/// @retval INVALID_CONFIG Missing callback, zero timeout, unknown variant or mismatched/invalid address.
/// @note Bounded value checks only: zero callbacks, no waits, no retained references or bus ownership.
/// Hardware wiring, exact part selection and callback bounds remain application responsibilities.
/// All returned messages have static lifetime and detail is zero.
Status validateDriverConfig(const DriverConfig& config);

/// @brief Check profile value/variant semantics without I2C, allocation or mutation.
/// @param[in] variant Explicit exact part; UNSPECIFIED and unknown values are invalid.
/// @param[in] profile Candidate settings, borrowed only for this call and left unchanged.
/// @retval OK All settings are semantically supported; this does not grant permission to write them.
/// @retval INVALID_CONFIG Invalid variant or profile enum, checked before capability restrictions.
/// @retval UNSUPPORTED_FEATURE Valid setting unavailable on this part, including a debounce from
/// another map, nonneutral absent controls or FOLLOW_PORT_PIN on TUSB322I.
/// @note Zero callbacks and waits; no retained references, observed state or hardware effects.
/// Neutral placeholders for absent controls are accepted. TUSB322I advertisement/duty/accessory values
/// remain valid; later apply checks proved actual values and separately rejects gated field changes.
/// All returned messages have static lifetime and detail is zero.
Status validateProfile(Variant variant, const DeviceProfile& profile);

/// @brief Decoded attachment; UNKNOWN never means a valid detached observation.
enum class Attachment : uint8_t {
  UNKNOWN,      ///< Unavailable, reserved or not decoded.
  DETACHED,     ///< Valid observation of no attachment.
  LOCAL_SOURCE, ///< Local port is attached as source.
  LOCAL_SINK,   ///< Local port is attached as sink.
  ACCESSORY     ///< Accessory attachment; not a normal peer power role.
};

/// @brief Independent local/peer power-role interpretation.
enum class PortRole : uint8_t {
  UNKNOWN,        ///< Role unavailable or undecoded.
  NOT_APPLICABLE, ///< Observed state has no normal power role, for example detached/accessory.
  SOURCE,         ///< Source power role.
  SINK            ///< Sink power role.
};

/// @brief Observed active CC direction with unavailable and inapplicable states kept separate.
enum class Orientation : uint8_t {
  UNKNOWN,        ///< Direction unavailable or undecoded.
  NOT_APPLICABLE, ///< Valid observed state has no applicable direction, for example detached.
  CC1,            ///< CC1 direction.
  CC2             ///< CC2 direction.
};

/// @brief Remote CC current indication; applicability requires a complete paired observation.
enum class DetectedCurrent : uint8_t {
  UNKNOWN,               ///< Indication unavailable, reserved or context not established.
  NOT_APPLICABLE,        ///< Valid observation in a context without remote current applicability.
  DEFAULT,               ///< Default USB current category; not measured load or a PD contract.
  MA_1500,               ///< 1.5 A CC indication in a documented applicable context.
  CHARGE_THROUGH_500_MA, ///< 500 mA audio charged-through indication in its documented context.
  MA_3000                ///< 3 A CC indication in a documented applicable context.
};

/// @brief Variant-specific accessory interpretation; raw codes alone do not prove applicability.
enum class Accessory : uint8_t {
  UNKNOWN,         ///< Accessory unavailable, reserved or context not established.
  NOT_APPLICABLE,  ///< Valid observation in a state where accessory interpretation is inapplicable.
  NONE,            ///< Valid documented no-accessory code in its applicable context.
  AUDIO,           ///< Audio accessory.
  CHARGE_THROUGH,  ///< Audio charged-through accessory.
  DEBUG,           ///< Debug accessory without a documented local role distinction.
  DEBUG_LOCAL_DFP, ///< Debug accessory with documented local DFP interpretation.
  DEBUG_LOCAL_UFP  ///< Debug accessory with documented local UFP interpretation.
};

/// @brief Software acquisition evidence; no value claims an atomic hardware snapshot.
enum class SnapshotCoherence : uint8_t {
  SEQUENTIAL,          ///< Sequential samples without matching-bookend evidence.
  BOOKENDS_MATCH,      ///< 0x09 bookends match under 0xE0; intervening changes remain possible.
  CHANGED_DURING_READ  ///< Bookends differ; dependent current/accessory/cable fields are invalid.
};

}  // namespace TUSB32x
