# Final audit

## Core review — prompt 37, 2026-10-29

Reviewed `bdfcc144f292d4e1cdaecea58796aa208a0bec66` on
`promptset/tusb32x-implementation-20261023`. Entry index/worktree were clean;
fetch confirmed upstream equality and zero ahead/behind. Prompts 1–36 and their
recorded prerequisites were complete. Prompt 38 integration/distribution review
and final handoff remain outstanding; this is not whole-promptset completion.

The lead read current production and test paths alongside the baseline and
prompts 1–19 requirements. Separate fresh-context gpt-6-astra/high reviewers
examined silicon/protocol and owner/time/portability behavior. A gpt-6.1-sol/high
reviewer refreshed the Projects inventory and peer comparisons. Reviewers were
read-only; the lead verified evidence, ran fresh builds, edited records and owned
Git. No new executable core defect was reproduced. Existing regression cases
already exercise the challenged boundaries, so no duplicate tests or speculative
production changes were added.

### Findings and disposition

| Severity | File/symbol | Reproduction/evidence | Fix | Regression | Remaining limit |
| --- | --- | --- | --- | --- | --- |
| Low, documentation | docs/library-comparison.md, Current evidence | Fresh Projects/Git enumeration contradicts the claim that all 17 HEADs are unchanged and the old FieldCore SHA | Record current inventory and changed revisions below; retain dated prompt1 artifacts and label its tool observations historical | Rechecked all 17 revisions/branches/dirty states and exact changed-scope diffs; local links checked | No sibling remote synchronization claim |
| Low, documentation | docs/protocol-conformance.md, validation row | Public Config.h and TUSB32x.cpp expose validateDriverConfig; validateBinding does not exist | Correct the mapped public symbol; link this current review beside historical test evidence | Existing validator/runtime and public link/visibility checks pass | No callable API changed |
| Qualification gap, not a code defect | Board and SDK integration prerequisites | Host tests cannot establish electrical timing, physical attachment/current resampling or a measured CPU/preemption bound | Preserve explicit qualification requirements and closed silicon gates | Software checks below; physical evidence NOT RUN | Prompt38 reviews integration/distribution; physical work needs an identified authorized fixture |

### Requirement and executable evidence

All names below are current production symbols or named cases in the existing
test files; the detailed [protocol matrix](protocol-conformance.md) remains the
TI/software requirement-to-test mapping. Fresh validation includes these suites.

