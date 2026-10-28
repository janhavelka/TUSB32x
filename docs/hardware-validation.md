# Hardware observation prerequisites and evidence

Physical serial, I2C, CC, IRQ, VBUS and adapter timing qualification are NOT RUN.
The runnable observation/explicit fixture plans and native/fake-serial tests establish software
behavior. A dry-run, linked firmware image or host PASS does not qualify a board.
This guide records prerequisites for an operator-authorized observation run; it
does not authorize powering, flashing or changing a fixture's USB power policy.

Use the exact assembled part number and board schematic. The
[variant matrix](variant-matrix.md), [register contract](register-map.md) and
[device procedures](device-behavior.md) retain unresolved source wording and
per-operation gates. An I2C ACK or shared identity signature cannot identify
enable polarity, package routing or every family capability.

## Exact part, supply and straps

These values come from the archived official TI sources listed below. Supply
ranges are recommended operation; absolute maximum ratings are stress limits.
The address columns contain seven-bit addresses, not on-wire address bytes.

| Exact variant | Operating VDD | ADDR low / high | Enable pin 11 | Pins 3 / 4 | Physical evidence |
| --- | --- | --- | --- | --- | --- |
| TUSB320 | 2.7..5.0 V | 0x60 / 0x61 | EN_N, active low | PORT / VBUS_DET | NOT RUN |
| TUSB320I | 2.7..5.0 V | 0x60 / 0x61 | EN_N, active low | PORT / VBUS_DET | NOT RUN |
| TUSB320LI | 2.7..5.0 V | 0x47 / 0x67 | EN_N, active low | PORT / VBUS_DET | NOT RUN |
| TUSB320HI | 2.7..5.0 V | 0x47 / 0x67 | EN, active high | PORT / VBUS_DET | NOT RUN |
| TUSB320LAI | 2.7..5.0 V | 0x47 / 0x67 | EN_N, active low | PORT / VBUS_DET | NOT RUN |
| TUSB320HAI | 2.7..5.0 V | 0x47 / 0x67 | EN, active high | PORT / VBUS_DET | NOT RUN |
| TUSB322I | 4.5..5.5 V | 0x47 / 0x67 | EN_N, active low | VBUS_DET / DIR | NOT RUN |

For all seven, CC1/CC2 are pins 1/2, ADDR pin 5, INT_N/OUT3 pin 6, SDA/OUT1 pin 7,
SCL/OUT2 pin 8, ID pin 9, GND pin 10 and VDD pin 12. TUSB322I has open-drain DIR
and no PORT pin; a stray PORT reference in its operating-mode table does not add one.
TUSB321/TUSB321AI GPIO-only devices are outside this I2C library.

ADDR high references device VDD. Floating ADDR selects GPIO mode and disables
I2C, so it cannot select a third I2C address. Keep the selected strap valid before
enable. The six 320 parts sample PORT at enable with valid VDD and after software
reset: low=UFP, floating=DRP, high=DFP. A later PORT transition is not a continuous
software role command. The runner compares its selected variant/address with
firmware; it does not edit the target or drive these straps.

HI/HAI EN high must be within 1.05..3.65 V; EN absolute maximum is 4 V. A 5 V VDD
does not permit tying that enable directly high to VDD. Active-low variants use
their exact EN_N requirements. No runner generates an enable pulse. Six-part
VDD ramp is at most 25 ms, and documented I2C availability is within 100 ms after
valid VDD and active enable; this is separate from attachment readiness. LI/LAI
externally driven startup text and TI SLAAE24 disagree about a 50 ms EN_N level.
Keep STARTUP_EN unresolved until the board has an independently qualified
sequence or exact-part TI clarification; do not invent a universal startup pulse.

All six 320 parts require VDD at least 3.5 V for local 3 A advertisement. In
particular, LAI at a qualified 3.3 V supply can detect a remote 3 A source; that
local-advertisement footnote does not prohibit remote detection. TUSB322I still
requires its full 4.5..5.5 V operating range. Current indication is not measured
load, a USB PD contract, or permission to enable a charger, VBUS switch or load.

## Wiring, sensing and supplies

Use common ground and record the connector-to-chip CC1/CC2 routing. SDA/SCL need
qualified pull-ups to a 1.65..3.6 V I2C rail, never 5 V at ESP32 pins. With 3.3 V
I2C the chip VDD must remain greater than 3.0 V. Select resistor values from bus
capacitance and rise time, with SCL at most 400 kHz; the strap's typical 4.7 kOhm
pull-up is not a universal SDA/SCL value. IRQ is an optional active-low open-drain
input; qualify its pull-up and MCU voltage. An ISR only signals the owner.

