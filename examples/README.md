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
processor; they do not prove firmware or hardware integration.

Arduino and native ESP-IDF transport/owner examples arrive in later prompts.
Current component registration is build plumbing only, with no linked firmware
or hardware qualification claim. No empty firmware environment or successful
hardware stub is provided. See [development metadata](../docs/development.md).
