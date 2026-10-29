# Engineering contracts

The initial 38 implementation stages are complete. Their instructions and original
index are in the [implementation archive](../archive/prompts/README.md). They are
historical requirements, not a queue of unfinished work.

The shared contracts below remain here because maintained source, tests and guides
refer to them. Keep them current when behavior changes. Start ordinary maintenance
with [AGENTS.md](../../AGENTS.md), the [documentation index](../README.md) and the
affected public guide; do not rerun the archived promptset by default.

| Contract | Responsibility |
| --- | --- |
| [Engineering baseline](ENGINEERING_BASELINE.md) | Exact public vocabulary, ownership, state, effects and time rules |
| [Engineering rules](ENGINEERING_RULES.md) | Portable core, transport ownership and distribution boundaries |
| [CLI contract](CLI_CONTRACT.md) | Command grammar, records, result ownership and bounded console work |
| [Chip evidence](CHIP_EVIDENCE.md) | TI and pinned public-code evidence, source conflicts and restrictions |
| [Maintainability](MAINTAINABILITY.md) | Code structure, naming and documentation rules |
| [Agent workflow](AGENT_WORKFLOW.md) | Task delegation, disjoint edits, review and lead-owned Git |
| [Common implementation instructions](COMMON.md) | Original stage workflow; current maintenance follows AGENTS.md |

The [final audit](../final-audit.md) and [validation record](../validation.md)
distinguish software checks from firmware builds and physical qualification.
Completed implementation does not mean every board or silicon capability is qualified.
