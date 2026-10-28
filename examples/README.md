# Examples

The [ordinary host consumer](host/basic/README.md) is implemented. It compiles two
application translation units, prints the stable package version and validates
local values, decodes a supplied register byte and projects UNINIT passive health
through the public API. The unbound facade performs no initialization or I/O.
Its explicitly fake callbacks fail if invoked; no device is present.
Build it using standalone CMake or its pinned native PlatformIO environment.

The [shared CLI](../docs/CLI.md) is implemented under `examples/common/`.
Its host fixture executes queries, target/lifecycle edits and staging without
handler callbacks; managed operation commands use the core through one poll per
turn. Results remain core-owned until explicit take. No startup command runs.
The CLI and console suites plus a persistent Python transcript test this same
processor; the complete diagnostic inventory and executable parity/negative
contract checks also use it. These prove no firmware or hardware integration.

The [Arduino Wire adapter](../docs/arduino-transport.md) is implemented, with host
contract tests and separate S2/S3 SDK link fixtures. It borrows the application
bus and enforces the documented SDK configuration, aggregate budget, shared gate
and failed-receive retirement. Complete board examples/CLI wiring and native
ESP-IDF transport remain later prompts. Builds do not qualify electrical
behavior or application timing. See [development metadata](../docs/development.md).
