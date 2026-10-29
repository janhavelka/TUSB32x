# Board design checklist

Prepared 2026-10-29 before a schematic or physical fixture exists. This is a
design-review worksheet, not a qualified reference circuit. Start with
TUSB320LAI as the library's primary target; select a different exact part only
after reviewing its separate requirements in the [variant matrix](variant-matrix.md).
The intended product role and power architecture are still undecided.

## Record the design choices

| Decision | Fill in before schematic approval |
| --- | --- |
| Exact orderable part and package | Part, temperature grade, footprint drawing/revision |
| Intended roles | Sink/UFP, source/DFP or DRP; allowed startup role |
| Power architecture | Device VDD source/tolerance/ramp, MCU rail, VBUS source/sink switch and protection ownership |
| Power-off cases | Cable present/absent, MCU off, controller off, independent pull-up rails |
| Interface | MCU/board, SDA/SCL pins, speed, shared-bus users and serialization |
| Straps | ADDR level/seven-bit address; PORT level on 320 parts |
| Enable | Exact polarity, passive or driven wiring, power-up sequence and evidence |
| Optional observations | INT_N, ID, and DIR only on 322I; pull-up rail and MCU pins |
| Qualification record | Schematic/BOM/layout revision, later assembled part markings and firmware identity |

## Schematic checks

- [ ] Keep USB power/data policy outside this controller library. Specify the
  external source switch, sink path, reverse-current protection and fault policy
  appropriate to the product. CC current indication is not a PD contract or a
  measured load. No software default authorizes sourcing power.
- [ ] For LAI/HAI keep VDD within 2.7–5.0 V including tolerance/transients. Local
  3 A advertisement needs at least 3.5 V; remote 3 A detection as a sink is a
  different function. A nominal 3.3 V design must not advertise local 3 A.
- [ ] Size local decoupling from the exact datasheet/application circuit and
  effective capacitance at bias/temperature. Keep the bypass loop short. Do not
  copy a universal capacitor pair or treat nominal capacitance as qualification.
- [ ] Pull SDA/SCL to a qualified 1.65–3.6 V rail compatible with the MCU. For
  3.3 V I2C maintain device VDD above 3.0 V during use. Choose resistors from bus
  capacitance, rise time and sink current; strap resistor examples are not I2C
  pull-up specifications. Maximum SCL is 400 kHz; initially plan 100 kHz.
- [ ] Strap ADDR low or high to device VDD: LAI uses 0x47 or 0x67. Floating ADDR
  selects GPIO mode, not I2C. These are seven-bit addresses, not 0x8E/0x8F wire bytes.
- [ ] For the six 320 parts, PORT low selects UFP, floating DRP, high DFP at
  startup/reset sampling. Decide this electrically; firmware initialization is
  read-only and will not repair an inappropriate strap.
- [ ] LAI EN_N is active low. Resolve or avoid reliance on the conflicting
  externally driven 50 ms startup wording using exact-part TI guidance; do not
  invent a universal enable pulse. Record VDD ramp and enable readiness separately
  from attachment readiness. See [TI questions](ti-clarification.md).
- [ ] Connect CC1/CC2 with documented connector routing. Review connector-side
  protection and signal loading against the exact part ratings. Record any swapped
  mapping; the core reports chip CC1/CC2 without guessing connector orientation.
- [ ] Provide the specified VBUS_DET sensing network. The external resistance is
  855–920 kOhm with 887 kOhm typical (900 kOhm in pin prose), not a direct VBUS
  connection. Check resistor tolerance, voltage and power at the intended VBUS
  extremes. The 4 V pad absolute maximum is distinct from system VBUS voltage.
- [ ] Do not use the six-part DFP-only VBUS_DET omission if UFP/DRP is possible.
  Prefer retaining sensing provisions for a flexible bring-up board.
- [ ] Check every pin under all power-off combinations. Follow the variant's
  non-failsafe list; do not back-power through straps or independent pull-ups.
  Do not rely on disputed ID supply-off tolerance.
- [ ] INT_N is optional open-drain, active low in I2C mode. Select a compatible
  pull-up and route a test point even if initial software uses polling. ISR work
  only signals an owner; it performs no I2C.
- [ ] Provide accessible ground, VDD, MCU/I2C pull-up rail, SDA, SCL, EN, INT_N,
  CC1, CC2, VBUS and VBUS_DET probe points. Account for probe loading, especially
  at the high-impedance VBUS_DET node. Expose strap choices and allow supply-current
  measurement without accidentally powering a second source.
- [ ] Check footprint, pin numbering, exposed-pad/land requirements if applicable,
  connector orientation and short return paths against the exact package drawing.

The numerical limits above are indexed to SLLSEQ8D §§5–6 pp3–6, §7.2 pp9–12,
§7.3 pp13–14 and initialization p28 in the
[electrical source table](variant-matrix.md#electrical-and-dead-battery-boundary).
Use that table and [hardware validation](hardware-validation.md) during schematic
review rather than treating this worksheet as a replacement datasheet.

## Changes required for other parts

| Part | Design difference to resolve |
| --- | --- |
| TUSB320 / TUSB320I | 0x60/0x61; source-capable startup after VDD loss can require explicit base recovery after identity validation. Never copy that recovery into generic setup. |
| TUSB320LI / TUSB320HI | Review TI's recorded recommendation against new DFP/DRP use and their separate electrical limits. |
| TUSB320HI / TUSB320HAI | Active-high EN; high operating range 1.05–3.65 V and absolute maximum 4 V. Do not tie EN to a 5 V VDD. |
| TUSB322I | 4.5–5.5 V device supply; VCONN operation has separate 5 V supply/bulk requirements. Pin3 is VBUS_DET, pin4 DIR, no PORT. No established DFP-only VBUS_DET omission. Advertisement, all 09 writes/fault handling and direct non-reset DRP return remain restricted. |

Do not make 320 and 322 footprints interchangeable by name alone. The
[bring-up plan](bring-up-plan.md) defines what the future assembled board must prove.
