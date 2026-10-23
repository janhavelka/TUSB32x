# Public implementation audit

Reviewed 2026-10-23. This is source research for the future library, not a claim
that any implementation was built, run or qualified on this project's hardware.
No third-party production code was copied into the repository.

Public code supplies useful examples and failure cases. It does not establish
silicon behavior. Recheck each adopted value against the exact TI variant,
datasheet revision, table and access semantics in [the TI audit](ti-family.md).
When code and TI disagree, record both and resolve the conflict; do not silently
copy a constant, rename an uncertain observation, or create a test that merely
agrees with the downloaded implementation.

## Inspected sources and ownership

The commit URLs below are immutable snapshots, not a claim that those commits
are the latest release. The linked file content and relevant licensing files
were read through the GitHub connector. The five primary source files were then
downloaded as exact pinned raw bytes to a task-specific temporary directory
outside this repository. Each computed Git blob SHA matched the connector's
reported blob. [Public implementation provenance](../reference/public-implementations.json)
records the byte counts, SHA-256 hashes, completed-write UTC timestamps and
source ownership. Source code itself is not archived in this repository.

| ID | Repository and pinned source | Git blob SHA | License evidence |
| --- | --- | --- | --- |
| C1 | [usbarmory/armoryctl, `tusb320/tusb320.go`, `4af21c7`](https://github.com/usbarmory/armoryctl/blob/4af21c7459faaaf50f6d0adde2e5ec4b0cc0952b/tusb320/tusb320.go) | `b29faa9fe6a57d7af371f9704b6801fd13d0b92c` | [Root LICENSE](https://github.com/usbarmory/armoryctl/blob/4af21c7459faaaf50f6d0adde2e5ec4b0cc0952b/LICENSE): BSD 3-clause |
| C2 | [betrusted-io/betrusted-ec, `sw/betrusted-hal/src/api_tusb320.rs`, `025d67d`](https://github.com/betrusted-io/betrusted-ec/blob/025d67d78d894cafa39f58dc2e62e0d57640301b/sw/betrusted-hal/src/api_tusb320.rs) | `3f3f43bc69ea9476769898ccde21175ddc68ba7c` | [Root LICENSE](https://github.com/betrusted-io/betrusted-ec/blob/025d67d78d894cafa39f58dc2e62e0d57640301b/LICENSE) and README identify CERN-OHL-1.2; this Rust file has no license header and its Cargo manifest has no license field. No separate firmware reuse grant was established. |
| C3 | [apfaudio/tiliqua, `gateware/src/rs/hal/src/tusb322.rs`, `ed79182`](https://github.com/apfaudio/tiliqua/blob/ed791829c73bfedddba511f1a88f2bec38788036/gateware/src/rs/hal/src/tusb322.rs) | `3386bb4ffcd138f286e3ef0f3be5e1f51fb88ca6` | [README](https://github.com/apfaudio/tiliqua/blob/ed791829c73bfedddba511f1a88f2bec38788036/README.md) identifies root CERN-OHL-S-2.0 and some BSD 3-clause files. This file has no license marker; its Cargo manifest has no license field. Do not assume a permissive file license. |
| C4 | [betrusted-io/xous-core, `emulation/peripherals/TUSB320LAI.cs`, `53d7724`](https://github.com/betrusted-io/xous-core/blob/53d772438a9aeea88972b59b9707a88142b63548/emulation/peripherals/TUSB320LAI.cs) | `3e2d4b79c499156d813b5ad8d31d771cdeb1c5e1` | File header: MIT, Antmicro |
| C5 | [mobiaqua/yocto, `meta-ti/recipes-kernel/linux/linux-ti64_6.12/0014-WIP-tusb322.patch`, `62073e0`](https://github.com/mobiaqua/yocto/blob/62073e0493961867d42d4d9fcc0003700308c640/meta-ti/recipes-kernel/linux/linux-ti64_6.12/0014-WIP-tusb322.patch) | `c81eb1b20185cd42c1b37da82479d70613381708` | Introduced driver header: GPL-2.0 SPDX. The patch explicitly identifies itself as WIP. A `meta-ti` directory does not make this a TI-maintained repository. |

The audit also inspected [upstream Linux at `1bf0ba4`](https://github.com/torvalds/linux/blob/1bf0ba46d9d2c784120fd9cb235c08add3a6e7be/drivers/extcon/extcon-usbc-tusb320.c)
and [OnePlus vendor Android code at `9323887`](https://github.com/OnePlusOSS/android_kernel_oneplus_msm8996/blob/93238877f9e8fbbbf801f2deeebc18f54d4bdc1b/drivers/misc/type-c-tusb320.c).
Their headers identify GPL-2.0 and GPL-2.0-or-later respectively. The Linux code
is a useful implementation reference; the older Android policy is a cautionary
example. Neither is a framework-neutral core to transplant.

## Facts corroborated by implementation code

These are independent code observations, with TI confirmation still required
before implementation. Values are unshifted field encodings unless marked as
register masks or bytes. Never mix a 7-bit device address with its bus address
byte, a register address, or an already shifted field value.

| Evidence | Concrete observation | Applicable conclusion |
| --- | --- | --- |
| C1 | Default application bus address `0x61`; read eight bytes starting at `0x00`, reverse for display | A base TUSB320 application uses the ADDR-high 7-bit address. It says nothing about LAI addresses or board strap discovery. |
| C2, C4 | Ascending TUSB320 ID bytes `30 32 33 42 53 55 54 00` | Compare all eight raw bytes in address order. C1 reads/reverses eight bytes but does not define the expected signature. A reversed display is not a different wire identity. |
| C2 | LAI address `0x47`; revision register `0xA0`; expected revision `0x02` | Corroborates LAI ADDR-low selection and published revision, not automatic variant detection. |
| C2 | `0x08` active cable mask `0x01`, accessory mask `0x0E`, detected current bits `[5:4]`, advertisement bits `[7:6]` | Keep advertisement and detection separate in types and outputs. |
| C2, C3 | Accessory codes `0,4,5,6,7`: none, audio, charge-through, debug DFP, debug UFP | Code 7 is a later-variant field interpretation; it must not be enabled on the original TUSB320/I by inheritance. |
| C2, C3 | Detected current codes `0,1,2,3`: default, medium, charge-through 500 mA, high | The special code 2 must remain distinguishable from ordinary default current. |
| C2 | `0x09` attached bytes `0x00/0x40/0x80/0xC0`; CC2 mask `0x20`; interrupt mask `0x10`; UFP-accessory disable `0x01` | Source/sink describe the local controller state. Attachment and orientation validity remain separate. |
| C2 | DRP source duty field bytes `0x00/0x02/0x04/0x06` mean 30/40/50/60% | These percentages are not a guarantee of fixed toggle durations or a software polling period. |
| C2 | LAI debounce bytes `0x00/0x40/0x80/0xC0` mean nominal 168/118/134/152 ms | These nominal hardware settings are not minimum software waits. Base variants use a different table. |
| C2 | `0x0A` role bytes `0x00/0x10/0x20/0x30`; disable-term `0x01`; reset `0x08`; Try.SNK `0x02` | Read the complete role-change procedure; a matching final role byte does not prove the sequence completed. |
| C2 | Separate register `0x45`, disable-Rd/Rp mask `0x04` | Do not conflate this control with `0x0A:0`; observe and preserve internal bits as required by TI. |
| C3 | TUSB322I 7-bit address `0x47`; register pointer is one byte; write frame is register then data | The application-specific address is not the entire address capability set. The public transport remains explicit and application-owned. |
| C5 | TUSB322 ID is ascending `32 32 33 42 53 55 54 00` | A base-family `30` in register zero is not a valid TUSB322 identity. C5 checks the reversed sequence one byte at a time; C3 reads eight bytes but has no expected-signature literal. |
| C3 | `0x09:3` decoded as VCONN fault; duty `[2:1]`; accessory disable `[0]` | Fault presence is corroborated; this code does not settle TI's ambiguous clear semantics. |
| C3 | TUSB322 mode codes `0/1/2` for DRP/UFP/DFP; disable-term, write mode, enable-term | Unlike TUSB320, code zero is DRP, not PORT-pin selection. TI SLLSEO5D §6.3.2.1–2 pp10–11 explicitly prescribes this sequence for DFP/UFP without a specified 5 ms delay. Qualify direct return to DRP separately. |

The TI documents used to resolve these fields are [SLLSEQ8D Tables 7-12](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf),
[SLLSEN9F Table 7-7](https://www.ti.com/lit/ds/symlink/tusb320.pdf), and
[SLLSEO5D Tables 6-6 through 6-11](https://www.ti.com/lit/ds/symlink/tusb322i.pdf).
The archived LAI printed pages 17-20 were re-read for this audit, including the
Try.SRC, default current, revision and interrupt-clear conflicts below.

## Concrete conflicts and unsafe patterns

### C1: Go monitor comments do not match the field it reads

`GetCurrentMode()` reads `0x08`, shifts by four and masks with three. That is
CURRENT_MODE_DETECT. Its comment calls the field CURRENT_MODE_ADVERTISE, which
is `[7:6]`. The implementation's display map also labels code zero as a fixed
0.5 A. TI specifies default USB current, including 500/900 mA depending on the
applicable USB context. Preserve the baseline `DetectedCurrent::DEFAULT`; the core does not infer USB
enumeration state or a guaranteed load allowance.

Use independent register vector `0x90`: advertisement code 2 means 3 A, while
detection code 1 means medium/1.5 A. A single `current` field cannot represent both.
Vector `0x20` has default advertisement and charge-through detection; it must not
collapse into the same interpretation as `0x00`.

### C2: LAI constants contain a reserved Try.SRC encoding

At the pinned source's line 58, `SOURCE_PREF_DRP_TRY_SRC` is the shifted byte
`0x04`. TI SLLSEQ8D Table 10 says field `[2:1]=10` is reserved and `11` is Try.SRC,
so the correct shifted Try.SRC value is `0x06`. This is a confirmed disagreement,
not a reason to create a compatibility alias. Golden tests must assert field
codes 0/1/3 are accepted and 2 is rejected before any callback; raw diagnostics
must still report the observed reserved code honestly.

The source also declares alternative revision `0x06`, but the assertion allowing
`0x02` or `0x06` is commented out. It provides no silicon revision provenance,
readback trace or TI source establishing `0x06`. The datasheet documents `0x02`.
Record `0x06` as an unresolved public-code clue and retain unexpected raw
revisions for diagnosis. Do not silently whitelist it or claim it proves LAI.

The initializer writes `0x0A=0x12` directly, without the documented LAI
disable-term/change-mode/wait/re-enable sequence. Its repeated I2C calls loop
until success with a per-call 1 ms timeout, so the total duration is unbounded.
Neither the direct role write nor the 1 ms policy is a silicon requirement.
The library must return transport failures and let its serialized owner choose
any retry; an individual backend timeout cannot bound an infinite outer loop.

IRQ clearing writes fixed `0x09=0x11`. That acknowledges bit 4 but also forces
UFP-accessory disable and 30% duty. For a current control value of `0x06`, the
intended ACK byte is `0x16`, not `0x11`; the latter silently changes two settings.
Read or use proved current controls and compose the W1C write deliberately.

### C3: useful TUSB322 decoding, incomplete general control contract

This small Rust driver separates current advertisement, detected current,
accessory, attachment and direction, which is useful domain decomposition.
Its `embedded-hal` I2C ownership is specific to Rust; borrow application
callbacks according to this project's C++ contract instead of adopting a second
transport abstraction.

`set_mode()` writes whole `0x0A` bytes: `0x01`, then disabled new mode, then
enabled new mode. Those writes also zero unrelated debounce/preference bits.
For an observed `0x0A=0xC2` and a switch to DFP, preserving the documented
debounce/preference controls gives `0xC3 -> 0xE3 -> 0xE2`.
Correction, 2026-10-23: the earlier audit called this merely conditional
arithmetic. SLLSEO5D §6.3.2.1–2 pp10–11 explicitly prescribes the three separate
disable/mode/enable writes for DFP/UFP. Unlike the extended 320 procedure, it
specifies no added 5 ms wait. The source instead writes
`0x01 -> 0x21 -> 0x20`. A generic implementation must preserve valid current
controls and keep partial effects when any write fails.

`soft_reset()` emits `0x0A=0x08` and immediately returns the transfer result.
That demonstrates command submission, not reset completion, fresh identity,
attachment stability or successful reconstruction of configuration trust.
Use the datasheet's reset timing and the owner's completion anchor. Keep any
software clock margin distinct from the hardware specification.

The driver combines a pointer write and read through `I2c::transaction`.
That is evidence of this implementation's framing, not proof that every family
member requires or tolerates every repeated-START implementation. Follow the
exact TI programming description, including STOP-separated pointer selection
where documented, and qualify the chosen adapter with actual bus evidence.

Its raw orientation and detected-current fields are returned even when not
attached. The general library must expose their interpretation/validity rather
than inventing a live CC1 connection or a usable current allowance.

### C4: an emulator is not independent silicon evidence

This Renode mock returns the known eight signature bytes, but treats a two-byte
write's second byte as the register pointer. Reads fill only the first byte for
the ID cases and advance its pointer once per read call. It has no faithful
register map, W1C behavior, role timing, reset model or fault behavior. Useful
signature literals do not make the model a valid protocol oracle.

Build the test fake from the TI-derived specification. Test the fake itself
against independently recorded examples and failure boundaries, and keep host
simulation separate from a physical-device result.

### C5 and older Android code: limited evidence, application policy

C5 corroborates the TUSB322 signature, `0x09` attachment/orientation masks and
8-bit register/value widths. It requires a pending interrupt before updating
state even during initial probing, ignores the ACK write result, and does not
implement a complete TUSB322 register/control interface. Its existence does not
settle advertisement-write or VCONN-fault-clear ambiguity.

The OnePlus vendor driver uses whole-byte `0x09=0x10/0x16` ACK/duty writes and a
software Try.SNK policy involving two reset writes separated by 25 ms. It also
owns wake locks, USB power notifications and global retry-attempt state. Those
are application-specific procedures, not evidence for a general reset-complete
wait or permission to implement automatic reset/recovery in this core.

The upstream Linux source has useful identity and 5/95 ms timing references,
but its reset call supplies mask `0x08` with value `1`. The same pinned
[regmap helper](https://github.com/torvalds/linux/blob/1bf0ba46d9d2c784120fd9cb235c08add3a6e7be/drivers/base/regmap/regmap.c#L3250)
inserts `value & mask`, so that call does not set bit 3. Test the actual composed
byte, not merely the fact that a reset API was called. Its mode-change cleanup
can replace the primary write error with a successful cleanup result, and its
IRQ ACK result is ignored. Preserve both operation outcome and possible device
effects in this library. Do not turn these source-review findings into claims
of observed failures on physical Linux systems.

## Required implementation and review checks

1. Re-fetch the relevant pinned source and inspect changes before adopting a
   new claim. Record exact variant, TI location, code location and resolution.
2. Independently assert all eight identity bytes and address selection. A
   one-byte match or an ACK is insufficient; a reversed display is not raw data.
3. Test `0x08=0x90` and `0x20` for separate advertisement/detection behavior.
4. Reject source-preference code 2 and compose Try.SRC as `0x06`, not `0x04`.
5. Report unexpected revision `0x06` without silently extending supported
   identity rules. Obtain TI evidence before changing the acceptance policy.
6. Verify ACK composition preserves current duty/accessory controls; include
   the `0x06 -> 0x16` example and an IRQ that changes during service.
7. Prove a reset request sets byte mask `0x08`. Transfer success alone must not
   mark reset complete, and a later read must not erase uncertain effects.
8. Fail each role-change write in turn. Preserve the first transport failure,
   partial state, termination uncertainty and immutable operation deadline.
9. Check preservation of non-target `0x0A` fields: `0xC3/0xE3/0xE2` follows
   the documented 322 DFP sequence. Extended 320 uses the same bytes with its
   separate required wait; the 322 source does not add that wait. DRP encodings
   and reset-to-DRP do not by themselves specify a direct non-reset return path.
10. Bound every callback and the entire operation; do not copy infinite retries,
    busy waits or immediate post-reset access from public implementations.
11. Derive device mock semantics from TI, and test protocol frames independently
    of the mock's implementation. Keep unverified framing visibly unqualified.
12. Keep VCONN-fault clear and TUSB322 advertisement-write semantics gated until
    official evidence resolves them; public readers do not resolve write access.

## Search limits

Search covered actual GitHub code, not only package descriptions: upstream
Linux, U-Boot, Zephyr, Espressif repositories, USB armory, Betrusted, Tiliqua,
vendor Android trees and the named WIP patch. U-Boot matches were imported
bindings and device trees; no native TUSB320 I2C driver was established. Targeted
Zephyr code search returned no TUSB320 result. Espressif results were a
[hardware guide](https://github.com/espressif/esp-iot-solution/blob/4962410c80a51cd5822da1d8d2041eb4285265f1/docs/en/usb/usb_overview/usb_typec_hardware_guide.rst),
not a verified TUSB320 driver. Arduino-oriented searches did not establish a
substantive maintained library during this audit. These are scoped search
results, not proof that no such implementation exists.

The GitHub connector briefly rate-limited one Linux contents request; its
approved raw-file fetch succeeded. No source content was guessed from an
unavailable page. Public-code review does not replace reading the vendor
datasheets or collecting real target/adapter traces during implementation.
