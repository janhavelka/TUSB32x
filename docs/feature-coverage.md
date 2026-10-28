# Feature, source and verification coverage

Prompts 2-18, updated 2026-10-27. This matrix defines the capability boundary;
values/Status, pure validators/decoding/composition, private transport/identity,
the owner facade with read-only initialization, profile observation and
reconciliation, explicit apply/desired verification, reset/base recovery, complete
read-only port refresh, explicit IRQ acknowledgement/bounded service, native
builds, raw diagnostics, passive health and host checks exist. CLI and hardware
qualification remain planned.
**Planned** means the
source boundary is established and implementation/testing is still due.
**Gated** means the named dependent operation/claim remains unavailable.
**Outside API** means board/application responsibility or an excluded device.
These are documentation labels, not new runtime health/status values.

Source IDs/revisions and unresolved gate IDs are defined in
[variant matrix](variant-matrix.md). B/L/A name two exact variants each, as
listed there; V names TUSB322I alone. Later operation tests below remain future
evidence; the implemented-values table records the subset delivered in prompts 5-17.
Actual checks are in
[validation](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md). Planned commands follow [CLI_CONTRACT](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/prompts/CLI_CONTRACT.md).

Prompt 3 completed the [register contract](register-map.md) and
[independent vectors](register-vectors.md): map/access/defaults, semantic
encodings, comparison/composition and negative cases. Rows below that name
prompt 3 now have their register specification evidence. Read-only initialization,
profile observation, apply, verification, reconciliation, reset, base recovery,
port refresh and IRQ acknowledgement/service have owner behavior; raw diagnostics and passive health are also delivered. CLI and framework
adapters remain planned. Prompt 10 implements and tests the pure
field codecs/comparison and qualified byte composer, without executing any
procedure. No unresolved gate was closed by byte arithmetic.

Prompt 4 completed the [device procedure guide](device-behavior.md). Every enabled
plan now has exact source citations, prerequisites, wait anchors, retained effects,
verification, callback/TX/RX ceilings and negative/failure vectors. Initialization,
profile reads and read-only reconciliation are implemented through prompt 12;
prompt 13 adds explicit profile application and desired verification; prompt 14
adds explicit reset and original-part recovery; prompt 15 adds complete port
refresh and cached publication; prompt 16 adds explicit IRQ acknowledgement and
bounded service; prompt 17 adds raw diagnostics and passive health. Prompt 18's
[protocol conformance matrix](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md)
maps the complete core requirement set to actual runtime test functions and
commands, including both legal addresses and both outcomes of failed raw writes.
The source-to-operation mapping
below supplements the feature rows without turning a future test into a PASS.

