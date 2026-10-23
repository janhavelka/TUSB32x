# Prompt 2 peer and public-source re-audit

Reviewed 2026-10-23. This is source/provenance evidence for the capability
specification, not an implemented driver or a hardware result. Siblings were
read-only. No sibling Git mutation, build, CI query or hardware action was made.
Read repository/parent AGENTS.md, COMMON, prompt 2, prompt index, current progress
and decisions, and relevant ENGINEERING_BASELINE/CHIP_EVIDENCE sections.

## Current sibling state

`git status --short` was empty for each sibling before and after inspection.
The three branch labels were `main`, with `origin/main` at the same captured
HEAD. These are local repository observations; sibling remotes were not fetched.

| Sibling | Full HEAD | Focus |
| --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | Explicit package/model, address and physical-pin capabilities |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | Reference provenance, exact geometry and archive/package boundaries |
| LDC1614 | `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` | Shared identity, configured capabilities and behavioral fake limits |

Each sibling's AGENTS.md was re-read. All require framework-neutral core code,
application-owned callbacks, preserved source licenses/hashes, honest hardware
claims and the existing Windows PlatformIO wrapper. Their recurring remote-CI
rules are not imported: this execution's explicit prompt-34 policy governs.

## Adopted and rejected peer conventions

### TMP1x2

- `include/TMP1x2/Config.h`: `Model`, `isValidModel`, `modelAddressMin`,
  `modelAddressMax`, `isValidAddress`, `hasAlertOutput` explicitly select BOM and
  package facts. `TMP112D_ADDRESS_SELECT` means the particular ADD0 X2SON package,
  not every ordering code containing TMP112D. Its missing ALERT pin does not
  remove register-backed thermostat/SMBus capabilities.
- `src/TMP1x2Configuration.cpp`: `validateConfig` checks model/address and rejects
  an unavailable physical ALERT input; `readAlertPin` also guards that capability.
  `src/BusOperations.cpp::decodeAlertCause` validates an SMBus Alert Response
  Address (ARA) response against the caller-selected model; it does not discover
  the model.
- `test/variants/test_variants.cpp` exhausts all 128 seven-bit addresses for three
  models, checks invalid enum values, requires bus-silent rejection, and tests
  absent physical ALERT separately from permitted register controls. Adopt this
  exhaustive configured-variant boundary and separate identity/capability facts.
- `examples/common/Tmp1x2CliOutput.cpp` and `Tmp1x2CliDiagnostics.cpp` label ACK
  and discovery identity unverified. `tools/hil_tmp1x2_runner.py`:
  `parse_settings`, `parse_firmware` and `Runner` identity checks distinguish
  operator-selected model from firmware Git/framework/target evidence. Adopt the
  separation; do not import sensor registers, clock callbacks or blocking APIs.
- `CMakeLists.txt` explicitly compiles core sources, defaults tests OFF and
  registers `variants`. `library.json` exports `include/`, `src/`, examples and
  reference README, excluding the reference payload. `.github/workflows/ci.yml`
  declares strict sanitizer host checks, PlatformIO 6.1.19, Arduino S2/S3 and
  native IDF 5.3.2/5.5.1/6.0.1. Source review is not a passed build.

### 24Cxx

- `docs/reference/manifest.json` records original/final URLs, retrieval UTC,
  manufacturer, revision, size, SHA-256 and original terms. Failed retrievals
  remain distinguished from archived files. `repository-inventory.json` pins
  repository commits and each acquired source/license artifact's bytes/hashes.
  `README.md` states the bounded survey and retains manufacturer-specific limits.
- `tools/check_reference_archive.py`: `check_file` rejects archive path escape,
  verifies SHA-256 and optional byte count, then checks every retained non-cache
  reference file is covered by `SHA256SUMS`. Adopt precise provenance and complete
  coverage rather than treating a manufacturer's repository as silicon authority.
- `include/EEPROM24Cxx/Config.h` makes `DeviceVariant` application-selected;
  `test/test_detection/test_detection.cpp` preserves selected geometry and
  `test/test_cli/test_cli.cpp` expects bank candidates with identity unverified.
  Adopt no identity inference from ACK; reject EEPROM geometry, write-cycle and
  compatibility API semantics as unrelated to TUSB32x.
