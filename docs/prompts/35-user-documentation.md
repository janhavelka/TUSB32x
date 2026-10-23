# 35 — Reconcile user guides and feature coverage

Read [COMMON.md](COMMON.md). Prerequisites: prompts 1–34, including current
implementation, examples, runners, package evidence and prompt 34's recorded CI
results. Do not request a fresh remote run for documentation. Documentation
already accompanies those changes. This prompt closes user-guide gaps; prompt 36
audits source documentation and maintainer instructions.

## Audit and delegation

Re-audit TMP1x2 guide/index structure, LDC1614 API/owner docs, INA228 CLI/HIL docs
and current FieldCore integration guidance. Read MAINTAINABILITY and the actual
implemented headers, help, runner commands and validation records.

A documentation agent may check links and example commands; a coding agent may
build snippets. Keep file ownership separate. Use strong review for subtle
time/effect or silicon wording. The lead verifies technical claims and owns Git
under AGENT_WORKFLOW. Do not generate boilerplate from symbol names.

## Reconcile the maintained guides

1. Make the root README describe the existing library: purpose, supported/partial/
   excluded variants, ordinary C++ ownership, dependencies and a short compiling
   example. Keep links to the promptset and dated audit. Do not imply every
   TUSB32x chip or USB PD support; state TUSB322I limits per feature.
2. Complete a navigable docs index covering architecture, API/status, transport,
   owner operations/time/effects, register/variant behavior, integration, CLI,
   tests/HIL, hardware limits, versions/releases and validation. Closely related
   topics may share a file; keep one authoritative contract and stable links.
3. Reconcile `docs/api-contracts.md`, `docs/owner-operations.md` and architecture
   guidance with exact method names, admission/rejection, output-on-failure,
   callback budgets, lifetimes, clock domain and serialized-owner/ISR limits.
   Explain Status versus operation outcome, HardwareEffect, trust and health.
   Successful takeResult means delivery, even when the copied operation failed.
4. Complete `docs/register-map.md`, `docs/variant-matrix.md` and
   `docs/feature-coverage.md`: every supported feature maps to API, CLI, test
   and exact TI revision/page/table. Separate generations, addresses, revision
   evidence and enable polarity. Configured variant is not physical qualification.
   Fix small local defects with tests. For a larger missing implementation, reopen
   its responsible prompt and keep this prerequisite BLOCKED until that unit is
   complete; do not expand a docs prompt into an unbounded implementation task or
   redefine support to hide a gap. Preserve genuine evidence blockers explicitly.
5. Explain requested versus actual local role, current/orientation/accessory
   applicability, ordinary detach, historical field validity, invalidated and age.
   Hardware multi-register snapshots are not atomic; IRQ latches coalesce and
   require explicit ACK. Keep attached-current tracking uncertainty visible.
   Base dead-battery recovery has explicit caller prerequisites and is not
   established by read-only initialization or a clear 0x45 bit.
6. Reconcile example READMEs and integration guidance: plain C++, Arduino, native
   IDF, clean package use and FieldCore-style mapping. Document supply/current,
   ADDR/PORT, EN polarity/startup wording conflict, pullups/back-power, VBUS_DET
   and optional INT. No guessed pins or universal source-current guarantee.
7. Align `docs/CLI.md`, testing, HIL and hardware-validation guides with current
   grammar, aliases, records, verdicts, artifacts, DTR/RTS, fixture criteria and
   cleanup rules. Link one command reference instead of copying divergent lists.
8. Reconcile `docs/validation.md`: command, environment, revision/dirty state,
   result, artifact and limit. Distinguish implemented, host verified, firmware
   linked and hardware validated for each exact part/board/framework.

## Verify and document

Re-open the actual pinned implementations and exact TI pages listed in
[CHIP_EVIDENCE](CHIP_EVIDENCE.md); inherited prompts are review inputs, not proof.
Correct stale constants, timing units and support claims with their tests and
guides. Explain base explicit-reset current refresh versus a fresh register read,
322 reset-to-DRP, distinct supply ranges and VBUS_DET requirements, and local
advertisement versus remote-current detection. Preserve min/typ/max labels for
CC/VCONN signaling and limits; do not imply measured VBUS delivery. Keep source
inspection, host simulation and exact-board physical results visibly separate.
Do not document unresolved public-code revision `0x06` as accepted silicon.

Build important snippets against real headers; label intentional pseudocode.
Include bind → accepted start → bounded poll → takeResult → inspect outcome/effect.
Do not use `if (!startStatus)` as rejection of IN_PROGRESS or return pointers to
temporary Status text. Check quick-start paths, package links and CLI/runner
grammar against implementation. Run local link checks and relevant snippet tests.

Use plain engineering English. Keep defaults, observations and unverified claims
distinct. Update README, CHANGELOG, guide index and progress with substantive
changes; a documentation correction does not create new hardware evidence.

## Finish this prompt

Required guide content, feature coverage and executable examples agree with the
implementation. Fix locally resolvable gaps and record real qualification limits.
Run focused checks, **commit and sync** on the existing execution branch and
verify upstream equality. Do not leave known guide contradictions for prompt 36.