| Scope from prompts 1–19 | Production review | Challenged evidence and result |
| --- | --- | --- |
| Portable structure, values, errors and builds (1,5,7,8) | Five public headers; TUSB32x.cpp; strict core target and isolated consumer | Validators, constexpr/multiple-TU contracts, independent includes, static Status strings/full detail, allocation sentinels and renamed public consumer PASS. No framework/heap or generator dependency introduced. |
| Variant/identity/capability gates (2,3,9,10) | findVariant, validateDriverConfig, checkIdentity/checkRevision, validateRange and codecs | All seven configured I2C variants/both addresses; ascending eight-byte signatures; reversed/partial IDs; base A0 and V45 rejection; unknown revision06 and enum errors PASS. Configured model/polarity is never inferred from ID. |
| Mixed access and silicon encodings (3,4,10,18) | composeWrite, encodeProfileRegister, comparePersistentSettings, validateRawByte | Literal reset08, Try.SRC06 versus reserved04, ACK B5→15/B4→14/06→16, reset-isolated DE→56 and protected45 vectors PASS. Desired staging does not leak into ACK. |
| Roles, apply and reconciliation (12,13,18) | planProfile, beginApplyProof, publishConfiguration | Actual changes only; base attached current-only update versus mode rejection; extended disable/mode/6-tick wait/enable; requested-disabled final states; no-op writes NONE; mismatchAndExplicitRecovery establishes KNOWN actual despite desired mismatch and permits new explicit apply. |
| Reset/recovery history (4,14,18) | reset/recovery phases, unresolved flags and shared proof | everyCallbackFault, everyInterruptionBoundary, recoveryHistoryAndExplicitRestart and unresolvedRecoverySurvivesConsumption PASS. Base recovery remains explicit read/04/31-tick wait/00/read; clear final bits cannot erase interrupted sequence. 322 reset proves semantic DRP, not a full POR image. |
| Terminal transport (6,9,19) | Protocol invoke/readRegisters/writeRegister and application callback boundary | One normalized terminal callback; scratch RX; clipped positive aggregate timeout; failed attempts counted; full detail; no retries. Both STOP-separated phases share one adapter budget. Callback count is not physical transaction count. Adapter host contracts pass; physical timing remains unqualified. |
| Owner time and bounded work (6,11,19) | poll and TUSB32xOwnerTime helpers | Zero/one/N budgets, fresh next-poll completion, 6/31/96-tick waits, sub-ms completion fixtures, immutable deadlines, late success versus primary backend error, half-range/rollover/backward/stalled time PASS. Direct bounded local pass; no sleep/spin. |
| Lifecycle, tokens and results (6,11,19) | bind/end/invalidate/forget, checkToken/finish/takeResult | Atomic failed bind; active/result backpressure; output preservation; exact take once; token exhaustion without wrap; no reset through end/bind/forget; changed callbacks with null context and two-target fairness PASS. Delivery OK remains separate from result.status. |
| Port/IRQ evidence (15,16,18) | decodePort, fixed PORT_FIRST09/PORT08/PORT_LAST09, bounded service | Complete-only publication, 0xE0 bookend comparison, failed-read history, conservative acquisition time, raw/typed validity, detach NOT_APPLICABLE, service pre-ACK snapshot retention and finiteLatchRaces PASS. No atomic/lossless-event promise. |
| Counters, effects and health (17,19) | invoke, clearTransportStats, projectHealth and raw diagnostics | All-callback versus managed totals, saturation, normalization, preserved failure/detail, stats clearing without trust/effect/result loss, and legal work while OFFLINE PASS. Latest operation effect is distinct from durable unresolved obligations. |
| Simplicity and scope | One private transport, composer and cooperative engine; fixed values/plans | No duplicate protocol implementation, compatibility facade, dynamic result queue or test-only public seed API found. Small private arithmetic fixtures cover saturation/exhaustion without billions of operations. |

### Silicon and pinned-code evidence

Reopened current official TI B/L/A/V datasheets; revision labels remain SLLSEN9F
(March2022), SLLSEP2D/SLLSEQ8D (May2017) and SLLSEO5D (March2021).
All eight archived PDFs match their recorded sizes/SHA-256. Exact source/page
mapping is retained in [register map](register-map.md),
[vectors](register-vectors.md) and [procedures](device-behavior.md).
The lead visually rechecked B Table7-7 p19, A Table10 p19 and V Tables6-10/6-11
p20; reviewers additionally traced timing/procedure and IRQ tables.

Pinned Linux driver/regmap wrapper/core and Betrusted bytes were reopened and
checked against [source provenance](reference/public-implementations.json).
Linux helper arithmetic still computes val & mask: reset value1 with mask08
cannot set bit3. Betrusted's Try.SRC04 is reserved and revision06 lacks verified
TI support. These code observations are rejected, not copied as silicon facts.
Primary callback errors remain visible; later cleanup/reconciliation cannot
rewrite the old failed operation. Existing tests exercise those counterexamples.

Still gated: unexplained revision06; TUSB322I advertisement and all register09
writes/fault-clear semantics; unproved direct non-reset return to DRP. Its DFP/UFP
recipe adds no invented five-ms wait, and its reset-to-DRP proof does not assume
every register resets. Base detected-current refresh requiring explicit reset
remains application policy; reads never reset. Attached-current tracking,
startup enable-polarity conflict, board orientation and electrical conditions
remain documented physical/source limits, not silently resolved by a fake.

### Current Projects inventory and peers

There are still 47 immediate directories and the same 17 independent I2C
libraries: TMP1x2, 24Cxx, INA228, INA3221, ADS1115, OPT4001, LDC1614, BME280,
SHT3x-main, LSM6DS3TR, MB85RC, MCP45HVX1, PCA9555, RV3032-C7, SCD41, SSD1315
and TCA9548A. All are clean on main, with normal .git directories. Active 24Cxx
remains included; the formerly empty EEPROM_24Cxx directory is now absent.
ESS23-RS-p08 is also absent. New DP8382x and MotorControl-CAN are excluded
Ethernet-PHY/CAN projects. Archives, dependencies, nested worktrees and non-I2C
projects remain excluded; FieldCore is consumer evidence and TUSB32x is self.

