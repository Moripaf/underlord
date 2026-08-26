# Underlord Architecture

Underlord is an seL4-based system for running Unikraft unikernels in VMs. The
current milestone establishes the process boundary: a root-task hypervisor
creates and supervises one VMM child. No guest VM is booted yet.

## System overview

```
QEMU qemu-arm-virt (AArch64)
        │
        ▼
elfloader → seL4 kernel
        │
        ▼
hypervisor root task
  ├── owns initial authority and allocation
  ├── embeds the VMM ELF in CPIO
  └── creates and supervises one child
        │
        ▼
my-vmm child task
  ├── receives a restricted CSpace
  ├── performs a READY handshake
  └── blocks awaiting control
```

The [hypervisor](hypervisor.md) owns root authority and VMM lifecycle. The
[VMM](vmm.md) defines the child-side capability and control protocol.

## Repository and modules

The source tree and documentation live in one project repository.

| Module | Purpose |
|---|---|
| `hypervisor/` | seL4 rootserver and VMM supervisor |
| `my-vmm/` | Restricted child process that will host VM logic |
| `underlord-utils/` | Shared bounded logging and component-specific log APIs |
| `tests/` and module-local `tests/` | Host and seL4/QEMU tests |

Generated files live in `build/`; the external seL4 checkout is selected
with `SEL4_ROOT`.

## Shared utilities

`underlord-utils` formats bounded records independently from the output
backend. It exposes VMM logging through `underlord/vlog.h` and keeps
`underlord/hlog.h` private to the hypervisor. Records use
`[LEVEL] module: message` and are printed with a terminating newline.

## Build and feedback loop

The project uses CMake and Ninja with the seL4 build system. Its defaults are
AArch64 `qemu-arm-virt`, ARM hypervisor support, GICv2, 1024 MiB RAM, and a
debug-oriented build. `my-vmm` is built first and packaged into the
`hypervisor` rootserver through `MakeCPIO`.

```sh
cd $PROJECTROOT
./init-build.sh --sel4-root /path/to/sel4-checkout
cd build
ninja
./simulate
```

`build/simulate` is the final runnable development artifact. Testing has a
separate build mode described in [Testing Strategy](testing_strategy.md).

## Current boundary

The system supports one static VMM, one startup handshake, and terminal fault
reporting. It does not yet provide guest creation, VM/vCPU setup, resource
delegation, virtual devices, restart/reclamation, monitoring, or an external
management API.