| Feature and state | Variants | Evidence class; exact source | Planned API / CLI | Required test or qualification evidence; owning prompts |
| --- | --- | --- | --- | --- |
| Register access/defaults and safe byte composition — Pure helpers and safe owner raw access implemented | B/L/A/V with exact write gates | TI fact: B Tables 7-6/7-7 pp16–19, L/A Tables 6–12 pp16–20, V Tables 6-6–6-11 pp17–20; masks, partial base defaults and policy in register guide | CommandTable, codecs, raw diagnostics / reg, wreg, decode | Literal D01–D36/C01–C21/N01–N25; eight access-mask partitions; base unknown defaults never asserted zero; separate profile comparison masks; PASS private access/transport9 and pure codecs/composition10; owner conformance18 PASS; CLI21/22 remains planned |
| Explicit variant/address validation and binding — Implemented; CLI Planned | Six 320 parts + V | TI fact: B pins p4/§7.5 p15; L/A pins p3/§7.4 p15; V pins p3/§6.5 p15. Policy: no auto-detection | validateDriverConfig, bind / variant, addr, bind, capabilities | Every legal pair, address/variant rejection and zero validator callbacks7; atomic owner binding and target association11; CLI20 |
| Eight-byte identity — Implemented through initialization | B/L/A: 30 32 33 42 53 55 54 00; V: 32 32 33 42 53 55 54 00 | TI fact: B Table 7-7 p16; L/A Table 7 p16; V Table 6-7 p17 | startInitialize, diagnostics / future init, probe, capabilities | Independent fixed/private transport vectors9 and read-only engine11; failure/mismatch retains raw evidence without inferred model; broader conformance18, CLI22 |
| Revision acceptance — Implemented through initialization; unknown revisions gated | L 01, A/V 02; B no A0 | TI fact: L/A Table 12 p20, V Table 6-11 p20; policy in variant guide; code observation REVISION_06 | startInitialize; readRegisterSync / init, reg, diag | Private comparisons9 and read-only engine11; B omits A0, unknown revision retains raw/mismatch evidence and cannot create KNOWN; conformance18, CLI22 |
| Read-only initialization/profile observation/reconciliation/verify — Implemented | All seven I2C variants with per-feature gates | TI fact: documented identity/control maps B pp16–19, L/A pp16–20, V pp17–20. Policy: no automatic writes | startInitialize/startReadProfile/startReconcile/startVerifyProfile / future init, profile read, verify, reconcile | Read-only initialize transcripts11 and P/F owner sequences12; separate raw/typed validity, partial-read preservation, no desired adoption or writes, all applicable termination controls and retained reset wait; desired mismatch and full-F verification13, conformance18 |
| Port attachment, role and orientation — Implemented; physical mapping unqualified | B/L/A/V | TI fact: B §7.3/Table 7-7 pp10–13,17–18; L/A §7.2/Tables 8–9 pp10–12,17–18; V §6.3/Tables 6-8–9 pp9–13,17–19 | startRefreshPort, portSnapshot / read, sample, freshness | Local/peer distinction, CC1/CC2 raw polarity, detached validity, CC/VBUS separately controlled, sequential-coherence changes; 3/15/18/30 |
| Current detection category — Implemented decoding/publication; physical freshness gated | B/L/A/V | TI fact/conflict ATTACHED_CURRENT: B §7.3.1.2 p11; L/A §7.2.1.2 p10; V §§6.3.2.2–3 pp10–11; G Figure 2-1 p8 | startRefreshPort / read, sample | Default is not unconditional 500/900 mA; charge-through category separate; no hidden reset on read; controlled attached advertisement-change fixture before fresh-measurement claim; 15/18/30 |
| Current advertisement — Implemented B/L/A; gated change on V | B/L/A/V | TI fact: B §7.3.2 p12/Table 7-7 p17, L/A §7.2.2 p11/Table 8 p17; conflict V_ADVERTISEMENT, V Figure 6-3/Table 6-8 p17 | DeviceProfile.sourceCurrent, startApplyProfile / current, apply | Reserved setting rejection, preserve other fields, no-op uses no writes; ≥3.5 V local-source prerequisite documented; V unchanged proved field permits other closed operations, changed field rejects; 3/13/18/30 |
| Base mode selection — Implemented | TUSB320, TUSB320I | TI fact: B §§7.4.1–2 p14/Table 7-7 p19 | DeviceProfile.portMode, startApplyProfile / mode, apply | Actual mode change rejects attached; unchanged mode does not block otherwise legal current change; no promise of race-free cable state; 3/4/13/18 |
| Extended mode selection — Implemented; LI/HI application caution | LI/HI/LAI/HAI | TI fact: L/A §7.3.2 p14, Tables 9–11 pp18–20; current LI/HI product-page recommendation in variant guide | startApplyProfile / mode, apply, capabilities | Separate termination-disable/mode/enable actions and wait after mode write; failures/cancellation each boundary; no LI/HI DFP/DRP qualification claim; 4/13/18/31 |
| 322 DFP/UFP selection — Implemented with write gates | TUSB322I | TI fact: V §§6.3.2.1–2 pp10–11, Table 6-10 p20 | startApplyProfile / mode, apply | Use exact V procedure, no inherited PORT mode or invented 5 ms rule; original errors/effects retained without cleanup; 4/13/18 |
| 322 direct return to DRP — Gated | TUSB322I | Unresolved V_RETURN_DRP: V §6.3.2 p9, §6.3.2.3 p11, Table 6-10 p20 | startApplyProfile / mode drp, apply | Resolve exact non-reset transition before enabling changes; valid existing DRP codes remain readable; no implicit reset fallback; 4/13/18 |
| Try.SNK / Try.SRC, UFP accessories, termination disable — Implemented; V UFP-accessory writes gated | L/A/V; absent on B | TI fact: L/A §7.2/Tables 8–10 pp10–11,17–19; V §6.3/Tables 6-8–10 pp9–12,17–20; V_FAULT_CLEAR affects 09 bit0 writes | DeviceProfile fields / preference, ufpaccessory, termination, apply | Reject B unsupported fields; reserved Try.SRC code2 never accepted; preferences not guaranteed role; termination effects not conflated with 45; V accessory changes reject without writes until safe 09 composition is proved; 3/4/10/13/18 |
| Duty/debounce and accessory decoding — Codes/apply/port implemented; V duty writes gated | B/L/A/V, exact per-variant encodings | TI fact: B Table 7-7 pp17–19; L/A Tables 8–10 pp17–19; V Tables 6-8–10 pp17–20; V_FAULT_CLEAR affects 09 bits2:1 writes | validateProfile, codecs, startApplyProfile, portSnapshot / duty, debounce, decode | Base and extended debounce differ; base accessory7 reserved; retain raw unknown values without fabricated semantic state; V duty changes reject without writes, while unchanged proved 09 controls permit independently supported 0A changes; derive complete cases in 3, implement 10/13/15/18 |
| Separate 45 Rd/Rp control — Implemented B/L/A; unavailable V | Six 320 parts only | TI fact: B Table 7-7 p19; L/A Table 11 p20; V complete map pp17–20 lacks45 | DeviceProfile.disableRdRp / rd-rp, apply | Internal bits preserved, no register45 on V, clear0A disable does not prove Rd/Rp enabled; safe transition procedure specified4 and implemented13; 3/4/10/13/18 |
| Explicit reset — Implemented, effect differences retained | B/L/A/V | TI fact: B Table 7-7 p19/§6.7 p7; L/A Table 10 p19/§6.7 p6; V Table 6-10 p20/§5.7 p7/§6.3.2 p9 | startReset / reset | Actual reset mask emitted, no repeat after uncertain write, timed completion proof; six-part PORT resampling, V returns DRP, no automatic desired replay; 4/14/18/31 |
| Post-dead-battery Rp recovery — Implemented B only | TUSB320, TUSB320I | TI fact: B §7.4.3 p14; conservative identity/readback prerequisites are library policy | startRecoverDeadBattery / recover-dead-battery | Exact base-only operation, owner-qualified VDD, transient statuses not published, timed sequence/effects tested; other variants reject before callbacks; 4/14/18/31 |
| IRQ observation — Implemented in refresh; ACK / bounded service — Implemented B/L/A, gated V | B/L/A/V | TI fact: B Table 7-7 p18; L/A Table 9 p18; V Table 6-9 pp18–19; N §8 p9. Unresolved V_FAULT_CLEAR | startRefreshPort, startAcknowledgeInterrupt, startServiceInterrupt / future irq read, irq ack, irq service | Explicit W1C, fresh actual controls preserved, no implicit ACK in reads, separate before/attempt/acceptance/after evidence; V raw fault retained, ACK/service rejected before traffic; finite race/failure tests16, conformance18 and physical qualification30 |
| VCONN — Autonomous silicon effect, no software enable API; fault clear gated | TUSB322I only | TI fact: V §5.3 p4/§5.5 p6/§6.3.8 p13; unresolved Table 6-9 p19 | Raw port/fault evidence / read, decode; no VCONN setter | Ra/source/detach physical fixture with board-owned 5 V VDD prerequisite and 4.75–5.5 V VCONN qualification; limit225/300/375 mA, R_ON1.25 Ω max documented; active cable not powered-VCONN proof; six-part APIs reject invented control; 3/15/30 |
| Safe raw register diagnostics — Implemented; V 08/09 writes gated | All seven, exact allowlists | TI fact: complete maps above; policy: explicit pointers, no bypass of identity/access/procedure gates including V_FAULT_CLEAR | readRegisterSync, writeRegisterSync / reg, wreg, decode | Base A0/V45 absent; holes/wrap rejected; mixed fields safely composed; V 09 raw writes rejected until safe fault handling is proved; reset/role/termination sequence bypass rejected; runtime diagnostics17, conformance18/21/22 |
| GPIO-only parts — Outside API | TUSB321 (321I in G summary), TUSB321AI (321A summary) | TI fact: G0/G1 §5 p3 and G Figure 2-1 p8 | No Variant members or probe addresses | Enum/address negative tests; guide identifies no I2C/enable pin; 5/9/35 |
| Power, straps, pin levels, bus timing — Outside API | Each exact variant | TI fact/conflicts: detailed tables and STARTUP_EN/SUPPLY_SUMMARY/L_CC_RATING/ID_OFF_RATING/BASE_VALID_TIME in variant guide | Application adapter/BoardConfig; bus/capabilities reports assumptions only | Wiring/supply/enable traces, aggregate adapter timing and fixture identity; ACK is not electrical qualification; 4/23–27/30–31 |
| USB policy, PD and board interchangeability — Outside API | Entire family | TI fact: N §3 p4; B §7.3.1.1 p11, L/A §7.2.1.1 p10 and V §6.3.2.1 p10 DFP exception; exact pin tables | No USB stack, charger/VBUS switch or mux control in core | Framework-free consumer builds; no PD/e-marker/ADC claims; board-specific orientation mapping and interoperability qualification; 19/27/30/35 |

