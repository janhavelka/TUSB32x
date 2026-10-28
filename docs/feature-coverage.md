# Feature, source and verification coverage

Reconciled against the implemented headers, engine, CLI and registered tests on
2026-10-29 (prompt 35). The seven configured I2C variants have the capabilities
below. **Implemented** describes software; **gated** names unavailable dependent
operations; **outside API** identifies board responsibility or excluded devices.
Physical qualification is **NOT RUN**. Configured variant, matching identity,
passing host tests and linked firmware are separate evidence.

The [variant matrix](variant-matrix.md) owns exact electrical limits and named
conflicts. Source IDs mean B = SLLSEN9F March 2022 (TUSB320/I), L = SLLSEP2D
May 2017 (LI/HI), A = SLLSEQ8D May 2017 (LAI/HAI), V = SLLSEO5D March 2021
(TUSB322I), N = SLAAE24 May 2021 and G = SDAA284 March 2026. G0/G1 are
SLLSEO6C August 2018 / SLLSER2A May 2017 (GPIO-only TUSB321/TUSB321AI).
All citations below use printed pages of the [repository-only archived sources](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/README.md).

The [register map](register-map.md) and [device procedures](device-behavior.md)
are the authoritative field and sequence contracts. The [CLI guide](CLI.md)
defines exact grammar and aliases: profile setters only stage a candidate;
`apply` explicitly admits a core operation. `read` performs live observation;
`sample` and `freshness` report cached evidence. All 55 command rows are implemented.

Tests below are real source files under `test/`, with named functions also mapped
in [repository-only protocol conformance](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md). P = `test_protocol.cpp`,
C = `test_codecs.cpp`, O = `test_operations.cpp`, F = `test_profile_observation.cpp`,
A = `test_profile_application.cpp`, R = `test_reset_recovery.cpp`,
S = `test_port.cpp`, I = `test_interrupt.cpp`, D = `test_diagnostics.cpp`,
Vd = `test_validators.cpp`. P/C run in `tusb32x_protocol`, O/F/A/R in
`tusb32x_operations`, S/I in `tusb32x_port`, D in `tusb32x_diagnostics` and
Vd in `tusb32x_validators`. CLI tests run in `tusb32x_cli` from
`cli/test_cli.cpp`, `cli/test_cli_operations.cpp` and `cli/test_cli_diagnostics.cpp`;
`cli_contract` and `cli_transcript` execute the built host fixture. The rows map
existing coverage; [repository-only validation](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md) records actual runs and their SHA,
including prompt 34's separate firmware/remote evidence. Fixture-plan tests use
modeled hardware and do not close physical qualification or silicon gates.

