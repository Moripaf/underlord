# Underlord Documentation

Underlord is building an seL4 root-task hypervisor and restricted VMM processes
for running Unikraft unikernels.

## Current-state documentation

- [Architecture](project_architecture.md): system overview, modules, shared
  utilities, build flow, and current project boundary.
- [Hypervisor](hypervisor.md): root authority, owned state, VMM lifecycle,
  capability delegation, and supervision contracts.
- [VMM](vmm.md): child behavior, capability view, and control protocol.
- [Testing strategy](testing_strategy.md): host tests, seL4 target tests, QEMU
  integration scenarios, and automation.

## Planning and research

- [Phase 1 hypervisor plan](phase_1_hypervisor_plan.md)
- [Unikraft boot requirements](docs/unikraft_boot_requirements.md)
