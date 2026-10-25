# Prompt 6 focused peer re-audit

Reviewed 2026-10-25 for admission, results, owner time, retained effects,
configuration evidence and diagnostics. Read repository/parent AGENTS.md, prompt
index, COMMON, AGENT_WORKFLOW, prompt 6 and the relevant ENGINEERING_BASELINE
sections. Revisited execution-5-peers and execution-1-owner, then inspected the
current declarations, implementations, test bodies, examples, runners and build
registration below. Paths are relative to `Projects/`. All sibling access was
read-only: no fetch, build, test, remote CI query or hardware action.

## Captured state and changed-source review

| Peer | Full HEAD | Branch / upstream | Worktree; local ahead/behind |
| --- | --- | --- | --- |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `main` / `origin/main` | clean; 0/0 |
| LSM6DS3TR | `e262bf22f3d0cf035fbbca10f45b3e0752bf2bac` | `main` / `origin/main` | clean; 0/0 |
| ADS1115 | `235af3e8c722661a89a44766a4ccd903a654f6fe` | `main` / `origin/main` | clean; 0/0 |
| INA3221 | `40838b56e41c25183478b086e9e214ceffe3290c` | `main` / `origin/main` | clean; 0/0 |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `main` / `origin/main` | clean; 0/0 |
| FieldCore-node | `a002abae533b0aa767c235c2197d3b548bdcdd83` | `rodtester` / `origin/rodtester` | clean; 0/0 |

No fetch was performed; equality with locally stored upstream refs is not a
claim about current server state. TUSB32x was clean at this audit's first edit,
HEAD `87979dc07fc319603f0bd6ed21224b45ea20d1d8`, on the existing execution branch
`promptset/tusb32x-implementation-20261023`. The lead owns synchronization and
shared-contract edits.

The five library HEADs equal execution-5-peers. Explicit diff checks of AGENTS,
include/src/test/examples/tools/scripts/workflows and root registration files
against those recorded HEADs were empty. FieldCore-node advanced from prompt 1's
`4e6a05e769383b95046df3936eee81984f4d9aa7`: in the same inspected source scopes,
only AGENTS and documentation/evidence tooling/tests changed. Its three named
I2C contract headers, owner/module implementation and routing fixture did not.
The AGENTS addition directs readers to its repository map, runtime connections
and focused verification guides; those current navigation entries were checked.
Historic reports were reused only after these comparisons and direct focused
source reads. No inspected evidence requires changing TUSB32x's settled baseline.

Read all six root AGENTS files. Shared useful rules are borrowed application
transport, serialized owner calls, fixed storage, bounded cooperative work,
preserved failures/effects and honest hardware qualification. Peer recurring CI
rules, physical-transaction vocabulary, sensor recovery and compatibility APIs
do not override TUSB32x's callback accounting or prompt-34-only remote CI policy.

## Admission, timing and retained results

LDC `include/LDC1614/LDC1614.h` and
`src/LDC1614.cpp::_startJob/poll/cancelJob/takeResult/_finishJob/_tracked` use
nonzero caller IDs, one active job and a fixed result ring. `poll` applies an
unsigned half-range deadline comparison and caps callback timeout, but distributes
the remaining allowance across multiple callbacks using the same supplied time.
`takeResult` clears output before checking availability; `end` clears results,
effects and totals. Adopt explicit bounded jobs and neutral saturating totals;
reject caller IDs, multiple-result admission, stale-time multi-callback execution,
failed-output mutation and evidence-erasing end. TUSB32x requires exact instance
tokens, one retained result, unchanged failed outputs and retained effects/results.

Read LDC `test/test_basic.cpp` bodies and their actual RUN_TEST registration:
`test_deadline_timeout_is_bus_silent_wrap_safe_and_caps_callback_timeout`,
`test_apply_exact_counts_failure_cancel_and_ambiguous_write_provenance` and
`test_transport_stats_are_diagnostic_only_and_failures_never_suppress_requests`.
They check zero-callback expiry, timeout clamping, every apply failure position,
original detail, no retry, bus-silent cancellation and authorized I/O after
repeated failures. These are useful regression patterns, not TUSB32x timing proof.

LSM `include/LSM6DS3TR/LSM6DS3TR.h` and
`src/LSM6DS3TR.cpp::_checkStart/_start/_read/_writeByte/_stepResetBoot/poll/
cancelActiveJob/takeResult` use uint64 instance tokens and one retained terminal
slot. Take checks availability and exact token before copying and returns delivery
OK. Write effects are marked only after the callback-ceiling guard. Its reset
wait is armed from a later caller sample with a separate 1 ms margin. Adopt these
causal boundaries. Reject its clearing of failed-start output tokens, exhaustion
reported as RESULT_NOT_AVAILABLE, unbind discarding results, fixed callback timeout,
direct `nowMs >= deadlineMs`, saturating wait dates and sensor restoration/replay.
TUSB32x owns TOKEN_EXHAUSTED, half-range time and unknown completion-time validity.

