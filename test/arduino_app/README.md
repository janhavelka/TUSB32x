# Arduino application recording tests

These suites compile the actual `ExampleApp.cpp`, Wire adapter, shared CLI and
application owner. The recording SDK runs one scenario per process so static
application state has its real lifetime. The test includes the application source
to drain its output mailbox directly instead of starting its infinite USB worker.
All setup, input and owner turns still use the production application functions.

Scenarios cover two-device read-only startup with a stalled output sink, startup
failure, rejected controller creation, failed preparation, failed preparation
cleanup, failed shutdown release, an already owned controller, normal shutdown,
and USB pin conflicts. They check that both bindings end before controller
release, failed release retains ownership without another attempt, and cached
diagnostics remain usable after setup rejection. The two USB-pin builds reject
an I2C pin or an optional IRQ before any controller or GPIO setup.
Shutdown also preserves a CLI terminal result and cancels an active CLI job into
its exact retained result without another callback. An external controller stays
initialized and is reported as `not_owned`.

These are host behavior tests. They do not establish SDK linkage, FreeRTOS
scheduling behavior, USB delivery, board timing or physical fixture readiness.
