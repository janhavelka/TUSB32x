# Prompt 18 protocol and failure-evidence peer audit

Read-only inspection on 2026-10-27. This is current source evidence, not a claim
that peer tests, firmware or hardware were run. No sibling was edited, fetched or
built; no remote CI was inspected or dispatched.

## State and instructions

TUSB32x starts this unit at `8507681e92938b3eb8d0a7aa7308a1c8318ecbaf`, on the
existing `promptset/tusb32x-implementation-20261023` execution branch. The lead
owns synchronization and Git mutations. During this delegated read, its progress
file had the lead's in-progress edit and the index was empty; that concurrent edit
was preserved. Re-read parent/local AGENTS, COMMON, prompt index, complete prompt
18, CHIP_EVIDENCE, AGENT_WORKFLOW, current progress/decisions and actual source/test
registration. Reopened each peer AGENTS and the relevant declarations,
implementation, examples, CLI, runners, CMake, PlatformIO and checked-in workflow.

| Peer | Branch | HEAD | Index/worktree |
| --- | --- | --- | --- |
| LDC1614 | main | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | Clean |
| LSM6DS3TR | main | `362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc` | Clean |
| PCA9555 | main | `498178e4d9e4acb6fab7d8b816ad5021efe8806a` | Clean |
| ADS1115 | main | `235af3e8c722661a89a44766a4ccd903a654f6fe` | Clean |

State commands were `git -C ../<peer> rev-parse HEAD`, `branch --show-current`,
`status --porcelain=v1` and `diff --cached --name-status`. Earlier audit reports
were navigation aids; the revisions and files above were freshly inspected.
Their instructions agree on external bus ownership, bounded callbacks, explicit
uncertain effects and hardware-evidence limits. TUSB's prompt-34-only remote-CI
rule takes precedence here.

## Concrete peer evidence and decisions

| Current source and named checks | Observation | Adopted or rejected for TUSB |
| --- | --- | --- |
| LDC `include/LDC1614/LDC1614.h::TransportStats`, `src/LDC1614.cpp::_readRegister/_writeRegister/_tracked`; `test/support/FakeLdc1614Device.h`; `test/test_basic.cpp::test_initialize_exact_transfer_counts_and_zero_one_large_budgets`, `test_initialize_fails_once_at_every_transfer_with_full_provenance` | The fixed fake has a 192-entry transfer log, callback counters, timeout/address/kind records and optional commit-before-error. The tests inspect zero/one/large budgets and the exact terminal callback count. `_tracked` counts each returned callback once and saturates totals. | Adopt fixed capacity, explicit callback-count assertions and both write-effect possibilities. TUSB already owns one accounting boundary. Reject copying sensor registers, destructive DATA latching, large-budget multi-callback execution or interpreting callbacks as a known physical transaction count. Also retain TUSB outputs on failure instead of copying LDC's pre-read scalar clearing. |
| LSM `include/LSM6DS3TR/LSM6DS3TR.h::OperationResult`, `src/LSM6DS3TR.cpp::_writeByte/_fail/_invalidateConfiguration`; `test/test_basic.cpp::test_ambiguous_write_effect_is_observable_and_never_retried` | A fault at callback three commits data before returning I2C_TIMEOUT detail -88. Exactly three callbacks occur; the result is INDETERMINATE and configuration becomes UNKNOWN. The fake records the applied effect independently of callback status. | Adopt original failure/detail, observable ambiguity, terminal failure and no automatic replay. Reject copying bank/FIFO restoration, sensor acquisition or a configuration engine into this library. |
| PCA `include/PCA9555/PCA9555.h::ObservedState/OperationResult`, `src/PCA9555.cpp::_writeRegs/_readPair/_mapTransportResult`; `test/test_public_contracts.cpp`; `test/test_basic.cpp::test_mixed_verify_reconciles_matches_and_keeps_mismatch_fenced`, `test_input_registers_remain_read_only_in_fake_and_public_api`, `test_transport_short_counts_are_rejected_and_never_fake_success` | Public compile contracts separate complete/partial writable images. Runtime tests distinguish observed bytes from write shadows, reconcile matching pairs after ambiguous writes, retain mismatches and reject RO writes. TransportResult exposes actual byte counts and write effect. | Adopt public behavioral tests, independent observations, safe write images and failure-at-each-phase checks. Compile assertions remain compile evidence only. Reject PCA's pair wrapping, pointer-park cleanup and count/effect callback interface as TUSB protocol facts; TUSB callbacks return Status and must report incomplete receives as failure. |
| ADS `include/ADS1115/ADS1115.h::OperationResult/hardwareConfigDirty`, `src/ADS1115.cpp` APPLY_WRITE_LOW_THRESHOLD/HIGH_THRESHOLD/CONFIG and `_requireConversionReconciliation`; `test/test_basic.cpp::test_begin_failure_after_first_apply_write_preserves_dirty_diagnostic`, `test_begin_failure_after_second_apply_write_preserves_dirty_diagnostic`, `test_begin_failure_on_third_apply_write_preserves_original_status_and_dirty` | Earlier writes remain present after a later failure; tests assert the original backend detail and dirty evidence. The fake has an `applyFailedWrite` switch; the engine retains possible conversion effects independently of the cached profile. | Adopt explicit previous-effect retention and no trust repair from unrelated success. Reject copying conversion timing, guessed device identity, synchronous compatibility facades, private-state test seeding or native-IDF-only root CMake into TUSB. |