| Feature and state | Variants | Evidence class; exact source | Implemented API / CLI | Required test or qualification evidence; owning prompts |
| --- | --- | --- | --- | --- |
| Register access/defaults and safe byte composition — Pure helpers and safe owner raw access implemented | B/L/A/V with exact write gates | TI fact: B Tables 7-6/7-7 pp16–19, L/A Tables 6–12 pp16–20, V Tables 6-6–6-11 pp17–20; masks, partial base defaults and policy in register guide | CommandTable, codecs, raw diagnostics / reg, wreg, decode | Literal D01–D36/C01–C21/N01–N25; eight access-mask partitions; base unknown defaults never asserted zero; separate profile comparison masks; PASS private access/transport9 and pure codecs/composition10; owner conformance18 PASS; CLI rawReadAliasesAndFailures, pureDecodeAndVariantGates and rawWriteEffectsAndProcedureDenial |
| Explicit variant/address validation and binding — Implemented | Six 320 parts + V | TI fact: B pins p4/§7.5 p15; L/A pins p3/§7.4 p15; V pins p3/§6.5 p15. Policy: no auto-detection | validateDriverConfig, bind / variant, addr, bind, capabilities | P::addressesAndBindings; Vd::bindingAddressesCallbacksAndTimeouts; CLI stagingAndAtomicTargets; no callbacks on rejected pairs |
| Eight-byte identity — Implemented through initialization | B/L/A: 30 32 33 42 53 55 54 00; V: 32 32 33 42 53 55 54 00 | TI fact: B Table 7-7 p16; L/A Table 7 p16; V Table 6-7 p17 | startInitialize, diagnostics / init, probe, capabilities | P::identityAndRevision, transportedIdentityAndRevision; O::identityAndUnsafeControls; CLI admissionAndAliases and successfulReadOnlyRoutesAndReset |
| Revision acceptance — Implemented through initialization; unknown revisions gated | L 01, A/V 02; B no A0 | TI fact: L/A Table 12 p20, V Table 6-11 p20; policy in variant guide; code observation REVISION_06 | startInitialize; readRegisterSync / init, reg, diag | P::identityAndRevision; O::identityAndUnsafeControls; D::rawReadMapAndSlot; CLI failuresAndVariantGates; revision 06 remains rejected |
| Read-only initialization/profile observation/reconciliation/verify — Implemented | All seven I2C variants with per-feature gates | TI fact: documented identity/control maps B pp16–19, L/A pp16–20, V pp17–20. Policy: no automatic writes | startInitialize/startReadProfile/startReconcile/startVerifyProfile / init, profile read, verify, reconcile | O::initializationAndBudgets; F::everyReadFailureAndInterruption; A::noOpAndVerification; CLI successfulReadOnlyRoutesAndReset; no writes or desired adoption |
| Port attachment, role and orientation — Implemented; physical mapping unqualified | B/L/A/V | TI fact: B §7.3/Table 7-7 pp10–13,17–18; L/A §7.2/Tables 8–9 pp10–12,17–18; V §6.3/Tables 6-8–9 pp9–13,17–19 | startRefreshPort, portSnapshot / read, sample, freshness | S::independentStatusEncodings, changedBookends, ccAndVbusEvidence; CLI successfulReadOnlyRoutesAndReset and portFormatterAndStatusMessageBounds; physical mapping NOT RUN |
| Current detection category — Implemented decoding/publication; physical freshness gated | B/L/A/V | TI fact/conflict ATTACHED_CURRENT: B §7.3.1.2 p11; L/A §7.2.1.2 p10; V §§6.3.2.2–3 pp10–11; G Figure 2-1 p8 | startRefreshPort / read, sample | S::independentStatusEncodings; modeled role-current fixture tests exercise explicit base reset; exact-part attached-current tracking remains unqualified |
| Current advertisement — Implemented B/L/A; gated change on V | B/L/A/V | TI fact: B §7.3.2 p12/Table 7-7 p17, L/A §7.2.2 p11/Table 8 p17; conflict V_ADVERTISEMENT, V Figure 6-3/Table 6-8 p17 | DeviceProfile.sourceCurrent, startApplyProfile / current, apply | C::encodeDomains; A::baseAttachmentAndWriteOrder, noOpAndVerification, gated322Changes; CLI stagingAndAtomicTargets, irqAndApplyEffects; local-source supply qualification remains external |
| Base mode selection — Implemented | TUSB320, TUSB320I | TI fact: B §§7.4.1–2 p14/Table 7-7 p19 | DeviceProfile.portMode, startApplyProfile / mode, apply | A::baseAttachmentAndWriteOrder, interruptedBaseModeEvidence; CLI irqAndApplyEffects; base cable-race prevention remains an owner prerequisite |
| Extended mode selection — Implemented; LI/HI application caution | LI/HI/LAI/HAI | TI fact: L/A §7.3.2 p14, Tables 9–11 pp18–20; current LI/HI product-page recommendation in variant guide | startApplyProfile / mode, apply, capabilities | A::literalModePlans, physicalModeWait, exactWaitGatesAndLateFinalTime; CLI irqAndApplyEffects; no LI/HI physical DFP/DRP claim |
| 322 DFP/UFP selection — Implemented with write gates | TUSB322I | TI fact: V §§6.3.2.1–2 pp10–11, Table 6-10 p20 | startApplyProfile / mode, apply | A::literalModePlans, ufpAndSettingsOnlyPlans, tusb322ModeWriteFailures; CLI failuresAndVariantGates; no inherited 5 ms wait |
| 322 direct return to DRP — Gated | TUSB322I | Unresolved V_RETURN_DRP: V §6.3.2 p9, §6.3.2.3 p11, Table 6-10 p20 | startApplyProfile / mode drp, apply | A::gated322Changes; D::rawWriteGuards; gate rejects actual changes without a reset fallback; existing equivalent DRP remains readable |
| Try.SNK / Try.SRC, UFP accessories, termination disable — Implemented; V UFP-accessory writes gated | L/A/V; absent on B | TI fact: L/A §7.2/Tables 8–10 pp10–11,17–19; V §6.3/Tables 6-8–10 pp9–12,17–20; V_FAULT_CLEAR affects 09 bit0 writes | DeviceProfile fields / preference, ufpaccessory, termination, apply | C::decodeBytesAndErrors, encodeDomains; Vd::profileCapabilityCombinations; A::ufpAndSettingsOnlyPlans, gated322Changes; CLI stagingAndAtomicTargets |
| Duty/debounce and accessory decoding — Codes/apply/port implemented; V duty writes gated | B/L/A/V, exact per-variant encodings | TI fact: B Table 7-7 pp17–19; L/A Tables 8–10 pp17–19; V Tables 6-8–10 pp17–20; V_FAULT_CLEAR affects 09 bits2:1 writes | validateProfile, codecs, startApplyProfile, portSnapshot / duty, debounce, decode | C::decodeBytesAndErrors, encodeDomains; S::independentStatusEncodings; A::gated322Changes; CLI pureDecodeAndVariantGates and stagingAndAtomicTargets |
| Separate 45 Rd/Rp control — Implemented B/L/A; unavailable V | Six 320 parts only | TI fact: B Table 7-7 p19; L/A Table 11 p20; V complete map pp17–20 lacks45 | DeviceProfile.disableRdRp / rd-rp, apply | A::internal45QualificationAndStagedControls; O::independentTerminationEvidence; D::rawWriteGuards; CLI failuresAndVariantGates; no 45 access on V |
| Explicit reset — Implemented, effect differences retained | B/L/A/V | TI fact: B Table 7-7 p19/§6.7 p7; L/A Table 10 p19/§6.7 p6; V Table 6-10 p20/§5.7 p7/§6.3.2 p9 | startReset / reset | R::literalSuccessAndDesiredRetention, everyCallbackFault, independentPhysicalWaits; CLI successfulReadOnlyRoutesAndReset; modeled reset-termination plans; physical reset timing NOT RUN |
| Post-dead-battery Rp recovery — Implemented B only | TUSB320, TUSB320I | TI fact: B §7.4.3 p14; conservative identity/readback prerequisites are library policy | startRecoverDeadBattery / recover-dead-battery | R::admissionAndPrechecks, literalSuccessAndDesiredRetention, recoveryHistoryAndExplicitRestart; CLI failuresAndVariantGates; modeled base reset-termination plans; no inferred readiness from clear 45 |
| IRQ observation — Implemented in refresh; ACK / bounded service — Implemented B/L/A, gated V | B/L/A/V | TI fact: B Table 7-7 p18; L/A Table 9 p18; V Table 6-9 pp18–19; N §8 p9. Unresolved V_FAULT_CLEAR | startRefreshPort, startAcknowledgeInterrupt, startServiceInterrupt / irq read, irq ack, irq service | I::everyAckByteAndStaged02, serviceUsesFinalControlsAndBookends, finiteLatchRaces, everyTransportFailure; S::admissionAndPublication; CLI irqAndApplyEffects and cachedInterruptHistory |
| VCONN — Autonomous silicon effect, no software enable API; fault clear gated | TUSB322I only | TI fact: V §5.3 p4/§5.5 p6/§6.3.8 p13; unresolved Table 6-9 p19 | Raw port/fault evidence / read, decode; no VCONN setter | Ra/source/detach physical fixture with board-owned 5 V VDD prerequisite and 4.75–5.5 V VCONN qualification; limit 225/300/375 mA min/typ/max, R_ON1.25 Ω max documented; active cable not powered-VCONN proof; six-part APIs reject invented control; 3/15/30 |
| Safe raw register diagnostics — Implemented; V 08/09 writes gated | All seven, exact allowlists | TI fact: complete maps above; policy: explicit pointers, no bypass of identity/access/procedure gates including V_FAULT_CLEAR | readRegisterSync, writeRegisterSync / reg, wreg, decode | D::rawReadMapAndSlot, rawWriteGuards, rawOrdinaryWrites, rawAckUsesOnlyActualControls; CLI rawReadAliasesAndFailures and rawWriteEffectsAndProcedureDenial |
| GPIO-only parts — Outside API | TUSB321 (321I in G summary), TUSB321AI (321A summary) | TI fact: G0/G1 §5 p3 and G Figure 2-1 p8 | No Variant members or probe addresses | Vd::bindingAddressesCallbacksAndTimeouts; P::accessAndRanges; no GPIO-only Variant member or discovery probe |
| Power, straps, pin levels, bus timing — Outside API | Each exact variant | TI fact/conflicts: detailed tables and STARTUP_EN/SUPPLY_SUMMARY/L_CC_RATING/ID_OFF_RATING/BASE_VALID_TIME in variant guide | Application adapter/BoardConfig; bus/capabilities reports assumptions only | Actual recording-backend Arduino/native-IDF tests and linked SDK fixtures; board supply/startup/aggregate timing traces remain NOT RUN; no core GPIO/rail control |
| USB policy, PD and board interchangeability — Outside API | Entire family | TI fact: N §3 p4; B §7.3.1.1 p11, L/A §7.2.1.1 p10 and V §6.3.2.1 p10 DFP exception; exact pin tables | No USB stack, charger/VBUS switch or mux control in core | tusb32x_portable_consumer and tusb32x_host_basic; no PD, e-marker, ADC, power switch or connector-mux API; physical interoperability remains outside host evidence |

## Procedure-to-API, CLI and test coverage

Source IDs and exact revisions above apply to these citations. P/T vector IDs
refer to the device guide, with arithmetic verification recorded in validation.

| Specified plan / variants | TI source and selected policy | Implemented API / CLI | Coverage and remaining qualification |
| --- | --- | --- | --- |
| Explicit pointer/STOP transport; B/L/A/V | B §7.5 pp15–16; L/A §7.4 p15; V §6.5 pp15–16; one aggregate terminal callback | Callback adapters / bus, reg | P27–P28; lock across both phases, no retries/pending DMA; scope traces and aggregate SDK bounds in 9/23/25 |
| I/P/F read-only proof; B/L/A/V; initialize/read/verify/reconcile implemented | Exact identity/map tables above; scalar controls, fixed one-pass proof and incomplete-effect restrictions are policy | startInitialize, startReadProfile, startVerifyProfile, startReconcile / init, probe, profile read, verify, reconcile | P/F exact reads, fresh completion, termination, reserved/partial/cancel/deadline and reset-pending proof12; no second F or writes; desired mismatch and mode-write history13; recovery history14/18 |
| Ordinary settings/45; B/L/A, only permitted 0A changes on V | B Table 7-7 pp17–19; L/A Tables 8–11 pp17–20; V Table 6-10 pp19–20 | startApplyProfile / current, duty, debounce, preference, termination, rd-rp, apply | P04/P08/P11–P12/P29; known internal 03=00 before W45 (conservative B policy), preserve independent gates and actual controls;10/13/18 |
| Base mode; B | B §§7.4.1–2 p14/Table 7-7 p19; owner excludes cable race | startApplyProfile / mode, apply | P09–P11; precheck before any write, no blanket attached-current ban;13/18/30 |
| Extended role; L/A | L/A §7.3.2 p14; held-disabled final state policy from Table 10 p19 | startApplyProfile / mode, termination, apply | P01–P04; T01–T02/T05–T12; failure at every separate write, six-tick wait;13/18–19/31 |
| DFP/UFP role; V | V §§6.3.2.1–2 pp10–11, Table 6-10 p20; no added 5 ms | startApplyProfile / mode, apply | P05–P08; no direct return-to-DRP or hidden reset; unchanged gated 08/09 permit independent 0A work;13/18 |
| One-shot reset; B/L/A/V | B Table 7-7 p19/§6.7 p7; L/A Table 10 p19/§6.7 p6; V Table 6-10 p20/§5.7 p7/§6.3.2 p9 | startReset / reset | P17–P20; T04/T07–T12; 96 ticks, actual controls not defaults, V semantic-DRP consistency policy, no desired replay;14/18–19/31 |
| Base recovery; B only | B §7.4.3 p14/Table 7-7 p19; identity, internal-zero/pre-read, final proof and other-transition gate are library policy | startRecoverDeadBattery / recover-dead-battery | P13–P16/P30; T03/T06–T12; 4 callbacks/6 TX/2 RX,31 ticks, no transient publication/hidden cleanup;14/18–19/31 |
| Fixed port observation; B/L/A/V; implemented | Status tables above;09/08/09 and E0 coherence comparison are policy | startRefreshPort / read, sample, irq read | P24/P26/P28; no retry-until-stable, no fresh physical-current inference;15/18/30 |
| ACK/service; B/L/A only; implemented | B Table 7-6 p16/Table 7-7 p18; L/A Table 9 p18; N §8 p9; bounded recheck policy | startAcknowledgeInterrupt, startServiceInterrupt / irq ack, irq service | P21–P26; ACK 2 callbacks/3 TX/1 RX, service 5 callbacks/6 TX/4 RX; complete pre-ACK observation survives later error; coalescing/reassertion, V rejects before traffic;16/18/30 |

