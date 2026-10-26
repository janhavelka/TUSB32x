# Changelog

This checkout contains audited implementation contracts and passive C++17 values.
Entries below distinguish those artifacts from planned device-driver functionality.

## Unreleased

### Added

- Portable C++17 CMake target `TUSB32x`, pure binding/profile validators with
  distinct invalid/unsupported results, registered native contract/validator
  checks and a standalone multi-file host consumer. Tests default OFF; ordinary
  linkage needs no Git, Python, Doxygen, framework or sibling dependency.
- Native build/consumer instructions and contributing guidance. Device operations,
  metadata/version tooling, firmware and hardware qualification remain planned.

### Changed

- Independently audited prompt 7 against its original requirements, source diff
  and current peers. Repeated native, consumer and documentation checks; corrected
  a TI table citation, repaired damaged evidence ranges and clarified Release
  selection for Ninja consumers.
  The validators and build behavior needed no correction.

- Independently audited prompt 6's contracts, values and fixtures against the
  original requirements and current peer code. Repeated local checks and added
  the reproducible fractional-time verification command; no code or API-contract
  correction was needed.

- Finished owner admission, state, time, publication and health contracts, with
  explicit result/effect retention and future failure vectors. Separated identity
  readiness from configuration trust to allow explicit interrupted recovery
  after rebind and read-only initialization. No driver methods are published yet.

- Independently audited prompt 5's actual headers, contracts and fixtures against
  the original requirements and current peers. Repeated strict compilation,
  linked Status tests, Doxygen and documentation checks; no code or API correction
  was needed. Recorded the review evidence and remaining implementation scope.

- Independently audited prompt 4 against TI sources, current peers and the
  committed diff. Made reset/ACK formulas explicit by variant, fixed the ACK
  table's unescaped bitwise-OR delimiter, and repeated procedure, timing,
  provenance and documentation checks; no silicon procedure
  or transfer bound changed.

- Independently reviewed prompt 3 against TI tables, source code and literal
  vectors. Clarified counted reads before recovery rejection and corrected the
  PCA9555 runner's evidence-retention description. Register encodings and silicon
  capability gates are unchanged.

- Independently reviewed prompt 2 against TI tables, pinned public sources and
  current peers. Added the TUSB322I VCONN supply prerequisite and extended its
  fault gate to all shared-register writes; completed missing visual table checks
  and clarified the TMP1x2 ARA source reference.

- Verified family capabilities against current TI sources and pinned public code.
  Corrected the TUSB322I VCONN resistance to a maximum, recorded LI/HI product
  recommendations and exact electrical conflicts, and selected strict revision
  acceptance without inferred capabilities.

- Independently reviewed execution prompt 1 and corrected peer console/lifetime
  claims, an API source citation, Doxygen fixture expectations and validation
  provenance. The chosen baseline and later prompt scopes remain unchanged.

- Audited all 38 prompts and shared contracts for correctness and missing
  requirements. Corrected TUSB322I role sequences and base attached-mode limits;
  specified raw profile validity, target association, reinitialization, no-op
  apply, time boundaries, callback error normalization and fixed port sampling.
- Added explicit CLI evidence forgetting and atomic variant/address selection,
  runner ownership/local-state restoration, SDK timeout and synchronous-handle
  checks, isolated IDF configuration/component discovery and Windows-safe archives.
- Kept the 38 bounded prompt scopes, local validation policy, source documentation
  and per-prompt commit/sync requirements; added a source-backed findings report.

- Confined remote CI execution and status checking in the first implementation
  promptset to prompt 34. Other prompts and final-review fixes use local checks;
  commit/sync requirements remain, and older CI runs retain their actual SHA.

- Require exact-source verification of inherited chip facts, explicit conflict
  handling and corrections across contracts, code, tests, Doxygen and guides.
- Add targeted public-code failure cases and TI electrical/timing/variant facts
  to the relevant numbered prompts while retaining the 38-task structure.