## Procedure-to-API, CLI and test coverage

Source IDs and exact revisions above apply to these citations. P/T vector IDs
refer to the device guide, with arithmetic verification recorded in validation.

| Specified plan / variants | TI source and selected policy | Planned API / CLI | Future evidence |
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
| ACK/service; B/L/A only; implemented | B Table 7-6 p16/Table 7-7 p18; L/A Table 9 p18; N §8 p9; bounded recheck policy | startAcknowledgeInterrupt, startServiceInterrupt / future irq ack, irq service | P21–P26; ACK 2 callbacks/3 TX/1 RX, service 5 callbacks/6 TX/4 RX; complete pre-ACK observation survives later error; coalescing/reassertion, V rejects before traffic;16/18/30 |

Initialization, profile observation, apply, desired verification, read-only
reconciliation, reset, base recovery, port refresh and IRQ acknowledgement/service
have host behavioral tests. Other procedure implementations and
all hardware qualification remain planned.
V_ADVERTISEMENT, V_FAULT_CLEAR, V_RETURN_DRP and the board/electrical/current
gates remain open. Prompt 4 did not qualify adapters or close those gates.

The completion path is deliberately incremental: prompt 3 closed per-register
read/write permissions and encodings; prompt 4 closed enabled silicon procedures
and effect/timing specification; prompts 5–6 fix the value/state contracts using those
results. Later rows become implemented/tested only when their real suites and
relevant physical checks run. Gated rows remain explicit without blocking
independent, verified capabilities.

