# Core protocol conformance

Prompts 16-18, independently re-audited 2026-10-27 from starting commit
`91e8eb4a1b070c04ca5a62453916978a72da578d`. This matrix maps TI/software
requirements to implemented APIs and named runtime cases. Tests use independent
literal bytes and field tables; production encoders are not expected-value
oracles. Evidence is host software only. Physical timing, electrical behavior,
framework firmware and board policy require separate qualification.

## Sources and registered commands

TI source abbreviations refer to the exact archived PDFs and
[provenance](reference/source/provenance.json):

- B: [SLLSEN9F](reference/source/TI-TUSB320-I-SLLSEN9F.pdf), March 2022,
  TUSB320/I; §7.4.3 p14, Table 7-7 pp16-19, timing p7.
- L/A: [SLLSEP2D](reference/source/TI-TUSB320LI-HI-SLLSEP2D.pdf) /
  [SLLSEQ8D](reference/source/TI-TUSB320LAI-HAI-SLLSEQ8D.pdf), May 2017,
  LI/HI and LAI/HAI; §7.3.2 p14, Tables 7-12 pp16-20, timing p6.
- V: [SLLSEO5D](reference/source/TI-TUSB322I-SLLSEO5D.pdf), March 2021,
  TUSB322I; §6.3.2.1-3 pp10-11, Tables 6-7 through 6-11 pp17-20, timing p7.
- Software: [owner/effect/time baseline](prompts/ENGINEERING_BASELINE.md),
  [register map](register-map.md), [procedure contract](device-behavior.md),
  [raw safety](api-contracts.md), [chip counterexamples](prompts/CHIP_EVIDENCE.md).

File abbreviations in the tables are actual files under `test/`: P =
`test_protocol.cpp`, C = `test_codecs.cpp`, O = `test_operations.cpp`, F =
`test_profile_observation.cpp`, A = `test_profile_application.cpp`, R =
`test_reset_recovery.cpp`, S = `test_port.cpp`, I = `test_interrupt.cpp`, D =
`test_diagnostics.cpp`, Vd = `test_validators.cpp`. A name after `::` is a real
function called by that executable's main or linked suite runner.
N = `test_protocol_conformance.cpp`, the public operation regression linked into
the existing operations executable; it adds no new suite or production interface.

| Command key | Registered suite selection | Debug result for final audit code |
| --- | --- | --- |
| PROTOCOL | `ctest --test-dir build/audit161718-final -C Debug -R '^tusb32x_protocol$' --output-on-failure` | PASS through ALL |
| OPERATIONS | `ctest --test-dir build/audit161718-final -C Debug -R '^tusb32x_operations$' --output-on-failure` | PASS through ALL |
| PORT | `ctest --test-dir build/audit161718-final -C Debug -R '^tusb32x_port$' --output-on-failure` | PASS through ALL |
| DIAGNOSTICS | `ctest --test-dir build/audit161718-final -C Debug -R '^tusb32x_diagnostics$' --output-on-failure` | PASS through ALL |
| VALIDATORS | `ctest --test-dir build/audit161718-final -C Debug -R '^tusb32x_validators$' --output-on-failure` | PASS through ALL |
| ALL | `ctest --test-dir build/audit161718-final -C Debug --output-on-failure` | PASS, 8/8 registered tests |

The lead configured `build/audit161718-final` with Ninja Multi-Config and
`TUSB32X_BUILD_TESTS=ON`, built Debug, then ran ALL successfully. The focused
selections above show registration and how to rerun a suite; they are not claimed
as separately executed commands. Reproduce the standard configuration with
`cmake -S . -B build/audit161718-final -G "Ninja Multi-Config" -DTUSB32X_BUILD_TESTS=ON`
and `cmake --build build/audit161718-final --config Debug`. Toolchain details and
Release evidence belong in [validation](validation.md); Release checks stay
active. Compile-only header/signature checks are not runtime protocol tests.
Remote CI remains prompt 34 work.

Debug runtime counts include 161,758 protocol checks, 8,602,548 codec checks,
622 new public-address checks, 20,490 owner checks, 35,454 profile-observation
checks, 93,440 apply checks, 88,779 reset/recovery checks and 75,426 diagnostics
checks. The registered port suite also passed; its detailed counts remain in
validation. This audit fixes stale literal 322I mode-image permission after equivalent DRP
recoding, and adds variant-specific failure/interruption tests. Public signatures
and silicon gates are unchanged.

