# Instructions shared by every numbered prompt

Each numbered prompt is one complete work unit. Execute them in numerical order
from 1 through 38 using [the index](README.md). Run only the requested prompt
unless the user authorized a sequence or the whole set. Do not implement later
units merely because their design is described in a shared contract.

## Start with the current state

Read repository and parent AGENTS.md, this file, the requested prompt and
`docs/implementation-progress.md` / `docs/design-decisions.md` when they exist.
Check that prerequisite outputs exist and their required checks passed. A missing
or broken prerequisite is not permission to add a placeholder: repair a small
local defect with its regression check, or report the exact blocked dependency.

Before every prompt, re-audit current relevant sibling I2C libraries under
`Projects/`: revisions, dirty state, AGENTS, declarations, implementation, tests,
examples/CLI, Python runners and workflows as relevant. Compare two useful peers
for material design choices. Record paths and adopted/rejected conventions in a
short progress entry. Prompt 1 performs the broad inventory; later prompts verify
relevant changes and reuse unchanged evidence. Keep siblings read-only.

## Read the relevant contracts

All shared requirements remain binding. Read the sections needed for this unit
and the boundaries it touches; use current progress/decisions to resume rather
than copying every long contract into each task or repeatedly redesigning it.

| Reference | Read for |
| --- | --- |
| [ENGINEERING_RULES](ENGINEERING_RULES.md) | Ownership, portability, silicon, health, examples and distribution rules by topic |
| [CHIP_EVIDENCE](CHIP_EVIDENCE.md) | Pinned public sources, verified chip facts, conflicts and concrete regression seeds |
| [ENGINEERING_BASELINE](ENGINEERING_BASELINE.md) | Exact API/type/error names, register-facing values, state/effect/time contracts and layout |
| [CLI_CONTRACT](CLI_CONTRACT.md) | Command grammar, bounded console work, records, correlation and result ownership |
| [MAINTAINABILITY](MAINTAINABILITY.md) | Direct code, simple engineering language, source Doxygen and maintained guides |
| [AGENT_WORKFLOW](AGENT_WORKFLOW.md) | Delegation, available model selection, disjoint edits and review |

Prepared choices are defaults backed by the audit. Change them only for a concrete
source, test or consumer reason, then update dependent contracts and evidence.
Verified TI facts and user instructions take precedence. Do not copy unrelated
sensor features, introduce framework dependencies or guess unresolved silicon facts.

For chip-dependent work, audit the relevant CHIP_EVIDENCE claims against the
actual pinned source and exact TI datasheet/errata. Check masks, shifts, units,
min/typ/max, access effects and variant applicability. Read helper implementations;
a TI-hosted repository, popular library or passing emulator can still be wrong.
Do not invent a value, silently repair a datasheet typo, or use copied code as its
own test oracle. Record conflicts, gate the affected feature, and correct all
dependent code/tests/docs when hard evidence resolves them. Hardware claims need
identified parts, wiring, firmware and measured traces; host tests prove logic.

## Keep each unit complete

Implement the prompt's stated scope, including its meaningful tests, error paths,
Doxygen/docstrings, affected README/CHANGELOG/guides and validation evidence.
Use small direct helpers and one existing transport/operation path. No TODO
behavior, successful stubs, disabled assertions or missing tests deferred to a
later conformance prompt. Planned signatures may be documented; public callable
declarations are added with working definitions and link tests. Early CLI/runner
stages expose only their implemented subset and label remaining work as future.

Subagents are allowed, not required. Select an available capability appropriate
to the task, give explicit file ownership and checks, and keep shared contracts
under the lead's control. A reviewer may read overlapping files; concurrent
authors may not edit them. The lead inspects diffs and integration results.

If context becomes short, checkpoint completed checked work and record the exact
remaining items. Resume the same prompt before moving on; do not call it complete
because a session ended. Required unavailable checks remain BLOCKED/NOT RUN.
Hardware qualification is separately reported and cannot be inferred from mocks.

## Finish and sync every prompt

Follow [AGENTS.md](../../AGENTS.md#git-workflow-one-branch-for-the-whole-promptset).
Create one new branch at the start of a new promptset execution, or use the branch
the user created for it. Record branch/base and keep it through all 38 prompts,
agents and resumed sessions. Editing this promptset is maintenance, not execution.

Before completion: review the scoped diff, run appropriate checks, update the
progress entry, documentation and coverage, then **commit and sync**. Also commit
and sync coherent checked units during a prompt. The lead alone handles Git;
preserve unrelated work. Fetch, safely integrate if needed, push the configured
upstream and verify equal commits and zero ahead/behind. Do not force-push or
make empty commits. No further routine commit/push permission is needed.

Report actual sync failures and preserve completed local work; continue useful
independent work without claiming synchronization. Release publication, sibling
edits and hardware flashing need their own authorization. The handoff states the
completed scope, checks, limitations, commit, branch and verified sync result.
