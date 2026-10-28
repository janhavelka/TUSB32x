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
and failed-receive retirement. The [interactive Arduino CLI](01_basic_bringup_cli/README.md)
and [two-device owner service](02_owner_service/README.md) share the same processor,
read-only startup owner and static USB output mailbox. Both have explicit S2/S3
environments; see [wiring and build instructions](../docs/arduino-examples.md).
Default firmware is diagnostic-only until board readiness, cleanup acceptance
and the adapter's timing bound are supplied. The [native IDF adapter](../docs/idf-transport.md)
uses borrowed synchronous bus/device handles, a shared controller gate and
source-derived timeout bounds. Its independent S2/S3 component fixtures link
without Arduino or the CLI; the native CLI application remains later work.
Builds do not qualify electrical behavior or application timing.
See [development metadata](../docs/development.md).