Read LSM `test/test_basic.cpp::test_reset_orders_required_modes_and_enforces_no_i2c_gate`
and `test_self_test_restores_verified_not_newly_desired_cancelled_profile`, with
their RUN_TEST entries. The first arms at a later sample and forbids I/O through
arm+15 before permitting it at arm+16. The second preserves desired G_16 while
restoring previously verified G_4 after a cancelled admission. Adopt separate
desired/verified evidence; omit sensor self-test restoration. Integer peer tests
do not prove TUSB32x's required sub-ms callback/wait vectors.

TMP `src/TMP1x2Operations.cpp::admit/takeResult` preserves failed admission output
and checks availability/token before copying the terminal result. Its registered
`test/jobs/test_jobs.cpp::admissionAndResults` checks token sentinels, pending
backpressure, zero-budget silence, wrong-token output preservation, successful
exact take and repeated-take RESULT_NOT_AVAILABLE without output mutation. Adopt
those tests and delivery semantics. Reject uint32 token/time, combined BUSY for
active/result slots, its exhaustion error and initialization/profile replay.

## Desired, observed and verified settings; replay differences

ADS `include/ADS1115/ADS1115.h::AppliedProfileSnapshot/OperationResult` and
`src/ADS1115.cpp::bind/startInitialize/startApplyProfile/startRecover/takeResult`
separate binding, candidate and applied profile. Full successful application
commits candidate to desired/applied and advances generation; admission already
sets APPLYING. Its registered
`test/test_basic.cpp::test_owner_safe_apply_profile_success_commits_candidate_snapshot_and_generation`
checks unchanged prior profile/generation at admission and the final generation.
Adopt copied candidate and explicit verified publication. TUSB32x instead retains
desired at admission and preserves proved actual trust until possible mutation;
initialize is read-only and reset does not replay desired.

Read ADS registered delayed/ambiguous-start tests
`test_owner_safe_cancel_after_ambiguous_start_preserves_transport_error` and
`test_owner_safe_ambiguous_delayed_start_uses_post_callback_quiet_interval`.
The latter applies a failed write, moves the callback clock 200 to 209, waits
without I/O from a fresh boundary and retains I2C_TIMEOUT/detail -202. Adopt fault
injection after possible commitment and primary transport-status retention.
Reject its conversion-reconciliation lifecycle as a TUSB32x cancellation policy.

INA `include/INA3221/INA3221.h::AppliedConfigState/HardwareEffect/JobResult` and
`src/INA3221.cpp::startReconcile/_markRegisterUnknown/_markRegisterDirty` plus the
profile engine's PROFILE_READ/WRITE/VERIFY stages preserve certainty and mismatch
evidence. A read can disprove old applied state before a later write. Reconcile
reads the retained profile, writes differences and verifies: it is replay-capable.
Its registered `test/test_owner_operations.cpp::test_owner_reconcile_reads_first_and_verification_mismatch_is_partial`
first checks 11 reads/no writes for a match, then explicitly expects a corrective
write and mismatch/PARTIAL after changed readback. Adopt evidence invalidation
when a read disproves old trust; reject that replay contract. Complete TUSB32x
read-only proof may establish KNOWN actual controls while desired comparison
fails; an unrelated read or incomplete transient proof cannot establish trust.

## Consumer ownership and lifetime

Re-read FieldCore-node
`include/TunnelMonitor/i2c/I2cOwnerTransport.h`, `I2cDeviceBinding.h`,
`I2cLifetime.h`, the current `src/devices/i2c/Ads1115Module.cpp` bind/deadline
adapter and `test/host/rodtester/i2c_routing.cpp` retention/time tests.
`I2cBusObservation` returns write effect, byte counts and authoritative
completedUptimeMs. These are richer observations than TUSB32x Status-only
callbacks; a bridge must not manufacture them or infer address-NACK phase from
generic Nack. The module owns its driver/context, adapts transport through thunks
and obtains time from its external owner. Keep that glue outside TUSB32x core.

`I2cDeviceBinding` supplies owner-serialized start/poll/take/cancel/invalidation
and a bus-silent optional wait hint. Adopt external scheduling and parked work
retaining effects; reject importing FieldCore DeviceId/routes/queues/templates.
Its cancellation fixture may need additional bus-silent polls to hand off a
cancelled module result; TUSB32x's own cancel contract remains immediate/local.

