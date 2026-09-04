# Underlord Agent Guidance

## Documents and their maintenance

Before planning or changing this project, read the documentation relevant to
the work:

- [`docs/project_architecture.md`](docs/project_architecture.md) for the system
  overview, repository layout, build flow, shared utilities, and cross-module
  concerns.
- [`docs/hypervisor.md`](docs/hypervisor.md) for root-task authority,
  allocation, VMM lifecycle management, and hypervisor contracts.
- [`docs/vmm.md`](docs/vmm.md) for the child VMM's capability view, startup
  behavior, and control protocol.
- [`docs/testing_strategy.md`](docs/testing_strategy.md) for test layers,
  targets, scenarios, and QEMU automation.

Do not duplicate concepts between documents. Update the document that owns the
concept and link to it from other documents when context is needed.

Every implementation plan must include an explicit documentation-update step.
Every execution that changes the project must update the relevant documentation
in `docs/` in the same change. Update `docs/project_architecture.md` whenever a
change affects system topology, the build, shared utilities, cross-module
boundaries, or the test workflow. Update `docs/hypervisor.md` or `docs/vmm.md`
when that module's behavior, capabilities, contract, or implementation changes.

## Coding Rules

All project-owned header files must use seL4-style `/***` documentation
comments. Document the header's intent and every public struct, enum, macro,
and function. Struct comments must name each field with `@param`; function
comments must name every input with `@param`, declare preconditions/expected
state, identify outputs with `@return`, and state observable side effects and
possible errors. Keep comments specific to the interface; do not restate the
implementation.

[`LINUS.md`](LINUS.md) is the ultimate coding-practices authority for this
project. Read it before writing or reviewing code and follow it in preference
to convenience, speculative abstractions, or unrelated cleanup. This is
OS-level code: design the data layout and capability ownership first, keep hot
paths and control flow obvious, make surgical changes, and verify claims with
reproducible evidence.

Write code so it can be tested without dragging an entire component's process
entry point into the test. Keep `main.c` limited to wiring and lifecycle setup;
put state transitions, capability-manifest validation, message encoding,
CPIO/ELF validation, and log formatting in small modules with clear inputs and
outputs. Keep pure logic independent of seL4 where that does not distort the
design, so it can have fast host unit tests.

Do not substitute host tests or console output for target correctness. Test
capabilities, CSpace contents, IPC, faults, ELF loading, allocators, and other
seL4-facing behavior in the configured AArch64 QEMU environment. Logs are
diagnostics; integration pass/fail must use explicit state, IPC, or another
machine-checkable result.

Define success before implementation and state how it will be verified. For a
multi-step change, write each step with its verification check. Do not add
test-only code or hooks to the production image when a separate test target can
exercise the behavior.

## Execution

Use `build/simulate` as the final runnable artifact for the normal
build-and-run feedback loop.