- `CMakeLists.txt` lists two production source files and tests default OFF.
  `library.json` exports reference README but no reference payload.
  `tools/check_package_contents.py` rejects `docs/reference/source/`,
  `docs/reference/datasheets/` and archived/generated artifacts from real packages.
  `.github/workflows/ci.yml` runs the archive guard and an extracted CMake package
  consumer, plus strict host, Arduino S2/S3 and native IDF matrix checks. Adopt
  the package guard and extracted-consumer evidence; no package was built here.

### LDC1614

- `include/LDC1614/Config.h`: `DeviceVariant` is explicit because silicon identity
  registers are shared; `validChannelMask` maps LDC1612 to `03`, LDC1614 to `0F`,
  and unsupported values to an empty mask. `CommandTable.h` records shared
  `DEVICE_ID_VALUE=3055`. `src/LDC1614.cpp`: `channelCountFor`, `validateConfig`
  and typed register access reject unsupported channels independently of identity.
- `test/test_basic.cpp`:
  `test_bind_is_zero_i2c_and_validates_complete_explicit_profile` rejects missing
  variant and CH2 selection on LDC1612 before transport; initialization identity
  failure checks preserve exact phase/cause and configuration uncertainty.
  `test_behavioral_fake_models_shadow_status_unread_and_intb` checks destructive
  DATA/STATUS effects. `test/support/FakeLdc1614Device.h` deliberately models
  latched data, unread/error/INTB evidence and failure-after-commit injection.
  Adopt behavioral fakes with independent expectations; the fake initializes
  four channel arrays and shared IDs and is not an automatic variant detector.
- `examples/common/Ldc1614Cli.cpp` exposes `variant` as BUILD_PROFILE_ONLY;
  `tools/ldc1614_hil_runner.py` requires separate configured-channel, chip identity
  and firmware identity evidence. Adopt that separation; reject initialization
  replay, its multi-result store and diagnostic-operation choices for TUSB32x.
- `CMakeLists.txt` is IDF-only and unconditionally requires Python/version checks;
  reject that ordinary-consumer dependency. `library.json` exports a datasheet;
  reject that exception for TUSB32x reference payloads. `platformio.ini` explicitly
  selects core/example source paths and Unity native tests.
  `.github/workflows/ci.yml` declares native, contract/readiness, Doxygen, Arduino
  S2/S3 and IDF 6.0.2 checks. No remote runs were inspected.

## Fresh public-source verification

All 13 entries in [public-implementations.json](../reference/public-implementations.json)
were downloaded from their exact pinned `raw.githubusercontent.com` commit/path
on 2026-10-23 between 14:38:52.915457 and 14:38:53.546158 UTC. Every file matched
the manifest's byte count, SHA-256 and Git blob SHA-1 (`blob <length> NUL <bytes>`):
**39 of 39 comparisons passed**. Source was cached only under the OS temporary
directory, outside this repository; no source reuse or vendoring occurred.

| Manifest ID | Bytes | License/attribution observation |
| --- | ---: | --- |
| ti-linux-driver | 16489 | GPL-2.0 SPDX; National Instruments, Michael Auchter |
| ti-linux-binding | 778 | GPL-2.0-only OR BSD-2-Clause SPDX; Auchter maintainer |
| ti-linux-regmap-header | 74869 | GPL-2.0-only SPDX; Wolfson, Mark Brown |
| ti-linux-regmap-core | 88105 | GPL-2.0 SPDX; Wolfson, Mark Brown |
| ti-linux-regmap-i2c | 9575 | GPL-2.0 SPDX; Wolfson, Mark Brown |
| ti-linux-xiaomi-board | 16138 | BSD-3-Clause SPDX; Yassine Oudjana |
| ti-linux-kumquat-board | 5794 | GPL-2.0+ OR MIT SPDX; Lukas Schmid |
| ti-linux-j721e-board | 30539 | GPL-2.0-only OR MIT SPDX; Texas Instruments |
| armoryctl-tusb320 | 1465 | File points to root LICENSE; fetched BSD three-clause text |
| betrusted-ec-tusb320lai | 6834 | No file grant; fetched root CERN OHL 1.2; Cargo author bunnie |
| tiliqua-tusb322i | 5692 | No file grant; fetched root CERN-OHL-S v2 and mixed-license README |
| xous-tusb320lai-emulator | 2426 | MIT file marker; Antmicro |
| mobiaqua-tusb322-wip | 6067 | Introduced driver GPL-2.0 SPDX; NI/Auchter; Zhangqun Ming patch |

