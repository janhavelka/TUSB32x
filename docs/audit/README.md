# TUSB32x preparation audit

Audit date: **2026-10-23**. The result is an implementation specification and
promptset. No TUSB32x driver, example firmware or hardware qualification has been
implemented by this preparation task.

## Scope and evidence

The parent `Projects/` directory contains **17 populated standalone I2C device
libraries** relevant to this review. `FieldCore-node` was inspected as an
application consumer, interpreting the user's `fieldcore-noce` reference.
`EEPROM_24Cxx` is empty; active EEPROM work is in `24Cxx`.

Top-level manifests and source trees were checked against prior inventories.
ADS1261/MAX31865 use SPI, AT21CS11 uses single wire, EE871-E2 uses E2 signaling,
and other serial/Modbus drivers, board applications and infrastructure utilities
are not additional standalone I2C device libraries. Archives, worktrees and
PlatformIO dependency copies were not counted again.

All 18 populated inspected repositories were clean at the captured local
revisions. This does not establish upstream synchronization or hardware state.
No sibling changes, fetches, builds, tests, flashes or remote CI runs were made.
TUSB32x was initially an empty folder without Git metadata.

| Evidence | Contents |
| --- | --- |
| [Repository snapshot](repository-snapshot.json) | Exact local commits, versions, branch/dirty state, manifest ownership and license |
| [Primary libraries](primary-libraries.md) | TMP1x2, 24Cxx, INA228, INA3221, ADS1115 and OPT4001; source-line evidence and conflicts |
| [Consumer and tooling](consumer-and-tooling.md) | Eleven other I2C libraries, FieldCore contracts, CLI/runners/tests/package/CI conventions |
| [TI family research](ti-family.md) | Official register/variant/timing evidence, limits and unresolved wording |
| [Reference archive](../reference/README.md) | Seven official PDFs with retrieval metadata and SHA-256 checksums |
| [Promptset](../prompts/README.md) | Master execution instruction, common contract and fifteen implementation stages |
| [Preparation validation](promptset-validation.md) | Checks of the promptset and reference artifacts, distinct from library validation |

## Audited libraries

| Directory | Manifest version | Main reference value |
| --- | --- | --- |
| TMP1x2 | 1.0.0 | Small TI family, shared CLI, owner jobs, USB console, runners and clean packages |
| 24Cxx | 1.0.0 | Shared neutral CLI, strict parser, bounded serial evidence, metadata/reference archive |
| ADS1115 | 2.0.2 | Transport/profile split, operation tokens and configuration provenance |
| BME280 | 2.2.0 | Byte registers, staged settings and trust independent of health |
| INA228 | 3.0.4 | Event evidence, explicit acknowledgement, CLI diagnostics and transfer assertions |
| INA3221 | 3.2.0 | Result/effect evidence, owner time and failure/cancellation behavior |
| LDC1614 | 3.2.0 | Current owner-first API, variant capabilities, neutral diagnostics and command contracts |
| LSM6DS3TR | 2.1.0 | Passive binding, external scheduling, tokened operations and bounded owner soak |
| MB85RC | 4.2.0 | Family descriptors, terminal transport/effect evidence and strict HIL reports |
| MCP45HVX1 | 2.0.0 | Typed mutation controls, executable CLI tests and validation entrypoint |
| OPT4001 | 1.2.2 | Terminal/help presentation; legacy lifecycle needs selective treatment |
| PCA9555 | 3.0.3 | Byte-register mutation, external ownership and write-effect reporting |
| RV3032-C7 | 3.1.0 | Status/health diagnostics and strong runner parsing; lifecycle differs |
| SCD41 | 1.3.2 | Long operations, result slots, effect evidence and clean consumers |
| SHT3x-main | 1.9.0 | Shared command processor and logical/physical diagnostic distinctions |
| SSD1315 | 4.0.3 | Bounded work/console/soak; public naming differs from dominant convention |
| TCA9548A | 1.1.0 | Shared-bus discipline, trust, registered native/console/transport suites |

## Selected direction

This is a synthesis, not a claim that the siblings share one identical API.
The implementation agent must repeat the relevant audit before **every stage**.

