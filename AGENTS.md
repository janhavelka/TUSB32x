# TUSB32x engineering and agent guidelines

## Scope and starting point

This repository will provide a general-purpose, framework-neutral C++17 library
for the verified TUSB320 family, starting with TUSB320LAI. It currently contains
the audit, TI references and implementation prompts. Do not describe planned
interfaces, commands or builds as implemented or tested.

Execute the numbered prompts when asked to implement the library. A request to
edit guidance or review the audit does not itself start driver implementation.

- Read [the parent rules](../AGENTS.md), [the promptset index](docs/prompts/README.md)
  and [COMMON](docs/prompts/COMMON.md) before implementation. COMMON includes the
  concrete engineering, CLI, maintainability and agent workflow contracts.
- Before **every prompt**, re-audit relevant current sibling public declarations,
  code, examples, CLI, tests, Python runners, workflows and AGENTS.md. Record
  revisions, dirty state, exact sources and conventions adopted or rejected.
  Historic reports are evidence to revisit, not permission to skip this step.
- Use [ENGINEERING_BASELINE](docs/prompts/ENGINEERING_BASELINE.md) for exact API,
  status, state and file names. Resolve source conflicts explicitly and update
  affected code, tests and documentation together. Do not add competing aliases.
- Sibling projects, including FieldCore-node, are read-only references unless the
  user requests changes there. This library must also serve ordinary C++ owners,
  Arduino and native ESP-IDF without a FieldCore dependency.

## Git workflow: one branch for the whole promptset

The user authorizes commits and synchronization for this work. These rules apply
to each numbered implementation prompt and to completed maintenance work here.

- Before editing, inspect status, staged changes, current branch and upstream.
  Fetch the configured remote and compare histories. Fast-forward when safe;
  preserve local and concurrent work. A dirty worktree alone is not a blocker.
- At the start of a **new promptset execution**, create and check out one new
  branch from the synchronized intended base before prompt 1 implementation work.
  Use a descriptive name such as `promptset/tusb32x-implementation-YYYYMMDD` and
  an unused suffix if needed. If the user already created a branch for this run,
  use it instead of creating a second one. Record its name and base commit in
  the progress file; establish its same-name upstream on the first push.
- Keep that **same execution branch for the entire promptset**, including later
  sessions and subagents. Resuming a recorded execution does not create a new
  branch. Do not create a branch/worktree per prompt or agent, or move work to
  main when a prompt finishes. A later, separate promptset execution gets its
  own new branch. Editing or auditing prompts alone is maintenance, not the start
  of their implementation execution.
- Assume other agents or the user may be editing. Stage only files or hunks owned
  by this task, including reviewed subagent changes. Never stash, discard,
  overwrite or commit unrelated work. Inspect the staged diff before committing.
- **Commit and sync after every completed prompt**, before reporting completion
  or starting the next prompt. Also commit and sync during a prompt whenever a
  coherent, checked unit is ready: for example, an API with its docs and tests,
  a verified protocol step, or a complete runner improvement. Do not wait for the
  entire promptset or force unrelated work into one large final commit.
- A commit must stand on its own: include required implementation, relevant
  tests, source Doxygen, affected guides and changelog changes. Run appropriate
  checks first. Do not commit broken intermediate edits simply to create a
  checkpoint; do not create empty commits when a completed review changes nothing.
- Write concise commit messages describing the resulting change in plain
  engineering language. Mention a prompt number when useful, but give the actual
  change a name rather than only "prompt done" or "updates".
- **Sync means fetch, safely integrate upstream changes if needed, push this
  branch to its configured upstream, and verify synchronization.** A local commit
  or successful fetch alone is not a sync. If histories diverge, preserve both
  histories and integrate with a normal merge on this branch when safe; resolve
  relevant conflicts and rerun affected checks. Do not rewrite shared history or
  force-push. Verify local/upstream commit equality and zero ahead/behind after
  the final fetch. Check the working tree for uncommitted task changes as well.
- If the established remote exists but upstream tracking is missing, set tracking
  for the intended same branch when unambiguous. Do not invent a remote or a new
  repository. Missing access/network, branch protection or conflicts that cannot
  be resolved without disturbing unrelated work are concrete sync limitations:
  retain completed local work and report the exact failure, local commit and
  remaining step. Never claim a failed push is synchronized or bypass protection.
  Continue independent authorized local work while synchronization is unavailable;
  keep the unsynced state visible and retry when possible. Do not turn a network
  outage into a reason to stop unrelated implementation or to claim sync success.
