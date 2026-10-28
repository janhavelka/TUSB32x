# Arduino Wire host contract fixture

`test_wire.cpp` compiles the real example adapter against a small recording
`TwoWire` surface. The fake headers supply only the SDK symbols used by that
adapter. They are not production headers, a bus simulator or evidence that an
Arduino firmware links. The separate S2/S3 SDK fixtures provide firmware evidence.

The checks use fixed expected bytes and explicit counts. They cover all four
seven-bit addresses, read lengths 1–8, pointer STOP/read STOP, the bus-free guard,
argument/setup rejection, nonblocking gate contention, owner-core/ISR rejection,
Wire status detail, short buffering, incomplete receive, controller retirement,
output preservation, timeout restoration, native 64-bit time and saturated
counters. No callback creates, ends or deletes the borrowed bus.

The external fake physical clock advances within backend calls. Normal cases use
fixed sub-millisecond durations; envelope cases charge each audited SDK wait and
cleanup allowance independently of the adapter's private arithmetic. Builds use
both 1000 Hz and 100 Hz ticks to exercise quantization and budgets too small to
start. A callback which deliberately exceeds its qualification checks failure
detection and phase clipping; that test does not claim such a backend is bounded.
The model assumes the audited SDK wait count and qualified application overhead.
It cannot establish the actual board's scheduling/preemption bound or electrical
behavior.

The ordinary `CHECK` function-like macro remains active in Release and returns a
nonzero process status on a failed check. There is no private core access, heap
fixture, script interpreter or duplicate operation engine.