Initialization, profile observation, apply, desired verification, read-only
reconciliation, reset, base recovery, port refresh and IRQ acknowledgement/service
have registered host behavioral tests. All physical hardware qualification
remains NOT RUN.
V_ADVERTISEMENT, V_FAULT_CLEAR, V_RETURN_DRP and the board/electrical/current
gates remain open. Prompt 4 did not qualify adapters or close those gates.

The following sections retain delivery-stage provenance. Their earlier suite
counts and checks belong to the stated prompt; the current registered matrix and
framework link results are in validation. No dated host result substitutes for
physical qualification or a later commit's remote result.

## Prompts 5-10 implemented values, validators, codecs and API mapping

These entries retain passive-value and build evidence from the initial stages.
Current method contracts are in [API contracts](api-contracts.md).

| Feature / state | API or value | Source / provenance | Actual check; remaining owner |
| --- | --- | --- | --- |
| Status — Implemented | Err, Status, errorName | Engineering baseline; LDC/LSM representation and TMP constexpr/explicit-bool mechanism in current peer audit | PASS 23 names/order, int32 extremes, invalid code names, constant evaluation, explicit conversion and multiple TUs; private callback normalization and all invalid Err values9 |
| Transport/settings — Values, validators and private transport implemented | I2cWriteFn, I2cWriteReadFn, DriverConfig, DeviceProfile, validators; private protocol helpers | Callback baseline plus exact variant/register tables B pp15-19, L/A pp15-20, V pp15-20 | PASS signatures/defaults/standalone headers, exhaustive validation, exact scripted frames/access/ranges, output preservation, strict identity/revision, failed-write acceptance and all/managed saturating accounting9; owner execution11-17 |
| Configuration/port — Values and complete observation implemented | ConfigurationSnapshot, ProfileField, PortSnapshot, PortField | Baseline and register/device guides; explicit raw/typed validity | PASS defaults, mask values and independent raw copy; publication/lifetime6, observations12/15 |
| Operation/diagnostics — Values and views implemented | OperationToken/Timing/Result/Progress, identity/IRQ values, DriverDiagnostics | Owner baseline and prompt4 retained effect rules | PASS fixed values/default invalidity and token domain; admission/time6/11, retained IRQ16, diagnostics17 |
| Health — Pure public projection and managed evidence implemented | DriverState, ManagedTransportEvidence, HealthPolicy/Snapshot | Baseline policy; no TI health claim | PASS defaults and private managed/raw totals/streak/timestamp separation9; public validation/projection/views17 |
| Decoder — Implemented | RegisterDecode; decodeRegister(Variant, uint8_t, uint8_t, RegisterDecode&) | Register-map explicit field tables; variant-specific reserved/access restrictions | PASS exhaustive mapped bytes/variants, reserved validity, invalid inputs and complete output preservation10; public linkage without private includes; CLI formatting22 |
| Owner facade — Lifecycle, operations, raw and passive diagnostics implemented | Lifecycle, startInitialize/startReadProfile/startApplyProfile/startVerifyProfile/startReconcile/startReset/startRecoverDeadBattery/startRefreshPort/startAcknowledgeInterrupt/startServiceInterrupt, tokened poll/cancel/snapshot/take, cached getters and totals clear | Baseline and current execution-11 through17 peer audits | Registered tusb32x_operations/tusb32x_port/tusb32x_diagnostics and delivered public-link checks; conformance18-19 |
| Host foundation — Implemented | TUSB32x target, tusb32x_compile_contracts and tusb32x_validators suites; examples/host/basic | Prompt7 and current peer build audit | Debug/Release native linkage, independent headers and standalone tests-OFF multi-file consumer; source-tree consumption only, packaging8/32, firmware23-27 |
| Source docs — Present | Five public headers with file/type/member/enum/function and generated version Doxygen | Maintainability baseline and current peer documentation conventions | PASS strict Doxygen, visibility of17 public structs and four version constants; complete omission guard33 |
| Metadata/version — Implemented | library.json truth, Version.h, generator sync/check/bump/set, aligned CMake/component/Doxygen fields | Current metadata peer audit and official manifest/build documentation | Isolated version fixtures, duplicate-declaration rejection, no-Git/enclosing-Git/dirty-example/no-op behavior; tests-OFF consumers need no maintainer tools |
| Component/native tool configuration — Implemented; firmware linked | Explicit-source IDF registration, managed pio.cmd, pinned native PlatformIO consumer | IDF 5.3.2 and platformio/native1.2.1 upstream sources | Host registration and launcher fixtures; native consumer and independent IDF links in prompts 25-26, extracted package links in 32/34 |

Host checks exercise read-only procedures, profile writes and reset/recovery through terminal fake
callbacks. No pass above represents a physical I2C transaction, framework
firmware build, remote CI run or physical qualification.

## Prompt 6 completed owner contracts

