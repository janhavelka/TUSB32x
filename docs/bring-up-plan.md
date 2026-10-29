# First-board bring-up plan

Prepared 2026-10-29. No board, schematic, port or authorized hardware session
exists yet. Every physical stage below is **NOT RUN**. Use the
[design checklist](board-design-checklist.md) first and retain the fuller
[hardware evidence rules](hardware-validation.md) and [runner guide](hil-runner.md).

## Prepare before a board exists

Record the exact target/role decisions in the checklist. Prepare an evidence
directory per later session containing schematic/BOM/layout revisions, assembly
markings, firmware commit/framework/target/build flags, equipment/probe settings,
rail/strap measurements, raw serial log, runner report and waveform captures.
Never infer flashed firmware identity from the host checkout.

Plan a current-limited supply, DMM, oscilloscope, voltage-compatible I2C logic
analyzer, suitable USB console/UART connection and known Type-C partners/cables.
Later role/current/accessory tests need controlled fixtures and loads appropriate
to the chosen power design; they are not part of the first read-only test.

Generate a no-I/O plan now, from the repository root:

```powershell
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite readonly --out build/preboard-readonly-plan
```

Expected: exit 2, hardware NOT RUN, no serial port opened. Change variant/address
only after selecting the actual design. This command validates plan construction,
not wiring or silicon behavior.

## Staged acceptance

| Stage | Work and evidence | Continue only when |
| --- | --- | --- |
| 1. Unpowered inspection | Check assembly orientation, net continuity, shorts, straps, sensing resistance and separate power paths against schematic. | No unresolved assembly discrepancy; planned rails and roles documented. |
| 2. Controlled power | Establish the chosen safe fixture state; measure VDD/ramp, enable, I2C rail and power-off backfeed cases. Avoid uncontrolled source-to-source connection. | Values meet exact-part requirements; STARTUP_EN has a documented board solution. |
| 3. Adapter qualification | Establish SDK/version, shared serialization, same-core/synchronous constraints, finite callback overhead allowance and accepted SDK cleanup. Measure worst-case scheduling/lock/phase behavior under stated load. | Adapter prerequisites are evidenced; do not substitute a fake-test overhead constant or turn BOARD_READY on merely to bypass checks. |
| 4. Console identity | Build/flash only the identified authorized target. Record version, framework/target, Git/dirty/build records, bus pins/framing/timeouts and queue-loss counters. | Firmware matches intended build and physical target; no unexplained startup or input-loss error. |
| 5. Read-only initialization | Let application-owned startup finish and consume its own result. Then run the explicit sequence below, with captured bus frames. | Exact identity/revision and complete configuration observations; no data writes/reset/ACK. |
| 6. Detached observation | Use a declared detached fixture and refresh status. Save raw09/08, validity, sequence and acquisition time. | Detached interpretation is coherent; orientation/current NOT_APPLICABLE is not a usable direction/current claim. |
| 7. Qualified partner observation | Attach a known compatible partner in both orientations; record CC and VBUS independently, plus raw/status/time evidence. | Observations agree with the declared fixture; register freshness is not assumed to prove current resampling. |
| 8. Explicit mutation suites | Separately authorize profile/IRQ/reset/recovery/fault tests with fixture preconditions and expected results selected beforehand. | Exact runner reports and traces support the scoped claim; uncertain writes are never blindly retried or rolled back. |

For stage3 use the [Arduino adapter](arduino-transport.md) or
[native-IDF adapter](idf-transport.md) qualification contract. Stages can iterate
during engineering, but a diagnostic experiment is not a passing qualification.
The examples default to diagnostic-only configuration until board readiness and
qualified timing inputs are supplied. Console readiness is not I2C readiness.

## First explicit CLI sequence

Use one human or runner owner, never both concurrently. Once startup owns no
active or retained result, issue commands individually and inspect each response:

```text
version
bus
console
capabilities
job
init
```

If the driver is unbound, use `bind` first after confirming configured
variant/address. If a result already exists, identify its owner; do not consume
a foreign result to free the slot. `init` admission reports a nonzero token.
With normal automatic polling enabled, wait for that token's terminal result,
query `result <token>` if needed, inspect the operation status, then explicitly
`result take <token>` using that exact decimal token. Delivery OK is not proof
the operation succeeded. No literal `<token>` is entered.

Only after successful initialization and exact take:

```text
read
```

Inspect and take its new exact terminal token in the same way, then query:

```text
sample
freshness
profile show
bus
console
```

These cached queries perform no I2C. `read` is a three-read 09/08/09 observation,
not an interrupt ACK. Verify the actual adapter trace uses explicit pointer
selection and the documented framing; one callback can contain multiple physical
bus phases. Primary write-callback count must not increase during this read-only
sequence; write-read callbacks do send pointer bytes.

For LAI expect ascending ID bytes `30 32 33 42 53 55 54 00` and revision02.
An unexpected revision, failed read or incomplete output stops acceptance; do
not change the configured model to make initialization pass. See the
[identity policy](variant-matrix.md#identity-and-revision-policy).

No `apply`, `reset`, `irq ack`, `irq service`, `wreg` or
`recover-dead-battery` belongs in this first read-only sequence. Base 320/I later
source startup requires a separately selected recovery after stable VDD and valid
identity; later variants do not use that workaround. Reading status never hides
a reset to refresh current detection.

Use the physical-run recipe in [hardware validation](hardware-validation.md#firmware-and-authorized-observation)
when a port/firmware/fixture is identified. Select DTR/RTS deliberately: opening
the port can reboot a board. Missing captures, overflow, unexpected reboot,
timeout, foreign result or uncertain cleanup prevents PASS. Preserve the failed
artifacts, stop further mutations and reassess the physical state. A local end
or cancellation does not switch off VBUS or undo a write.

## Software checks that need no board

No account link is required. Existing host tools already cover the portable
suite. The remaining IDF6.1 check needs an isolated audited SDK6.1.0 installation,
its matching Python environment and S2/S3 toolchains; the validation tool accepts
`--idf-path` and `--idf-python`. Do not replace the working IDF5.5.5 environment
or install a second PlatformIO Core.

The sanitizer check needs a supported Linux C++ compiler plus ASan/UBSan runtimes,
CMake/Ninja and the test Python dependencies. An existing suitable Linux machine,
an explicitly provisioned WSL/container, or an explicitly requested CI run can
provide it. These are tools/environment paths, not links to a hardware device.
No tool installation, CI dispatch or hardware access is performed by this plan.
