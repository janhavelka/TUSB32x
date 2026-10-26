# Feature, source and verification coverage

Prompts 2-9, updated 2026-10-26. This matrix defines the capability boundary; passive
values/Status, pure validators, private transport/identity, native builds and host
checks exist, but no device facade, CLI or hardware
qualification exists yet. **Planned** means the
source boundary is established and implementation/testing is still due.
**Gated** means the named dependent operation/claim remains unavailable.
**Outside API** means board/application responsibility or an excluded device.
These are documentation labels, not new runtime health/status values.

Source IDs/revisions and unresolved gate IDs are defined in
[variant matrix](variant-matrix.md). B/L/A name two exact variants each, as
listed there; V names TUSB322I alone. Silicon/operation tests below remain future
evidence; the implemented-values table records the subset delivered in prompts 5-9.
Actual checks are in
[validation](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md). Planned commands follow [CLI_CONTRACT](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/prompts/CLI_CONTRACT.md).

Prompt 3 completed the [register contract](register-map.md) and
[independent vectors](register-vectors.md): map/access/defaults, semantic
encodings, comparison/composition and negative cases. Rows below that name
prompt 3 now have their register specification evidence; implementation and
behavioral tests remain planned. No unresolved gate was closed by byte arithmetic.

Prompt 4 completed the [device procedure guide](device-behavior.md). Every enabled
plan now has exact source citations, prerequisites, wait anchors, retained effects,
verification, callback/TX/RX ceilings and negative/failure vectors. These are
specified operations, not executable API support. The source-to-operation mapping
below supplements the feature rows without turning a future test into a PASS.