## Variant, framing and field matrix

| TI/software requirement | Variant/capability | Public API | Named runtime coverage | Command |
| --- | --- | --- | --- | --- |
| Seven-bit low/high addresses; configured part is an owner fact, not inferred polarity. B Table 7-5 p15; L/A Table 5 p15; V Table 6-5 p15 | Six 320 variants, V separately; B 60/61, L/A/V 47/67 | `validateDriverConfig`, `bind`, `startInitialize` | P::addressesAndBindings; Vd::bindingAddressesCallbacksAndTimeouts; O::initializationAndBudgets, identityAndUnsafeControls; N::publicOperationsUseBothLegalAddresses propagates both addresses through initialize/refresh/ACK/raw for all seven | PROTOCOL, VALIDATORS, OPERATIONS |
| All eight ascending identity bytes; reversed/partial/wrong signature rejected. B Table 7-7 p16; L/A Table7 p16; V Table6-7 p17 | Shared 320 signature; distinct 322 signature | `startInitialize`, `diagnostics` | P::identityAndRevision, transportedIdentityAndRevision; O::identityAndUnsafeControls, identityPublicationNeedsFreshTime | PROTOCOL, OPERATIONS |
| B has no documented A0; L revision 01, A/V 02; unknown policy retains raw revision without accepting unsupported identity | Explicit revision capability | `startInitialize`, `readRegisterSync`, `diagnostics` | P::accessAndRanges, identityAndRevision; O::identityAndUnsafeControls; D::rawReadMapAndSlot | PROTOCOL, OPERATIONS, DIAGNOSTICS |
| Explicit pointer, legal contiguous ranges, no hole/wrap; exact address/kind/bytes/length/timeout and no extra callbacks. B §7.5; L/A §7.4; V §6.5 | All seven | Managed `poll`, `readRegisterSync` | P::accessAndRanges, framingTimeoutsAndScratch, scriptedFixtureRejectsMismatches; O::initializationAndBudgets | PROTOCOL, OPERATIONS |
| Exact receive success is the callback contract; failed/partial receive never publishes scratch; preserve full backend detail and unchanged caller output | All seven; Status-only callback cannot independently measure received length | `readRegisterSync`, `poll`, `takeResult` | P::framingTimeoutsAndScratch, transportedIdentityAndRevision, normalizedStatusesAndAcceptance, scriptedFixtureRejectsSuccessfulShortReceive (rejects a contradictory fixture before copying); D::callbackNormalizationAndRawIsolation; S::everyReadFailureAndInterruption | PROTOCOL, DIAGNOSTICS, PORT |
| Reject invalid enums/parameters/timeouts without callbacks; static-lifetime Status messages | All seven, capability-specific controls | `validateBinding`, `validateProfile`, `decodeRegister`, starts/raw methods | Vd::malformedProfileEnums, retainedStatusText; C::decodeBytesAndErrors, encodeDomains, composeRejections; D::rawStatusPrecedence | VALIDATORS, PROTOCOL, DIAGNOSTICS |
| Advertisement C0 and detection 30 are independent; preserve reserved codes and raw bytes. B/L/A 08, V Table 6-8 | All seven; V advertisement write gated | `decodeRegister`, `startReadProfile`, `startRefreshPort` | C::decodeBytesAndErrors, encodeDomains, compareSettings; S::independentStatusEncodings, everyFinalStatusByte; D::preservedRawRepresentations | PROTOCOL, PORT, DIAGNOSTICS |
| Every role/accessory/current/duty/preference/debounce code; Try.SRC code 3 and reserved code 2; base versus extended debug/debounce | B Table 7-7; L/A Tables 8-10; V Tables 6-8 to 6-10 | `decodeRegister`, `validateProfile`, apply/read/refresh | C::decodeBytesAndErrors, encodeDomains; Vd::profileCapabilityCombinations; S::independentStatusEncodings, everyFinalStatusByte | PROTOCOL, VALIDATORS, PORT |
| Local/peer role and applicability require paired observation; detached is valid NOT_APPLICABLE without usable current/orientation | All seven; conservative accessory/debug applicability | `startRefreshPort`, `portSnapshot` | S::independentStatusEncodings, changedBookends, ccAndVbusEvidence | PORT |
| ACK actual 09=B5→15 / base B4→14; preserve applied controls rather than staged profile. Settings do not clear pending IRQ | B/L/A; V 09 gate | `startAcknowledgeInterrupt`, `startServiceInterrupt`, `writeRegisterSync` | C::composeIrqResetAndOrdinary; P::composedWriteFrames; I::everyAckByteAndStaged02, unrelatedProfileWritesPreserveLatch; D::rawAckUsesOnlyActualControls | PROTOCOL, PORT, DIAGNOSTICS |
| Isolate reset bit 08; never replay a read reset bit into settings. DE→56 is field arithmetic only: actual DE rejects settings, while settled D6→56 is safe | B/L/A/V 0A masks | `decodeRegister`, `startReset`, `startApplyProfile`, `writeRegisterSync` | C::composeIrqResetAndOrdinary; R::literalSuccessAndDesiredRetention, freshResetControlsAndIdentityContradictions; D::rawWriteGuards | PROTOCOL, OPERATIONS, DIAGNOSTICS |
| Protect 45 internal bits 03, require qualified safe image; no raw bypass of role/reset/recovery sequences or guessed settings image | B/L/A 45; V 45 unavailable | apply/reset/recovery and raw APIs | C::composeModesAndRecovery, composeRejections; A::internal45QualificationAndStagedControls; D::rawWriteGuards, protected45EvidenceIsIndependent, rawReadSafetyEvidence | PROTOCOL, OPERATIONS, DIAGNOSTICS |
| Equivalent 322I DRP00/11 is semantic equality, not permission to write a contradicted literal mode image; full proof renews permission | V Table 6-10 p20 plus software safe-image policy | `readRegisterSync`, `startReadProfile`, `writeRegisterSync`, `startApplyProfile`, `startReconcile` | D::equivalentModeObservationsRevokeStaleWriteImages covers both directions and raw/partial managed observations; raw and typed writes reject without traffic until full proof | DIAGNOSTICS |
| Base unattached precheck only for actual mode change; cable can race read→write. Attached current-only update remains legal; no-op verifies without writes; pre-write cancellation preserves actual proof | B §7.4.1-2 p14, Table 7-7 p19 | `startApplyProfile` | A::baseAttachmentAndWriteOrder, noOpAndVerification, prewriteAndEveryBoundary, interruptedBaseModeEvidence | OPERATIONS |
| Separate disable/mode/wait/enable, retain requested disabled final state; 0A disableTerm and 45 disableRdRp independently effective | L/A §7.3.2 p14; B/L/A 45; 5 ms plus a separate one-tick software margin | `startApplyProfile`, `configurationSnapshot` | A::literalModePlans, ufpAndSettingsOnlyPlans, physicalModeWait, exactWaitGatesAndLateFinalTime; O::independentTerminationEvidence | OPERATIONS |
| Explicit bounded reset, no desired-profile replay; reset mask/value counterexample and 95 ms maximum plus a one-tick policy margin | All seven; per-variant proof plan | `startReset`, `startReconcile` | R::literalSuccessAndDesiredRetention, everyCallbackFault, everyInterruptionBoundary, independentPhysicalWaits; F::reconcileResetWait | OPERATIONS |
| Explicit qualified base dead-battery read 45/write 04/wait 31 ticks/write 00/read 45, maximum four callbacks, no transient snapshot | B §7.4.3 p14 only | `startRecoverDeadBattery` | R::admissionAndPrechecks, literalSuccessAndDesiredRetention, everyCallbackFault, everyInterruptionBoundary, recoveryHistoryAndExplicitRestart | OPERATIONS |
| V has separate ID, DRP00 semantics, no 45; raw IRQ bit 10 and fault bit 08 are separate facts; DFP/UFP recipes add no invented wait | V §6.3.2.1-3, Tables 6-7 to 6-11 | initialize/decode/refresh/apply | P::identityAndRevision, accessAndRanges; C::decodeBytesAndErrors; A::literalModePlans, ufpAndSettingsOnlyPlans, gated322Changes; S::everyFinalStatusByte | PROTOCOL, OPERATIONS, PORT |
| Gate V advertisement writes, every 09 write/fault-clear and unsupported direct return-to-DRP; supported independent changes still work | Unresolved V access/procedure evidence, not inferred family support | `startApplyProfile`, IRQ starts, `writeRegisterSync` | A::gated322Changes; I::admission; C::composeRejections; D::rawWriteGuards | OPERATIONS, PORT, PROTOCOL, DIAGNOSTICS |