- Split the original fifteen stages into 38 complete prompts numbered 1–38,
  with dependency-aware scopes, tests/docs/delegation and per-prompt commit/sync.
- Shortened the common entry instructions and separated technical rules by topic;
  added explicit incremental API, prerequisite and incomplete-session boundaries.
- Corrected chip/protocol, snapshot, reconciliation and CLI result/correlation
  instructions after an independent promptset audit; added explicit original
  TUSB320/I dead-battery recovery to the planned API.
- Simplified repeated diagnostics into host-runner sequences and reduced duplicate
  firmware state, grammar, documentation, audit and build requirements.
- Clarified branch lifetime: create one new branch for each new promptset
  execution, then retain it through all prompts, sessions and agents.
- Expanded AGENTS.md using current sibling Git, CI, ownership and engineering
  rules, adapted to the future TUSB32x library and its concrete contracts.
- Required commits and verified upstream synchronization after every prompt and
  at coherent completed units within prompts, on one branch for the promptset.
- Expanded the fifteen implementation stages with concrete peer source references,
  selected API/error/state names, file responsibilities and test expectations.
- Made readable, simple code and plain engineering language requirements for
  implementation, comments, documentation, CLI messages and agent handoffs.
- Required source Doxygen and affected README, changelog and guide updates during
  every stage; the documentation stage now checks consistency rather than starting
  documentation at the end.

### Added

- Compile-only passive observation, explicit apply/cancellation and two-device
  owner examples, plus independent unsigned-time and clock-extension vectors.

- Implemented portable C++17 Status, callback/settings, operation/snapshot,
  diagnostic/health and register-decode result values with complete Doxygen.
- Added the exact planned API inventory, decoder boundary and architecture guide,
  plus reusable standalone-header, constexpr and multiple-translation-unit checks.
  Future driver methods remain outside delivered headers until implemented.

- Completed prompt 4 transport and device procedures: exact role changes,
  one-shot reset, original-part recovery, IRQ service, wait anchors, callback/byte
  ceilings and independent failure/time vectors. Added explicit internal-register
  qualification and retained-effect rules; unresolved silicon gates remain.

- Completed prompt 3 register access, field encodings and independent positive/
  negative vectors. Distinguished partial TUSB320/I defaults from complete
  extended reset tables, protected internal bits from reserved read-only bits,
  and readable profile settings from write permissions. Preserved TUSB322I
  access gates and equivalent DRP encodings; no driver behavior is claimed.

- Completed prompt 2 variant/source boundary and feature-to-test matrix, with
  named unresolved gates and board prerequisites. Added the separate TUSB321AI
  source and original-terms/current-verification metadata for all eight TI PDFs.

- Completed execution prompt 1: current inventory of 17 I2C libraries and the
  FieldCore consumer, source-backed engineering decisions, local tool inventory
  and a 38-prompt progress ledger on one recorded execution branch. Driver code
  and later prompt work remain unimplemented.

- Shared chip-evidence reference and TI-first public implementation audits,
  including reset-mask, reserved Try.SRC, error-preservation and IRQ examples.
- Provenance for 13 pinned public source files from six repositories, including
  ownership/license evidence, commit/blob IDs, byte counts and SHA-256 hashes.
- Additional TI evidence for addresses, identity, register access, supplies,
  physical timing, current refresh, 322 reset-to-DRP and unresolved source claims.

- Engineering baseline for Status, owner methods, transport/health evidence,
  configuration trust, results/effects and public naming conventions.
- Shared CLI/tooling contract with concrete grammar, output records and verification.
- Complexity-based subagent/model selection and independent review guidance.
- Doxygen omission checks, source documentation rules and explicit simplicity gates.

## Initial planning — 2026-10-23

- Audited seventeen sibling I2C libraries and FieldCore-node as a consumer.
- Prepared the original fifteen-stage promptset and recurring-audit contract.
- Archived seven official TI references with retrieval metadata and SHA-256 hashes.