Changed HEADs since the [prompt1 inventory](audit/execution-1-inventory.json):

| Peer | Current HEAD |
| --- | --- |
| INA3221 | 41b6184b75f3c152611d4236f7ea41a5ce77bc2c |
| ADS1115 | 235af3e8c722661a89a44766a4ccd903a654f6fe |
| LDC1614 | 1db38b63cb93a561b6e0bea51017ab94102ebd24 |
| LSM6DS3TR | 362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc |
| MCP45HVX1 | c6c7feefd79b9fc16ddc7b7786ad187e5185fde6 |
| SSD1315 | 9586b61363b2b70966f84e218f786bcfe9906e76 |

Other library revisions match that inventory. For these six changed repositories,
diffs of AGENTS, include/src/test, tools/scripts/examples, workflows and build/
package manifests from prompt1 are empty. Critical behavior was reopened rather
than inferred from HEAD alone. No sibling fetch/build/edit was performed.

| Fresh evidence | Selected distinction |
| --- | --- |
| LSM6DS3TR src/LSM6DS3TR.cpp bind/checkStart/cancel/takeResult; test/test_basic.cpp; AGENTS, examples, tools and CI | Keep validate-first admission/exact take; reject result-dropping unbind and direct deadline model. |
| ADS1115 src/ADS1115.cpp bind/takeResult; owner tests and example/runner/CI | Keep transactional admission/passive OFFLINE; reject sensor recovery replay and compatibility facades. |
| LDC1614 src/LDC1614.cpp bind/end/startJob/poll/take; fixed fake/backpressure tests | Keep bounded borrowed transport/saturation; reject FIFO results and teardown clearing evidence/counters. |
| TMP1x2 src/TMP1x2.cpp bind/end/health; current CLI/runner/tests | Keep bus-silent end and passive health vocabulary; reject unbind-before-validation, retained binding and owner timeout counted as I2C failure. |
| PCA9555 src/PCA9555.cpp bind/poll; public tests | Keep fixed storage/validity; reject READY-on-bind and chip-specific pointer cleanup semantics. |
| INA3221 test/test_owner_operations.cpp and owner implementation | Keep legal I/O after OFFLINE; reject profile-replaying reconciliation. |
| FieldCore-node 5ad9292dc8481cf08be9a61f662495a6474d1da0, clean rodtester; I2cOwnerTransport/I2cDeviceBinding/I2cLifetime, src/i2c/I2cTask.cpp, docs/guidelines/i2c_peripherals.md and test/native/test_owner_i2c/test_i2c_task_fake.cpp | Keep external serialized ownership, bounded aggregate callbacks and richer completion evidence only when actually supplied. Do not import routing/parking scheduler, recovery, generic Nack classification or native-test diagnostic hooks. |

### Validation and handoff boundary

