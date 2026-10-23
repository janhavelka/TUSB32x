# TUSB32x library planning

This repository currently contains an audit and implementation promptset, not a
finished device driver. The requested starting device is **TI TUSB320LAI**. The
planned library covers the verified I2C-capable TUSB320 variants through explicit
capabilities; the name is not a claim of compatibility with every TUSB32x product.

Start with [the promptset](docs/prompts/README.md). It contains a master execution
prompt, a common contract, and fifteen ordered implementation prompts. Every
stage requires a fresh audit of relevant sibling I2C libraries before editing.

The supporting [audit](docs/audit/README.md) records source observations from
this repository's parent directory, `Projects/`.
`FieldCore-node` is the consumer inspected for the user's `fieldcore-noce`
reference. Its architecture informs integration requirements; it will not be a
library dependency.

The audit and promptset were prepared on **2026-10-23**. Sibling repositories were
inspected without changing them. Audit observations are not claims that their
builds, CI, or hardware tests passed during this task.

The implementation agent must keep native host, Arduino, native ESP-IDF, package,
CI, and physical hardware evidence separate. No implementation or hardware
validation is claimed by the existence of this promptset.