The table below retains prompt 6's specification provenance. Prompt 11 now
implements its lifecycle, token/result and initialization time subset; prompt 12
adds profile observation and read-only reconciliation; prompt 13 adds explicit
apply and desired verification; prompt 14 adds reset/base recovery in the registered
owner suite. [Method/state rules](api-contracts.md#method-admission-and-output-preservation)
and vectors O01–O25 remain normative for broader conformance checks.
Peer provenance is [execution-6-peers](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-6-peers.md); register and
procedure sources remain the exact TI revisions already listed above.

| Feature / variant | API / CLI mapping | Completed specification and actual evidence | Delivered behavioral coverage |
| --- | --- | --- | --- |
| Admission/results, all bound I2C variants | bind/start/poll/cancel/end/take; CLI operation correlation | Exact precedence, output preservation, one token/result slot, lifetime and different-target gates; signature and owner-example syntax PASS | 11, conformance18/19, CLI20–22 |
| Time/callback budgets, all | OperationTiming/Progress; owner scheduling, CLI limits | Immutable half-range deadlines, one callback, fresh wait anchors, immediate transport failure, six local stages; 60 constexpr vectors and earlier fractional-time arithmetic PASS | 9/11/14/19, adapters23/25/26 |
| Configuration proof, exact applicable fields | initialize/read/apply/verify/reconcile; CLI profile commands | I/P/F publication, generations, identity readiness versus KNOWN, mismatch semantics and no replay; passive configurationWriteUnresolved default/copy PASS | 11–14/17–19 |
| Recovery/reset/termination, variant-gated | reset/recover; explicit CLI mutations | Separate obligations, current disable controls, acceptance and completion; p4 sequence ceilings unchanged; O08/O09/O18/O19 | 13/14/18/19 |
| Port/IRQ, B/L/A plus gated V | refresh/ACK/service; CLI observation and service | Delivered S timestamps/bookends/validity, publication before ACK, retained complete/partial evidence and finite work; O20/O21; no queue or automatic reservice | 15/16, broader conformance18 and CLI21/22 |
| Raw diagnostics/health, capability-gated | read/write Sync, stats/diagnostics/projectHealth; CLI reg/wreg/probe/health | Raw/all/managed accounting, unknown callback times, passive thresholds, no OFFLINE gate; source Doxygen and public behavioral diagnostics17; O22/O23/O25 | broader conformance18/19, CLI21/22 |
| Generic external owner, all | Two driver/token pairs on borrowed shared bus | Initial compile contracts plus delivered public runtime owner cases; no FieldCore include or core scheduler | Runtime owner27 below; framework adapters23–26 |

No runtime operation, health helper or validator is linked by prompt 6. The
configurationWriteUnresolved addition was passive at that stage; prompt 13 now
exercises its managed apply/effect update rules. Firmware link evidence is recorded
in prompts 23-26/32/34 below; physical qualification remains NOT RUN.

## Prompt 12 delivered observation and proof

`startReadProfile` executes P, with callback/TX/RX ceilings 4/4/4 on B/L/A
and 3/3/3 on V. It publishes complete raw/decoded observation, preserves earlier
complete evidence on failed partial acquisition and never adopts desired, reads
identity or promotes configuration trust. Complete contradictions invalidate old
current trust; reserved encodings remain raw evidence with invalid typed fields.

`startReconcile` executes F once: 5/5/12 on B, 6/6/13 on L/A and 5/5/12 on V.
Matching identity and complete raw controls can establish initialized; KNOWN
requires all applicable persistent fields and termination/reset/effect proof.
An already pending reset receives a conservative 96-tick wait before F. A reset
first discovered during F remains pending and returns CONFIGURATION_UNKNOWN.
Both operations use the existing token/result/deadline/budget engine and have
HardwareEffect::NONE. No observation clears port invalidation or permits gated
TUSB322I writes. The exact host commands and outcomes are recorded in validation.

Prompt 13 adds public desired mismatch and interrupted apply/mode tests.
Prompt 14 adds reset and base-recovery history coverage through their public
operations; no private-state injection or desired setter was needed.
The fresh [peer audit](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-12-peers.md)
records the adopted snapshot/full-proof conventions and rejected profile replay.

## Prompt 13 delivered application and desired verification

`startApplyProfile` validates the complete candidate and composed plan before
retaining desired input at admission. It preserves proved actual/port evidence
until the first possible mutation, changes differing fields only, then runs F.
No-op F writes nothing and has effect NONE. Actual controls supply preservation
bits; pending desired settings cannot enter an unrelated write. Base actual mode
changes require an unattached precheck before any writes, with the cable race
explicitly retained as an application limit. Current-only updates have no such
mode-change restriction.

Extended 320 mode changes use separate disable/mode/enable steps, preserve
requested final disable controls and wait six owner ticks after fresh mode-write
completion for TI's 5 ms minimum plus the separate quantization margin. TUSB322I
uses its own DFP/UFP procedure without adding that wait. Advertisement, all 09
writes and direct return to DRP remain gated; unchanged proved gated controls
permit independently supported updates. Autonomous VCONN remains device behavior
subject to board power prerequisites.

Maximum apply callback/TX/RX ceilings are B 10/14/13, L/A 12/18/13 and
V 8/11/12; final disableTerm omits enable and reduces L/A to 11/16/13 or V to
7/9/12. At most six planned writes and one callback per nonzero-budget poll
bound work; wait/deadline-only turns make no callback. Requested byte ceilings
include pointers and exclude address/ACK framing. Verify/no-op use F only.

`startVerifyProfile` requires retained desired and runs read-only F, including
any required prior effect wait. Complete differing actual controls can become
KNOWN with CONFIGURATION_MISMATCH. An interrupted apply can be followed by
read-only reconciliation and a new explicit apply from proved actual settings.
There is no retry, rollback, cleanup or replay after failure/cancel/end; the
original error/detail and independent termination/reset obligations survive.
The fresh [prompt 13 peer audit](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-13-peers.md)
records current source choices and the official TI recheck. Exact host outcomes
remain in validation; no result here is physical hardware qualification.

## Prompt 14 delivered reset and base recovery

`startReset` uses a fresh actual 0A image to compose one command with bit 08,
waits 96 elapsed owner ticks from fresh post-callback time and runs F once.
Its callback/TX/RX ceilings are B 7/8/13, L/A 8/9/14 and V 7/8/13. Clear reset
and variant postconditions are required; TUSB322I must return to semantic DRP.
Reset preserves desired input, never retries an uncertain command and does not
claim POR or attachment readiness. Six 320 parts can resample PORT, and 322I's
autonomous VCONN effects remain subject to board-owned supply/power policy.

`startRecoverDeadBattery` is restricted to original TUSB320/TUSB320I with current
identity and caller-qualified stable VDD/readiness. Its literal R45/W4504/wait31/
W4500/R45 sequence is bounded to four callbacks, six TX bytes and two RX bytes.
Nonzero initial internal bits 03 reject before writes; final bits 07 must clear.
The 31-tick wait includes the separate one-tick margin beyond TI's 30 ms minimum.
No transient recovery port status is published. Success proves this sequence;
full configuration trust still requires F.

Both operations use the existing result/token/deadline and transport paths.
Original errors/details, partial or indeterminate effects and independent reset/
recovery obligations survive cancellation, expiry, end/rebind and result take.
Read-only reconciliation observes current controls; a clear 45 cannot prove a
missing historical recovery sequence. A later explicit recovery attempt is
allowed for its own uncertainty, after any different mode/reset transition is
reconciled. Other variants reject recovery without traffic. Existing 322 write
restrictions remain in force. The current [peer and TI recheck](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-14-peers.md)
records exact source choices; actual checks belong in validation. No hardware
or linked framework firmware qualification is implied.

## Prompt 15 delivered complete port refresh

`startRefreshPort` and cached `portSnapshot` use the existing owner engine and
pure codecs with one fixed 09/08/09 plan. All variants have a three-callback,
three-requested-TX-byte/three-RX-byte ceiling, zero writes and no retry. Complete
observations publish final09, raw/typed validity, local/peer roles, applicability,
coherence, original acquisition time and captured configuration provenance.
Changed E0 bookends invalidate current/accessory/active-cable fields while retaining
raw data and independently valid fields. Failure, cancellation or deadline before
publication retains the old snapshot. End/external invalidation/possible writes
mark historical data invalidated; configuration verification cannot freshen it.

The registered `tusb32x_port` suite covers publication and public operation
boundaries; actual commands/counts belong in validation. Sequence counts
publications, saturates and is not an edge count or unique result identity.
Observation of IRQ does not ACK it. Explicit prompt-16 operations own ACK/service;
TUSB322I fault remains raw with its unresolved access context. Exact-board
orientation, attached-current tracking, VBUS/CC interactions and VCONN/electrical
behavior remain physically unqualified. Current categories establish neither
measured load nor USB PD nor board power permission. The current peer review is
[execution-15-peers](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-15-peers.md).

## Prompt 16 delivered explicit IRQ acknowledgement and service

Both methods use the same instance-local timing/token/result contract and existing
poll engine. Current validated identity is required; UNKNOWN configuration is
allowed when fresh actual controls supply a safe ACK image. TUSB322I rejects both
before traffic under V_FAULT_CLEAR. ACK-only reads 09 and writes the safely composed
W1C command once. Service publishes complete 09/08/09 port data before that write
and reads 09 once afterward. Callback ceilings are two and five; requested byte
ceilings are three TX/one RX and six TX/four RX. Larger poll budgets do not add
callbacks beyond one per poll, and no path retries until clear.

The port suite exercises literal actual-control composition, staged/actual
differences, failures before/after simulated latch clearing, reassertion,
coalescing and stuck IRQ without a pin. Complete pre-ACK fields, sequence and time
survive later ACK/recheck failure. IRQ before/attempt/acceptance/after facts remain
independent. Recheck does not replace the port snapshot; ACK preserves its
historical IRQ sample and does not create a settings proof obligation. No queue,
GPIO ownership or automatic follow-up is introduced. Physical IRQ level/timing
and wiring remain unqualified. Actual local commands/counts are in validation;
the current peer review is
[execution-16-peers](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-16-peers.md).

The independent prompt-16 audit adds service cases with different first/final
09 controls and IRQ levels. ACK must preserve final controls; changed attachment/
direction bookends withhold dependent field validity. Success and ACK/recheck
failure retain the same complete pre-ACK observation. Mutation checks demonstrate
that the regressions reject stale-control composition and false matching bookends.

## Prompt 17 delivered raw diagnostics and passive health

`TUSB32xDiagnostics.cpp` implements both scalar Sync methods and the two pure
health functions. Every raw access uses the existing sole accounting boundary;
no second wrapper, clock or operation engine is introduced. KNOWN actual settings
and current raw proof supply safe mixed-register writes. Raw mode/reset/termination/
recovery shortcuts and 322I08/09 writes reject locally. Ordinary writes invalidate
trust before callback entry; accepted writes still require proof. Pure ACK uses
actual09 controls and preserves profile/port evidence. Protected45 contradictions
also revoke typed45 write permission without disabling independent settings.

The registered `tusb32x_diagnostics` suite covers all mapped bytes/variants, exact
admission status precedence, scratch-buffer preservation, every callback Err byte,
possible effects, current versus staged images, raw/managed accounting, bounded
arithmetic and saturation without public test seeds. Health threshold1/default3/
UINT32_MAX and invalid0 cases, OFFLINE explicit work, failed initialization,
clear invariance and unresolved recovery after result take use runtime behavior.
The [diagnostics guide](diagnostics.md) states the independent views and limits.
Prompt 18 completes the core protocol coverage audit. Prompt 19 completes owner
portability checks. Prompt 22 completes CLI diagnostics below; prompts 23-26 deliver the adapters
and linked firmware. Electrical qualification remains NOT RUN.

## Independent prompts 16-18 audit

The 2026-10-27 re-audit adds TUSB322I mode-write fault and interruption coverage
and corrects stale literal 0A image permission after equivalent DRP00/11
observations. Semantic trust remains KNOWN; raw/typed 0A changes require complete
proof again. The updated [conformance matrix](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md)
links the new named cases to the registered suites. IRQ behavior, passive health
and unresolved silicon gates are unchanged.

## Prompt 19 owner portability

The [host ownership matrix](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/owner-portability.md)
maps each time/lifetime/accounting requirement to named registered tests. Existing
sub-ms wait, rollover, health and interruption cases are retained. New evidence
covers two-target fairness during waits/errors/repeated IRQ service, finite queue
and result capacity, explicit108/110/111 callback deadline handling, generation
saturation and a public-only two-TU consumer with allocation traps. This adds
host evidence without changing silicon gates or introducing a core scheduler.

## Prompt 20 local CLI and console

The [local CLI guide](CLI.md) defines the implemented grammar and schemas. All 27
canonical rows and their aliases/help topics execute zero transport callbacks.
Help/version/color/verbose/markers, cache queries, atomic ended target edits,
bind/end/forget-evidence and validated independent profile staging are delivered.
Core desired/observed/applied provenance and raw/field validity remain separate.

The registered `tusb32x_cli` and `tusb32x_console` suites exercise rejection,
poison/fragment/budget/record bounds, backpressure/loss, target/effect fences,
variant-specific staging, source attribution and request/generation limits.
`cli_transcript` uses one persistent `tusb32x_cli_host` process, validates real
records, executes help parity against the Python rows and rejects damaged frames.
The fixture has no separate passing build-only CTest. Queue 8192/record 512/drain 256
and input 192/token 12/payload 32/turn 64 remain explicit. Lost SESSION notices retain
their reason until capacity returns. Prompt 21 adds operation/control/result
routes below; prompt 22 completes the inventory. Adapter and firmware evidence
is recorded separately below.

## Prompt 21 managed CLI coverage

All 44 implemented rows and help metadata run through the same shared sources.
The added routes below use the existing core engine and retained result. Their
handlers admit/query/cancel locally; tick makes the sole owner poll. The
[CLI guide](CLI.md) gives grammar, time bounds and schemas. Named test functions
are in `test/cli/test_cli_operations.cpp`, executed by `tusb32x_cli`.

| CLI grammar / aliases | Core API or evidence | Executed checks |
| --- | --- | --- |
| init / initialize / probe | startInitialize | admissionAndAliases; successfulReadOnlyRoutesAndReset: no reset/ACK/replay, exact identity/profile reads |
| read / status; irq read | startRefreshPort | admissionAndAliases; budgetsAndAutomaticProgress; retained historical/invalidated port evidence |
| profile read; reconcile | startReadProfile; startReconcile | successfulReadOnlyRoutesAndReset: read-only, unchanged desired, pending IRQ retained |
| apply; verify / selfcheck / selftest | startApplyProfile; startVerifyProfile | admissionAndAliases; irqAndApplyEffects; failuresAndVariantGates: desired capture, verification/failure, 322I changed-field gates |
| irq ack / service | startAcknowledgeInterrupt; startServiceInterrupt | irqAndApplyEffects: actual 50% versus staged/desired 30%, exact W1C bytes and bounded recheck |
| reset; recover-dead-battery | startReset; startRecoverDeadBattery | successfulReadOnlyRoutesAndReset; immutableTimeAndLateSuccess; failuresAndVariantGates: literal reset command, base-only 04/wait31/00, admission denial |
| job / status / progress; job auto / step | progress; one tick poll | budgetsAndAutomaticProgress; rejectedOwnerClockSamples: zero/one/255, explicit overrides auto, one callback, no hidden clock errors |
| cancel / job cancel / stop | cancel; retained effects | admissionAndAliases; irqAndApplyEffects: no rollback after possible write, configuration UNKNOWN |
| result / job result; result take | resultSnapshot; takeResult | retainedResultsAndCorrelation; outputLossAndTakeReservation: origin/nonce, request 24 after 25, stale token, repeated query, take once and pre-consumption reservation |

The existing CLI suite also executes real full-width RESULT/RESULT_DETAIL/END
formatting and completion after request exhaustion. `cli_transcript` validates
178 requests in one persistent process plus two fault sessions, all 44 executed
help rows and terminal payload/closure negatives. This is host protocol/logic
evidence, not serial/CC/INT, electrical power policy or firmware qualification.

## Prompt 22 complete command coverage

The shared CLI now has all 55 fixed rows and aliases. Handlers below delegate to
public core methods or the injected input reader, never a separate register
composer/engine. They are exercised in `test/cli/test_cli_diagnostics.cpp` within
the existing `tusb32x_cli` suite. The [complete guide](CLI.md) freezes grammar,
33 record schemas, counter sources, validity and unsupported-feature restrictions.

| CLI routes / aliases | Core API / source | Executed evidence |
| --- | --- | --- |
| reg / rreg / rawread | readRegisterSync; public decodeRegister for returned codes | rawReadAliasesAndFailures; pureDecodeAndVariantGates; everyRawCallbackStatus: one callback, all variants/errors, original detail and no managed accounting |
| decode | decodeRegister | pureDecodeAndVariantGates: no binding/I2C, reserved fields remain invalid, supplied codes do not imply port applicability |
| wreg / rawwrite | writeRegisterSync | rawWriteEffectsAndProcedureDenial; everyRawCallbackStatus: actual W1C/duty preservation, forbidden sequence shortcuts, uncertain acceptance/trust; failedRawWriteUnderBackpressure never retries |
| irq / intpin | cached diagnostics.interrupt / opt-in Platform.readInput | cachedInterruptHistory; pinAndInvalidationEvidence: historical versus sampled level, missing/failing reader remains unknown, no GPIO setup |
| invalidate / forget-evidence | invalidateDeviceState / forgetDeviceEvidence | pinAndInvalidationEvidence; parserAndOwnerExclusion: explicit reason, retained effects/results/historical port; active/bound/result gates and target edits never forget implicitly |
| drv / health / state; diag | projectHealth(default policy); cached diagnostics | healthAndManagedProbe; automaticTrafficIsSeparate: managed-only projection, independent trust/effects, original failure/time validity and no policy/admission control |
| xfer_stats / counters; xfer_assert; xfer_reset | transportStats / pure exact comparison / clearTransportStats | rawReadAliasesAndFailures; healthAndManagedProbe; root CLI limit fixtures: all-callback source, saturated rejection, exact zero/one deltas and idle clear invariance |
| cfg / settings / profile show | configurationSnapshot including CONFIG_DETAIL | configurationMismatchDetails: independently observed mismatch reg8/expected64/observed128/mask192 and acquisition time, result retained |

`tools/check_cli_contract.py` executes all canonical/alias/help routes through the
built fixture and independently checks callback/write effects and all required
records. The transcript reuses its help comparison. Registered `cli_contract`
and `cli_contract_negative` reject deliberately broken alias/schema dependencies.
There is still one `cli_transcript` registration. Maximum-width actual formatter
fixtures and all 33 schema bounds include newline/NUL; console and raw congestion
cases keep whole-record loss honest. None of these host tests qualify serial,
adapter framing, connector/CC/INT behavior or firmware memory/timing.

## Independent CLI audit evidence

Prompts 20-22 retain all 55 canonical routes and 164 executed parity routes. The
grouped-help regression verifies contiguous topic sections rather than only row
presence. Transcript session tests reject records after closure, including local
requests. The owner regression covers both five-versus-eight-record reservation
outcomes before exact result take, keeping original completion and query delivery
separate. No core method, silicon gate or framework support claim changes.

## Arduino adapter (prompt 23)

Example-only write/writeRead callbacks and ownerNowMs are implemented for the
pinned S2/S3 SDK configuration. Actual host contract suites compile the adapter
with recording backends at normal/coarse tick rates; real SDK fixtures link both
targets. Explicit pointer STOP/read STOP, aggregate clipping, error detail,
short transfers, shared gate/retirement, timeout restoration, 64-bit time and
counter saturation have executable evidence. See [integration](arduino-transport.md)
for the source-derived bound and application prerequisites. Core compilation and
CLI dispatch are unchanged. Board scheduling/CPU qualification, electrical traces
remain separate work.

## Arduino applications (prompt 24)

The independent prompts 23–25 audit additionally registers eight
`tusb32x_arduino_app_<scenario>` owner/lifecycle cases and two USB-pin builds.
They execute actual ExampleApp setup/loop, the adapter and shared CLI. Existing
Wire ownership, failed creation/preparation/deletion, USB stalls, exact startup
and CLI result retention, and USB GPIO rejection now have direct application
regressions. Adapter tests also reject unsupported clock/rate configurations
and distinguish zero budget from invalid arguments.

| Requirement / capability | Implementation | Executable evidence |
| --- | --- | --- |
| Read-only startup, exact result ownership, failure without retry | ExampleOwner; primary hands off to shared Cli | tusb32x_example_owner, tusb32x_example_integration |
| Fixed two-target fairness and deferred explicit IRQ service | Secondary ExampleOwner, shared Context, alternating application turns | tusb32x_example_owner; four SDK images |
| USB backpressure independent of owner progress | OutputMailbox; static worker; one CLI ConsoleBuffer | tusb32x_output_mailbox concurrent blocked worker; tusb32x_cli applicationRecordOrdering; tusb32x_example_integration |
| Local shutdown and retained external results | ExampleOwner::stop, ExampleApp shutdown before Wire.end | tusb32x_example_owner, tusb32x_example_integration; actual SDK linkage |
| Explicit variant/address/pins, no universal enable pulse | BoardConfig and board qualification gate | Compile-time/runtime validation; SDK images; physical fixture NOT RUN |
| Existing core/CLI and adapter semantics | Same engine, command table and terminal Wire callbacks | Registered core/CLI/console/adapter regressions |

See [Arduino integration](arduino-examples.md) for exact supply/strap/current,
VBUS_DET and VCONN requirements. Host/SDK checks do not qualify physical behavior.

## Native IDF adapter (prompt 25)

| Requirement / capability | Implementation / source | Named executable evidence |
| --- | --- | --- |
| Borrowed exact target; reject async/invalid configuration before SDK | tusb32x_idf::validateContext; IDF5.5.5 queue-depth selection | localRejections, borrowedLifecycleAndSharing in tusb32x_idf_transport_1000/100 |
| Explicit pointer STOP/read STOP, seven-bit addresses | write/writeRead; TI programming sections cited in native guide | exactTransfers; real idf_s2/idf_s3 links |
| Aggregate finite budget, coarse ticks, unchanged error output | phaseTimeout/transfer; audited SDK wait-site bound | aggregateTime, publicationAtDeadline at 1000/100Hz |
| Full native error detail, no guessed NACK or partial bytes | sdkStatus, BackendStats | failuresAndRetirement, clockAndSaturation |
| Shared controller retirement and non-owning shutdown | BusContext; same-core synchronous SDK lifetime | failuresAndRetirement, borrowedLifecycleAndSharing; application cleanup rules documented |
| Framework-free core and actual native component linkage | Existing root component; test/idf_sdk main and real adapter | 32 registered host suites; S2/S3 firmware.elf/bin without Arduino/CLI sources |

The [native integration guide](idf-transport.md) states the source revision,
exact configuration gates and required application qualification. Fake timing
tests exercise the finite envelope, not physical ISR/waveform behavior. Native
CLI, flashing, HIL and board timing/electrical qualification remain separate.

## Native IDF CLI application (prompt 26)

| Requirement | Actual application/API path | Executed evidence |
| --- | --- | --- |
| Native core + shared CLI without Arduino | basic/main source list, native ExampleApp + adapter | Real IDF5.5.5 S2/S3 default and qualified-path links; symbols/source lists inspected |
| Isolated configuration and renamed roots | Project SDKCONFIG/dependency lock per absolute build directory; directory-derived REQUIRES | tusb32x_idf_example_config real host CMake fixture and renamed-root actual S2 image |
| Explicit safe default wiring and readiness | Native BoardConfig, local validateTarget and exact copied SDK metadata | Diagnostic/pin-conflict, configured/absent IRQ and existing-owner cases |
| Read-only startup ownership | Existing ExampleOwner initialize/refresh/exact take; CLI tick drains only during startup | Startup/failure/full-output cases; no write/ACK/reset/recovery; no unowned result fence |
| Bounded native console | Exclusive UART0 RX64/turn, FIFO offer256, retained suffix, zero-wait SDK calls | Multiline/partial/zero output, before/after-read error and full-event-queue fail-closed cases |
| Outcome/effect retention before deletion | startupOwner.stop/end, CLI snapshot, owned device then bus then console release | Active/startup/retained-result shutdown, creation/deletion failures, no cleanup retries, external bus preserved |
| FIFO completion and finite teardown | uart_wait_tx_done(...,0), 64-turn cap plus optional failed-delete report phase | FIFO pending/stalled, backpressure, console-delete failure and configured-runtime scenarios |
| Actual incremental firmware identity | Example-only custom build target; exact-root Git/dirty/UTC/target | Four Python configuration/identity cases; renamed archive reports unknown rather than parent Git |

Twenty-eight application behavior scenarios are registered separately, all using
actual application/shared/core sources. The full matrix has 61 Windows CTests
(60 elsewhere), with unchanged shared CLI/console/transcript/parity registrations
and both native adapter tick-rate suites. Host fakes and real linked images are
separate proof layers; serial delivery, IRQ/CC/VBUS/electrical timing and board
stack/overhead qualification remain NOT RUN. See the
[native guide](../examples/esp_idf/basic/README.md) for commands and prerequisites.

## Prompt 27 plain C++ owner integration

| Contract | Public consumer/owner behavior | Executed evidence |
| --- | --- | --- |
| Runnable framework-free ordinary owner | Existing host/basic main/profile C++17 TUs, fixed simulated LAI callbacks and synthetic time | tusb32x_host_basic plus standalone tests-OFF CMake/native PlatformIO runs; admission, one-callback ceiling, exact take, status/effect and explicit end |
| Delivery and operation outcome remain separate | Explicit apply followed by external reset and independently selected read-only reconcile | Host requires delivery OK with CONFIGURATION_MISMATCH payload, NONE effect/no replay and KNOWN differing actual configuration |
| Two independent targets sharing external serialization | Existing Scheduler/Bus test fixture; no FieldCore include or product queue in core | Existing waiting/IRQ/failure fairness and queue/result backpressure; staleRequestsPreserveBothOwners adds active/terminal stale-token rejection without peer damage |
| Cancellation/effects and separate owner policy | cancel retains exact result; later caller-selected startReconcile | cancelledApplyNeedsResultTakeAndSeparateReconcile covers pre-write NONE/KNOWN and post-write PARTIAL/UNKNOWN, result fence and read-only actual proof; existing late-write matrix covers INDETERMINATE errors |
| External power/reset notification is local | invalidateDeviceState, unrelated raw read, separately selected reconcile and refresh | externalPowerEventNeedsFreshProofAndPortPublication keeps old observation invalidated until a complete new publication; another owner continues |
| Borrowed bus/context outlives drivers | Bus declared before driver storage; explicit end/result delivery while context live | shutdownEndsBothOwnersBeforeBorrowedBusDies; scope-exit end checks consume zero callbacks and never hold locks between polls |
| Caller-selected original-part startup recovery | Existing typed startRecoverDeadBattery after identity/readiness | waitingOwnerDoesNotHoldBus retains qualified base recovery without copying its sequence into consumer code; read-only reconcile does not certify its history |

The [integration mapping](integration.md) connects generic request/service/result/
diagnostics/power concepts to exact public boundaries. FieldCore paths remain
source evidence, with richer completion/effect fields explicitly outside the
Status-only bridge. Existing independent-header and isolated portable-consumer
checks are reused. No core API, protocol, framework/product dependency or silicon
capability changes; these are simulated host proofs, not Type-C qualification.

## Prompt 28 Python sessions and evidence

| Contract | Implemented boundary | Executed evidence |
| --- | --- | --- |
| Strict schema and framing | Existing contract parser/closure validator plus bounded Session assembly; extension fields cannot replace proof | hil_runner fake serial: fragments, CRLF/ANSI noise, echo-only PASS, missing/duplicate/contradictory records and exact profile sources |
| Request/token ownership | Increasing new ACK/MARK IDs, one original operation and exact result take; older origin traffic stays valid | Query/marker interleaving, wrong/reused/foreign tokens, exhaustion and repeated copies; live C++ original/take exchange |
| Deadlines and provenance | Pre-open finite budgets, immutable host operation deadline, checked late replies and separate firmware/host identities | Virtual clock late write/read/fragment and admission tests; expected unknown identity denial, changed identity/boot/loss invalidation |
| Local restoration and effects | Checkpointed local staging/output/auto values; applicable, available baseline and complete restored profile proof without apply | Absent setter field, wrong validity/variant/raw flag/value rejection; cleanup contradiction invalidates the session; real C++ restoration and failed payload under delivery OK |
| Capture and verdicts | Streamed timestamped TX/RX, bounded tail/counters, individual atomic JSON/Markdown replacement, sticky save_error | Artifact caps, atomic replace/fsync/corrective-save faults, required API end marker/original JOB, primary/cleanup separation and final-save Ctrl-C130 |

The [session guide](hil-runner.md) records exact APIs, limits, optional serial
backend, artifacts and exit conventions. The existing cli_transcript registration
now executes the actual Session against the built C++ fixture; hil_runner adds
fake-serial/artifact tests under one finite registration. No observation suite,
serial hardware or physical Type-C qualification is claimed here.

## Prompt 29 finite observation plans

| Requirement | Implemented path | Executed evidence |
| --- | --- | --- |
| Usable CLI and pre-open guards | main, ObservationPlan; readonly default, repeated suites, target/identity, finite time/capture limits, dry-run | 26 planning/invocation tests; malformed numeric/enum/identity/pair/port/cap guards; no serial open during dry-run |
| Complete readonly | run_plan with cached routes, initialization, zero/one budgets, malformed fences, separate staging, verify precondition, profile/read results | Reactive PlanPeer and full production plan against persistent shared C++ CLI; explicit signed failure and no data-write callbacks |
| Fresh bounded soak | Finite count/quiet-gap/duration/whole-run bounds; relevant raw/field/coherence/sequence/time proof; detached accepted | Fake clock with no sleeps; stale/invalid/unknown/contradictory/saturated evidence, deadline and callback failures; bounded aggregate count/last sample |
| Mapped register observation | Ordinary one-callback reg commands, explicit per-variant maps and preserved read effects | Allowlist checks for all seven variants; actual LAI map in native readonly; exact all-source counts, counter saturation/regression and wrong-attribution denial |
| Ownership and honest outcomes | Original result/query/exact take; no foreign control or retry of BUSY/uncertain consumption; local restoration only | Native output-full reservation retains exact result; older completion after newer queries; failure cleanup, loss/no replay, checkpoint and truthful 0/1/2/130 exits |

The [runner guide](hil-runner.md) and [hardware prerequisites](hardware-validation.md)
describe concrete commands and the physical NOT RUN boundary. These source-checkout
tools do not add Python to consumers or HIL to ordinary tests. No hardware mutation
suite, firmware bulk scheduler, public C++ API or silicon capability was added.

## Prompt 30 explicit fixture plans

| Requirement | Implemented path | Executed software evidence |
| --- | --- | --- |
| Strict setup and finite transitions | Fixture JSON validation, explicit suite/fixture identity, expected firmware and bounded operator input | Pre-open malformed/absent/voltage/capability tests; Windows/POSIX readiness, queued input, late/frozen/regressed clock and typed attestation guards |
| Orientation/accessory | Ordinary apply/verify/read, same reversible partner CC1/CC2, exact supported accessories and local mode | Reactive physical model plus complete persistent C++ plans; reserved/unknown/invalidation and base-versus-extended cases |
| Role/current | Sink peer categories, explicit base reset/revalidation, source category apply/verify, resolved DRP | Fake base attached-current stays stale until reset; source supply gate includes retained sink3A and cleanup; native complete LAI plan |
| IRQ preservation | Wired input preflight, actual50/staged30 ACK, unrelated duty30 write with latch pending, reassert/coalesced service | Independent latch and actual/staged profiles, native real core effects; missing input before I2C, ACK/service failures and pending recheck |
| Safe cleanup/verdicts | Success-only attested detach and original actual profile apply/verify, local-only cleanup after failure | Mutation boundary failures/cancel/deadline, target/attachment changes, exact owned take, blocked device cleanup despite local success, report/interruption survival |

The single hil_runner and cli_transcript registrations execute these tests with
finite timeouts. TUSB322I fixture mutation plans are explicitly gated before
traffic; no firmware command, core scheduler, register composer or power control
was added. All physical serial/CC/INT/current/supply evidence remains NOT RUN.

## Independent audit of prompts 29 and 30

| Requirement | Corrected boundary | Runtime regression |
| --- | --- | --- |
| Immutable host timing | One absolute deadline shared across group and Session entry; original duration remains a ceiling | Delayed construction previously produced late PASS; completed orientation plus a full-run soak ceiling previously rejected; both now obey the original bound |
| Finite external transition | Check the exact callback deadline before accepting either ready or missing input | Exact/late300s reply under1000s run cannot authorize mutation or become timely NOT RUN |
| Actual/staged evidence | CONFIG masks/raw bytes agree with observed fields; complete validated candidate and raw controls precede mutation and match post-verify actual settings | Missing acquisition proof, ignored staging, changed unrelated fields and staged50/applied50 ACK experiment reject before further device work |
| Schema extension compatibility | Compare only frozen known fields; reuse Session result-copy validation | Changing extension telemetry remains recorded without false state/result mismatch |
| Independent native oracle | Literal named LAI input bytes, separate from decoded expectations | Four complete plans plus wrong-CC and wrong-current expectations against the persistent C++ CLI; both mismatches save FAIL and blocked cleanup |

## Prompt 31 and complete nine-family runner audit

| Family | Real plan and prerequisites | Finite end and tested failure evidence |
| --- | --- | --- |
| readonly | run_plan; explicit target/port for hardware, firmware framing, idle ownership; no apply/reset/ACK | One bounded command/operation sequence and end marker; native normal/failed callback/output-full take, foreign ownership and parser-state fences |
| soak | run_plan; current coherent observation, finite count/interval/duration/run ceilings | At most100000 observations under86400s; fake-clock deadline/invalidation/stale/unknown/counter failures, no hidden current-refresh reset |
| orientation | run_fixture_plan; declared reversible partner, expected firmware and supply/startup readiness | Both CC1/CC2 then attested detach/conditional cleanup; actual native plan and deliberately wrong orientation rejection |
| accessory | Same plan; exact declared capability/fixture type and variant | Finite supported accessory list with detach between types; native plan, reserved/absent/unsupported/invalidated rejection |
| role-current | Same plan; controlled dual-role peer and local source supply capability | Finite sink/source categories and DRP; explicit base reset when required; native plan and wrong-current rejection, unexpected partner/supply/failure tests |
| interrupt | Same plan; wired opt-in INT and physical assertion/reassertion | Single bounded ACK/service, actual50/staged30 preservation and unrelated pending latch; native plan and fault/coalescing/pending/missing-input cases |
| reset-termination | Same plan; separately authorized reset, base-only stable-VDD/source-startup recovery action | Explicit normal reset, before-callback cancel/timeout, distinct controls and exact results; complete native LAI/320/320I plans plus literal bytes/31-96ms waits, failed/late/accepted writes and retained uncertainty |
| fault | Same plan; separately authorized NACK/disconnect/power-cycle and exact adapter error codes | No automatic retry or profile restoration; fake and full native LAI faults, local invalidation/raw-read fence and fresh identity; original source power epoch blocks stale startup cleanup |
| console | Same plan plus explicit reconnect owner; authorized finite backpressure/reconnect/reboot, base target-VDD continuity | Whole help response, two fresh Session domains and read-only initialization; complete native two-zero-write path,64-zero-write LOSS failure, reboot/stale/identity/target/cleanup exceptions |

The existing hil_runner loads test_tusb32x_recovery.py; cli_transcript uses the
same explicitly built C++ host. No second scheduler, hardware protocol, result
store or test registration exists. Every new suite has setup/identity/action and
transition rejection tests. Original result codes/details/effects remain raw;
only preregistered exact negative statuses satisfy a negative experiment.
Unexpected failure, missing cleanup, capture loss and interruption remain saved
failed/incomplete evidence. Hardware serial/supply/CC/INT/reset timing remains
NOT RUN. Console may observe322 while its mutation-suite gates remain closed.

## Prompt 32 package mechanics

The original 76-file PlatformIO/IDF contract includes core/public headers, metadata,
licenses and complete shared/framework example inputs. Bounded hostile archive
fixtures run in tusb32x_package_safety; tusb32x_package_consumer extracts a real
archive, renames it, checks target/source origins and runs public cooperative
initialization with tests OFF. The managed-Core distribution is separately packed
and checked; extracted Arduino basic/owner S2/S3 and native CLI S2/S3 images supply
framework link evidence. See [package commands](development.md#archive-and-clean-consumers).
The checker is maintainer tooling, not a core dependency or arbitrary-code sandbox.
No install/export, release publication or physical hardware qualification is claimed.
The current 77-file contract also includes the owner guide; local package checks
verify that addition separately from the earlier extracted firmware links.

## Independent prompts 31/32 audit

Recovery readiness is a saved run fact across suite/session groups: a base UFP
power-cycle experiment cannot authorize a later source apply. Both base variants
exercise separate/combined groups and fresh explicit recovery in runtime tests;
the persistent C++ path checks the same owner fence without claiming POR behavior.
Package proof checks actual compiler header dependencies against extracted,
fixture and toolchain roots. tusb32x_package_origins runs the complete positive
consumer and rejects injected checkout and unrelated outside headers. Directory
properties and translation-unit names alone cannot prove header provenance.

## Local validation guards

The source-checkout tools/validate.py selects disjoint native/contracts/doxygen/
python/package CTest groups and explicit arduino/idf-s2/idf-s3 package image links.
It checks actual inventory and JUnit execution, saves command logs and partial
reports, and fails missing tools or unfinished scope. Boundary negative fixtures
supplement independent headers and the allocation-sentinel consumer. Actual
Doxygen plus HTML/XML visibility, helper comments and broken-reference negatives
cover source documentation. Existing CLI parity, version, archive, session and
transcript registrations are reused; runner modules are not discovered twice.
Numeric separators cannot hide forbidden source tokens; real Doxygen fixtures
reject alias-hidden declarations and undocumented pointer/trailing-return helpers.
Storage-failure and overflow fixtures verify incomplete/interrupted exits and
16 MiB retained command-log bounds. The workflow guard checks actual extracted
image commands, archive identity and matrix-target selection, including no-op
and failure-suppression negatives.
See the repository-only [validation guide](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md#local-validation-entry-point).
No validator selection invokes physical HIL, flashing, publication or remote CI.

## Prompt 34 declared CI checkpoint

The [declared matrix](development.md#declared-ci-matrix) and source-checkout
[workflow](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/.github/workflows/ci.yml)
separate check ownership and consume one identified archive for firmware.
The completed audit's [run 40782369910](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910) passed all 14 jobs on
`c50f8ddda8809425ede2161586466c95fa650ae6`. The rows below have actual artifact evidence;
physical qualification remains NOT RUN.

| Required check | Declared coverage | Evidence boundary |
| --- | --- | --- |
| Host | GCC 14 and Clang 18 Debug/Release; Clang 18 ASan/UBSan Debug | Five Linux rows, strict warnings and active Release assertions; PASS at the recorded checkpoint SHA |
| Python/documentation/package | One Release owner for each registered suite group | Existing validator inventory/JUnit checks reused; PASS at the recorded checkpoint SHA |
| Workflow contract | Actual YAML, verified action release pins, matrix/ownership/artifact bounds | PyYAML 6.0.3 required by check_ci.py; negative fixtures integrated into validate_runner |
| Arduino distribution | Basic CLI and owner service on S2/S3 from the shared archive | Four images, PlatformIO 6.1.19/pioarduino 55.03.311/Arduino 3.3.11; all four links PASS at the recorded SHA |
| Native distribution | Genuine native CLI on S2/S3, IDF 5.5.5 and v6.1 (SDK 6.1.0) | Four images with immutable container digests and isolated configurations; all four links PASS at the recorded SHA |
| Evidence retention | Logs, JUnit, package/consumer provenance and useful firmware outputs | Seven days, expected paths checked; no physical HIL, flash or publication |

The manifest's `idf >=5.3.0` remains portable-core admission metadata. The example
adapter's declared exact SDK audit rows are 5.5.5/6.1.0; the floor does not qualify
the adapter on intervening or later SDKs. Earlier recorded 5.5.5 and Arduino
links retain their original tested revisions. The checkpoint records its own
verified SHA; later commits do not inherit its remote result.
