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

The RX-loss scenarios model the pinned SDK's 256-byte queue and dropped packet
suffixes. `rx-overflow` fills it with newlines and a `reset` prefix, loses its
invalid suffix, then supplies a later newline. `rx-overflow-after-read` fills
the queue between the owner's capacity check and its single-byte dequeue while
a read operation is active. Both must permanently stop input before a truncated
command can run; the admitted read still finishes and retains its successful
result. Queue capacity minus one after dequeue is deliberately rejected because
it cannot distinguish that race from a nearly full queue. The tests also verify
that later emptying the queue cannot restore command admission.

These are host behavior tests. They do not establish SDK linkage, FreeRTOS
scheduling behavior, USB delivery, board timing or physical fixture readiness.
Queue guards require one reader and the pinned SDK default size; they do not
detect every physical disconnect/reconnect or establish a new USB session.