VBUS_DET requires the exact sensing network: the external resistor is
855/887/920 kOhm min/typ/max, described as 900 kOhm in the pin prose, with an
internal nominal 95 kOhm pull-down. Select actual resistance/tolerance and
resistor voltage/power ratings for the fixture's full VBUS range and operating
conditions. The specified system VBUS range 4/5/28 V min/nom/max is not a pad
rating or an instruction to apply 28 V. The VBUS_DET pad absolute maximum is 4 V;
TUSB322I also specifies a recommended 0..4 V pad range. A direct VBUS-to-pad
connection is not the documented network.

Only an exclusively DFP design using a 320 variant may use the VBUS_DET omission
exception; it does not qualify a design that can enter UFP/DRP. No corresponding
exception is established for 322I. UFP attachment depends on the device's VBUS
conditions as well as CC. Record the network, intended roles and measured supply
behavior instead of inferring electrical correctness from detached samples.

Respect the exact variant's non-failsafe pins and supply sequencing. Avoid
back-driving unpowered ADDR/PORT/DIR/VBUS_DET or treating every ID pin as freely
5 V tolerant with VDD absent. The variant matrix preserves the A/V ID leakage
versus absolute-maximum conflict. The examples leave enable and power control
to the board; software readiness switches are assertions by the integrator.

TUSB322I can autonomously supply VCONN from VDD through its FET to the unused CC
after source attachment with Ra, and remove it on detach. TI requires a 5 V VDD
source after Attached.SRC; the VCONN output condition is 4.75..5.5 V with VDD
at 5 V. Its VCONN current limit is 225/300/375 mA min/typ/max, not a guaranteed
300 mA output. The specified FET resistance is at most 1.25 Ohm and VDD bulk
capacitance 10..200 uF. The full device operating-supply range alone does not
qualify VCONN. The core owns no VCONN switch policy; the six 320 parts do not
provide this VCONN function, and active-cable indication is not powered-VCONN
evidence.

## Firmware and authorized observation

The Arduino and native-IDF BoardConfig defaults select LAI/0x47, SDA GPIO4 and
SCL GPIO5, with board readiness and qualified overhead disabled. Those GPIOs
are examples, not detected fixture wiring. Inspect the
[Arduino example guide](arduino-examples.md) or
[native application guide](../examples/esp_idf/basic/README.md), together with
the [Arduino](arduino-transport.md) or [native-IDF](idf-transport.md) adapter
contract. Supply the exact board-readiness, positive qualified overhead and
SDK-cleanup acceptance inputs before hardware startup. Never substitute a host
test constant for measured timing qualification. All physical bus users must
accept the audited SDK's internal controller reset/bus-clear behavior on error.

Arduino diagnostics use the selected board's native USB console. The native-IDF
application instead uses UART0, 115200 baud, 8N1, TX GPIO43/RX GPIO44 through a
3.3 V USB/UART bridge with common ground and crossed TX/RX. Verify the selected
board and routing. Neither guide's linked S2/S3 image is physical serial proof.
Opening a port can reset a board even when DTR/RTS are requested unchanged.

First save a plan without opening any port:

```powershell
python tools/tusb32x_hil_runner.py --dry-run --variant tusb320lai --address 0x47 --suite readonly --out build/qualified-fixture-plan
```

This saves hardware NOT RUN and exits 2. For a physically authorized run, edit
the variant/address to the identified fixture and enter the actual port and
firmware values. This recipe has not been executed against hardware:

```powershell
$fixturePort = Read-Host 'Explicit authorized physical serial port'
$fixtureCommit = Read-Host 'Expected flashed firmware commit (40 hex characters)'
$fixtureFramework = Read-Host 'Expected firmware framework name'
$fixtureTarget = Read-Host 'Expected firmware target'
python tools/tusb32x_hil_runner.py --port "$fixturePort" --variant tusb320lai --address 0x47 --expect-commit "$fixtureCommit" --expect-framework "$fixtureFramework" --expect-target "$fixtureTarget" --suite readonly --suite soak --count 20 --interval-ms 1000 --duration-ms 30000 --run-timeout 90 --out build/qualified-fixture-observation-001 --dtr unchanged --rts unchanged
```

See the [runner guide](hil-runner.md) for optional pyserial installation, finite
limits and exit meanings. Expected values are operator expectations until fresh
firmware records match them. Host checkout identity and labels are separate
facts. The plans initialize read-only, own and take each admitted result, inspect
ordinary mapped registers and restore only saved local CLI settings. They never
apply, reset, ACK or recover dead battery. In particular, base 320/I recovery is
an explicit separate operation, not a prerequisite the runner silently performs.
Raw reads may change software trust evidence; 322I 09 fault-read effects remain
unresolved. Do not describe the run as electrically non-invasive.
Base SLLSEN9F section 7.3.1.2, printed page 11, requires periodic explicit software
reset to update detected current after the peer changes its advertisement while
attached. These readonly/soak plans never perform that reset: fresh register
publication alone cannot prove attached-current tracking, and this base policy
is not generalized to later variants. CC indication is not measured load or a
USB PD power contract.

