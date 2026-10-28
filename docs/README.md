# Documentation

- [API contracts](api-contracts.md): implemented values, validators, decoder and
  read-only initialization, profile observation, explicit apply, desired
  verification, read-only reconciliation, explicit reset, base recovery, complete
  read-only port refresh and explicit IRQ acknowledgement/bounded service.
  Raw Sync diagnostics and pure passive health are also implemented.
- [Architecture](architecture.md): public/core/example responsibilities and the
  application-owned transport boundary.
- [Development](development.md): version truth, generator exits, tool choices,
  component registration and package contents.
- [Transport](transport.md): implemented private framing, access, accounting and
  identity checks, with adapter and later owner limits.
- [Arduino transport](arduino-transport.md): borrowed Wire callbacks, SDK limits,
  aggregate deadline, controller retirement and S2/S3 build fixtures.
- [Arduino examples](arduino-examples.md): CLI and two-device owner applications,
  explicit wiring/qualification, USB isolation, source-filtered S2/S3 builds and shutdown.
- [Diagnostics and health](diagnostics.md): safe raw access, retained effects,
  all-source/managed callback accounting and pure health projection.
- [Variants](variant-matrix.md), [register map](register-map.md) and
  [register vectors](register-vectors.md): source-backed capability and encoding
  specifications; they do not claim driver or hardware qualification.
- [Device behavior](device-behavior.md): implemented profile/reset/recovery/port/IRQ
  procedures, specified later operations, work bounds and silicon evidence.
- [Feature coverage](feature-coverage.md): implemented, planned and gated work.
- [Protocol conformance](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md):
  repository-only requirement-to-test matrix and host evidence limits.
- [CLI and operations](CLI.md): bounded input/output, local staging, managed
  operations, raw/pure diagnostics, health/counters, retained-result delivery and
  schema 1 records; executable parity checks cover the complete fixed inventory.
- [Examples](../examples/README.md): the current portable consumer.

Repository-only [implementation prompts](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/prompts),
[audit evidence](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/audit)
and [validation history](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md)
are maintained in the source checkout and excluded from normal packages.
