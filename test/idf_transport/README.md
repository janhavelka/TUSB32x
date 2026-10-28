# Native ESP-IDF adapter host contracts

The two registered suites compile the actual `examples/esp_idf/Tusb32xIdf.cpp`
against a small recording SDK interface at 1000Hz and 100Hz tick rates. The tests
exercise callback arguments, borrowed handle/address association, synchronous
setup restrictions, gate contention and release, exact transmit/receive buffers,
STOP-separated pointer framing, error detail, controller retirement, unchanged
caller output on error, aggregate budget clipping, coarse native ticks, backend
statistics and 64-bit owner time. Successful targets share and reuse one bus
gate. Configuration snapshots remain unchanged, and core bind/end/destruction
perform no SDK transfers or resource operations.

The fake clock advances independently at backend events. Its optional worst-case
script consumes the audited SDK's seven timeout waits and two 50ms bus clears
per phase, including tick rounding. This checks budget arithmetic and fresh
remaining-budget behavior. It cannot prove the SDK's actual wait-site count,
synchronous buffer lifetime, preemption bound, physical timing or board wiring.
The separate real SDK fixture supplies compile/link evidence.

No resource creation, deletion, configuration or recovery functions are declared
by the fake SDK. Accidental adapter calls to those APIs fail compilation.