## Evidence and NOT RUN criteria

Keep each output directory intact, including failed/incomplete `report.json`,
summary and streamed transcript. Record exact part/marking, board revision,
schematic/fixture, ADDR/PORT/enable straps, supply/pull-up/network values, cable
and peer, firmware commit/tree/build/framework/target, operator, date and any
logic-analyzer/scope traces. Identify what was actually measured. Advancing valid
port samples can establish fresh software observation; repeated detached state
does not establish attachment, current tracking, VBUS switching or power safety.

Missing authorized hardware/port, unqualified wiring/timing, unknown firmware,
an unavailable adapter counter or unresolved fixture prerequisite means the
affected physical check is NOT RUN/BLOCKED. A failed command, unexpected effect,
lost session or missing output stays failed/incomplete; successful local cleanup
cannot repair it. Lost-session restoration is recorded, never replayed after a
reconnect. Serial delivery, I2C waveform/timing, CC orientations/current tracking,
IRQ behavior and VBUS/VCONN measurements remain separate evidence categories.
No observation-plan PASS closes an unresolved silicon gate by itself.

## TI authority and provenance

The facts above were checked against the unmodified archived editions; printed
page numbers below identify their relevant tables. The
[source inventory](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/reference/README.md)
describes retained vendor terms; hashes and retrieval metadata are in its linked
provenance record. Reference payloads are excluded from ordinary packages.

