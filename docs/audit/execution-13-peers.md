# Prompt 13 current peer and profile-write review

Read-only source review on 2026-10-26 for explicit profile application and desired
verification. Earlier audits were revisited, then the current sources below were
reopened. No sibling was edited, fetched, built or tested. No remote CI run was
dispatched or inspected.

## Repository state and instructions

TUSB32x began this review at `724b7668aa47a6d927e6e5521e84143fe0295a2b`,
branch `promptset/tusb32x-implementation-20261023`, upstream
`origin/promptset/tusb32x-implementation-20261023`. Its index/worktree were clean
at the initial observation. Concurrent prompt 13 edits use disjoint ownership;
the lead alone handles Git mutations, validation, progress and synchronization.

Reopened `../AGENTS.md`, repository `AGENTS.md`, prompt index, `COMMON.md`,
`AGENT_WORKFLOW.md`, `13-profile-application.md`, baseline profile/effect/time
contracts and `device-behavior.md`. The peer AGENTS retain injected transport,
serialized owner access, explicit uncertainty and honest validation limits.
Their recurring CI rules do not override this execution's prompt-34-only remote
checkpoint.

| Current peer | Branch | HEAD | Index/worktree |
| --- | --- | --- | --- |
| ADS1115 | main | 235af3e8c722661a89a44766a4ccd903a654f6fe | Clean |
| LSM6DS3TR | main | 362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc | Clean |
| INA3221 | main | 41b6184b75f3c152611d4236f7ea41a5ce77bc2c | Clean |
| PCA9555 | main | 498178e4d9e4acb6fab7d8b816ad5021efe8806a | Clean |

Each state comes from current `rev-parse HEAD`, `branch --show-current` and
`status --short`. These revisions were inspected directly without synchronization
or remote-run polling in the siblings.

## Admission, generations, writes and preservation

ADS1115 `include/ADS1115/ADS1115.h::startApplyProfile/AppliedProfileSnapshot`
and `src/ADS1115.cpp::startApplyProfile` validate the whole candidate before
starting and keep a private candidate until readback. The commit path copies
candidate to desired/applied after verification. `test/test_basic.cpp` registers
`test_owner_safe_apply_profile_success_commits_candidate_snapshot_and_generation`
and cancellation cases at pending stages. Adopt complete validation, atomic
output handling, retained results and explicit publication boundaries. Reject
its commit-time desired adoption, wrapping generation policy and replaying
recovery: TUSB retains desired and advances its saturating generation at accepted
admission, even if cancelled before traffic. Proved old actual controls remain
usable until the first possible mutation.

LSM6DS3TR `src/LSM6DS3TR.cpp::_writeByte/_stepConfigure` and
`include/LSM6DS3TR/LSM6DS3TR.h` separate uncertain writes and current trust from
historical verified configuration. `_stepConfigure` yields at final readback and
arms settling from later caller time. `test/test_basic.cpp::
test_configuration_settle_arms_after_readback_with_changing_budgets` advances
time within callbacks, changes the next budget between zero and 255 and proves
that the later sample anchors the interval. The same file registers
`test_reconcile_preserves_an_existing_unexpired_settle_gate` and
`test_reconcile_is_read_only_and_only_full_verification_clears_unknown`.
Adopt independent callback-time tests, zero-budget local transitions and retained
uncertainty. Reject sensor settling durations, desired-dependent actual proof
and its multi-callback poll allowances. TUSB's TI role wait follows completed
mode write, not final readback; each poll yields after at most one callback.

INA3221 `include/INA3221/INA3221.h::HardwareEffect/JobResult` and
`src/INA3221.cpp::_finishJobFailure/_finishJobSuccess/_writeMayHaveReachedDevice`
preserve the failure status while separating NONE, PARTIAL and INDETERMINATE.
`test/test_owner_operations.cpp::
test_owner_every_managed_write_ambiguity_is_indeterminate_without_retry` uses
`test/support/ScriptedTransport.h::WriteEffect::COMMIT_BEFORE_STATUS` at each
managed write; backend timeout can follow a changed register. Adopt separate
terminal outcome/effect, unchanged full backend detail, no retry and independent
applied-then-error fixtures. Reject its read-first desired-replaying reconciliation
and sensor register masks. A TUSB mode/reset procedure cannot be replayed to
repair uncertainty implicitly.

PCA9555 `include/PCA9555/PCA9555.h::startApplyImage/startVerifyImage`,
`src/PCA9555.cpp::_compareObserved` and `test/test_basic.cpp::
test_verify_caller_mismatch_preserves_a_truthful_protocol_shadow` distinguish a
caller expectation mismatch from truthful actual state. The test verifies a
different expected output, retains actual shadow validity, then proves a later
cached update preserves actual bits. `test_verify_matching_external_image_reconciles_observation_and_shadow`
and `test_register_pair_round_trip_and_odd_start_wrap_match_chip_protocol`
exercise external-image proof and nonzero bit preservation. Adopt these
independent actual/expected and preservation tests. Reject GPIO pair semantics,
idempotent reassertion and pointer-park cleanup. TUSB uses scalar mixed-register
composition, writes differences only and never adds cleanup after a failure.

