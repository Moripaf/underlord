# Underlord Architecture

Underlord is an seL4-based system for running Unikraft unikernels in VMs. The
current milestone establishes fixed Phase-2 contracts for one AArch64
Unikraft guest hosted by one VMM child.

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
  ├── embeds the VMM and both compatible hello ELFs in CPIO
  └── creates and supervises one child
        │
        ▼
my-vmm child task
  ├── receives a restricted CSpace
  ├── reports versioned guest lifecycle events
  └── owns one bounded construction budget
```

The [hypervisor](hypervisor.md) owns root authority and VMM lifecycle. The
[VMM](vmm.md) defines the child-side capability and control protocol. The
[memory architecture](arch/memory_architecture.md) owns guest RAM and delegated
capability details. The [boot sequence](arch/boot.md) owns the ordered target
startup and terminal-shutdown flow. The [capability model](arch/capability_model.md)
owns CSpace layout and authority delegation.

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
The minimum emitted level is selected at configure time with
`UNDERLORD_LOG_LEVEL` (default `INFO`); lower-level utility functions compile
to no-ops.

## Build and feedback loop

The project uses CMake and Ninja with the seL4 build system. Its defaults are
AArch64 `qemu-arm-virt`, ARM hypervisor support, GICv2, 2048 MiB RAM, and a
debug-oriented build. `my-vmm` is built first and packaged into the
`hypervisor` rootserver through `MakeCPIO`. Normal builds require externally
built `c-hello` and `cpp-hello` ELFs with matching `.config` sidecars;
`UNDERLORD_GUEST` selects which bundled image starts. Underlord validates but
does not build or modify catalog applications. All Underlord-facing cache
options and their defaults are declared together in `settings.cmake`; the
top-level `CMakeLists.txt` retains only validation and build-graph logic.

Phase 2 uses 2048 MiB rather than 1024 MiB because the rootserver's placement
at 1 GiB leaves no 2^28 non-device boot untyped. The fixed VMM construction
budget requires that contiguous capability even though the guest itself uses
only 128 MiB.

```sh
cd $PROJECTROOT
./init-build.sh --sel4-root /path/to/sel4-checkout \
  -DUNDERLORD_GUEST=cpp-hello
cd build
ninja
./simulate
```

`build/simulate` is the final runnable development artifact. Testing has a
separate build mode described in [Testing Strategy](testing_strategy.md).

The normal 2048 MiB QEMU artifact boots the configured guest, captures its
hello through the trapped UART, accepts its PSCI terminal event, and emits one
`UNDERLORD_PHASE2_RESULT: PASS` marker. The intentional post-PASS block is
terminated by the QEMU test harness.

## Current boundary

The fixed Phase-2 scope excludes multiple guests/vCPUs, virtio, SMP, dynamic
resource requests, restart/reclamation, monitoring, and an external management
API. See the detailed [Phase-2 plan](phase_2_initial_vmm_plan.md).
