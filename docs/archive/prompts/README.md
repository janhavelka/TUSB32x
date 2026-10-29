> Archived on 2026-10-29 after completion of the 38-stage implementation.
> This is historical task guidance, not pending work. Use the
> [current documentation](../../README.md) and [engineering contracts](../../prompts/README.md)
> for maintenance. Recorded commands and stage-specific claims below describe the
> implementation process; current limits are in the final audit and validation.

# TUSB32x implementation promptset

This set contains **38 complete prompts**, numbered **1 through 38** in execution
order. It produces a general-purpose C++17 library for the verified TUSB320 family,
starting with TUSB320LAI. The repository currently contains audited planning
artifacts, passive C++17 values/Status helpers, a native library with pure
validators, private transport/identity, pure register codecs/composition and
deterministic metadata/version tools. The owner facade now implements lifecycle,
token/result ownership, read-only initialization, profile observation, explicit
apply, desired verification, read-only reconciliation, explicit reset and original
TUSB320/I dead-battery recovery, complete read-only port refresh with cached
publication, explicit IRQ acknowledgement/bounded service, safe raw diagnostics
and passive health. Prompt 18 closes core protocol coverage gaps. Framework
adapters and S2/S3 example applications are implemented in prompts 23–26;
physical qualification remains separate. Prompts 20–21 deliver
the bounded local CLI plus managed operation/control/result routes. Prompt 22
completes all diagnostic commands and executable alias/help/record parity.
Prompt 27 adds runnable simulated plain C++ ownership and generic integration
evidence without a FieldCore dependency.
Prompt 28 provides strict Python session framing, provenance, bounded evidence
and saved outcomes. Prompt 29 implements finite readonly/soak/register observation
plans and their live native integration. Prompt30 adds explicit orientation,
accessory, role-current and interrupt plans with deterministic/native evidence;
physical fixture qualification remains NOT RUN.
Prompt31 completes reset-termination, fault and console plans with explicit
action authorization, retained failure/effect evidence and fresh reconnect framing.

Prompt32 proves bounded safe archives and clean renamed package consumers,
including extracted Arduino/native-IDF image links. Prompt33 adds the explicit
local validation entry point, disjoint registrations and real checker failure
fixtures. These software checks do not imply physical fixture qualification.

Prompts 34–38 completed the CI checkpoint, maintained documentation and independent
core/integration reviews. The [final audit](../../final-audit.md) records fixes,
current local verification and remaining qualification limits. The development
version is unchanged; the final local IDF 6.1 image rebuild is BLOCKED by the
missing SDK and physical testing remains NOT RUN. Prior CI evidence stays tied
to its recorded tested commit, rather than the final reporting commit.

The [scope audit](../../audit/promptset-scope-review.md) explains the split from the
original fifteen stages and maps their requirements to the new units. The split
follows dependencies and independently testable outcomes, not equal text lengths.

The later [correctness and completeness review](../../audit/promptset-completeness-review.md)
records corrected chip sequences, owner/evidence rules, adapter limits and tooling
gaps. Its fixes are incorporated into the prompts and shared contracts below.

## Execute one prompt

Give the agent repository access and name the exact file from the table:

```text
Execute docs/archive/prompts/1-ecosystem-audit.md completely.
Read AGENTS.md, ../AGENTS.md, docs/prompts/COMMON.md and the prompt first.
Complete its scoped work, tests, source documentation and affected guides.
Use subagents when useful under AGENT_WORKFLOW.md. Update progress, commit and
sync before reporting completion. Do not start another prompt in this invocation.
```

For later invocations replace the filename with the next numbered prompt. Keep
the same checkout and recorded execution branch. Resume an incomplete prompt
before advancing. No prompt requires executing the old zero-based files.

## Execute the complete set

