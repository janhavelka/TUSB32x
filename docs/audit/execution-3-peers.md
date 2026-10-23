# Prompt 3 focused peer re-audit

Reviewed 2026-10-23 for register contracts and independent encoding vectors.
Read parent/repository AGENTS.md, COMMON, prompt 3, the prompt index, current
progress/decisions, ENGINEERING_BASELINE and each peer's AGENTS.md. Peer sources
were read-only: no fetch, edit, build, remote CI query or hardware action.
This report supplies engineering conventions, not TI silicon authority or
implemented TUSB32x behavior. The register guide owns TI-derived values.

## Captured repository state

`git status --short` was empty for all three peers. Each was on `main`, tracking
`origin/main`, with local `HEAD...@{upstream}` ahead/behind `0/0`. These are local
reference comparisons; no claim is made about newly published remote commits.

| Peer | Full HEAD |
| --- | --- |
| PCA9555 | `498178e4d9e4acb6fab7d8b816ad5021efe8806a` |
| LDC1614 | `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` |
| ADS1115 | `381236ebfed813cd7a9aca34520ce031d8a0ff6b` |

Peer AGENTS agree on injected, non-owning transport, framework-neutral core,
explicit partial effects and source/build/hardware evidence separation. Their
Windows PlatformIO wrappers remain authoritative for their own builds. Their
chip behavior, compatibility APIs, health rules and CI policies do not override
TUSB32x's baseline or the initial promptset's prompt-34-only remote CI policy.

## PCA9555: byte registers, write guards and uncertainty

Sources inspected together:

- `include/PCA9555/CommandTable.h`: `REG_INPUT_PORT_0` through
  `REG_CONFIG_PORT_1`, `DEFAULT_OUTPUT`, `DEFAULT_POLARITY`, `DEFAULT_CONFIG`
  and `ERRATA_SAFE_CMD`. Comments name RO/RW access, reset values and pair-only
  auto-increment; they also identify the interrupt pointer workaround.
- `include/PCA9555/PCA9555.h`: public `writeRegister`, `writeRegisters`,
  `writeOutputs`, `writePin`, `readRegisters` and shadow/observation accessors.
  `src/PCA9555.cpp`: `validRegister`, `inputRegister`, `_writeRegs`,
  `_writePair`, `_writePort`, `writeRegisters`, `_parkPointer` and
  `_readInputRegisters`. `_writeRegs` builds a fixed three-byte buffer and
  rejects invalid address/length/RO writes before a callback. It invalidates a
  shadow on an ambiguous write; advanced scalar writes cannot establish a
  complete pair. Raw direction writes return UNSUPPORTED to preserve the
  output-preload procedure. Odd-start two-byte writes explicitly reverse the
  canonical combined value, matching this chip's pair wrapping.
- `test/test_basic.cpp`: `FakeBus`, `FakeTransaction`, `FakeFault`,
  `test_raw_configuration_writes_are_rejected_without_io`,
  `test_partial_raw_write_invalidates_whole_pair_until_full_pair_write`,
  `test_ambiguous_apply_write_is_terminal_at_each_write_phase`,
  `test_register_pair_round_trip_and_odd_start_wrap_match_chip_protocol`,
  `test_synchronous_input_reads_always_park_pointer_and_return_both_ports`,
  `test_input_registers_remain_read_only_in_fake_and_public_api` and
  `test_invalid_register_lengths_ports_and_pins_are_bus_silent`.
  Assertions require zero callbacks on raw direction rejection, an invalid
  shadow after a scalar raw write, exactly one failed write at each injected
  phase without advancing, literal odd-start bytes `12,34` producing port0
  `34`/port1 `12`, and exactly two callbacks for input read plus pointer park.
  The named tests appear in `main`'s `RUN_TEST` list.

Adopt named address/access/reset constants, fixed buffers, bus-silent invalid
inputs, explicit guards against bypassing a required procedure, and independently
specified write/failure expectations. Preserve distinct observation and writable
trust: a successful ordinary read does not automatically restore a write shadow.
Reject pair wrapping, little-endian port packing, pointer parking and mandatory
post-read cleanup as TUSB32x rules. PCA's richer `WriteEffect` transport cannot
be assumed from TUSB32x's Status-only callbacks. Its successful pointer cleanup
is intentionally omitted from success counters; TUSB32x instead counts every
callback in its neutral totals and maintains its own filtered health evidence.

