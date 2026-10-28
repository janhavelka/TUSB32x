# Prompt 30 peer and fixture evidence audit

Read-only source audit on 2026-10-28. This agent owns only this report. No
sibling edits, builds, hardware access, Git mutations or remote CI inspection
were performed. Implementation and verification remain the lead's work.

## Entry state and rules

Read local `rev-parse HEAD` and `status --short` for each checkout. All five
worktrees were clean at this audit's entry. Paths below are relative to the named
checkout under `C:/Users/Honza/Documents/Projects/`.

| Checkout | Full HEAD |
| --- | --- |
| TUSB32x | `1f0fb7604ad1696aea9f3067c320d3b3c2fca89e` |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` |

The execution branch remains `promptset/tusb32x-implementation-20261023`.
The lead owns staged-diff review, fetch, history integration, commits and sync.
This report proves local source review, not remote equality or passing peer tests.

Read parent/local and all four peer root `AGENTS.md` files, the promptset index,
COMMON, prompt 30, AGENT_WORKFLOW, the fixture/port/profile/capability/IRQ and
evidence sections of CLI_CONTRACT, ENGINEERING_BASELINE, ENGINEERING_RULES and
CHIP_EVIDENCE. Revisited the prior prompt-29 report only as an index to current
sources. TUSB32x has no `.github/workflows/` directory at entry; remote CI remains
prompt 34 work. Each peer's `.github/workflows/ci.yml` was read as source.

## TMP1x2: physical prerequisites and conditional cleanup

Inspected `include/TMP1x2/TMP1x2.h`,
`examples/common/Tmp1x2Cli.cpp`, `tools/hil_tmp1x2_runner.py`,
`tools/test_hil_tmp1x2_runner.py` and `.github/workflows/ci.yml`.

- Adopt recording whether physical ALERT input exists separately from internal
  comparator evidence. `Runner.alert()` checks configured availability/pin and
  requests `alertpin`; the CLI labels it GPIO-only. TUSB32x must likewise require
  explicit wired INT and physical observation before claiming connector/INT
  qualification. A modeled latch or decoded orientation cannot satisfy it.
- Strengthen the peer's absent-pin SKIP: a selected TUSB fixture suite must retain
  missing physical evidence as NOT RUN and cannot finish overall PASS.
- Adopt `Runner.restore()` refusing commands after firmware restart or framing
  loss. `test_reboot_prevents_automatic_restore_to_unchecked_target` asserts no
  cleanup commands and unverified restoration. Adopt separate presentation and
  hardware restoration outcomes and durable partial evidence.
- Reject copying its captured sensor-profile replay, comparator thresholds,
  conversion sleeps, or implicit eligibility from unchanged serial framing.
  A USB target can remain connected while partner, attachment or supply changes.
  Before any TUSB cleanup write, revalidate target identity, current attachment,
  power assumptions, owner slot and the permitted fixture transition. Never
  restore an old source role solely because it was the entry profile.

Its workflow registers strict CMake/CTest with ASan/UBSan, documentation and
separate Arduino/native-IDF builds. Reuse evidence separation and local test
registration; its framework versions and observed source registration do not
qualify TUSB32x or establish a passing run.

## 24Cxx: bounded authorization and retained primary failure

Inspected `include/EEPROM24Cxx/EEPROM24Cxx.h`,
`examples/common/Eeprom24CxxCli.cpp`, `tools/hil_runner.py`,
`tools/test_hil_runner.py`, `tools/test_hil_guards.py` and
`.github/workflows/ci.yml`.

- Adopt pre-open checks for exact mutation scope. `parse_args()` requires explicit
  write permission and a bounded model-fitting scratch window; TUSB32x instead
  needs an explicit named fixture and suite-specific supply/wiring/role criteria.
  Selecting an I2C family cannot authorize board-power control.
- Adopt stopping before mutation when baseline evidence cannot be preserved.
  `test_backup_io_failure_prevents_programming_and_never_restores` and
  `test_changed_durable_backup_prevents_programming` assert no later write.
  Retain TUSB checkpoint/transcript failures before each mutation as terminal
  evidence failures, with no success summary left behind.
- Adopt `test_identity_mismatch_stops_before_device_procedures` and
  `test_preflight_failure_never_cancels_foreign_work`. The finite failure path
  explicitly sends no cleanup bytes after uncertain framing or foreign work.
  No automatic replay or foreign-result take may be introduced for TUSB suites.
- Adopt malformed negative-record coverage: `test_missing_wrong_or_malformed_status_fails`
  rejects incomplete/duplicate-like details, hidden CR payload and terminal escape
  sequences. A claimed setup rejection needs exact typed terminal evidence and
  zero callbacks, rather than a matching error substring.
- Reject scratch RAM/preimage/CRC restoration, EEPROM address geometry and
  firmware bulk scratch procedures as USB policy. Readback can verify register
  state but cannot reverse a real attach, VBUS transition or current negotiation.
  Preserve primary outcome, cleanup attempt/outcome and residual uncertainty
  separately; missing cleanup proof prevents overall PASS.

The peer workflow runs strict native CTest and reference/package guards plus
separate firmware targets. It does not provide physical fixture qualification.

## LDC1614: requested gates must affect the verdict

Inspected `include/LDC1614/LDC1614.h`, `tools/ldc1614_hil_runner.py`,
`tools/test_ldc1614_hil_runner.py` and `.github/workflows/ci.yml`.

- Adopt `overall_status()` refusing empty commands/transcript and incomplete
  command outcomes. `combine_requested_gate_status()` prevents a requested
  NOT_RUN gate from preserving PASS.
- Adopt explicit visible fixture gates and checking that requested wired fixture
  actions actually enter the finite plan. Relevant tests are
  `test_physical_fixture_gates_are_always_visible_as_not_run`,
  `test_requested_wired_fixture_commands_are_actually_scheduled`,
  `test_explicit_optional_gate_failures_affect_overall_status` and
  `test_repeat_and_dry_run_remain_bounded_not_run`.
- Adopt fresh value proof instead of echoes, as covered by
  `test_requested_values_cannot_be_satisfied_by_stale_echoes`; retain exact TUSB
  schemas, request/origin/token identity and validity masks. Unknown accessory
  encodings remain raw unknown evidence, never a plausible fixture type.
- Reject copying UNKNOWN/SKIP names, literal exit values, device-side schedulers,
  acquisition assumptions or profile replay at initialization. TUSB uses the
  existing session/verdict layer and ordinary scalar CLI operations.

The peer workflow explicitly runs Python HIL tests separately from native,
Arduino and native-IDF builds. Source inspection proves registration only.

## INA228: desired state, hardware proof and exclusion

Inspected `include/INA228/INA228.h`, `src/INA228.cpp`,
`examples/01_basic_bringup_cli/main.cpp`, `test/test_basic.cpp` and
`.github/workflows/ci.yml`.

- Adopt separate desired settings and hardware trust. `getSettings()` exposes
  `_config` while calibrated evidence additionally requires clean hardware and
  `SYNCHRONIZED`. A cached desired profile is insufficient applied proof.
  TUSB should retain captured profile/trust/generation and prove its selected
  stage/apply/verify sequence against actual attachment after completion.
- Adopt `test_active_job_excludes_other_hardware_apis_but_allows_cache_access`:
  hardware APIs fail BUSY without transfers during active work and after
  cancellation until the result is consumed. Cache queries remain available.
- Adopt `test_job_identity_exactly_once_and_rebinding_do_not_leak_context` and
  `test_legacy_sample_step_does_not_adopt_owner_job`: wrong tokens and rebinding
  cannot consume another operation; terminal results remain owned exactly once.
- Reject INA228's reset/reinitialize configuration replay and destructive alert
  read semantics. TUSB initialization/reconciliation remain read-only, ACK is
  explicit, and uncertain effects never cause automatic role/apply/ACK retries.

The peer workflow keeps native tests, CLI/parser/owner checks, firmware links,
documentation and packaging separate. It is evidence for useful test structure,
not proof of TUSB fixture behavior.

## Current TUSB implementation and failure seeds

Revisited `include/TUSB32x/TUSB32x.h`, `src/TUSB32xOperations.cpp`,
`src/TUSB32xProtocol.cpp`, `tools/tusb32x_hil_runner.py`,
`test/test_interrupt.cpp`, `test/test_profile_application.cpp`,
`test/test_port.cpp` and `test/CMakeLists.txt`.

- Actual port publication and IRQ sequencing live in `TUSB32xOperations.cpp`;
  the baseline's intended `TUSB32xPort.cpp` is not a current source file.
  Fixed 09/08/09 bookends publish one complete observation; failed refreshes can
  leave historical raw/time evidence. Require `invalidated=0`, required valid
  fields and a new completed acquisition before using it as fixture evidence.
- `everyAckByteAndStaged02`, `actualControlsWithoutTrust` and
  `serviceUsesFinalControlsAndBookends` verify ACK composition from actual control
  bits. `unrelatedProfileWritesPreserveLatch` isolates each non-ACK settings write.
  The runner must execute staged 30% with actual/applied 50%, ACK/service and
  verify 50% preservation through the real persistent C++ CLI fixture.
- `everyTransportFailure` injects all terminal callback errors at every IRQ
  phase, including acceptance-then-error at ACK. `interruptedBoundaries` covers
  cancel/end/deadline/invalidation. `finiteLatchRaces` models coalescing,
  reassertion and stuck pending. These are host models, not physical INT traces.
- Profile tests `prewriteAndEveryBoundary`, `faultsAtEveryWriteAndProofRead`,
  `interruptedBaseModeEvidence`, `tusb322ModeWriteFailures` and
  `tusb322ModeInterruptionBoundaries` retain partial/indeterminate effects and
  original failures. A failure after a write is not safe retry or restore proof.
  Base actual mode changes have detached precheck; a fixture must also prevent
  attachment racing that advisory observation.
- Current `validateWrite()` rejects every TUSB322I 08/09 write; interrupt
  admission rejects TUSB322I ACK/service before traffic. Keep capability denial
  tests distinct from absent-fixture results. Supported unrelated 0A operations
  must not inherit an invented blanket variant denial.
- Reuse the single `hil_runner` and `cli_transcript` registrations. The latter
  receives `$<TARGET_FILE:tusb32x_cli_host>` explicitly. Keep fake serial/clock,
  actual persistent CLI, firmware links and physical evidence separate.

## Archived TI source confrontation

Read selected tables/prose directly from the archived official PDFs with pypdf;
page numbers below are printed PDF pages. SHA-256 was recomputed and matched
[provenance](../reference/source/provenance.json). This is an archive review;
no new current-source download or physical result is claimed.

| Archived source | SHA-256 |
| --- | --- |
| [SLLSEQ8D, May 2017](../reference/source/TI-TUSB320LAI-HAI-SLLSEQ8D.pdf) | `62f7b3f65338e25ebd6fd46ef04474ba806b06b1cc04bc0b3d7b8191b01ca87d` |
| [SLLSEP2D, May 2017](../reference/source/TI-TUSB320LI-HI-SLLSEP2D.pdf) | `6e570c562b3bc9ee0c822353f3abebfad692ab27bfd862ff1c07da57e8c0ff81` |
| [SLLSEN9F, March 2022](../reference/source/TI-TUSB320-I-SLLSEN9F.pdf) | `1306e448c3f3b169f3b90854710033b9a1aede1d04ba35088f3d2e3ed260d2c4` |
| [SLLSEO5D, March 2021](../reference/source/TI-TUSB322I-SLLSEO5D.pdf) | `7cbd82a81e2b32f56caeca413a6be83372c91e401cae32a0cf85db0646dc9683` |

- LAI/HAI SLLSEQ8D Table 8 p17 and LI/HI SLLSEP2D Table 8 p17 distinguish
  advertisement 08[7:6] (`00` default, `01` 1.5 A, `10` 3 A, `11` reserved)
  from detected 08[5:4] (`00` default, `01` medium, `10` charge-through 500 mA,
  `11` high). Raw reserved accessory encodings 001..011 must survive as unknown.
  Debug encodings describe the device's local DFP/UFP role, not a second peer
  role. Compare the paired attachment/current applicability and validity.
- SLLSEQ8D and SLLSEP2D Table 9 p18 identify local Attached.SRC/Attached.SNK,
  orientation 0=CC1/1=CC2 and explicit write-one IRQ clearing. The reset default
  orientation is CC2 but proves no attached connector orientation. IRQ RCU is
  Read/Clear/Update, not evidence of read-to-clear. The note requires observing
  successful clearing/rewrite if needed; the core's bounded service is not a
  lossless event queue and must not gain an automatic retry loop.
- LAI SLLSEQ8D section 6.5 p5 footnote 1 requires VDD >=3.5 V for local 3 A
  advertisement. It does not impose that local advertisement condition on a UFP
  detecting a remote 3 A advertisement. CC categories prove neither measured
  load nor a USB PD contract. Fixture power policy remains application-owned.
- Original TUSB320/I SLLSEN9F section 7.3.1.2 p11 explicitly requires periodic
  I2C_SOFT_RESET for CURRENT_MODE_DETECT updates after attached partner-current
  changes. No reset period is supplied there. A physical current-change plan
  must record an explicitly selected reset and fresh identity/profile/port proof;
  a read-only/soak read cannot prove that update. Do not hide reset in refresh,
  ACK or cleanup, or generalize the original-part policy to later generations.
- TUSB322I SLLSEO5D Figure 6-3/Table 6-8 p17 conflict: diagram/prose imply RW
  advertisement while Type says R. Keep advertisement writes gated. Table 6-9
  pp18-19 explicitly describes IRQ W1C but only labels VCONN_FAULT RCU with no
  clear transaction. Keep all mixed 09 writes, including ACK/service and raw
  writes, gated until safe fault composition is established; do not infer that
  reading or IRQ ACK clears the fault.
- TUSB322I section 5.3 p4 requires 4.5..5.5 V device VDD. Section 5.5 p6 gives
  VCONN current-limit min/typ/max 225/300/375 mA, not a guaranteed 300 mA budget.
  Section 6.3.6 p13 requires the VBUS_DET resistor connection. Section 6.3.8 p13
  describes autonomous VCONN after source attachment/Ra and a board-provided 5 V
  VDD; section 5.3 specifies VCONN 4.75..5.5 V with VDD=5 V. General device VDD
  range alone does not prove that VCONN operating condition. Preserve exact
  wiring, straps, supply, board, part, adapter and fixture assumptions.

## Audit validation and limits

Validated this report's UTF-8 without BOM, LF/final newline, trailing whitespace,
relative Markdown links and cited source/test symbol existence. These are
documentation/source checks only. No peer test, C++ build, firmware link, serial
port, connector, INT, VBUS or VCONN qualification was run. Hardware is NOT RUN.
The lead records prompt-30 software checks, final commit and synchronization.