| Feature and state | Variants | Evidence class; exact source | Planned API / CLI | Required test or qualification evidence; owning prompts |
| --- | --- | --- | --- | --- |
| Register access/defaults and safe byte composition — Specified; implementation Planned | B/L/A/V with exact write gates | TI fact: B Tables 7-6/7-7 pp16–19, L/A Tables 6–12 pp16–20, V Tables 6-6–6-11 pp17–20; masks, partial base defaults and policy in register guide | CommandTable, codecs, raw diagnostics / reg, wreg, decode | Literal D01–D36/C01–C21/N01–N25; eight access-mask partitions; base unknown defaults never asserted zero; separate profile comparison masks; actual suites due9/10/18/21/22 |
| Explicit variant/address validation — Implemented; binding/CLI Planned | Six 320 parts + V | TI fact: B pins p4/§7.5 p15; L/A pins p3/§7.4 p15; V pins p3/§6.5 p15. Policy: no auto-detection | validateDriverConfig, bind / variant, addr, bind, capabilities | PASS every legal pair, all address/variant bytes and zero callbacks in7; binding/transport/CLI remain9/11/20 |
| Eight-byte identity — Planned | B/L/A: 30 32 33 42 53 55 54 00; V: 32 32 33 42 53 55 54 00 | TI fact: B Table 7-7 p16; L/A Table 7 p16; V Table 6-7 p17 | startInitialize, diagnostics / init, probe, capabilities | Independently fixed vectors; reverse order, wrong NUL, same-address unrelated device, partial/failed transfer, no inference of enable/model; 9/12/18/22 |
| Revision acceptance — Planned; unknown revisions gated | L 01, A/V 02; B no A0 | TI fact: L/A Table 12 p20, V Table 6-11 p20; policy in variant guide; code observation REVISION_06 | startInitialize, readRegisterSync / init, reg, diag | Base never probes A0; unknown/06 returns CHIP_ID_MISMATCH without write permission; raw value retained; mismatch cannot create KNOWN; 9/12/18/22 |
| Read-only initialization and profile evidence — Planned | All seven I2C variants with per-feature gates | TI fact: documented identity/control maps B pp16–19, L/A pp16–20, V pp17–20. Policy: no automatic writes | startInitialize, startReadProfile, startVerifyProfile, startReconcile / init, profile read, verify, reconcile | Read-only transcripts, distinct observed/desired/verified state, missing fields not defaults; 3/6/12/18 |
| Port attachment, role and orientation — Planned | B/L/A/V | TI fact: B §7.3/Table 7-7 pp10–13,17–18; L/A §7.2/Tables 8–9 pp10–12,17–18; V §6.3/Tables 6-8–9 pp9–13,17–19 | startRefreshPort, portSnapshot / read, sample, freshness | Local/peer distinction, CC1/CC2 raw polarity, detached validity, CC/VBUS separately controlled, sequential-coherence changes; 3/15/18/30 |
| Current detection category — Planned; physical freshness gated | B/L/A/V | TI fact/conflict ATTACHED_CURRENT: B §7.3.1.2 p11; L/A §7.2.1.2 p10; V §§6.3.2.2–3 pp10–11; G Figure 2-1 p8 | startRefreshPort / read, sample | Default is not unconditional 500/900 mA; charge-through category separate; no hidden reset on read; controlled attached advertisement-change fixture before fresh-measurement claim; 15/18/30 |
| Current advertisement — Planned B/L/A; gated change on V | B/L/A/V | TI fact: B §7.3.2 p12/Table 7-7 p17, L/A §7.2.2 p11/Table 8 p17; conflict V_ADVERTISEMENT, V Figure 6-3/Table 6-8 p17 | DeviceProfile.sourceCurrent, startApplyProfile / current, apply | Reserved setting rejection, preserve other fields, no-op uses no writes; ≥3.5 V local-source prerequisite documented; V unchanged proved field permits other closed operations, changed field rejects; 3/13/18/30 |
| Base mode selection — Planned | TUSB320, TUSB320I | TI fact: B §§7.4.1–2 p14/Table 7-7 p19 | DeviceProfile.portMode, startApplyProfile / mode, apply | Actual mode change rejects attached; unchanged mode does not block otherwise legal current change; no promise of race-free cable state; 3/4/13/18 |
| Extended mode selection — Planned; LI/HI application caution | LI/HI/LAI/HAI | TI fact: L/A §7.3.2 p14, Tables 9–11 pp18–20; current LI/HI product-page recommendation in variant guide | startApplyProfile / mode, apply, capabilities | Separate termination-disable/mode/enable actions and wait after mode write; failures/cancellation each boundary; no LI/HI DFP/DRP qualification claim; 4/13/18/31 |
| 322 DFP/UFP selection — Planned subject to complete procedure proof | TUSB322I | TI fact: V §§6.3.2.1–2 pp10–11, Table 6-10 p20 | startApplyProfile / mode, apply | Use exact V procedure, no inherited PORT mode or invented 5 ms rule; original errors/effects survive cleanup; 4/13/18 |
| 322 direct return to DRP — Gated | TUSB322I | Unresolved V_RETURN_DRP: V §6.3.2 p9, §6.3.2.3 p11, Table 6-10 p20 | startApplyProfile / mode drp, apply | Resolve exact non-reset transition before enabling changes; valid existing DRP codes remain readable; no implicit reset fallback; 4/13/18 |
| Try.SNK / Try.SRC, UFP accessories, termination disable — Planned; V UFP-accessory writes gated | L/A/V; absent on B | TI fact: L/A §7.2/Tables 8–10 pp10–11,17–19; V §6.3/Tables 6-8–10 pp9–12,17–20; V_FAULT_CLEAR affects 09 bit0 writes | DeviceProfile fields / preference, ufpaccessory, termination, apply | Reject B unsupported fields; reserved Try.SRC code2 never accepted; preferences not guaranteed role; termination effects not conflated with 45; V accessory changes reject without writes until safe 09 composition is proved; 3/4/10/13/18 |
| Duty/debounce and accessory decoding — Planned; V duty writes gated | B/L/A/V, exact per-variant encodings | TI fact: B Table 7-7 pp17–19; L/A Tables 8–10 pp17–19; V Tables 6-8–10 pp17–20; V_FAULT_CLEAR affects 09 bits2:1 writes | validateProfile, codecs, startApplyProfile, portSnapshot / duty, debounce, decode | Base and extended debounce differ; base accessory7 reserved; retain raw unknown values without fabricated semantic state; V duty changes reject without writes, while unchanged proved 09 controls permit independently supported 0A changes; derive complete cases in 3, implement 10/13/15/18 |
| Separate 45 Rd/Rp control — Planned B/L/A; unavailable V | Six 320 parts only | TI fact: B Table 7-7 p19; L/A Table 11 p20; V complete map pp17–20 lacks45 | DeviceProfile.disableRdRp / rd-rp, apply | Internal bits preserved, no register45 on V, clear0A disable does not prove Rd/Rp enabled; safe transition procedure due4; 3/4/10/13/18 |
| Explicit reset — Planned, effect differences retained | B/L/A/V | TI fact: B Table 7-7 p19/§6.7 p7; L/A Table 10 p19/§6.7 p6; V Table 6-10 p20/§5.7 p7/§6.3.2 p9 | startReset / reset | Actual reset mask emitted, no repeat after uncertain write, timed completion proof; six-part PORT resampling, V returns DRP, no automatic desired replay; 4/14/18/31 |
| Post-dead-battery Rp recovery — Planned B only | TUSB320, TUSB320I | TI fact: B §7.4.3 p14; conservative identity/readback prerequisites are library policy | startRecoverDeadBattery / recover-dead-battery | Exact base-only operation, owner-qualified VDD, transient statuses not published, timed sequence/effects tested; other variants reject before callbacks; 4/14/18/31 |
| IRQ observation — Planned; ACK / bounded service — Planned B/L/A, gated V | B/L/A/V | TI fact: B Table 7-7 p18; L/A Table 9 p18; V Table 6-9 pp18–19; N §8 p9. Unresolved V_FAULT_CLEAR | startRefreshPort, startAcknowledgeInterrupt, startServiceInterrupt / irq read, irq ack, irq service | Explicit W1C, actual controls preserved, no implicit ACK in reads, new event during ACK/recheck; V raw fault retained, ACK/service rejected before any write until safe 09 composition is proved; 3/4/16/18/30 |
| VCONN — Autonomous silicon effect, no software enable API; fault clear gated | TUSB322I only | TI fact: V §5.3 p4/§5.5 p6/§6.3.8 p13; unresolved Table 6-9 p19 | Raw port/fault evidence / read, decode; no VCONN setter | Ra/source/detach physical fixture with board-owned 5 V VDD prerequisite and 4.75–5.5 V VCONN qualification; limit225/300/375 mA, R_ON1.25 Ω max documented; active cable not powered-VCONN proof; six-part APIs reject invented control; 3/15/30 |
| Safe raw register diagnostics — Planned; V 09 writes gated | All seven, exact allowlists | TI fact: complete maps above; policy: explicit pointers, no bypass of identity/access/procedure gates including V_FAULT_CLEAR | readRegisterSync, writeRegisterSync / reg, wreg, decode | Base A0/V45 absent; holes/wrap rejected; mixed fields safely composed; V 09 raw writes rejected until safe fault handling is proved; reset/role/termination sequence bypass rejected; 3/9/10/21/22 |
| GPIO-only parts — Outside API | TUSB321 (321I in G summary), TUSB321AI (321A summary) | TI fact: G0/G1 §5 p3 and G Figure 2-1 p8 | No Variant members or probe addresses | Enum/address negative tests; guide identifies no I2C/enable pin; 5/9/35 |
| Power, straps, pin levels, bus timing — Outside API | Each exact variant | TI fact/conflicts: detailed tables and STARTUP_EN/SUPPLY_SUMMARY/L_CC_RATING/ID_OFF_RATING/BASE_VALID_TIME in variant guide | Application adapter/BoardConfig; bus/capabilities reports assumptions only | Wiring/supply/enable traces, aggregate adapter timing and fixture identity; ACK is not electrical qualification; 4/23–27/30–31 |
| USB policy, PD and board interchangeability — Outside API | Entire family | TI fact: N §3 p4; B §7.3.1.1 p11, L/A §7.2.1.1 p10 and V §6.3.2.1 p10 DFP exception; exact pin tables | No USB stack, charger/VBUS switch or mux control in core | Framework-free consumer builds; no PD/e-marker/ADC claims; board-specific orientation mapping and interoperability qualification; 19/27/30/35 |