## LDC1614: access masks, independent literals and behavioral fakes

Sources inspected together:

- `include/LDC1614/CommandTable.h`: named registers, masks and shifts;
  `CONFIG_RESERVED_VALUE`, `MUX_CONFIG_RESERVED_VALUE`,
  `MASK_ERRCFG_ALLOWED`, `MASK_RESET_DEV`, `MASK_IDRIVE` and
  `MASK_INIT_IDRIVE`. The reset command is described separately from persistent
  controls; initial drive current is explicitly read-only. The deglitch
  conflict comment records conflicting table/field evidence instead of hiding it.
- `src/LDC1614.cpp`: `buildConfigRegister`, `buildMuxConfigRegister`,
  `configWriteAt`, `configurationComparisonMask`, `_readRegister`,
  `_writeRegister`, `readRegister16` and `writeRegister16`. Builders start with
  required reserved images. Comparison masks deliberately exclude sleep or
  live initial-drive bits where applicable. Raw diagnostic writes reject
  nonconfiguration/variant-inapplicable addresses and invalidate applied trust
  when mutation is possible. The raw API is not a universal safe field composer.
- `test/support/FakeLdc1614Device.h`: `write`, `writeRead`, `failOnTransfer`,
  `scheduleConversionAfter`, `readValue`, `applyReadSideEffects` and
  `applyWrite`. The fixture logs fixed-capacity transfers, can commit a write
  before returning failure, latches LSB on MSB read, and models STATUS/unread/INTB
  consumption. It is more than a byte-array readback mirror.
- `test/test_basic.cpp`:
  `test_expected_configuration_register_maps_every_variant_register_and_mask`
  independently asserts literal masks `F8FD`, `F800`, `DFFF`, `F83F` and expected
  images such as `3C81`, `1002`, `0209`;
  `test_exact_register_replay_for_both_variants_and_all_replay_jobs` supplies
  literal expected write arrays rather than calling the production builder;
  `test_behavioral_fake_models_shadow_status_unread_and_intb` injects a new
  conversion between MSB and LSB and still requires old LSB `4567`;
  `test_initialize_fails_once_at_every_transfer_with_full_provenance`,
  `test_apply_exact_counts_failure_cancel_and_ambiguous_write_provenance` and
  `test_ldc1612_binding_rejects_ldc1614_only_registers_and_masks` cover failure
  and capability boundaries. These suites have explicit `RUN_TEST` entries.

Adopt separate masks for writable, live/read-only, command and comparison
purposes; visibly document unresolved source conflicts; keep independent literal
vectors and a small behavioral fixture for destructive reads/commands/faults.
The fake itself imports production constants and permissively stores most raw
writes, so passing it alone cannot establish reserved-bit correctness or silicon
facts. TUSB32x vectors must be independently derived from TI and must model its
own IRQ/reset mixed-access semantics. Reject LDC's 16-bit MSB-first framing,
conversion coherency rules, sensor timing, initialization replay, reset/reapply
and result FIFO policy; TUSB32x initialization is read-only with one result slot.

## ADS1115: validation before field composition

Sources inspected together:

- `include/ADS1115/CommandTable.h` and `include/ADS1115/Config.h`: masks/shifts,
  typed `Mux`, `Gain`, `DataRate`, and public `validateDeviceProfile`,
  `validateComparatorProfile`, `validateChannelRequest` declarations.
- `src/ADS1115.cpp`: compile-time mask/shift checks; `isValidMux`, `isValidGain`,
  `isValidDataRate`, `decodePgaBits`, the three public validators and
  `_buildConfigRegisterFor`. Validation rejects out-of-range typed inputs
  before composition. Gain codes 6/7 decode to the canonical 0.256 V gain;
  setters accept only codes 0..5. The composer shifts/masks validated fields;
  `kConfigReadbackMask` excludes the live/start OS bit.
- `test/test_basic.cpp`:
  `test_begin_rejects_invalid_enum_values_without_bus_access`,
  `test_public_setters_reject_invalid_enum_values_without_bus_access`,
  `test_owner_safe_profile_validation_boundaries`,
  `test_config_setters_write_expected_config_bits` and
  `test_write_config_normalizes_datasheet_pga_aliases_for_0_256v`.
  The invalid-setter test supplies mux8/gain6/rate8/mode2 and invalid comparator
  enums, requires INVALID_PARAM, zero reads/writes and unchanged cached fields.
  Profile tests distinguish INVALID_CONFIG from invalid channel requests.
  Alias tests require canonical code5 for both alias images and UNKNOWN trust
  after the diagnostic start. Each named test has a `RUN_TEST` entry.

