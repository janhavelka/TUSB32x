# Prompt length, complexity and execution review

Date: 2026-10-23. Baseline: `6eced5c`, the corrected fifteen-stage promptset.
This maintenance review reorganizes instructions; it does not execute them or
claim that a driver, firmware image or hardware result exists.

Later chip-evidence enrichment retains these 38 completion boundaries. The size
measurements below describe the original split; current measurements are in
[the validation record](promptset-validation.md). Detailed research lives in
[CHIP_EVIDENCE](../prompts/CHIP_EVIDENCE.md) and audit reports, with targeted
additions to applicable prompts rather than a new implementation stage.

## Findings and changes

The original prompts ranged from 768 to 2,062 whitespace-separated words.
Length understated their scope: owner operations combined lifecycle, timing and
several mutation sequences; CLI combined parser, console, command mapping,
asynchronous results and transcripts; packaging combined archive defense,
consumers, guards and CI. Each contained independent completion boundaries.

There are now **38 complete prompts numbered 1–38**, with unpadded filenames in
execution order. Each owns prerequisites, a focused current peer audit, optional
delegation, concrete work, meaningful checks, documentation and commit/sync.
They are roughly 560–860 words each. This is a measured result, not a word limit
that permits requirements to be removed.

COMMON shrank from 2,663 to 671 words. Its technical sections remain in
[ENGINEERING_RULES](../prompts/ENGINEERING_RULES.md); exact contracts remain in
ENGINEERING_BASELINE and CLI_CONTRACT. Read the sections relevant to each unit
and changed dependencies. All requirements still apply without making every
long reference an unconditional entry step for every narrow task.

Total numbered text grew because smaller units need complete acceptance and
handoff instructions. The goal is smaller reasoning and execution scope, not
fewer total repository words. No word count guarantees completion in every model
context. Progress records allow a prompt to resume without marking it complete.

## Coverage of every original prompt

Original labels below identify retired files at the baseline commit. Execute the
current [ordered index](../prompts/README.md), not these historical identifiers.

| Original prompt | Words | Main complexity | New owners |
| --- | ---: | --- | --- |
| 00 audit | 817 | Inventory, conventions and consumer evidence | 1 retains the full inventory with scoped delegation |
| 01 TI specification | 2,062 | Sources, generations, registers and silicon sequences | 2 sources/capabilities; 3 registers/vectors; 4 framing/procedures/timing |
| 02 API contracts | 1,179 | Values, signatures, admission, effects and time | 5 values/API inventory; 6 owner/state contracts |
| 03 repository | 768 | Host build, metadata, version and doc checks | 7 host foundation; 8 metadata/version/wrapper; 33 full omission guard |
| 04 protocol | 1,609 | Transport, identity, codecs, writes and raw access | 9 transport/identity; 10 codecs/composition; 17 raw API; sequences tested in 13–14 |
| 05 owner operations | 1,603 | Engine plus observation, apply, verify, reset and recovery | 11 engine/initialize; 12 read/reconcile; 13 apply/verify; 14 reset/recovery |
| 06 port/health | 1,271 | Publication, IRQ races, counters and projection | 15 snapshots; 16 IRQ; 17 diagnostics/health; counting starts in 9 |
| 07 conformance | 2,009 | Fault, timing, scheduling and portability matrix | Feature tests accompany 9–17; 18 protocol/effect gaps; 19 timing/fairness/portability |
| 08 CLI | 1,106 | Parser/console, routes, jobs and executable contract | 20 local foundation; 21 operations/results; 22 diagnostics/full parity |
| 09 Arduino | 986 | Adapter, two examples and two targets | 23 adapter/SDK fixtures; 24 CLI/owner examples |
| 10 IDF/consumers | 961 | Native adapter, frontend and generic integration | 25 native adapter; 26 native CLI; 27 ordinary C++/owner |
| 11 Python/HIL | 1,309 | Sessions/provenance, observations and mutation suites | 28 sessions; 29 observations; 30 port fixtures; 31 recovery/fault/console; validator in 33 |
| 12 packaging/CI | 1,214 | Archive defense, consumers, guards and matrix | 32 packages; 33 guards/validator; 34 CI |
| 13 documentation | 1,136 | Guides, coverage, Doxygen and maintenance | 35 user guides/coverage; 36 source/maintainer docs |
| 14 final audit | 1,126 | Independent subsystem review and final closure | 37 silicon/owner; 38 integration/distribution/final evidence |

## Length of each new prompt

Whitespace-separated counts after integration. The ordered index names each
deliverable; these counts exclude shared reference documents. The recurring
entry contract is 671 words, with technical reading selected by scope.

| Prompt | Words | Prompt | Words |
| ---: | ---: | ---: | ---: |
| 1 | 759 | 20 | 688 |
| 2 | 782 | 21 | 733 |
| 3 | 837 | 22 | 693 |
| 4 | 824 | 23 | 602 |
| 5 | 727 | 24 | 634 |
| 6 | 859 | 25 | 598 |
| 7 | 620 | 26 | 596 |
| 8 | 686 | 27 | 595 |
| 9 | 652 | 28 | 744 |
| 10 | 683 | 29 | 700 |
| 11 | 687 | 30 | 706 |
| 12 | 634 | 31 | 720 |
| 13 | 666 | 32 | 704 |
| 14 | 679 | 33 | 689 |
| 15 | 634 | 34 | 755 |
| 16 | 624 | 35 | 609 |
| 17 | 608 | 36 | 558 |
| 18 | 665 | 37 | 592 |
| 19 | 699 | 38 | 613 |

## Dependency corrections

- Planned signatures stay in design documents or explicit compile-only fixtures.
  Public callable declarations arrive with working definitions, tests and Doxygen;
  no successful or UNSUPPORTED placeholders hide unfinished implementation.
- Transport counting begins in prompt 9. Later diagnostics expose existing
  evidence instead of retrofitting accounting after mutation operations.
- Read/reconcile in 12 works without desired settings. Apply in 13 creates the
  desired image and enables useful verification. No temporary desired setter or
  initialization side effect is needed to test future work.
- Prompt 5 defines the small pure register-decoding contract, 10 implements it,
  and 22 formats its result. CLI decoding requires no private source headers,
  duplicate silicon codecs or fake transport.
- CLI and Python foundations expose finished subsets. One real CLI fixture and
  transcript registration are extended rather than recreated at each stage.
- Arduino/IDF startup operations have an explicit application owner. It takes
  each exact result before further startup/CLI work, avoiding an unowned retained
  result that blocks the console.
- Generator fixtures use one named file from 8. Package checks exist in 32 before
  the validator wraps them in 33. Full Doxygen omission checks arrive in 33;
  accurate source documentation starts with the first code.
- User docs fix small local defects. Larger missing implementation reopens its
  responsible prerequisite; the docs prompt remains blocked until that unit is
  complete instead of absorbing unbounded work or concealing the omission.

## Completion and review limits

Every prompt retains current relevant sibling audits, optional complexity-based
subagents with disjoint ownership, plain language, Doxygen/docstrings, affected
general docs, and checked commit/sync on the same execution branch. Tests stay
with behavior; conformance and documentation stages close remaining gaps.

Three agents drafted independent scopes. Follow-up read-only reviews checked
dependencies, original coverage and completion risks. The lead integrated their
work and updated shared references. Mechanical validation is recorded in
[promptset-validation](promptset-validation.md). These checks validate instructions
and links, not future runtime behavior, physical hardware or model completion time.