## Procedure-to-API, CLI and test coverage

Source IDs and exact revisions above apply to these citations. P/T vector IDs
refer to the device guide, with arithmetic verification recorded in validation.

| Specified plan / variants | TI source and selected policy | Planned API / CLI | Future evidence |
| --- | --- | --- | --- |
| Explicit pointer/STOP transport; B/L/A/V | B §7.5 pp15–16; L/A §7.4 p15; V §6.5 pp15–16; one aggregate terminal callback | Callback adapters / bus, reg | P27–P28; lock across both phases, no retries/pending DMA; scope traces and aggregate SDK bounds in 9/23/25 |
| I/P/F read-only proof; B/L/A/V | Exact identity/map tables above; scalar controls, fixed one-pass proof and incomplete-effect restrictions are policy | startInitialize, startReadProfile, startVerifyProfile, startReconcile / init, probe, profile read, verify, reconcile | P07/P20/P28, any reset discovery retains pending evidence, no second F; desired mismatch and recovery history retained;9/11–12/18 |
| Ordinary settings/45; B/L/A, only permitted 0A changes on V | B Table 7-7 pp17–19; L/A Tables 8–11 pp17–20; V Table 6-10 pp19–20 | startApplyProfile / current, duty, debounce, preference, termination, rd-rp, apply | P04/P08/P11–P12/P29; known internal 03=00 before W45 (conservative B policy), preserve independent gates and actual controls;10/13/18 |
| Base mode; B | B §§7.4.1–2 p14/Table 7-7 p19; owner excludes cable race | startApplyProfile / mode, apply | P09–P11; precheck before any write, no blanket attached-current ban;13/18/30 |
| Extended role; L/A | L/A §7.3.2 p14; held-disabled final state policy from Table 10 p19 | startApplyProfile / mode, termination, apply | P01–P04; T01–T02/T05–T12; failure at every separate write, six-tick wait;13/18–19/31 |
| DFP/UFP role; V | V §§6.3.2.1–2 pp10–11, Table 6-10 p20; no added 5 ms | startApplyProfile / mode, apply | P05–P08; no direct return-to-DRP or hidden reset; unchanged gated 08/09 permit independent 0A work;13/18 |
| One-shot reset; B/L/A/V | B Table 7-7 p19/§6.7 p7; L/A Table 10 p19/§6.7 p6; V Table 6-10 p20/§5.7 p7/§6.3.2 p9 | startReset / reset | P17–P20; T04/T07–T12; 96 ticks, actual controls not defaults, V semantic-DRP consistency policy, no desired replay;14/18–19/31 |
| Base recovery; B only | B §7.4.3 p14/Table 7-7 p19; identity, internal-zero/pre-read, final proof and other-transition gate are library policy | startRecoverDeadBattery / recover-dead-battery | P13–P16/P30; T03/T06–T12; 4 callbacks/6 TX/2 RX,31 ticks, no transient publication/hidden cleanup;14/18–19/31 |
| Fixed port observation; B/L/A/V | Status tables above;09/08/09 and E0 coherence comparison are policy | startRefreshPort / read, sample, irq read | P24/P26/P28; no retry-until-stable, no fresh physical-current inference;15/18/30 |
| ACK/service; B/L/A only | B Table 7-6 p16/Table 7-7 p18; L/A Table 9 p18; N §8 p9; bounded recheck policy | startAcknowledgeInterrupt, startServiceInterrupt / irq ack, irq service | P21–P26; 2 or 5 callbacks, raw observations survive later error, coalescing/reassertion, V09 writes gated;16/18/30 |

