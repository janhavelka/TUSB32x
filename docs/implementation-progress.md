# Implementation progress

Execution started: 2026-10-23 (Europe/Prague).
Branch: `promptset/tusb32x-implementation-20261023`.
Base: `ca0cfe9fa9471a2bf327a6957c60ebd5b2c86b35` (main and origin/main equal after fetch).
Keep this branch for this entire execution, including resumed sessions.
Current authorization: independently audit completed prompt 13; later prompts remain separate.

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
| [14: reset-recovery](prompts/14-reset-recovery.md) | NOT STARTED |
| [15: port-snapshots](prompts/15-port-snapshots.md) | NOT STARTED |
| [16: interrupt-service](prompts/16-interrupt-service.md) | NOT STARTED |
| [17: diagnostics-health](prompts/17-diagnostics-health.md) | NOT STARTED |
| [18: protocol-conformance](prompts/18-protocol-conformance.md) | NOT STARTED |
| [19: owner-portability](prompts/19-owner-portability.md) | NOT STARTED |
| [20: cli-foundation](prompts/20-cli-foundation.md) | NOT STARTED |
| [21: cli-operation-control](prompts/21-cli-operation-control.md) | NOT STARTED |
| [22: cli-command-coverage](prompts/22-cli-command-coverage.md) | NOT STARTED |
| [23: arduino-transport](prompts/23-arduino-transport.md) | NOT STARTED |
| [24: arduino-examples](prompts/24-arduino-examples.md) | NOT STARTED |
| [25: idf-transport](prompts/25-idf-transport.md) | NOT STARTED |
| [26: idf-cli-example](prompts/26-idf-cli-example.md) | NOT STARTED |
| [27: generic-owner-consumers](prompts/27-generic-owner-consumers.md) | NOT STARTED |
| [28: python-session](prompts/28-python-session.md) | NOT STARTED |
| [29: observation-runners](prompts/29-observation-runners.md) | NOT STARTED |
| [30: port-fixture-suites](prompts/30-port-fixture-suites.md) | NOT STARTED |
| [31: recovery-fixture-suites](prompts/31-recovery-fixture-suites.md) | NOT STARTED |
| [32: package-consumers](prompts/32-package-consumers.md) | NOT STARTED |
| [33: validation-guards](prompts/33-validation-guards.md) | NOT STARTED |
| [34: ci-matrix](prompts/34-ci-matrix.md) | NOT STARTED |
| [35: user-documentation](prompts/35-user-documentation.md) | NOT STARTED |
| [36: source-maintainer-docs](prompts/36-source-maintainer-docs.md) | NOT STARTED |
| [37: final-core-review](prompts/37-final-core-review.md) | NOT STARTED |
| [38: final-integration-review](prompts/38-final-integration-review.md) | NOT STARTED |

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
