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
| Maintainability | TMP1x2 explicit Model helpers and LDC1614 explicit DeviceVariant/channel checks | bafc340 / c0ab4c1 | Small descriptor only for identical verified behavior; one existing operation path; no discovery/override framework | Family differences fit typed values and direct checks | API/source review in 5/9/18; no driver added in this prompt |