All procedure implementations and behavioral/hardware tests remain planned.
V_ADVERTISEMENT, V_FAULT_CLEAR, V_RETURN_DRP and the board/electrical/current
gates remain open. Prompt 4 did not qualify adapters or close those gates.

The completion path is deliberately incremental: prompt 3 closed per-register
read/write permissions and encodings; prompt 4 closed enabled silicon procedures
and effect/timing specification; prompts 5–6 fix the value/state contracts using those
results. Later rows become implemented/tested only when their real suites and
relevant physical checks run. Gated rows remain explicit without blocking
independent, verified capabilities.

## Prompts 5-8 implemented values, validators and planned API mapping

These entries distinguish executable passive-value checks from future device
behavior. Exact fields and planned signatures are in [API contracts](api-contracts.md).

| Feature / state | API or value | Source / provenance | Actual check; remaining owner |
| --- | --- | --- | --- |
| Status — Implemented | Err, Status, errorName | Engineering baseline; LDC/LSM representation and TMP constexpr/explicit-bool mechanism in current peer audit | PASS 23 names/order, int32 extremes, invalid code names, constant evaluation, explicit conversion and multiple TUs; private callback normalization and all invalid Err values9 |
| Transport/settings — Values, validators and private transport implemented | I2cWriteFn, I2cWriteReadFn, DriverConfig, DeviceProfile, validators; private protocol helpers | Callback baseline plus exact variant/register tables B pp15-19, L/A pp15-20, V pp15-20 | PASS signatures/defaults/standalone headers, exhaustive validation, exact scripted frames/access/ranges, output preservation, strict identity/revision, failed-write acceptance and all/managed saturating accounting9; owner execution11-17 |
| Configuration/port — Passive values implemented | ConfigurationSnapshot, ProfileField, PortSnapshot, PortField | Baseline and register/device guides; explicit raw/typed validity | PASS defaults, mask values and independent raw copy; publication/lifetime6, observations12/15 |
| Operation/diagnostics — Passive values implemented | OperationToken/Timing/Result/Progress, identity/IRQ values, DriverDiagnostics | Owner baseline and prompt4 retained effect rules | PASS fixed values/default invalidity and token domain; admission/time6/11, retained IRQ16, diagnostics17 |
| Health — Passive values and private evidence accounting implemented | DriverState, ManagedTransportEvidence, HealthPolicy/Snapshot | Baseline policy; no TI health claim | PASS defaults and private managed/raw totals/streak/timestamp separation9; public validation/projection/views17 |
| Decoder — Result implemented, function Planned | RegisterDecode; decodeRegister(Variant, uint8_t, uint8_t, RegisterDecode&) | Register-map explicit field tables; variant-specific reserved/access restrictions | PASS passive defaults and planned signature syntax; independent decode/error/output-preservation vectors10, CLI formatting22 |
| Facade — Planned | Exact inventory in api-contracts.md and compile-only fixture | Baseline exact signatures and completed procedure gates | PASS syntax/noncopy/nonmove checks only; callable engine9-17 |
| Host foundation — Implemented | TUSB32x target, tusb32x_compile_contracts and tusb32x_validators suites; examples/host/basic | Prompt7 and current peer build audit | Debug/Release native linkage, independent headers and standalone tests-OFF multi-file consumer; source-tree consumption only, packaging8/32, firmware23-27 |
| Source docs — Present | Five public headers with file/type/member/enum/function and generated version Doxygen | Maintainability baseline and current peer documentation conventions | PASS strict Doxygen, visibility of17 public structs and four version constants; complete omission guard33 |
| Metadata/version — Implemented | library.json truth, Version.h, generator sync/check/bump/set, aligned CMake/component/Doxygen fields | Current metadata peer audit and official manifest/build documentation | Isolated version fixtures, duplicate-declaration rejection, no-Git/enclosing-Git/dirty-example/no-op behavior; tests-OFF consumers need no maintainer tools |
| Component/native tool configuration — Implemented; firmware Planned | Explicit-source IDF registration, managed pio.cmd, pinned native PlatformIO consumer | IDF 5.3.2 and platformio/native1.2.1 upstream sources | Host registration fixture, launcher argument/exit fixtures and native linkage; native-IDF firmware25-26, full packages32 |