- No further permission is needed for these routine commits and pushes. Release
  tags, registry/release publication, hardware flashing and sibling changes remain
  separate actions requiring the corresponding task authorization.
- The lead owns staging, commits, fetch/integration and pushes for shared work.
  Subagents do not mutate Git or switch branches. Keep commit/sync evidence in
  progress reports and final handoffs. Record completed commit IDs in the next
  progress update; do not create a self-referencing commit-hash update loop.

## CI and local verification

- Inspect current workflows and checks before implementation. Reuse their
  commands, pinned tools, generated-file rules and test registration.
- Run focused local checks for each meaningful change. Documentation-only work
  needs applicable text/link/consistency checks, not firmware or remote CI after
  every commit. Committing and syncing does not imply dispatching CI every time.
- For major behavior/API, integration, build/toolchain/dependency/workflow or
  release changes, follow the parent policy: verify existing CI on the **exact
  final commit** through terminal results; dispatch when needed. Prior green CI
  does not verify new edits. Do not repeatedly dispatch unchanged suites.
- Fix confirmed failures without weakening assertions, skipping required suites,
  suppressing documentation warnings or changing triggers to obtain green output.
  Respect existing hooks and required checks.
- Test public behavior, including rejected inputs and failures at write/timing
  boundaries. Text searches supplement tests; they cannot prove protocol,
  portability, bounded work, allocation behavior or CLI execution.
- Register suites so they actually run; zero tests is not success. Keep assertions
  active in Release tests. Use supported strict warnings and host sanitizers.
- Separate host tests, package consumption, linked Arduino/native-IDF firmware,
  remote CI and physical hardware evidence. Report PASS/FAIL/NOT RUN/BLOCKED
  accurately. Mocks, SDK-header compilation and workflow files prove none of the
  later layers by themselves.

## Repository layout and boundaries

The intended responsibilities below become real as their stages are implemented.
Do not create empty files or successful hardware stubs to fill the tree.

| Location | Responsibility |
| --- | --- |
| `include/TUSB32x/` | Public TUSB32x.h, Config.h, Status.h, CommandTable.h and generated Version.h |
| `src/` | Binding, protocol, operations, port decoding and passive diagnostics |
| `examples/common/` | One portable CLI, bounded console and shared example configuration |
| `examples/arduino/`, `examples/01_basic_bringup_cli/`, `examples/02_owner_service/` | Application-owned Arduino transport and examples |
| `examples/esp_idf/` | Native IDF transport and application; no Arduino facade |
| `examples/host/basic/` | Ordinary C++ consumer with application callbacks |
| `test/` | Protocol, owner, port, diagnostics, CLI, console and consumer tests |
| `tools/`, `scripts/` | Guards, Python runners/tests, version generator and PlatformIO wrapper |
| `docs/` | Maintained API/integration/CLI/testing guides, decisions and validation |
| `docs/reference/` | Vendor artifacts and provenance; excluded from core compilation |
| `.github/workflows/` | Registered host, documentation, package and framework checks |

- Namespace and primary class: `TUSB32x::TUSB32x`; include
  `<TUSB32x/TUSB32x.h>`; core CMake target `TUSB32x`.
- Public headers and core sources remain standard C++17. No Arduino, Wire,
  ESP-IDF, FreeRTOS, logging, platform clock/delay, global bus or heap allocation.
  Framework APIs belong in their application adapters.
- `examples/common/` is example code, not a transitive core dependency. It must
  compile on the host. Keep native IDF native; do not introduce Arduino shims.
- Plain CMake/package consumers must not need sibling include paths, Git, Python,
  Doxygen or framework headers merely to compile and link. Tests default OFF.

## Maintainable code and naming

- Prefer direct, readable code, small value types and explicit state transitions.
  Check whether existing code can be reused, simplified or removed before adding
  a new class, file or abstraction. Each addition needs a current responsibility.
- Keep one transport path, one safe register-write composer and one operation
  engine. Do not add generic managers, registries, compatibility APIs, plug-in
  systems or speculative extension points.
- Bound loops, state transitions, callbacks, waits, queues, buffers and output.
  No hidden retries, allocation, busy-wait polling loops, fake success or silent
  failures. Fake devices belong in tests and clearly identified host examples.
