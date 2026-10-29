# Documentation

Start with the [project overview and compiling example](../README.md), then read
the owner guide for application flow. These guides describe implemented behavior
and its limits. The initial implementation is complete; physical board and timing
qualification remain NOT RUN, and TUSB322I support remains partial.

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

For software checks without a board, start with the source-checkout
[validation setup](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/software-validation-setup.md).
The source checkout also maintains [test commands and registration](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/README.md),
[protocol conformance](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/protocol-conformance.md),
[final core and integration review](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/final-audit.md),
[validation history](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md)
and [contribution/release instructions](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/CONTRIBUTING.md).
These files are repository-only, so packaged guides link to their source paths.

Before designing a board, use the source-checkout
[board design checklist](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/board-design-checklist.md),
[staged bring-up plan](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/bring-up-plan.md)
and [prepared TI clarification questions](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/ti-clarification.md).
These worksheets record undecided hardware choices and do not qualify a circuit.

The [recorded CI checkpoint](https://github.com/janhavelka/TUSB32x/actions/runs/40782369910)
passed 14 jobs at `c50f8ddda8809425ede2161586466c95fa650ae6`. It applies only to
that commit and establishes no physical qualification. Local IDF 6.1 rebuilds
remain blocked by the unavailable SDK; earlier CI evidence retains its original SHA.

Current contributors use [AGENTS.md](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/AGENTS.md)
and the [engineering contracts](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/prompts).
The [completed implementation prompts](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/archive/prompts)
and [dated audit sources](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/audit)
are source-only history, not alternate user instructions or hardware evidence.
