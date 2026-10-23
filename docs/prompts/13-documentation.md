# P13 — Reconcile public documentation and integration guidance

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 docs index,
variant/integration/validation guides, LDC1614 API/owner docs, INA228 CLI/HIL docs,
and current FieldCore consumer guidance. Describe the implementation that exists.

## Work

1. Replace the planning-only top-level README with accurate library documentation,
   retaining a link to this promptset and dated audit. Lead with supported devices,
   purpose, portable ownership and a short compiling example. Do not claim every
   TUSB32x chip or USB PD capability. Mark TUSB322I extension status per feature.
2. Complete a navigable docs index with architecture, API/status, transport, owner
   operations/time/effects, variant/register behavior, integration, CLI, testing,
   HIL and hardware limits, version/release and validation documents.
3. Public API docs explain ownership/lifetime, blocking bounds, callback timeouts,
   thread/ISR constraints, retained result behavior, bus effects, variant support,
   output-on-failure, clock requirements, trust and recovery. Document raw writes
   and W1C/reset semantics as carefully as typed setters.
4. Publish exact supported-device table, source citations and per-feature coverage
   linking API symbols, CLI routes and tests. Separate original/second/third-gen
   controls, addresses, revisions and enable polarity; distinguish configured
   variant from experimentally verified hardware.
5. Give complete wiring/startup examples: supply/current conditions, ADDR/PORT,
   EN polarity and unresolved timing wording, I2C pullups/back-power, VBUS_DET,
   optional INT. State what the board/application owns. No guessed pin assignment
   or universal low-current/source capability claim.
6. Explain current/orientation/accessory validity, requested vs actual local role,
   ordinary detach, stale observations, coalescing IRQs, non-atomic hardware
   snapshots, explicit acknowledgement and attached-current tracking uncertainty.
7. Include Arduino and native IDF build/run commands, generic host/owner example,
   clean package use, FieldCore-style adaptation and testing entrypoints. All
   snippets must compile or be clearly labeled pseudocode; test important snippets.
8. Reconcile AGENTS, CHANGELOG, CONTRIBUTING, license, component/library metadata,
   generated version, Doxygen, CLI help and runner docs. No copied chip names,
   contradictory grammar, stale path, fictitious remote or unrun PASS badges.
9. Finalize `docs/validation.md`: each check's command, environment, revision/dirty
   state, result, evidence path and limit; hardware per exact part/board/framework.
   Separate implemented, host verified, firmware linked and hardware validated.

## Tests and completion

- Doxygen and local links pass; package documentation avoids excluded references.
- CLI help/contract/runner grammar agree; examples and snippets build where claimed.
- Review feature coverage against the full P01 matrix: implement missing applicable
  features or explicitly record an evidence blocker. Do not quietly redefine scope.
- Record remaining uncertainty plainly without burying it in obsolete audit notes.