- Use PascalCase types, camelCase functions/locals/fields, `_camelCase` private
  members and CAPS_CASE constants/enum values. Use typed enums and `constexpr`
  constants instead of macros for values. Follow the checked-in format settings.
- Use simple engineering English in comments, Doxygen, diagnostics, guides,
  commit messages and handoffs. Explain causes, limits and decisions; avoid
  marketing claims and comments that merely narrate assignments.

## Transport, ownership and cooperative operations

- The application owns bus creation, pins, locks, device selection, monotonic
  time, scheduling, IRQ handling, retry/recovery and USB power/data policy. Core
  callbacks borrow the transport; they never reconfigure or recover a shared bus.
- Callbacks return a terminal Status, respect one aggregate timeout including
  lock acquisition and all phases, never retain buffers, and never re-enter the
  instance. Do not leak framework errors or infer a NACK phase from generic errors.
- Owner access is serialized; public APIs are not ISR-safe. An ISR signals the
  owner, which performs library work later. No I2C, logging or blocking in an ISR.
- `bind`, validation, `start*`, cancellation, cached access, result delivery,
  invalidation and `end` are bus-silent. Only `poll` advances managed I2C work.
  Explicit `readRegisterSync`/`writeRegisterSync` diagnostics have their own
  one-callback limit and cannot bypass required multi-step silicon procedures.
- One active operation and one retained terminal result use instance-local tokens.
  Reject replacement work until the result is taken. Admission returns
  IN_PROGRESS; delivery returns OK even when the copied result contains failure.
- Count terminal callback attempts, not assumed physical transactions. Zero poll
  budget means no callback; default budget is at most one. A combined callback
  may include pointer selection and read, with one aggregate timeout.
- Use the baseline's owner-supplied uint64 millisecond domain, bounded intervals,
  rollover rules and fresh completion-time handling. Status-only callbacks do
  not prove elapsed time. Preserve the separate 1 ms hardware-wait margin; never
  extend the immutable operation deadline or shorten a TI minimum wait.
- Cancellation, timeout and `end` do not undo hardware effects. Preserve results,
  uncertain writes and termination/reset evidence. Reconciliation observes;
  it does not blindly replay a failed role change or reset.

## Status, health and device evidence

- Use `Status { Err code; int32_t detail; const char* msg; }` with static-lifetime
  messages and the exact baseline methods/error names. Preserve the original
  transport error and full backend detail. Owner TIMEOUT and I2C_TIMEOUT differ.
- Keep desired settings, observed values and verified applied state separate.
  Partial/raw/external changes invalidate trust as specified. An unrelated
  successful read does not repair an uncertain configuration.
- Count every callback in TransportStats, including diagnostics. Managed health
  evidence is an explicitly filtered view. No local rejection, wait, valid detach
  or deadline without a callback becomes a transport failure.
- Optional health uses UNINIT, READY, DEGRADED and OFFLINE as a passive projection.
  It cannot reject legal operations, schedule retries or control recovery. Clearing
  statistics cannot repair trust, erase hardware effects or consume a result.
- Port attachment, local/peer role, current indication, validity/freshness,
  configuration trust, IRQ evidence and operation outcome remain separate facts.
  Unknown or unavailable fields must not become plausible default observations.

## TUSB32x silicon rules

- Treat [CHIP_EVIDENCE](docs/prompts/CHIP_EVIDENCE.md), public implementations and
  these prompts as claims to verify, not silicon authority. Before implementing
  a register, timing or sequence, confront it with the exact current TI datasheet,
  errata and cited source bytes. Inspect helper semantics and variant context.
  Never invent missing facts or change expected tests to match copied code.
- Record conflicts and correct all affected prompts/contracts, constants, tests,
  Doxygen and guides when evidence changes. Keep unsupported facts gated; do not
  silently choose the convenient source. Hardware observations require an exact
  part/board/firmware/fixture/trace and do not establish family-wide behavior.
- Verify exact variants against the archived official TI sources before changing
  register maps, masks, identity checks, access semantics or timing. Keep source
  revisions/pages and unresolved wording visible in the specification.
- TUSB320LAI is primary; LAI/HAI, LI/HI and original TUSB320/TUSB320I have explicit
  capability differences. Gate TUSB322I operations on their own evidence. TUSB321
  GPIO-only operation is outside the I2C library. ACK or a shared ID is not proof
  of an exact variant or every family capability.
