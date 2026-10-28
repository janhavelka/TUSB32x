# Engineering decisions

## Prompt 25: native IDF adapter (2026-10-28)

Use IDF5.5.5 from pinned pioarduino55.03.311, verified against the exact official
driver bytes. Root component registration remains portable. The example adapter
lives directly under examples/esp_idf with the same small file naming pattern as
the Arduino adapter; a speculative common subdirectory is unnecessary until the
native application exists. Native SDK fixture main stays under test/idf_sdk.

The [peer audit](audit/execution-25-peers.md) supports borrowed ownership,
relocatable component discovery and coarse-error handling. Reject peer-created
bus ownership/recovery and direct whole-timeout forwarding. Use a shared bus
context and per-target borrowed context instead of a registry. Exact copied
creation configuration is application evidence; the opaque SDK API cannot prove
the handle/address/bus association for us.

The [source-derived bound](idf-transport.md#aggregate-timeout-and-buffer-proof)
reserves every possible synchronous wait/cleanup and qualified CPU overhead.
STOP-separated pointer/read preserves TI framing. Queue depth must be zero,
event callbacks absent, ACK checking enabled, logs/PM disabled, and the owner
pinned to the bus-creating core. Failed receive retires every controller user
because SDK stale read state can survive the error. No adapter recovery or
framework behavior enters core. These gates and SDK error mapping are checked
by actual-adapter host fixtures; S2/S3 images independently prove native linkage.

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

## Prompt 10: pure codecs and qualified byte composition

- Keep the public supplied-byte decoder in CommandTable.h with its fixed
  RegisterDecode result and real linkable definition. Implement pure codecs and
  one write composer in a small current-responsibility unit, sharing the existing
  closed variant table with transport. No public encoder, raw setter, registry,
  generic byte RMW API or second owner engine is introduced.
- Keep raw field meaning separate from paired applicability. Prompt 10's detached
  NOT_APPLICABLE wording is clarified to refer to complete port decoding in prompt 15;
  the frozen public decoder continues to report raw CC1/CC2 and current codes.
  Reserved codes clear their individual validity while retaining the full byte.
- Typed encoding validates complete candidates, but represents readable 322I
  fields without granting write access. Actual-field validity is checked from
  decode masks, not by validating a partial profile's default placeholders.
  Fieldwise comparison preserves equivalent 322I DRP00/11 and still compares
  preference, debounce and disable controls. Unrelated updates retain raw00.
- Compose only whole named requested fields from proved actual controls.
  Dedicated intent/stage checks prevent ordinary mode changes and require owner
  detached/disable/mode/wait/recovery proofs. Those flags do not implement a job
  or verify their own provenance; the later serialized engine supplies them.
  ACK/reset use safe actual images even when overall configuration is UNKNOWN,
  ignoring staged desired values and retaining older obligations. Ordinary
  settings require resolved effects; recovery rejects different pending mode/
  reset but may resolve its own old recovery history with configuration UNKNOWN.
- Keep arithmetic separate from permission: DE->56 excludes reset in field
  encoding, but qualified composition rejects observed reset1. Nonzero 45 internal
  bits can be preserved arithmetically but remain unqualified for ordinary or
  recovery writes. Preserve existing conservative B internal-zero policy.
  All 322I 08/09 writes, missing 45 and real non-reset return-to-DRP remain gated.
  No VCONN fault-clear meaning is inferred from IRQ or its RCU label.
- Derive test tables and expected bytes from current verified archived TI tables,
  not production helpers or a fake silicon model. Reopened pinned Linux reset
  mask08/value1 and regmap val&mask semantics confirm the ineffective call;
  the Rust Try.SRC04 constant remains reserved. Tests assert real reset08,
  Try.SRC06, ACK preservation, strict local rejection and zero pure callbacks.


## Prompt 11 implementation decisions (2026-10-26)

- Keep lifecycle and the first cooperative engine together in
  `src/TUSB32xOperations.cpp`; pure validators remain in `TUSB32x.cpp`. Reuse the
  existing protocol/decoder instead of another transport or register image.
  `TUSB32xOwnerTime.h` holds bounded token arithmetic for direct exhaustion
  verification without exposing a public seed hook.
- One acyclic poll pass handles time validation, pending completion and at most
  one terminal callback. Larger budgets remain ceilings; zero budget permits
  only local work. Matching identity segments publish after fresh time and the
  deadline check; complete contradictory segments invalidate immediately.
- Derive termination from applicable disable controls and transient evidence,
  independently of unrelated reserved advertisement/preference values. Full
  configuration proof still requires every applicable profile field.
- Register the real owner suite; move future-only signatures to the explicitly
  named compile-only planned fixture. The production facade declares only
  implemented methods. Update all CMake/PlatformIO/IDF/package source lists in
  the same change so extracted consumers receive the real implementation.

## Prompt 12: read-only observation and reconciliation (2026-10-26)

- Extend the existing engine with two real methods and a shared admission/read
  path. Keep P (persistent controls) separate from F (identity then P). Reuse
  existing codecs and terminal transport; no generic verification framework,
  alternate engine, public desired setter or write/cleanup/replay is needed.
- Complete P publishes raw bytes, independent typed validity and the time before
  its first read. It does not initialize, adopt desired or advance verified
  generation. Immediate contrary evidence invalidates current proof, while a
  later failure/cancellation preserves the previous complete observation.
  Termination can be independently known without configuration trust, so a
  changed applicable disable bit invalidates that proof too.
- Reconciliation needs binding only. Complete matching identity plus raw P can
  restore initialized usability; every applicable field and required effect
  proof are needed for KNOWN actual settings. Advance the saturating verified
  generation only there. Compare retained desired afterward without replacing
  it; prompt 13 supplies desired through apply and owns that public-path test.
- Retained pending reset uses a conservative 96-tick wait from the first fresh
  poll, then one F. A reset newly discovered during F stays pending; do not
  append another pass. Complete waited F with reset clear establishes reset
  settling independently of unrelated reserved settings. Keep acceptance and
  historical recovery evidence separate. Only interrupted L/A mode requires
  the corresponding six-tick wait; base/322 receive no invented mode delay.
- Matching reconciliation identity does not erase a previous complete revision
  when its subsequent revision read fails. Complete identity contradiction
  still invalidates immediately. No read-only operation repairs port freshness
  or changes an already copied terminal result.
- Fresh peers confirm separate desired/observed/applied evidence and bounded
  result handling. Adopt LSM's read-only verification pattern but reject its
  desired-dependent admission/publication; reject INA3221's profile-replaying
  reconciliation. Exact revisions, source/test symbols and other differences
  are in [execution-12-peers](audit/execution-12-peers.md).

## Prompt 13: explicit profile application and verification (2026-10-26)

- Extend the same engine with a fixed plan of at most six composed writes and
  shared F proof. Validate the whole candidate and every changed-field gate
  before adopting desired or consuming a token. Preserve old actual proof on
  admission/pre-write cancellation; clear the previous desired-mismatch flag
  when accepting new desired input. No public desired setter or second engine.
- Actual controls determine preserved bits. Base mode change runs one detached
  precheck before any write; current-only changes remain allowed attached.
  L/A uses distinct disable/mode/enable writes and six elapsed ticks from the
  fresh post-mode callback sample. Holding final termination disabled still
  waits before F. TUSB322I uses its own sequence without that added wait;
  advertisement/09/direct-DRP changes remain gated, unchanged values do not.
- Record possible effect before each write and keep the first backend error.
  Successful callbacks mean PARTIAL, matching complete F means CONFIRMED, and
  no-op F remains NONE. Failed callbacks are INDETERMINATE regardless of error
  name. All mode writes retain mode-proof uncertainty, including base's single
  write. Reconcile/verify require full F before clearing it; base/322 add no
  mode wait. Diagnostic effect/kind/status remain terminal history while live
  configuration/mode flags expose active uncertainty.
- Verify shares reconciliation's prior-effect wait and full identity/control
  proof. Complete actual mismatch is KNOWN actual with CONFIGURATION_MISMATCH;
  desired stays unchanged and the old job is never recertified. A new explicit
  apply can progress from that actual image. Historical reset acceptance alone
  does not forbid apply after current reset settling and configuration are proved.
- Exact TI references and current peer revisions/accepted/rejected rules are in
  [execution-13-peers](audit/execution-13-peers.md). Public failure/cancellation,
  independent fractional-clock and literal-vector tests cover these decisions.
  No retry, cleanup, framework dependency, new package source or hardware
  qualification is introduced.

## Prompt 14: explicit reset and original-device recovery (2026-10-26)

- Add two real start methods to the existing engine and transport. Reset obtains
  fresh actual 0A controls, uses the existing composer once, anchors 96 elapsed
  ticks after callback return and runs one complete F. No desired-profile replay
  or exact invented reset-byte comparison. B/L/A may resample PORT; 322I requires
  semantic DRP. A contradictory independently valid mode prevents reset completion
  even if another field is reserved. Incomplete proof or asserted reset returns
  CONFIGURATION_UNKNOWN; otherwise contradictory 322I role returns mismatch.
- Base-only recovery requires current identity and external VDD readiness. Reject
  other variants before traffic and initial internal45 bits before writing. Use
  TI's literal04, fresh 31-tick wait, literal00 and final low-three-bit proof.
  Four callbacks and 6TX/2RX bytes; no transient port observation or cleanup.
- One shared write path records possible effects before transport. Failed writes
  retain their original error/detail and INDETERMINATE effect; no retry. Reset
  acceptance uncertainty remains historical through later successful proof/reset.
  Successful recovery clears only its sequence obligation: termination can become
  ENABLED while full configuration remains UNKNOWN pending F. Final04 establishes
  DISABLED without sequence success; internal-bit anomalies remain UNKNOWN.
- A completed new reset may return OK/CONFIRMED while older recovery history keeps
  configuration UNKNOWN. Clear controls alone never prove the historical recovery
  recipe. Only an explicit new recovery request may repeat it. Pending other
  reset/mode work rejects recovery; own recovery history alone does not.
- Current peer/TI sources, revisions and rejected conventions are recorded in
  [execution-14-peers](audit/execution-14-peers.md). Fixed public tests and an
  independent exhaustive reset-byte probe cover these policies. Host evidence
  does not qualify board readiness, physical wait accuracy or autonomous VCONN.

## Prompt 15: complete port observations (2026-10-26)

- Reuse one engine/transport and pure register decoder. Three small private phases
  stage09/08/09; one private `decodePort` applies the documented context matrix.
  Only complete fresh publication replaces cached data. No retry-until-stable,
  IRQ ACK, reset, event bus, framework dependency or second state machine.
- Capture time and configuration provenance before the first read. Final09 owns
  attachment/direction/IRQ; E0 comparison qualifies cross-register fields only.
  Sequence saturates, counts publications and does not establish edge identity.
- Preserve raw evidence for reserved/contradictory contexts. Detached and accessory
  roles are NOT_APPLICABLE; accessory direction and extended UFP-debug current
  remain UNKNOWN under conservative library applicability policy. See the exact
  [matrix and TI references](register-map.md#complete-port-interpretation-implemented-prompt-15).
- Negative control evidence may invalidate current configuration during an
  incomplete refresh; it cannot replace or invalidate the historical port record.
  New publication records acquisition-time trust, not later trust. Other explicit
  invalidation/write/lifecycle rules remain intact. Refresh never repairs trust.
- [Current peers](audit/execution-15-peers.md) support separate trust/health and
  complete publication. Reject sensor completion timestamps, chip-side atomicity,
  guessed Linux detached roles and unqualified VCONN/orientation projections.
  Source review and host tests do not qualify electrical behavior.

## Prompt 16: explicit ACK and bounded service (2026-10-26)

- Add two fully implemented starts to the existing engine. Both require current
  identity, permit UNKNOWN configuration with fresh safe09 controls, and reject
  TUSB322I before traffic under the existing V_FAULT_CLEAR write gate. No GPIO,
  dispatcher, queue, retry or implicit service operation is added.
- ACK-only reads09 then composes/writes W1C once: two callbacks,3TX/1RX. Service
  reuses complete09/08/09 publication then ACK and one09 recheck: five callbacks,
  6TX/4RX. The composer preserves actual duty/accessory controls; desired staging
  and RO status are never write images. No inter-clear wait is invented.
- Service IRQ-before evidence requires complete S; ACK-only requires fresh09.
  Both publish after the fresh-time/deadline check. Failed partial observation
  preserves prior diagnostic IRQ history. New operation results start empty.
  Attempt/acceptance update at the write callback; recheck publication needs the
  next fresh owner turn. One callback remains the ceiling even with larger budgets.
- A pure ACK does not invalidate port/configuration or create a mode/reset/recovery
  obligation. Possible writes have INDETERMINATE effect until callback acceptance;
  accepted ACK is PARTIAL until the requested operation completes, then CONFIRMED.
  These labels never prove the latch stayed clear. Failures preserve first error,
  accepted effects and the complete pre-ACK port without retry or cleanup.
- Contrary control reads still revoke current configuration trust immediately;
  ACK/service preserve historical port fields/flag as refresh does. Recheck is
  separate IRQ evidence, not a replacement port publication or proof repair.
  Coalescing/reassertion requires owner policy; pendingAfter=true can accompany OK.
- TI B Table7-6 p16 and Table7-7 p18, L/A Table9 p18 and V Table6-9 p19 were
  re-read. B/V CSR and L/A RU-change wording are distinct. V fault RCU alone
  establishes no clear transaction; the fault remains raw bit3, independent of10.
  Pinned Linux ignored ACK errors/raw echoes and Betrusted fixed11/unbounded retry
  are rejected after helper inspection. Current engineering references are in
  [execution-16-peers](audit/execution-16-peers.md). Hardware qualification is separate.

## Prompt 17 raw safety and passive views (2026-10-26)

- Keep the existing protocol invocation as the sole accounting source. All-source
  stats include raw Sync; managed evidence explicitly excludes it. No callback-time
  evidence exists in Status-only callbacks, so raw failures invalidate time validity
  rather than borrowing owner timestamps. Clearing source totals cannot erase
  managed health, last failure or retained hardware obligations.
- Raw writes require initialized KNOWN actual settings and relevant complete-proof
  images. Reuse the safe composer; never fetch controls inside a one-callback write,
  silently normalize the byte or replay staged desired values. Reset, role,
  termination and recovery changes require typed operations. Ordinary equal-value
  writes still revoke trust before invocation. Pure ACK preserves actual controls
  and records acceptance without manufacturing managed IRQ observations.
- Protected45 bits03 are excluded from user-profile equality. A raw/partial read
  can contradict their older complete-proof image while user settings remain KNOWN.
  One private write-image validity bit revokes only45 permission; both raw and typed
  apply check it. Complete F proof renews it only for documented internal-zero
  controls. No duplicate register mirror or global independent-write ban is needed.
- Historical ambiguous reset acceptance can coexist with settled COMPLETED/KNOWN
  actual proof. It remains visible but does not reject independent safe writes.
  Current pending-reset or unresolved write/sequence evidence still rejects them.
- Health is a pure projection of caller-copied managed evidence and a positive
  uint32 threshold. It owns no admission, clock, state repair or recovery. READY
  may coexist with UNKNOWN trust; OFFLINE permits legal explicit operations.
  Peer units and rejected synthetic mismatch failures are recorded in
  [execution-17-peers](audit/execution-17-peers.md).

## Prompt 18 protocol conformance (2026-10-27)

- Keep existing adversarial operation tests and map their real function names to
  requirements in [protocol conformance](protocol-conformance.md). Add only the
  missing public high-address paths and raw failed-write/ACK outcomes, plus the
  fixed fake's successful-short-receive rejection. The public address regression
  detects a wrong-address mutation that the earlier operations suites missed.
- Status-only callbacks have no received-length field. Adapter code must reject
  incomplete receives; the fake must reject contradictory scripts. This check
  must never be presented as the core detecting a dishonest successful callback.
- Keep TI facts separate from test assumptions. A modeled ACK clearing the latch
  does not qualify IRQ race timing; a 322I raw read without a clear write does
  not resolve the fault's consuming-read question. DE-to-56 remains arithmetic:
  actual DE has reset asserted and settings writes reject until proof is safe.
- The [fresh peer audit](audit/execution-18-peers.md) confirms fixed storage,
  explicit callback counts and original-error/effect retention. No new production
  API, engine, simulator, replay policy or silicon support is needed for this unit.

## Independent prompts 16-18 audit (2026-10-27)

TUSB322I DRP00/11 is semantic equivalence, not literal write-image equality.
A raw or incomplete observation can contradict the old mode bits without
invalidating the verified DRP setting. Retain one private mode-image validity
flag, revoke it on such an observation or normal invalidation, and renew it only
after complete proof. Raw 0A writes and typed 0A changes check it. No-op proof and
explicit reset with fresh precheck remain available. This follows the existing
protected45 evidence pattern without another register cache or public API.

Separate 322I failure/interruption tests are necessary because its DFP/UFP plan
has no L/A wait and no 45 proof read. The existing LAI matrix did not exercise
those branches. Keep literal scripts in the current fixture and reuse existing
error normalization tests instead of multiplying the entire suite by every part.

## Prompt 19: executable owner portability (2026-10-28)

The [current peer audit](audit/execution-19-peers.md) and
[coverage matrix](owner-portability.md) retain the single terminal-callback engine.
A small test-owned fixed scheduler proves two physical targets can progress while
one waits, fails or services a still-pending IRQ. Queue/result backpressure is an
external owner responsibility; no production scheduling abstraction is added.

Private `nextGeneration` now supplies both existing saturating increments so a
bounded boundary fixture tests the exact production arithmetic without public
seeds. Behavior is unchanged. The independent consumer imports built objects and
public headers only, inherits required ABI/sanitizer flags, and executes allocation
traps with checked negative sentinels. Its Python isolation coordinator remains a
maintainer-only dependency. Hardware and framework claims require separate evidence.

## Prompt 20 local CLI ownership and framing (2026-10-28)

- Keep the CLI example-only. One fixed dispatcher, staging value and console
  borrow core/platform callbacks; no core code or source list gains a CLI dependency.
  No setup command, automatic bind/init or device poll exists in this local stage.
- Validate complete syntax, candidate values, target pair and optional borrowed-handle
  constraints before mutation. Target edits require ended/idle/no result and cannot
  discard unresolved core effects. Accepted part changes stage common validated
  defaults; address changes preserve staging. Explicit forget uses the core method.
- Show core profiles with their retained association and exact observed validity;
  local target selection cannot add valid defaults to historical core records.
  Source variant is explicit. Staging validity does not grant capability-gated writes.
- Separate framing from presentation. Feed consumes at most 64 bytes and one command
  per turn, with sticky poison and CRLF coalescing. Whole-line processCommand uses
  the same one-command fence for already collected input. Output is 8192 bytes,
  maximum record storage512 including NUL, one nonblocking drain at most 256.
- Keep one finite help cursor retaining request/nonce; new local commands remain
  responsive. Help rows advertise only implemented routes. Static Python rows are
  checked by executing C++ help and every canonical/alias route, never by generation.
- Request IDs do not wrap. SESSION/LOSS are explicit uncorrelated schema exceptions;
  retain the exhaustion reason across congestion and retry only when space exists.
  Machine code/detail never depend on human text; injected status text is bounded
  and sanitized before presentation. Host-only friend access covers mathematical
  limits without public seed setters or production commands.

## Prompt 21 core-owned operation flow (2026-10-28)

- Dispatch calls each canonical core start API once, without traffic. Only accepted
  apply captures CLI staging as core desired. No initialization reset/replay exists.
- Automatic stepping defaults on for CLI-admitted tokens only. Explicit job step
  selects the sole poll budget for that turn, including zero; it may also explicitly
  advance a represented application operation. Never silently adopt foreign work.
  The application must not separately poll the same driver in that CLI owner turn.
- Keep only origin request/nonce/token and an announcement flag. Core owns the
  immutable retained result; output loss never consumes or retries its announcement.
  Unattributed externally admitted results report origin_request=0.
- Reserve five maximum whole records before take (ACK, optional human status,
  RESULT, RESULT_DETAIL and END), plus three for an unannounced original completion.
  Serialized dispatch then emits synchronously; congestion retains the core slot
  and loss evidence. No deferred destructive take or second result store exists.
  A reservation rejection is BUSY, not a fabricated lost record; count only actual
  failed response enqueues. A near-full queue can deliver the complete rejection.
- JOB code/detail describe a cached inspection or that explicit poll. A rejected
  nonterminal owner-time sample remains visible and leaves the operation active.
  A completed operation failure stays in RESULT; successful take/step delivery
  remains separate from that payload outcome and possible hardware effects.

## Prompt 22 diagnostic sources and parity (2026-10-28)

- Keep the eleven remaining routes in the existing dispatcher/output unit. Raw
  access uses core Sync methods and supplied-byte decoding uses the public decoder;
  no write mask, protocol, target scan or second renderer is introduced.
- Report raw callback invocation from the documented terminal Status domain:
  OK/I2C_* means one actual callback; local admission errors mean zero. This remains
  exact when source counters saturate. ATTEMPTED preserves possible failed-write
  effects rather than implying UNCHANGED; transport detail stays signed/full-width.
- Label XFER as all_callbacks and DRV/MANAGED as managed_callbacks. Reject any
  saturated exact assertion without summing counters. CLI xfer_reset requires no
  active work or retained result, narrower than the unrestricted core clear API.
  A valid clear changes neither managed health nor device trust/effects/history.
- Add CONFIG_DETAIL for acquisition and mismatch evidence. Separate decoder code
  validity from paired port applicability and cached IRQ history from input levels.
  Input reading is explicit, optional, nonblocking and never configures GPIO.
- Use one executable parity checker against actual help/commands/records and
  backend deltas, sharing its help comparison with the persistent transcript.
  Negative alias/required-field dependencies must fail. No source-token-only
  inventory or extra passing transcript registration replaces behavior checks.


## Arduino application integration, prompt 24

- Share one portable CLI queue for both command and APP diagnostics. A copied
  256-byte mailbox isolates SDK USB waits in a static worker; no SDK transmit
  call runs in the device owner. This is required by the pinned S2 TinyUSB mutex
  paths, not a general asynchronous framework.
- Application startup owns and consumes only its init/refresh tokens. It never
  depends on CLI result-response capacity. The primary hands off to the CLI;
  a secondary fixed owner shares the same controller Context and has a reserved
  address. IRQ signals are coalesced and retained through startup.
- Keep actual board settings in examples/arduino/BoardConfig.h; common owner and
  mailbox code stays portable. Default diagnostic-only firmware cannot invent a
  qualified timing bound or resolve TI's enable-polarity startup conflict.
- Retain static USB resources after local bus shutdown. Stop admission, preserve
  result/effect evidence, end both bindings and only then release exclusive Wire.

Evidence and current peer revisions: [execution audit](audit/execution-24-arduino-examples.md).

## Independent adapter/application audit, 2026-10-28

- Restrict both pinned SDK adapters to 100–400kHz and native IDF to the default
  clock source. This is a deliberately small audited SDK subset, not a new TI
  minimum. Reject configurations whose SDK timing arithmetic is unqualified;
  do not duplicate HAL timing calculations in the adapter.
- Wire application ownership requires an uninitialized controller before
  `begin`; its success result alone does not establish ownership. Check `end`
  and retain retired context/resources on failure without an automatic retry.
- Reserve active USB GPIOs from I2C/IRQ configuration so rejected setup retains
  its diagnostic console. Exercise actual application code with thin recording
  SDK boundaries; keep physical scheduling and linkage evidence separate.

See [audit findings](audit/review-23-25.md) and [adapter integration](idf-transport.md).

## Prompt 26 native application boundaries (2026-10-28)

- Reuse the shared CLI, startup owner and native adapter. IDF's core0-pinned main
  task is sufficient; no extra scheduler, input task or command/result store.
- Select exclusive UART0 TX43/RX44 on both S2/S3. Disable SDK stdio/secondary
  console/logging, use zero-wait reads and available FIFO output, and retain the
  unconsumed input suffix. UART mutex bounds depend on sole ownership, not merely
  a zero wait argument. Stream errors or event saturation fail input closed.
- Default to diagnostics until explicit board readiness, qualified positive CPU
  overhead and SDK cleanup acceptance. Handle/address metadata is application
  evidence; CLI edits cannot silently replace an opaque SDK device.
- End binding and inspect/report exact results/effects before owned teardown.
  Failed deletion preserves static resources and is never retried. Software queue
  empty is not wire completion: a finite zero-wait FIFO check precedes deletion.
- Isolate SDKCONFIG and the component-manager lock by absolute build directory,
  resolve the directory-derived component name and regenerate example identity
  every incremental build. Core consumers gain no Git/Python/framework dependency.

## Prompt 27 ordinary consumers and ownership (2026-10-28)

- Extend the existing host/basic two-TU consumer to run a deliberately narrow
  simulated LAI transport and synthetic owner clock. Do not package a test fake
  as production protocol or introduce a framework/product dependency.
- Register that actual main as a behavioral CTest: admission IN_PROGRESS,
  one-callback polling, exact take, payload/effect distinction and explicit end
  have always-active checks in Release. Delivery OK with mismatch is exercised.
- Reuse the existing generic scheduler fixture for missing stale-token,
  cancellation/invalidation/proof/freshness and teardown combinations. Keep the
  shared bus declared before driver storage and explicitly end while context is
  alive. Product queue/lifetime concepts are evidence, not a new chip wrapper.
- Preserve Status-only uncertainty and fresh post-callback owner time. FieldCore
  completion/effect fields and its millisecond-scaled nowUs hook cannot become
  stronger timestamp/acceptance claims through this boundary.

## Prompt 28 strict sessions and saved evidence (2026-10-28)

- Reuse typed schemas and move the existing single-request closure validator into
  the pure CLI contract module. Keep new-input ordering and one owned original
  operation in Session, not a growing request/token journal or second scheduler.
- Preserve the admission's host deadline through wait/query/marker traffic. Keep
  result delivery, operation payload, firmware owner timing and hardware effect
  separate; a failed payload becomes sticky failed proof even with good cleanup.
- Separate streamed artifacts into one helper with fixed tail, explicit cap loss,
  bounded report data and same-directory atomic file replacement. JSON is
  authoritative if summary replacement fails; every cap/save fault prevents PASS.
- Snapshot and restore local values only under verified identity and idle/result
  ownership, checking the returned values. Do not apply hardware, silently retarget,
  cancel foreign work or replay local changes after reboot/reconnect/framing loss.
- Optional pyserial3.5 uses closed-handle DTR/RTS configuration and finite I/O
  timeouts. Port open itself has no portable bound or no-reset guarantee; actual
  serial and hardware qualification remain separate from fake/compiled fixtures.

## Prompt 29: finite host observations, not another firmware engine

- Keep selected plans in one small observation module using the existing Session,
  parser, artifacts and ordinary CLI commands. Repeated CLI suites normalize to
  readonly/soak; no device-side bulk command or second result cache is introduced.
- Compare firmware's configured target with the requested pair. Bind an ended
  selected owner only after idle/result fences; never retarget, apply or reset
  to make an observation pass. Unknown identity cannot satisfy expected values.
- Use exact all-source callback deltas with auto off and unsaturated adapter-write
  evidence. Separately prove active cached-query progression in controlled native
  turns. Callback attempts do not identify physical transaction phases.
- Treat interval as a quiet gap after completed observation. Count, optional soak
  duration and one immutable whole-run deadline all bound work, including result
  delivery/local cleanup. Reject impossible minimum waits before opening.
- Require current invalidation/validity/coherence, advancing sequence and supported
  acquisition time, not changed attachment. Stop at saturated freshness/count
  evidence rather than invent precision. Preserve raw read effects and unresolved
  TUSB322I09 semantics; read-only is not a claim of electrical non-interference.
- A refused result-take is retained proof, not permission to retry or drop the
  slot. Stop with residual evidence. Restore only verified local values without
  hardware apply; lost sessions never replay restoration. Dry-run exits 2 because
  hardware was not run, even when its finite plan validation passes.