Adopt validation before masking, explicit semantic mapping, preserved outputs
on rejected input and readback masks that exclude transient command/status bits.
Do not assume contiguous enum ordinals when the chip has sparse encodings:
TUSB32x Try.SRC is explicit code3, while code2 remains reserved. Do not import
ADS's alias normalization for TUSB reserved fields: retain raw evidence and
invalidate the affected interpretation. ADS setter assertions often compare
against the same enum/masks as production; supplement that pattern with TI-derived
literal bytes. Reject all ADC 16-bit framing, conversion timing, signed samples,
comparator thresholds and synchronous compatibility service loops.

## Examples, runners and registration inspected

| Peer | Current source and relevant boundary |
| --- | --- |
| PCA9555 | `examples/01_basic_bringup_cli/main.cpp`, `examples/espidf_basic/main/main.cpp::cmdWriteReg/cmdWriteRegs`, `examples/common/I2cTransport.h`; raw writes accept only 02..05 and bounded byte values, reaching the same guarded public API. `tools/run_i2c_hil.py`, `check_hil_contract.py`, `test_run_i2c_hil_parser.py` retain bounded transcripts and explicitly disclaim identity from ACK. Do not import firmware stress schedulers or duplicated framework CLIs. |
| LDC1614 | `examples/common/Ldc1614Cli.cpp::printRegisterValue` and REG/WREG dispatch, `examples/esp_idf/basic/main/Ldc1614IdfCli.cpp` matching dispatch; reads label register/access/reserved validity, WREG parses bounded address/16-bit value and calls `writeRegister16`. `tools/ldc1614_cli_contract.py`, `ldc1614_hil_runner.py`, `test_ldc1614_hil_runner.py` inspect firmware Git/clean evidence and produce NOT_RUN without serial hardware. TUSB32x uses one portable CLI and host sequencing, not duplicated sessions or on-device repetition. |
| ADS1115 | Arduino `examples/01_basic_bringup_cli/main.cpp`, native `examples/esp_idf/basic/main/main.cpp` config/reg/wreg dispatch, and `examples/02_owner_safe_poll/main.cpp`; bounds are checked before narrowing. `tools/run_i2c_hil.py` validates firmware commit/clean markers and classifies bounded output; its conversion constants are ADC-specific. No register readback or ACK establishes exact chip identity. |

Read each `platformio.ini`, root `CMakeLists.txt` and
`.github/workflows/ci.yml`. All register native Unity tests via
`test_framework=unity`, `test_build_src=true` and source filters; the inspected
`RUN_TEST` lists actually include the suites above. These peers' root CMake files
are IDF component registrations, not standalone CTest builds. PCA/LDC select
C++17; ADS native/IDF select C++11, which is rejected for TUSB32x's C++17 core.

Checked-in workflows run `pio test -e native`, timing/CLI/native-IDF guards,
version checks and their respective runner parser tests. They declare Arduino
S2/S3 plus native-IDF builds: PCA IDF5.4, LDC IDF6.0.2, ADS IDF5.3.5 container
with a digest. PlatformIO Core is pinned to 6.1.19 (LDC via
`requirements-dev.txt`); Arduino platform URLs pin 55.03.311. Doxygen is registered
for all three; PCA/ADS register package checks and LDC a clean-consumer check.
Adopt actual suite registration and separate validation layers, not peer build
versions or declarations of successful support. No workflow was dispatched or
polled, and no peer command was built or tested for this source audit.

## Outcome and check limits

Source comparison is complete for this focused prompt. Preserve one small
TUSB-specific write composer and direct validators; no generic register engine,
compatibility aliases or peer-dependent core is justified. Independent expected
bytes, mixed-access safety, reserved/variant negatives and procedure-only write
gates belong in the register guide now and future behavioral suites later.

Read-only state/source/registration inspection: PASS. Peer builds/tests, TUSB32x
driver behavior, firmware links and physical qualification: NOT RUN. This prompt
adds documentation only; reading assertions does not mean those tests passed.
Local report checks passed: UTF-8/whitespace, captured full revisions, and all 18
named peer test definitions paired with their actual `RUN_TEST` registrations.