| Concern | Selected direction and reason |
| --- | --- |
| Maintenance identity | Dominant Thymos Solution s.r.o / Jan Havelka metadata and MIT project code; preserve copied notices; no invented remote/team |
| Public structure | `TUSB32x::TUSB32x`, `include/TUSB32x/`, C++17, source/tests/tools/scripts/docs and shared example processor |
| Runtime ownership | External application owns bus, time, serialization, GPIO, power/USB policy, scheduling and recovery |
| Operations | Passive binding/admission/cancel/end; tokened retained results; explicit bounded owner polling, as in recent LDC/LSM/ADS/INA |
| Transfer budget | One terminal callback by default; distinguish callback attempts from physical bus transactions if STOP-separated pointer/read uses two transactions |
| Health and statuses | Familiar structured Status, neutral TransportStats/DriverDiagnostics; optional passive four-state projection that cannot block/retry operations |
| State confidence | Desired settings, applied trust, observed port status, freshness, IRQ evidence and hardware effects are separate facts |
| CLI | TMP/24Cxx/SHT-style shared neutral implementation with familiar grouped help, strict parsing, cached/live distinction and owner jobs |
| Platforms | Plain C++ first; Arduino S2/S3 and true native IDF adapters/examples; no FieldCore or ESP32 requirement in the core |
| Host tooling | Actual CLI fixtures, strict correlated serial responses, firmware provenance, bounded transcripts and explicit incomplete verdicts |
| Distribution | Generated version metadata, extracted renamed consumers, no sibling paths or Python/Git requirement just to compile the package |
| Verification | Behavioral host/fault tests, executable CLI/runner tests, actual firmware links, exact-commit CI where available, physical tests reported separately |

Important conflicts are recorded with source paths in the two library reports.
Examples include OPT4001's best-effort writing `end()`, OFFLINE scheduling behavior,
stale architecture guidance, varying health counter lifetimes, mixed shared-CLI
portability, and inconsistent example/SDK conventions. These are reasons to audit
current implementations instead of cloning a library wholesale.

## Family scope

The initial complete driver target is the six explicitly documented TUSB320
variants: TUSB320, TUSB320I, TUSB320LI, TUSB320HI, TUSB320LAI and TUSB320HAI, with
LAI as the primary acceptance device. Addresses and controls differ between
generations; exact BOM selection is required. [TI family evidence](ti-family.md)

TUSB322I is a mandatory separately audited extension with distinct ID, reset-mode
meaning and autonomous VCONN/fault semantics. Its unresolved advertisement access
wording must be closed or the affected operation explicitly unavailable. TUSB321/
TUSB321A are GPIO-only and excluded from the I2C driver. Other prefix-similar
devices require a new capability audit. [TI source archive](../reference/README.md)

The promptset specifically guards mixed RO/RW/W1C/reset registers, staged-vs-applied
ACK configuration, incomplete role transitions, timeout/cancel effect retention,
IRQ coalescing, snapshot coherence limits and conflicting startup wording.
It does not turn a CC controller into a USB PD stack or a board power policy.

## Expanded implementation instructions

The follow-up prompt revision turns the audit choices into concrete contracts:

- [Engineering baseline](../prompts/ENGINEERING_BASELINE.md): exact proposed API,
  errors, state/health names, fields, lifetime rules and source responsibilities.
- [CLI/tooling contract](../prompts/CLI_CONTRACT.md): selected grammar, diagnostic
  records, bounds, test paths and runner semantics.
- [Maintainability](../prompts/MAINTAINABILITY.md): direct code, plain engineering
  language, Doxygen in source and affected general docs updated every stage.
- [Agent workflow](../prompts/AGENT_WORKFLOW.md): user-authorized delegation,
  model choice by complexity and independent review with disjoint file ownership.

These are selected TUSB32x rules, not a claim that every sibling uses identical
symbols. The numbered stages contain current peer paths, chip tables, concrete
tests and documentation deliverables. Read the updated validation record for the
scope of this documentation-only revision.

## Repository guidance adopted from sibling AGENTS

On 2026-10-23 the guidance was compared again against TMP1x2, 24Cxx, LDC1614,
LSM6DS3TR, INA3221 and ADS1115. Their current AGENTS.md files consistently require
one promptset branch, checked commits/pushes for completed chunks, preservation of
concurrent changes, lead-owned Git operations and confirmed upstream sync.
The user's explicit request makes that workflow applicable to TUSB32x.

The updated [AGENTS.md](../../AGENTS.md) also adopts LDC1614/LSM6DS3TR's explicit
ownership and source-evidence rules, INA3221/ADS1115's small-change and reuse
principles, and TMP1x2/24Cxx's portable CLI, tooling and honest-validation rules.
These were fitted to TUSB32x's selected API and TI behavior. Sensor operations,
automatic initialization replay, compatibility facades and device-specific
health counting were not copied. Parent CI policy still distinguishes routine
documentation checks from major milestones requiring exact-commit remote CI.

## Use

The subsequent [correctness and simplicity review](promptset-correctness-review.md)
records confirmed prompt defects, resolved ambiguities, simpler implementation
choices and verified TI facts that were retained. It supersedes conflicting
earlier planning observations without claiming implementation or hardware results.

Open the [master prompt](../prompts/README.md) and execute the ordered stages.
The common contract carries the selected defaults, while each stage names the
current siblings and vendor evidence to re-read. Future implementation reports
must describe actual executed checks; this preparation audit supplies no green
build, CI, electrical or hardware result for a driver that does not yet exist.
