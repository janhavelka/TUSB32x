# Implementation progress

Execution started: 2026-10-23 (Europe/Prague).
Branch: `promptset/tusb32x-implementation-20261023`.
Base: `ca0cfe9fa9471a2bf327a6957c60ebd5b2c86b35` (main and origin/main equal after fetch).
Keep this branch for this entire execution, including resumed sessions.
Current authorization: complete prompt 38 integration/distribution review and final handoff.

## Prompt status

| Prompt | Status |
| --- | --- |
| [1: ecosystem-audit](prompts/1-ecosystem-audit.md) | COMPLETE |
| [2: family-capabilities](prompts/2-family-capabilities.md) | COMPLETE |
| [3: register-contracts](prompts/3-register-contracts.md) | COMPLETE |
| [4: silicon-procedures](prompts/4-silicon-procedures.md) | COMPLETE |
| [5: value-api-contracts](prompts/5-value-api-contracts.md) | COMPLETE |
| [6: owner-state-contracts](prompts/6-owner-state-contracts.md) | COMPLETE |
| [7: host-build](prompts/7-host-build.md) | COMPLETE |
| [8: metadata-versioning](prompts/8-metadata-versioning.md) | COMPLETE |
| [9: transport-identity](prompts/9-transport-identity.md) | COMPLETE |
| [10: register-codecs](prompts/10-register-codecs.md) | COMPLETE |
| [11: owner-engine](prompts/11-owner-engine.md) | COMPLETE |
| [12: profile-observation](prompts/12-profile-observation.md) | COMPLETE |
| [13: profile-application](prompts/13-profile-application.md) | COMPLETE |
| [14: reset-recovery](prompts/14-reset-recovery.md) | COMPLETE |
| [15: port-snapshots](prompts/15-port-snapshots.md) | COMPLETE |
| [16: interrupt-service](prompts/16-interrupt-service.md) | COMPLETE |
| [17: diagnostics-health](prompts/17-diagnostics-health.md) | COMPLETE |
| [18: protocol-conformance](prompts/18-protocol-conformance.md) | COMPLETE |
| [19: owner-portability](prompts/19-owner-portability.md) | COMPLETE |
| [20: cli-foundation](prompts/20-cli-foundation.md) | COMPLETE |
| [21: cli-operation-control](prompts/21-cli-operation-control.md) | COMPLETE |
| [22: cli-command-coverage](prompts/22-cli-command-coverage.md) | COMPLETE |
| [23: arduino-transport](prompts/23-arduino-transport.md) | COMPLETE (qualified SDK subset; hardware not tested) |
| [24: arduino-examples](prompts/24-arduino-examples.md) | COMPLETE (four SDK images; physical qualification not run) |
| [25: idf-transport](prompts/25-idf-transport.md) | COMPLETE (two native SDK links; physical qualification not run) |
| [26: idf-cli-example](prompts/26-idf-cli-example.md) | COMPLETE |
| [27: generic-owner-consumers](prompts/27-generic-owner-consumers.md) | COMPLETE |
| [28: python-session](prompts/28-python-session.md) | COMPLETE |
| [29: observation-runners](prompts/29-observation-runners.md) | COMPLETE |
| [30: port-fixture-suites](prompts/30-port-fixture-suites.md) | COMPLETE (software evidence; hardware separate) |
| [31: recovery-fixture-suites](prompts/31-recovery-fixture-suites.md) | COMPLETE (software evidence; hardware separate) |
| [32: package-consumers](prompts/32-package-consumers.md) | COMPLETE (software evidence; hardware separate) |
| [33: validation-guards](prompts/33-validation-guards.md) | COMPLETE (software evidence; hardware separate) |
| [34: ci-matrix](prompts/34-ci-matrix.md) | COMPLETE (14 CI jobs; eight extracted images; hardware NOT RUN) |
| [35: user-documentation](prompts/35-user-documentation.md) | COMPLETE (local guides/snippets/package checks; physical qualification separate) |
| [36: source-maintainer-docs](prompts/36-source-maintainer-docs.md) | COMPLETE (source/API visibility and maintainer checks; hardware separate) |
| [37: final-core-review](prompts/37-final-core-review.md) | COMPLETE (e12ecb7; independent review and local checks; hardware separate) |
| [38: final-integration-review](prompts/38-final-integration-review.md) | COMPLETE (review/fixes and available local matrix; new IDF 6.1 links BLOCKED by missing SDK; hardware NOT RUN) |

## Prompt 1 execution record

Scope: current ecosystem inventory and selected engineering contracts; no driver
implementation. Lead handles decisions and Git. Three inherited-model subagents
have disjoint audit-report ownership: inventory/exclusions, owner/consumer, and
CLI/tooling. Sibling repositories are read-only; no sibling fetch, build or edit.

Sources and decisions: [library comparison](library-comparison.md) and
[engineering decisions](design-decisions.md), backed by the three execution
audit reports and full inventory. Local checks passed; see [validation](validation.md).
All 17 peers remain clean at their captured revisions. FieldCore is clean on
rodtester at 4e6a05e. The baseline is confirmed with explicit peer exclusions.
Native IDF was not located; no source/build/hardware support is inferred. Remote CI is
outside this prompt. Completed commit and sync evidence belongs in the final
handoff and the next progress update, avoiding a self-referencing commit loop.

## Independent prompt 1 review

Reviewed completed commit `18aba0b6baa2b10ce5ab9c2b77e51bffb831b7a0` against the
original prompt and its diff from the recorded base. At review start it matched
the same-name origin branch after fetch, with zero ahead/behind and a clean tree.
Three fresh-context reviewers checked coverage/scope, source correctness and
inventory/validation. The lead verified findings against actual peer files.
Corrections and repeated checks are recorded in [validation](validation.md).
No later prompt was started; the execution branch remains unchanged.

## Prompt 2 execution record

Started from synchronized commit `9670c53b15b3e8c338479134e539ef5b4dd441d4`, the
completed independent prompt 1 review. Local and same-name origin branch matched
after fetch, with zero ahead/behind and a clean worktree. The execution branch
and original base are unchanged.

Scope: family capabilities, source provenance and board prerequisites only.
The lead owns variant/coverage matrices, shared decisions, source metadata and
Git. Delegation used two gpt-6-astra agents at high reasoning for disjoint B/L
and A/V/GPIO source reviews, and gpt-6.1-sol at high reasoning for the peer/public
source audit. The latter owned only `docs/audit/execution-2-peers.md`; other agents
were read-only. Reviewers then checked the completed matrices independently.

Focused peers remained clean on main: TMP1x2 `bafc340`, 24Cxx `936aa85`, LDC1614
`c0ab4c1`. Full revisions, exact declarations, tests, AGENTS and build/package
paths are in [the focused report](audit/execution-2-peers.md). Adopt explicit
variant/address and shared-identity boundaries, independently derived fakes and
complete provenance. Reject sensor semantics, guessed exact identity, inherited
blocking/retry behavior, permissive unexplained revisions and packaged PDFs.
No sibling repository was fetched or modified.

The [variant matrix](variant-matrix.md) establishes the six explicit 320 parts,
separate 322I gates, GPIO-only exclusions, exact board electrical requirements
and strict future-revision policy. [Feature coverage](feature-coverage.md) maps
planned APIs/CLI to exact sources and future tests. Current TI product pages add
the LI/HI DFP/DRP recommendation; source corrections include the 322I VCONN
resistance maximum. Dated preparation reports retain explicit correction notes.
All eight PDFs retain original terms; the seven old byte streams are unchanged.
No driver, full register specification or complete procedure implementation was
introduced. Prompt 3 remains NOT STARTED.

Validation and remaining silicon/board gates are recorded in
[validation](validation.md) and the variant matrix. Hardware qualification is
not inferred from the source review. Commit/sync evidence belongs in the handoff
and the next progress update, avoiding a self-referencing commit edit.
Source hashes, document links, metadata, preservation and focused peer-state
checks passed. Review findings were resolved against the actual sources.

## Independent prompt 2 review

Reviewed completed commit `30bc8a88d47bb599c92db8a8ddd1faa234e464af` against the
full original prompt and its diff from `9670c53b15b3e8c338479134e539ef5b4dd441d4`.
At review start, HEAD and the same-name origin branch matched after fetch, with
zero ahead/behind and a clean tree. The execution branch and original base remain
unchanged. Three fresh-context, read-only agents reviewed coverage/contracts,
silicon claims and source/peer provenance in parallel; the lead verified the
findings against rendered TI pages, current peer code and the actual diff.

Corrections add the board-owned 5 V VDD/VCONN condition and explicitly gate all
322I register09 writes until safe fault handling is established. This includes
ACK/service, duty, UFP-accessory and raw writes; unchanged proved controls still
permit independently supported operations elsewhere. The peer report now names
TMP1x2's ARA decoder precisely. Five previously text-only register-table pages
were visually checked. The two contract reviewers found no remaining issue in
the correction diff.

Fresh official/public downloads, archive hashes and retained metadata, peer
states, the peer archive guard, documentation links and whitespace checks passed;
details are in the independent-review section of validation.md. No source bytes,
numbered prompts or production code changed. Hardware and future behavior tests
remain unexecuted; named silicon/board gates remain explicit. Prompt 3 is NOT
STARTED. Commit/sync evidence belongs in this review's handoff and the next
progress update, avoiding a self-referencing commit edit.

## Prompt 3 execution record

Started from synchronized prompt 2 review commit
`cd591d4b56b83bda4f3f641bf5ee1c86f5d8a08c`. HEAD and upstream matched after fetch,
with a clean tree; the recorded execution branch and original base are unchanged.
Scope: register access, encodings and independent vectors, without production
code or prompt 4 procedure implementation. The lead owns the register guide,
vectors and shared records. Two gpt-6-astra/high agents review B/L/A and V source
tables read-only; one gpt-6.1-sol/high agent owns the focused peer audit report.
Only the lead changes Git state. All three agents subsequently reviewed the
actual register guide/vectors read-only; no confirmed defect remained. The lead
checked the source pages, peer implementations, public helpers and arithmetic.

PCA9555 remained clean on main at `498178e`, LDC1614 at `c0ab4c1` and ADS1115 at
`381236e`;
full revisions and source/test/build paths are in audit/execution-3-peers.md.
Adopt named access masks, procedure guards, explicit semantic validation and
independent literal bytes. Reject pair wrapping/pointer parking, ADC framing and
timing, and fake models that turn shared constants into their own oracle.

docs/register-map.md and docs/register-vectors.md complete the map and 82 literal
cases: 36 identity/range/decode cases, 21 composition cases and 25 failure/boundary
cases. They separate partial base defaults, protected internal bits, live/command
bits and profile comparison. Shared evidence now records the exact audio
charged-through meaning and the V accessory reset width typo. V08/V09 write gates
and V_RETURN_DRP remain unresolved; unchanged proved controls permit independent
supported work. No runtime API, driver or procedure implementation was added.

Local validation passed: eight source hashes, eight access-mask partitions,
all 21 literal compositions, selected field/regmap/DRP arithmetic, peer state and
all 18 cited test registrations, source helper hashes, Markdown links and diff
checks. validation.md records source pages and check limits. Future behavioral
tests, physical qualification and firmware/package builds remain NOT RUN. There
is no C++ source requiring Doxygen in this prompt; the guide specifies the
citations/comments due with future CommandTable/codecs. Prompt 4 is NOT STARTED.
Commit/sync evidence belongs in the handoff and next progress update, avoiding
a self-referencing commit edit.

## Independent prompt 3 review

Reviewed completed commit `a7cf07055ff46bde2cf460d4cee3e2066b30c77c` against the
full original prompt and its diff from `cd591d4b56b83bda4f3f641bf5ee1c86f5d8a08c`.
At review start, HEAD matched the same-name origin branch after fetch, with zero
ahead/behind and a clean tree. The execution branch and original base are unchanged.
Three fresh-context, read-only agents reviewed coverage/simplicity, silicon
semantics and verification/peer evidence in parallel. The lead checked their
findings against the actual contracts, peer code and scoped diff.

No register-mask, encoding or silicon-gate defect was found. Two narrow
corrections distinguish admission rejection from rejection after a counted read
in vector N18, and identify the PCA9555 runner's retained classifier evidence
accurately. No production code or new procedure contract was introduced.

The fresh review repeated source-table inspection, archive/public-source hashes,
mask/vector arithmetic, peer/test-registration checks and documentation checks;
see the independent prompt 3 review in validation.md for evidence and limits.
Behavioral/hardware tests remain future work. Prompt 4 remains NOT STARTED.
Commit/sync evidence belongs in this review's handoff and the next progress
update, avoiding a self-referencing commit edit.

## Prompt 4 execution record

Started from synchronized prompt 3 review commit
`ff0d6a21b65579b7bc3b9b454b16a14b21b74257`. HEAD and same-name upstream matched
after fetch, with zero ahead/behind and a clean tree. The execution branch and
original base are unchanged. Scope is transport, silicon procedures, timing and
failure/effect specification, without production code or physical qualification.

Two gpt-6-astra/high agents independently reviewed exact TI procedures and
time/effect/transfer arithmetic read-only. One gpt-6.1-sol/high agent owned only
docs/audit/execution-4-peers.md and then reviewed the full guide read-only.
The lead owns device-behavior.md, shared records, integration and Git. All three
reviewed the actual procedure draft; confirmed gaps were resolved against sources.

PCA9555, LDC1614, ADS1115 and LSM6DS3TR initially were clean at the revisions in
the focused report. Final recheck found concurrent LDC/ADS documentation edits;
their inspected source/test/build files and all four HEADs were unchanged.
Those sibling changes were left untouched. Adopt explicit guarded transfers,
external ownership, fresh wait anchors and retained effects. Reject pair wrapping, ADC framing, fixed or
renewed per-phase timeouts, saturating owner time, automatic replay and claims
that a mock establishes silicon or SDK bounds. This task fetched or edited no sibling.

The guide specifies STOP-separated explicit-pointer reads; one aggregate terminal
callback budget; scalar identity/control proof; role/reset/recovery/port/IRQ
plans; independent byte/callback ceilings and 30 procedure/12 timing vectors.
Ordinary45 qualification and recovery-versus-other-transition admission are
explicit conservative policy. Reset success, settled-state evidence and retained
recovery uncertainty are separate. All existing V and board/source gates remain.

README, register/variant/coverage guides, baseline, chip evidence and decisions
now reference the completed procedures. Verification outcomes and any remaining
limits are recorded in validation.md. No C++ source exists for Doxygen yet;
future source citations are specified. All 34 applicable transfer-bound triples,
11,988 fractional-time checks, eight archive hashes, four public-source hashes,
18 cited peer test registrations and 422 local links passed. All three reviewers
found no remaining concrete issue in the completed guide. Prompt 5 is NOT STARTED. Commit/sync
evidence belongs in the handoff and next progress update, avoiding a self-reference.

## Independent prompt 4 review

Reviewed completed commit `d5884da8f1a5b17d38f506b440a66a710c75e7fb` against the
full original prompt and its diff from `ff0d6a21b65579b7bc3b9b454b16a14b21b74257`.
At review start, HEAD matched the same-name origin branch after fetch, with
zero ahead/behind and a clean worktree. The execution branch and base are unchanged.
Three fresh-context, read-only reviewers checked coverage/simplicity, exact TI
silicon procedures and verification/peer claims in parallel. The lead reviewed
the actual diff, source contracts, peer changes and independent arithmetic.

No silicon/procedure defect, missing requirement or incorrect transfer ceiling
was confirmed. Reset and ACK expressions now state each variant's hexadecimal
mask directly, avoiding ambiguous slash shorthand; pointer wording is more
precise. Moving the ACK formula outside its table also fixes the unescaped
bitwise-OR table delimiter. These are documentation corrections, not behavior changes.

All repeated checks passed; the independent prompt 4 section of validation.md
records source pages, counts and current peer revisions. LDC1614, ADS1115 and
LSM6DS3TR advanced through documentation-only commits since the original audit;
their inspected code/test/build evidence is unchanged and all four peers are
clean. Siblings remained read-only. No implementation, hardware qualification
or prompt 5 work was added. Commit/sync evidence belongs in this review's handoff
and the next progress update, avoiding a self-referencing commit edit.

## Prompt 5 execution record

Started from `d29cb36b3e0f5ed57dc5512964ce40e2b5326ae2`, the independently
reviewed prompt 4 completion. HEAD and same-name origin branch were equal after
fetch, ahead/behind 0/0 and worktree clean. The execution branch/base are unchanged.
Prompt 5 is complete; prompt 6 remains NOT STARTED.

Implemented four portable public headers: exact constexpr Status/errors,
terminal callback/binding/settings values, operation/result/snapshot/diagnostic
values, and the passive RegisterDecode result. Every public type/member/enum/
function has source Doxygen. Default values have no I2C, clock, allocation or
implicit power policy. The noncopyable/nonmovable facade, validators, decoder
and health projection remain planned signatures in the API guide and a separate
compile-only fixture. No fake method definitions or public unresolved calls exist.

[Current peer evidence](audit/execution-5-peers.md) records all five clean HEADs,
actual declarations/implementations/tests, examples, runner record names, AGENTS
and registration. The selected Status and profile split stand. Peer sensor
aliases/replay, combined hardware/host profiles and ABI-specific sizes were rejected.
Sibling sources remained read-only and no sibling build/fetch occurred.

The lead owns shared contracts and Git. Three disjoint delegated tasks used
gpt-6.1-sol/high for the four value headers, gpt-6.1-sol/high for the peer report
and later linked passive-value test, and gpt-6-astra/high for independent
callback/error/value/scope review. The lead wrote the API/architecture guides,
planned fixture and second translation unit, reviewed actual headers and peer
evidence, integrated changes and ran the final checks.

Review refinements: result IRQ evidence belongs only to that operation while
diagnostics may retain historical IRQ observations; independent recovery/mode/
reset uncertainty cannot be overwritten by the newest result. The decoder stores
field codes without inventing port applicability. Copyability tests make no ABI
compatibility promise. These close previously unspecified value details without
changing silicon procedures or adding competing state rules.

PASS: four standalone strict C++17 headers; linked NDEBUG passive/Status consumer
with 23 error names, constexpr/explicit-bool/full-detail checks and independent
validity; syntax-only planned API; strict Doxygen and all 17 public structs visible;
local documentation/provenance/scope checks. Exact commands/results are in
[validation](validation.md). Local generated Doxygen output is ignored under
build/; the final run's output is in TEMP. No generated files are committed.

README, CHANGELOG, AGENTS, architecture/API, baseline, coverage, decisions and
test instructions were updated. Variant/register/procedure guides and archived
TI evidence were reviewed and need no semantic changes. No firmware/package,
driver behavior or hardware qualification is claimed. Commit/sync evidence is
reported in the handoff and next progress update, avoiding self-reference.

## Independent prompt 5 review

Reviewed commit `c5fcc983e36cd3c79005a420fc4db7b752c73346` and its complete diff
from `d29cb36b3e0f5ed57dc5512964ce40e2b5326ae2` after rereading original prompt 5
in full. At review start, the same execution branch was clean and equal to its
upstream after fetch, with zero ahead/behind. The recorded execution base is unchanged.

Three fresh-context, read-only agents audited requirements/simplicity,
correctness/edge cases, and tests/peer evidence in parallel. They used
gpt-6.1-sol/high, gpt-6-astra/high and gpt-6.1-sol/high respectively. The lead
checked the actual headers, baseline signatures, documentation and scoped diff,
and independently repeated the required compilation, linked consumer and
Doxygen checks. All five peer revisions and clean states still match the
original prompt 5 audit; sources, tests and registration substantiate its claims.

No confirmed bug, omitted prompt 5 requirement or unnecessary abstraction was
found. Public code and API remain unchanged; this commit records the completed
review only. All checks passed, including four standalone headers, the NDEBUG
two-TU consumer, planned-signature syntax, Doxygen visibility of 17 structs and
80 Markdown files with 452 local links. Validation records the exact scope and limits.

Prompt 6 remains NOT STARTED. Detailed owner-state semantics, validators/build,
decoder implementation and device operations stay in their assigned later
prompts. No runtime protocol, framework/package or hardware qualification is
claimed. The lead owns the review commit and synchronization; final evidence is
reported in the handoff and next progress update without a self-reference.

## Prompt 6 execution record

Completed 2026-10-25 on the same execution branch/base. Starting commit was
`87979dc07fc319603f0bd6ed21224b45ea20d1d8`, the independent prompt 5 review.
Before editing, the worktree was clean and fetch confirmed equality with the
same-name origin upstream (0 ahead / 0 behind). Prerequisite prompts 1–5 and
their actual validation records were read; no driver/build stage was started.

Completed the API method/admission, lifecycle/result, configuration, termination,
reset, IRQ, owner-clock, snapshot and health tables. Added exact rejection and
interruption vectors O01–O25 for later behavioral suites. Fixed three contract
gaps found during review: matching identity plus complete raw profile can permit
explicit recovery while configuration remains UNKNOWN; failed no-write proof
acquisition alone preserves earlier actual trust; ordinary settings/raw-write
uncertainty remains visible through later read-only results in the new passive
configurationWriteUnresolved flag. Baseline and procedure cross-references now
agree with these rules; no silicon encoding, sequence, wait or byte ceiling changed.

Three fresh-context agents ran disjoint tasks in parallel: gpt-6.1-sol/high
audited current peer code/tests/examples/runners/registration in
[execution-6-peers](audit/execution-6-peers.md); gpt-6.1-sol/high authored only
owner_examples.cpp and time_arithmetic.cpp; gpt-6-astra/high independently reviewed
time/effects/admission and later the integrated contracts and fixtures read-only.
The peer agent also performed an independent final coverage/simplicity review.
The lead selected shared contracts, checked actual peer source/test bodies and
the diff, updated public Doxygen/docs/tests, and repeated required verification.
Final reviewer passes found no remaining confirmed issue in the scoped work.

Current LDC/LSM/ADS/INA/TMP revisions match prompt 5 and remain clean; FieldCore
is now a002abae533b0aa767c235c2197d3b548bdcdd83, rodtester, clean. Exact paths,
full revisions and adopted/rejected rules are in the focused audit. Siblings
remained read-only; their tests/builds/remote CI were not run. No unrelated peer
replay, stale-time batching or evidence-erasing teardown was imported.

PASS: strict isolated compilation of four public headers; linked NDEBUG
two-translation-unit value/Status executable including the new flag's independent
storage; syntax-only planned signatures and observation/apply/cancel/two-device
examples; 60 constexpr unsigned-time vectors; Doxygen warnings-as-errors with all
17 public structs visible; 13 procedure-bound rows/34 variant counts, 30 procedure
vectors/12 time vectors, 11,988 exact fractional-time checks and 8 unchanged TI
PDF hashes; Markdown links/anchors, numbered prompt inventory and diff whitespace.
Commands, evidence and limits are in [validation](validation.md).

All output is in TEMP; no generated artifacts are committed. Methods remain
planned in docs and compile-only fixtures, with no missing public link promises
or fake successful driver. Runtime state/transport/health, firmware/package and
physical qualification are NOT RUN and assigned to later prompts. Prompt 7 is
NOT STARTED. The lead commits/syncs this checked unit; final branch/commit/equality
evidence is reported in the handoff and the next progress update.

## Independent prompt 6 review

