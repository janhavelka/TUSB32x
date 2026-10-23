# Promptset correctness and completeness review

Date: **2026-10-23**. Reviewed the 38 numbered prompts and their shared contracts
against preparation commit `8dfc8e26f8ff065a8c646df86ee127bae6e3da58`.
This is a review of implementation instructions, not an implemented driver.

The review used three independent scopes: silicon and source evidence;
API, ownership and state; and CLI, adapters, runners and packaging. The lead
integrated their findings and checked shared rules, dependencies and consistency.
This report supersedes conflicting earlier preparation notes. Exact chip claims
still require verification during implementation against the selected TI revision.

## Confirmed errors corrected

| Finding | Correction and required acceptance case | Main instructions |
| --- | --- | --- |
| The earlier public-code audit called the TUSB322I termination sequence conditional, although TI explicitly documents it for entering DFP and UFP. | Use disable, select mode, enable: clean-byte DFP `01 → 21 → 20`, UFP `01 → 11 → 10`; preserve unrelated controls. TI specifies no extra 5 ms wait for this sequence. Keep direct non-reset return to DRP separately reviewed. | 3, 4, 13; CHIP_EVIDENCE and silicon audit reports |
| An attached-state prohibition could reject all original TUSB320/I profile changes. | Reject an actual attached mode change before any write. Independently permitted current advertisement changes remain possible. An unchanged mode must not trigger a termination cycle. | 4, 13, 18 |
| A complete profile could make every TUSB322I apply depend on an unresolved advertisement write. | Gate actual unsupported changes, using valid actual evidence. An unchanged readable field does not require a write. Unknown evidence cannot be treated as an unchanged value. | Baseline, 7, 13 |
| A one-byte decoder was expected to infer attachment-dependent current/accessory applicability. | Decode field codes without fabricated context; establish applicability from paired port observations. | 5, 10, 15 |
| “Bounded retries” for port coherence conflicted with the no-hidden-retry contract. | Use one fixed `09 / 08 / 09` observation. Compare attachment/direction bits `0xE0`; a mismatch clears dependent field validity. Matching bookends are not proof of atomic capture. Never restart until stable. | Baseline, 15, 18 |
| Apply admission or pre-write cancellation could discard a still-proved actual configuration. | Retain the accepted desired image separately. Invalidate actual evidence immediately before a possibly mutating write; a rejected or cancelled pre-write operation cannot claim a hardware effect. A no-op apply verifies without writes. | Baseline, 6, 13 |
| Callback error normalization handled IN_PROGRESS but left other illegal owner/validation statuses able to escape. | Accept only OK and documented I2C_* callback codes. Normalize every other code, including invalid enum values, to I2C_ERROR while preserving backend detail and conservative effects. | AGENTS, baseline, 9, 18 |
| A timeout argument was treated as proof of a bounded SDK operation. | Audit locks, repeated waits, tick granularity and cleanup in the selected SDK. Require synchronous use and serialization covering all bus users; reject unsupported configurations or budgets before backend work. Detecting lateness after return does not bound blocking. | Engineering rules, baseline, 23, 25 |
| The native-IDF callback address could disagree with its borrowed device handle, or asynchronous callbacks could outlive borrowed buffers. | Store and check the handle's configured address before SDK work; require synchronous handles without asynchronous event callbacks. The application replaces/reconfigures a handle when changing address. | 25, 26 |
| Separate native-IDF build directories could still reuse the project's sdkconfig. | Give each target its own absolute SDKCONFIG path consistently through configure/build. Verify S2 → S3 → S2 without configuration leakage. | 26, 32, 34 |
| The package contract confused the CMake target TUSB32x with a directory-derived IDF component name. | Resolve the actual component name in renamed clean consumers; exercise it instead of relying on the checkout folder name. | 26, 32 |

## Missing contracts and coverage added