- Deliberately compose mixed RO/RW/W1C/reset writes. IRQ ACK preserves actual
  observed/trusted control bits, never unapplied staged settings. Status reading
  is not an implied ACK; coalescing latches do not promise lossless event capture.
- Keep 0A disableTerm and 45 disableRdRp distinct. Apply the verified per-variant
  role-change sequence and required wait. Do not label termination enabled while
  another applicable disable control remains effective or evidence is unresolved.
- Initialization is read-only; applying a profile and resetting are explicit
  operations. No automatic reset, role change, IRQ clear or desired-profile replay.
- Base TUSB320/I post-dead-battery recovery is the explicit
  `startRecoverDeadBattery` operation in the existing engine: qualified 0x45
  writes 0x04, at least 30 ms wait, then 0x00 and verification. The application
  establishes stable VDD and when source-capable startup needs it. Do not apply
  this sequence to later variants or infer its history from one register read.
- CC current indications are not measured load or a USB PD contract. The core
  owns no VBUS switch, charger, USB stack or external power policy. Document the
  TUSB322I's device-side VCONN effects without turning them into application policy.

## Examples, tooling and documentation

- Maintain one shared CLI and thin Arduino/native-IDF front ends using
  [CLI_CONTRACT](docs/prompts/CLI_CONTRACT.md). Keep help, aliases, strict parsing,
  typed output, bounded console work and cached/live/staged/write semantics aligned.
- Keep firmware commands tied to single core operations. Python runners sequence
  repeated reads, dumps, soak tests and fixture suites, taking each owned result.
  Do not create a second firmware scheduler or terminal-result store for them.
- Python runners must correlate fresh requests/results and check firmware-reported
  identity. Host checkout identity is not proof of flashed firmware. Preserve
  failed/incomplete evidence; missing fixtures are NOT RUN, not PASS.
- On Windows use `scripts/pio.cmd` once created, with the existing VS Code-managed
  PlatformIO executable. Do not install another PlatformIO Core. If a required
  tool is unavailable, report that check and continue independent work.
- Use actual repository commands once implemented. Initial intended checks are
  CMake/CTest with `TUSB32X_BUILD_TESTS=ON`, version `check`, Doxygen, contract,
  Python and package guards, then relevant S2/S3 Arduino/native-IDF links. The
  prompts define exact names; no command is considered passed merely by existing.
- Documentation accompanies each behavior change: file/type/member/function
  Doxygen, Python docstrings, affected README/CHANGELOG/guides and validation.
  Document units, ownership, lifetimes, preconditions, side effects, error meaning
  and work bounds. Explain subtle masks, wait anchors and state invariants near
  the implementation. Prompts 35–36 reconcile documentation maintained throughout.
- Follow [MAINTAINABILITY](docs/prompts/MAINTAINABILITY.md). Documentation checks
  must catch missing comments and hidden public APIs, not just produce no warnings.
- `library.json` is version truth once introduced. Use the generator's
  sync/check/bump/set workflow; do not hand-edit generated Version.h. Keep package,
  component, CMake and Doxygen versions aligned. Do not bump a release per prompt.
- Preserve evidenced maintainer/license metadata and vendor terms. Keep reference
  URLs, revisions, retrieval dates and SHA-256 hashes. Do not compile or package
  vendor reference sources as production code or invent tested support claims.

## Delegation and completion

- Subagents and model selection by task complexity are authorized. Follow
  [AGENT_WORKFLOW](docs/prompts/AGENT_WORKFLOW.md), use available runtime models,
  assign disjoint write scopes and freeze shared contracts before parallel edits.
  Delegate independent work; the lead integrates and reviews returned evidence.
- Keep `docs/implementation-progress.md` current during execution: prompt status,
  branch, sources re-audited, decisions, checks, completed commits, sync status and
  unresolved work. Use `docs/design-decisions.md` for material contract choices.
- Before finishing each prompt: review the full change, run appropriate checks,
  update documentation, commit coherent completed work, sync and verify upstream.
  Fix locally resolvable failures. Record real limitations rather than claiming
  completion from plans, tests that did not run or an unsuccessful push.
- Handoff states the result, validation and limits, branch/commit and sync result,
  with links to useful files. Keep hardware qualification separate from software
  readiness and avoid claiming support beyond the evidence.