Reviewed 2026-10-25 against the full original prompt 6 and the actual
`87979dc..b129c37` diff. Starting commit was
`b129c37be1feeba9c4474f8b1788a9b42c9ac86e`. The recorded execution branch/base
is unchanged; the worktree was clean and fetch confirmed upstream equality,
zero ahead/behind, before review edits.

Three fresh-context read-only reviewers worked in parallel: gpt-6-astra/high
checked correctness and time/effect edge cases; gpt-6.1-sol/high mapped every
requirement and reviewed scope/simplicity; gpt-6.1-sol/high independently compiled
the fixtures and rechecked actual peer implementations, tests and registration.
The lead read the original prompt, contracts, values, fixtures and scoped diff,
verified findings against those artifacts and repeated local verification.

No confirmed code or API-contract defect, missing prompt 6 deliverable or
unnecessary abstraction was found. Identity readiness with UNKNOWN configuration,
raw-versus-managed evidence and retained reset-acceptance uncertainty are explicit
documented policies, not bugs. All six peer revisions and clean states still
match execution-6-peers; no sibling was modified, fetched, built or tested.

Fixed a gap in verification documentation: the independent fractional-time check
was described without its runnable command. Validation now contains the exact
self-contained 11,988-case Fraction command. No public value, planned API,
silicon rule, driver implementation or example behavior changed.

PASS: strict four-header compilation; linked NDEBUG two-TU passive consumer;
all three syntax-only fixtures and their 60 arithmetic assertions; strict Doxygen
and visibility of all 17 public structs; procedure/timing arithmetic, archived
source hashes, local links and scoped whitespace. Detailed commands and evidence
are in validation. Runtime driver behavior, adapters, firmware/packages and
hardware remain NOT RUN. Prompt 7 remains NOT STARTED. The lead commits/syncs
this audit record and reports exact final equality in the handoff.

## Prompt 7 execution record

Completed 2026-10-26 on the same execution branch/base. Starting commit
cdbb36cda836ef1cb7c4d66864d93c5ec2b31418 is the completed independent prompt6
review. Before editing, status/index were clean and fetch confirmed equality
with the same-name origin upstream, zero ahead/behind. No new branch was created.

Re-audited six current peers in [execution-7-peers](audit/execution-7-peers.md):
full revisions, clean states, AGENTS, actual declarations/implementations,
examples/CLI, Python runners, CMake registration and workflow sources. The lead
checked the returned source choices against actual TMP/24C/ADS/LDC files.
Adopted explicit PUBLIC C++17/includes, tests OFF, private strict warnings,
active Release checks and real multiple-TU runtime linkage. Rejected LDC's
configure-time Python/version dependency, sensor-specific restrictions and
build-only consumer success as behavioral proof. All siblings stayed read-only.

Added one small static TUSB32x target, Config.h declarations and real pure
validateDriverConfig/validateProfile definitions in src/TUSB32x.cpp. Kept passive
values/Status unchanged; removed those two functions' planned-only redeclarations.
Invalid values return INVALID_CONFIG, unavailable valid settings return
UNSUPPORTED_FEATURE. Null user context and positive uint32 timeout extremes are
valid. Debounce/absent controls use the verified variant maps. TUSB322I readable
profile settings remain semantically valid independently of later changed-field
write gates; no existing gate or silicon procedure changed. Four fresh TI PDFs
match their archives byte-for-byte; all eight archived hashes remain valid.

Delegation used gpt-6.1-sol/high for disjoint peer-audit report ownership and
test/test_validators.cpp plus examples/host/basic ownership. Signatures/rules were
fixed before parallel authoring. A fresh gpt-6-astra/high reviewer checked the
integrated code/build/consumer/contracts read-only and found no functional/build
defect; its stale coverage-disclaimer finding was fixed. The lead owns all
integration, documentation, source review, test registration and Git operations.
No agent switched branches or mutated Git/siblings.

PASS: strict Debug and Release builds with four isolated public-header TUs and
three compile-only planned fixtures; exactly two real CTest suites execute in
each configuration; 76,452 validator cases with zero counting callbacks;
existing Status/constexpr/int32/multi-TU checks; direct strict NDEBUG validator
execution by the test agent; tests-OFF standalone Release consumer; clean renamed
source copy under unrelated Git metadata, no maintainer tools on child PATH,
C++17 propagation from an explicit C++11 consumer baseline; strict source/example
Doxygen and XML visibility of17 structs/validators; local links/anchors, source
hashes, scoped whitespace and independent diff review. Exact versions/commands
and evidence are in [validation](validation.md).

ASan/UBSan BLOCKED: installed MinGW cannot link -lasan/-lubsan; WSL is not
installed. MSVC/Clang NOT RUN (not available on PATH). Driver behavior, framework
firmware, package export/archive consumption and physical qualification remain
NOT RUN. No unavailable tool was installed and no missing behavior was stubbed.
No remote CI was dispatched or polled. Prompt8 remains NOT STARTED: this user
request names prompt7, and COMMON scopes execution to the requested unit.

Updated source Doxygen, README, CHANGELOG Unreleased, AGENTS, CONTRIBUTING,
architecture, API/feature coverage, test/consumer guidance, decisions and progress.
Register/procedure/variant guides were reviewed and left unchanged because no
silicon capability, map or sequence changed. Package/CLI/HIL guides and workflows
remain later responsibilities, with no empty placeholder files created.
The lead commits this coherent checked unit, fetches/pushes the existing upstream
and verifies commit equality and clean task state. The resulting completed commit
and final synchronization evidence are in the handoff and next progress update;
this entry does not create a self-referencing commit-hash update loop.

## Independent prompt 7 review

Reviewed 2026-10-26 against the complete original prompt 7 and the actual
cdbb36c..d221635 code/documentation diff. Starting commit was
`d22163546c927a03717322f2f47aeb0798d4b1c6`. Before editing, the existing execution
branch and same-name upstream were equal after fetch, zero ahead/behind, and the
worktree/index were clean. The prior completed implementation commit is now
recorded here without a self-referencing hash update.

Three fresh-context agents worked read-only in parallel: gpt-6.1-sol/high mapped
each requirement to actual code/docs; gpt-6-astra/high checked correctness, edge
cases, archived TI tables and simplicity; gpt-6.1-sol/high re-audited all six
current peers and independently checked tests/consumers. All inspected the
original prompt and actual diff rather than accepting earlier summaries. The
lead read the source/diff and checked every reported finding against the files.
Current peer revisions/clean states and adopted/rejected choices remain those in
[execution-7-peers](audit/execution-7-peers.md), refreshed for this review.

No functional/build/API defect, missing prompt 7 deliverable, material test gap
or unnecessary implementation abstraction was found. Fixed one source citation:
TUSB322I's absent 0x45 register is established by SLLSEO5D Table 6-6, not its
identity Table 6-7 on the same page. Repaired damaged page/prompt range punctuation
in decisions/validation, clarified Ninja Release selection in the consumer guide
and corrected the feature guide's stale description of its implemented table.
Validator logic, public values, build registration and tests are unchanged.

PASS: strict Debug/Release builds and exactly two CTest suites; 76,452 validations
with zero callbacks; direct strict optimized NDEBUG validator/passive executables;
four isolated public-header TUs and three planned compile fixtures; clean renamed
tests-OFF consumer under unrelated Git metadata without maintainer tools on child
PATH, inheriting C++17 from a C++11 baseline; strict Doxygen and exact visibility
of 17 public structs/real functions; archived source hashes and relevant TI
tables; Markdown links/anchors, whitespace and full scoped diff review. All
reviewers also independently built/tested their assigned boundary. Exact lead
commands/results are in [validation](validation.md).

ASan/UBSan remain BLOCKED: the fresh probe cannot link missing -lasan/-lubsan.
MSVC/Clang, framework firmware, archive/package qualification and physical hardware
remain NOT RUN. No remote CI was dispatched or polled. Prompt 8 remains NOT
STARTED; this review starts no later implementation unit. The lead commits and
syncs these checked corrections on the same branch; final commit/equality and
clean-state evidence belong in the handoff and next progress update.

## Prompt 8 execution record

Completed 2026-10-26 on the same execution branch/base, starting from the completed
prompt 7 audit commit `93f2d31374dc16bcc95800d0e0c516c4372faa4a`. Initial status/index
were clean; fetch showed same-name upstream equality and zero ahead/behind.
The two prerequisite native suites passed before metadata changes. No new branch
or later implementation unit was started.

Re-audited current TMP1x2/24Cxx generators/guards, LDC component CMake,
INA3221/OPT4001 packaging, author/license/format conventions, AGENTS, manifests,
generated files, workflows, actual scripts/tests and relevant example/CLI/runner
identity sources. [execution-8-peers](audit/execution-8-peers.md) records all full
revisions, clean states, exact paths and selected/rejected rules. Lead verified
actual source bodies and official IDF/PlatformIO documentation. Chose native 1.2.1
and existing Core 6.1.19; IDF >=5.3.0 is admission only, pending real firmware links.

Added development 0.1.0-dev.1 metadata, explicit package boundaries, native IDF
registration sharing the real source list, generated stable version constants,
strict sync/check/bump/set, managed Windows launcher, native PlatformIO consumer,
MIT/ownership/format files and strict Doxyfile. No consumer invokes Python/Git;
no version text contains timestamps/commits or claims flashed firmware identity.
Maintainer Python fixtures are registered only with tests ON. No-op sync/set
preserves bytes/mtimes and invalid arguments/metadata/templates fail distinctly.

Delegation: gpt-6.1-sol/high owned only the peer audit, then reviewed packaging/
component/launcher read-only; another gpt-6.1-sol/high owned only the existing-name
tools/test_version_sync.py suite after its format/exits were fixed; gpt-6-astra/high
reviewed generator/error paths, CMake, metadata, tests, package and launcher
independently read-only. The lead owns metadata/generator/build integration, docs,
all fixes and Git. No reviewer found a confirmed functional defect. Lead corrected
an ad hoc Doxygen check's filename assumption using generated XML IDs; no warning
was suppressed and all public values/functions remained visible.

PASS: generator sync/check and root byte+mtime no-op; 17 isolated version cases,
including missing/malformed SemVer, every version-field drift, exact argument/
error exits, overflow, preflight, missing header/I/O failure, no Git, enclosing
Git, dirty example and no child-process attempts; Debug/Release 3/3 CTests; five
independent public headers, constexpr/cross-TU version values and existing 76,452
validator cases; plain tests-OFF consumer; actual native PlatformIO compile/link/
execution; real 29-file archive with forbidden content absent; renamed extracted
strict consumer under unrelated Git without maintainer tools on child PATH,
inheriting C++17 from a C++11 baseline; host-only component-registration fixture;
six launcher argument/status comparisons and missing-install failure; strict
Doxygen 17 structs/four version values and public functions; local links/anchors,
Python docstrings/style, unchanged eight vendor hashes and scoped whitespace.
Exact commands, versions and limits are in [validation](validation.md).

The existing Core installed its resolved tool-scons 4.40801.0 while running the
native example; no second Core was installed. Actual IDF manager archive creation,
linked native-IDF/Arduino firmware, full hardened package matrix, the omission
checker/negative fixtures, remote CI and physical hardware remain NOT RUN/later
scope. Host sanitizers remain unavailable in the installed MinGW; no new sanitizer
pass is claimed. clang-format is unavailable on PATH; its simple config was
reviewed. Source-only register/variant/procedure guides needed no silicon changes.

Updated README, CHANGELOG, CONTRIBUTING, AGENTS, docs/examples indexes,
development/API/architecture/coverage, test/example guides and decision records.
The lead commits this complete checked unit, fetches/pushes the existing upstream
and verifies equality/clean task state. Final commit/sync evidence is in the
handoff and next progress update without a self-referencing hash edit.

## Prompt 8 independent re-audit

Completed 2026-10-26, starting at `af0d2d44b812dfda006e5513e18b6c669451bb6d` on
the same execution branch/base. Original prompt 8 was reread in full; actual
implementation diff `93f2d31374dc16bcc95800d0e0c516c4372faa4a..af0d2d4` and current
sources were reviewed independently. Initial worktree/index were clean; fetch
confirmed upstream equality and zero ahead/behind. The prior prompt 8 commit
was synchronized successfully; this audit starts no later implementation stage.

Delegated three disjoint read-only reviews: gpt-6.1-sol/high checked requirements,
documentation and scope; gpt-6-astra/high checked generator correctness, edge
cases, fixtures and simplicity; gpt-6.1-sol/high checked packaging, component
registration, launcher and tests-OFF portability. Each reread the applicable
rules and actual code/diff and ran independent checks. The lead verified the
findings, made all edits and owns Git. Current sibling revisions/clean states and
exact source choices were refreshed in [execution-8-peers](audit/execution-8-peers.md).

Confirmed and fixed three gaps: alternate-form duplicate managed declarations
could let version check return 0 despite an effective conflicting CMake version;
an inherited ERRORLEVEL environment variable could turn a failed launcher child
into success; and 25 relative links in exported guides targeted excluded source
files. Declaration counting now precedes canonical replacement, the launcher
locally clears CMD's environment shadow, and repository-only guide links use
existing source-host URLs. Corrected the stale changelog claim that version
tooling was still planned. No package version, public C++ API, source list,
dependency floor or silicon rule changed.

PASS: 18 isolated generator fixtures; two new Windows launcher fixtures with
16 status/shadow combinations and missing-install failure; Debug/Release 4/4
registered CTests; sync/check and no-op bytes/mtimes; strict Doxygen and visibility;
six CRT argument-forwarding comparisons; clean native PlatformIO compile/link/run;
real 29-file archive, all relative packaged-guide links resolved; renamed extracted
strict Release consumer beneath unrelated Git with maintainer tools absent from
child PATH and tests OFF; host-only component-registration fixture; local links,
Python docs/syntax/style, unchanged vendor hashes and scoped diff/whitespace.
Exact commands and evidence are in [validation](validation.md).

Real IDF archive tooling, framework firmware, hardware, full package matrix and
complete omission guard remain later scope. Existing unavailable compiler,
formatter and sanitizer limits remain explicit. No remote CI was dispatched or
polled. The lead commits and synchronizes these checked corrections on the same
branch; final commit/equality and clean-state evidence belong in the handoff and
next progress update.

## Prompt 9 execution record

Completed 2026-10-26 on the same execution branch/base, starting from synchronized
prompt 8 audit commit `d7fbd59a3604d1af70dc9c751b44fa55742fb878`. Initial worktree/
index were clean; fetch confirmed equal upstream and zero ahead/behind. Original
prompt 9, parent/current AGENTS, COMMON, baseline transport/status/capability/
accounting and current API/register/procedure contracts were reread. Prompt 10
remains separate; no public device facade or unfinished operation was introduced.

Fresh [execution-9-peers](audit/execution-9-peers.md) records PCA9555/ADS1115/
LDC1614 full revisions, clean states, exact source/example/runner/test/workflow
paths and adopted/rejected choices. Lead verified relevant actual implementations,
current TI source bytes and all eight archived hashes; read address/framing/map/
revision pages and visually inspected all four identity tables. Pinned Go/Rust/
Renode source hashes were re-fetched and their counterexamples reread. No source
was copied into core; no silicon gate or revision policy changed.

Delegation: gpt-6.1-sol/high owned only the peer audit, then reviewed package/
source/build/document boundaries read-only. Another gpt-6.1-sol/high owned only
the fixed scripted fake/protocol tests after shared declarations were frozen.
gpt-6-astra/high reviewed TI/error/possible-write/accounting contracts and actual
production/tests read-only, then independently built/ran the final registered
Release protocol suite. The lead owns production, build/package lists, integration,
docs and Git. No blocking reviewer issue remained; review improved timestamp/
identity/IRQ preservation cases, source citations and framing-only fixture wording.

Implemented private TUSB32xProtocol.h/.cpp with mapped single-callback reads/
writes, fixed scratch receive storage, one terminal normalization/accounting
boundary, explicit raw/managed classification, saturating counters, retained
failure evidence, clipped positive budgets and independent possible-write results.
Exact eight-byte identity and per-part revision comparisons are pure. All invoked
failed writes remain possibly accepted without phase evidence. TUSB322I 08/09,
absent registers and read-only targets remain gated; safe byte composition and
owner prerequisites are explicitly deferred to their owning implementation stages.

PASS: 161,461 active protocol checks, focused registered suite, strict Debug/
Release 5/5 CTests, independent strict Release and optimized NDEBUG direct builds,
five public-header TUs, existing validator/value/version/launcher suites, version
sync/check/no-op, strict Doxygen and public/private visibility, actual native
PlatformIO build/run, real 32-file archive with explicit matching component
selection, renamed extracted tests-OFF strict consumer beneath unrelated Git
without maintainer tools on child PATH, host-only component registration, local
links/anchors, source style/whitespace and unchanged vendor hashes. Exact tool
versions/commands/results are in [validation](validation.md).

ASan/UBSan remain BLOCKED by missing -lasan/-lubsan in installed MinGW; MSVC,
Clang and clang-format are unavailable on PATH. SDK framing/aggregate timeout,
real IDF archives, framework firmware, full omission/package guards and hardware
remain NOT RUN/later scope. No remote CI was dispatched or polled and no sibling
was changed. The lead commits/synchronizes this complete checked unit on the
same branch, verifies upstream equality and clean task state, and reports the
final commit in the handoff/next progress update without a self-reference loop.

## Prompt 9 independent re-audit

Completed 2026-10-26, starting at synchronized prompt 9 implementation commit
`84dd5c269318f50b377243843bdc9306590fe517` on the same execution branch/base.
Initial worktree/index were clean and fetch confirmed equal upstream, zero
ahead/behind. Reread the original prompt in full and inspected its actual diff
from `d7fbd59a3604d1af70dc9c751b44fa55742fb878`, current code/tests and applicable
contracts. No later prompt was started.

Three fresh read-only reviewers ran in parallel: gpt-6.1-sol/high checked
requirements, documentation, scope, build/package lists and simplicity;
gpt-6-astra/high checked implementation, TI rules, bounds, normalization,
possible-write and accounting semantics; gpt-6.1-sol/high checked test quality,
edge cases, Release behavior and current peer sources. Each inspected actual
files/diff and ran independent native checks. The lead verified the code,
evidence and results. Peer HEADs/clean states and adopted/rejected choices were
refreshed in [execution-9-peers](audit/execution-9-peers.md).

No confirmed functional defect, unmet original requirement, necessary test gap
or unnecessary abstraction was found. Production, tests, public API, source/
package lists and silicon rules remain unchanged. Only audit evidence and the
Unreleased audit note are updated.

PASS: fresh strict Debug/Release 5/5 CTests and 161,461 active protocol checks;
independent reviewer builds, direct optimized NDEBUG checks, 542 supplemental
edge checks and detection of all seven isolated intentional production mutants;
current TI downloads/archived hashes and exact pinned Go/Rust/Renode bytes;
strict Doxygen/public-private visibility, version check/no-op, clean native
PlatformIO, real 32-file package and renamed extracted tests-OFF consumer under
unrelated Git without maintainer tools on child PATH; fresh host-only component
registration, local links/anchors and scoped diff/whitespace. Detailed commands,
tool versions, evidence and limits are in [validation](validation.md).

The fresh sanitizer probe still fails for missing -lasan/-lubsan; unavailable
MSVC/Clang/clang-format and later SDK/hardware/firmware/IDF archive qualification
remain explicit limits. No sibling was changed and no remote CI was dispatched
or polled. Lead commits/synchronizes this checked audit on the existing branch
and verifies equality/clean state; final commit is reported in the handoff and
next progress update without a self-referencing hash edit.

## Prompt 10 execution record

Completed 2026-10-26 on the same execution branch/base, starting from prompt 9
audit commit `dd82a796e51dcfefa77db36a921d69aec0b12346`. Initial worktree/index
were clean; fetch confirmed equal upstream, zero ahead/behind. Reopened the full
prompt, parent/current AGENTS, COMMON, maintainability/workflow, baseline settings/
decoder/capability rules and current API/register/procedure contracts. Prompt 11
is not started. No device facade, raw setter or operation executor was added.

Fresh [execution-10-peers](audit/execution-10-peers.md) records exact clean peer
revisions and source/header/example/CLI/runner/test/workflow paths for PCA9555,
LDC1614 and TMP1x2. Adopted named masks, explicit variant/access gates, literal
expected bytes and unchanged-error outputs. Rejected GPIO-pair/sensor RMW,
zero-on-error outputs, CLI-local duplicate conversions and configure-time tools.
No sibling was edited or built; TUSB32x still has no workflows at this stage.

Lead re-downloaded B/L/A/V TI PDFs and matched archived SHA-256 hashes; reopened
the exact register/procedure pages and visually inspected A Table 10. The strong
reviewer independently reread B pp14,16-19, L/A pp14,17-20 and V pp9-11,17-20,
including visual V08 access/fault ambiguity checks. Lead re-fetched/hash-checked
the pinned TI-hosted Linux driver/regmap and Betrusted Rust source, tracing
mask08/value1 through val&mask and the reserved Try.SRC04 constant. These remain
counterexamples, not code or silicon authority. No capability gate changed.

Delegation: gpt-6.1-sol/high owned only the peer report, then reviewed actual
production/build/package/public linkage read-only and ran a separate strict
Release build. A second gpt-6.1-sol/high owned only test/test_codecs.cpp after
private signatures were frozen. gpt-6-astra/high independently reviewed TI,
W1C/reset, variant/stage ambiguity, production/tests and the final diff read-only,
and reran the registered Release protocol suite. Lead owns shared declarations,
production composition, integration, documentation, review and Git.

Implemented public pure decodeRegister, private typed persistent-field encoding,
fieldwise settings comparison and one safe write composer. The codecs share the
existing closed variant table with transport. Actual/control validity is separate
from desired settings; absent/reserved fields retain raw evidence and clear only
their flags. Explicit intent/stage proofs prevent ordinary mode/recovery shortcuts,
exclude live/reserved/IRQ/reset bits and retain equivalent 322I DRP00. All 322I
08/09 writes, absent45 and real non-reset return-to-DRP remain gated.

Review resolved three contract edges: ACK/reset accept fresh safe actual bytes
despite overall UNKNOWN; intrinsic intent/access errors precede missing actual
proof; the 322I DRP feature gate applies to ordinary requests too. Added focused
regressions for each, including 193 intrinsic cases under three missing-proof
combinations. Clarified prompt 10's detached wording against the frozen decoder:
single-byte decoding reports raw codes; complete NOT_APPLICABLE context is prompt 15.
DE->56 and protected45 preservation are arithmetic, not permission to write an
unresolved reset or unqualified internal image. No final reviewer blocker remains.

PASS: 8,602,548 independent active codec checks plus 161,670 protocol checks;
strict native Debug/Release 5/5 CTests, focused suite and independent strict/
optimized NDEBUG runs; standalone public-header linkage, updated multi-TU host
consumer with zero fake attempts, native PlatformIO, explicit three-source host
component fixture and real 34-file archive. A fresh renamed extracted tests-OFF
consumer beneath unrelated Git builds/runs without maintainer tools on child PATH.
Version sync/check/no-op, strict Doxygen/public visibility, source/docs/links and
all eight vendor hashes pass. Exact commands/tools/results are in [validation](validation.md).

