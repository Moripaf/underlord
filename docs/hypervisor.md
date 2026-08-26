# Hypervisor Module

`src/hypervisor/` is the sole seL4 rootserver. It retains initial authority,
creates one restricted VMM process, and supervises that process.

## Owned state

`hypervisor_context_t` owns the bootinfo-backed `simple_t`, root `allocman`,
VKA, and root VSpace. Bootstrap uses a 1 MiB static allocator pool and adds a
4 MiB virtual pool.

Each `vmm_instance_t` records:

- a stable instance ID;
- the `sel4utils_process_t` child resources;
- root-held control and fault endpoints; and
- lifecycle state: `created`, `starting`, `running`, or `faulted`.

The lifecycle transitions are implemented as pure logic in
`vmm_lifecycle.c`; invalid transitions preserve the current state.

## Startup and supervision

1. `hypervisor_bootstrap()` initializes root allocation and VSpace services.
2. The manager verifies the embedded `my-vmm` CPIO image.
3. `sel4utils` constructs the child process from the embedded ELF.
4. The manager allocates and copies the protocol control endpoint into the
   child, starts it, and waits for the [VMM READY contract](vmm.md#control-contract).
5. After READY, the instance becomes `running` and the hypervisor waits on its
   fault endpoint.

A startup error moves a starting instance to `faulted`. A runtime fault is
terminal: it is logged and the root task remains blocked. Resources are not
currently reclaimed.

## Authority and capability contract

The hypervisor retains initial root-task authority and provisions the child
according to the [VMM capability
contract](vmm.md#runtime-behavior-and-capabilities). It remains the sole
allocator; resource delegation policy is not implemented yet.

The hypervisor guarantees that a VMM is not reported as running until its CPIO
image is validated, process construction succeeds, and a valid READY message
is received. The precise child CSpace slot and message encoding belong to the
[VMM contract](vmm.md).

## Source map

| File | Responsibility |
|---|---|
| `root_bootstrap.c` | Root allocator and VSpace initialization |
| `vmm_image.c` | Pure VMM image metadata validation |
| `vmm_lifecycle.c` | Pure lifecycle state transitions |
| `vmm_manager.c` | Process creation, capability grant, handshake, and faults |
| `main.c` | Bootstrap/start/supervise wiring |
