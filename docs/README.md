# Documentation

Start with the [project overview and compiling example](../README.md), then use
the owner guide for application flow. These guides describe the implemented API;
an unresolved silicon gate or missing physical qualification stays explicit.

## API and ownership

| Guide | Use it for |
| --- | --- |
| [Architecture](architecture.md) | Core, adapter, application, CLI and runner responsibilities |
| [API and Status contracts](api-contracts.md) | Exact methods, error meanings, output preservation and passive values |
| [Owner operations](owner-operations.md) | Bind, accepted admission, bounded poll, exact result take, timing, effects, cancellation and shutdown |
| [Transport](transport.md) | Terminal borrowed callbacks, aggregate timeout, framing, access and identity |
| [Diagnostics and health](diagnostics.md) | Raw access, all-source versus managed counters, retained evidence and passive health |
| [Integration](integration.md) | Ordinary C++ and FieldCore-style ownership, shared-bus fairness and lifetimes without product dependencies |

## Device interpretation and electrical limits

| Guide | Use it for |
| --- | --- |
| [Variant matrix](variant-matrix.md) | Configured generations, seven-bit addresses, revision evidence, enable polarity and capability gates |
| [Register map](register-map.md) | Exact TI register meanings, read/write access and persistent/IRQ/reset masks |
| [Register vectors](register-vectors.md) | Independent byte examples and reserved/unknown encodings |
| [Device behavior](device-behavior.md) | Profile changes, reset/recovery waits, port applicability and bounded interrupt service |
| [Feature coverage](feature-coverage.md) | Feature-to-API/CLI/source/test mapping, exact TI evidence and unresolved restrictions |
| [Hardware validation](hardware-validation.md) | Supply/straps/VBUS_DET/INT, fixture prerequisites and physical NOT RUN criteria |

## Applications, commands and tests

| Guide | Use it for |
| --- | --- |
| [Examples index](../examples/README.md) | Runnable simulated host consumer and Arduino/native-IDF applications |
| [Arduino examples](arduino-examples.md) | CLI and two-device owner setup, board readiness, USB isolation and shutdown |
| [Arduino transport](arduino-transport.md) | Qualified Wire configuration, SDK blocking/cleanup and borrowed callback limits |
| [Native IDF transport](idf-transport.md) | Synchronous handles, shared serialization and audited SDK configuration |
| [Native CLI application](../examples/esp_idf/basic/README.md) | SDK 5.5.5/6.1.0 S2/S3 builds with separate absolute build/SDKCONFIG paths |
| [CLI reference](CLI.md) | One authoritative command/alias grammar, staging, machine records and output-loss rules |
| [HIL runner](hil-runner.md) | Readonly/soak and seven explicit fixture plans, identities, deadlines, artifacts, verdicts and conditional cleanup |
| [Development and packages](development.md) | Stable version truth, sync/check/bump/set, supported tools and clean archive consumption |

The source checkout also maintains [test commands and registration](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/README.md),
[protocol conformance](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md),
[validation history](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md)
and [contribution/release instructions](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/CONTRIBUTING.md).
These files are repository-only, so packaged guides link to their source paths.

Implementation, host verification, linked firmware and physical qualification
are separate evidence levels. The recorded CI checkpoint identifies its exact
tested commit; later guide edits do not extend that result. No physical serial,
I2C/CC/current, INT or supply timing is qualified by host models or image links.
See the [recorded checkpoint](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910).

The [38 implementation prompts](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/prompts)
and [dated audit sources](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/audit)
are source-only history, not alternate user instructions or hardware evidence.