ASan/UBSan remain BLOCKED by missing MinGW -lasan/-lubsan; MSVC/Clang/clang-format
are unavailable on PATH. Owner jobs, actual wait timing, complete port decoding,
framework firmware, SDK framing/timeout and physical hardware remain later work.
No remote CI was dispatched/polled. Lead commits/synchronizes this complete checked
unit on the established branch and verifies upstream equality/clean state; final
commit is reported in the handoff and next progress update without a hash loop.

## Prompt 10 independent re-audit

Completed 2026-10-26, starting at synchronized implementation commit
`80995089454db7bd4cc4f28e1c66c62a98469266` on the same execution branch/base.
Initial worktree/index were clean; fetch confirmed equal upstream, zero
ahead/behind. Reread the complete original prompt from `dd82a79` and reviewed
the actual `dd82a79..8099508` diff, current production/tests and frozen contracts.
No later implementation prompt was started.

Three fresh reviewers ran in parallel with disjoint scopes: gpt-6.1-sol/high
checked requirements, scope, simplicity, public linkage, build/package and docs;
gpt-6-astra/high checked TI, W1C/reset, variant ambiguity, actual-state/stage
proofs and output preservation; another gpt-6.1-sol/high checked test independence,
edge cases, Release execution and current peer sources. Each inspected actual
files and ran independent native checks. The lead verified their findings against
code, diff and source evidence. The last reviewer updated only the dated
[peer audit](audit/execution-10-peers.md); the lead owns integration and Git.

No confirmed functional defect, missing original requirement, necessary test gap
or unnecessary abstraction was found. Production, tests, public declarations,
source/package lists and silicon gates remain unchanged. The apparent original
DE/reset, normal45 and detached-direction conflicts were checked against contracts
already present at `dd82a79`: byte arithmetic does not authorize an unresolved
reset/unqualified internal image, and the frozen single-byte decoder reports raw
codes while complete paired applicability belongs to prompt 15. This audit did not alter
those prerequisites or use rewritten prompt text as its acceptance evidence.

README, AGENTS, CONTRIBUTING, API/architecture, register/variant/device guides,
feature/source/test matrix and Doxygen were reviewed and remain accurate; no
behavior or command changed. Only audit/progress/validation and the Unreleased
review note are updated. Fresh TI downloads and pinned Linux/regmap/Rust bytes
match their recorded hashes; actual tables and counterexample helpers were reread.

