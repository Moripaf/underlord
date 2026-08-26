# VMM Module

`src/my-vmm/` is a non-root child executable. It is the future home of guest
VM logic; today it proves restricted process startup and communication with the
hypervisor.

## Runtime behavior and capabilities

The VMM logs its startup, encodes and sends READY to the hypervisor, then
blocks on its control endpoint without consuming CPU. `VMM_FAULT_TEST`, off by
default, deliberately faults after READY to exercise hypervisor supervision.

The VMM has its own TCB, CSpace, VSpace, stack, and fault endpoint created by
`sel4utils`. It receives no untyped, IRQ, device, I/O, scheduling, VM, or vCPU
capabilities.

## Control contract

`vmm_protocol.h` binds the control endpoint to
`SEL4UTILS_FIRST_FREE` (currently CSpace slot 8). The hypervisor installs the
endpoint there before starting the child.

`vmm_protocol_core.h` defines a target-independent, versioned, one-word IPC
protocol:

| Message | Meaning |
|---|---|
| `READY` | Child initialization completed and the hypervisor may mark it running |
| `FAULT` | Reserved encoded notification; runtime seL4 faults currently use the fault endpoint |

Encoding and decoding reject null outputs and unknown values without changing
the caller's output storage. Protocol version or slot changes must update both
participants and their tests together.

## Source map

| File | Responsibility |
|---|---|
| `main.c` | Startup, READY send, optional fault, and blocking wait |
| `vmm_protocol_core.c` | Pure message encoding and decoding |
| `vmm_protocol.h` | seL4 capability-slot binding |
| `vmm_protocol_core.h` | Shared protocol values and API |

Guest image loading, allocator bootstrap, vCPU creation, guest memory, and
virtual devices are not implemented.
