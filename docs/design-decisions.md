# Engineering decisions

Prompt 1, 2026-10-23. These decisions confirm the selected
[engineering baseline](prompts/ENGINEERING_BASELINE.md) for this execution.
They describe required future behavior, not implemented features. Current source
evidence is in [the comparison](library-comparison.md) and its linked reports.
Short revisions below identify the full hashes in the execution inventory.

| Concern | Peer file/symbol | Revision | Selected rule | Reason | Check |
| --- | --- | --- | --- | --- | --- |
| Ownership | LSM6DS3TR Config.h callbacks; ADS1115 include/ADS1115/Config.h:175 DriverConfig; FieldCore I2cOwnerTransport.h | 513c564 / 381236e / 4e6a05e | Application owns transport, pins, locks, clock, scheduling and recovery; callbacks borrow context | Supports ordinary C++, bare metal and framework owners | Independent headers, fake callbacks and generic two-target consumer in prompts 7/19/27 |
| Status | LDC1614 and LSM6DS3TR include/*/Status.h; TMP1x2 Status.h | c0ab4c1 / 513c564 / bafc340 | Exact Err and Status vocabulary from baseline; static messages, int32 detail, explicit bool | Familiar shape without importing unrelated error codes | Constant expressions, full detail, invalid enums and callback normalization in 5/9 |
| Naming/layout | LDC1614 Config.h/Status.h and TMP1x2 include/src/examples/tools | c0ab4c1 / bafc340 | TUSB32x namespace/class, PascalCase types, camelCase functions, _camelCase members; standard C++17 | One readable public vocabulary and stable responsibilities | Public consumers and source review in 5/7/19 |
| Language/build | TMP1x2 and 24Cxx CMakeLists.txt; ADS1115 platformio.ini | bafc340 / 936aa85 / 381236e | C++17 core, standalone CMake plus IDF component; tests OFF by default | Peers differ: ADS native/IDF C++11 is not the chosen contract | Clean core build without Git/Python/Doxygen/framework headers in 7/32 |
| Binding | TMP1x2.cpp bind; ADS1115.cpp bind | bafc340 / 381236e | Validate before replacing binding; include callbacks in target association | TMP's early teardown can lose a usable binding on invalid input | Invalid rebind preserves state; changed callbacks with null context in 11 |
| Work/results | LSM6DS3TR.cpp takeResult; ADS1115 owner methods; LDC1614 poll | 513c564 / 381236e / c0ab4c1 | One active operation, one retained terminal slot, instance-local nonreused tokens; token-checked poll | Small state machine and explicit backpressure, no generic scheduler | Token/slot/lifecycle cases in 11/19 |
| Teardown | OPT4001.cpp end; LDC1614.cpp end; LSM6DS3TR.cpp unbind | dab818e / c0ab4c1 / 513c564 | end is bus-silent and retains results/effects; no reset or rollback | Reject OPT power-down write and peers' evidence-clearing teardown | Callback-zero checks and retained cancellation in 11 |
| Deadline/partial write | INA3221 PollContext/HardwareEffect; FieldCore I2cBusObservation | e45622d / 4e6a05e | Terminal callbacks with aggregate finite budgets; fresh owner time after callback; uncertain writes stay uncertain | Status-only callback carries less evidence than FieldCore transport | Failure-after-write, quantization and immutable deadlines in 9/13/19/23/25 |
| Profiles | ADS1115 AppliedProfileSnapshot; INA3221 startReconcile | 381236e / e45622d | Separate desired/observed/verified state; reconcile only observes | INA reconciliation replay is unsuitable for role/reset uncertainty | No-op apply, interrupted writes and read-only reconcile in 12/13/18 |
| Health/counters | LDC1614 TransportStats; LSM6DS3TR diagnostics; TMP1x2 track | c0ab4c1 / 513c564 / bafc340 | All-callback totals plus managed subset; passive UNINIT/READY/DEGRADED/OFFLINE projection | No hidden diagnostic traffic or health-based admission policy | Counting, saturation and projection table in 9/17 |
| Byte registers | PCA9555.cpp; BME280.cpp and protocol tests | 498178e / d538f2e | One chip-specific write composer; fixed raw observations and validity | Borrow small explicit code, not GPIO/environmental-sensor semantics | TI-derived mixed RO/RW/W1C/reset vectors in 3/10/18 |
| CLI | TMP1x2 shared Cli dispatch/output; SHT3x shared command processor | bafc340 / f65f155 | One portable parser, grouped help, strict input and bounded console; thin framework frontends | Reject blocking stress commands and a second firmware scheduler/result store | Host-executed command/console/transcript suites in 20-22 |
| Host runners | LDC1614 hil_runner/cli_contract; INA228 run_i2c_hil; 24Cxx test_cli_transcript | c0ab4c1 / b28c5ea / 936aa85 | Fresh request/token/session correlation, bounded logs, explicit owned-result consumption | Flashed identity and unknown effects cannot be inferred from host checkout | Fake serial/clock and real CLI fixture in 28-31 |
| Examples/pins | TMP1x2 and 24Cxx examples/esp_idf/basic and Arduino examples | bafc340 / 936aa85 | Real Arduino S2/S3 and native-IDF links; application-selected pins, exact part/address/straps | Board values are example inputs, not portable driver defaults | Core has no platform includes; handle/address and SDKCONFIG isolation in 23-27 |
| Package/version | TMP1x2 and 24Cxx scripts/generate_version.py; TMP1x2 tools/check_package.py; 24Cxx tools/check_package_contents.py | bafc340 / 936aa85 | library.json version truth; checked generated Version.h; renamed extracted consumers | Consumers must not need the generator; preserve required shared example files | Generator fixtures and archive/consumer tests in 8/32 |
| Documentation | LDC1614 Doxyfile; INA228 Doxyfile; TMP1x2 tools/check_doxygen.py | c0ab4c1 / b28c5ea / bafc340 | Missing public comments must fail and public API must remain visible | TMP EXTRACT_ALL=YES alone cannot establish comment coverage | Missing-comment and hidden-API negative fixtures in 33/36 |
| Style/simplicity | INA228 formatting and LDC1614 source; TMP1x2 common code | b28c5ea / c0ab4c1 / bafc340 | Small direct helpers, fixed values, one transport path/composer/engine; useful Doxygen with behavior | Avoid inherited frameworks, compatibility aliases and private-helper boilerplate | Review each changed behavior; docs with code, not postponed |
| CI/registration | TMP1x2 and 24Cxx CMakeLists.txt and .github/workflows/ci.yml | bafc340 / 936aa85 | Register real suites, preserve Release assertions; focused local checks, remote checkpoint only prompt 34 | Source review is not a green build; user CI-frequency override remains | Nonzero tests and declared matrix in 7/33/34 |
| Maintenance/license | TMP1x2 and 24Cxx library.json/LICENSE; peer AGENTS | bafc340 / 936aa85 | Thymos Solution s.r.o / Jan Havelka metadata, MIT project code; preserve third-party notices | Use evidenced identity and existing TUSB32x remote; no copied vendor implementation | Metadata and license audit in 8/32/36 |
| Release/Git | TMP1x2 and LDC1614 AGENTS and generators | bafc340 / c0ab4c1 | One execution branch; commit/sync each complete prompt; no release bump per prompt | Coherent reviewable units; release publication remains separate | Progress and upstream equality at each handoff |

## Platformization boundary

FieldCore-node is a consumer example, not a dependency or a template to clone.
Its current owner includes routed targets, parked work and exact cancellation
identities. These belong above this library: an application adapter supplies the
selected route/transport, serializes ownership and translates terminal statuses.
A reused context pointer does not prove the same physical target; notify external
changes and obey evidence-forgetting rules before replacing an unresolved target.

Bare-metal code can supply terminal register transfers and monotonic time without
an RTOS. Arduino and IDF callbacks stay under examples, with their own bus and
resource lifetime. A FieldCore bridge must preserve its richer observations at
the application layer; it cannot invent equivalent completion/acceptance evidence
inside the simpler Status-only core API. Required hardware waits therefore use
the baseline fresh-time rule. No platform services or product health policy move
into the core to make one consumer convenient.

## Resolution and remaining evidence

No new framework or API aliases are needed. The current source audit confirms the
baseline with the explicit exclusions above. Use the baseline as the exact name,
state and field contract; these decisions record why it was selected.

Chip-specific capabilities, masks, timing and unresolved TI wording are owned by
prompts 2-4. Sibling source cannot settle them. SDK pins and actual aggregate
timeout bounds must be proved in their adapter prompts. Native IDF is not located
in this shell; this does not justify substituting Arduino-as-component. Physical
fixtures and all future build/test claims remain separate from prompt 1 completion.

## Prompt 2: family and evidence decisions

Completed 2026-10-23. Exact source revisions/pages, all named variants and electrical
conditions are in [variant matrix](variant-matrix.md); implementation/test work is
tracked in [feature coverage](feature-coverage.md). The focused
[peer/public re-audit](audit/execution-2-peers.md) records current declarations,
helpers, tests, licenses and package rules. These are future implementation rules.

| Concern | Peer file/symbol or TI source | Revision | Selected rule | Reason | Check |
| --- | --- | --- | --- | --- | --- |
| Explicit capabilities | TMP1x2 Config.h modelAddressMin/hasAlertOutput; LDC1614 Config.h validChannelMask; B/L/A/V exact pin/ID tables | bafc340 / c0ab4c1; SLLSEN9F/SLLSEP2D/SLLSEQ8D/SLLSEO5D | Six explicit 320 variants, separately gated 322I, no 321 GPIO target; LAI primary | Shared identity cannot establish exact BOM, polarity or capabilities | Variant/address negative cases and independent eight-byte vectors in 9/18 |
| Revision policy | TMP1x2 variant tests, LDC1614 shared DEVICE_ID_VALUE; L/A Table 12, V Table 6-11 p20 | bafc340 / c0ab4c1; L/A/V above | Exact 01/02 only, no base A0; unexpected revision retained with CHIP_ID_MISMATCH, no write/trust promotion | Public 06 assertion and permissive Linux logging do not establish silicon support | Unexpected/reversed/partial identity and write-gate tests in 9/12/18 |
| Provenance/package | 24Cxx manifest/check_reference_archive.py/check_package_contents.py; TMP1x2 library.json; original TI notices | 936aa85 / bafc340; eight exact PDF records | Retain old bytes/metadata/terms; exclude reference payload from production and packages | Reference research is not MIT source; ordinary consumers need no research tools | Eight PDF hashes/terms verified now; extracted package guard in 32 |
| LI/HI selection caveat | Current TI LI/HI product pages; L mode definitions | Pages checked 2026-10-23; SLLSEP2D | Retain documented compatibility, expose TI DFP/DRP discouragement; choose LAI examples | Product recommendation differs from existence of register capabilities | Capability/help/docs check in 22/35; no LI/HI DFP/DRP qualification claim |
| Board ownership and unresolved claims | B/L/A/V exact electrical/pin tables versus N/G summaries | Exact revisions in variant guide | No core pin/power policy; no universal disputed startup recipe; exact 5.0 V extended 320 ceiling; narrow 322 write gates | Independent read/identity work need not wait for unrelated silicon ambiguities | Named gates/source requirements in variant guide; physical prerequisites in 23–31 |
| 322 VCONN supply | V §5.3 p4 and §6.3.8 p13 | SLLSEO5D; rechecked 2026-10-23 | Record the board-owned 5 V VDD prerequisite and 4.75–5.5 V VCONN condition | Device operation at 4.5–5.5 V alone does not qualify active-cable power | Supply/output qualification in the prompt 30 VCONN fixture; no core rail control |
| 322 mixed-register fault boundary | V Table 6-9 pp18–19 | SLLSEO5D; rechecked 2026-10-23 | Gate every 09 write until safe fault handling/composition is proved; include ACK/service, duty, UFP-accessory and raw writes | All share byte09 with fault bit3; knowing the intended RW field does not settle the other written bits | Future rejection/composition tests in 3/10/13/16/18/22; unchanged proved 09 settings do not block independent 0A work |
| Maintainability | TMP1x2 explicit Model helpers and LDC1614 explicit DeviceVariant/channel checks | bafc340 / c0ab4c1 | Small descriptor only for identical verified behavior; one existing operation path; no discovery/override framework | Family differences fit typed values and direct checks | API/source review in 5/9/18; no driver added in this prompt |

## Prompt 3: register and vector decisions

Completed 2026-10-23. The [register guide](register-map.md) owns the exact
variant tables and [vectors](register-vectors.md) own independent literal
expectations. [Focused peer evidence](audit/execution-3-peers.md) records actual
declarations, implementations, assertions and registration at the revisions below.

| Concern | Peer file/symbol or TI source | Revision | Selected rule | Reason | Check |
| --- | --- | --- | --- | --- | --- |
| Access and raw guards | PCA9555 CommandTable.h, writeRegisters/_writeRegs, raw-configuration rejection tests; B/L/A/V maps | 498178e; exact TI revisions in guide | Named byte registers, variant allowlists and procedure guards; no PCA pair wrapping/pointer parking | Generic byte transport cannot replace chip-specific access rules | D09/N01–N05/N19; future protocol/CLI suites |
| Independent masks/vectors | LDC1614 configurationComparisonMask, configWriteAt and literal replay tests; FakeLdc1614Device::applyWrite | c0ab4c1 | Separate persistent/protected/live/command/compare masks and fixed TI-derived expected bytes | A fake importing production constants or accepting arbitrary writes is not an independent silicon oracle | Eight mask partitions and 21 composition vectors checked now; future behavioral fake in9/18 |
| Semantic validation | ADS1115 validateDeviceProfile/decodePgaBits and invalid-setter tests; B/L/A/V encoding tables | 381236e | Validate before masking, explicit enum mappings; reject reserved codes and unsupported variant/time pairs | ADC alias normalization and 16-bit framing do not apply to this device | Complete field tables and reserved/variant cases D10–D35/N06–N10; future codec suite |
| Base defaults and protected bits | B Table 7-7 pp17–19 versus L/A Tables 8–11 pp17–20 | SLLSEN9F/SLLSEP2D/SLLSEQ8D | Base defaults carry validity masks only; preserve45 internal RW bits separately from reserved R | Base has no full reset column or numeric internal default; preservation arithmetic is not abnormal-image qualification | Visually checked tables; D36/C17–C20/N18; no new recovery procedure |
| V mode equality and gates | V Tables 6-8–6-10 pp17–20; variant-matrix V_ADVERTISEMENT/V_FAULT_CLEAR/V_RETURN_DRP | SLLSEO5D | DRP00/11 equal fieldwise; preserve unchanged raw mode; V08/09 writes stay gated, including raw | Equivalent mode cannot hide preference mismatch or authorize uncertain access | D32–D34/C14/N11–N15; no source resolution found |
| Scope and simplicity | Baseline single composer; focused peer source comparison | Current prompt3 audit | Two maintained guides with literal tables; no production stubs, generic register framework or new runtime API | Specification and arithmetic can be complete before transport/engine implementation | Full prompt review; later behavioral checks explicitly NOT RUN |

## Prompt 4: transport, procedure and timing decisions

Completed 2026-10-23. [Device behavior](device-behavior.md) is the authoritative
procedure guide, with exact source revisions/pages, independent vectors and
sequence-derived ceilings. [Focused peer evidence](audit/execution-4-peers.md)
records current code, tests, registration and deliberately rejected differences.

| Concern | Peer file/symbol or TI source | Revision | Selected rule | Reason | Check |
| --- | --- | --- | --- | --- | --- |
| Read framing and lifetime | ADS ownerWriteRead; LDC _readRegister; TI programming chapters | 381236e / c0ab4c1; B F/L D/A D/V D | Explicit one-byte pointer, STOP then read, one lock and aggregate callback timeout | TI documents STOP; combined public transfers do not mandate repeated START or prove bounded SDK calls | P27–P28; physical/backend checks due 9/23/25 |
| Time and uncertain effects | LSM _stepResetBoot/_writeByte; ADS delayed-start/wrap tests; PCA/LDC failed-write fixtures | 513c564 / 381236e / 498178e / c0ab4c1 | Next fresh owner sample, separate 1 ms margin, uint64 half-range time, original failure retained | Reject stale pre-callback anchors, saturation, copied sensor delays and cleanup replay | T01–T12 and write-boundary fault matrix; future18–19 |
| Small bounded plans | LDC behavioral fake and ADS owner tests; TI maps | Current peers above | One identity/proof path, scalar controls, one-pass verification, fixed09/08/09 observation | No generic register mirror, polling-until-clear or duplicated engine | All operation callback/TX/RX ceilings derived and checked |
| Mode/final termination | B §7.4.1–2 p14; L/A §7.3.2 p14; V §6.3.2.1–2 pp10–11 | Exact PDFs above | Base detached precheck before writes plus owner race exclusion; distinct disable/mode/enable writes; L/A6-tick wait; preserve requested disabled end state | TI sequences differ;45 cannot replace0A and cancellation cannot restore a port | P01–P12; no added V5 ms or hidden DRP reset |
| Internal45 qualification | B Table 7-7 p19; L/A Table 11 p20; PCA guarded raw access | B F/L D/A D;498178e | Ordinary 45 changes require known internal 03=00; B zero is conservative library policy, not numeric default | Preserving an abnormal image does not qualify it; no guessed repair of TI-internal bits | P29; composition vectors with nonzero internal bits remain arithmetic only |
| Recovery and history | B §7.4.3 p14; LSM/PCA retained effects | B F;513c564 /498178e | Four callbacks,04/wait31/00/read; reject a different pending transition, permit explicit restart of own uncertain recovery | A final zero or newer reset result cannot prove or erase historical sequence evidence | P13–P16/P30; sixTX/twoRX; no intermediate snapshot |
| Reset proof | B/L/A reset tables, V §6.3.2 p9 and Table 6-10 p20 | B F/L D/A D/V D | One command,96-tick wait,F; record actual controls; V DRP readback is conservative consistency policy; completed settling is separate from acceptance | No all-register default assumption or automatic desired reapply; own reset success can coexist with older recovery UNKNOWN | P17–P20; pending-reset discovery never adds a secondF |
| IRQ evidence | B Table 7-6 p16/Table 7-7 p18; L/A Table 9 p18; V Table 6-9 p19; TI-hosted error paths | Exact PDFs; Linux 97becb1 | ACK actual controls, service one complete observation/ACK/recheck, preserve data on later failure | Coalescing prevents lossless events; no invented5 ms ACK wait; V fault handling remains unresolved | P21–P26; ACK2/service 5 callbacks; original error and pending evidence |
| Source and scope | Four fresh official PDFs; pinned Linux/regmap helpers; peer reports | Verified 2026-10-23 | No new gate resolved, no driver/adapter success claim; future Doxygen carries exact source and policy distinctions | Source review and arithmetic are not physical qualification | Hashes, visual tables, local links and scoped diff; validation records limits |

## Prompt 5: portable values and callable boundary

Completed 2026-10-23. [Current peer evidence](audit/execution-5-peers.md) records
full revisions and actual declaration/source/test/runner/registration inspection.
The [API inventory](api-contracts.md) and [architecture](architecture.md) separate
implemented passive values from later callable work. No settled silicon rule changed.

| Concern | Peer file/symbol | Revision | Selected rule | Reason | Check |
| --- | --- | --- | --- | --- | --- |
| Status | LDC/LSM Status.h; TMP Status.h::errorName/operator bool | 1db38b6 / e262bf2 / bafc340 | Exact 23-code baseline, full int32 detail, static text, explicit bool and header constexpr names | No aliases, callback/owner timeout confusion or missing constexpr definition | All names/order, invalid codes, signed extremes, conversion and two linked translation units |
| Config versus settings | LSM Config.h; ADS Config.h::DriverConfig/DeviceProfile; INA Config.h | e262bf2 / 235af3e / 40838b5 | Borrowed callbacks and exact variant/address in DriverConfig; silicon settings only in DeviceProfile | No host calibration, bus ownership or power authorization in a candidate | Isolated headers, callback signatures, default/validity checks |
| Evidence storage | ADS AppliedProfileSnapshot; INA AppliedConfigState/JobResult; LSM diagnostics | 235af3e / 40838b5 / e262bf2 | Fixed desired/observed/verified/raw values; diagnostic identity/IRQ plus independent recovery/mode/reset uncertainty | Later reset must not hide earlier recovery; result consumption must not erase needed facts | Default invalidity and independent copied values now; transition behavior in6/11-17 |
| IRQ attribution | TMP exact-token result tests; LSM copied result/progress; completed device guide | bafc340 / e262bf2; prompt4 | Result IRQ is operation-local; diagnostics retain historical IRQ work, never current latch guarantee | An early failed service must not inherit a previous ACK success | Header/API review now; failed-service regression required in16 |
| Public scope | LSM test_compile_contracts.cpp; TMP core_no_arduino consumer | e262bf2 / bafc340 | Four complete value headers; future noncopyable facade only in compile-only fixture | No stubs or unimplemented link promises | Strict header compilation, fixture syntax and linked real Status consumer |
| Pure decode | Existing TI field tables and public-value vocabulary | B F / L D / A D / V D | One planned namespace decodeRegister; fixed RegisterDecode stores field codes with validity, not a single-byte port snapshot | Shared CLI decoding without guessed context or private dependencies | Exact planned signature and rejection order; implementation/vectors in10 |
| Documentation/style | LDC Doxyfile; TMP public documentation checks | 1db38b6 / bafc340 | Complete Doxygen now, strict warnings and actual XML visibility; no new documentation framework | Undocumented/hidden values cannot pass on generated output alone | Doxygen1.13.2 warnings-as-errors; all17 public structs visible |

No ABI size/serialization promise is inferred from standard layout and trivial
copyability. Core values use fixed storage; architecture-specific size quotas are
not copied from a sensor library. There is no new scheduler, allocation layer,
event bus or general register container. Validators remain prompt7; detailed
state/admission tables remain prompt6; hardware qualification remains separate.

## Prompt 6: owner, time and retained evidence

Completed 2026-10-25. [Current peer evidence](audit/execution-6-peers.md) records
six clean local revisions, actual implementations/tests/examples/runners and
registration. The [API contract](api-contracts.md) completes every method's
admission/output, state, timing, publication and lifetime rules. No silicon
sequence, callback ceiling, register gate or future callable name changed.

| Concern | Peer file/symbol | Revision | Selected rule | Reason | Check |
| --- | --- | --- | --- | --- | --- |
| Slot/tokens | TMP Operations.cpp::admit/takeResult; LSM _checkStart/takeResult | bafc340 / e262bf2 | Exact instance uint64 tokens, BUSY versus RESULT_PENDING, immutable terminal copy, delivery OK even for failure | Delayed owner work cannot advance or consume a replacement; no peer FIFO or failed-output mutation | Method/token rows; O01–O05 future runtime vectors; planned-signature compilation |
| Time | LDC poll; LSM reset wait; ADS delayed/ambiguous write tests | 1db38b6 / e262bf2 / 235af3e | Fresh post-callback owner turn, half-range uint64 time, zero-budget local progress, immediate frozen callback failure, six bounded local stages | Reject stale-time callback batching, direct comparisons and invented completion timestamps | 60 static arithmetic vectors; p4 exact fractional-time vectors; future O12–O16 |
| Desired and actual | ADS startApplyProfile; INA startReconcile; LSM profile tests | 235af3e / 40838b5 / e262bf2 | Desired at admission only; no-op/pre-write failure preserves old proof; shared read-only F can establish KNOWN actual on desired mismatch | Reject corrective reconcile/replay and premature APPLYING; failed acquisition alone is not contradictory evidence | Configuration/publication table and future O07–O11 |
| Initialization readiness | TMP/ADS separate transport/identity lifecycle; TUSB base recovery prerequisite | bafc340 / 235af3e; prompt4 | Matching I plus complete raw P establishes identity readiness even when controls/effects leave CONFIGURATION_UNKNOWN | Explicitly refines baseline's earlier broad failed-initialize rule: otherwise end/rebind recovery deadlocks behind its identity gate | Baseline health/lifecycle corrected; future O09; no claim a failed/incomplete I/P initialized |
| Ordinary write uncertainty | INA unknown/dirty register evidence; LDC retained write provenance | 40838b5 / 1db38b6 | Add one passive configurationWriteUnresolved flag, separate from recovery/mode/reset history and latest result | Raw write then read-only result NONE must not hide different-target admission uncertainty; avoid inference from UNKNOWN or old effect | Public default/copy check; future O25; no extra state enum or engine |
| Raw effects and cause | LDC tracked diagnostics; LSM operation results | 1db38b6 / e262bf2 | Diagnostic effect may describe a later raw write; last operation fields remain managed history. Invalidation reason is local, never a counted transport error | Preserve provenance without fabricated operation records; callback time flags remain invalid with Status-only backend | Doxygen/row consistency; future O16/O17/O22 |
| Owner lifetime | FieldCore I2cOwnerTransport/I2cDeviceBinding/I2cLifetime; TMP exact-token ownership | a002aba / bafc340 | Borrow context until end, retain both callback pointers in association, serialized external owner and checked uint32 extension | No FieldCore dependency, implicit bus ownership or false physical identity inference | Compile-only two-device observation/apply/cancel examples; no linked facade |
| IRQ/health | TMP/LSM retained evidence; LDC/INA diagnostic-only offline policies | bafc340 / e262bf2 / 1db38b6 / 40838b5 | Complete S survives ACK/recheck failure; named validity separates sampled/accepted/rechecked facts; pure health ignores raw traffic | No event queue, retry policy, stale-time completion or trust repair from counters | Future O18–O24; strict passive-value/Doxygen checks |

The initialization refinement and ordinary-write flag are deliberate gap fixes
found while completing the contract, not evidence that peer conventions establish
TI behavior. Runtime transitions, actual adapter bounds and physical qualification
remain future checks. Owner examples and arithmetic stay outside delivered headers.

## Prompt 7: portable target and pure validators

Completed 2026-10-26. The [current peer audit](audit/execution-7-peers.md)
reopened all six requested libraries and their build/test/consumer inputs.

- Keep one explicit `TUSB32x` static CMake target with PUBLIC C++17/include
  requirements, private supported compiler warnings and tests OFF by default.
  Reject LDC1614's configure-time Python/version check for ordinary consumers.
  There is no version or framework component claim at this stage.
- Put both real validators in Config.h and the small existing-responsibility
  source unit src/TUSB32x.cpp. Remove their compile-only redeclarations. Preserve
  all passive types and leave unavailable facade/health/decoder methods planned.
- Invalid binding/profile enums return INVALID_CONFIG before any capability
  restriction. Valid but unavailable settings return UNSUPPORTED_FEATURE. Null
  context, positive uint32 timeout extremes and independent disable controls
  remain valid where documented; no sensor timeout cap or role policy is imported.
- Separate profile semantics from actual-state write gates. All legal readable
  TUSB322I advertisement/duty/accessory values pass pure validation; future apply
  compares proved actual fields and rejects only gated changes. Validation cannot
  prove device state, authorize return-to-DRP or close any existing silicon gate.
- Reuse the existing active passive checks as tusb32x_compile_contracts, add the
  exhaustive tusb32x_validators runtime suite, and compile independent headers and
  planned fixtures as objects only. A standalone multi-file consumer links/runs
  actual API with deliberately failing fake callbacks; it is not a driver stub
  or an extra passing behavioral test.

The lead rechecked the actual TI source bytes: four fresh official datasheets
match their archived revisions exactly, and all eight archived hashes remain
valid. Relevant address tables p15 and profile fields B pp17-19, L/A pp17-20,
V pp17-20 agree with the existing contract; no silicon rule or gate changed.
Debug/Release checks, a clean renamed tests-OFF consumer under unrelated Git
metadata with maintainer tools absent from PATH, and strict Doxygen visibility
passed. Sanitizers are unavailable in the installed MinGW distribution; no
firmware, package export, remote CI or hardware qualification is inferred.

## Prompt 8: stable development metadata

Completed 2026-10-26. The [fresh peer audit](audit/execution-8-peers.md) records
actual generators, guards, manifests, ownership, workflows and source revisions.

- Select `0.1.0-dev.1` as explicit development SemVer. library.json is the only
  version truth; generated Version.h and literal CMake/component/Doxygen fields
  are synchronized by an explicit Python command. CMake stores both the full
  SemVer and its numeric project-version core because project VERSION cannot
  represent prerelease/build suffixes. No release or per-prompt bump is implied.
- Publish only four useful stable constexpr values: uint32 major/minor/patch and
  the full static character array VERSION. Reject volatile Git/time macros,
  VERSION_CODE arithmetic and inherited compatibility aliases. Generator root
  comes from its own script path, with no Git or child-process discovery.
- Use strict ASCII SemVer, bounded uint32 components and documented exit codes:
  0 success, check drift 1, invalid inputs/templates/I/O 2. Preflight all managed
  templates before writes; preserve bytes/mtimes on no-op sync/set. Document
  possible partial updates after an actual write failure instead of claiming
  multi-file atomicity. One isolated fixture suite grows in later guard work.
- Share one explicit source list between ordinary CMake and conditional native
  IDF registration. Reject the peer's configure-time Python requirement. Python
  3.10+ is found only for tests ON. IDF >=5.3.0 is the peer-backed admission floor,
  not a firmware-support claim; no speculative upper cap, Arduino, peripheral or
  consumer-product dependency is added. Revisit SDK evidence with real adapters.
- Keep normal packages to reviewed consumer files. PlatformIO uses an allowlist;
  IDF excludes all then re-includes the same set because its include semantics
  differ. Reference/vendor artifacts, audits, prompts, tests/tools/scripts and
  build output stay out. Actual PlatformIO archive selection and plain extracted
  consumption are tested now; full hardened distribution validation remains 32.
- Reuse the existing managed Core 6.1.19 through pio.cmd and pin native 1.2.1 for
  the real current host example. No generator build hook or firmware stub is
  introduced. Launcher tests compare real CRT argument bytes/status against
  direct invocation; missing installation fails explicitly. Formatting keeps
  current include order and adds no framework-first preference.
- Preserve evidenced Thymos Solution s.r.o / Jan Havelka / MIT metadata and the
  current @janhavelka owner. Vendor terms and archived bytes remain untouched.
  Doxyfile has public inputs, fatal missing-documentation warnings and visible
  generated values; the complete omission checker remains prompt 33.

### Prompt 8 independent audit corrections

- Count managed declarations separately from canonical replacement syntax.
  Alternate quoting, trailing comments and CMake command case/layout must not
  hide a duplicate version. Keep the maintained declarations canonical and fail
  ambiguity with exit 2 before writes; a general build-language parser is not
  needed for these explicit fields.
- Clear an inherited ERRORLEVEL environment variable inside the launcher's
  existing setlocal scope before running Core. Otherwise it shadows CMD's dynamic
  status and can report success for a failed child. Keep faithful `%*` forwarding
  and the explicit captured exit. Register Windows-only temporary CMD regression
  fixtures alongside the existing version suite; require no compiler or Core.
- Retain the explicit package content set. Use source-host URLs for intentionally
  excluded evidence and relative links between shipped guides, so archive
  navigation works without shipping vendor or audit machinery.

## Prompt 9: private terminal transport and identity

- Keep one private ProtocolContext borrowing existing owner config/stats/
  diagnostics and one invocation boundary. No public facade/raw methods,
  ownership manager, synchronous acquisition engine or framework transport is
  introduced. Pure signature/revision comparisons consume complete observations;
  the later owner retains raw validity and authorizes lifecycle actions.
- A fixed seven-variant capability table gates revision/RdRp presence and exact
  revision values. Range reads stay wholly in 00..0A (up to 11 bytes) or one
  present 45/A0. Writes receive precomposed bytes and enforce access, including
  the complete TUSB322I 08/09 gates. Safe composition and sequence prerequisites
  remain mandatory caller work in prompts 10 onward.
- All invoked failed writes retain POSSIBLE acceptance, including NACK_ADDR and
  I2C_BUSY. Current Status callbacks have no independent phase/byte channel, so
  the transport cannot prove that an attempted data-changing write had no effect.
  CALLBACK_SUCCEEDED describes only a complete callback, never final verified
  HardwareEffect::CONFIRMED. Read pointer selection is not a data-changing write.
- Count attempts before invoking, normalize afterward, and update saturating
  all-source totals exactly once. Raw callbacks do not touch managed history.
  Keep legal callback detail/text; normalize every owner/progress/invalid code
  to I2C_ERROR while preserving full detail. Retain last-failure evidence after
  success and invalidate only affected callback-time flags, leaving numeric
  history untouched. Transport never promotes initialized or configuration trust.
- Clip the uint64 available budget before uint32 narrowing and reject zero
  locally. Fixed scratch RX is published only after complete normalized OK;
  adapters must detect short transfers before returning OK. TI's selected
  STOP-separated pointer/read framing needs application-wide serialization and
  one proved aggregate backend bound, not renewed per-phase timeouts.
- Use TI-derived scripted expectations, independent of production codecs, for
  exact frames, range/variant rejection, all status bytes, partial-read failures,
  before/after-error write effects, saturation and exact identity/revision.
  The inspected Renode pointer/multi-byte behavior and public revision 06 are
  counterexamples, not accepted device behavior.