Fresh maintained validator execution passed **61 disjoint tests per configuration**
in Debug and Release: native 53, contracts 6, doxygen 2. Release checks remain active.
The independent owner reviewer also built and ran seven focused Release tests,
including the isolated consumer with allocation/failure sentinels. Lead build logs
establish correspondence to the reviewed source; reused reviewer binaries alone
are not the final-build proof. Commands/artifacts are in
[prompt37 validation](validation.md#prompt-37-independent-core-review).

This unit changes review documentation only; no core/API/mask/timing/test behavior
changed. No new adapter/CLI defect was found. Prompt38 retains the full integration,
distribution, SDK and final handoff review. No remote CI was dispatched or queried;
the earlier 14-job PASS belongs only to c50f8ddda8809425ede2161586466c95fa650ae6 /
run40782369910. Physical fixture, flash, serial and HIL checks are NOT RUN.

## Integration and distribution review — prompt 38, 2026-10-29

Started clean at `e12ecb74c2a6870665709233cbfaaa5549a804e5` on the same
execution branch, with fetched upstream equality. The prompt 37 core review and
its corrections remain intact. Two fresh-context gpt-6.1-sol/high reviewers
examined CLI/session/runner integration and distribution/documentation; a separate
gpt-6-astra/high reviewer examined SDK deadlines, application lifetimes and board
evidence. All scopes started read-only. The lead verified findings against source
and assigned disjoint fixes; Arduino file ownership transferred sequentially
between the counter and receive-loss corrections. Siblings remained read-only.

### Findings and disposition

| Severity | File/symbol | Reproduction/evidence | Fix | Regression | Remaining limit |
| --- | --- | --- | --- | --- | --- |
| Medium, integration | Both ExampleApp.cpp platform/configuration callbacks; tusb32x_hil_plans.py bus | Both frontends omitted Platform.counters; run_plan unconditionally blocked readonly/soak before init when adapter_available was zero. A documented optional CLI field did not satisfy prompt 29's usable observation plans | Count primary write/write-read invocations in private application wrappers and supply cached counters; preserve arguments, context, timeout and status | Actual Arduino/native application startup fixtures check BUS availability, pointer-read versus data-write, failed attempts, end/bind persistence and saturation; Arduino additionally excludes secondary/prewarm traffic | Counts prove callback kinds for the selected primary target, not physical transactions or an entirely read-only shared bus; board/session gates still apply |
| High, command integrity | Arduino ExampleApp.cpp input loop | Pinned HWCDC drops packet suffixes when its RX queue fills; USBCDC also drops on queue exhaustion. Unbounded fake input missed a valid command prefix surviving dropped junk and later receiving a newline | Fence new input permanently on suspected queue loss, while existing operations and output continue; check capacity before and immediately after each dequeue | Actual application rx-overflow and rx-overflow-after-read cases model bounded SDK input, dropped suffixes and concurrent fill; no reconstructed reset/write and admitted work remains owned | Conservative full-queue rejection; sole reader and audited fixed SDK capacity required. No physical USB/reconnect qualification |
| Low, documentation | Progress status/authorization, development introduction, design-decisions and test README | Prompt 37 still said NOT STARTED in the top ledger; live package/test counts and incomplete-library wording lagged implemented evidence | Reconcile the current ledger, development status, 77-file package and current test inventory; retain dated historical counts | Version/manifest, link, documentation and actual registration checks | Development version is unchanged; no release/tag publication |

The counter wrappers are direct calls, not another transport path or scheduler.
The core, register composer, adapter framing and SDK timeout algorithms did not
change. The runner still blocks unavailable/saturated counters and retains its
foreign-result, identity, loss and cleanup checks.

The USB finding was checked against the installed pinned SDK and official
[HWCDC.cpp 3.3.11](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/HWCDC.cpp)
receive ISR and
[USBCDC.cpp 3.3.11](https://github.com/espressif/arduino-esp32/blob/3.3.11/cores/esp32/USBCDC.cpp)
`_onRX`. Both establish a default 256-byte queue. S2 CDC-on-boot can begin before
application setup, so resizing a live queue was rejected. A capacity-minus-one
post-read check catches a producer filling the queue between the precheck and
the sole consumer's dequeue. This is software evidence, not a measured USB trace.

### Requirement and source review

| Prompts / concern | Reviewed outputs and executable evidence |
| --- | --- |
| 20–22 grammar, effects, bounded records | Shared Cli/ConsoleBuffer, command table and real cli_host; hostile grammar, full queues, result response reservation before take, manual-step replacement, cached/staged/live semantics and aliases remain covered |
| 23–26 framework integration | Actual Wire/native adapters and application fixtures; borrowed handles, address/configuration denial, same-core ownership, synchronous terminal buffers, shared nonblocking gate, coarse/expired budgets, retirement and no automatic recovery; changed application counters and Arduino RX fence tested separately |
| 27–29 ordinary owner and sessions | Renamed public consumer and allocation sentinels; persistent CLI transcript and fake serial test older completion after newer query, stale/duplicate/reboot/loss records, full-width detail, finite deadlines, primary outcome and exact owned take |
| 30–31 explicit fixtures and cleanup | Existing runner guards preserve unknown/incomplete verdicts, durable base recovery prerequisite across sessions, output artifacts, no foreign result consumption and conditional restoration; no hidden reset/current-refresh policy |
| 32 package | Matching literal PlatformIO/IDF allowlists; bounded safe extraction rejects Windows aliases, traversal, duplicate/case collisions and links; real renamed tests-OFF consumers and compiler dependency provenance; no checkout/sibling/maintainer-tool dependency |
| 33–34 checks and CI | Actual finite, disjoint CTest inventory; Release assertions and negative guards; current workflow source inspected only. Prior remote result remains attached to its original tested SHA |
| 35–36 documentation and visibility | Public XML/HTML visibility and missing-comment negative fixtures, generated version boundaries, packaged links and source API/CLI/feature mapping; live drift corrected without rewriting historical results |

Fresh clean peer references: TMP1x2 `bafc34067530eb340e55c42dae3fad4eb795b6e9`,
24Cxx `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e`,
LDC1614 `1db38b63cb93a561b6e0bea51017ab94102ebd24`,
INA228 `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b`,
INA3221 `41b6184b75f3c152611d4236f7ea41a5ce77bc2c` and
OPT4001 `dab818e575fc25a0949d962bc0df600eb2ed5200`.
Reopened common CLI/output, adapters, finite runners, package/version tools,
component CMake, tests and workflow sources. Retain strict grammar, bounded
records, terminal evidence and renamed consumers. Reject bulk firmware schedulers,
blind restoration, fixed component-directory assumptions, enclosing Git identity
and source-only dependency provenance. Exact example/runner paths remain in the
CLI contract and dated peer audit records; the prompt 37 inventory remains current.

Board guidance was compared again with CHIP_EVIDENCE and exact TI B/L/A/V tables:
seven-bit 0x60/0x61 versus 0x47/0x67, explicit pointer/STOP framing and 400 kHz
maximum; exact supply/straps/enable readiness; LAI/HAI 2.7–5.0 V and local 3 A
advertisement at least 3.5 V; 322I 4.5–5.5 V, separate VBUS_DET requirement and
5 V VDD prerequisite for autonomous VCONN. The 225/300/375 mA VCONN limit and
CC microamp signaling remain distinct from VBUS load. No physical interpretation,
startup wording conflict, current-refresh requirement or closed silicon gate was
relaxed to obtain passing software evidence.

Final commands, image provenance, local tool limitations and the supported
software/hardware evidence matrix are recorded in the prompt 38 validation entry.

### Final support and qualification handoff

| Delivered scope | Evidence and limit |
| --- | --- |
| Six named TUSB320 variants, primary LAI | Portable C++17 core and explicit variant gates; external transport/time/ownership, retained results/effects, profile/reset/recovery/port/IRQ/raw/health behavior. Host software evidence, not exact-part physical qualification |
| Partial TUSB322I | Separate identity, read observations, qualified DFP/UFP and reset; advertisement/all 09 writes and direct apply-to-DRP remain unavailable. No invented fault-clear transaction or VCONN application policy |
| CLI, examples and Python plans | One shared 55-row CLI, ordinary simulated C++ owner, Arduino basic/two-device owner, native-IDF application, bounded observation/fixture runners. Primary callback proof now available; RX loss cannot authorize reconstructed commands |
| Host/package/local firmware | All 74 Windows registrations covered in Debug/Release by current scoped runs; Doxygen, clean archive consumers, four current Arduino images and two current IDF 5.5.5 images PASS. Exact commands, inputs and artifacts in [final validation](validation.md#prompt-38-final-integration-and-distribution-review) |
| IDF 6.1 and other unavailable local tools | New S2/S3 image checks BLOCKED by the missing local SDK. Prior 6.1/Linux sanitizer CI evidence retains the earlier tested SHA; it is not final-commit proof |
| Hardware and release | Flash, serial/HIL, electrical and board scheduling qualification NOT RUN. No release/tag/registry publication. Source gates and application board prerequisites remain mandatory |

No locally reproduced implementation defect remains open. Promptset software
implementation and reviews are finished; the missing local SDK and physical
qualification prevent a claim that every final framework image or board has been
validated. Final commit/sync equality is reported in the handoff. Historical remote
PASS remains run40782369910 at c50f8ddda8809425ede2161586466c95fa650ae6 only.