| Exact parts | TI document | Relevant printed pages |
| --- | --- | --- |
| TUSB320/TUSB320I | [SLLSEN9F, March 2022](https://www.ti.com/lit/ds/sllsen9f/sllsen9f.pdf) | Pin table 4; recommended/absolute limits 5; electrical 6..7; PORT/dead battery 14 |
| TUSB320LI/TUSB320HI | [SLLSEP2D, May 2017](https://www.ti.com/lit/ds/sllsep2d/sllsep2d.pdf) | Pins 3..4; recommended limits 4; electrical 5..6; mode/startup 13..14 |
| TUSB320LAI/TUSB320HAI | [SLLSEQ8D, May 2017](https://www.ti.com/lit/ds/sllseq8d/sllseq8d.pdf) | Pin table 3; recommended/absolute limits 4; electrical 5..6; remote detection 11 |
| TUSB322I | [SLLSEO5D, March 2021](https://www.ti.com/lit/ds/sllseo5d/sllseo5d.pdf) | Pin table 3; limits 4; electrical 5..7; VBUS/VCONN 13; register 09 fault 18..19 |

The [variant guide](variant-matrix.md) records source conflicts, including
SLAAE24 startup wording, exact-part operating ceilings and V fault semantics.
Current source URLs and archived bytes have separate provenance; a later vendor
revision requires a new review before changing the established contract.

## Executable fixture plans and physical proof

The [fixture runner instructions](hil-runner.md#explicit-physical-fixture-suites)
define the strict setup JSON, expected firmware values, bounded ready/skip
transitions and exact CLI sequences. Before choosing a mutation suite, record
the actual board/full part, adapter/firmware, fixture/partner, ADDR and PORT
straps, enable polarity, measured VDD, pull-ups, INT and resistor-coupled VBUS_DET.
The operator owns power readiness and source/sink fixture policy; the runner
controls no supply or VBUS hardware. Startup readiness is explicit. Original
TUSB320/I source-capable startup also needs the separately completed qualified
recovery procedure after VDD return and identity, not a guessed register history.

Orientation needs the same identified reversible partner in both chip CC
orientations and externally checked connector mapping. No cable means NOT RUN.
Accessories need exact declared resistor/fixture types; base audio, charge-through
and debug are supported, while extended debug local DFP/UFP interpretations are
separate. A debug-local-UFP fixture uses local UFP mode. Unsupported capabilities
block before callbacks; unknown raw codes remain unknown and cannot PASS.

Role-current needs a controlled dual-role partner capable of the selected CC
categories, fixture-owned safe VBUS and sufficient supply for each local source
advertisement. The plan changes the same attached peer's advertisement and
compares the current indication with physical fixture evidence. Original
SLLSEN9F section7.3.1.2 p11 requires explicit software reset to update current;
the mutation plan records each reset plus identity/profile/port revalidation.
No such reset occurs in readonly/soak. Later variants' attached-current wording
is not resolved by a timestamp or passing model. All local source-capable 3 A
candidates, including cleanup, need VDD at least3.5V; a remote3A indication does
not establish that local requirement or any measured current/PD contract.

Interrupt needs a wired opt-in application reader checked before I2C/mutations,
active-low assertion and deassertion, a second physical event and multiple events
while the latch is pending. The plan records applied50/staged30 ACK preservation,
an unrelated duty write retaining the pending latch and one bounded service's
ACK/recheck. It cannot count every edge or infer fault clear from RCU. TUSB322I
fixture mutation plans remain BLOCKED before I2C due unresolved09/fault/advertisement
and return-to-DRP gates. Its required4.5..5.5V VDD, VBUS_DET and autonomous
VCONN225/300/375mA min/typ/max remain applicable; typical300mA is no guarantee
or software output permission.

Cleanup requires an explicit original-partner/supply/target attestation, fresh
matching firmware, coherent detached port and known profile. Successful plans
may restore the original actual profile via apply/verify. Changed partner,
target/supply, invalid data, failed or uncertain work prevents automatic device
restoration. An old source role must never be restored onto a different partner.
Record primary outcome, device cleanup, local cleanup and residual uncertainty
independently; readback cannot undo a USB event. Missing cleanup proof prevents
overall PASS. Timeout is FAIL, skip/absent fixture is NOT RUN, unsupported setup
is BLOCKED; reports preserve all of them.
Before any apply or staged-versus-applied IRQ experiment, the runner requires
consistent CONFIG raw/field acquisition masks and a validated complete staged
candidate. A successful setter status alone cannot prove the staged value.
Injected fixture replies are checked against their exact transition deadline
even when the overall run has time remaining. Native status-byte fixtures are
independent of plan expectations; deliberately wrong orientation/current
expectations fail without claiming physical evidence.

All deterministic fixtures and the persistent C++ plans exercise software
evidence only. Their modeled assertion/attachment and fabricated **test** identity
are labeled physical hardware NOT RUN. Actual serial/adapter timing, connector,
INT, CC/current, VBUS/VCONN and supply/strap qualification remains NOT RUN until
an authorized exact-part/board/firmware/fixture run saves physical observations.

## Reset, fault and console qualification

The runner now has nine explicit families; the three recovery plans and their
strict action declarations are described in [runner setup](hil-runner.md#reset-fault-and-console-setup).
Reset-termination uses the real typed reset/recovery operations. Record both
0A.disableTerm and45.disableRdRp where applicable, aggregate termination,
configuration trust and original result effects independently. Missing evidence
for either control cannot mean globally enabled. Capture actual pre-mutation
configuration, target, firmware, supply/startup assumptions and all outcome/cleanup
evidence. Cancellation and timeout cannot undo a write or a USB event.

Original320/I recovery needs a separately selected source-capable startup after
verified stable VDD and identity: qualified45=04, at least30ms,45=00 and readback
(SLLSEN9F7.4.3p14). Software uses the separate1ms margin; modeled31/96ms waits
do not qualify an electrical clock. A clear final45 alone proves no historical
sequence. A new power epoch invalidates earlier recovery readiness. Original-part
recovery experiments require a fresh coherent detached port before the first write.
Supply attestation alone does not establish current attachment. Base fault
power-cycle plans stop with blocked cleanup when source startup needs new recovery;
there is no automatic second recovery. A sink-only UFP fault plan can finish
without presenting Rp, while its saved base_recovery_required fact still blocks
later source-capable suites until newly qualified explicit recovery. This fact
survives session changes and cannot be replaced by the original setup declaration.
Original-part console tests require
continuous target VDD across the separately authorized MCU reboot.

Authorize NACK/disconnect/power-cycle and serial backpressure/reconnect/reboot
individually in the actual setup. The operator owns fixture transitions, supply,
VBUS and partner safety; the runner sends no power-control command. Avoid
back-powering unpowered parts through SDA/SCL/CC/enable networks (SLLSEN9F p14,
SLAAE24 p7). Retain resistor-coupled VBUS_DET sensing, including322 DFP sensing.
Its4.5..5.5V supply and225/300/375mA VCONN min/typ/max are not a guaranteed300mA
current budget. EN_N high-to-low or software reset establishes322 DRP operation
(SLLSEO5D6.3.2p9), not a complete POR register image. Its mutation-suite gates
remain closed; the native role-reset test is software evidence only.

Reopening a serial port can itself reset firmware. Close old local state, create
fresh framing and verify reported firmware and target before later work; old
tokens/success records cannot certify the new session. Native zero-output writes,
process restarts and modeled faults prove bounded handling only. Actual serial
delivery, reboot, loss visibility, adapter fault phase/aggregate bound, supply,
CC and physical reset/recovery timing all remain NOT RUN until separately measured.
Keep failed primary work, cleanup and residual uncertainty in the saved artifacts.
