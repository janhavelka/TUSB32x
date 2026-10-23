# Changelog

This checkout currently contains library planning and implementation prompts.
Entries below describe those artifacts, not released driver functionality.

## Unreleased

### Changed

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
