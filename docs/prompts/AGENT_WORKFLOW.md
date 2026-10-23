# Delegation and model selection

The user explicitly permits subagents and choosing their models for this work.
Use them when an independent task benefits from separate attention. The lead
owns API decisions, integration, final checks and the user-facing result.

## Select by the work, not by the prompt number

Inspect the active runtime's available models and reasoning levels before
spawning. The examples below use model names exposed in this editing session;
they are a project workflow choice, not a permanent model-availability or price
claim. In another runtime use its supported equivalents. If selection is not
supported, inherit the lead model and state that limit without blocking work.
The session-specific model examples were checked on 2026-10-23; future execution
must use its current runtime catalog rather than assume these aliases still exist.

| Work | Preferred capability | Current-session example | Reasoning starting point |
| --- | --- | --- | --- |
| File inventory, link checks, agreed table formatting, narrow doc consistency | Fast model for tightly specified work | `gpt-6-luna` | low or medium |
| Normal implementation, focused tests, CLI/parser tooling, build integration | General coding model | `gpt-6.1-sol` | medium; high for interacting contracts |
| Variant ambiguities, W1C/reset races, uncertain write effects, clock/deadline and cancellation design | Strong reasoning model | `gpt-6-astra` | high |
| Independent final protocol/ownership review | Strong reviewer with fresh task context | `gpt-6-astra` or capable coding model | high |

Use higher reasoning when the problem needs it, not automatically for mechanical
work. Escalate a task when evidence conflicts, an invariant remains unclear or
repeated focused fixes fail. Do not ask a faster model to certify a difficult
hardware race merely because it wrote the table. Do not lower review quality to
fit a model choice; narrow the task or select a more capable available model.

Avoid claims about exact cost or comparative speed without current evidence.
Do not install tools, change account settings or require an API key merely to
select a subagent already supported by the runtime. Never put credentials in a
delegation message.

## Give each agent a bounded contract

Before spawning, specify:

```text
Objective: the exact result to produce or question to resolve.
Read first: AGENTS.md, COMMON.md, applicable baseline sections and named evidence.
Owned files: the exact files it may edit, or "read-only review".
Constraints: no sibling edits; preserve API/state/transport contracts and user work.
Documentation: update source Doxygen and affected guides/changelog with the code.
Validation: concrete checks, including negative/fault cases where relevant.
Return: changed files or evidence paths, decisions, test results, unresolved facts.
Model choice: selected capability/reasoning and one short reason if non-default.
```

Provide enough context for a fresh agent: the user goal, exact stage, current
decision records and file ownership. A model override may require a fresh-context
spawn in the active tool; follow its schema. Do not assume a new agent has the
conversation or can see an unsent design decision.

Assign disjoint write scopes. A source author and independent reviewer may inspect
the same files, but only one edits each file at a time. Freeze shared signatures
before parallel implementation; send changes promptly to dependent agents.
The lead merges compatible edits and resolves contradictory recommendations by
source and tests. Agent consensus is not evidence of silicon behavior.

The lead alone stages, commits, integrates upstream and pushes shared changes.
The lead creates one new branch when starting a new promptset execution, or
resumes its recorded branch. Keep that branch across all prompts and sessions;
subagents use the same checkout with disjoint write scopes, not separate
per-agent branches/worktrees. Follow AGENTS.md: commit and sync each completed
prompt and any coherent, checked unit ready during a prompt. Include code,
tests and affected documentation together. Preserve unrelated user/agent work,
verify upstream synchronization and report concrete failures. Routine commits
and pushes are already authorized; do not ask again at each handoff.

Use the runtime's concurrency limit. Do not recursively spawn work without a
clear independent purpose. Keep dependent steps local and sequential. Waiting
for a reviewer does not prevent the lead from doing unrelated work.

## Useful delegation within the smaller prompts

The numbered prompt owns a complete unit; delegation helps finish that unit,
not expand it into later prompts. One implementation owner and a focused reviewer
are often enough. Do not spawn a team merely because a slot is available.

| Prompts | Independent work worth delegating | Work retained by lead |
| --- | --- | --- |
| 1 | Library subsets, consumer and metadata inventory | Convention choices and contradictions |
| 2–4 | Exact TI generation review, independent masks/timing | Family boundary, sequences and unresolved facts |
| 5–6 | Public type/admission/time contract review | Canonical API and executable scope boundaries |
| 7–8 | Host consumer or version-generator fixtures | Build registration and metadata truth |
| 9–17 | Independent bytes, faults or effect/time review | One transport/engine and shared state semantics |
| 18–19 | Conformance gaps, timing/fairness/portability checks | Coverage quality and registration |
| 20–22 | Parser/output fixtures and help/alias checks | Command effects, single poll and result ownership |
| 23–27 | Backend fixtures, target configs or generic consumer | Aggregate timeout, lifecycle and frontend parity |
| 28–31 | Fake-serial failures, artifact and fixture review | Session identity, verdicts and cleanup rules |
| 32–34 | Clean consumers, checker negatives or CI review | Supported matrix; remote CI only at prompt 34 |
| 35–36 | Links/snippets, API visibility and metadata consistency | Technical accuracy and maintainable docs |
| 37 | Independent silicon and owner reviewers | Core finding triage and regression fixes |
| 38 | Independent integration and distribution reviewers | Final matrix, qualification limits and handoff |

Mechanical documentation work still needs engineering review when it describes
behavior. Do not assign the writer of a subtle protocol change as its only reviewer.
The lead reads returned diffs and runs appropriate integration checks; a subagent
report saying PASS is not a substitute for checking what actually ran.

Record substantive delegation in the implementation progress entry: task, owner,
model or inherited mode, changed files, evidence and unresolved findings. Keep it
short; no transcript dumping or extra report file for every tiny task.

## Guidance source and limits

Official guidance recommends focused independent tasks and coordination when
agents share files. The runtime's own tool schema determines how this session can
spawn and select models; these prompts do not require implementing an Agents API
client. [OpenAI multi-agent guidance](https://developers.openai.com/api/docs/guides/agents-api/multi-agent)

Reasoning support varies by model. Choose effort supported by the active model
and task; do not assume every setting is available everywhere.
[OpenAI reasoning guidance](https://developers.openai.com/api/docs/guides/reasoning)
