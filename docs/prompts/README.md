# TUSB32x implementation promptset

Prepared 2026-10-23 after auditing the local I2C library ecosystem. This set is
designed to produce a complete, general-purpose library, not a FieldCore-only
wrapper or an Arduino-only driver. TUSB320LAI is the primary acceptance device.

## Execute

Give an agent repository access and the following master prompt. Alternatively,
run one numbered prompt at a time; each imports [COMMON.md](COMMON.md) and must
re-audit its current sibling references. The preceding implementation and reports
stay in the same checkout. Do not restart or replace them between stages.

```text
Implement the TUSB32x library by executing docs/prompts/00-audit.md through
docs/prompts/14-final-audit.md in order. Read AGENTS.md, ../AGENTS.md, this index,
and docs/prompts/COMMON.md first. Treat COMMON.md as requirements for every stage.
Read its ENGINEERING_BASELINE.md, CLI_CONTRACT.md, MAINTAINABILITY.md and
AGENT_WORKFLOW.md companion contracts. They already select concrete defaults;
verify them against current sources rather than redesigning names from scratch.

Before each stage, inspect the current relevant sibling I2C libraries, including
actual public headers, implementation, examples/CLI, tests, Python runners, CI
and AGENTS.md. Record their revisions and the conventions adopted or rejected.
Re-audit changed references even if an earlier stage or historic report covered
them. Do not claim uniformity where peers differ. Do not change sibling projects.

Use the TI source archive and official datasheets to verify every supported
variant. Start with TUSB320LAI; implement the explicitly verified family matrix,
not a speculative wildcard family. Never infer exact silicon from ACK alone.

Finish each stage's implementation and meaningful checks, update its evidence
and progress, commit and sync, then continue. Also commit and sync coherent,
checked units during a prompt as they become ready. Routine design choices are
yours to resolve using the audited evidence. Ask only for missing information
that actually prevents dependent work; continue independent work. Hardware/tooling gaps must be reported
as NOT RUN or BLOCKED, never PASS. Do not silently drop required family features,
frameworks, tests, CLI parity, Python runners or packaging to finish sooner.

Use agents where useful for independent audit, implementation in disjoint files,
or final review. You may select their supported models and reasoning by task
complexity using AGENT_WORKFLOW.md. The lead owns integration and reviews their
evidence. Keep code direct, small and readable. Use simple engineering English.
Write Doxygen in source and update affected README/CHANGELOG/guides as each
behavior is implemented; documentation is part of every stage's completion gate.
Keep the entire promptset on its existing branch, across prompts, sessions and
subagents. The lead owns Git mutations, preserves unrelated changes and follows
AGENTS.md: fetch/integrate safely, commit completed units, push to the configured
upstream and verify synchronization. No further permission is needed for routine
commits/pushes. Do not create a branch/worktree per prompt or rewrite shared history.
Report actual sync failures. Release publication and hardware flashing require
their own task authorization and target information.

At completion provide the API and capability summary, changes, exact validation
results, remaining hardware evidence and links to docs/validation.md and the final
audit. A configured CI job, host mock, or SDK-header compile is not a linked
firmware image or hardware validation.
```

## Ordered stages

The detailed contracts are maintained once so stage prompts do not drift:

| Contract | Concrete decisions |
| --- | --- |
| [Engineering baseline](ENGINEERING_BASELINE.md) | API names/signatures, Err codes, diagnostics/health, state tables, lifetimes and file layout |
| [CLI and tooling contract](CLI_CONTRACT.md) | Commands/aliases, effects, output records, parser/runner and test conventions |
| [Maintainability and documentation](MAINTAINABILITY.md) | Simple code and language, per-file Doxygen, doc updates and negative checker fixtures |
| [Agent workflow](AGENT_WORKFLOW.md) | Model selection by complexity, bounded delegation, disjoint writes and independent review |

| Stage | Outcome |
| --- | --- |
| [00 — Audit](00-audit.md) | Current inventory, ownership/conventions matrix and reference choices |
| [01 — TI specification](01-ti-specification.md) | Source archive, variant/register/timing/feature matrix |
| [02 — API and contracts](02-api-contracts.md) | Portable public API and explicit state/ownership contracts |
| [03 — Repository skeleton](03-repository.md) | Buildable host core, metadata, versioning and project guidance |
| [04 — Protocol](04-protocol.md) | Exact transfers, register access, identity and safe bit handling |
| [05 — Owner operations](05-owner-operations.md) | Lifecycle, configuration, reset, deadlines and cancellation |
| [06 — Port, IRQ and health](06-port-health.md) | Typed observations, bounded interrupt service and passive health |
| [07 — Conformance tests](07-conformance-tests.md) | Protocol/state/fault/portability regression matrix |
| [08 — Shared CLI](08-shared-cli.md) | Full diagnostic interface and executable parser tests |
| [09 — Arduino](09-arduino.md) | S2/S3 bring-up and bounded owner examples |
| [10 — IDF and generic consumers](10-idf-consumers.md) | Native IDF, plain C++ and FieldCore-style owner examples |
| [11 — Python and HIL](11-python-hil.md) | Evidence-checked serial runners and physical test runbook |
| [12 — Packaging and CI](12-packaging-ci.md) | Reproducible package consumers and complete CI gates |
| [13 — Documentation](13-documentation.md) | Complete integration/API/CLI/test/release documentation |
| [14 — Final audit](14-final-audit.md) | Independent gap closure and honest qualification report |

Tests belong alongside each behavior change. Stage 07 adds cross-cutting
conformance and hostile cases; it is not permission to defer earlier tests.
Documentation similarly evolves with the code and is reconciled in stage 13.
Every stage must identify changed public symbols, updated source documentation,
affected general docs and meaningful checks before it is marked complete.

## Progress and handoff

At execution create `docs/implementation-progress.md` with a row for every stage:
NOT STARTED, IN PROGRESS, COMPLETE, or BLOCKED; branch/revision/dirty state;
references re-audited; changes; commands and outcomes; completed commits and
verified upstream sync (or its exact failure); remaining work. Record important API
choices in `docs/design-decisions.md`. Do not mark a required validation COMPLETE
when only its instructions or fixtures exist.

After each stage summarize the resulting behavior, evidence, commit/sync result
and unresolved decisions. Do not call a local commit alone a completed sync.
A new session keeps the same branch and reads those two files plus the common
contract before continuing. Preserve hardware limitations through the final report.

Supporting evidence: [audit index](../audit/README.md),
[TI family audit](../audit/ti-family.md), and [reference archive](../reference/README.md).