## Required observable cross-operation scenarios

These existing equivalents retain readable names; no duplicate solely for a
suggested prompt name is required.

| Prompt 18 scenario | APIs and variants | Named runtime tests and observable boundary | Command |
| --- | --- | --- | --- |
| `failed_write_may_have_applied` | apply B/L/A/V; reset all seven; recovery B; ACK B/L/A; supported raw writes | A::faultsAtEveryWriteAndProofRead and tusb322ModeWriteFailures cover each mutating phase with both applied/no-applied error; R::everyCallbackFault and I::everyTransportFailure do the same for their writes. D::rawOrdinaryWrites adds both raw effect outcomes and an unrelated successful ID read without trust repair; rawAckUsesOnlyActualControls models failed ACK both clearing and retaining the latch. Original error/detail, earlier effects and terminal callback count survive. A::mismatchAndExplicitRecovery and R::recoveryHistoryAndExplicitRestart reject blind replay/trust repair. | OPERATIONS, PORT, DIAGNOSTICS |
| `cancel_after_each_phase_is_bus_silent` | all owner starts; per-variant supported plans | O::interruptionAtEveryPhase; F::everyReadFailureAndInterruption; A::prewriteAndEveryBoundary, tusb322ModeInterruptionBoundaries; R::everyInterruptionBoundary; I::interruptedBoundaries. Include first transfer, each later callback, waits, final callback before fresh publication, end/result retention; O::reinitializationAndAtomicBinding and R::recoveryHistoryAndExplicitRestart cover rebind. | OPERATIONS, PORT |
| `failed_status_read_preserves_observation` | refresh all seven; service B/L/A | S::everyReadFailureAndInterruption retains fields/time across failed partial observation and invalidation. S::changedBookends clears dependent validity while retaining raw 09/08/09. I::everyTransportFailure, serviceUsesFinalControlsAndBookends, recheckContradictionRetainsPublication retain complete pre-ACK history/time through later failure. | PORT |
| `irq_reassertion_remains_visible` | ACK/service B/L/A; raw read all seven | I::finiteLatchRaces covers each read/ACK/recheck race, stuck/coalesced latch and finite five-callback service; exactWritesAndBounds and everyAckByteAndStaged02 inspect literal writes. S::admissionAndPublication performs read-only refresh; status read is no implicit ACK and no lossless edge count is claimed. | PORT |
| `reconcile_partial_profile_allows_explicit_apply` | read/reconcile/verify/apply, supported variants | A::mismatchAndExplicitRecovery establishes KNOWN actual despite desired mismatch, preserves desired and old failed result, then explicitly applies; interrupted transitions remain UNKNOWN until complete proof. F::incompletePostWaitProof and partialTerminationContradictionWithoutTrust reject incomplete proof. | OPERATIONS |
| `dead_battery_recovery_retains_interrupted_sequence` | recovery B320/I only | R::literalSuccessAndDesiredRetention, everyCallbackFault, everyInterruptionBoundary, recoveryHistoryAndExplicitRestart cover read/write 04/wait/write 00/read, every fault/cancel/deadline boundary, four-callback ceiling, no transient port publication and retained history after take/end/rebind. R::admissionAndPrechecks rejects internal bits 03 and other-variant recovery; D::unresolvedRecoverySurvivesConsumption retains diagnostics without forbidding explicit recovery restart. | OPERATIONS, DIAGNOSTICS |

## Interpretation limits

An OK callback promises the entire requested receive; Status contains no actual
byte count. A backend that silently fills fewer bytes and returns OK violates
that contract. A rejecting fake catches contradictory fixtures; host core tests
cannot prove a physical adapter's length or aggregate timeout behavior.

Shared identity cannot prove exact variant, board straps or enable polarity.
Tested V gates remain gates: host expectations cannot resolve TI's advertisement
access contradiction, VCONN-fault clear transaction or direct non-reset DRP
procedure. Callback counts, historical operation effects, raw observation,
configuration trust and port applicability remain separate evidence.

The [current peer audit](audit/execution-18-peers.md) records fresh source/state
inspection and adopted/rejected conventions. Final execution results and tool/
hardware limits are maintained in [validation](validation.md).
