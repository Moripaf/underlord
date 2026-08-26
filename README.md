# Underlord

Underlord is a seL4-based hypervisor project for running Unikraft unikernels
as isolated virtual machines. Its long-term goal is to manage VMM and VM
lifecycle, capability delegation, resource allocation, and monitoring while
keeping the trusted root-task boundary small and explicit.

## Current state

The current implementation runs on AArch64 QEMU. With A root-task hypervisor
and a VMM that is spawned as child of the hypervisor and is tasked
with running the actual unikraft vms.

## Documentation

- [Architecture](docs/project_architecture.md): system overview, module map,
  shared utilities, build flow, and current project boundary.
- [Hypervisor](docs/hypervisor.md): root authority, owned state, VMM lifecycle,
  capability delegation, and supervision contracts.
- [VMM](docs/vmm.md): child behavior, capability view, and control protocol.
- [Testing strategy](docs/testing_strategy.md): host tests, seL4 target tests,
  QEMU integration scenarios, and automation.
- [Phase 1 hypervisor plan](docs/phase_1_hypervisor_plan.md): original process
  separation milestone and implementation rationale.
- [Unikraft boot requirements](docs/thinking/unikraft_boot_requirements.md):
  guest boot research and assumptions.

## Build and run

```sh
./init-build.sh --sel4-root /path/to/sel4-checkout
cd build
ninja
./simulate
```

`build/simulate` is the normal runnable development artifact.

## Tests

refer to the [Testing strategy](docs/testing_strategy.md) to see how to build,
run and add tests
