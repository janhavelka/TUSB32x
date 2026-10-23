# Implementation progress

Execution started: 2026-10-23 (Europe/Prague).
Branch: `promptset/tusb32x-implementation-20261023`.
Base: `ca0cfe9fa9471a2bf327a6957c60ebd5b2c86b35` (main and origin/main equal after fetch).
Keep this branch for this entire execution, including resumed sessions.
Current authorization: execute prompt 3 only; do not start prompt 4.

## Prompt status

| Prompt | Status |
| --- | --- |
| [1: ecosystem-audit](prompts/1-ecosystem-audit.md) | COMPLETE |
| [2: family-capabilities](prompts/2-family-capabilities.md) | COMPLETE |
| [3: register-contracts](prompts/3-register-contracts.md) | COMPLETE |
| [4: silicon-procedures](prompts/4-silicon-procedures.md) | NOT STARTED |
| [5: value-api-contracts](prompts/5-value-api-contracts.md) | NOT STARTED |
| [6: owner-state-contracts](prompts/6-owner-state-contracts.md) | NOT STARTED |
| [7: host-build](prompts/7-host-build.md) | NOT STARTED |
| [8: metadata-versioning](prompts/8-metadata-versioning.md) | NOT STARTED |
| [9: transport-identity](prompts/9-transport-identity.md) | NOT STARTED |
| [10: register-codecs](prompts/10-register-codecs.md) | NOT STARTED |
| [11: owner-engine](prompts/11-owner-engine.md) | NOT STARTED |
| [12: profile-observation](prompts/12-profile-observation.md) | NOT STARTED |
| [13: profile-application](prompts/13-profile-application.md) | NOT STARTED |
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