PASS: fresh strict Debug/Release 5/5 CTests; 8,602,548 codec and 161,670 protocol
checks, independent optimized NDEBUG execution, public tests-OFF consumers,
strict Doxygen/visible decoder, version check/no-op, clean native PlatformIO,
real 34-file package with a renamed extracted consumer under unrelated Git and
no maintainer tools on child PATH, fresh host-only component registration,
links/anchors/whitespace and all eight vendor hashes. Exact tools, commands and
limits are in [validation](validation.md#prompt-10-independent-audit-2026-10-26).

Fresh ASan/UBSan probe remains BLOCKED by missing MinGW -lasan/-lubsan; MSVC,
Clang and clang-format remain unavailable on PATH. Later owner timing, paired
port interpretation, firmware and hardware remain NOT RUN. No sibling was edited
or remote CI dispatched/polled. Lead commits and synchronizes this checked audit
on the existing branch, verifies equality and clean state, and reports the final
commit in the handoff/next progress update without a self-referencing hash edit.


## Prompt 11: owner engine (2026-10-26)

Started clean at `fe1f7caf732b723734d1b87ee50252b09d6928fe`, synchronized with
origin on the recorded execution branch. That commit completes the independent
prompt-10 audit. Re-read prompt 11, root/parent rules, COMMON, the baseline and
API/procedure contracts. Current peer paths, HEADs, dirty state, source/tests,
examples, runners and registration are in [execution-11-peers](audit/execution-11-peers.md).
Siblings remained read-only; no peer builds, fetches or remote CI checks.

Implemented the real non-copyable facade with atomic binding, local end/external
invalidation/forgetting, token-checked initialization/poll/cancel, exact retained
result delivery, cached evidence and all-source statistics clearing. The single
engine shares the existing terminal transport and pure decoder, has named private
phases, fixed storage, no retry/replay or initialization writes, and one callback
per owner turn. Tokens never reset or wrap; unresolved target association includes
both callbacks even with null context. Late successful reads need fresh owner
time before publication; actual callback errors retain their original detail.

Complete raw identity/profile proof establishes initialized usability independently
of configuration trust. Raw/history validity is not current authorization.
Termination uses its own disable-control proof. Existing effects and pending reset
history survive teardown/rebinding/result consumption. Later operations are not
published or replaced by stubs. The compile-only future fixture now derives a
separately named planned type from the delivered facade.

Delegation: gpt-6.1-sol/high owned current peer research and general docs; another
owned operation tests/registration and future fixture migration. gpt-6-astra/high
independently reviewed time, tokens, association, evidence and scope read-only.
The lead owned production/build/package changes, integrated and inspected all
returned diffs, and ran final checks. Review found and fixed two gaps before
completion: unrelated reserved profile fields had erased independent termination
proof, and matching identity segments had published before fresh time validation.
Regression cases cover both plus contradiction-at-deadline priority.

Updated public Doxygen, API/ownership/architecture/feature documentation, README,
CHANGELOG, contributor/test/agent guidance and package source selections. Strict
package verification found and corrected local links to repository-only audit/
validation pages. Preserved the old behavioral-check anchor. No version bump.
Register encodings, variant gates and hardware procedures are unchanged.

PASS: Debug and Release 6/6 registered CTests each; 20,138 operations checks,
161,670 protocol checks and 8,602,548 codec checks. Strict header/future fixture
compilation, public method linkage, Doxygen and visibility of all 15 delivered
owner methods, no-op version sync, local links/anchors, native PlatformIO and
36-file extracted package consumption also pass. Detailed commands and limits
are in [validation](validation.md#prompt-11-owner-engine-2026-10-26).

No firmware/hardware or later-operation qualification is claimed. The installed
MinGW sanitizer runtimes remain unavailable; no sanitizer pass is claimed. Lead
commits/pushes the complete checked unit on this same branch and verifies upstream
equality and a clean worktree before handoff. The final commit belongs in the
handoff and next progress update, avoiding a self-referencing hash-edit loop.


## Prompt 11 independent audit (2026-10-26)

Started clean at synchronized `2dd43046cdfe7f2eed92d8fa372e27305cad58f4`, the
completed prompt-11 implementation, on the existing execution branch. Reread
original prompt 11 in full and reviewed its actual `fe1f7ca..2dd4304` diff,
current production/tests and frozen contracts. The selected IDE prompt 16 was
not executed. Three fresh parallel reviewers covered requirements/simplicity/
public consumption (gpt-6.1-sol/high), timing/effects/correctness (gpt-6-astra/high),
and tests/current peers (gpt-6.1-sol/high). Lead independently checked their
source findings, diffs and probes, and ran the final integration checks.

No production defect was found. Corrected bind's Doxygen: binding validation
returns INVALID_CONFIG, not UNSUPPORTED_FEATURE. Added a missing persistent
regression across all seven variants: observed reset, end/same-target rebind,
then a complete clear-register initialization must retain MAY_BE_PENDING and
UNKNOWN. Current identity usability may still become initialized; changing the
unresolved target remains rejected. The engine already met this contract.
No public signature, runtime behavior, chip gate or build/package rule changed.

The [fresh peer review](audit/execution-11-peers.md#independent-implementation-recheck-2026-10-26)
records unchanged driver revisions and FieldCore's now-dirty unrelated worktree;
all sibling work was preserved. Updated source/test documentation, Unreleased,
progress and validation. Reviewed README, AGENTS, CONTRIBUTING, API/architecture,
feature matrix and TI guides: their implemented scope and contracts remain
accurate, so no incidental edits were needed.

PASS: final fresh-tree Debug/Release 6/6 CTests each; 20,490 owner checks,
161,670 protocol and 8,602,548 codec checks; 371 additional public-path probes;
strict Doxygen/all 15 public owner methods visible; source-equal extracted
36-file package/ordinary consumer, version/no-op and links/whitespace checks.
The independent extracted public consumer also calls all 15 methods and completes
initialization. Exact commands/limits are in
[validation](validation.md#prompt-11-independent-audit-2026-10-26).
No firmware/hardware or later-operation evidence is claimed. Sanitizers remain
unavailable from the recorded installed-tool probe; not rerun for this comment/
regression-only change. Lead commits and synchronizes the checked audit on this
branch and reports final commit/equality/clean state in the handoff, recording
the commit in the next progress update without a self-referencing edit loop.

## Prompt 12: profile observation and read-only reconciliation (2026-10-26)

Started clean at synchronized `921132461e676ea2f05f7c4e6ed88edc641a294d`, the
completed independent prompt-11 audit. The execution branch/base remain unchanged.
Reread prompt 12, instructions, frozen profile/generation/effect contracts and
the TI proof matrix. Current peer declarations, implementation, tests, examples,
CLI/runners and registration were re-audited in
[execution-12-peers](audit/execution-12-peers.md). No sibling modifications,
fetches, builds or remote CI inspection occurred.

Implemented `startReadProfile` and `startReconcile` in the existing cooperative
engine. Shared admission, scalar observation and semantic comparison preserve
tokens, deadlines, one callback per turn and one retained result. P publishes
complete raw/typed observations without trust promotion or desired adoption;
F proves actual identity/configuration and required effects, restoring initialized
usability after external invalidation without a write. Reserved fields and
unresolved reset/recovery evidence remain explicit. Pending reset uses one
conservative wait and one F, never a replay or retry. Future apply supplies desired
input and owns its mismatch case in prompt 13; no fake setter was introduced.

Delegation: gpt-6.1-sol/high agents owned peer/general documentation and focused
tests in disjoint files. A gpt-6-astra/high reviewer independently checked proof,
time, effects and simplicity read-only. Lead owned shared production/publication
helpers, resolved findings, inspected returned code and tests, and ran integrated
verification. Review caught a stale independent termination proof after a changed
disable bit followed by cancellation/read failure; the regression failed before
the fix and passed after. Matching identity now also preserves an older complete
revision if the subsequent reconciliation revision read fails. Reset settling
proof is kept separate from unrelated reserved profile fields.

Updated public Doxygen and API/ownership/procedure/architecture/variant/feature
guides, metadata descriptions, README/CHANGELOG and test/contributor/agent guidance.
The procedure guide names actual helpers and exact callback/TX/RX ceilings.
Two real methods leave the planned fixture; no future operation is declared in
production. No version bump, new core file, framework dependency or chip write
permission was added. Source provenance remains unchanged.

PASS: Debug and Release 6/6 CTests each, with 34,042 new profile checks and
20,490 existing owner checks. Strict native suites, previous protocol/codec regressions, public
linkage, Doxygen visibility, native PlatformIO, source-equal extracted package,
tests-OFF consumer and documentation/version checks are recorded in
[validation](validation.md#prompt-12-profile-observation-2026-10-26).
Hardware/framework firmware remains separate and untested. Installed MinGW
sanitizer runtimes remain unavailable from the recorded same-day probe; no
sanitizer pass is claimed. Public apply/mode/recovery-write cases remain their
owning later prompts, not uncompleted read-only work.

Lead commits and synchronizes this complete checked feature on the same branch,
then verifies local/upstream equality and clean task state. Final commit evidence
belongs in the handoff and next progress update, not a self-referencing hash edit.

## Prompt 12 independent audit (2026-10-26)

Started clean at synchronized `db6650064690883a57c60ac6b349edffe495df61` on
the existing execution branch. Reread original prompt 12 in full and reviewed
the actual `9211324..db66500` diff, current implementation, tests and contracts.
The selected IDE prompt 16 was not executed. Three fresh-context parallel
reviewers checked proof/time/effects (gpt-6-astra/high), requirements/scope/
documentation/package simplicity (gpt-6.1-sol/high), and tests/current peers
(gpt-6.1-sol/high). Lead inspected the actual probes and source evidence and
repeated the integrated checks.

No runtime defect was found. The confirmed gap was operation-level publication
coverage: pure decoder tests covered every encoding, but owner tests did not
directly verify every nondefault field after the complete P/F assembly. Added
compact literal-vector tests for all seven variants, both read-only operations,
every valid setting value, raw/typed validity, independent termination and
generation/trust behavior. Expected profiles come from explicit semantic tables,
not the production decoder. No production behavior or API changed.

Independent public probes exercised 6,912 single-register byte scenarios and
43,008 complete valid-profile plans. Both passed without private-state access
or write permission. Fresh peer checks confirm the same revisions and clean
states recorded in [execution-12-peers](audit/execution-12-peers.md). No sibling
was changed, fetched or built. Desired mismatch and actual interrupted write
histories remain explicitly assigned to prompts 13-14.

Updated test documentation, Unreleased, progress, peer evidence and validation.
Reviewed the API/ownership/procedure/variant/architecture guides and source
Doxygen; they remain accurate without incidental edits. Exact final commands,
counts and limits are in
[validation](validation.md#prompt-12-independent-audit-2026-10-26).
Lead commits and synchronizes this checked audit on the same branch, verifies
upstream equality and a clean worktree, and reports the commit in the handoff.

## Prompt 13: explicit profile application and verification (2026-10-26)

Started clean and synchronized at `724b7668aa47a6d927e6e5521e84143fe0295a2b`,
the completed prompt-12 independent audit, on the same execution branch.
Reread prompt 13 and the profile/effect/clock contracts. Current ADS1115,
LSM6DS3TR, INA3221 and PCA9555 source, tests, examples, runners, instructions and
registration were re-audited; exact revisions and choices are in
[execution-13-peers](audit/execution-13-peers.md). Fresh official TI downloads
matched the four archived datasheets byte-for-byte. No sibling was changed,
fetched or built; no remote CI gate was added.

Implemented `startApplyProfile` and `startVerifyProfile` in the existing engine.
Admission validates and composes at most six writes before retaining desired
input. Actual controls supply preserved fields; only changes are written, with
separate per-variant mode procedures and complete F readback. Extended 320 waits
six elapsed ticks from fresh post-mode time; base/322 inherit no added wait.
Unchanged gated 322 controls permit other qualified writes. No-op apply and
verify are read-only. Complete mismatch proves KNOWN actual independently of
desired and permits later explicit application after reconciliation.

Two gpt-6.1-sol/high agents owned disjoint tests and peer/general documentation;
a gpt-6-astra/high reviewer checked timing, uncertain effects, proof and
simplicity independently. Lead integrated shared source and reviewed the actual
diff and test/probe code. Review caught two gaps: interrupted base mode writes
must retain mode proof, and active apply must not overwrite terminal diagnostic
effect/kind/status. Both regressions failed before correction and pass afterward.
No retry, cleanup, rollback, extra engine or public test hook was introduced.

PASS: all six CTests in Debug and Release; 87,978 application/verification,
35,454 profile-observation and 20,490 owner checks; prior protocol/codec suites.
Independent literal-byte public probe: 46,592 valid old/new mode/control pairs
across seven variants. Doxygen, native PlatformIO, source-equal 36-file archive,
tests-OFF tool-independent consumer, links/provenance and version checks passed.
Exact commands, correction history and limits are in
[validation](validation.md#prompt-13-profile-application-2026-10-26).

Updated API/member Doxygen, architecture/ownership/procedure/feature/variant
guides, metadata descriptions, README/CHANGELOG and contributor/test guidance.
Only the two implemented methods leave planned fixtures. No version bump,
future operation declaration or firmware/hardware qualification was added.
Physical timing, cable interlock and electrical/VCONN qualification remain
external; installed MinGW sanitizer runtimes are unavailable as already probed.

Lead commits and synchronizes this complete checked feature on the same branch,
then verifies upstream equality and clean task state. Commit evidence belongs
in the final handoff and next progress update, avoiding a self-referencing edit.

## Prompt 13 independent audit (2026-10-26)

Started clean and synchronized at `7ea5583c52407e0e755dc89e185db1eab7512238`
on the existing execution branch. Reread the full original prompt 13 and actual
`724b766..7ea5583` diff, current source/tests and contracts; the selected IDE
prompt 16 was not executed. Three fresh-context parallel reviewers checked
clock/effect/proof correctness (gpt-6-astra/high), test coverage and independent
failure probes (gpt-6.1-sol/high), and requirements/simplicity/docs/package/current
peers (gpt-6.1-sol/high). Lead inspected the source, probes and returned changes
and repeated verification in a fresh build tree.

No production defect was found. Confirmed regression gaps were successful UFP
role changes and ordinary 0A updates without a mode change. The original
permanent suite exercised DFP/DRP, while UFP was only staged and cancelled;
settings-only 320 control writes lacked direct positive coverage. Isolated
mutations that corrupt UFP writes or omit standalone termination-disable writes
survived the original operation suite. Added compact independent literal cases
to the existing application tests, preserving the existing engine and fixture.

Rechecked all four peers at the same revisions and clean main branches recorded
in [execution-13-peers](audit/execution-13-peers.md); the named code/tests/examples/
runners/instructions/registration still support the selected conventions.
No sibling changes, fetches or builds occurred. Public API, production code,
TI gates, source lists and framework boundaries remain unchanged.

Updated test documentation, Unreleased, peer audit evidence, progress and
[validation](validation.md#prompt-13-independent-audit-2026-10-26). Final checks
include fresh strict Debug/Release CTests, Doxygen visibility, documentation/
provenance/version checks and extracted ordinary package consumption. Hardware,
firmware and unavailable sanitizer runtimes remain separate evidence limits.
Lead commits and syncs this checked audit on the same branch, then verifies
upstream equality and a clean worktree; final commit evidence is in the handoff.

## Prompt 14: reset and original-device recovery (2026-10-26)

Started clean and synchronized at `a6bcccfe3353461fcc8ee838fcdb0f946a81e821`
on `promptset/tusb32x-implementation-20261023`. Re-read the original prompt,
COMMON, owner/time/effect baseline and TI procedures. Current sibling evidence
and byte-identical official TI downloads are recorded in
[audit](audit/execution-14-peers.md). No sibling writes or remote CI checks.

Implemented `startReset` and `startRecoverDeadBattery` in the existing engine.
Reset composes from fresh actual controls, attempts once, waits 96 fresh-anchored
ticks and performs bounded identity/control proof. Base-only recovery performs
R45/literal04/31-tick wait/literal00/R45 with four callbacks. Both preserve desired,
original backend errors, historical evidence and terminal result ownership.
There is no automatic replay, cleanup, port publication or new engine.

Parallel general coding agents owned disjoint tests and guides/peer audit; a
strong reasoning reviewer independently checked TI, time/effects and an exhaustive
public-API probe. The lead reviewed the actual source, tests and diff. Review
caught a false COMPLETED possibility when an unrelated reserved field hid a valid
non-DRP 322I reset result; fixed independent field validity and error precedence.
Tests also caught missing cached WAITING state before the fresh wait anchor.
A fixture's reserved preference byte was corrected from valid36 to reserved34.
No expected silicon behavior was weakened to make tests pass.

PASS: Debug/Release six CTests each; 86,298 new reset/recovery assertions;
previous owner/profile/protocol/codec suites; strict independent 1,792 command
and 512 post-reset vectors; Doxygen with 21 delivered methods; native PlatformIO;
source-equal 36-file package and tool-independent tests-OFF consumer;
links/provenance/version checks. Exact commands and limits are in
[validation](validation.md#prompt-14-reset-and-original-device-recovery-2026-10-26).

Updated public Doxygen, API/architecture/procedure/variant/feature guides,
metadata descriptions, README/CHANGELOG, contributor/agent/test guidance and
planned fixtures. No version bump or future callable declaration. Hardware,
firmware and unavailable sanitizer runtimes remain separate evidence limits.
Lead commits and syncs this completed checked unit on the existing branch and
verifies upstream equality/clean state; final commit evidence is in the handoff.

## Prompt 14 independent audit (2026-10-26)

Started clean and synchronized at `a33af83acdb5a92cc6e95bef4e0cbc747232cf07`
on the existing execution branch. Re-read full prompt 14 and actual
`a6bcccf..a33af83` diff. Three fresh-context parallel reviewers checked production
correctness/time/effects (gpt-6-astra/high), test gaps and independent probes
(gpt-6.1-sol/high), and requirements/TI/current peers/docs/simplicity
(gpt-6.1-sol/high). The selected IDE prompt 16 was not executed.

No production behavior defect was found. Confirmed gaps were a reset regression
that did not distinguish cached from fresh debounce controls, missing focused
post-reset identity/revision contradiction and callback clipping cases, stale
class Doxygen claiming operations besides apply only read, and stale top-level
progress authorization/status. Corrected documentation and added small literal
public-operation tests using the existing fixture. No engine refactor, new API,
framework dependency or silicon-policy change was needed.

Lead inspected and reran the mutation evidence: a faulty implementation sourcing
reset debounce from cached proof passed the original 86,298-check suite, but the
new cases reject it on all seven variants (21 failed assertions). The unchanged
real engine passes 88,779 reset/recovery assertions. A separate probe passed
1,825 additional checks, including zero-budget pending-completion processing.

Current peer revisions/main/clean states and adopted/rejected rules remain as
recorded in [execution-14-peers](audit/execution-14-peers.md); named source/tests,
examples/runners/registration/instructions and exact archived TI pages were
reopened. No sibling mutation or remote CI inspection occurred.

PASS: fresh strict Debug/Release six CTests each, previous owner/profile/protocol/
codec suites, Doxygen visibility, links/provenance/version checks and source-equal
package consumption without maintainer tools. Commands and evidence limits are in
[validation](validation.md#prompt-14-independent-audit-2026-10-26). Hardware and
firmware remain unqualified; installed sanitizer runtimes remain unavailable.
Lead commits/syncs the checked audit on this same branch and verifies equal
upstream commits and a clean worktree. Final commit evidence is in the handoff.

## Prompt 15: complete port observations (2026-10-26)

Started clean and synchronized at `caba75b21bab02e5bdc7bff47c611f797b2ef3dc`
on the existing execution branch. Completed prompt 15 only; IRQ remains prompt16.
Re-read full prompt, COMMON, baseline and exact archived TI status/applicability
pages. Current BME280/LDC1614/INA228 evidence and adopted/rejected behavior are in
[execution-15-peers](audit/execution-15-peers.md). No sibling changes or remote CI.

Implemented `startRefreshPort` and complete cached publication in the existing
engine. Fixed09/08/09 reads use pure codecs; only complete fresh-time publication
replaces history. Final09 and E0 bookends give independently qualified fields;
no retry, ACK/reset, GPIO or power policy. Acquisition time/configuration provenance
precede the first read; sequence saturates. Negative current control evidence can
revoke configuration trust while incomplete refresh preserves the old port record.
Conservative accessory applicability and raw322 fault limits remain explicit.

A general coding agent owned disjoint tests, another owned peer/docs work, and a
strong reasoning reviewer checked TI/production/edge cases independently. Lead
reviewed actual changes, fixed a strict flag-OR warning, checked fixture corrections
and ran the independent all-bookend probe. Added an explicit independent CC/VBUS
fixture and corrected overly broad322 debug wording; no remaining behavior defect.

PASS: strict Debug/Release seven CTests each; 494,257 port checks; unchanged prior
owner/profile/reset/protocol/codec suites; independent13,031,431 checks over458,752
observations; Doxygen22 owner methods; native PlatformIO; source-equal36-file
package/tool-independent consumer; links/provenance/version checks. Exact commands
and separate firmware/hardware/tooling limits are in
[validation](validation.md#prompt-15-complete-port-observations-2026-10-26).

Updated source/public Doxygen, applicability/procedure/API/architecture/variant/
feature guides, metadata descriptions, README/CHANGELOG and contributor/test/agent
instructions. No version bump, new production file or future callable API.
Lead commits and syncs this checked unit on the same execution branch, verifies
upstream equality and a clean worktree, and reports the final commit in handoff.

## Prompt 15 independent audit (2026-10-26)

Started clean and synchronized at `c9b43d23f88af70139947d9399e92edcba283d51`
on the existing execution branch. Re-read full original prompt 15 and actual
`caba75b..c9b43d2` diff. Three fresh-context parallel reviewers inspected engine/
time/evidence (gpt-6-astra/high), test coverage and mutation probes
(gpt-6.1-sol/high), and requirements/TI/current peers/documentation/simplicity
(gpt-6.1-sol/high). Prompt 16 was not executed.

No production defect was found. Added a missing regression for a persistent
control contradiction first observed in final09. It verifies immediate UNKNOWN
configuration through fresh completion, cancellation, expiry and rejected backward
time, while historical port evidence and acquisition provenance remain intact.
An isolated mutant skipping final09 control observation passed all 494,257 old
port checks; the added cases reject it with 63 failures. The real engine passes
495,667 port checks. Lead inspected the mutation and reproduced both outcomes.

Corrected stale documentation that deferred implemented port/owner support or
claimed no read-only operation clears port invalidation. Clarified that current
refresh publishes IRQ only in PortSnapshot; InterruptSnapshot ACK/service history
belongs to prompt 16. Dated the inherited official-download claim explicitly.
No new runtime API, refactor, dependency or silicon policy was needed.

Reopened peer/TI sources confirm the existing BME280/LDC1614/INA228 revisions,
clean states and selected conventions in [execution-15-peers](audit/execution-15-peers.md).
PASS: fresh strict Debug/Release seven CTests each, affected/prior suites, separate
1,326-check public probe, Doxygen visibility, links/provenance/version checks and
extracted source-equal package consumption without maintainer tools. Exact commands
and remaining hardware/tooling limits are in
[validation](validation.md#prompt-15-independent-audit-2026-10-26).
Lead commits and syncs this checked audit on the same branch and verifies upstream
equality/clean state; final commit evidence is in the handoff.

## Prompt 16: explicit interrupt acknowledgement and service (2026-10-26)

Started clean and synchronized at `e964fe5d584100e9e98c3a20f09e5cd38b673f1b`
on the existing execution branch. Completed prompt 16 only. Re-read the full
prompt, baseline IRQ/effect/port/owner contracts and exact TI W1C/fault pages.
Current INA228/PCA9555/LDC1614 sources, clean revisions, instructions, examples,
CLI/runners and workflow definitions are recorded in
[execution-16-peers](audit/execution-16-peers.md). Siblings remained read-only;
remote CI remains prompt 34 work.

Implemented `startAcknowledgeInterrupt` and `startServiceInterrupt` with the
existing engine, protocol wrapper and safe composer. ACK uses fresh actual
controls and two callbacks. Service shares complete port publication, performs
one ACK and one recheck, within five callbacks. Pending-before, write attempt,
callback acceptance and sampled-after remain independent. Complete pre-ACK data
survives later failure; no retries, GPIO, queue or automatic follow-up is added.
TUSB322I ACK/service remain gated by unresolved fault/09-write semantics.

Parallel agents owned disjoint tests and peer/documentation work; an independent
reasoning reviewer checked silicon, production and an exhaustive public probe.
The lead reviewed actual code/tests/diffs and reran the probe. Review found no
remaining production defect. Corrected fixture assumptions about incomplete
service publication and contrary advertisement evidence, plus stale changelog
wording. Doxygen now distinguishes each operation's IRQ record from retained
diagnostic history. No new production file, dependency or version bump was needed.

PASS: strict Debug/Release seven CTests each, 67,081 IRQ checks plus 495,667 port
checks, previous owner/profile/reset/protocol/codec suites, independent 66,048
checks across 3,072 IRQ operations, Doxygen visibility for 24 implemented owner
methods, native PlatformIO and a source-equal 36-file package consumer without
maintainer tools. Exact commands and evidence limits are in
[validation](validation.md#prompt-16-explicit-interrupt-service-2026-10-26).
Physical IRQ wiring/levels/timing and framework firmware remain unqualified.
The lead commits and syncs this checked unit on the same branch, verifies upstream
equality and clean state, and reports the final commit in the handoff.

## Prompt 16 independent audit (2026-10-26)

Started clean and synchronized at `671134de40d90a0ebb3c6e30ca7eb54a11470c4d`
on the existing execution branch. Re-read full prompt 16 and actual
`e964fe5..671134d` diff. Three fresh-context reviewers checked engine/time/effects
(gpt-6-astra/high), test oracles and mutation evidence (gpt-6.1-sol/high), and
requirements/TI/current peers/documentation/simplicity (gpt-6.1-sol/high).
The lead verified findings against source and reran the independent probes.

No production defect was confirmed. Added missing service regressions for ACK
composition from final09 controls when bookends differ, and changed-bookend
field validity through successful service and ACK/recheck failure. Two isolated
faulty engine copies passed the old suite: one reused first09 controls, the other
made service bookends always match. The new cases reject them with 240 and 144
failures respectively. The real unchanged engine passes 71,617 IRQ checks and
495,667 port checks. Corrected stale delivered-API scope and the test-file inventory.

Reopened exact TI IRQ/fault pages and current INA228/PCA9555/LDC1614 sources
confirm the revisions, clean states and choices in
[execution-16-peers](audit/execution-16-peers.md). No sibling changes or remote CI.
PASS: fresh strict Debug/Release seven CTests each, prior operation/protocol/codec
suites, 144 independent failed-ACK lifecycle cases, Doxygen visibility, links/
provenance/version checks and source-equal extracted package consumption.
Commands and hardware/tooling limits are in
[validation](validation.md#prompt-16-independent-audit-2026-10-26).
No new API, dependency, runtime refactor or silicon-policy change was needed.
The lead commits and syncs this checked audit on the same branch and verifies
upstream equality/clean state; final commit evidence is in the handoff.

## Prompt 17 execution record (2026-10-26)

Started at synchronized `df0fc2253cb01cfcfdb0973d14791b7d345f5a95`, the completed
prompt16 independent audit, with clean index/worktree and zero ahead/behind after
fetch. Same execution branch/base. Read full prompt17, parent/local AGENTS, COMMON,
workflow/maintainability, baseline Sync/accounting/health, access/capability matrix,
public API, current sources/tests/examples and package/build definitions. No local
workflow exists; remote CI remains prompt34 only.

Fresh current peer revisions, dirty states, exact declarations/code/test/CLI/
runner/build/workflow paths and adopted/rejected units are in
[execution-17-peers](audit/execution-17-peers.md). The lead spot-checked callback
wrappers and synthetic mismatch accounting against actual sources. Reopened TI
B SLLSEN9F Table7-7 pp17–19, L/A SLLSEP2D/SLLSEQ8D Tables8–11 pp17–20, and V
SLLSEO5D Tables6-8–6-10 pp17–20. Current official PDF downloads match all four
archived hashes. Unresolved V advertisement/fault access remains gated.

Delegation: gpt-6.1-sol/high independently audited peers and then runtime/tests;
another gpt-6.1-sol/high owned only `test/test_diagnostics.cpp`. gpt-6-astra/high
reviewed raw-write/effect/variant safety read-only. Lead owns production/shared
headers, docs/build/package integration and Git. No sibling or agent Git mutation.

Implemented raw facade methods through the existing accounting boundary/composer,
pure health functions and complete copied views. One private45 write-image validity
bit avoids partial profile publication or blocking independent fields. Safety review
confirmed and fixed typed45 apply ignoring revocation after raw contradiction; its
regression failed before the fix and passes afterward. Removed the now-empty
future facade sketch and use delivered declarations in compile-only fixtures.
See design decisions and validation for current proof/clock/effect limits.

PASS: strict Debug/Release eight CTests each, 55,218 diagnostics checks plus all
prior suites; strict Doxygen with 17 public structs/26 owner methods and real
health functions visible; version sync/check/no-op, local links/anchors/UTF-8/LF,
source-equal38-file archive and independent tests-OFF consumer beneath unrelated
Git without maintainer tools on child PATH. Standalone ordinary/native PlatformIO
consumers and host-only five-source component registration also pass. Final review
added runtime evidence that differing staged duty/accessory settings never enter
raw ACK composition. No remaining confirmed production issue.

Fresh sanitizer link probes fail because installed MinGW lacks libasan/libubsan;
MSVC/Clang/clang-format remain unavailable. Physical hardware and framework firmware
are NOT RUN; remote CI remains prompt34. Exact commands/results and limits are in
[validation](validation.md#prompt-17-raw-diagnostics-and-passive-health-2026-10-26).
The lead commits and syncs this complete checked unit on the same branch, verifies
upstream equality/clean state, and reports final commit/sync in the handoff and
next progress update. Prompt18 remains NOT STARTED.

## Prompt 18 protocol conformance (2026-10-27)

Started clean at `8507681e92938b3eb8d0a7aa7308a1c8318ecbaf`, the completed prompt 17,
on the recorded execution branch with zero ahead/behind after fetch. Re-read the
full prompt, shared instructions, implemented contracts/source/tests and current
TI register/effect evidence. The [fresh peer audit](audit/execution-18-peers.md)
records exact revisions, clean states, source/test/example/runner/build paths and
adopted/rejected patterns for LDC1614, LSM6DS3TR, PCA9555 and ADS1115. No sibling
changes or remote CI requests.

Delegation: gpt-6-astra/high independently reviewed protocol/effect safety;
gpt-6.1-sol/high audited tests and owned the disjoint protocol/address regressions;
another gpt-6.1-sol/high owned the peer report and conformance matrix. The lead
reviewed actual changes, expanded raw failure cases, integrated documentation,
ran final checks and owns Git. No production defect was confirmed.

Closed missing public high-address coverage for all seven configured variants,
both possible raw-write/ACK failure outcomes and unrelated-read trust retention.
The fixed fake now rejects a short receive falsely marked OK. Kept existing
operation interruption/race tests and mapped real runtime names in
[protocol conformance](protocol-conformance.md), without a second simulator or
duplicate suite. Corrected a test comment that implied unproved 322I fault-read
non-consumption. No production behavior, API, dependency or silicon gate changed.

PASS: focused checks; full strict Debug and Release eight CTests each; strict
Doxygen and visibility; source links/anchors, eight TI hashes and no-op version
sync; source-equal 38-file package and renamed tests-OFF Release consumer beneath
unrelated Git without maintainer tools on child PATH. A deliberately wrong
high-address mutation passes the old operations suites but triggers 112 new
assertion failures. Exact commands/counts are in
[validation](validation.md#prompt-18-protocol-conformance-2026-10-27).

ASan/UBSan are BLOCKED by missing MinGW runtimes; hardware, framework firmware
and native PlatformIO execution were NOT RUN in this test/documentation unit.
Existing TI/board gates remain unresolved. The lead commits and syncs this checked
unit on the same branch, verifies upstream equality and clean state, and reports
the final commit in the handoff/next progress update. Prompt 19 remains NOT STARTED.

## Independent prompts 16-18 audit (2026-10-27)

Started clean and synchronized at `91e8eb4a1b070c04ca5a62453916978a72da578d`, the
completed prompt18, on the same execution branch. Re-read all three original
prompts in full and checked the implemented source, tests, baseline, TI evidence
and diffs rather than adopting the prior summary. Two gpt-6-astra/high reviewers
independently checked IRQ and raw/effect/health correctness; gpt-6.1-sol/high
checked coverage, oracles, fake behavior and simplicity. Tracked test ownership
was disjoint; the lead owns production fixes, documentation, checks and Git.

Fresh sibling states are unchanged and clean on main: INA228 b28c5ea, PCA9555
498178e, LDC1614 1db38b6, LSM6DS3TR 362b140, TMP1x2 bafc340, ADS1115 235af3e and
INA3221 41b6184. Relevant declarations, implementations and runtime registrations
were revisited; the exact paths and accepted/rejected patterns in
[execution16](audit/execution-16-peers.md),
[execution17](audit/execution-17-peers.md) and
[execution18](audit/execution-18-peers.md) remain applicable. No peer edits,
fetches or builds. TI B/L/A IRQ wording, V Table6-9/6-10 and V DFP/UFP recipes
were reopened. Physical effects and unresolved gates remain separate evidence.

Confirmed and fixed stale 322I mode-image permission after a raw/partial
observation of equivalent DRP00/11 bits. Semantic KNOWN state stays intact;
contradicted literal 0A images cannot authorize raw or typed writes until full
proof. A bounded independent probe reproduced the pre-fix write; new registered
regressions fail against the old source and pass after the fix. No-op proof and
explicit fresh-precheck reset remain available. Closed distinct 322I mode-failure
and cancellation/deadline coverage gaps. IRQ behavior and health needed no fix.

PASS: full strict Debug and Release 8/8 CTests each; 75,426 diagnostics and
93,440 apply checks plus prior suites; independent 216-check mode-image probe;
Doxygen/visibility, links/anchors/hash/version checks, 38-file package with a
tool-independent extracted consumer and native PlatformIO consumer. ASan/UBSan
are BLOCKED by missing runtimes. Hardware and framework firmware are NOT RUN;
remote CI remains prompt34. See [validation](validation.md#independent-prompts-16-18-audit-2026-10-27).
The lead commits and syncs this complete checked audit on the same branch and
reports final commit/equality in the handoff. Prompts 16-18 remain complete;
prompt19 is not started.

## Prompt 19 execution (2026-10-28)

Resumed the existing branch from clean, fetched, synchronized
`4207107e5fc9bf2d5b8f7464bb4342f7763317b9` (0 ahead/0 behind). No branch change.
The lead owns production/registration/integration and Git. Two coding agents own
disjoint consumer and scheduler fixtures; a strong reasoning agent independently
reviews timing and current peers. See [peer revisions](audit/execution-19-peers.md).

Coverage gaps found: fixed-capacity external scheduling/backpressure, literal
physical108/110/111 deadline scenario, generation boundary arithmetic and isolated
all-method consumption with allocation traps. No production timing defect found.
Existing behavior is preserved while both generation increments now use a small
private saturating helper. Final verification and sync evidence follow below.

PASS: nine registered CTests in both strict Debug and Release; 4,196 scheduler,
369 isolated-consumer and20,493 owner checks, plus existing core regressions.
Consumer check/allocation sentinel exits1/86/86 are verified in both builds.
Independent review corrected inherited compile/link flag handling in the new
harness; restricted Windows PATH and runtime DLL isolation now pass. The lead
reviewed actual sources and results, not only agent summaries.

PASS: Doxygen/visible API, links/anchors/hash/version consistency, current38-file
package and extracted tests-OFF consumption, native PlatformIO consumer. ASan and
UBSan remain BLOCKED by missing runtimes; firmware/hardware are NOT RUN. No remote
CI request. Exact commands, counts and scoped limitations are in
[validation](validation.md#prompt-19-owner-portability-2026-10-28).

The lead commits/syncs this complete unit on the same execution branch and verifies
upstream equality in the handoff. Prompt20 remains unstarted and requires its own
execution request; no CLI or framework implementation was added here.

## Prompt 20 execution (2026-10-28)

Started clean and synchronized after fetch at
`c07e4e5473ffd5719b81f0451c79f3b15fa41532` (completed prompt 19), on the same
execution branch/base. Read current parent/local AGENTS, prompt index, COMMON,
full prompt 20, workflow/maintainability, CLI grammar/appearance/records/bounds and relevant
core baseline/ownership rules. Actual public methods/tests verify prerequisites.
No local workflow exists; remote CI remains prompt 34 only.

Fresh TMP1x2/24Cxx/SHT3x/LDC1614 revisions, clean state, exact source/test/CLI/
runner/build/instruction/workflow paths, malformed-input/backpressure cases and
accepted/rejected conventions are in [execution 20](audit/execution-20-peers.md).
No sibling edits, builds or Git mutations. Core silicon/access behavior is unchanged;
staging uses existing validators rather than copying register conversion.

Delegation: gpt-6.1-sol/high owned ConsoleBuffer/console tests, then the disjoint
output/schema/CLI guide; another gpt-6.1-sol/high owned CLI tests, persistent fixture
and transcript. gpt-6-astra/high independently audited peers and reviewed bounds/
ownership/framing read-only. Lead owns shared signatures/dispatch, registration,
integration/docs/verification and Git. No branch/worktree per agent.

All 27 local canonical rows/aliases are implemented; no future hardware handlers.
Review corrected core-profile attribution after local part changes, base accessory
capability, bounded printHelp/status-text scanning, exhaustion admission and pending
notice preservation, split-CRLF direct-input admission and response metadata.
Failed acknowledgements report UNCHANGED rather than a successful staging action;
both the C++ behavior and Python schema reject misleading failure labels.
Regressions exercise actual code and maximum-width formatter paths. PASS: strict
Debug/Release 12/12 CTests each; 133,669 CLI checks, 17,787 console checks and 51
persistent C++ transcript requests with typed schema/closure negatives. Positive
interleaved help/marker/staging keeps the original request/nonce and one END.
Strict Doxygen and XML visibility show the documented core plus Cli/Console APIs;
version/no-op, links/anchors/UTF-8/LF and current 43-file package checks pass.

PASS: ordinary tests-OFF CMake/native PlatformIO consumers, and extracted ordinary
and CLI C++17 applications beneath unrelated Git without maintainer tools on child
PATH. CLI object storage is 9280 host-ABI bytes including the 8192-byte queue;
this is not linked board-RAM evidence. Core source lists and behavior are unchanged.
ASan/UBSan fresh link probes are BLOCKED by missing MinGW runtime libraries;
MSVC/Clang/clang-format are unavailable. Firmware/hardware are NOT RUN; remote CI
remains prompt 34. Exact commands and limits are in
[validation](validation.md#prompt-20-bounded-local-cli-2026-10-28).

The lead commits/syncs this checked complete unit on the same execution branch,
verifies upstream equality and clean state, and reports final commit in the
handoff/next progress update. Prompt 21 remains unstarted.

## Prompt 21 execution (2026-10-28)

Started clean after fetch at synchronized prompt 20 commit
`072015c5b5d977bfe81d42978bbb587b5b8e2fea`, on the same execution branch/base.
Read parent/local AGENTS, index, COMMON, full 21, workflow/maintainability,
CLI grammar/staging/records/ownership and core cooperative/time/trust/silicon
contracts. The three prerequisite CLI/console/transcript CTests passed again.
There is no local workflow; remote CI remains prompt 34 only.

Fresh read-only TMP1x2/24Cxx/LDC1614/INA228 revisions, status, exact declarations,
dispatch, cleanup tests, runner, budget and workflow evidence are recorded in
[execution 21](audit/execution-21-peers.md). Keep core result ownership rather than
copying the peers' automatic take/cached-result designs. Basic callbacks still
require a fresh owner turn even when an explicit budget exceeds one.

Delegation: gpt-6.1-sol/high owns disjoint operation test vectors; another owns
the persistent fixture/Python schema/transcript. gpt-6-astra/high audits peers
and independently reviews ownership/timing/effects. The lead owns production,
integration, registration, guides/checks and all Git. No sibling edits or builds.

Implementation uses existing start APIs, one tick poll and request/nonce/token
metadata only. Explicit result take reserves its complete bounded response before
core consumption. Review found and corrected hidden owner-time poll rejection;
the JOB schema exposes poll status separately from terminal operation status.
Final review also corrected successful result delivery without its required
payload, contradictory cancelled/timed-out schemas, misleading step effects and
synthetic loss accounting on refused take reservation. Near-full refusal delivers
BUSY without inventing a dropped record; actual failed enqueues retain LOSS.

PASS: all 12 CTests in strict Debug/Release, followed by affected CLI/console/
transcript checks after the final accounting fix. Counts: 149,227 local/framing
and 7,801 operation checks, 17,787 console checks, 178 persistent requests plus two
fault sessions. Exact byte/fixture effects cover ACK versus staged/desired,
unrelated W1C preservation, write/readback errors, late/invalid time, reset and
qualified base recovery. Full output does not prevent cancel/end or expiry;
request exhaustion does not stop previously admitted work.

PASS: strict Doxygen/XML API visibility, Python docstrings/style, local links,
UTF-8/LF, deterministic version check, current 43-file package and renamed core/
CLI C++17 consumers beneath unrelated Git without maintainer tools on child PATH.
Standalone CMake and managed native PlatformIO host consumers also pass. CLI
object size is 9416 bytes on this host ABI; board RAM/stack is unqualified.
Fresh ASan/UBSan probes remain BLOCKED by missing runtimes; MSVC/Clang/clang-format
are unavailable. Serial/CC/INT, physical stable-VDD recovery, framework firmware
and adapter aggregate timing remain NOT RUN. No remote CI request.

See [validation](validation.md#prompt-21-managed-cli-operations-2026-10-28) for exact
commands and limits. The lead commits/syncs this checked unit on the same execution
branch and verifies equality/clean state in the handoff; record the resulting
commit in the next progress update. Prompt 22 remains unstarted.

## Prompt 22 execution (2026-10-28)

Started clean after fetch at synchronized prompt 21 commit
`e1f5538f669af4f926af355f2bba904da9478049`, on the same execution branch/base.
Read current parent/local AGENTS, index, full 22, COMMON, workflow/maintainability,
CLI grammar/records/ownership and core raw/accounting/health/example contracts.
The prerequisite CLI/console/transcript CTests passed again. No local workflow
exists; remote CI remains prompt 34 only.

Fresh TMP1x2/24Cxx/SHT3x/LDC1614/INA228 revisions, clean state, exact declarations,
diagnostic/help/alias/parity/runner/workflow paths and adopted/rejected conventions
are in [execution 22](audit/execution-22-peers.md). No sibling edits or builds.
Comparison with core declarations found eleven missing canonical routes: now 55
complete rows, all through public core APIs or the opt-in application input hook.

Delegation: gpt-6.1-sol/high owns independent diagnostic C++ cases; another owns
Python schemas, the persistent fixture/transcript and executable parity checker.
gpt-6-astra/high audits peers and reviews raw safety, accounting and coverage.
Lead owns production/build/docs/integration/verification and all Git operations.

Raw callback attempts use ATTEMPTED, including backend failures, rather than
claiming unchanged hardware/configuration. Local gates remain bus-silent.
The reviewer identified missing profile mismatch/acquisition evidence; the new
bounded CONFIG_DETAIL exposes it. All-source totals and managed health have
explicit source labels. Assertion saturation uses a private pure helper tested
with copied limit values, not public test seeds. A failed accepted raw write under
full-output congestion retains UNKNOWN trust/uncertainty and never retries.

PASS: strict Debug and Release each run all 14 registered CTests. The combined CLI
suites execute 766,146 checks; the console executes 24,146. Executable parity
checks 55 rows/164 canonical, alias and help routes, plus deliberate broken alias
and required-field dependencies. The transcript runs 248 requests in one process,
two managed-fault sessions, four input-reader modes and two raw-fault sessions.
All 33 schemas and real formatter paths retain maximum-width/newline/NUL bounds.

PASS: strict Doxygen/XML visibility, Python docstrings/style, local links/anchors,
UTF-8/LF/whitespace and deterministic version sync/check. The current 43-file
package matches source bytes and excludes test/audit/reference/tooling artifacts.
Renamed tests-OFF ordinary and CLI applications beneath unrelated Git compile,
link and run C++17 without maintainer tools on child PATH. CLI object storage is
9416 host-ABI bytes; board RAM/stack remains unqualified. Standalone CMake and
installed managed PlatformIO native consumers pass. Core and version are unchanged.

Fresh ASan/UBSan probes are BLOCKED by absent MinGW runtimes; MSVC/Clang/clang-format
are unavailable. Framework firmware, serial/CC/INT, adapter aggregate timing and
electrical qualification are NOT RUN. Remote CI remains prompt 34 only.
Exact commands/results are in
[validation](validation.md#prompt-22-complete-cli-diagnostics-and-parity-2026-10-28).

The lead commits/syncs this complete checked unit on the same execution branch,
verifies upstream equality and clean state, and reports the resulting commit in
the handoff/next progress update. Prompt 23 remains unstarted.

## Independent prompts 20-22 audit (2026-10-28)

Started from clean, fetched, synchronized `6d99a29` on the existing execution
branch. Re-read all three original prompts and current shared contracts. Three
inherited-model reviewers independently inspected parser/console/target staging,
managed operation/results, and diagnostics/parity tooling. The lead inspected
actual code/diffs, integration and before-fix reproductions; no prompt23 work.

Confirmed gaps: help rows followed implementation-stage order and repeated topic
headings; transcript closure protection covered admitted jobs but not closed local
requests. Fixed grouping by reordering the existing fixed arrays, and extended
per-request closure checks without imposing completion order across open requests.
Added the previously untested eight-record take-reservation branch: existing
production ownership was correct. Current peer evidence and requirement coverage
are in [the audit](audit/cli-20-22-review.md). Final checks and sync follow below.

Final review also closed the EOF validation gap and corrected the explicit-step
parity fixture: a sufficiently long admitted deadline now makes the check require
one real callback, followed by explicit cancellation after measurement.

PASS: fresh strict Debug and Release each run 14/14 registered tests. The CLI
executes 766,425 checks (156,606 local, 8,027 operation, 601,792 diagnostic).
Persistent execution covers 248 main-session requests plus fault/input sessions
and 11 new ordering/EOF cases; parity covers 55 rows/164 routes. The original
help implementation fails 17 new assertions and the original reader accepts a
duplicate END, independently reproducing the corrected gaps.

PASS: Doxygen visibility, Python style/docstrings, links/anchors/provenance,
version consistency, installed PlatformIO native consumer and the 43-file
source-equal archive with isolated tests-OFF core/CLI consumers. ASan/UBSan
remain BLOCKED by missing link runtimes; framework/hardware evidence is NOT RUN.
Full commands are in [validation](validation.md#independent-cli-audit-2026-10-28).
Only the reviewed audit unit is committed and synchronized on the existing
branch; final commit/equality is reported in the handoff. Prompt 23 is unstarted.


## Prompt 23: Arduino transport (2026-10-28)

Entry `eba12f2` is clean and synchronized on the existing execution branch.
The previous CLI audit commit equals its upstream. Read prompt 23, root/parent
rules, COMMON, transport/time/lifetime, chip framing and maintainability contracts.
Two inherited-model agents independently audited peer adapters and the pinned
SDK deadline/terminality paths. A third owns disjoint recording host fixtures;
the peer auditor owns only the S2/S3 link fixtures. The lead owns the adapter,
registration, integration, documentation and Git.

Selected installed pioarduino 55.03.311 / Arduino 3.3.11 / IDF 5.5.5 commit
b774170ff46. Support is deliberately narrow: S2/S3, globally disabled Arduino
HAL locks, shared nonblocking gate, prewarmed devices, same initialization core,
SDK logging disabled and a qualified application CPU/scheduling bound. Per-phase
SDK waits/cleanup are reserved before traffic. The exact driver can retain a
receive-state flag after failure, so a failed receive retires the whole controller
until explicit application teardown/reinitialization. No adapter recovery/retry.
Wire code 2 remains generic; read errors expose only returned length.


Implemented example-only terminal callbacks, 64-bit native owner clock, saturating
Wire-call/buffer counters and one shared gate. A fresh independent reviewer found
and corrected a post-copy timeout edge; its regression fails against the old path.
The adapter preserves output on failure, restores timeout on every acquired-session
exit and rejects insufficient SDK envelopes before traffic. No custom driver,
core framework include, retries, second CLI or bus lifecycle ownership was added.

PASS: strict Debug/Release each ran all 16 registered CTests. Final focused runs
cover the added publication regression and compile guards; adapter suites execute
1,129 checks at 1000Hz and 1,106 at 100Hz. All prior core/CLI/consumer/tooling suites
remain passing. The lead also rebuilt both actual SDK targets: S2 and S3 link
C++17 core, real Wire/HAL and every adapter entry point. Compilation databases
verify global lock/log flags. S2 uses 17,888 RAM/250,632 flash bytes; S3 uses
22,968 RAM/288,413 flash bytes. These are build-fixture sizes, not complete apps.

PASS: strict Doxygen/public visibility, links/anchors/provenance and no-op version
checks; the expanded 46-file package matches source and IDF selection, excludes
fixtures/reference code, and supports isolated tests-OFF core/CLI consumers without
maintainer tools. Exact sources/choices are in [the audit](audit/execution-23-arduino.md)
and commands/results in [validation](validation.md#prompt-23-arduino-transport-2026-10-28).
Application scheduling qualification, physical timing/electrical tests and flashing
are NOT RUN. Complete board examples and native IDF adapter remain later work.
The lead commits/syncs this unit on the same execution branch and reports verified
upstream equality in the handoff; no self-referencing hash update is required.


## Prompt 24: Arduino CLI and owner examples (2026-10-28)

Continued from synchronized `eace1c3a33961bfad83798af4c15ae1ecb9b6d68` on the same
execution branch. The fresh [peer/SDK audit](audit/execution-24-arduino-examples.md)
records TMP1x2 and LSM6DS3TR revisions, actual examples/tests/runners/CI definitions,
and accepted/rejected conventions. Official USB documentation was checked against
pinned Arduino3.3.11 and bundled TinyUSB implementation.

Delivered both source-filtered S2/S3 applications, one shared processor and
unchanged adapter. Application startup owns read-only init/port results and takes
its exact tokens independently of output. The secondary owner shares the one
controller gate and retirement state, alternates turns and accepts deferred
coalesced IRQ requests. CLI results remain explicit-take. Shutdown stops admission,
ends bindings and retains/report effects before releasing application-owned Wire.
The static USB worker stays alive; no SDK TX call can hold up an owner turn.

The board configuration remains Arduino-specific under examples/arduino rather
than the baseline's prospective common BoardConfig path. Portable owner/mailbox
helpers and the shared CLI remain under common. One narrow application-record
queue method avoids a second output queue and record interleaving. No additional
command parser, core scheduler, transport implementation or power policy was added.

Three inherited-capability agents handled independent peer/USB reviews, portable
owner/mailbox helpers and tests, and Arduino application/build integration with
disjoint files; the lead reviewed source, fixed integration findings, registered
checks and owns Git. Confirmed fixes include the Arduino cli macro collision,
S2 atomic flag handoff, raw override narrowing checks, startup IRQ retention,
configured-input validity, enable/IRQ pin ownership and distinct terminal/admission status reporting.

PASS: Debug and Release each execute 20 registered suites. New owner/integration/
mailbox suites execute 924/277/1574 checks; metadata/configuration tests execute
seven cases including five positive and 25 rejected real C++ compilations. All
prior core, CLI, console, adapter, consumer and metadata regressions pass.
All four real SDK images link; two additional enabled-path owner images also link.
Strict Doxygen, visibility, links, version, provenance and diff checks pass.
The 60-file package matches source/IDF selection; extracted core and CLI consumers
build/run without maintainer tools. Exact commands and limits are in
[validation](validation.md#prompt-24-arduino-applications-2026-10-28).

Default images intentionally require explicit board readiness, SDK cleanup
acceptance and a qualified whole-callback overhead before I2C setup. Synthetic
values in enabled-path build checks are not board qualification. Flash, serial,
HIL, electrical/USB-level measurements and timing qualification are NOT RUN.
No identified board was authorized. Native-IDF work remains prompt 25.
Commit/sync and final upstream equality are reported in the handoff without a
self-referencing commit-hash update. Remote CI remains prompt 34 only.


## Prompt 25: native ESP-IDF adapter (2026-10-28)

Started from synchronized `0d7be0c95c653eaeba7b17398e9f7867522e3af4` on the same
execution branch, with clean status and zero ahead/behind after fetch. Prompt 24
is complete. The [fresh peer audit](audit/execution-25-peers.md) records current
LDC1614, MB85RC and TMP1x2 revisions, dirty states, sources/tests/examples/runners/
workflows, and accepted/rejected ownership/build/timeout patterns.

Implemented native synchronous callbacks, local setup validation and native owner
time under examples/esp_idf. The application retains bus/device ownership and
provides exact copied configuration, one shared controller gate, same-core task
pinning, cleanup acceptance and qualified whole-callback overhead. The adapter
rejects asynchronous configuration and mismatched addresses before traffic,
divides the aggregate budget over SDK waits/cleanup and preserves native error
detail. Failed receives retire the entire shared controller because the pinned
SDK can retain read state. Output is published only after complete success.
No core/framework dependency, CLI application, custom driver or recovery path
was introduced. Exact source locations and bound are in [integration](idf-transport.md).

Delegation used gpt-6.1-sol/high for disjoint peer/native-consumer files and
recording-backend tests, and gpt-6-astra/high for independent SDK timing/lifetime
review. The lead owned adapter/API/registration/docs and checked actual source,
diffs, artifacts and symbol tables. Review clarified inactive SDK buffer-pointer
retention, task pinning and valid test register ranges. Consumer binding now
requires successful adapter validation. Full SDK warning overrides were not
introduced: PlatformIO propagates main flags into third-party SDK code. The
actual adapter is compiled with strict warnings in both host contract suites;
native core keeps its strict flags and adapter uses SDK build warnings.

PASS: Debug and Release each execute 22 registered suites. Native adapter suites
execute 1602 checks at 1000Hz and 1572 at 100Hz. Both actual native S2/S3 fixtures
configure/compile/link all advertised adapter functions and the root core with
no Arduino or CLI frontend sources. Exact configuration/target/source boundaries
and linked symbols were inspected. Strict Doxygen, public visibility, links,
UTF-8/whitespace, version and reference hash checks pass. The 63-file package
matches source and IDF inclusion; isolated extracted core/CLI consumers build/run
without maintainer tools. Commands/artifacts are in
[validation](validation.md#prompt-25-native-idf-transport-2026-10-28).

The selected native SDK is IDF5.5.5/package3.50505.0 via pioarduino55.03.311.
The lead independently matched the local I2C source to official b774170ff46 bytes.
Other SDK revisions and physical CPU/ISR/waveform/electrical qualification remain
unverified. Flash, serial and HIL are NOT RUN. Native CLI remains prompt 26.
Commit/sync and verified upstream equality are reported in the handoff; the lead
alone changes Git. Remote CI remains prompt 34 only.

## Independent prompts 23–25 audit (2026-10-28)

Reviewed from clean, synchronized `7f23b901b97325fa3c49f3a1e4f26bd05d4231f0`
on the same `promptset/tusb32x-implementation-20261023` branch. All three original
prompts were reread in full. Parallel coverage/application and SDK lifetime/time
reviews inspected actual code, tests and the completed implementation diffs.
The lead verified findings, integrated fixes and owned all Git actions.

Fixed Wire zero-budget status, unsupported SDK clock/rate admission, Arduino
ownership of an already initialized controller, ignored teardown failures and
active USB GPIO reassignment. Corrected the coarse-tick budget example. Added
ten registered actual application scenarios and extended the existing adapter
and BoardConfig tests without a new production abstraction or core dependency.
[Audit findings](audit/review-23-25.md) map each correction to source evidence.
Peer revisions were rechecked clean and unchanged; sibling research stayed read-only.

PASS: all 32 host suites in both Debug and Release; strict warnings and active
checks retained. Wire contracts execute 1298/1275 checks and native contracts
1806/1766 at 1000/100Hz. Actual S2/S3 links pass for both adapter fixtures and all
four complete Arduino applications. Strict Doxygen, API visibility, local links,
source encoding, vendor hashes, version consistency and extracted tool-independent
core/CLI consumers pass. See [validation](validation.md#independent-prompts-2325-audit-2026-10-28)
for commands and evidence limits. The final independent source review found no
further confirmed defect under the documented SDK/application prerequisites.

Host fakes do not establish real scheduling or electrical timing. Board CPU
overhead qualification, flash, serial and HIL remain NOT RUN. Native CLI remains
prompt26; remote CI remains prompt34. Commit and verified sync are reported in
the handoff rather than creating a self-referential commit-ID update.

## Prompt 26: native ESP-IDF CLI application (2026-10-28)

Started clean and synchronized after fetch at independent adapter/application
review commit `4445cdc9a67aceed1347b8685aa2a2a347a79743` on the same execution
branch. Read current parent/local rules, COMMON, full 26, CLI/ownership and native
adapter contracts; prerequisites 1-25 are present. Fresh peer/SDK evidence is in
[audit](audit/execution-26-peers.md). No sibling edits/builds, remote CI or hardware.

The application uses the existing shared CLI, core, adapter and ExampleOwner.
UART0 has explicit TX43/RX44 defaults on both targets; SDK stdio/secondary console
and logging are disabled. One pinned core0 main task owns UART, I2C/device handles
and copied metadata. Defaults keep I2C disabled until board readiness, qualified
CPU overhead and SDK cleanup acceptance are supplied. Startup explicitly observes
identity then port and consumes each exact owned result before CLI handoff.

Delegation: gpt-6.1-sol/high owns application CMake/configuration, incremental
identity and configuration fixtures; another owns recording native application
fault tests. gpt-6-astra/high owns fresh peer/SDK audit and independent lifetime/
console review. The lead owns production lifecycle, integration/docs and Git.
Review already corrected UART event-queue saturation admission, repeated failed
resource deletion and hardware-FIFO completion before console deletion. Final
host checks pass in Debug and Release, with 61 registered tests each (28 native
application scenarios and four native configuration/identity fixture cases).

PASS: actual IDF5.5.5 S2/S3 default and qualified-path images plus renamed-root S2
and its incremental source rebuild. The lead independently inspected real source
lists, linked symbols, target/configuration and metadata. SDKCONFIG hashes remain
unchanged across the other target's reconfiguration; dependency locks are also
per-build. The renamed root uses component renamed-lib and unknown Git/tree.
Defaults use the exclusive UART route; enabled-build numeric timing values are
compile illustrations, not physical qualification.

PASS: strict Doxygen/XML API visibility, Python docstrings/style, version/no-op,
local links, reference hashes and UTF-8/LF/whitespace. The 73-file package matches
source and IDF selection; isolated renamed tests-OFF core and CLI C++17 consumers
build/run without maintainer tools. Standalone CMake and managed native host
consumers pass. Core source lists, behavior and stable library version are unchanged.

Fresh ASan/UBSan links are BLOCKED by missing MinGW runtimes; other host compilers
and clang-format remain unavailable. Physical UART/USB-bridge/IRQ/CC/VBUS, board
stack/CPU overhead, SDK aggregate timing and flashing/HIL remain NOT RUN. No
identified board/port was authorized. Remote CI remains prompt34 only.
See [validation](validation.md#prompt-26-native-idf-cli-application-2026-10-28)
for exact commands, versions, artifact sizes and limits. The lead commits/syncs
this complete checked unit on the same branch and verifies clean upstream equality
in the handoff; record that commit in the next progress update. Prompt27 is unstarted.

## Prompt 27: plain C++ and generic owner integration (2026-10-28)

Started clean after fetch at synchronized native-application commit
`9e4b08a62303dbb9a6bbbfdbacb6aeb42154533c`, on the same execution branch/base.
Read current parent/local rules, COMMON, full27, workflow/maintainability, exact
operation/result/time and ownership contracts, public declarations and registered
owner/consumer tests. Relevant workflow definitions remain absent locally; remote
CI remains prompt34 only. No sibling edits/builds or hardware activity.

Fresh FieldCore-node, LDC1614 and TMP1x2 revisions, clean state, exact declaration/
implementation/test/runner/workflow/AGENTS paths and adopted/rejected conventions
are in [execution27](audit/execution-27-peers.md). The integration guide maps owner
concepts to public boundaries, explicitly preserving the limits of Status-only
completion/effect evidence and backend aggregate bounds. No product types enter core.

Delegation: gpt-6.1-sol/high owns the simulated ordinary host example; another
owns only missing cases in the existing public owner scheduling fixture.
gpt-6-astra/high owns fresh peer evidence and independent lifetime/effect review.
The lead integrates, registers the actual host executable, documents and runs
checks, and owns all Git. The review confirmed bus/context declaration order
needed correction so it outlives the two driver instances; explicit end/take
checks now demonstrate teardown. Existing fairness/recovery/fault matrices are
reused rather than duplicated. No core change, product wrapper, new scheduler or
public test hook was needed. The runnable host explicitly simulates LAI and
synthetic time; delivery OK with a CONFIGURATION_MISMATCH payload demonstrates
that delivery and operation success remain separate.

PASS: strict full native Debug and Release, 62/62 registered CTests each, including
the real shipped host main/profile target, existing public portable consumer and
all prior suites. The owner scheduling fixture runs 5,487 checks with zero
failures. All five public headers still compile independently. Standalone
tests-OFF CMake and managed PlatformIO native consumer build/link/run pass.
Final review also fixed duplicate target creation when the standalone project
explicitly enables tests. That configuration now discovers 62 tests and passes
the two affected host/operation suites; the tests-OFF path is unchanged.

PASS: 74 source-equal package files with matching IDF selection and resolvable
shipped links. Renamed extracted core/host and independent shared-CLI consumers
build/run beneath an unrelated Git parent with maintainer tools absent from child
PATH. The same public two-owner fixture also builds/runs there using tests OFF,
without private core headers, framework/sibling paths, Git or Python. The initial
isolated owner execution lacked MinGW runtime DLLs; copying only compiler runtime
DLLs into a dedicated tool-free PATH resolved it, without a delivered source change.

PASS: strict Doxygen/public XML visibility, Python documentation/style, stable
version/no-op sync, local links, reference hashes and UTF-8/LF/whitespace checks.
Fresh ASan/UBSan links remain BLOCKED by missing MinGW runtimes. Other host
compilers and clang-format are unavailable. Physical Type-C/IRQ/VBUS, serial,
real adapter aggregate timing and flashing/HIL remain NOT RUN; no board/port was
identified or authorized. Framework links from prior stages were not repeated
for this framework-free consumer change. Remote CI remains prompt34 only.
See [validation](validation.md#prompt-27-plain-c-and-generic-owner-integration-2026-10-28)
for exact versions, commands and limits. The lead commits/syncs this complete
checked unit and verifies clean upstream equality in the handoff; record that
commit in the next progress update. Prompt28 remains unstarted.

## Prompt 28: strict Python sessions and evidence artifacts (2026-10-28)

Started clean after fetch at synchronized prompt27 commit
`19fcd2c37837ef823fd852581d4cfe42b6bb8d5c`, on the same execution branch/base.
Read current parent/local rules, COMMON, full28, machine/runner contracts,
maintainability and actual CLI schemas/record production/live tests. Fresh peer
revisions, state, exact files and decisions are in execution-28-peers.md. No
workflow definitions exist locally; remote CI remains prompt34 only.

Delegation: gpt-6.1-sol/high owns the focused peer audit; another owns only the
artifact helper and its failure tests; another owns fake-serial session tests.
gpt-6-astra/high independently reviews framing/correlation/deadline/ownership.
The lead owns session production, live C++ integration, docs/build registration
and Git. Record schemas stay unchanged; one-request closure validation moves
from the transcript test into the existing pure CLI contract module for reuse.
Use a single absolute deadline, at most one owned operation, strict new-input
ACK/MARK ordering with older original completions allowed, and streamed evidence
with bounded tail. No copied scratch policy, full transcript journal, hardware
suite stub or hidden reset/retry/reconnect policy is introduced.

Optional pyserial3.5 was checked against official package/API documentation;
pre-open DTR/RTS configuration has OS/backend limits and cannot guarantee a board
will not reset on open. No physical serial port was opened.

PASS: 87 deterministic Python session/artifact tests, including 26 dedicated
artifact failure cases; two real C++ Session exchanges added to the existing
single cli_transcript registration. The original 248-request transcript and
damaged-record checks remain passing. Full strict Debug and Release CTest each
pass 63/63 registrations, with the built fixture path explicit and finite timeouts.

Independent review found and corrected write-deadline renewal, buffered/trailing
framing, owned-token correlation, profile-source completeness, successful-delivery
versus payload failure, verified local restoration and saved close/capture failure
handling. The lead verified fixes against source and actual C++ output, not agent
summaries. Original operation deadlines and response caps persist through later
waits/queries; no growing transcript/token/request journal was added.

PASS: strict Doxygen/XML visibility, complete Python module/function/class docs,
100-column style, local links, version/no-op checks, vendor hashes and UTF-8/LF/
whitespace. The package contains 75 source-equal files with matching IDF selection,
including the session guide, while maintainer runners remain excluded. Renamed
tests-OFF core/host, shared-CLI and public two-owner consumers build/run beneath an
unrelated Git parent without Git/Python/Doxygen/PlatformIO on child PATH.

No core/firmware source or stable library version changed. Prior sanitizer probes
remain blocked by absent MinGW runtimes; no alternative host compiler or
clang-format is installed. Serial/USB/IRQ/CC/VBUS, real adapter timing and flashing/
HIL remain NOT RUN. Remote CI remains prompt34 only. See
[validation](validation.md#prompt-28-strict-python-sessions-and-evidence-2026-10-28)
for exact commands/results and limits. The lead commits/syncs this complete unit
on the same branch and verifies upstream equality in the handoff; record that
commit in the next progress update. Prompt29 is unstarted.

## Independent audit of prompts 27 and 28 (2026-10-28)

Started clean after fetch at synchronized prompt28 commit
`e1e79e6fb440fe2c3eb56d2df215b46722dcded3`, on the same execution branch/base.
Reread both original prompts in full, current parent/local rules, COMMON,
workflow/maintainability, owner timing/lifetime and CLI machine/runner contracts.
Compared actual implementation commits, public declarations and executed fixtures.
Fresh read-only FieldCore-node, TMP1x2, LDC1614 and 24Cxx revisions, exact files,
clean state and adopted/rejected conventions are in the consolidated
[independent review](audit/review-27-28.md). No local workflows exist; remote CI
remains prompt34 only. No sibling edits/builds or hardware activity occurred.

Delegation: gpt-6.1-sol/high independently reviewed prompt27 coverage, simplicity,
lifetimes and fairness and ran fresh ordinary/owner consumers. Another audited
prompt28 artifact faults/peers, then owned only the artifact helper/tests and
three documentation files. gpt-6-astra/high reviewed actual session correlation,
deadline, staging and result ownership with fresh context. The lead reproduced
findings, owned session/live-fixture fixes, reviewed every returned diff, integrated
checks/documentation and retained all Git ownership. Shared schemas, public C++
API, core transport and operation engine remain unchanged.

Prompt27 had no confirmed defect. Prompt28 corrections require API end-marker
and original JOB proof, accept exact silent parser rejections without a duplicate
argument parser, require available/applicable saved staging including each edited
field, and validate complete restoration. Contradictory cleanup invalidates the
session. Expired public deadlines and late public checkpoints now save failure/
interruption and precise unconsumed/already-taken result facts. Sticky save_error
and one corrective JSON replacement prevent a failed summary save from leaving
an authoritative PASS. Final-save Ctrl-C gets one bounded interruption save.
No serial retry, hardware restoration, second result cache or later runner plan
was added. The stronger review's additional edge cases were confirmed and fixed
before handoff, with focused regressions rather than contract weakening.

PASS: 104 deterministic runner/artifact tests (30 dedicated artifact cases),
full strict Debug and Release 63/63 CTests each, and affected four CLI/runner
registrations repeated after final session corrections. Live C++ sessions now
also execute malformed known-route arguments and literal dotted input, proving
typed rejection preserves framing. Fresh ordinary tests-OFF Release and independent
public headers pass; the two-owner fixture runs 5,487 checks with zero failures.
The isolated portable consumer executes 369 checks and allocation/failure sentinels.

PASS: 75 source-equal packed files and matched IDF selection, renamed extracted
host/shared-CLI/public-owner consumers beneath unrelated Git with maintainer tools
absent from child PATH; stable version/no-op sync, Doxygen/public XML, Python
docstrings/style, local links, vendor hashes and UTF-8/LF/whitespace checks.
Exact commands, versions and outcomes are in the validation audit entry. Hardware
serial/CC/IRQ/VBUS and real adapter timing remain NOT RUN. No framework rebuild or
remote CI was needed for these tooling-only corrections. Sanitizer/other compiler
availability is recorded separately. Prompt29 remains unstarted.

The lead commits and syncs the complete checked audit on the existing branch,
then verifies equal upstream commits, zero ahead/behind and no uncommitted task
changes in the handoff; record its hash in the next progress update.

## Prompt 29: finite observation plans and native integration (2026-10-28)

Started clean after fetch at synchronized independent-audit commit
`aca463158485d13fe8d4437e1962dda4a4c32516`, on the same execution branch/base.
Read current parent/local rules, COMMON, full29, CLI runner/record and baseline
observation/register contracts, workflow and maintainability. Fresh clean peer
HEADs, exact declarations/CLI/runner/tests/AGENTS/workflow paths and adopted/
rejected choices are in [execution29](audit/execution-29-peers.md). No workflow
definitions exist locally; remote CI remains prompt34 only. No sibling edits/
builds, physical serial open, flashing or hardware actions occurred.

Delegation: gpt-6.1-sol/high owned the fresh peer audit, then reactive plan and
failure tests in the existing runner suite; another owned planning/pre-open/dry-run
tests and the two runner/hardware guides. gpt-6-astra/high reviewed ownership,
freshness and deadline evidence, then owned only the persistent C++ fixture and
transcript integration. The lead owned production planning/main, shared contracts,
integration/review, remaining docs/checks and all Git. Disjoint files remained
explicit, and reviewers' findings were confirmed against code/live output.

Implemented complete readonly and finite soak plans, a per-variant mapped register
list, strict pre-open target/identity/timing/capture guards, normalized repeated
suites and portless dry-run artifacts. Plans reuse Session/parser/artifacts and
ordinary commands, compare the configured target, reject foreign ownership,
bind only an already selected ended target, and never apply/reset/ACK/recover.
Local staging/auto/output restoration stays separate from core state and effects.
Result delivery/take remain exact and owned; BUSY take is retained evidence with
no hidden retry. Aggregate count/last observation avoids a growing sample journal.

Review fixes include concrete selected-suite descriptions, quiet-gap semantics,
unknown/contradictory/invalidation/coherence/raw/sequence/time freshness rejection,
counter saturation/regression/consistency, staged parser-state preservation,
zero callback deltas during staging/rejected verification, primary failure
preservation, final-save lateness and nonadvancing wait refusal. Actual wire
volume review also exposed unnecessary dependence on the CLI's 1000ms default:
init/profile/verify use their existing optional timeout clipped to the host
deadline. Port/soak reads use existing automatic finite polls within their fixed
1000ms owner deadline; explicit init steps still prove zero/one budgets. No grammar
change or physical timing qualification is inferred from these measurements.

PASS: 161 deterministic Python tests, including 26 planning/pre-open cases and
the original session/artifact suites. The unchanged cli_transcript registration
runs 248 original requests, two strict sessions, three full native readonly plans
(normal, failed callback, output-full take) and two controlled owner-turn sessions.
Full strict Debug and Release pass 63/63 CTests each; the four affected CLI/runner
registrations were repeated after final corrections. Source/registered tests use
no pyserial, real sleeps or implicit hardware. Exact built fixture paths and
finite timeouts remain unchanged. The actual dry-run saves LAI/0x47 and hardware
NOT RUN with exit2; help runs successfully.

PASS: version sync/check/no-op stability, strict Doxygen/public XML, complete
Python docs/100-column style, local links, vendor hashes and UTF-8/LF/whitespace.
The 76-file source-equal package includes the new hardware guide, retains explicit
source lists/IDF selection and excludes maintainer tools. Renamed extracted host,
shared CLI and public two-owner consumers build/run beneath unrelated Git with
maintainer tools absent from child PATH. ASan/UBSan remain unavailable in the
installed MinGW runtime; alternative compilers/clang-format remain absent.
No firmware rebuild or remote CI was required for host tooling/fixture changes.
Physical serial/I2C/CC/IRQ/VBUS and real adapter timing remain NOT RUN. See
[validation](validation.md#prompt-29-observation-plans-and-native-integration-2026-10-28).

The lead commits and synchronizes this complete checked unit on the execution
branch, then verifies upstream equality, zero ahead/behind and no task changes in
the handoff. Record its hash in the next progress update. Prompt30 is unstarted.

## Prompt 30: explicit port/current/interrupt fixture plans (2026-10-28)

Started clean after fetch at synchronized prompt29 commit
`1f0fb7604ad1696aea9f3067c320d3b3c2fca89e`, on the same execution branch/base.
Read current parent/local rules, COMMON, full30, CLI runner/records, baseline
port/IRQ/profile/capability and ownership contracts, maintainability and agent
workflow. Fresh peer HEAD/dirty states, exact declarations/CLI/fixture tests/
runners/AGENTS/workflow paths and adopted/rejected choices are recorded in
[execution30](audit/execution-30-peers.md). Exact archived TI tables/prose and
hashes were checked again; public sensor scratch/restore policy was rejected.
No sibling edits/builds, real serial port, flash, physical fixture or remote CI.

Delegation used gpt-6.1-sol/high for the read-only peer audit, strict setup and
finite console helper/tests, independent fake-wire fixture cases, and launch-only
C++ physical model/transcript plus command-line tests, with explicit disjoint
files. gpt-6-astra/high independently reviewed capability ambiguities, physical
evidence, reset/IRQ effects, timing and cleanup. The lead owned production plans,
shared dispatch, integration, returned-diff review, docs/checks and Git. Actual
source and behavior, rather than reports alone, confirmed each finding.

Implemented all four named suites using ordinary CLI operations and the existing
Session/parser/artifact layer. Strict fixture declarations/expected firmware and
finite operator transitions precede physical work; no board power operation is
implied. Capture known actual profile/trust/generations and current valid coherent
port, stage from actual fields, take each exact owned result once and stop on
failure. Conditional successful cleanup revalidates identity/target/original
partner/supply/detach before restoring original actual settings. Failed work never
retries uncertain writes or hardware restoration. Local staging/output/auto
cleanup cannot certify device cleanup or erase failure/effects.

Independent review corrected base charge-through support, extended debug-local
interpretation/mode, CLI3a0 spelling, source-capable3A supply inheritance, missing
INT preflight, unknown/contradictory raw/profile evidence, sticky cleanup-failure
handling, queued operator input, inaccurate dry-run actions and mixed-group proof
retention. Base current updates explicitly reset, revalidate and obtain a separate
settled physical observation, retaining pre/post-reset evidence. All322 fixture
mutation plans remain visibly gated before I2C. No core API/composer/scheduler or
firmware grammar changed. Prompt31 is unstarted.

PASS:225 deterministic Python tests (including33 fixture,18 setup/console and13
entrypoint cases), all four complete production fixture plans against the actual
persistent C++ CLI, full Debug63/63 and final Release63/63 CTests. Final affected
Debug10/10 was repeated after terminal-consistency and evidence corrections.
An initial three-assertion failure exposed a test hook that corrupted only copied
results; strict original-result preservation was correct. Inject original and
retained payloads consistently; keep assertions and rerun the complete registration.

PASS: help and four-suite no-port dry-run with actual exit2/hardware NOT RUN,
fresh tests-OFF host execution, version/no-op sync, strict Doxygen/public XML,
Python docstrings/style, links/vendor hashes and UTF-8/LF. The76-file source-equal
package passes renamed extracted host/shared-CLI/public-owner consumption beneath
unrelated Git without maintainer tools in child PATH (owner5487 checks/0failures).
Fresh ASan/UBSan probes fail for missing MinGW runtimes; alternative compilers/
clang-format remain unavailable. Physical serial/I2C/CC/INT/current/VBUS/VCONN/
supply and real adapter timing remain NOT RUN. Exact commands/versions/results:
[validation](validation.md#prompt-30-fixture-plans-and-bounded-physical-evidence-2026-10-28).

The lead reviewed the complete unit and independent final requirements review
reported no remaining production gap. Commit/sync this completed prompt on the
same execution branch, verify upstream equality/zero ahead-behind and no task
changes in the handoff; record its hash in the next progress update. Prompt31 is
unstarted. No physical qualification is inferred from software PASS.

## Independent audit of prompts 29 and 30 (2026-10-28)

Started clean at completed prompt30 commit
`770e4f6bb892de2df72fb712693d90759ca7c6cd` on
`promptset/tusb32x-implementation-20261023`; fetch confirmed equal upstream and
zero ahead/behind. Re-read both original prompts in full, current parent/local
rules, COMMON, CLI/runner contracts, baseline observation/profile/IRQ/capability
and owner/timing rules, maintainability and delegation instructions. Compared the
actual implementation and tests with requirements rather than prior summaries.

Fresh read-only peer review found these clean revisions:

| Peer | HEAD |
| --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` |

Inspected each peer's root AGENTS and `.github/workflows/ci.yml`, public headers
`include/TMP1x2/TMP1x2.h`, `include/EEPROM24Cxx/EEPROM24Cxx.h`,
`include/LDC1614/LDC1614.h` and `include/INA228/INA228.h`, and shared CLI headers
`examples/common/Tmp1x2Cli.h`, `examples/common/Eeprom24CxxCli.h` and
`examples/common/Ldc1614Cli.h`. Re-read TMP1x2
`tools/hil_tmp1x2_runner.py` and `tools/test_hil_tmp1x2_runner.py`,
24Cxx `tools/hil_runner.py` and `tools/test_cli_transcript.py`, LDC1614
`tools/ldc1614_hil_runner.py` and `tools/test_ldc1614_hil_runner.py`, and INA228
`test/test_basic.cpp` transfer-budget/owner-exclusion cases. Revisited current
TUSB `test/test_interrupt.cpp`, `test/test_profile_application.cpp`, public
declarations, shared CLI, Python schemas/plans/session and CTest registration.
The archived base/LI-HI/LAI-HAI TI tables were re-read; native literal vectors cite
SLLSEQ8D Tables8/9 pp17-18. Adopt finite runs, saturation rejection, persistent
actual CLI and independent observations/separate outcomes. Reject sensor bulk
commands, EEPROM scratch restoration, textual PASS and peer-specific health or
cleanup policies. No local workflow exists; no remote CI or sibling changes.

Parallel independent reviewers used gpt-6.1-sol/high for prompt29 timing/plans
and peer/requirements/native coverage, and gpt-6-astra/high for mutation safety,
evidence and cleanup. After findings were confirmed, disjoint ownership covered
observation/invocation tests, fixture tests and the native transcript. The lead
owned production corrections, integration, final actual-diff review, docs and Git.

Confirmed and fixed renewed whole-run deadlines after Session construction,
mixed-suite duration rejection after earlier fixture work, and external fixture
replies accepted after their individual transition deadline. One absolute bound
now survives all groups without changing requested duration ceilings. Late true
and false replies both fail. Runtime regressions reproduced the deadline defects
before the correction and passed afterward.

Confirmed false PASS paths from missing CONFIG acquisition/field proof and
ignored/contradictory CLI staging. Require complete matching observed evidence,
stage and validate the complete actual-derived candidate, and verify unchanged
fields as well as requested changes. IRQ's 30%-staged/50%-actual experiment now
proves the differing stage before ACK. Readonly similarly proves its exact local
edit. Unknown extensions remain recorded without becoming semantic requirements;
Session remains the single immutable known-result comparison. Native physical
inputs now use independent literal bytes rather than the plan's expectations;
two deliberately wrong expectation cases prove real C++ evidence rejects them.
Updated stale guidance and affected guides; no core, adapter, CLI grammar,
scheduler, firmware route or test registration was added.

PASS: final full strict Debug63/63 and Release63/63 CTests, including237 Python
runner cases, four complete native fixture plans and two independent native
mismatch rejections. Fresh tests-OFF host runs;76 source-equal packed files and
renamed extracted host/shared-CLI/public-owner consumers pass beneath unrelated
Git without maintainer tools on child PATH (owner5487 checks/0failures).
Readonly and four-suite portless dry-runs save actual exit2/hardware NOT RUN.
Version/no-op sync, Doxygen/public XML, Python docs/style, links/vendor hashes and
UTF-8/LF/whitespace checks pass. Exact commands and limitations are in the
[validation audit](validation.md#independent-audit-of-prompts-29-and-30-2026-10-28).

ASan/UBSan probes still fail for missing installed MinGW runtimes; alternative
compilers/clang-format remain absent. Physical serial/I2C/CC/INT/current/supply/
VBUS/VCONN and real adapter timing remain NOT RUN. No hardware or framework build
claim follows from native model success. Prompt31 remains unstarted. The lead
commits and syncs this complete checked audit on the same execution branch, then
verifies fetched upstream equality, zero ahead/behind and no task changes before
handoff; record that commit in the next progress update.

## Prompt 31: reset, fault and console suites (2026-10-28)

Started clean on `promptset/tusb32x-implementation-20261023` at synchronized
audit commit `b8eb52fa52e800fe3b53d33f4e53384d37f37544`; fetch confirmed equal
upstream. Read full31, current parent/local rules, COMMON, promptset index,
CLI/runner contracts, reset/recovery/effects/time/trust baseline and engineering
rules, maintainability and delegation instructions. Fresh peer HEADs/dirty state,
exact declarations/CLI/tests/runners/workflows and adopted/rejected choices are in
[execution31](audit/execution-31-peers.md). No sibling changes/builds, serial port,
flash, supply operation, physical fixture or remote CI.

Delegation: gpt-6.1-sol/high audited peers, then owned only the new recovery
test module; another owned the persistent native fixture/transcript cases.
gpt-6-astra/high independently checked exact archived TI reset/recovery/electrical
requirements and reviewed effects, timing, epochs and cleanup. The lead owns
production/shared contracts, integration/actual-diff review, remaining docs,
verification and all Git. File ownership stayed disjoint.

Implemented three explicit finite plans using existing Session, ordinary typed
CLI operations, checkpoints and exact owned-result take. Setup independently
authorizes physical actions; no board-power control or generic mutating selection.
Expected negative outcomes are exact and fixed before admission, with original
status/detail/effect retained and unexpected failure sticky. Readiness and the
pre-recovery actual baseline precede original-part recovery. Reset/termination
checks distinguish both controls, current trust and historical sequence evidence.
Fault and console never replay an old profile after power/reboot; original source
startup after a new VDD epoch stays blocked for explicit owner recovery. Console
closes old local state, then establishes new nonce/identity/target sessions.

Independent integration/review found and fixed help-text reboot false positives,
missing pre-recovery baseline, malformed reconnect return masking the original
failure, stale original-part startup evidence after power loss, and historical
reset-acceptance uncertainty incorrectly predicting current reconcile failure.
The all-nine-family coverage table records each real plan, prerequisite, finite
end and failure path. Core C++, silicon gates and firmware grammar are unchanged.

PASS: full strict Debug63/63 and Release63/63 CTests; final affected runner/
transcript/core/console checks pass4/4 in both configurations and include306 Python cases
(69 recovery cases) under
the existing90s hil_runner limit. Final review requires fresh detached-port proof
before original recovery writes as well as its saved actual configuration baseline.
The single30s transcript retains prior coverage, adds five complete production
plans (reset LAI/320/320I, fault and console), actual zero-write backpressure/loss,
literal reset/recovery bytes/waits, uncertain effects, failed/late callbacks,
output-full take and fresh reconnect domains. An all-nine mixed fake invocation
also passes with one absolute deadline and bounded artifacts. No assertions,
contract or required registration was weakened to obtain these outcomes.

PASS: ordinary tests-OFF host build/run;76 source-equal packed files, matching
IDF selection and renamed extracted host/shared-CLI/public-owner consumers under
unrelated Git without maintainer tools on child PATH (owner5487 checks/0failures).
Help and three-suite portless dry-run save actual exit2/hardware NOT RUN. Version
sync/check/no-op stability, strict Doxygen/XML,14 Python modules' full docstrings/
style, local links/vendor hashes and UTF-8/LF/whitespace pass. Fresh ASan/UBSan
link probes fail for absent MinGW runtimes; alternative compilers/clang-format
remain unavailable. Commands and exact evidence are in
[validation](validation.md#prompt-31-reset-fault-and-console-suites-2026-10-28).

Physical serial/I2C/CC/INT/supply/VBUS/VCONN and actual reset/recovery timing
remain NOT RUN. Native literal traces, artificial faults and restarted host
processes are labeled modeled evidence. Original-part new-power source startup
and unresolved322 mutation gates remain explicit. No unchanged firmware rebuild
or remote CI was needed for host runner/test/docs changes. The lead commits and
syncs this complete checked prompt on the same branch, verifies fetched upstream
equality/zero ahead-behind and no task changes, and reports the commit in the
handoff; record its hash in the next progress update. Prompt32 is unstarted.

## Prompt 32 complete: safe archives and clean consumers, 2026-10-28

Starting HEAD/upstream458706ae7ebedeb24d43e2e7a7f6a48732865ba5 (prompt31),
clean index/worktree; fetch and comparison equal on the existing
promptset/tusb32x-implementation-20261023 branch. No branch/worktree, release,
hardware or remote-CI action was introduced. Fresh audits are
[peer/consumer](audit/execution-32-peers.md) and
[framework](audit/execution-32-frameworks.md); all five peers were clean and
unchanged. Read parent/local rules, COMMON, prompt32, workflow/maintainability,
package/ownership baseline/rules, actual manifests/CMake/generated files and
current example commands before editing.

Disjoint delegation: gpt-6.1-sol/high owns copied public consumer and peer audit;
gpt-6-astra/high owns independent extraction review and hostile archive tests;
gpt-6.1-sol/high owns installed-tool and six extracted-image evidence. Lead owns
checker, explicit content rules, registration, integration and Git. No sibling
writes or Git mutations by agents. Frozen interfaces preceded test implementation.

Implemented matching literal76file manifests, exact-content/source-equality check,
bounded private archive snapshots/manual extraction, copied renamed CMake consumer
and two finite real CTests. Framework links use one frozen managed-Core archive,
SHA256230c31cafbfce9932ff4732075f1f8619f39855adaf0b0f50688c148d1454e08;
exported source/docs are frozen while those links run. Core/public APIs and stable
Version.h are unchanged. No install/export infrastructure is added.

Independent red regressions found hidden concatenated tar content, recursive PAX
headers, semantic PAX size concealment and a reported hash from reopened input.
The checker now rejects nonzero trailing bytes and unsupported extensions before
interpretation, accepts only actual packer mtime metadata, bounds physical headers
and reports its immutable bounded snapshot hash. All30hostile/archive cases pass.
First CTest package run also exposed an isolation check correctly rejecting its
nested checkout workspace; compilation now always uses an external clean tree.
Focused Debug/Release package reruns pass2/2 each; all63prior suites passed in
both initial full runs. Final full checks, framework links and sync follow below.

Final prompt32 evidence supersedes its in-progress artifact above: final actual
managed-Core archive SHA25697549728fd29eae2660630a011a0005991e39bc99ce2ad62aea703821a357316,
76source-equal regular files and173local package links/anchors. The copied CMake
consumer compiles/links/runs with tests OFF and no maintainer tools, including
beneath a temporary unrelated Git parent. The ordinary tests-OFF host example
also runs. Explicit export rules retain all shared/example sources and firmware
identity scripts while rejecting private/vendor/generated content. Version truth
and stable Version.h are unchanged; no install/export API is introduced.

Final Debug and Release suites PASS65/65 each; affected package suites rerun2/2
each after fixes. The safety unit file finishes35runtime tests, including malformed
DEFLATE, exact invocation exits and immutable snapshot hashing. Full source diff,
Doxygen/public visibility, version stability, all local links and UTF-8/docstrings
are checked. Final commands, paths, tools and limits are in
[validation](validation.md#prompt-32-archives-and-clean-consumers-2026-10-28).
Independent requirements review found and fixed one stale README completion claim
and the malformed-DEFLATE error path, in addition to the extraction review fixes.

All6final-artifact framework images link from fresh renamed extracted sources:
4Arduino basic/owner S2/S3 plus2genuine native-IDF CLI images. The lead verifies
actual ELF/BIN/map hashes, source paths and native app_main/APIs/noArduino.
Native build/SDKCONFIG/lock paths are separate; every image reports Git/treeunknown
even beneath an unrelated parent repository. No source file in extraction changed.
See [exact framework evidence](audit/execution-32-frameworks.md). A retracted
intermediate documentation artifact has cancelled partial output, never a final
success claim; the final matrix and clean consumer use the final reported hash.

Physical serial/I2C/CC/INT/VBUS/VCONN and timing are NOT RUN. No firmware flash,
fixture action, SDK installation, release publication, sibling edit or remote CI
was performed. Prompt33 remains unstarted. Lead commits and syncs this complete
checked unit on the existing branch and verifies fetched upstream equality,
zero ahead/behind and no remaining task edits before handoff. Record its completed
commit hash in the next progress entry, avoiding a self-referencing hash loop.

## Independent audit of prompts 31 and 32 complete, 2026-10-28

Starting synchronized HEAD c76e101cfc6339a48572573093233efdefb1c55d, the completed
prompt32 commit, on promptset/tusb32x-implementation-20261023; clean index/worktree,
fetch showed0ahead/behind. Read both original prompts fully, current contracts,
actual b8eb52f..458706a and458706a..c76e101 diffs and actual implementation/tests.
No new branch/worktree, remote CI or physical work. Prompt33 remains unstarted.

Fresh parallel reviewers: gpt-6-astra/high for recovery/effects and archive safety;
gpt-6.1-sol/high for coverage/peers/actual artifact verification. Lead independently
verifies findings and owns production runner/docs/integration/Git. After review,
disjoint coding scopes are recovery unit tests; package checker/tests/consumer
CMake; and native transcript cases. Core/public C++ and SDK/tool pins are unchanged.

Current TMP1x2 bafc34067530eb340e55c42dae3fad4eb795b6e9,24Cxx
936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e,INA228
b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b,OPT4001
dab818e575fc25a0949d962bc0df600eb2ed5200,LDC1614
1db38b63cb93a561b6e0bea51017ab94102ebd24 andINA3221
41b6184b75f3c152611d4236f7ea41a5ce77bc2c are all clean and unchanged. Reviewed
TMP1x2 runner cleanup tests/check_package/version generation,24Cxx hil_runner
and tests/check_package_contents,INA228 test_basic/owner example and public API,
OPT4001 clean package checker,LDC1614/INA3221 native CMake/application sources
and all their AGENTS/workflow definitions. Keep exact owned results, primary
failure, safe conditional cleanup and real renamed consumer/image links; reject
unbounded extraction, automatic replay/restore and consumer version generation.
No checked-in TUSB workflows exist yet; no remote run was queried.

Confirmed P1: a base fault plan ending safely in UFP could carry its original
startup declaration into a later source-capable suite without fresh recovery.
Archived TI SLLSEN9F7.4.3p14 re-opened: after VDD returns,04/wait30ms/00 precedes
Rp. A checkpointed base_recovery_required fact now survives session/group changes;
only newly qualified explicit recovery clears it. Both base variants have red
then green cross-group/combined-group and positive fresh-recovery regressions.
No recovery is automatic and no board power policy was added.

Confirmed P2: compile_commands/directory assertions did not detect an absolute
header from outside extraction. A full copied-checkout experiment returned PASS
while Ninja recorded the escaped dependency. The package check now audits actual
compiler header dependencies against extracted/fixture and detected toolchain
roots, with complete positive and injected-header negative consumers. Remaining
verification and new archive/image evidence follow below.

Final audit verification: strict Debug66/66 and Release66/66 CTests PASS, including
311session/runner tests,39package unit cases and actual complete header-provenance
consumers. Native persistent CLI checks both base variants' retained recovery
obligation across sessions, with zero later callback/command deltas. New CTest
package_origins executes a clean package and two specifically rejected outside
header imports; Ninja, Ninja Multi-Config and real MinGW Make depfiles pass.
Independent final strong review found no further confirmed defect.

Fresh managed-Core archive SHA256
35082d9163f1c0568d267ed057d51d964a3fcdb89143771b601327a4b3099a24 has76source-equal
regular files and173local links. A renamed clean public consumer compiles, links
and runs with no maintainer tools on child PATH, now checking actual headers.
All6fresh extracted Arduino/native-IDF S2/S3 image links and actual dependency
checks pass; unknown Git identity beneath unrelated parent, native APIs/noArduino
and separate build/SDKCONFIG/locks are verified. The lead hashes actual artifacts
and source bytes; detailed commands/paths/versions are in
[validation](validation.md#independent-audit-of-prompts-31-and-32-2026-10-28).
Version stability, strict Doxygen/visibility, Markdown and changed-module complete
Python docstrings/style checks pass. No core API/code, version or SDK pin changed.

Physical serial/I2C/CC/INT/VDD/VBUS/VCONN/reset timing remain NOT RUN. No hardware
or remote CI was invoked. The library still has one transport/composer/engine;
runner readiness and compiler dependency guards add no scheduler or product policy.
The lead reviews/stages/commits and syncs this audited unit on the same branch,
verifying fetched upstream equality,0ahead/behind and no remaining task edits.
Record the completed audit commit in the next progress entry, avoiding a hash loop.

## Prompt33 - local validation and checker failure evidence

Starting revision c71e7ca4997a244359e43a3fd0047071de1ce0e8 completes the prompts31/32
audit. Fetch confirmed equal upstream and0ahead/behind on the same
promptset/tusb32x-implementation-20261023 branch; worktree/index were clean.
Read the original prompt33, COMMON, parent/local AGENTS, AGENT_WORKFLOW,
MAINTAINABILITY, verification/package rules and actual CMake/CTest/guards first.
Initial inventory had66finite tests, one built-fixture cli_transcript and one
hil_runner that already loads six related modules. No workflow exists yet here;
remote CI remains prompt34 work and was not queried.

Fresh read-only peers: TMP1x2 bafc34067530eb340e55c42dae3fad4eb795b6e9,
24Cxx936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e,
OPT4001dab818e575fc25a0949d962bc0df600eb2ed5200,
LDC16141db38b63cb93a561b6e0bea51017ab94102ebd24 and
MCP45HVX1c6c7feefd79b9fc16ddc7b7786ad187e5185fde6; all clean.
Read their AGENTS, checked-in CI and actual TMP boundary/Doxygen/version guards
and tests,24Cxx transcript registration, OPT package/timing/Doxyfile and MCP
tools/validate.py. Adopt finite real parser/HTML/XML checks, sys.executable,
argument lists, the existing managed launcher and single transcript registration.
Reject EXTRACT_ALL/warning suppression, small hardcoded public-type inventories,
successful missing-tool SKIPs, unbounded subprocesses and duplicate full suites.

Delegation used general gpt-6.1-sol/high owners for the Doxygen guard/fixtures,
boundary guard/fixtures and CLI helper comments, and version/registration review
plus extracted firmware helper/fixtures. Shared interfaces and write scopes were
fixed; lead owns validator, CMake, public/core comments, docs and all Git.
A separate gpt-6-astra/high reviewer checks verdict/ownership and hidden APIs.
Version review found the existing18tests already cover sync/check/bump/set,
malformed values, each drift field, idempotent bytes/mtimes and no-parent-Git;
no duplicate suite or release bump is needed.

Implemented one validate.py entry point, explicit disjoint CTest labels,
inventory/actual JUnit checks, finite children, saved partial reports and explicit
native/contracts/doxygen/python/package/arduino/idf-s2/idf-s3 selections. The
firmware path packs once, reuses the hardened package consumer, then links the
extracted shared sources. No HIL, serial probing, flashing, publishing or CI action
is reachable. Static boundaries supplement the existing allocation traps and
independently included headers. New real Doxygen checks exposed missing helper
comments and deleted copy/move member docs; these are fixed with no C++ behavior
change. Independent negative review also closes public hiding through Doxygen
access/append/predefinition settings instead of trusting warning-free output.

Focused negative suites are being run before the combined local validation.
Exact final commands/results and firmware evidence will be appended after
execution; missing physical/alternative-toolchain evidence remains separate.

Prompt33 implementation and local verification are complete. New registrations
bring the inventory to72on Windows/71elsewhere, with finite bounds and disjoint
ownership. Focused negatives pass:37contracts,28real-Doxygen,15validator and17
firmware cases; unchanged version18cases pass. Independent strong review's
hidden-API, explicit-tool, interpreter, registration and output-path findings
are fixed and verified against actual code/fixtures. Eight production/example
C++files changed comments only, with identical executable tokens.

Both strict Debug and Release execute72/72PASS through the corrected entry
point, including the existing311-case runner and live C++ transcript. The first
full invocation retained a real relative-build-tool failure and drove absolute
tool resolution before CMake. The corrected full invocation's final native-S3
SDK LCD compilation then hit a GCC internal segmentation fault; its report
correctly stays FAIL. A separately selected S3 run passed with identical76-file
payload, SDK/compiler/flags; the other5image checks passed in that full run.
All final6ELF/BIN/map records were independently rehashed, with11extracted
project sources each, actual used-header origins, native APIs/noArduino,
unknown Git identity and distinct absolute SDKCONFIG/locks. No automatic retry,
weakened flag, SDK pin change or rewritten failure verdict was introduced.

Actual commands, reports, archive hashes, paths, versions and the intermittent
compiler limitation are in [validation](validation.md#prompt33-execution-2026-10-28).
Full evidence: build/validation/run-u1bhwqzn; explicit S3 evidence:
build/validation/run-llfyeh3g. No single full-run PASS is claimed for the compiler
failure; all required individual software checks have positive evidence. Final
version/Doxygen/Markdown/Python format and scoped whitespace checks pass.
All76exported payload bytes remain equal to the built extractions; no new core
consumer dependency or package-content change was introduced.

Physical serial/I2C/CC/INT/power/timing remains NOT RUN. Other host compilers/
sanitizers retain their documented tool gaps. Remote CI was not queried or
dispatched; no hardware, flashing or publication action occurred. The lead
reviews/stages this coherent unit, commits and syncs the same execution branch,
and reports final commit/upstream equality in the handoff. Record that completed
commit in the next progress update, avoiding a self-reference commit loop.


## Prompt 34: dedicated CI and extracted firmware checkpoint (complete)

Started from synchronized clean fc77aae9f81e908ffaa72f2f83781fcb9cba9750 on
promptset/tusb32x-implementation-20261023; fetch showed zero ahead/behind and no
staged changes. This is the promptset's sole remote checkpoint. Parent/local
AGENTS, COMMON, workflow/maintainability/verification contracts and prompt 34
were re-read. Prior prompt 33 is committed and synchronized at that starting SHA.

Fresh read-only peer audits and official pin sources are recorded in
[audited pins](audit/execution-34-pins.md) and [SDK comparison](audit/execution-34-sdk.md).
TMP1x2, OPT4001, LDC1614, INA3221 and 24Cxx were inspected at their actual clean
revisions; no sibling files or Git state were changed. Adopt separate bounded
jobs, immutable official actions, explicit CTest ownership and actual extracted
image links; reject duplicated source/package matrices and inherited SDK claims.

Parallel agents owned the workflow guard/negatives, official-pin audit and native
SDK comparison/version gate. An independent strong read-only review found POSIX
launcher symlink bugs and missing negative guard cases. Fixes preserve compiler
and virtualenv executable spelling, enforce actual compiler/sanitizer/container
arguments and reject conditionally disabled required steps. The lead owns the
workflow, helper integration, documentation and Git. Current IDF 6.1 is audited
separately from Arduino's retained IDF 5.5.5 dependency.

The workflow declares five strict Linux host rows, one Python/docs/package owner
each, four Arduino images and four native IDF SDK/target images. Firmware consumes
one hashed PlatformIO archive, validated and renamed by the existing hardened
extractor. A new explicit extraction-only mode claims no consumer link. Package
consumer evidence is copied into bounded CI artifacts. Full verbose CTest output,
registration JSON, JUnit, failed logs and firmware ELF/map/identity evidence are
retained seven days. No HIL, flashing, publication or physical fixture is invoked.

Local checks are underway before the implementation commit. The first native
firmware check linked its image but correctly failed inspection: IDF records the
selected compiler in generated compiler metadata, not CMakeCache. The inspector
is corrected and the original failure remains saved. Remote execution is PENDING;
workflow source and local links are not remote PASS. Exact commands/results and
tested SHA/run URL will follow after verification.


Implementation commit f521f55b2d713a2be30686d2c22fe4c1a76b7bc1 was pushed and
final-fetch verified equal to upstream, zero ahead/behind and clean worktree.
Its existing push run [40775037702](https://github.com/janhavelka/TUSB32x/actions/runs/40775037702)
is the first remote checkpoint attempt. Five Linux host rows and documentation
passed; the package gate failed and correctly prevented firmware links. The
failure is being diagnosed before a grouped correction; no failed/skipped row
is reported as passed. The actual tested SHA remains explicit.


The first remote run f521f55 retained FAIL: package consumers linked but their
blanket Ninja dependency count included ELF linker records enabled by CMake 4 /
modern GNU ld. Querying only actual compile_commands object outputs preserves
exact nonempty VALID records and all used-header escape checks. The 41-case
package unit suite and three actual Release package registrations pass locally.

All five Linux host rows passed (53 native plus 6 contracts each), including
Clang ASan/UBSan, and the two real Doxygen checks passed. Python's other six
registrations passed; hil_runner alone hit the old 90-second suite ceiling.
An independent timed run completed all 311 cases successfully in 65.922 seconds
locally, with finite multi-suite artifact tests dominating and no isolated hang.
The independent CPU profile identified duplicate complete JSON serialization
at every checkpoint: 20.56 of 28.7 instrumented seconds in the hottest case,
versus 0.525 seconds in fsync. Checkpoints now reuse their already validated
encoding. Registration saves verbose per-case progress; the original 90-second
suite ceiling, serial/operation deadlines and all evidence checks remain intact. The original failed artifacts remain attached to run40775037702.
The grouped corrections will be checked by the next exact-commit push run.


After the encoding correction, all 312 runner cases pass locally in 56.997 seconds
(previous 311-case baseline 65.922 seconds). All 31 focused artifact cases pass,
including fresh-state/exact-byte, once-per-save encoding and invalid-state checks.
No report limit, atomic replacement order, fsync or failure/correction path changes.
The extra case is owned by the existing hil_runner registration. Package41-case
negatives and actual positive/outside-header Release consumers pass. The original
failed CI reports remain intact; these changes are grouped for the next push.


Correction commit aa377ceee86d7c4f23177e987e8769151471df9f was synchronized
and tested by push run [40776243182](https://github.com/janhavelka/TUSB32x/actions/runs/40776243182).
All five host rows, workflow guard, documentation, Python and package jobs passed.
Each host row executes 53 native plus 6 contracts; docs2/package3/Python7 have
matching inventory/JUnit names, no omitted/skipped tests. Actual host compilers
are GCC 14.2.0 and Clang 18.1.3. Python runs all 312 runner cases in 86.915 seconds
(CTest87.094), under the unchanged90-second ceiling; transcript also passes.

The real 76-file archive SHA256 is
57aeb72a99727f013aab98d3828edb311ceba062596326424a3da06bb9d78cfe,
with tests-OFF consumer PASS at /tmp/package-348po76v/renamed-tusb32x.
Both genuine IDF5.5.5 S2/S3 image rows pass. The lead downloaded artifacts
12854373225/12854693178 and rehashed ELF/BIN/map/SDKCONFIG/lock bytes; each
records11extracted sources, native symbols/noArduino and unknown archive Git.

The run correctly remains FAIL: Arduino stopped before build because its
metadata-only toolchain is materialized by the pinned platform during pio run,
after the helper's premature executable check. Both IDF6.1 images linked, then
header-provenance inspection rejected picolibc/no-rtti headers absent from the
bare compiler probe. These are helper defects, not authority to broaden roots,
remove gates or replace real links with SDK-header compilation. The next grouped
correction uses actual configured compiler evidence and defers Arduino inspection
until platform materialization. Additional generated compiler metadata is retained
for diagnosis. Failed/incomplete artifacts remain unchanged. Physical proof is
still NOT RUN.


The next firmware-helper correction passes23focused cases and an actual existing
5.5.5 S2 image reinspection (11sources/5artifacts), saved separately under
reinspection-abi/result.json. IDF6.1 now requires every extracted source command
to name the exact generated compiler response file, then probes with those ABI
flags and the build directory. Missing, empty, relative, variable, mismatched or
duplicate response references fail. Precise returned roots remain existing absolute
directories; there is no wider installation allowlist. Compiler/probe/root/response
hash metadata is saved, together with flags/specs/generated CMake evidence.
Arduino performs its normal pinned platform build before resolving materialized
nm/compiler executables; the mocked sequencing regression cannot yield image PASS.
Existing Windows paths and managed Core remain unchanged. No SDK/tool pin or
firmware source was changed to bypass these failures.


Third implementation commit0ec200d3e1ab65552b81d4272f71568b147431be was
synchronized and tested by push run [40777727121](https://github.com/janhavelka/TUSB32x/actions/runs/40777727121).
Host/docs/package checks again pass. The final helper guard runs23cases.
The runner's prior312-case PASS took86.915s; this run reached278successful cases
before the90.0102s suite deadline. Verbose output identifies continuing bounded
multi-suite artifact tests, not a stalled first case. This demonstrates insufficient
whole-suite CPU margin. The single registration now permits180seconds and retains
every test, assertion, cap and simulated operation deadline. No retry, test omission
or protocol timeout change is introduced. This replaces the measured inadequate
wall-clock ceiling; prior90-second failures and positive results remain recorded.

Prompt 34 is complete. The final implementation commit
9aa8898c65ca1eebc5f7ea241dfac4bbc815283b is synchronized and its existing
[push run 40778512875](https://github.com/janhavelka/TUSB32x/actions/runs/40778512875)
completed SUCCESS with all 14 jobs PASS. Five strict host configurations execute
53 native plus 6 contracts each; docs 2, package 3 and Python 7 registrations all
pass with exact inventory/JUnit names. The 312-case runner completes in 93.180s
under the measured 180s host suite limit; all operation/assertion checks remain.

The single final archive SHA256 is
c492a92e2d1feb65356283329bc09d8a2e6bfda13a428d081e6e41715d287eb7,
301887 bytes / 76 source-equal files. The clean CMake consumer runs without
maintainer tools. All eight extracted images link and pass provenance: four
Arduino basic/owner S2/S3 images and four genuine native IDF 5.5.5/6.1 target rows.
Independent artifact audits rehash actual images/maps, verify 11 project sources
per image, exact headers/SDK roots, unknown archive Git identity and native
linkage without Arduino. All 15 bounded artifacts exist; seven-day retention is verified.

[Final validation](validation.md#prompt-34-completed-remote-checkpoint) records
commands, actual tool versions, run/job evidence, artifact paths/hashes, prior
failed attempts and corrections. The stale progress-table rows 30-33 are aligned
with their recorded completed software work; later prompts remain NOT STARTED.
This documentation-only reporting unit receives local link/format/version checks
and its own commit/sync. It does not create another remote gate or extend the
9aa8898 CI result to its reporting commit. Physical serial/I2C/CC/INT/power/timing
remains NOT RUN; no flashing, publication or sibling modification occurred.

## Independent audit of prompts 33 and 34, 2026-10-28

Re-read both original prompts completely, COMMON, AGENT_WORKFLOW, maintainability
and verification/package rules. Started at clean `e6d3ba9ab910d672fcf2abda1848ac0838d46912`
on `promptset/tusb32x-implementation-20261023`; fetch confirmed upstream equality
and zero ahead/behind. Reviewed original implementation diffs from `fc77aae`
through `9aa8898`, current sources and actual registered commands. Parallel
reviewers independently checked validator/registration/version behavior,
Doxygen visibility/omissions, and workflow/package/firmware provenance. The lead
verified findings, reviewed their corrections and integrated disjoint files.

Fresh read-only peer evidence:

| Peer revision; clean worktree | Sources re-opened and decision |
| --- | --- |
| TMP1x2 `bafc34067530eb340e55c42dae3fad4eb795b6e9` | AGENTS, CMake, workflow, version generator/tests and Doxygen checker/tests. Keep explicit commands, strict generation and visibility; reject copying a permissive extraction setting or assuming its green SDK rows qualify TUSB. |
| 24Cxx `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | AGENTS, CMake and tools/test_cli_transcript.py. Keep the explicit built fixture path and real C++ exchange; do not substitute recorded text or EEPROM mutation policies. |
| OPT4001 `dab818e575fc25a0949d962bc0df600eb2ed5200` | AGENTS, Doxyfile, workflow/action pins and tools/check_clean_consumer_package.py. Keep actual clean consumers and full action pins; reject warning suppression or SHA-shape checks as sufficient proof. |
| LDC1614 `1db38b63cb93a561b6e0bea51017ab94102ebd24` | AGENTS, Doxyfile and examples/esp_idf/basic/main/CMakeLists.txt. Keep strict warnings and real component linkage; do not copy fixed directory-derived component names or enclosing-Git identity. |
| INA3221 `41b6184b75f3c152611d4236f7ea41a5ce77bc2c` | AGENTS, CMake and examples/esp_idf/basic/main/CMakeLists.txt. Retain native app_main/API evidence; SDK-shaped host fixtures remain a separate evidence level. |

Confirmed corrections remain inside the original validation/package scope:

- Numeric apostrophes no longer mask following C++ code. The original allocation
  bypass compiles as C++17; lexical and runtime allocation checks remain distinct.
- Doxygen rejects alias-based hiding and checks documented pointer/reference and
  trailing-return helpers, including attached pointer/name spelling. It shares
  the boundary lexer's token recognition while preserving XML source positions.
- Final report I/O failure returns incomplete; interrupted report I/O retains
  exit 130 and prior evidence. Command logs retain at most 16 MiB; overflow fails
  even after exit zero. Firmware logs also trim an overshoot after termination.
- Workflow review requires actual ordered scripts, matching archive identity and
  matrix targets. Negative scripts cover omission, echo/no-op, heredoc, duplicate
  arguments and ignored failures. No workflow, SDK pin or firmware source changed.
- Ninja Multi-Config package inspection selects the built configuration and its
  Ninja manifest. The existing package-origin suite now executes its clean
  consumer alongside standard Ninja and both outside-header negatives. Four
  35-second child bounds fit a documented 160-second suite bound.

Final local checks pass: Release 72/72 registrations; Debug native/contracts
59/59; final affected Doxygen 2/2 after the attached-declarator correction.
Inventory remains 53 native, six contracts, two Doxygen, eight Python on Windows,
and three package tests. Assertions and all six strict warning flags are present
in actual Debug/Release commands. Negative suites execute 39 boundary, 33 Doxygen,
43 validator/CI, 24 firmware and 42 package cases. Version tests execute 18 cases;
sync/check changes neither bytes nor mtimes. Full commands and artifacts are in
[validation](validation.md#independent-audit-of-prompts-33-and-34-2026-10-28).

The four official action release tags were re-resolved to the existing SHAs.
The prior exact-SHA run remains independently confirmed successful: 14 jobs at
`9aa8898c65ca1eebc5f7ea241dfac4bbc815283b`, with cached artifact ZIP digests and
32 image/map/config/lock hashes rechecked. All eight images contain 11 extracted
project sources. This audit does not transfer that result to new commits.
The lead will commit/sync this checked correction and inspect the existing
push-triggered checks affected by these validator/tool changes. No manual
dispatch, HIL, flashing, publication or sibling mutation is involved.

The correction was committed as `ac211e03a60f4b70a9005d07d6b6dde6f9927c1e`
and synchronized with clean worktree and zero ahead/behind. Its existing push
[run 40781392477](https://github.com/janhavelka/TUSB32x/actions/runs/40781392477)
passes host/ASan/UBSan, docs, workflow, package and native image checks. The
Python owner exposed a separate registration limit: cli_transcript timed out
at 30.0201s without emitted progress. The other six Python registrations pass,
including all 312 runner cases in 138.589s (earlier checkpoint: 93.180s).
The failed artifact and JUnit remain retained; this is not a green full run.

The single complete transcript registration now has a finite 60-second suite
budget and four flushed phase checkpoints after successful assertions. Existing
command, write, marker, core operation, cleanup and simulated-clock deadlines are
unchanged, as are all cases and assertions. Focused transcript verification and
the next affected push result are recorded in validation; no unrelated source
firmware matrix is run locally.

The measured transcript correction was committed as
`c50f8ddda8809425ede2161586466c95fa650ae6` and synchronized with zero ahead/behind.
Its existing [push run 40782369910](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910)
completed **SUCCESS: 14/14 jobs**, exact tested SHA verified, at 22:50:15 UTC.
All 15 required artifacts exist for that SHA with bounded retention. The actual
Python artifact proves transcript PASS in 17.8099s, all four phase checkpoints
and 248 requests; runner 312 cases pass in 94.474s, validator/CI 43, firmware 24
and version 18. Native/sanitizer/docs/package checks and all eight extracted
firmware links pass. No manual dispatch or job rerun was needed; the initial
failed run remains separately recorded.

The final archive is 302,545 bytes / 76 files, SHA256
`654a5213022bc570eeb346f56d084cdbdaaf75cdbc5928406e25450fa6cf0240`.
Final Python/package artifacts are independently checked; all 71 Linux test
names/owners match and no Python JUnit case is skipped. Prior 32-file independent
image rehashing stays tied to `ac211e0`, not reassigned to this new SHA. The
final checked image jobs inspect their own extracted sources, headers, symbols
and artifacts. Evidence is under build/audit-33-34-final-ci and the lead's
ci-final.json; validation records actual commands, results and remaining limits.

Audit complete: confirmed defects fixed, full scoped diff reviewed, local checks
and final CI passed. This final reporting unit changes documentation only and
receives link/version/whitespace checks plus its own commit/sync. It does not
create a further remote gate or extend the tested SHA to a reporting commit.
Physical serial/I2C/USB/CC/INT/supply/timing qualification remains NOT RUN.

## Prompt 35: user guides and feature coverage (2026-10-29)

Started from `45da5296792ee4a38120e9c4a65f23ce6951adce`, the completed
prompts 33/34 audit reporting commit. The tree and index were clean; fetch
confirmed equality with the same-name origin branch and zero ahead/behind.
The execution branch and original base remain unchanged. The recorded CI proof
stays at `c50f8ddda8809425ede2161586466c95fa650ae6`, run 40782369910;
this prompt neither requests nor inspects another remote run.

Fresh review covered COMMON, AGENT_WORKFLOW, MAINTAINABILITY, prompt 35, current
public headers, core operation/transport/diagnostics code, named runtime cases,
the shared CLI/runner schemas, actual help/parity behavior, manifests and existing
workflow commands. Three agents owned disjoint owner/API, CLI/example and
silicon/coverage guide edits. An independent reviewer rechecked the silicon
changes, source hashes, named tests and links; the owner reviewer checked the
integrated README, extracted snippet, test dependencies and manifest changes.
Only the lead handles Git. No sibling was modified.

Re-audited peer revisions were clean:

| Peer | HEAD | Concrete reviewed sources and choices |
| --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | AGENTS, docs/README.md, docs/integration.md, docs/hil-runner.md, examples/README.md, examples/common/BoardConfig.h, examples/esp_idf/basic/main/CMakeLists.txt and CI. Adopt topical navigation and explicit firmware identity/console routing; reject inherited repeated-START, numeric DTR, 32-bit time and profile replay. |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | AGENTS, docs/I2C_INTEGRATION.md, include/LDC1614/LDC1614.h, src/LDC1614.cpp, test/test_basic.cpp and CI. Adopt desired/actual separation, lifetimes and copied evidence; reject its queue/ID semantics, same-timestamp multi-callback assumptions, failed-delivery output resets and NACK no-effect inference. |
| INA228 | `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | AGENTS, docs/README.md, docs/CLI.md, tools/run_i2c_hil.py, tools/test_run_i2c_hil_parser.py, basic example README and CI. Adopt one command reference and framed provenance; reject scans, startup recovery and human-substring verdicts. |
| FieldCore-node | `5ad9292dc8481cf08be9a61f662495a6474d1da0` | AGENTS, docs/guidelines/ownership.md, include/TunnelMonitor/i2c/{I2cTask,I2cOwnerTransport,I2cDeviceBinding,I2cLifetime}.h, contracts/Time.h, core/TimeService.h and src/i2c/I2cTask.cpp. Adopt serialized service/lifetimes and monotonic half-range time; reject product queues, richer completion/effect metadata and milliseconds presented as measured microseconds. |

Reopened all eight archived TI PDFs and eight pinned public implementation/
regmap source files. PDF hashes and public-source bytes/SHA-256/Git blob IDs
match the provenance manifests; temporary reviewed bytes and extracted pages are
under `build/prompt35-silicon`. Exact printed B pp11/14, L p14, A pp4/5/14/19,
V pp4/6/9/13/18-20, N pp3/5/9 and G pp8-9 preserve current-refresh/reset,
recovery, role waits, supply/VBUS_DET, enable conflict and VCONN min/typ/max
boundaries. Public source examples remain counterexamples, not silicon authority.
No unresolved feature gate or revision-06 policy was changed.

README and the docs index now describe the delivered library and lead to one
owner workflow, exact API/state tables and the authoritative CLI reference.
Removed redundant API/procedure inventories and stale planned-feature claims.
Coverage maps features to API, CLI, named runtime tests and exact TI evidence.
Corrected remaining failed-write wording: Status-only NACK/BUSY cannot erase
possible acceptance. Native SDK 5.5.5/6.1.0 commands use isolated configuration
paths. The native frontend's unknown callback-kind counters visibly block full
readonly/soak proof without hiding its usable manual CLI observations.

The literal README C++ fence is extracted by CMake and executed in the existing
contract registration. Nine simulated paths cover success, invalid inputs,
transport/identity failure and immutable deadlines. The host-only fixture adds
no production code, public hook, framework dependency or new test registration.
Both literal manifests now ship the new owner guide (77 files); all repository-
only evidence links remain valid outside the archive.

Local evidence is detailed in [validation](validation.md#prompt-35-user-guides-and-compiling-examples).
Debug/Release README checks, five relevant prior core suites, CLI parity and its
negative check, standalone tests-OFF host execution, version, Doxygen visibility,
boundary and all three package registrations pass. The final managed archive
and renamed consumer pass with 206 packaged links; 139 repository Markdown files
have 838 valid local links/fragments. Initial new package-link failures were
fixed and the affected checks rerun. Registration remains 72 finite Windows
tests with one owner each. Three documented portless plans save plan PASS and
hardware NOT RUN/exit 2. No full firmware matrix, HIL, physical action, release
or new CI checkpoint is required or claimed for these guide changes.

Full scoped review and local checks are complete. Commit/sync evidence belongs
in the handoff and next progress update; no self-referencing commit ID is added.

## Prompt 36: source documentation and maintenance (2026-10-29)

Started from synchronized `e37f0a35aa0cbad5cf6a4f46d22032e2adce253d`, the
completed prompt-35 guide/snippet/package unit. Status and index were clean;
fetch confirmed the same execution branch and zero ahead/behind. Original base
and branch are unchanged. Prompt 37 has not started. The earlier remote result
remains attached only to `c50f8dd` / run 40782369910; no new CI status was queried.

Read COMMON, AGENT_WORKFLOW, MAINTAINABILITY, prompt 36, selected baseline
Status/transport/result/effect/time/lifetime contracts, current test registration,
workflow commands, metadata and license. Delegation used gpt-6-astra/high for
core/silicon/lifetime review and two gpt-6.1-sol/high agents for disjoint example
C++ and Python/workflow reviews. Lead owned maintainer guides, checker integration
and Git. Independent final reviews checked maintainer-policy preservation and
the user-facing changelog; all source findings were verified against code/tests.

Fresh read-only peer evidence:

| Peer, clean HEAD | Exact sources and selected/rejected conventions |
| --- | --- |
| LDC1614 `1db38b63cb93a561b6e0bea51017ab94102ebd24` | Doxyfile, AGENTS.md, CONTRIBUTING.md, scripts/generate_version.py and .github/workflows/ci.yml. Retain strict undocumented/parameter warning failure and coordinated guides; direct rendering plus a landing-page check cannot establish complete API visibility. |
| TMP1x2 `bafc34067530eb340e55c42dae3fad4eb795b6e9` | Doxyfile, tools/check_doxygen.py, tools/test_check_doxygen.py, AGENTS.md, scripts/generate_version.py, CMakeLists.txt and CI. Retain HTML/XML visibility, private exclusion, finite child deadline and links; reject EXTRACT_ALL=YES and synthetic XML as proof of real-parser omission detection. No CONTRIBUTING.md exists. |
| 24Cxx `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | AGENTS.md, scripts/generate_version.py, library.json, platformio.ini, CMakeLists.txt, CI and test/core_no_arduino/compile_main.cpp. Retain manifest truth, stable checked generation and existing managed Core; reject consumer-time package generation/firmware metadata injection. No CONTRIBUTING.md or dedicated version test exists. |
| OPT4001 `dab818e575fc25a0949d962bc0df600eb2ed5200` | AGENTS.md, CONTRIBUTING.md, scripts/generate_version.py, tools/check_version_header_contract.py, library.json, platformio.ini, CMakeLists.txt and CI. Retain ownership/check conventions; reject import-time sync, unbounded Git child, incompatible aliases/numeric-only version assumptions, broad test discovery and unrelated branch/release rules. |

All five public headers, eight private core files and 23 example C++ files were
inspected. Generated Version.h remains correct and was not hand-edited. Reopened
archived B pp7/14/16/18-19, L/A pp6/14/18-19 and V pp6-7/9-11/18-20 before TI
citation corrections. Useful comments already covered most invariants; changes
clarify static-lifetime callback Status text, initialization versus trust, fresh
wait anchors including reconciliation, 322 fault bit 3/mask 08, exact TI timing,
adapter borrowing/configuration and CLI callback waits versus scheduler sleeping.
No public alias/API, silicon gate, mask, wait duration or executable behavior changed.
The selected baseline's optimistic address-NACK phase exception was corrected to
match the existing conservative Status-only acceptance path and runtime cases.

Reviewed all 34 Python modules, including lead-owned Doxygen guard/fixtures.
Expanded meaningful input/output/deadline/file/exit contracts in identity helpers,
checkers and session/plans. The Doxygen child has a 120-second deadline; local
page/source checks are not falsely claimed to share that whole-command bound.
Initial serial open remains explicitly outside the run deadline with no portable
open-timeout guarantee. Core consumers gained no generator/tool dependency.

AGENTS and CONTRIBUTING now link maintained responsibilities rather than repeat
the implementation diary. Commands, delegation, same-branch lead-only Git,
authorized sync, SDK pins, managed Windows Core, local/remote CI boundary and
release authorization are preserved. CHANGELOG now records meaningful Unreleased
features/fixes/documentation, with execution history retained in progress and
validation. License, ownership, manifests, Doxyfile and generated versions agree;
no version bump or warning/extraction suppression was needed.

[Validation](validation.md#prompt-36-source-documentation-and-maintainer-guidance)
records 60 disjoint Release CTests and four focused Debug core checks, all PASS.
The real Doxygen guard checks 36 C++ files; 33 existing negative cases reject
omissions, hidden APIs and broken refs. Generated pages/XML independently show
16 principal types, 41 public function entries with applicable parameter/return contracts
and all 23 Err values. Private extraction remains NO. Existing registry retains
72 finite Windows tests, one owner each; Release assertions remain enabled.
Exact C++ tokens/literals and Python executable ASTs are unchanged. Clean renamed
tests-OFF consumer, independent headers, version fixtures, CLI transcript/parity
and final link/metadata checks pass. No firmware matrix, physical HIL, flashing,
publication or sibling mutation was performed or inferred.

The complete scoped unit is reviewed and ready for authorized commit/sync.
Final commit/equality evidence belongs in the handoff and next progress update.

## Independent prompts 35–36 audit (2026-10-29)

Started at `6e96f8efb62b19f78c4e1951b6ed71fa2c574e21`, clean index/tree,
equal to the same-name upstream after fetch with zero ahead/behind. That commit
completed prompt 36; `e37f0a35aa0cbad5cf6a4f46d22032e2adce253d` completed
prompt 35. Read both original prompts in full and inspected their actual
`45da529..HEAD` diff, current declarations, implementation and tests. The branch
remains `promptset/tusb32x-implementation-20261023`; prompt 37 has not started.

Three fresh-context reviewers worked in parallel: gpt-6.1-sol/high for guide/
coverage review, gpt-6.1-sol/high for tooling/metadata and gpt-6-astra/high for
source, silicon, ownership and timing. Shared contracts were unchanged. Disjoint
source/guide and firmware-helper corrections were delegated after verification;
the lead owned checker/validator integration, evidence, review and Git. A separate
read-only review checked the lead's fixes against the actual code and tests.

Fresh read-only references, all clean when inspected:

| Peer HEAD | Exact inspected sources and decisions |
| --- | --- |
| TMP1x2 `bafc34067530eb340e55c42dae3fad4eb795b6e9` | AGENTS.md, docs/README.md, Doxyfile, tools/check_doxygen.py, tools/test_check_doxygen.py, scripts/generate_version.py, CMakeLists.txt and .github/workflows/ci.yml. Retain guide navigation, visibility/negative fixtures and manifest truth; reject EXTRACT_ALL=YES as omission evidence. No CONTRIBUTING.md exists. |
| LDC1614 `1db38b63cb93a561b6e0bea51017ab94102ebd24` | Doxyfile, AGENTS.md, CONTRIBUTING.md, scripts/generate_version.py and .github/workflows/ci.yml. Retain strict warnings and coordinated owner/API guidance; a quiet renderer is insufficient visibility proof. |
| INA228 `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | AGENTS.md, docs/README.md, docs/CLI.md and docs/validation/hardware-validation-procedure.md. Retain explicit command provenance and physical prerequisites; reject sensor assumptions for Type-C fixtures. |
| FieldCore-node `5ad9292dc8481cf08be9a61f662495a6474d1da0` | AGENTS.md; include/TunnelMonitor/i2c/I2cOwnerTransport.h, I2cTask.h, I2cDeviceBinding.h and I2cLifetime.h; include/TunnelMonitor/contracts/Time.h; src/i2c/I2cTask.cpp. Retain serialized ownership and fresh time; its richer completion/effect evidence and millisecond-derived microsecond clock do not survive a Status-only bridge. |
| 24Cxx `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | AGENTS.md, scripts/generate_version.py, library.json, platformio.ini, CMakeLists.txt, CI and test/core_no_arduino/compile_main.cpp. Retain managed Core/stable generation; reject consumer-time metadata injection. No CONTRIBUTING.md exists. |
| OPT4001 `dab818e575fc25a0949d962bc0df600eb2ed5200` | AGENTS.md, CONTRIBUTING.md, scripts/generate_version.py, tools/check_version_header_contract.py, library.json, platformio.ini, CMakeLists.txt and CI. Retain ownership/version checks; reject volatile versions and unrelated branch/release policy. |

Verified all eight archived TI PDF hashes and eight pinned public-code source
caches against provenance. Reopened TI timing/register pages: SLLSEN9F pp7/11/14/
16/18–19, SLLSEP2D/SLLSEQ8D pp6/14/18–20 and SLLSEO5D pp4/6–7/9–11/19–20.
Reviewed every maintained public/core/example C++ file and all 34 Python modules,
plus generated API pages and the actual behavioral cases for unchanged outputs,
ACK preservation, no-op apply, timing anchors, cancellation and result ownership.

Confirmed corrections:

- `irq service` also publishes a new port sample before ACK/recheck; the register
  guide's old list omitted it. Source and runtime service assertions agree.
- Both Arduino and native-IDF front ends omit optional callback-kind counters.
  Complete readonly/soak plans are BLOCKED on both, as the production guard and
  existing no-initialization regression prove. Guides and example READMEs now
  disclose this; ordinary CLI observations remain available.
- The real generated API exposed five private `TUSB32x::detail` structs despite
  EXTRACT_PRIVATE=NO. Source-only extraction boundaries now keep those private,
  while public decodeRegister remains visible. Ten nontrivial private definitions
  gained adjacent invariant comments. New real-Doxygen fixtures reject leaked
  private types and still enforce file/helper documentation when excluded.
- Firmware-check and validator child helpers could accept exit zero observed
  after their original deadlines. Remaining-budget waits and post-completion
  checks now reject equality/late success. Deterministic before/equal/late cases
  retain logs and check the original budget. Process creation and synchronous
  trusted-input writes retain their disclosed backend limits.

No C++ executable token/literal, public API, silicon rule, package version,
source list or SDK pin changed. Python executable changes are confined to the
two deadline helpers and private-API guard, with tests. Warning checks and public
visibility gates were preserved. No extra framework or checker was introduced.

[Fresh validation](validation.md#independent-prompts-3536-audit-2026-10-29)
records the passing focused checks, initial probes/failures and corrected output.
Final generated pages show 18 principal types, all 23 Err values and applicable
parameter/return contracts with no private core compound. Registration remains
72 finite owned tests; Release assertions and strict warnings stay active.
No firmware matrix or remote CI was queried/repeated; the earlier result belongs
only to `c50f8ddda8809425ede2161586466c95fa650ae6` / run 40782369910.
Hardware qualification remains NOT RUN. Commit/sync evidence belongs in the
handoff and next progress update, without a self-referencing hash update.

## Prompt 37 — independent core review (2026-10-29)

Started clean at `bdfcc144f292d4e1cdaecea58796aa208a0bec66`, the completed
prompts35–36 audit, on `promptset/tusb32x-implementation-20261023`. Fetch verified
equal upstream and zero ahead/behind. Read current root/parent guidance, COMMON,
prompt37, baseline, verified register/procedure outputs and actual source/tests.
Prerequisites1–36 were complete; prompt38 was not started.

Separate read-only gpt-6-astra/high agents audited protocol/silicon and owner/
time/portability, with gpt-6.1-sol/high refreshing inventory and peer evidence.
The lead independently reviewed transport/raw diagnostics, status/accounting,
source/test correspondence and integrated reports. No new executable core defect
was reproduced; no speculative abstraction, duplicate test or production change
was added. Confirmed documentation findings corrected the stale current inventory
and nonexistent validateBinding name. Source Doxygen/API remained accurate and
unchanged. Findings use the required severity/evidence/fix/regression/limit table
in [final-audit](final-audit.md), including separate physical gaps.

Re-enumerated 47 Projects roots and the same 17 independent I2C libraries. Six
peer HEADs advanced but inspected source/test/build scopes remain unchanged;
all 17 were clean on main. FieldCore is clean on rodtester at 5ad9292. Exact changed
revisions, exclusions and reopened owner/transport/health comparisons are in the
final audit. Siblings remained read-only; no remote synchronization inferred.
Current TI revision labels remain B/F and L/A/V/D. Eight PDF hashes/sizes and
pinned public-code helper evidence were checked. Reset08/Try.SRC06/ACK masks,
322 reset-to-DRP, unresolved access/fault gates and base explicit current-reset
policy remain supported by source rather than emulator assumptions.

Maintained validator PASS: native 53 + contracts 6 + doxygen 2 = 61 disjoint CTests
in each fresh Debug/Release build. Strict warnings and Release assertions remain
active; isolated consumer checks allocation/failure sentinels and no maintainer
tool dependency. Independent owner review also built seven focused Release cases.
[Validation](validation.md#prompt-37-independent-core-review) records exact commands,
logs, limits and documentation checks. This review changes only docs; prompt38
retains integration/distribution/final handoff. Hardware, flash/serial/HIL and
new remote CI are NOT RUN. Prior remote PASS remains attached only to c50f8dd /
run40782369910. The lead commits/syncs the checked review on the same branch and
reports final equality in the handoff, without a self-referencing hash loop.

## Prompt 38 ? final integration/distribution review (2026-10-29)

Resumed clean at prompt 37 commit `e12ecb74c2a6870665709233cbfaaa5549a804e5`;
fetch verified the same execution branch and upstream equality. Prerequisites
1?37 were complete. Separate gpt-6.1-sol/high CLI/session and distribution/docs
reviewers plus gpt-6-astra/high adapter/lifetime review began read-only. Refreshed
peer paths/revisions and findings are in [final audit](final-audit.md).

Closed two independently reproduced integration gaps. Both frontends now expose
primary-target callback-kind counters required by readonly/soak plans, without
substituting SDK calls or physical transaction totals. Arduino now fences input
when its audited USB RX queue may have dropped bytes; an invalid suffix can no
longer disappear and turn a retained prefix into a reset command. Existing work,
results and output continue. The lead checked actual SDK loss paths and source
changes. Disjoint source ownership transferred sequentially between agents;
no sibling changes, new scheduler, transport/composer, retry or protocol API.

Real application cases cover counter scope, failures, saturation, end/bind,
prewarming/secondary exclusion and the RX dequeue race. RX cases failed before
the correction (five and four assertions, including unintended reset/write), then
passed. Registered two finite scenarios; validator/CI count checks now enforce
74 Windows tests / 73 elsewhere. Corrected live docs and the top progress ledger;
dated evidence stays intact. The independent final diff reviewer found no further
executable defect after the fixes.

[Final validation](validation.md#prompt-38-final-integration-and-distribution-review)
combines 63 fresh native/contract/documentation tests per Debug/Release with the
11 Python/package tests per configuration and the affected validator rerun. Four
current extracted Arduino images and two native IDF 5.5.5 images pass; each has
11 source files and checked compiler dependency/artifact provenance. Native
sources were byte-equal to their earlier extraction after the independent Arduino
fix, so those images were reused without an identical rebuild. Final packaged
prose is checked separately against the same compilation inputs.

All locally reproduced implementation defects are closed. A new IDF 6.1 S2/S3
link remains BLOCKED: this Windows environment has SDK 5.5.5 only, no Docker and
no installed WSL. No new 6.1 binary qualification is claimed. The existing 14-job
CI PASS belongs only to `c50f8ddda8809425ede2161586466c95fa650ae6`, run40782369910.
No remote job/status request, release/tag, hardware access or flash occurred.
The development version remains 0.1.0-dev.1; silicon gates and exact-board
qualification remain visible. Promptset implementation/reviews are finished
with these explicit validation limits. Final commit/sync equality is reported
in the handoff, without a self-referencing hash update.


## Pre-board design preparation ? 2026-10-29

User requested a board-design checklist, first-board bring-up plan and research
into unresolved TI questions; no board design exists yet. Started clean at
732026b3e93046c3f241f8581592d8486a2232f6 on the existing execution branch,
with fetched upstream equal (0/0). This is documentation maintenance after
prompt38, not another implementation stage. No tool installation was requested.

Added source-only board-design-checklist, bring-up-plan and ti-clarification
worksheets, indexed without expanding the 77-file distribution. Reused the
maintained electrical, adapter and runner contracts; no peer implementation
convention or software behavior changed. An independent read-only TI reviewer
(inherited runtime model) checked current official PDF/E2E evidence and the new
guides. No message was sent to TI. Questions explicitly identify a pre-hardware
project and preserve all current silicon gates.

Corrected the current variant guide: 322I's "once in" Attached.SNK wording is
ambiguous, not proof of LAI's one-report interpretation. No current-tracking
claim was enabled. Current official documents remain SLLSEQ8D/SLLSEO5D/SLAAE24;
archived provenance is unchanged. Exact sources and bounded search limitations
are in ti-clarification.md. Prior final commit732026b remains the software/build
baseline; final sync for this documentation unit is reported in the handoff.

Checks: all documentation links, actual runner dry-run exit2/zeroI/O, static
contracts, generated version and archive extraction/link checks passed. See the
new validation entry. Firmware/host behavioral suites were not repeated for
unchanged code. IDF6.1/Linux sanitizer tools and physical evidence remain missing;
no CI dispatch, port opening, flashing or tool installation occurred.
