# Examples

The [ordinary host consumer](host/basic/README.md) runs an explicitly simulated
TUSB320LAI with synthetic monotonic time through public callbacks. Two application
translation units demonstrate binding, accepted admission, one-callback polling,
exact-token delivery, explicit apply, copied port evidence, external invalidation
and independently selected read-only reconciliation. Delivery OK remains distinct
from a failed operation payload. Build/run it with standalone tests-OFF CMake or
the pinned native PlatformIO environment. No physical chip is present. The
[generic owner guide](../docs/integration.md) maps the same API to a serialized
two-device owner without any product or framework dependency.

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
without Arduino or the CLI. The complete [native CLI application](esp_idf/basic/README.md)
links that adapter, core and shared CLI for IDF5.5.5 on S2/S3, with exclusive UART0,
explicit readiness, read-only startup ownership and checked bounded teardown.
Builds do not qualify electrical behavior or application timing.
See [development metadata](../docs/development.md).
