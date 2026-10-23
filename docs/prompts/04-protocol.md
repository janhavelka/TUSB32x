# P04 — Implement exact register transport and variant decoding

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** PCA9555 register
write effects, ADS1115/INA3221 transfer/status handling, LDC1614 variant validation,
and TMP1x2 register access/testing. Re-read the TI tables chosen in P01.

## Work

1. Implement explicit variant descriptors and register allowlists. Address validation
   distinguishes original 320 `0x60/0x61` from LI/HI/LAI/HAI `0x47/0x67`. Unsupported
   address/model/features fail locally. No automatic writes, broad scan or variant
   guessing in bind. Expose pin-polarity facts without owning pins.
2. Implement byte-oriented pointer reads/writes, exact length checking, documented
   burst boundaries and serialized access. Use the transfer framing decision from
   P01; do not label repeated START a TI requirement. Short transfers fail without
   publishing a partial value. Do not depend on the device's implicit pointer.
3. Check the full eight-byte original-family signature in ascending register order
   `30 32 33 42 53 55 54 00`. Report ACK, matching interface, configured model and
   revision as separate facts. Do not require an undocumented `0xA0` read on base
   320/I; extended `0x01/0x02` revisions are evidence, not enable-polarity detection.
   Decide future-revision handling explicitly; avoid rejecting benign new revisions
   or granting unsupported capabilities without evidence.
4. Implement pure, tested codecs for every supported field, including different
   debounce encodings, source preference and accessory interpretation. Keep raw
   unknown encodings alongside validity; never map reserved status to a valid role.
5. Compose writes by access class, not generic byte read-modify-write. Persistent
   settings must write zero to W1C/command bits unless specifically executing that
   operation. IRQ acknowledgement preserves observed/trusted **applied** RW fields,
   never unapplied desired/staged settings. If actual RW state is unknown, first
   read/reconcile or return an explicit unavailable result. Updating
   debounce/mode/preference must not replay a set soft-reset bit.
6. Keep `0x45 DISABLE_RD_RP` distinct from extended `0x0A DISABLE_TERM` and its
   state-machine procedure. Respect TI-internal reserved fields. No reads/writes
   of undocumented registers on a variant; raw access is not a feature bypass.
7. Separate tracked access, raw diagnostic access and passive cached views. Raw
   writes expose side effects and invalidate trust. Raw reads/probes bypass the
   optional managed health projection but still appear in all-callback statistics.
8. For TUSB322I, implement only the independently resolved capability subset:
   distinct ID, no PORT default, no documented `0x45`, and independent VCONN fault
   W1C semantics. Explicitly disable unresolved operations and report why.

## Tests and completion

- Script exact address/TX/RX/length/framing/timeout expectations; reject unexpected
  calls. Test both legal addresses per variant, reversed/wrong/partial ID, unknown
  revision, pointer wrap/burst limits and read failure with unchanged output.
- Exhaustively test small field codecs, illegal enum casts and reserved writes.
  Mutating a mixed register with IRQ/reset bits set must not acknowledge/reset.
- Stage a profile different from applied hardware while IRQ is pending; ACK must
  leave the applied profile unchanged and must not opportunistically apply staging.
- Fault injection: before callback, address/data NACK where distinguishable, generic
  bus error, timeout, short read/write, late success, and ambiguous write effect.
- All constants reference TI evidence. A fake models enough W1C/self-clearing/live
  behavior to expose a wrong implementation, not simply echo written bytes.
- Run native tests and strict framework-free compilation. Record adapter framing
  that still needs physical validation without claiming protocol certainty.
