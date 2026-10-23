# Changelog

This checkout currently contains library planning and implementation prompts.
Entries below describe those artifacts, not released driver functionality.

## Unreleased

### Changed

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

- Engineering baseline for Status, owner methods, transport/health evidence,
  configuration trust, results/effects and public naming conventions.
- Shared CLI/tooling contract with concrete grammar, output records and verification.
- Complexity-based subagent/model selection and independent review guidance.
- Doxygen omission checks, source documentation rules and explicit simplicity gates.

## Initial planning — 2026-10-23

- Audited seventeen sibling I2C libraries and FieldCore-node as a consumer.
- Prepared the original fifteen-stage promptset and recurring-audit contract.
- Archived seven official TI references with retrieval metadata and SHA-256 hashes.