`I2cLifetime.h`'s `i2cResumeTime` converts modulo-uint32 due ticks using a checked
horizon below 2^31 rather than zero-extension. `timeAndScan` checks wrap from
0xFFFFFFFE to 0x100000003, unchanged output on ambiguous half-range input,
over-horizon rejection and late callbacks retaining committed effects. The
routing main actually invokes `retainedIdentityAndTransportFence` and
`timeAndScan`. These support explicit correlation, clock adaptation and lifetime
rules; they prove no TUSB32x adapter or linked firmware.

## Two-peer decisions and integration records

| Material choice | Current peers compared | Selected TUSB32x contract |
| --- | --- | --- |
| Admission/result ownership | LSM one slot and TMP exact take versus LDC ring | One nonzero instance uint64 token; distinct BUSY/RESULT_PENDING; exact poll/cancel/snapshot/take; failed outputs unchanged. |
| Owner time and callback bounds | LDC deadline sharing; LSM fresh reset anchor; ADS delayed callback | Immutable half-range uint64 deadline, one fresh post-callback owner turn, aggregate timeout clamped to remaining time, separate 1 ms hardware margin. |
| Desired versus actual state | ADS candidate commit; INA disproved certainty; LSM cancelled desired versus verified | Desired retained at admission; observed and verified publications independent; pre-write rejection/cancel preserves proved actual state. |
| Reconciliation and reset | INA corrective reconcile; ADS recover replay; LSM reset/recover | Reconcile only observes; reset/recovery are explicit silicon operations with no desired replay. |
| Effects and diagnostics | LDC each-phase fault provenance; INA PARTIAL/uncertain effects | Preserve original transport failure/detail, confirmed earlier effects and uncertain acceptance independently from outcome. |
| Health and source totals | LDC diagnostic-only failures; INA explicit I/O after OFFLINE | Count every callback in totals; raw Sync excluded only from managed health; OFFLINE neither gates nor retries. |
| Consumer lifetime | FieldCore borrowed transport and TMP exact take | External owner serializes devices, takes each result and correctly extends clocks; core has no consumer dependency. |

Re-read command producers and matching parser meanings: LDC
`examples/common/Ldc1614Cli.cpp::printResult` and `tools/ldc1614_hil_runner.py`
distinguish command/session envelopes from operation outcome/effects and require
firmware Git/clean records. LSM `examples/01_basic_bringup_cli/main.cpp::printProfiles/printTerminalResult`
and `tools/run_hil.py::require_success` distinguish staged/desired/verified,
token/state/status and changed evidence. TMP `examples/common/Tmp1x2Cli.cpp` and
`tools/hil_tmp1x2_runner.py::operation_admission/operation_status` match fresh
token/kind, with firmware identity checks separate. ADS
`examples/02_owner_safe_poll/main.cpp::consumeTerminalResult` takes the exact
active token before replacement; `tools/run_i2c_hil.py` validates firmware's
`ADS1115 library commit:` record and preserves NOT_RUN. INA
`examples/01_basic_bringup_cli/main.cpp::printOwnerResult` emits separate
status/effect, while `tools/hil_cli_runner.py::extract_hil_frame` matches both
HIL_BEGIN and HIL_END token/sequence. FieldCore's
`test/python/test_hil_runner_verdicts.py` and imported
`scripts/hil_48h_stress.py` distinguish exact owner terminals and firmware
identity. Adopt correlation and provenance; retain TUSB32x CLI_CONTRACT grammar
and one core engine rather than a peer's extra firmware scheduler.

Checked root CMake, platformio.ini and `.github/workflows/ci.yml` registration
for the five libraries. LDC/LSM/ADS/INA register native Unity with source builds;
their named tests are called by RUN_TEST (INA through runOwnerOperationTests).
ADS native C++11 is deliberately rejected; TUSB32x is C++17. TMP registers its
jobs executable as CTest `jobs`, tests default OFF, and workflow CMake/CTest uses
strict warnings and ASan/UBSan. Peer runner checks are registered separately:
LDC test_ldc1614_hil_runner.py, LSM test_run_hil.py, ADS --parser-test and INA
--parser-self-test; TMP CTest hil_runner uses test_hil_tmp1x2_runner.py.
FieldCore's `test/python/test_rodtester_host.py::test_i2c_routing` compiles the
actual owner/route adapter with its pinned dependency, executes the fixture and
requires PASS routing. `scripts/verify.py` and `.github/workflows/verify.yml`
register repository verification. This is source review, not executed validation.

## Validation boundary

PASS: current local revision/branch/upstream/dirty capture, changed-source
comparison and focused source/test/registration review; report whitespace check
and existence checks for 20 referenced source paths. Final sibling recheck
confirmed the same HEADs and clean worktrees.
Peer tests/builds, remote CI, firmware and hardware: NOT RUN. The audit selects
conventions; it proves no TUSB32x runtime, protocol, SDK or hardware behavior.