```text
Execute all 38 prompts listed in docs/archive/prompts/README.md, in numerical order.
Read AGENTS.md, ../AGENTS.md and docs/prompts/COMMON.md first.
For a new execution create one new branch from the synchronized intended base
before prompt 1, or use the branch the user created for this run. Record branch
and base in docs/implementation-progress.md. Keep that branch throughout every
prompt, agent and resumed session. When resuming, use the recorded branch and
finish the first incomplete prompt.

Each prompt is a complete work unit. Verify prerequisite outputs, re-audit the
current relevant sibling I2C libraries, and read the shared contract sections
needed for its scope. Preserve the concrete names, source facts and engineering
decisions unless evidence requires a documented correction. Keep siblings read-only.

Implement only working advertised behavior at each stage. Include meaningful
tests, Doxygen/docstrings and affected README/CHANGELOG/guides with each change.
No successful stubs, incomplete public APIs or testing/documentation debt passed
to later prompts. Planned later features remain explicit future scope.

Use supported subagents/models according to task complexity and AGENT_WORKFLOW.
Assign disjoint files and keep shared contracts and integration under the lead.
Use simple engineering English and direct maintainable code.

After each prompt, review and check its complete output, update progress, commit
and sync before continuing. Also commit and sync coherent checked units during a
prompt. The lead handles Git, preserves unrelated work and verifies equal local/
upstream commits and zero ahead/behind. No per-prompt branches or force-push.
Routine commits/pushes are authorized. Report exact sync failures.

Use focused local validation throughout this first promptset. Remote CI execution
and run-status checking belong only to prompt 34; do not repeat them after other
prompts, commits or substantive final-review fixes. This user-requested exception
overrides the recurring parent CI requirement. Preserve existing hooks and
required merge/release checks. Record the actual SHA tested at prompt 34 and keep
later local validation distinct; no new remote run is required for final handoff.

Resolve routine design choices from evidence. Continue useful independent work
when a tool or fact is missing, but keep required checks BLOCKED/NOT RUN and do
not mark incomplete dependent work COMPLETE. Hardware flashing and release
publication require their own authorization and target information.

At the end report implemented family/API, examples/CLI/runners, actual validation,
remaining hardware/variant limits, branch/commit/sync, and links to the final audit
and validation records. Host mocks, build files and SDK header shims are not
linked firmware or physical hardware qualification.
```

## Ordered prompts

Read these in numerical order even if an editor sorts filenames lexically.
A prompt lists its immediate evidence/dependencies; all preceding prompts are
expected complete. Later references identify ownership of future work, not a
requirement to implement it early.

| # | Prompt | Finished output |
| --- | --- | --- |
| 1 | [Audit the library ecosystem](1-ecosystem-audit.md) | Current conventions, ownership and consumer evidence |
| 2 | [Verify family capabilities](2-family-capabilities.md) | Source archive, identity, electrical and variant limits |
| 3 | [Specify registers and encodings](3-register-contracts.md) | Exact access rules, masks and independent vectors |
| 4 | [Specify silicon procedures](4-silicon-procedures.md) | Transport framing, timing and effect sequences |
| 5 | [Define value and API contracts](5-value-api-contracts.md) | Documented types, Status and planned signatures |
| 6 | [Define owner and state contracts](6-owner-state-contracts.md) | Admission, results, effects, time and snapshot rules |
| 7 | [Build the portable host foundation](7-host-build.md) | Linked passive API, registered tests and consumer |
| 8 | [Add metadata and version tooling](8-metadata-versioning.md) | Manifests, version generator, wrapper and basic Doxygen |
| 9 | [Implement transport and identity](9-transport-identity.md) | Counted callback path and private identity access |
| 10 | [Implement register codecs](10-register-codecs.md) | Pure decoding and mixed-register write composition |
| 11 | [Implement the owner engine](11-owner-engine.md) | Lifecycle, tokens, timing and read-only initialization |
| 12 | [Implement profile observation](12-profile-observation.md) | Read profile and read-only reconciliation |
| 13 | [Implement profile application](13-profile-application.md) | Explicit apply and desired-profile verification |
| 14 | [Implement reset and recovery](14-reset-recovery.md) | Reset and original TUSB320/I recovery procedures |
| 15 | [Implement port snapshots](15-port-snapshots.md) | Complete observations, validity, invalidation and age |
| 16 | [Implement interrupt service](16-interrupt-service.md) | Explicit ACK and bounded observe/ACK/recheck |
| 17 | [Implement diagnostics and health](17-diagnostics-health.md) | Bounded raw access, evidence and passive projection |
| 18 | [Close protocol regression gaps](18-protocol-conformance.md) | Fault, variant, access and effect conformance |
| 19 | [Verify timing and portability](19-owner-portability.md) | Quantized time, fairness, no heap and public linking |
| 20 | [Build the shared CLI foundation](20-cli-foundation.md) | Bounded parser/console and usable local commands |
| 21 | [Add CLI operation control](21-cli-operation-control.md) | Managed operation routing, polling and retained results |
| 22 | [Complete CLI command coverage](22-cli-command-coverage.md) | Remaining diagnostics, grammar/help parity and transcripts |
| 23 | [Implement Arduino transport](23-arduino-transport.md) | Terminal callbacks and real S2/S3 adapter fixtures |
| 24 | [Complete Arduino examples](24-arduino-examples.md) | CLI and owner-service images for S2/S3 |
| 25 | [Implement native IDF transport](25-idf-transport.md) | Native callbacks and linked component fixture |
| 26 | [Complete the native IDF example](26-idf-cli-example.md) | Shared CLI and resource lifecycle on S2/S3 |
| 27 | [Prove generic owner integration](27-generic-owner-consumers.md) | Runnable ordinary C++ and two-device owner consumers |
| 28 | [Implement the Python session](28-python-session.md) | Strict records, correlation, provenance and artifacts |
| 29 | [Add observation runners](29-observation-runners.md) | Read-only checks, bounded soak and register dumps |
| 30 | [Add port fixture suites](30-port-fixture-suites.md) | Orientation, accessory, role/current and IRQ checks |
| 31 | [Add recovery fixture suites](31-recovery-fixture-suites.md) | Reset, termination, recovery, fault and console checks |
| 32 | [Prove clean package consumption](32-package-consumers.md) | Hardened extraction and isolated framework consumers |
| 33 | [Complete validation guards](33-validation-guards.md) | Registered checks, Doxygen negatives and validator |
| 34 | [Complete the dedicated CI checkpoint](34-ci-matrix.md) | Pinned host/package/firmware jobs; the initial promptset's one remote checkpoint |
| 35 | [Reconcile user documentation](35-user-documentation.md) | Guides, buildable snippets and feature coverage |
| 36 | [Audit source and maintainer docs](36-source-maintainer-docs.md) | Visible documented APIs and accurate maintenance commands |
| 37 | [Review silicon and owner behavior](37-final-core-review.md) | Independent core findings fixed with regressions |
| 38 | [Finish integration review](38-final-integration-review.md) | Final validation, qualification limits and synced handoff |

