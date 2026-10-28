# Native application recording tests

These tests compile the actual `examples/esp_idf/ExampleApp.cpp`, native adapter,
shared CLI and `ExampleOwner`. The fixture includes the application source to
inspect exact retained tokens and ownership when the recording SDK deletes a
resource. Startup, UART input, CLI execution, polling and shutdown still run
through the production application functions. Each scenario runs in a separate
process so application static storage retains its actual lifetime.

The recording SDK checks explicit UART0 pin/configuration, an RX ring with no TX
ring, zero-wait reads of at most 64 bytes and FIFO offers of at most 256 bytes.
Multiple commands in one read preserve unconsumed input; partial output accepts
preserve the remaining bytes. Overflow before or during a read of an incomplete
write command disables input rather than executing its later suffix. A saturated
event queue also disables input, including when all retained events report data.
An existing console is never acquired or deleted. Default absent IRQ input
performs no GPIO read; a configured input is read only by the explicit command.

Read-only startup takes the exact initialize and refresh results before CLI
admission, including during output backpressure. Failure retains backend timeout
detail and leaves local diagnostics usable. CLI operations keep their own core
results until an explicit take. Shutdown cancels active work without another
callback, takes the startup owner's result, preserves a CLI result and ends the
binding before deleting owned device, bus and console resources. Removal failures
retain owned storage and are attempted once; device removal failure blocks bus
deletion. Zero-wait UART completion polling preserves FIFO bytes until wire
completion. A permanently full output sink or unfinished UART still permits
finite shutdown after at most 64 drain turns and retains output-abandonment
evidence. A failed console deletion permits at most 64 further diagnostic turns
without a second deletion attempt. Finite configured runtime also triggers
shutdown without an explicit stop request.

The suite also exercises independent UART configuration/pin/install failures,
bus/device creation failures, deletion failures, optional IRQ setup failures,
failed immediate setup cleanup, default diagnostic-only startup and an I2C/UART
pin conflict. A controller already owned by another consumer is left alive.
Recording calls verify copied bus/device metadata,
physical handle association, serialization, one callback per owner turn, no
automatic write/ACK/reset/recovery and no implicit retry.

These are host behavior checks. The fake SDK does not prove real SDK linkage,
FreeRTOS scheduling, UART delivery, physical timing, board readiness or hardware
qualification. Independent S2/S3 SDK application builds provide separate link
evidence; serial and hardware validation require an identified physical fixture.
