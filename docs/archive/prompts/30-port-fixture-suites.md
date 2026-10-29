# 30 — Implement port, current and interrupt fixture suites

Deliver complete orientation, accessory, role-current and interrupt plans using
ordinary CLI operations. Keep physical fixture prerequisites explicit, and prove
the planning/evidence paths with deterministic tests when hardware is unavailable.

## Read and re-audit

Read [COMMON](../../prompts/COMMON.md), [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md),
[CLI_CONTRACT](../../prompts/CLI_CONTRACT.md), and the port/IRQ/profile/capability sections of
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md) and
[ENGINEERING_RULES](../../prompts/ENGINEERING_RULES.md). Prompts 28–29 must have a working runner,
strict sessions, artifacts and observation plans. Use the verified TI matrix and
current implementation rather than assuming all TUSB32x variants share behavior.

Re-audit relevant TMP1x2/24Cxx fixture prerequisites and cleanup evidence, LDC1614
suite verdicts and INA228 staged/applied and owner-exclusion assertions. Record
revisions, dirty state, actual files and choices. Read the TUSB IRQ and role-change
failure tests. Sensor scratch-window or unconditional restore policies do not fit.

## Add explicit fixture plans

1. Add the four named suites to repeated --suite selection and help. Require
   --fixture <id> and the suite's declared setup prerequisites for mutation and
   fixture-dependent actions. Record board/part/adapter/fixture identity, exact
   wiring and operator assumptions. No generic --all silently includes writes.
   No board-power operation is authorized merely by selecting an I2C library.
2. Capture firmware identity, selected variant/address, profile/trust/generation,
   current valid attachment and supply assumptions before writes. Unknown identity
   cannot satisfy identity-required mutation. Revalidate after reconnect/reset and
   before cleanup. Use the existing session/parser/verdict layer, not a second
   framework, marker protocol or result store.
3. Orientation requires a reversible connector fixture and evidence for both
   orientations, local/peer role meaning and relevant validity bits. An absent
   cable/fixture is NOT RUN, not a plausible default orientation or PASS.
4. Accessory requires exact supported fixture types and variant capabilities.
   Test capability denial on unsupported variants before traffic. Preserve raw
   unknown encodings; do not reinterpret them as a supported accessory.
5. Role-current checks fixture/power capability, local sink/source/DRP, supported
   CC advertisement categories, stage/apply/verify and actual attachment evidence.
   CC indication is neither measured load nor a PD contract. Do not infer safe
   external VBUS/current policy or enable hardware the application did not select.
   Keep unresolved TUSB322I advertisement writes gated.
6. Interrupt requires wired INT, observed assertion, explicit ACK/service and
   reassertion/coalescing evidence. Test staged!=applied 0x09 preservation and an
   unrelated settings write leaving the latch pending. Service is bounded and
   not lossless edge capture. RCU alone does not prove read-to-clear; do not run
   unresolved TUSB322I fault-clear actions as if their semantics were established.
7. Keep each plan finite: declared time/count limits, explicit operator/fixture
   transitions and a terminal verdict for timeout or missing input. Complete and
   take each core operation's result before starting the next. Stop on unexplained
   failure. The runner must never automatically retry uncertain role/apply/ACK
   effects or introduce hidden firmware bulk commands.
8. Restore only when target, attachment and supply assumptions still permit it.
   Never restore an old source role after the partner/target changed. A safe
   cleanup attempt is itself explicit work with evidence; readback cannot undo a
   real USB event. Keep primary outcome, cleanup outcome and residual uncertainty
   separate. Missing cleanup proof prevents overall PASS, even after good tests.

## Test the complete plans

Apply [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md)'s physical-evidence rules: record actual
supply/straps and distinguish LAI local 3 A advertisement's >=3.5 V condition
from remote-current detection. Use 4.5-5.5 V supply for 322 fixtures. Its VCONN
limit is 225/300/375 mA min/typ/max, not a 300 mA guarantee; retain required VBUS_DET.
Vary peer advertisement while attached and compare reads with physical evidence.
Base SLLSEN9F section 7.3.1.2, p11 requires periodic explicit software reset to
update detected current; a read alone cannot prove that update. Never hide resets
in readonly/soak plans or generalize this policy to later variants.

Extend runner tests using fake serial/clock and scripted fixture observations.
For each suite cover successful evidence, absent fixture, unsupported capability,
invalidated port data, missing expected identity, setup rejection before writes,
unexpected attachment change, deadline/cancel/failure at mutation boundaries and
cleanup eligibility. Verify report/checkpoint survival and truthful exit codes.

Exercise applicable command sequences against the real persistent C++ CLI fixture,
including stage 30% with applied 50% and IRQ ACK retaining 50%. Distinguish modeled
assertion/attachment from physical INT/connector proof. Reuse existing registered
hil_runner/cli_transcript tests and run focused affected checks. Missing hardware
leaves physical checks NOT RUN; it does not excuse unfinished plan/parser tests.

## Delegate, document and sync

Optional: delegate orientation/accessory fixtures and tests to a general coding
agent with disjoint ownership. Use a separate stronger-reasoning reviewer for
role/current, IRQ preservation and cleanup eligibility. Freeze shared suite data
and verdict rules first; the lead reviews evidence and owns Git.

Update Python docstrings, `docs/hil-runner.md`, `docs/hardware-validation.md`, CLI
links, README, CHANGELOG, coverage, validation and progress. Include real setup
criteria, permitted transitions, cleanup and NOT RUN/BLOCKED meanings. Do not
flash or manipulate physical fixtures without the applicable task authorization
and target details. Commit/sync coherent checked work and the completed prompt on
the same execution branch; verify upstream. Report software plan completion
separately from exact-part/board/fixture hardware qualification.