Tests and documentation belong with the behavior they describe. Prompts 18–19
close cross-cutting test gaps; 35–36 reconcile maintained documentation; 37–38
independently review and fix. None permits earlier unfinished work.

## Shared contracts and reading scope

[COMMON](../../prompts/COMMON.md) is the short per-prompt entry and completion contract.
[ENGINEERING_RULES](../../prompts/ENGINEERING_RULES.md) retains the technical rules by topic.
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md) owns exact API/state/time names;
[CLI_CONTRACT](../../prompts/CLI_CONTRACT.md) owns grammar and records.
[CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md) carries the dated TI/public-code facts, exact
vectors, timing/electrical conditions and unresolved claims. Prompts 2–4 establish
the verified specification; relevant implementation and review prompts apply its
counterexamples. Re-open exact datasheets and pinned files, inspect helper
semantics, and correct inherited claims when evidence disagrees. Never treat this
promptset, a TI-hosted driver or a passing emulator as silicon authority.
[MAINTAINABILITY](../../prompts/MAINTAINABILITY.md) and [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md)
apply throughout. Read relevant sections and changed dependencies rather than
reprinting or redesigning every contract for each prompt. No rule was waived by
moving repeated technical prose out of COMMON.

A prompt is scoped to one coherent deliverable with its own acceptance checks.
It may take more than one session; word count cannot guarantee model capacity.
If context is tight, commit checked work, record exact remaining items and resume
that same prompt. Do not silently reduce scope or mark it finished.

## Progress and handoff

At prompt 1 create `docs/implementation-progress.md` with a row for every prompt:
NOT STARTED, IN PROGRESS, COMPLETE or BLOCKED; branch/base, current revision/dirty
state, peer evidence, completed scope, checks/outcomes, documentation, completed
commits, verified sync or exact failure, and remaining items. Keep material
contract choices in `docs/design-decisions.md`.

Each handoff reports scoped behavior, evidence, limitations and Git state.
Required unavailable checks stay visible; physical qualification remains separate.
Unrequested remote CI outside prompt 34 does not block prompt completion.
Record completed commit IDs in the next update without a self-referencing commit
loop. A new session reads progress and decisions before continuing on the same
execution branch. Promptset maintenance alone does not start implementation.

Supporting evidence: [audit index](../../audit/README.md),
[TI family audit](../../audit/ti-family.md), [reference archive](../../reference/README.md).