## Examples, CLI, Python runners and registration

Reopened ADS `examples/02_owner_safe_poll/main.cpp`,
`examples/01_basic_bringup_cli/main.cpp` and `tools/run_i2c_hil.py`; LSM
`examples/02_owner_soak/main.cpp`, `examples/common/ProfileCli.h` and
`tools/run_owner_soak.py`; INA `examples/01_basic_bringup_cli/main.cpp` and
`tools/hil_cli_runner.py`; PCA `examples/01_basic_bringup_cli/main.cpp` and
`tools/run_i2c_hil.py`. ADS explicitly schedules recovery after taking a result;
LSM commits draft edits only after validation and correlates sequence/generation;
INA's CLI labels reconciliation as reapply/verify and its runner correlates token
and sequence. PCA takes its bounded apply result and its runner distinguishes
verdict/evidence/timing. Adopt explicit owner result consumption and future
fresh-request correlation; reject automatic recovery policy, replaying command
semantics and host identity as proof of flashed firmware. No TUSB CLI or runner
is added by prompt 13.

Reopened each peer's `platformio.ini`, root CMake and
`.github/workflows/ci.yml`, plus real `RUN_TEST` registration in the named
suites. Workflows use PlatformIO Core 6.1.19 and native tests, guards and Doxygen.
ADS pins native-IDF v5.3.5 with a digest; LSM uses v5.4.4; INA uses v6.0.1;
PCA's action specifies v5.4. These are inspected definitions, not run evidence.
Current TUSB root/test CMake explicitly registers four C++ suites including
`tusb32x_operations`, Python version fixtures and the Windows launcher suite.
It retains strict private warnings, active Release assertions, tests OFF by
default and no Python dependency for ordinary consumption. No checked-in TUSB
workflow exists yet; CI implementation/checkpoint remains prompt 34.

## TI role procedures and public-code counterexample

Reopened archived PDF bytes with pypdf: B SLLSEN9F March 2022 p14;
L SLLSEP2D May 2017 p14; A SLLSEQ8D May 2017 p14; V SLLSEO5D March 2021
pp10-11. Fresh official downloads on 2026-10-26 matched all four archived
SHA-256 values in `docs/reference/source/provenance.json` exactly. Official
sources remain [B](https://www.ti.com/lit/ds/symlink/tusb320.pdf),
[L](https://www.ti.com/lit/ds/symlink/tusb320li.pdf),
[A](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf) and
[V](https://www.ti.com/lit/ds/symlink/tusb322i.pdf).

B allows MODE_SELECT changes only while unattached. The selected library
precheck applies only when mode actually changes; software cannot make cable
state atomic. L/A explicitly require termination-disable, mode change, 5 ms wait
and re-enable in attached and unattached states. V explicitly specifies
disable/mode/enable for DFP/UFP without that wait; it does not close the direct
non-reset return-to-DRP gate. Nonzero actual 0A=C2 changing to DFP therefore
preserves controls as C3/E3/E2; only L/A has the six-tick software wait. Holding
requested disableTerm omits enable. A separate 45 disable must remain distinct.
V's autonomous VCONN behavior does not move board power policy into core.

Reopened pinned TI-hosted Linux
[`extcon-usbc-tusb320.c`](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/drivers/extcon/extcon-usbc-tusb320.c)
and `include/linux/regmap.h` through the GitHub connector. The latter forwards
mask/value through `regmap_write_bits`; in `tusb320l_set_mode`, a failed mode
write jumps to re-enable and replaces the original return code. Successful
cleanup can hide failure. Retain the first TUSB backend failure, possible write
effects and no cleanup instead. No public driver code was copied into core.

No new evidence closes V_ADVERTISEMENT, V_FAULT_CLEAR or V_RETURN_DRP. Apply
rejects changed advertisement, every 09 write and direct DRP transitions locally;
unchanged proved gated fields still permit independently supported work.
Complete F can establish KNOWN actual configuration despite desired mismatch;
read-only reconciliation can observe partial apply after its required wait,
allowing a later explicit apply without replaying the interrupted job. Hardware,
linked framework firmware and remote CI results are not established by this
review; exact local validation and commit/sync evidence belong to lead records.

## Independent audit recheck (2026-10-26)

From synchronized TUSB32x commit `7ea5583`, the fresh scope reviewer independently
rechecked all four peer HEADs, main branches and clean states above. Reopened
the named declarations, implementations, registered tests, examples, runners,
instructions, build files and workflow definitions; selected and rejected rules
remain supported. No sibling was changed, fetched or built. This recheck did
not redownload TI sources or query remote CI. The lead and separate reviewers
audited the actual prompt-13 implementation diff and its required test coverage;
current outcomes are in the implementation progress and validation records.
