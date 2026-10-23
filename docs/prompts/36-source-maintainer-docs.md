# 36 — Audit source documentation and maintainer guidance

Read [COMMON.md](COMMON.md). Prerequisites: prompts 1–35, including the executable
documentation guard from prompt 33. This prompt verifies source/API visibility
and maintenance instructions already written throughout implementation. It is
not permission to defer documentation until now.

## Audit and delegation

Re-audit LDC1614 Doxygen warnings/extraction, TMP1x2 API visibility checks and
two useful peer CONTRIBUTING/AGENTS/version workflows. Inspect actual settings
and tests rather than copying quiet but incomplete documentation builds.
Read MAINTAINABILITY, selected baseline contracts and current build commands.

A fast agent may check paths and metadata; a coding agent may inspect generated
API coverage. Use strong review for subtle silicon, lifetime and timing comments.
Assign read-only reviews or disjoint doc scopes. The lead integrates and owns
Git according to AGENT_WORKFLOW.

## Complete the documentation audit

1. Inspect every production public header, core source, adapter and shared CLI
   source for useful file/type/function/member documentation. Check enum values,
   units, valid ranges, default/unknown meaning, ownership and lifetime. Private
   helpers need concise explanations of non-obvious invariants, not boilerplate.
2. Check callback comments state terminal completion, borrowed buffers, aggregate
   timeout including lock and all phases, serialized access and no reentry.
   Match zero-I2C, staged/apply, raw diagnostic, cancellation/end and output-on-
   failure claims against actual behavior tests.
3. Confirm nearby W1C/reset masks and role/recovery timing cite exact TI evidence.
   Explain preservation of actual controls during ACK, fresh completion anchors
   and retained possible effects. Remove unsupported promises such as thread-safe,
   atomic hardware snapshot or automatic recovery.
4. Inspect generated API pages, not only Doxygen's exit code. Main class, Status/
   Err, DriverConfig/DeviceProfile, operations, snapshots and diagnostics must
   appear with parameter/return documentation. Public symbols hidden by extraction
   settings are failures. Keep private implementation private.
5. Run the existing omission fixtures: missing public comment, wrong parameter,
   missing enum-value documentation, broken reference and hidden public type.
   Check source/example documentation excluded from public extraction separately.
   Fix source or checker defects; do not suppress warnings or enable EXTRACT_ALL
   to conceal omissions. Generated Version.h comments come from its generator.
6. Reconcile AGENTS and CONTRIBUTING with actual layout, delegation/model policy,
   authorized branch/commit/sync behavior, supported tools, Windows wrapper,
   focused/full commands, test registration, documentation and version workflow.
   They must not require a second PlatformIO installation or nonexistent scripts.
7. Align CHANGELOG, license/ownership, library/component metadata, generated
   version, Doxyfile and release instructions. Use meaningful user-visible entries;
   no prompt diary, invented release/date/remote or unrun PASS badge. Keep
   `library.json` authoritative and use sync/check/bump/set.
8. Review Python module/exported helper docstrings for inputs, timeout/exit
   semantics, files and side effects. Keep test comments concise and make unusual
   fakes understandable. Avoid C++ tags in Python.

## Verify and simplify

Run the existing Doxygen/visibility/link/negative-fixture and version checks.
Confirm required test registration and Release assertion behavior remain intact.
Check important maintenance commands against their real entrypoints; do not
rerun unrelated firmware matrices solely for prose changes.

Review duplicated state tables, stale comments, unused public aliases and generic
claims. Keep one authoritative explanation and direct links. Rebuild relevant
snippets or behavior tests when a correction exposes a real implementation
problem; document the fix in the same unit.

## Finish this prompt

Source docs, generated API, metadata and maintainer instructions agree with the
current implementation and actual verification evidence. Update affected guides,
CHANGELOG, validation and progress; record any real missing-tool limitation.
**Commit and sync** this complete unit on the same execution branch and verify
upstream equality. Required omissions cannot be marked complete by hiding output.
