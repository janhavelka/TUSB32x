# Maintainability, language and documentation rules

These rules apply during every stage, including tests, examples, Python tools and
review fixes. They are part of [COMMON.md](COMMON.md), not an optional final polish.

## Write for the next engineer

Use plain engineering English in prompts, code comments, documentation, CLI text,
commit descriptions and handoffs. Name the thing, state what it does, give its
units or limits, and explain the consequence. Keep necessary terms such as I2C,
W1C and deadline; explain them at first use. Avoid promotional claims, invented
terminology, vague quality labels and comments that merely repeat the code.

Prefer these descriptions:

| Avoid | Use |
| --- | --- |
| Robust enterprise-grade recovery orchestration | The application decides when to retry after an I2C error. |
| Seamlessly reconcile device semantics | Read the registers and update the observed profile. Do not write settings. |
| Atomic snapshot | The completed software snapshot is published in one update. Hardware registers may change between reads. |
| Safe reset | Reset may change attachment and leaves configuration untrusted until checked. |
| Health repaired | The last tracked transfer succeeded. Configuration trust is unchanged. |

Put operational facts in the relevant header and guide. Do not make a reader
search an audit report to learn that a function performs I2C or may leave
termination disabled. Audit reports explain decisions; maintained API docs
describe the current contract.

## Keep the implementation small and direct

1. Follow the selected layout in [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md).
   A file has a clear job. Separate protocol access, owner operations, decoded
   observations and CLI presentation when that makes changes easier to review.
   Do not create a file/class for every register, enum or helper.
2. Use simple value types, fixed-size storage, named constants, explicit branches
   and small private helpers. Prefer an explicit state enum and switch for this
   small device over a generic state-machine framework, template registry, plugin
   system, command DSL or inheritance tree.
3. Keep one implementation of register access, timeout clipping, write-effect
   handling and state transitions. Typed APIs, CLI and any compatibility helper
   must not maintain separate versions of those rules.
4. Keep raw bytes and decoded facts distinct. Avoid a writable mirror of every
   register plus several independent copies of the same truth. Store only what
   is needed for desired state, verified state, observations and pending work.
5. Make state ownership obvious. Prefer private members to file-scope mutable
   data. Use local temporaries until a read is complete; commit outputs once.
   Public getters return value snapshots without hidden reads or repairs.
6. Use ordinary include guards or pragma once consistently. Include what each
   file uses. Public headers compile independently; never rely on framework or
   transitive includes. Avoid `using namespace` in public headers.
7. Keep error handling explicit. Check each fallible return once, preserve the
   original error/detail, and record possible hardware effects. Do not convert
   failure to success for convenience or use exceptions to hide control flow.
8. Avoid speculative extensibility. The six verified 320 variants need a small
   capability table, not a universal USB controller framework. Add 322-specific
   fields only for verified supported behavior. Do not expose private job phases
   merely because tests want to inspect them.
9. Preserve one canonical API name per action. CLI aliases are useful; duplicate
   C++ aliases such as initialize/init/begin/startInit are unnecessary in a new
   library unless a real consumer needs them. Document any compatibility layer.
10. Keep formatting mechanical: start from INA228's two-space C++ indentation,
    attached braces and 100-column preference; review its include groups because
    Arduino-first ordering does not belong in core. Python uses four spaces and
    PEP 8 names. UTF-8, LF, final newline, no trailing whitespace. Do not rewrite
    unrelated files just to change style.

Use readability judgment rather than arbitrary line-count or abstraction quotas.
A longer explicit register table can be clearer than a compact generic mechanism.
When adding a helper, explain the repeated rule or boundary it makes clearer.

## Doxygen is written with the code

Every new or changed production C++ header/source and example/helper source gets
an accurate `@file` / `@brief` describing its responsibility. Public headers
document namespace/type purpose, each enum value, public fields and public
functions. Use `///<` for short field/value comments. Include units, valid range,
unknown/default meaning, ownership and lifetime where relevant.

For each fallible operation document:

- What it does, whether it is cached, admits work, or performs a callback.
- Preconditions: binding, identity, configuration trust, owner serialization,
  board readiness and supported variant where applicable.
- `@param[in]` / `@param[out]` with units, time domain, buffer lifetime and whether
  outputs stay unchanged on rejection/failure.
- `@return` or `@retval` for success, in-progress and meaningful rejection/failure
  codes. Do not list unrelated codes simply to make the block longer.
- Maximum callback work and timeout meaning; whether it may wait and how the
  owner advances it. A cached getter states that it performs no I2C.