The inspected LDC/LSM/ADS runtime cases are invoked by their `RUN_TEST` lists in
`test/test_basic.cpp`; PCA also registers the mixed/RO/short-count cases there.
PCA's separate public-contract translation unit contains static assertions, not
runtime behavioral tests. TUSB's matrix uses named existing cases and the
registered CTest executable that actually calls them.

## Examples, runner and build review

Exact inspected application/runner paths:

| Peer | Example and CLI | Python runner |
| --- | --- | --- |
| LDC | `examples/01_basic_bringup_cli/main.cpp`, `examples/common/Ldc1614Cli.cpp`, native `examples/esp_idf/basic/main/main.cpp` | `tools/ldc1614_hil_runner.py`, `tools/test_ldc1614_hil_runner.py` |
| LSM | `examples/01_basic_bringup_cli/main.cpp`, `examples/common/ProfileCli.h`, native `examples/idf/basic/main/main.cpp` | `tools/run_hil.py` |
| PCA | `examples/01_basic_bringup_cli/main.cpp`, `examples/common/CliShell.h`, native `examples/espidf_basic/main/main.cpp` | `tools/run_i2c_hil.py`, `tools/test_run_i2c_hil_parser.py` |
| ADS | `examples/01_basic_bringup_cli/main.cpp`, `examples/02_owner_safe_poll/main.cpp`, native `examples/esp_idf/basic/main/main.cpp` | `tools/run_i2c_hil.py` |

LDC separates core callback counters from adapter statistics. LSM correlates
accepted operation tokens with fresh terminal records and validates the reported
transaction ceiling. ADS checks firmware-reported commit/clean markers against
host metadata; PCA's runner records host provenance and checks command completion
framing. Adopt explicit units and fresh result correlation in the later TUSB CLI
and runner stages. Do not claim those stages, physical fixtures or firmware links
were implemented by this protocol audit, and do not treat host checkout identity
as proof of flashed firmware.

Reopened all four root `CMakeLists.txt`, `platformio.ini` and
`.github/workflows/ci.yml`. Their native PlatformIO environments register Unity
with `test_build_src=true`; workflows actually invoke `pio test -e native` and
separate version, timing, CLI, package/documentation and firmware checks. Current
sources use Core 6.1.19 and pioarduino 55.03.311; native IDF sources select LDC
6.0.2, LSM 5.4.4, PCA v5.4 and ADS's digest-pinned 5.3.5 container. These are
source observations; no tool was installed and no peer job was run. LDC's CMake
runs Python version checking during IDF registration; ADS selects C++11 there.
Neither is imported into TUSB's tests-OFF C++17 consumer path.

TUSB `test/CMakeLists.txt` registers the real protocol, operations, port and
diagnostics executable suites with strict warnings and active Release checks.
Compile-only header contracts have no CTest entry. At the starting revision there
is no checked-in `.github/workflows` directory. Tests, host example and Python
version/wrapper tooling exist; framework CLI/HIL runners remain later work.

## TI evidence and limits

Reopened archived PDFs with `pypdf`, rather than trusting CHIP_EVIDENCE's copied
claims: B SLLSEN9F Table 7-7 pp16-19, §7.4.3 p14 and switching table p7; L
SLLSEP2D and A SLLSEQ8D §7.3.2 p14, Tables 8-12 pp17-20 and timing p6; V
SLLSEO5D Tables 6-7 through 6-11 pp17-20 and timing p7. B recovery says qualified
45 write 04, at least 30 ms, write 00; L/A mode change says disable, mode, at least
5 ms, enable. The one-tick margin is owner-clock policy, not a changed TI minimum.
Register 45 internal bits 03 stay protected; base numeric internal defaults are not
invented. V advertisement and fault-clear conflicts stay gated, along with direct
non-reset return to DRP. See [conformance](../protocol-conformance.md) and the
[source provenance](../reference/source/provenance.json).

This audit supplies source coverage and requirement mapping. The lead records
actual local test results in [validation](../validation.md) after integration.
Peer tests, framework firmware and physical hardware are NOT RUN by this audit;
source inspection and host fakes establish none of those layers.