No pass above represents a successful I2C transaction,
silicon decoder, framework build, CI run or physical qualification.

## Prompt 6 completed owner contracts

All rows below are specified, with real checks limited to passive values and
compile-only fixtures. [Method/state rules](api-contracts.md#method-admission-and-output-preservation)
and future vectors O01–O25 are normative for their later implementation stages.
Peer provenance is [execution-6-peers](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/audit/execution-6-peers.md); register and
procedure sources remain the exact TI revisions already listed above.

| Feature / variant | API / CLI mapping | Completed specification and actual evidence | Future behavioral owner |
| --- | --- | --- | --- |
| Admission/results, all bound I2C variants | bind/start/poll/cancel/end/take; CLI operation correlation | Exact precedence, output preservation, one token/result slot, lifetime and different-target gates; signature and owner-example syntax PASS | 11, conformance18/19, CLI20–22 |
| Time/callback budgets, all | OperationTiming/Progress; owner scheduling, CLI limits | Immutable half-range deadlines, one callback, fresh wait anchors, immediate transport failure, six local stages; 60 constexpr vectors and earlier fractional-time arithmetic PASS | 9/11/14/19, adapters23/25/26 |
| Configuration proof, exact applicable fields | initialize/read/apply/verify/reconcile; CLI profile commands | I/P/F publication, generations, identity readiness versus KNOWN, mismatch semantics and no replay; passive configurationWriteUnresolved default/copy PASS | 11–14/17–19 |
| Recovery/reset/termination, variant-gated | reset/recover; explicit CLI mutations | Separate obligations, current disable controls, acceptance and completion; p4 sequence ceilings unchanged; future O08/O09/O18/O19 | 13/14/18/19 |
| Port/IRQ, B/L/A plus gated V | refresh/ACK/service; CLI observation and service | S timestamps/bookends/validity; publication before ACK; partial evidence retained; no queue or automatic reservice; future O20/O21 | 15/16/18/21/22 |
| Raw diagnostics/health, capability-gated | read/write Sync, stats/diagnostics/projectHealth; CLI reg/wreg/probe/health | Raw/all/managed accounting, unknown callback times, passive thresholds, no OFFLINE gate; source Doxygen PASS; future O22/O23/O25 | 17–19/21/22 |
| Generic external owner, all | Two driver/token pairs on borrowed shared bus | Passive observation/apply/cancel examples and uint32 extension compile under strict C++17; no FieldCore include or driver implementation | Real generic owner27; framework adapters23–26 |

No runtime operation, health helper or validator is linked by prompt 6. The
configurationWriteUnresolved addition is a passive value; its update rules still
need the future write/effect engine tests. Hardware and firmware remain NOT RUN.