The supplemental root-license files were fetched at the same pinned commits.
SHA-256: armoryctl `LICENSE`
`82532d5aa635c7c19ff97945fc7f37bd681676306ed1cdce1668d559df636f97`;
betrusted-ec `LICENSE`
`e5d8c837f87ce76c91e5595e71e55e6f25595fb406617563dfe15453ec5e97c3`;
tiliqua `LICENSE`
`253ad3f89603e728abfa60c36fbcaf8225cf55c1eab12725f19fb3d74d647f3a`.
Tiliqua's README says some software is BSD three-clause only where individual
headers specify it; this inspected driver has no such marker. No permissive
firmware reuse grant was established for the two Rust files.

## Public-code findings and limits

- Linux `tusb320_check_signature` (lines 101-117) compares descending addresses
  7..0 with NUL,T,U,S,B,3,2,0. Its ascending vector is therefore
  `30 32 33 42 53 55 54 00`. `tusb320l_get_revision` reads A0 only through the
  selected extended ops. Probe logs the revision; it does not verify `01`/`02`,
  distinguish exact suffixes, or permit future revisions on TI's authority.
  Binding supports only `ti,tusb320` and `ti,tusb320l` categories.
- Regmap is not a shifted-field helper: `include/linux/regmap.h:1340`
  `regmap_write_bits` requests forced update; `regmap.c:3250`
  `_regmap_update_bits` composes `(orig & ~mask) | (val & mask)`. Thus Linux's
  reset call with mask `08`, value `01` does **not** set reset. Extended mode
  change has a five-ms sleep and cleanup can overwrite the original mode-write
  error. `regmap-i2c.c:172` `regmap_i2c_read` submits pointer/read messages in one
  `i2c_transfer`, checks exact message count and preserves negative backend error.
  None establishes framework-free aggregate time bounds for this library.
- Xiaomi board selects `ti,tusb320l` at 47; Kumquat selects `ti,tusb320` at 60.
  J721E's `typec-dir-debounce-ms` comment names TUSB321 (line 962), with GPIO
  direction handling rather than a TUSB320 I2C declaration. These are board
  integration examples, not exact-variant discovery or physical qualification.
- Armory `GetDeviceID` reverses ascending bytes for display. `GetCurrentMode`
  shifts 08 by four, reading detection despite its advertisement comment; its
  default-current `0.5 A` text cannot establish a universal USB power allowance.
- Betrusted supplies the correct ascending 320 vector and revision `02`, plus
  unexplained alternate `06`; the revision assertion is commented out. Infinite
  transport retry loops and whole-byte writes are explicit reasons not to adopt
  its lifecycle. Its `SOURCE_PREF_DRP_TRY_SRC=04` is a conflict seed, not authority.
- Tiliqua reads raw identity without an expected signature/revision check.
  Its `set_mode` makes whole-byte 0A writes and no wait; `soft_reset` submits `08`
  without verifying completion. That code does not settle 322 capability gates.
- Xous/Renode has a minimal identity stub: a two-byte write uses `data[1]` as
  pointer, reads fill only `result[0]` for ID addresses, and it models neither
  revision nor real mixed-register effects. Passing it cannot prove protocol.
- Yocto's WIP `tusb322_check_signature` changes the ascending first byte to `32`
  but proves no exact revision/capabilities; copied NI attribution, `meta-ti`
  location and a WIP title are not TI silicon authority or production evidence.

All above are code observations. Exact variant/electrical/timing facts and the
revision-06 gate require the TI-source review in the companion matrix. No new
unresolved access failure was encountered in this focused audit.

## Checks

- PASS: 13 fresh public downloads, all 39 byte/hash/blob comparisons.
- PASS: file SPDX/root license and called regmap-helper source inspection.
- PASS: read-only `python ../24Cxx/tools/check_reference_archive.py`:
  15 documents, 28 source artifacts, 66 checksums.
- PASS: final sibling status remained clean at the captured HEADs.
- NOT RUN: sibling builds/tests beyond the archive checker, package extraction,
  linked framework firmware and physical hardware. This report proves none of
  those layers. No remote CI was dispatched, polled or required.
