# Prompt 12 current peer and observation review

Read-only review on 2026-10-26 for profile observation and read-only
reconciliation. This report reopens current sources; earlier audit reports are
context, not substitutes for this inspection. No sibling was edited, fetched,
built or tested. No remote CI status was inspected.

## Repository state and instructions

TUSB32x started this review at `921132461e676ea2f05f7c4e6ed88edc641a294d`,
branch `promptset/tusb32x-implementation-20261023`, upstream
`origin/promptset/tusb32x-implementation-20261023`. The index/worktree were
clean at that observation; production, tests and documentation subsequently
changed concurrently under the lead's disjoint ownership. Git mutations and
synchronization belong to the lead.

Reopened `../AGENTS.md`, repository `AGENTS.md`, `docs/prompts/COMMON.md`,
`AGENT_WORKFLOW.md`, the prompt index, `12-profile-observation.md`, baseline
profile/generation/state/method/time sections, and relevant progress/decisions.
Sibling AGENTS require framework-neutral injected transport, serialized owner
work, explicit effects, bounded callbacks and accurate evidence. Their recurring
CI rules do not override this repository's prompt-34-only remote checkpoint.

| Current peer | Branch | HEAD | Index/worktree |
| --- | --- | --- | --- |
| ADS1115 | main | 235af3e8c722661a89a44766a4ccd903a654f6fe | Clean |
| LSM6DS3TR | main | 362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc | Clean |
| INA3221 | main | 41b6184b75f3c152611d4236f7ea41a5ce77bc2c | Clean |

Each state comes from current `rev-parse HEAD`, `branch --show-current` and
`status --short`. All three current revisions differ from the original baseline's
dated October 23 evidence and were inspected directly.

## Public declarations, implementation and tests

ADS1115 `include/ADS1115/ADS1115.h::AppliedProfileSnapshot`,
`src/ADS1115.cpp::getAppliedProfile`, candidate verification/publication and
`takeResult` distinguish a committed record from current trust. Snapshot Doxygen
explains that a generation can advance on verification before a subsequent data
read fails. The implementation publishes candidate/desired/applied settings only
after verification. Its `test/test_basic.cpp` registers
`test_owner_safe_apply_profile_success_commits_candidate_snapshot_and_generation`,
`test_threshold_observer_does_not_poison_cached_recovery_profile`,
`test_get_applied_profile_leaves_output_unchanged_when_unbound`, and cancellation
at pending stages. Adopt separate historical values/current usability, output
preservation, exact result ownership and explicit commit boundaries. Reject its
wrapping generation policy and recovery replay: TUSB generations saturate and
read-only reconciliation never writes or adopts a candidate.

LSM6DS3TR `include/LSM6DS3TR/LSM6DS3TR.h::startReconcile` and
`src/LSM6DS3TR.cpp::startReconcile/_stepConfigure(nowMs,true)` were followed
through the actual helper. Reconciliation reads the main-bank selector, identity
and all managed registers; `reconcileOnly` excludes writes. A mismatch records
expected/observed evidence and invalidates trust. A final caller-time boundary
precedes publication. `test/test_basic.cpp` directly registers
`test_reconcile_is_read_only_and_only_full_verification_clears_unknown`,
`test_reconcile_known_profile_preserves_generation_and_settle_evidence`, and
`test_reconcile_preserves_an_existing_unexpired_settle_gate`. Adopt complete
identity/control proof, no writes, literal trace checking and fresh-time
publication. Reject its prerequisite for a retained desired profile, publication
of desired only on equality, sensor settling state, and unchanged-generation
reconciliation semantics. TUSB must establish KNOWN actual controls independently
of desired equality and advance verifiedGeneration on complete proof. LSM
reconciliation is read-only; it must not be confused with its separate replaying
recover/configure paths.

INA3221 `include/INA3221/INA3221.h::AppliedConfigState/HardwareEffect/JobResult`,
`src/INA3221.cpp::startReconcile/_startProfileJob` and the
`PROFILE_READ/PROFILE_WRITE/PROFILE_VERIFY` path show read-first reconciliation
that writes a differing desired register and verifies it. The read can invalidate
old applied evidence before a later failure. `test/test_owner_operations.cpp::
test_owner_reconcile_reads_first_and_verification_mismatch_is_partial` explicitly
scripts eleven reads for unchanged state and a read/write/read when it differs,
then checks PARTIAL and full mismatch evidence. Its
`test/support/ScriptedTransport.h` is the exact-trace precedent. Adopt adverse
observation invalidating old trust, retained status/detail and independent effect
classification. Reject profile-replaying reconciliation, automatic cleanup,
initialization prerequisite and multi-callback owner turns. USB role/reset
uncertainty cannot be repaired by silently replaying a requested image.

## Owner examples, CLI and Python sources

Reopened ADS `examples/02_owner_safe_poll/main.cpp` result take and explicit
`startRecover` scheduling, Arduino `examples/01_basic_bringup_cli/main.cpp`,
and `tools/run_i2c_hil.py` recovery/verification and cancellation checks. The
runner explicitly describes recovering by reapplying the cached profile and
warns that address ACK is not identity proof. Keep result ownership and explicit
application recovery decisions; do not import its corrective command semantics
or ADC plausibility into TUSB identity proof.

