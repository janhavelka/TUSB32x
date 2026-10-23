# Public implementation audit: TI-hosted sources

Audit date: 2026-10-23. This is source inspection, not a build, hardware test or
qualification of this future library. The source files were read through the
GitHub connector at a fixed commit, then their raw bytes were independently
downloaded and hashed. Downloaded source remains outside this repository; this
report contains findings and provenance, not copied driver implementation.

The checked source-byte metadata is retained in
[public-implementations.json](../reference/public-implementations.json).

## Search scope and ownership

The preferred source is the official `TexasInstruments` GitHub organization.
Organization-wide searches for `tusb320` and `TUSB32` found the Linux extcon
driver, its binding and board descriptions. `TUSB320LAI` and `tusb322` returned no
matches in that index. Targeted `tusb320` searches in `mcupsdk-core`, `mspm0-sdk`
and `simplelink-lowpower-f2-sdk` returned no matches. The secondary
`TexasInstruments-Sandbox` organization contains another Linux copy in
`wireless-next`. These searches do not prove that no other TI implementation
exists, and duplicated Linux files are not independent silicon evidence.

The inspected repository is
[TexasInstruments/ti-linux-kernel](https://github.com/TexasInstruments/ti-linux-kernel),
default branch `ti-linux-6.18.y`, pinned at
[`97becb13b81c314c1d17fcf2226aac9a32086f3d`](https://github.com/TexasInstruments/ti-linux-kernel/commit/97becb13b81c314c1d17fcf2226aac9a32086f3d).
The connector reports this commit's creation time as 2026-10-22T22:36:55Z.
The matching `git.ti.com/cgit/ti-linux-kernel/ti-linux-kernel/` page was not
accessible through the web reader during this audit. GitHub provided the full
files. No conclusion depends on an unread cgit page.

TI owns the repository, but the TUSB320 driver's header credits **National
Instruments Corporation**, 2020, and author Michael Auchter. National Instruments
and Texas Instruments are different companies. Call this a TI-hosted Linux
driver, not a TI-authored reference driver or a vendor guarantee. Its SPDX tag is
`GPL-2.0` and its module license is GPL v2. The device-tree binding is
`GPL-2.0-only OR BSD-2-Clause`. Inspect licenses before any reuse; this audit
extracts facts and defects and does not authorize copying implementation.

## Pinned source inventory

All paths below are relative to the pinned TI Linux repository. Raw file writes
completed on 2026-10-23 between 11:58:22 and 11:58:24 UTC after retrieval.
Sizes and SHA-256 values cover exact raw
file bytes; Git blob SHA-1 values also matched the connector responses.

| Source | Pinned path and important location | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| L1 driver | [drivers/extcon/extcon-usbc-tusb320.c](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/drivers/extcon/extcon-usbc-tusb320.c): constants line 22, identity 101, role changes 120/140, reset 174, revision 201, current 215, port decode 279, IRQ 374, probe 513, compatible table 595 | 16489 | `017faab2f72df13ca7b4796c60145b85e0f28913604292f108613e4401614a57` |
| L2 binding | [Documentation/devicetree/bindings/extcon/extcon-usbc-tusb320.yaml](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/Documentation/devicetree/bindings/extcon/extcon-usbc-tusb320.yaml) | 778 | `8197c89011ea87fcc1822e3b250d082a780fe647604da6f4cd98c63ffde06bf1` |
| L3 mask API | [include/linux/regmap.h](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/include/linux/regmap.h): regmap_write_bits line 1340 | 74869 | `0444ee165f9bfb7cbb2a0abafa982fb415ddf544bb00a9c7bdf70f562fd8d2ef` |
| L4 mask implementation | [drivers/base/regmap/regmap.c](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/drivers/base/regmap/regmap.c): update at line 3250, mask arithmetic at 3270–3271 | 88105 | `5ad5df5cfbd6bccee21c47f3acebf41f59428da5a49a3a3d1b103eb29510ee28` |
| L5 transport | [drivers/base/regmap/regmap-i2c.c](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/drivers/base/regmap/regmap-i2c.c): combined read line 172, backend selection 306 | 9575 | `6900f436ba20cd39d8413a25e86bb4d925d09aebe6647af9b998883d0a90f4b1` |
| L6 extended board | [arch/arm64/boot/dts/qcom/msm8996-xiaomi-common.dtsi](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/arch/arm64/boot/dts/qcom/msm8996-xiaomi-common.dtsi): typec node line 209 | 16138 | `1803341eaccbc58e7e9bd5306bb274815826d5ab8c598a932876180d554bb8f1` |
| L7 base board | [arch/arm/boot/dts/allwinner/sun8i-v3s-netcube-kumquat.dts](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/arch/arm/boot/dts/allwinner/sun8i-v3s-netcube-kumquat.dts): typec node line 120 | 5794 | `48424d412ac74237639b70e377711900ce2dd7eeb77746b43904a898360f7e68` |
| L8 TI board | [arch/arm64/boot/dts/ti/k3-j721e-sk.dts](https://github.com/TexasInstruments/ti-linux-kernel/blob/97becb13b81c314c1d17fcf2226aac9a32086f3d/arch/arm64/boot/dts/ti/k3-j721e-sk.dts): GPIO direction debounce lines 961–962 | 30539 | `3e17731c7b2eef5c2fcdeb87b5410fea4e88cdeec6512b0cc667511f3bc92304` |

L3–L5 credit Wolfson Microelectronics and Mark Brown and carry GPL tags. L6 is
BSD-3-Clause, credits Yassine Oudjana; L7 is `GPL-2.0+ OR MIT`, credits Lukas
Schmid; L8 is `GPL-2.0-only OR MIT`, credits Texas Instruments. Repository owner,
file author, license and electrical authority remain separate attributes.

## Useful facts, checked against the TI documents

The following extracted facts agree with the archived
[base datasheet](../reference/source/TI-TUSB320-I-SLLSEN9F.pdf),
[LI/HI datasheet](../reference/source/TI-TUSB320LI-HI-SLLSEP2D.pdf) and
[LAI/HAI datasheet](../reference/source/TI-TUSB320LAI-HAI-SLLSEQ8D.pdf), subject to
the variant limits in the last column. Base Table 7-7 pp16–19 and extended
Tables 7–12 pp16–20 are the register authorities. PDF pages 17/19 relevant to
the accessory/reset findings were also visually inspected during this audit.

| Extracted L1 fact | Concrete value | Decision and limit |
| --- | --- | --- |
| Register widths | 8-bit register pointer; 8-bit value | Adopt; L1 regmap config is consistent with TI programming sections |
| Signature | Driver compares registers 7 down to 0 with NUL then TUSB320; ascending bytes are `30 32 33 42 53 55 54 00` | Adopt bytes, not its reversed loop as an API requirement; a shared signature cannot identify the exact 320 variant |
| Advertisement | 0x08 mask 0xC0, codes 0/1/2 = default/1.5 A/3 A | Adopt for the six verified 320 parts; code 3 reserved; no 322I write permission follows from this source |
| Detected current | 0x08 mask 0x30, codes 0/1/2/3 = default/medium/charge-through/high | Adopt raw decoding; context determines whether usable. Code 2 has its own 500 mA accessory meaning |
| Accessory | 0x08 mask 0x0E; 0 none, 4 audio, 5 charge-through, 6/7 debug DFP/UFP | Adopt extended meanings only. Base 320/I code 6 is generic debug and code 7 is reserved |
| Active cable | 0x08 bit 0, mask 0x01 | Preserve raw evidence; do not infer an external VCONN switch or policy |
| Attachment | 0x09 mask 0xC0; 0 detached, 1 local DFP/source, 2 local UFP/sink, 3 accessory | Adopt local-role direction. Peer role is the complementary observation when applicable |
| Orientation | 0x09 bit 5, mask 0x20 | 0 means CC1, 1 means CC2. Detached data does not establish a usable orientation |
| IRQ status | 0x09 bit 4, mask 0x10 | TI establishes W1C and generation-specific sources; separate read, publication and acknowledgment |
| Mode | 0x0A mask 0x30; unshifted 0/1/2/3 = PORT/UFP/DFP/DRP | Adopt only for 320 family; 322I mode 00 is DRP and has no PORT pin |
| Disable term | Extended 0x0A bit 0, mask 0x01 | Adopt separate from 0x45 bit 2; neither alone proves both controls allow terminations |
| Soft reset | 0x0A bit 3, mask 0x08 | Adopt bit position, reject L1 call-site value described below |
| Revision | Read 0xA0 only for the driver's extended compatible | Supports capability gating; TI supplies expected 0x01 LI/HI and 0x02 LAI/HAI. L1 merely logs the value |
| Base mode change | L1 refuses while its cached attachment is nonzero | TI requires unattached state. Cached state is weaker than fresh evidence and cannot prevent a cable race |
| Extended mode change | Disable term, write mode, sleep 5 ms, re-enable | Corroborates TI's sequence; retain cooperative owner scheduling and the existing separate 1 ms clock margin |
| Reset delay | L1 sleeps 95 ms after its reset call | Matches TI's maximum reset interval, not a readiness or configuration guarantee; the reset call itself is defective |

The driver has no full family capability model, 0x45 handling, base dead-battery
recovery, DRP-duty API, debounce API, source-preference API or UFP-accessory
control API. Their absence in this driver does not remove TI requirements.
Its two compatibles `ti,tusb320` and `ti,tusb320l` are Linux match keys, not
proof that LI, HI, LAI and HAI are interchangeable.

## Reject these copied behaviors; turn them into regression cases

1. **Wrong reset value.** L1 `tusb320_reset` passes mask `0x08` and value `1` to
   `regmap_write_bits`. L3 passes that value unchanged to the update function;
   L4 computes `(old & ~mask) | (value & mask)`. Since `1 & 0x08 == 0`, the call
   clears bit 3 rather than asserting it. TI Table 7-7 p19 / Table 10 p19 requires
   a one in bit 3. This is a source-level defect confirmed by API arithmetic,
   not a measured hardware result. Test the actual outgoing byte: reset emits
   the 0x08 bit and keeps 0x01 independent; a normal setting write emits no reset.
2. **Primary error lost during cleanup.** In `tusb320l_set_mode`, a failed mode
   write jumps to re-enable cleanup; that call overwrites the original error.
   Successful cleanup can therefore make the function return success after a
   failed mode change. Keep the first failure and explicit uncertain effects.
   The planned library performs no hidden recovery write. Inject failure before
   and after each possible hardware effect and prove the result stays failed.
3. **IRQ acknowledgment result ignored.** `tusb320_state_update_handler` ignores
   the final `regmap_write` status and reports handled. Preserve an already
   complete port observation, but return ACK failure and retain IRQ uncertainty.
4. **Incomplete observation can still be acknowledged.** The Type-C helper can
   fail its 0x08 read and return void; its caller still acknowledges 0x09. Test
   failure of any required read or a partial burst. If using Linux's read order,
   include successful 0x09 followed by failed required 0x08. Do not publish invented
   current/accessory fields or silently report full service success.
5. **Raw register echo is not a general write composer.** L1 echoes the observed
   0x09 byte. This retains observed persistent settings but also echoes live and
   reserved bits and has no 322I fault-field policy. Deliberately write only the
   approved persistent image plus explicit 0x10 W1C. For extended actual 0xB5,
   expected ACK is 0x15; for base actual 0xB4, expected ACK is 0x14. Unapplied
   desired configuration must not leak into either ACK.
6. **Linux fallback roles are not observations.** Detached/unknown states are
   projected to sink/device defaults; orientation is published even detached;
   accessory roles are described in the source as guesses. Preserve raw values
   and use unknown/not-applicable in this library rather than these defaults.
7. **VCONN role is inferred from power role.** L1 sets the Type-C framework VCONN
   role equal to its chosen power role. This does not establish that TUSB320 has
   an integrated VCONN switch or that external VCONN is on. Keep device evidence
   and board-controlled power policy separate; handle 322I only from its sources.
8. **Generic accessory decoding crosses generations.** L1 exposes code 7 for
   all matches. Base 320/I reserves it. Test base 0x0E as reserved and extended
   0x0E as debug/local UFP; base 0x0C remains generic debug, not proved local DFP.
9. **Probe has effects.** L1 configures advertised current, updates/acknowledges
   status, requests a reset and updates again during probe. This library's init
   remains read-only; profile apply, reset and IRQ ACK remain explicit operations.
10. **A sleep does not prove its preceding write worked.** A 95 ms sleep after
    the incorrect reset call is not proof of reset. Assert write bytes and later
    evidence independently. Do not inherit undocumented retry or settling delays.

## Board and transport findings are not silicon rules

- L2's example uses base address 0x61; L7 uses 0x60; L6 uses extended address
  0x47. Exact TI address tables establish the other extended strap address 0x67.
  The driver takes an application-provided I2C client and performs no address
  scan. Keep seven-bit addresses separate from on-wire read/write address bytes.
- L7 requests a level-low IRQ, L6 a rising-edge IRQ, and L1 starts from a
  falling-edge default but uses the configured IRQ trigger when available.
  These are different board/kernel configurations. They cannot redefine the
  chip's active-low open-drain INT_N. The application chooses GPIO trigger and
  deferred service; the core must not hard-code a board's IRQ number or polarity.
- L8 is a TI-authored board description, but its 700 ms GPIO direction debounce
  belongs to a TUSB321/SerDes board integration. It is not a TUSB320 register
  debounce encoding, role-change wait or reset delay. The comment's 133 ms does
  not replace exact per-variant debounce tables.
- L5's native-I2C backend submits pointer write and data read as two messages
  in one transfer; backend selection can instead use SMBus helpers. This is
  implementation evidence for Linux framing, not proof that repeated START is
  required by every variant. TI programming prose describes STOP after pointer
  selection. Record the selected adapter transaction and verify it on hardware;
  do not silently turn either transport pattern into an unsupported universal
  mandate. One logical callback can contain multiple physical bus phases.
- Linux heap allocation, threaded IRQs, regmap, extcon, USB role switches and
  blocking sleeps solve Linux integration problems. They are not appropriate
  dependencies or ownership rules for the framework-neutral C++17 core.

## Required use by the implementation prompts

Prompts 2–4 must re-open the pinned files and the exact current TI documents,
classify facts as datasheet-verified, implementation-only, conflicting or
unresolved, and record any corrections. Search results and function names are
leads; review actual expressions, called API semantics, error paths and emitted
bytes. A familiar driver, many copies, a TI-owned repository or a passing fake
test cannot override the exact part's datasheet. Never fill gaps with guessed
registers, flags, timings, reset images or measured-support claims.

Prompts 9–10 should use the identity, masks and reset-value counterexample;
13–16 should use the sequencing, error, validity and IRQ counterexamples;
18–19 should require independent byte/failure vectors; 23–26 and 30–31 should
retain board/framing and real-fixture distinctions. Recheck these findings in
the final reviews, updating affected code, tests, Doxygen and guides together.
Leave only the dependent operation gated when evidence stays unresolved. The
existing 322I advertisement and VCONN fault-clear questions are not resolved by
any inspected Linux file.
