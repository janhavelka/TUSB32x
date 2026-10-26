# Documentation

- [API contracts](api-contracts.md): implemented values, validators, decoder and
  read-only owner engine, with future device methods explicitly labeled.
- [Architecture](architecture.md): public/core/example responsibilities and the
  application-owned transport boundary.
- [Development](development.md): version truth, generator exits, tool choices,
  component registration and package contents.
- [Transport](transport.md): implemented private framing, access, accounting and
  identity checks, with adapter and later owner limits.
- [Variants](variant-matrix.md), [register map](register-map.md) and
  [register vectors](register-vectors.md): source-backed capability and encoding
  specifications; they do not claim driver or hardware qualification.
- [Device behavior](device-behavior.md): specified later procedures and evidence.
- [Feature coverage](feature-coverage.md): implemented, planned and gated work.
- [Examples](../examples/README.md): the current portable consumer.

Repository-only [implementation prompts](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/prompts),
[audit evidence](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/audit)
and [validation history](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md)
are maintained in the source checkout and excluded from normal packages.