- Hardware effects and evidence that survive cancel/timeout/end. Use `@note` for
  normal constraints; reserve `@warning` for a real easily missed side effect.

Example documentation shape for the selected result API:

```cpp
/// @brief Consume the retained result for one completed operation.
/// @param[in] token Token returned by the accepted start call.
/// @param[out] out Receives the result on OK; unchanged on failure.
/// @retval OK The result was copied and its slot was released.
/// @retval RESULT_NOT_AVAILABLE No terminal result is ready.
/// @retval TOKEN_MISMATCH The token does not identify the retained result.
/// @note Performs no I2C. Call from the same serialized owner as poll().
Status takeResult(OperationToken token, OperationResult& out);
```

The example establishes documentation detail; its return codes must stay aligned
with the final declaration and tests. Do not paste a block whose semantics differ.

Private helpers need short comments for non-obvious invariants, not a full public
contract on every trivial accessor. Explain *why* a write masks out IRQ/reset
bits, why a delay starts after completion, and why a failed write remains uncertain.
Place a TI document/revision/section/table reference next to register constants
and unusual sequences. Link to the detailed register guide rather than copying
large vendor passages into source.

Test files use a short file comment describing the contract under test and named
cases showing setup/action/expectation. Explain unusual fake behavior. Do not add
public test-only hooks or `#define private public` just to inspect implementation.
Python modules and exported helpers get docstrings covering inputs, return/exit
semantics, timeouts, output files and side effects; Python does not need C++ tags.

## General documentation changes with every behavior change

At each stage inspect the following and update the affected files in the same
change. Do not add meaningless edits to unaffected files; record which were
reviewed and why no change was needed. Stage 13 checks consistency, not first-time
documentation.

| Artifact | Update when |
| --- | --- |
| `README.md` | Public capability, quick start, supported variant/framework or important limitation changes |
| `CHANGELOG.md` | User-visible API, behavior, diagnostic, tool, build or documentation change; use an Unreleased section before a release |
| `AGENTS.md` / `CONTRIBUTING.md` | Working rules, validation commands, source layout or supported tools change |
| `docs/api-contracts.md` / `docs/architecture.md` | Ownership, methods, states, lifetimes or timing changes |
| `docs/register-map.md` / `docs/variant-matrix.md` | Register behavior, variant capability or interpretation changes |
| `docs/CLI.md` | Grammar, output fields, aliases, side effects or command prerequisites change |
| `examples/README.md` and example READMEs | Build, board/wiring, backend or startup behavior changes |
| `docs/testing.md` / `docs/hil-runner.md` | Commands, fixtures, expected records, verdicts or test setup changes |
| `docs/feature-coverage.md` | Feature/API/CLI/test mapping changes |
| `docs/validation.md` | Actual checks run and resulting evidence change |
| `docs/design-decisions.md` | A material tradeoff changes; state the chosen rule, reason and consequence briefly |

Do not bump a version for every prompt. Keep `library.json` authoritative and
use the agreed version workflow for a meaningful release decision. Do not invent
contributors, release dates, badges or tested configurations.

## Documentation checks must detect omissions

Inspect LDC1614's Doxyfile: `EXTRACT_ALL=NO`, undocumented/parameter/error warnings
enabled, warning failure enabled. TMP1x2's checker adds useful generated HTML/XML
visibility and link checks, but its `EXTRACT_ALL=YES` alone does not prove comments
exist. Adopt the useful parts of both and test the combined checker.

Create `tools/check_doxygen.py` with a bounded Doxygen invocation, clear missing-
tool failure and generated output beneath the build directory. Public API pages
must be visible; private implementation need not become published API. Check
source/example file documentation separately where those files are excluded from
the public Doxygen input. Generated Version.h documentation comes from its generator.

Use small negative fixtures: missing public comment, wrong parameter name, missing
enum value documentation, broken internal link and hidden public type. A Doxygen
run that emits no warnings only because everything is hidden is a failed check.
Do not set EXTRACT_ALL or suppress warnings merely to make missing docs pass.

Review semantic accuracy too: Doxygen cannot prove that `cancel()` is bus-silent
or that an output remains unchanged. Behavioral tests verify those claims.

## Per-stage finish gate

Before reporting a stage complete, show: changed behavior and public symbols;
focused checks and outcomes; source/Doxygen and general docs updated; any
intentional baseline deviation; and remaining evidence gaps. Confirm no duplicate
state/logic, unnecessary framework, stale comment, ignored error, unused public
method or accidental sibling dependency was introduced. Fix locally resolvable
issues before moving on.
