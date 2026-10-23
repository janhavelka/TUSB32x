# Contract shared by every implementation prompt

Every numbered prompt incorporates this file. Requirements apply across stages;
later prompts must preserve earlier contracts. If evidence calls for a change,
update the decision record, API, tests, CLI, runner and docs together.

Read these companion contracts as part of this file:

- [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md): selected names, types,
  statuses, state tables, method contracts and file responsibilities.
- [CLI_CONTRACT.md](CLI_CONTRACT.md): selected command grammar, records and tools.
- [MAINTAINABILITY.md](MAINTAINABILITY.md): simple code, engineering language,
  source Doxygen and general documentation required in every stage.
- [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md): authorized delegation, model choice,
  file ownership and review responsibilities.

These are prepared defaults, grounded in the audit. Re-audit to check them, not
to restart design from a blank page. Change a default only for a specific source,
consumer or test reason; update all affected contracts and record that reason.
Concrete baseline tables take precedence over this file's broad descriptions.
Verified TI facts and the user's instructions take precedence over both.

## Git completion rule for every prompt

Follow [AGENTS.md](../../AGENTS.md#git-workflow-one-branch-for-the-whole-promptset).
The user authorizes routine commits and pushes for this repository. Keep this
entire promptset on its existing branch across stages, sessions and subagents.
After each completed prompt, commit the checked code/tests/docs and sync before
reporting completion or starting the next prompt. Also commit and sync a coherent,
checked unit during a prompt when it is ready; do not defer everything to P14.
The lead stages only task-owned work and handles Git for subagents.

Sync includes fetching, safely integrating upstream work if needed, pushing to
the configured upstream and confirming zero ahead/behind. Preserve concurrent
changes and shared history; no force-push or per-prompt worktrees. Record branch,
completed commit IDs and sync result in progress/handoff evidence. A real missing
remote/access/network or conflict is reported specifically; a failed push is not
a completed sync. Continue independent local work while that limitation remains,
preserve its unsynced status and retry when possible. Routine Git synchronization
does not authorize release/tag or registry publication, hardware flashing or
sibling changes.

## Mandatory recurring audit

Before editing for **every** prompt, inspect current relevant peers beneath the
parent `Projects/` directory. Read their AGENTS, public declarations, actual
implementations, command dispatch/help, tests, runners and workflows as relevant.
Record commit, dirty status, file paths, and what was adopted or rejected. Compare
at least two useful peers for each material design choice. Recheck conflicting
documentation against code and tests. If a peer is unavailable, record it and
use the available evidence; do not manufacture equivalence or block unrelated work.

Baseline references are TMP1x2, 24Cxx, INA228, INA3221, ADS1115, OPT4001, LDC1614,
BME280, SHT3x-main, LSM6DS3TR, MB85RC, MCP45HVX1, PCA9555, RV3032-C7, SCD41,
SSD1315 and TCA9548A. Re-discover new siblings in stage 00 and the final audit.
FieldCore-node is a consumer reference. Exclude archives, `.pio` dependencies,
worktrees and empty placeholders from independent-library counts.

Borrow conventions, not unrelated device features. Do not add EEPROM geometry,
temperature conversions, FIFO, SMBus general-call reset, CRC or ADC calibration
because a peer offers them. No copied sensor-specific symbols or README claims.

## Naming, organization and maintenance ownership

Use the exact selected names in ENGINEERING_BASELINE. Do not invent a second
name for a concept that already has one. Public methods, error names, CLI keys,
tests and docs must agree; raw backend detail does not replace a typed Status.

- Repository, manifest, CMake target and public namespace: `TUSB32x`; primary
  class `TUSB32x::TUSB32x`; public include `<TUSB32x/TUSB32x.h>`.
- Public headers in `include/TUSB32x/`: main facade, `Config.h`, `Status.h`,
  `CommandTable.h` (or the audited register-table equivalent), generated
  `Version.h`; focused internal implementation under `src/`. Additional public
  types are allowed where they improve clarity, not speculative abstraction.
- Strongly typed compact enums (usually `enum class ... : uint8_t`; wider
  bitmask/identifier types where necessary), uppercase enum values/constants,
  camelCase public methods/fields and `_camelCase` private members, consistent
  with the current peer selected in stage 00. No new lowercase namespace solely
  to follow another ecosystem's style.
- Use `Status { Err code; int32_t detail; const char* msg; }` with static-lifetime
  messages, `ok()`, `is()`, `inProgress()`, explicit bool and `Ok()/Error()`.
  Preserve useful transport detail and differentiate invalid input, unsupported
  feature, not bound, not initialized/trusted, busy, progress, cancelled, timeout,
  identity mismatch and terminal bus errors. Do not copy irrelevant sensor errors.
- The dominant audited metadata uses Thymos Solution s.r.o, `info@thymos.cz`,
  `https://www.thymos.cz`, and Jan Havelka as maintainer, with MIT for project
  code. Preserve that evidenced ownership. Do not invent a GitHub team, repository
  URL or contributor. Review actual CODEOWNERS validity for the chosen host;
  keep vendor artifacts under their original terms.
- Add AGENTS, README, CHANGELOG, CONTRIBUTING, LICENSE, Doxyfile, `.gitignore`,
  formatting/editor conventions, metadata and relevant maintenance/security
  guidance in the established style. Avoid boilerplate that makes false claims.

## Portable transport and application ownership

- Standard C++17 core; no framework includes, RTOS, threads, loggers, global
  instances, exceptions as control flow, heap allocation, platform time/sleep,
  GPIO configuration, Wire/bus handles, or owning peripheral lifetimes.
- Application callbacks perform synchronous **terminal** transfers, enforce a
  finite timeout including lock acquisition, serialize the physical bus and
  guarantee return before borrowed buffers expire. No retained pointer, pending
  DMA, callback reentry, hidden retries, bus initialization or bus reset.
- Follow the sibling write and combined write/read callback shapes with address,
  buffers/lengths, `timeoutMs` and `void* user`. A register read always explicitly
  sets its pointer. Specify whether the callback uses STOP-separated phases or
  repeated START based on TI evidence and adapter validation: the datasheet
  describes a STOP after pointer selection and does not mandate repeated START.
  The application must serialize the complete logical access. One aggregate
  callback timeout covers lock acquisition and all bus phases, not a full new
  timeout for each phase. Count and bound underlying physical work honestly.
  Any typed transport extension
  must be justified by side-effect evidence, not require a whole new framework.
- Do not invent NACK phase, partial counts or proof of no write from a generic
  framework error. Preserve unknown/possibly-applied outcomes. A write that
  returned an error may have changed the device.
- Application chooses exact variant/address, bus speed, clock, IRQ/power pins,
  locking, scheduling, policy, recovery and teardown. No process-wide singleton.
  Two independent driver instances must be usable on one externally serialized bus.
- Core instance access is serialized by its owner. Do not hold a bus lock across
  poll intervals. No I2C or logging in GPIO ISR; ISR signals an owner only.

## Cooperative lifecycle and evidence

- Binding, validation, operation admission, cancellation, cached getters, retained
  result access and normal teardown are bus-silent. Name any synchronous wrapper
  explicitly and document its worst-case work. `end()` is not a chip reset.
- Expose a consistent start/poll/cancel/result operation model, one pending
  operation per instance, with generation/token protection and retained terminal
  results. Reject busy/invalid/stale requests without traffic or evidence loss.
- Define `maxTransfers` as **terminal transport callback attempts**, following the
  owner's scheduling boundary. Default poll invokes at most one; zero invokes
  none; a failed callback consumes one. A STOP-separated pointer-write/read can
  contain two bus transactions within that callback, so never call this a
  one-transaction guarantee. Document per-callback byte/transaction/time bounds;
  record adapter transaction counts separately where observable. If a consumer
  needs a stricter physical-transaction bound, provide a validated framing/adapter
  contract that meets it. Limit CPU work as well as I2C. Larger callback budgets
  are explicit and bounded, never implicit busy loops.
- Supply monotonic time externally. Use rollover-safe comparisons and bounded
  intervals; clip callback timeout to operation time remaining. Waiting for hardware
  or retry eligibility never sleeps, spins or starts a second hidden transfer.
  Recheck deadline after callback return using authoritative clock/completion
  evidence; if unavailable, defer time-sensitive completion to the next fresh
  owner timestamp rather than claiming a stale time is current. For budgets
  larger than one, charge
  aggregate elapsed time and refresh time between callbacks. Anchor hardware
  waits to known completion time or the next fresh owner timestamp; stale
  pre-callback time must not shorten a required hardware delay.
  The selected millisecond clock contract and one-tick hardware-wait margin in
  ENGINEERING_BASELINE also apply. A floor-rounded timestamp cannot by itself
  prove that five elapsed ticks means five complete milliseconds after the write.
- Separate desired settings, observed register values and trusted applied state.
  Track configuration generation and observation age/provenance. Raw writes,
  external reset/power loss, partial apply and ambiguous writes invalidate trust
  as appropriate; a later successful unrelated read cannot restore it.
- Recovery is an explicit device operation. It does not recover a shared bus or
  force a port role automatically. After an uncertain role/termination/reset
  write, observe/reconcile before a new policy action; never blindly replay it.
- Cancellation/timeout are local outcomes, not rollback. Preserve possible write
  effects and disabled-termination/reset-pending state through end/rebind as
  needed. Document explicit evidence-forgetting and cleanup/reconciliation paths.

## Health, status and USB meaning

- Expose neutral `TransportStats` and `DriverDiagnostics` following the newer
  owner-oriented peers. Offer an optional passive health projection/helper with
  sibling vocabulary `UNINIT`, `READY`, `DEGRADED`, `OFFLINE` for diagnostics.
  Define every projection transition and configurable failure threshold. Health
  is not operation admission, retry, recovery or scheduling policy. The helper's
  thresholds belong to diagnostic/application policy, not the silicon profile.
  Health getters and reset-statistics helpers perform no transport.
- Count **tracked callback** transfer attempts once; count failed preconditions,
  invalid commands, pending work and valid detached states separately. No hardware
  attempt means no I2C failure. Success/failure and consecutive-failure semantics
  must be explicit and tested, including counter saturation.
- Keep an all-callback transport statistics source covering every callback,
  including raw/probe traffic. Any managed-only projection is explicitly a filtered
  view, never a hidden exclusion from the source counter. Separate callback
  attempts, backend bus transactions/bytes and filtered managed-health counts;
  do not fabricate transaction-phase information the backend does not expose.
- Raw access/probes bypass managed health; independent adapter counters include
  all callback traffic, with physical transactions separately described. Resetting
  counters never repairs state or config trust.
  An OFFLINE projection never suppresses an explicitly requested legal operation
  or starts auto-retry. Recovery remains an application decision.
- Communication health, configuration trust, data validity/freshness, USB port
  attachment, interrupt pending/acknowledged state, and operation outcome are
  independent. Preserve raw evidence if a valid transfer returns an unknown encoding.
- Model DFP/UFP relative to the **local port**; make peer-vs-local roles explicit.
  Distinguish requested role from actual attachment. Invalid/not-applicable
  orientation/current must not become a plausible default.
- Current detection/advertisement describes Type-C CC signaling, not measured
  current or a USB PD contract. Default current is not an unconditional 500 mA
  allowance. Expose accessory/active-cable/DRP capabilities only when documented.
- No external VBUS/VCONN switch, charger, load-current limit, USB mux, stack role
  or power policy ownership in the core. Board examples describe where applications
  decide. TUSB322I's autonomous VCONN can change as an effect of explicit role/reset/
  termination operations; expose/document that effect rather than promising none.

## Family and silicon correctness

- Primary device TUSB320LAI; explicit verified tiers include LAI/HAI, LI/HI and
  original TUSB320/TUSB320I. Keep separate enable polarity, addresses, revision
  evidence, controls, debounce encodings and timing capabilities. Unsupported
  variants/features fail before traffic. TUSB321 GPIO-only is outside I2C support.
- Audit TUSB322I as a separate family extension and implement evidence-supported
  operations if the specification closes. Its identity/defaults/VCONN and register
  differences need explicit capability handling. Track unresolved advertisement
  access wording; do not silently include it or omit it from the family matrix.
- Never claim every TUSB32x part is register compatible. Package/order suffixes
  and a matching ID cannot alone establish capabilities or enable polarity.
- Verify all masks/defaults/addresses/endian behavior against official TI PDFs.
  Preserve reserved and RO semantics; deliberately compose writes to registers
  containing W1C and reset bits. A read-modify-write is not generically safe.
- Reading status does not imply interrupt acknowledgement. Preserve pending IRQ
  across unrelated settings updates. Bounded service reads, publishes evidence,
  acknowledges explicitly and rechecks as documented; it does not promise lossless
  edge capture from a coalescing hardware latch or atomic multi-register snapshots.
- Address/role/enable straps, startup timing, VDD restrictions and board wiring
  assumptions are documented inputs. Do not turn ambiguous datasheet startup text
  into a fabricated timing guarantee. GPIO-only mode cannot be accessed by I2C.

## Examples, CLI and runners

- Keep one portable command processor under `examples/common/` and thin Arduino
  and native IDF front ends. Preserve peer grouped help, aligned command column,
  optional ANSI colors, command aliases, typed status formatting and diagnostics.
- Cached vs live commands and configuration-changing commands are explicit.
  Startup/help/health do not reset, change role, acknowledge IRQ or drive power.
  Strict parse/arity/range validation precedes any mutation or transfer.
- Expose all implemented applicable chip features, owner stepping/budgets/results,
  health, trust, provenance, transport counters, variant capabilities, register
  inspection and bounded diagnostics. No irrelevant peer command filler.
- Console processing and output are bounded; delayed USB or a lost host cannot
  stall the owner. Handle line overflow and report dropped output. No unbounded
  Serial-ready wait, watch loop, memory-growing latency log or background auto-reset.
- Python runners validate fresh correlated replies, exact completion and firmware
  identity; stale `PASS` text is not evidence. Keep transcripts and structured
  reports bounded/streamed with separate host and flashed-firmware provenance.
- Default HIL checks observe. Role/current/reset/termination experiments require
  explicit runner selection and a suitable fixture. Capture baseline and verify
  appropriate restoration, but never blindly restore a role after attachment or
  target identity changed. Missing fixtures are NOT RUN, not PASS.

## Packaging, verification and reporting

- Documentation is part of each behavior change. Write/update Doxygen in C++
  headers and source, Python docstrings, README/CHANGELOG and affected guides
  before completing the stage. P13 reconciles already maintained documentation.
- Keep implementation readable and small using MAINTAINABILITY. Avoid speculative
  abstractions, duplicate logic, unnecessary aliases and comments that restate code.
- Subagents and model selection are explicitly allowed under AGENT_WORKFLOW.
  Use simple engineering English in their tasks, output and the final implementation.

- `library.json` is version truth; `scripts/generate_version.py` provides
  `sync/check/bump/set`, with stable generated headers and honest unknown Git
  identity in source archives. Firmware identity belongs to example builds.
- Support ordinary CMake C++ consumers, PlatformIO Arduino and native ESP-IDF
  component consumers. Headers/core compile/link without framework include paths.
  Tests off by default for downstream consumers; no Python requirement to link.
- Use pinned, verified toolchain/action versions according to current peers and
  official SDK documentation. Do not install a second Windows PlatformIO Core.
  Reuse `scripts/pio.cmd`; Linux CI may install its pinned isolated tool version.
- Add behavioral host tests, CLI execution tests, runner fake-serial tests,
  version/boundary/package guards, clean package consumers, Arduino S2/S3 builds,
  native IDF S2/S3 builds across the selected supported matrix, strict warnings
  and appropriate sanitizers. Register tests so CI actually runs them.
- Do not pass an asserted behavior using text search alone. Include a non-Arduino
  consumer and an owner scheduler fixture, with fault injection for every protocol
  boundary. Do not disable assertions in Release-mode tests accidentally.
- Maintain a feature-to-API-to-CLI-to-test-to-source matrix and `docs/validation.md`
  with PASS/FAIL/NOT RUN/BLOCKED per actual check. Hardware evidence identifies
  exact part, board, framework, firmware, wiring, fixture and date.
- Follow parent CI policy for meaningful changes and exact-final-commit checks
  when a remote exists. Never count prior green CI as verification of new edits.
  A missing remote/hardware/tool is a limitation, not permission to falsify results.