## Prompts 5-10 implemented values, validators, codecs and planned API mapping

These entries distinguish executable passive-value checks from future device
behavior. Exact fields and planned signatures are in [API contracts](api-contracts.md).

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
| Component/native tool configuration — Implemented; firmware Planned | Explicit-source IDF registration, managed pio.cmd, pinned native PlatformIO consumer | IDF 5.3.2 and platformio/native1.2.1 upstream sources | Host registration fixture, launcher argument/exit fixtures and native linkage; native-IDF firmware25-26, full packages32 |

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

| Feature / variant | API / CLI mapping | Completed specification and actual evidence | Future behavioral owner |
| --- | --- | --- | --- |
| Admission/results, all bound I2C variants | bind/start/poll/cancel/end/take; CLI operation correlation | Exact precedence, output preservation, one token/result slot, lifetime and different-target gates; signature and owner-example syntax PASS | 11, conformance18/19, CLI20–22 |
| Time/callback budgets, all | OperationTiming/Progress; owner scheduling, CLI limits | Immutable half-range deadlines, one callback, fresh wait anchors, immediate transport failure, six local stages; 60 constexpr vectors and earlier fractional-time arithmetic PASS | 9/11/14/19, adapters23/25/26 |
| Configuration proof, exact applicable fields | initialize/read/apply/verify/reconcile; CLI profile commands | I/P/F publication, generations, identity readiness versus KNOWN, mismatch semantics and no replay; passive configurationWriteUnresolved default/copy PASS | 11–14/17–19 |
| Recovery/reset/termination, variant-gated | reset/recover; explicit CLI mutations | Separate obligations, current disable controls, acceptance and completion; p4 sequence ceilings unchanged; future O08/O09/O18/O19 | 13/14/18/19 |
| Port/IRQ, B/L/A plus gated V | refresh/ACK/service; future CLI observation and service | Delivered S timestamps/bookends/validity, publication before ACK, retained complete/partial evidence and finite work; O20/O21; no queue or automatic reservice | 15/16, broader conformance18 and CLI21/22 |
| Raw diagnostics/health, capability-gated | read/write Sync, stats/diagnostics/projectHealth; CLI reg/wreg/probe/health | Raw/all/managed accounting, unknown callback times, passive thresholds, no OFFLINE gate; source Doxygen and public behavioral diagnostics17; O22/O23/O25 | broader conformance18/19, CLI21/22 |
| Generic external owner, all | Two driver/token pairs on borrowed shared bus | Passive observation/apply/cancel examples and uint32 extension compile under strict C++17; no FieldCore include or driver implementation | Real generic owner27; framework adapters23–26 |

No runtime operation, health helper or validator is linked by prompt 6. The
configurationWriteUnresolved addition was passive at that stage; prompt 13 now
exercises its managed apply/effect update rules. Hardware and firmware remain NOT RUN.

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
Prompt 18 completes the core protocol coverage audit. Prompt 19 completes the core owner portability checks. CLI,
adapters and electrical qualification remain later work.

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