| Gap | Completed requirement | Main instructions |
| --- | --- | --- |
| Profile reads could lose raw values, reserved encodings or per-register validity. | Preserve a small fixed raw profile observation and decoded-field validity alongside interpreted values. Never convert absent or invalid evidence into a plausible default/full proof. | Baseline, 5, 12, CLI_CONTRACT |
| Complete new profile evidence could contradict an older KNOWN image without demoting it. | A contradiction or applicable reserved encoding invalidates old current trust and affected port usability. Preserve historical verified data; read-profile alone cannot promote the replacement image. | Baseline, 12 |
| Variant/address/context did not distinguish changed callbacks with a shared or null context. | Include both callback pointers in target association. Define one configuration owner per physical target; an external writer must coordinate and invalidate evidence. A bus lock alone does not protect cached state. | AGENTS, baseline, 6, 11, 27 |
| Reinitialization could retain READY/initialized identity from a prior successful probe. | Clear current initialization/identity usability when init is accepted; preserve historical observations and unresolved effects. Identity mismatch cannot retain trusted current configuration. | Baseline, 6, 11, 17 |
| Owner time checks did not fully define duplicate, backward or half-range timestamps. | Use the selected unsigned millisecond domain and explicit half-range checks. Equal ticks are legal but add no time. A fresh sample after a callback may have the same numeric tick. Without a fresh completion time, mark it invalid rather than reuse last-poll time. | Baseline, 6, 11, 19 |
| Snapshot publication counter wrap and boundary tests were unspecified. | Saturate the counter, reserve zero for no publication, and use operation tokens for correlation. Test narrow internal arithmetic helpers at limits without public test hooks or billions of calls. | Baseline, 6, 15, 19 |
| The CLI could retain device effects but had no explicit way to forget them. | Add `forget-evidence` with unbound/inactive/no-result preconditions and lifecycle help. It forgets evidence; it does not repair hardware. Address/bind commands cannot silently forget. | CLI_CONTRACT, 20, 22 |
| Separate variant and address setters could reject both steps of a cross-family change. | Allow an explicit atomic `variant <name> <address>` pair. After validation and evidence gates, an actual variant change stages its documented candidate defaults; address-only changes preserve staging. Failed edits change nothing and no address is guessed. | CLI_CONTRACT, 20 |
| Observation runners could change local CLI settings permanently or cancel another owner's result. | Require an idle session, track only owned tokens, save/restore changed local staging/output/auto-poll settings without applying hardware settings, and report unrestored state if a session is lost. | 28, 29 |
| Archive checks did not cover Windows path aliases. | Reject alternate data streams/colon names, reserved device names, trailing dot/space aliases and case-insensitive collisions before extraction. Keep the existing checker small and test malicious archives. | 32 |
| The general local validator could accidentally reintroduce remote CI polling. | `--checks all` remains local. Prompt 34 owns the initial promptset's remote checkpoint; later fixes retain local checks and the earlier run's actual SHA. | 33, 34 |
| Generic family documentation could imply interchangeable pins or connector-relative orientation. | Record 320 pin 3 PORT / pin 4 VBUS_DET versus 322 pin 3 VBUS_DET / pin 4 DIR. CC1/CC2 describe chip inputs; board routing determines connector orientation. | 2, 3, CHIP_EVIDENCE |
| TUSB322I VCONN fault access wording was insufficient to establish a clear operation. | Retain raw fault evidence and its validity; do not silently reuse IRQ ACK or invent a typed fault-clear operation. | 3, CHIP_EVIDENCE |
| Documentation requirements could produce private-helper boilerplate instead of useful contracts. | Require complete public Doxygen and useful file/source explanations; do not publish private symbols or demand repetitive comments on trivial private helpers. | MAINTAINABILITY |

## Source checks

Chip facts were checked against the archived official TI sources and the current
[TUSB322I datasheet](https://www.ti.com/lit/ds/symlink/tusb322i.pdf), SLLSEO5D,
sections 6.3.2.1–6.3.2.2, printed pages 10–11. The base attached-mode/current
distinction comes from SLLSEN9F sections 7.3.2 and 7.4.2, printed pages 12 and 14.
The [family audit](ti-family.md), [additional TI evidence](additional-ti-evidence.md)
and [public-code audit](community-public-implementations.md) record the corrected
context. Archived PDFs and the pinned source manifest remain unchanged.

SDK assumptions were checked against these primary sources, rather than inferred
from API parameter names:

- [Arduino-ESP32 3.3.0 Wire implementation](https://github.com/espressif/arduino-esp32/blob/3.3.0/libraries/Wire/src/Wire.cpp),
  blob `cda098d2d5b616f99fd608cb0f4ea22ec173c6fa`: internal locks can use
  portMAX_DELAY; setTimeOut alone is not a callback-wide bound. The implementation
  prompt must inspect the actual pinned release it selects.
- [ESP-IDF v5.5.1 I2C master implementation](https://github.com/espressif/esp-idf/blob/v5.5.1/components/esp_driver_i2c/i2c_master.c),
  blob `562e6788a33d0c796d0cb3b718e72df47a42e0da`: timeout values can be reused for
  locks/transaction waits and failure paths perform internal bus/controller cleanup.
  Adapter-added retries/recovery remain prohibited; SDK-internal behavior must be
  disclosed as an application choice.
- [ESP-IDF v5.5.1 I2C API](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32/api-reference/peripherals/i2c.html)
  distinguishes synchronous calls from callback-driven asynchronous operation.
- [ESP-IDF v5.5.1 build system](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32/api-guides/build-system.html)
  documents project configuration and SDKCONFIG; a build-directory split alone
  does not establish separate configuration files.
- [Windows file naming rules](https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file)
  establish reserved names, case behavior, streams and trailing-character hazards.

Current read-only peer checks included LDC1614 (`c0ab4c1`), LSM6DS3TR (`513c564`),
PCA9555, TMP1x2 and FieldCore-node (`154a4dc`). Their concrete implementation
conventions informed the fixes; none was changed or treated as silicon authority.
In particular, FieldCore's richer completion-time transport does not mean a
Status-only TUSB32x callback carries the same timing evidence.

## Remaining limits and validation

Unresolved TI advertisement/fault access wording and the direct non-reset 322 DRP
transition remain narrow evidence gates. The prompts require corrections when
real sources disagree; they do not turn an open source driver or a mock into a
hardware specification. No inferred electrical behavior was promoted to a fact.

The 38-prompt sequence, per-prompt audit/docs/tests/commit/sync requirements and
single execution branch remain intact. Local artifact checks and exact counts
are recorded in [preparation validation](promptset-validation.md). No driver,
firmware, remote CI or physical hardware validation was performed by this review.
