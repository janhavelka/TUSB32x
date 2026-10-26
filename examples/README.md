# Examples

The [ordinary host consumer](host/basic/README.md) is implemented. It compiles two
application translation units, prints the stable package version and validates
local values, decodes a supplied register byte and projects UNINIT passive health
through the public API. The unbound facade performs no initialization or I/O.
Its explicitly fake callbacks fail if invoked; no device is present.
Build it using standalone CMake or its pinned native PlatformIO environment.

Arduino and native ESP-IDF transport/owner examples arrive in later prompts.
Current component registration is build plumbing only, with no linked firmware
or hardware qualification claim. No empty firmware environment or successful
hardware stub is provided. See [development metadata](../docs/development.md).