Reopened LSM `examples/02_owner_soak/main.cpp`,
`examples/common/ProfileCli.h`, and `tools/run_owner_soak.py`. The example checks
result generation and paired probe/reconcile counts; the runner parses sequence,
generation, operation/contract failures and transport totals. CLI drafts are
updated atomically after complete validation. Adopt separation of staged input,
observed output and verified evidence; do not present these peer firmware records
as TUSB runner or physical qualification. TUSB CLI and Python sessions remain
later prompts.

Reopened INA `examples/01_basic_bringup_cli/main.cpp`, native
`examples/esp_idf/basic/main/main.cpp`, and `tools/hil_cli_runner.py`. The runner's
`extract_hil_frame` matches token and sequence and checks terminal status. Its
OWNER-026 through OWNER-028 start/service/take reconciliation explicitly. Adopt
fresh request/result correlation later; reject using its `recover` or
`job reconcile` write semantics as TUSB read-only behavior. No command alias,
firmware scheduler or second result store is added in prompt 12.

## Registration and checked-in CI definitions

Reopened all three `platformio.ini`, root CMake files and
`.github/workflows/ci.yml`, plus current TUSB root/test CMake definitions. The
peers compile native source and register real Unity cases with RUN_TEST;
LSM also has compile/profile CLI fixtures. TUSB retains tests OFF by default,
explicit real sources, strict private warnings, active Release assertions,
finite CTest timeouts, and the registered `tusb32x_operations` suite. Compile-only
future API fixtures never count as runtime proof.

The peer workflows pin PlatformIO Core 6.1.19, invoke native tests, timing and
CLI/IDF contract guards and documentation checks. Current ADS native-IDF uses
v5.3.5 with a pinned container digest, LSM uses v5.4.4 through the IDF action,
and INA uses v6.0.1. These are reviewed definitions, not successful run evidence.
TUSB has no checked-in `.github/workflows` directory yet; its dedicated CI
implementation/checkpoint remains prompt 34. Local validation and final commit/
sync evidence belong to the lead's validation/progress records.

## Archived TI control provenance and selected boundary

Reopened bytes from the four archived PDFs with pypdf, not copied public driver
masks: B SLLSEN9F March 2022 pp17-19, L SLLSEP2D May 2017 pp18-20,
A SLLSEQ8D May 2017 pp18-20, V SLLSEO5D March 2021 pp18-20.
`docs/reference/source/provenance.json` retains official URLs, dates and SHA-256
hashes. L/A Table 10 and V Table 6-10 separately define 0A DISABLE_TERM and the
self-clearing reset command. B Table 7-7 and L/A Table 11 independently define
45 bit2 DISABLE_RD_RP. Both disable controls must qualify enabled termination;
V has no 45 observation. V Table 6-10 defines both mode00 and mode11 as DRP;
semantic comparison must preserve their raw difference without inventing a
persistent mismatch. V Table 6-9 defines VCONN fault in the mixed 09 byte but
provides no general clear transaction; read availability does not grant writes.
No vendor source was changed or copied into production code.

The selected P plan is scalar 08/09/0A, plus 45 only on B/L/A. F is identity
(and A0 on L/A/V) followed by P. These are library proof plans, not a vendor
claim of atomic snapshot or physical current freshness. P publishes observation
without identity readiness, desired adoption or trust promotion. Complete
contradictory controls invalidate old current trust. F can establish initialized
and KNOWN actual controls when its full field/effect proof is complete. Reserved
encodings retain raw bytes with honest typed invalidity. Prior pending reset
requires its conservative 96-tick interval and one F; reset first discovered in
that F remains unresolved without appending a second pass. No role/reset/recovery
replay or hidden write is permitted.

Public desired input arrives only with apply in prompt 13. Desired mismatch and
interrupted configuration/mode/recovery-write tests remain owned by prompts
13-14; no setter, private-state injection or test-success claim is invented for
them here. Hardware and linked framework firmware remain separate evidence.

## Independent implementation recheck (2026-10-26)

The follow-up audit reread original prompt 12 and the actual implementation
diff from `9211324` to `db66500`. Current ADS1115, LSM6DS3TR and INA3221 HEADs,
main branches and clean worktrees still match the table above. Reopened the
profile/reconcile declarations, matching source/tests, owner examples, CLI/runner
semantics, AGENTS and registration. The adopted/rejected rules remain unchanged:
separate historical observations from current trust, retain terminal results,
and keep TUSB reconciliation read-only. In particular, INA's desired replay
remains unsuitable and LSM's desired-dependent proof is not TUSB's actual-state
proof. No peer source was edited, fetched, built or tested.

The independent audit found no runtime defect in the delivered P/F engine.
Additional public probes and a compact retained all-setting publication table
check the operation boundary independently of the pure decoder tests. Later
desired-write/history tests stay with their original owning prompts; no public
test seed or future method was added to make those cases reachable early.
