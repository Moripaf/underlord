# VMM Module

`my-vmm/` is a non-root child executable. It owns the fixed Phase-2 guest
contract and its pure image/lifecycle admission rules.

## Runtime behavior and capabilities

The VMM has a 16-bit CSpace. Its protocol defines `VMM_READY` followed by
guest loading, booting, hello, and clean-stop events. Guest lifecycle is
`NONE → LOADING → BOOTING → RUNNING → STOPPED`; any nonterminal state may
become terminal `FAILED`.

The VMM has its own TCB, CSpace, VSpace, stack, and fault endpoint created by
`sel4utils`. On startup it bootstraps a local allocman, VKA, and VSpace from
the exact metadata in manifest version 2, reserving its IPC-buffer page and a
4 MiB allocator virtual pool before it reports `VMM_READY`. It first reserves
the root-created stack-and-guard range so allocator growth cannot replace a
live stack mapping. The complete slot,
anonymous-RAM, device, and authority model belongs to
[Memory architecture](memory_architecture.md).

Before `VMM_READY`, the VMM validates the descriptor at `0x7000000000`. The
descriptor and selected compatible guest bytes are mapped read-only by the
root task; their frame capabilities are retained in root CSpace. After READY, VMM emits
`GUEST_LOADING` only after admitting this immutable input.

## Verified implementation boundary (2026-09-05)

Pure admission now handles `SHT_NOBITS`, rejects overflow-safe invalid ranges,
and validates Unikraft `.uk_bootinfo` magic, version, and nonzero region count.
The normal AArch64 QEMU run boots the configured guest, captures
`Hello from Unikraft!`, accepts its PSCI `SYSTEM_OFF`, and produces the
protocol-derived PASS marker.  On that accepted terminal SMC, the VMM first
suspends the vCPU, sends `GUEST_STOPPED`, and blocks on its local VM endpoint;
it does not return through libsel4vm's fault-reply path or from `main`.

## Control contract

`vmm_protocol.h` binds the control endpoint to
`SEL4UTILS_FIRST_FREE` (currently CSpace slot 8). The hypervisor installs the
endpoint there before starting the child.

`vmm_protocol_core.h` defines version-2 messages:

| Message | Meaning |
|---|---|
| `VMM_READY` | VMM-local construction services are ready |
| `GUEST_LOADING` / `GUEST_BOOTING` | Guest image acceptance and entry preparation progressed |
| `GUEST_STARTED` / `GUEST_STOPPED` | Hello token observed / PSCI clean shutdown observed |
| `GUEST_FAILED` | Terminal typed failure, with stage and signed error code |

Encoding and decoding reject null outputs and unknown values without changing
the caller's output storage. Protocol version or slot changes must update both
participants and their tests together.

## Source map

| File | Responsibility |
|---|---|
| `main.c` | Startup and supervision-event wiring |
| `vmm_guest_contract.c` | Descriptor and guest lifecycle validation |
| `vmm_elf.c` | Strict target-independent ELF admission checks |
| `vmm_resources.c` | Manifest-backed local allocman, VKA, and VSpace bootstrap |
| `vmm_protocol_core.c` | Pure message encoding and decoding |
| `vmm_protocol.h` | seL4 capability-slot binding |
| `vmm_protocol_core.h` | Shared protocol values and API |

The fixed platform contract is GICv2, generic timer, PSCI SMC `SYSTEM_OFF`,
and output-only PL011 at `0x09000000`; virtio and SMP are out of scope.
