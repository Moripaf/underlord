# Hypervisor Module

`hypervisor/` is the sole seL4 rootserver. It retains initial authority,
creates one restricted VMM process, and supervises that process.

## Owned state

`hypervisor_context_t` owns the bootinfo-backed `simple_t`, root `allocman`,
VKA, and root VSpace. Bootstrap uses a 1 MiB static allocator pool and adds a
4 MiB virtual pool.

Each `vmm_instance_t` records:

- a stable instance ID;
- the `sel4utils_process_t` child resources;
- root-held control and fault endpoints; and
- a root-owned descriptor/ELF mapping that the child can read but cannot
  modify; and
- a root-owned image arena whose frames are directly retyped and shared
  read-only with the child; and
- lifecycle state: `created`, `starting`, `running`, or `faulted`.

The lifecycle transitions are implemented as pure logic in
`vmm_lifecycle.c`; invalid transitions preserve the current state.

## Startup and supervision

1. `hypervisor_bootstrap()` initializes root allocation and VSpace services.
2. The manager resolves the selected `c-hello` or `cpp-hello` CPIO entry into
   an immutable image view, copies it into an arena-backed root mapping, and
   maps the descriptor and image read-only at VMM address `0x7000000000`.
3. `sel4utils` constructs the child process from the embedded ELF.
4. The manager allocates and copies the protocol control endpoint into the
   child, starts it, and waits for the [VMM READY contract](vmm.md#control-contract).
5. After READY, the instance becomes `running`; it accepts `GUEST_LOADING`,
   `GUEST_BOOTING`, and `GUEST_STARTED` in order.  It emits exactly one
   `UNDERLORD_PHASE2_RESULT: PASS` only after the VMM reports its post-hello
   PSCI `SYSTEM_OFF` as `GUEST_STOPPED`, then remains blocked on its endpoint.

A startup error moves a starting instance to `faulted`. A runtime fault is
terminal: it is logged and the root task remains blocked.  Neither terminal
path returns from the root task. Resources are not currently reclaimed.

## Authority and capability contract

The hypervisor retains initial root-task authority and provisions the child
according to the [VMM capability contract](vmm.md#runtime-behavior-and-capabilities).
Manifest version 2 carries the selected ordinary untyped's exact size bits and
physical base; the root retains its original cap. The complete slot and memory
contract belongs to [Memory architecture](memory_architecture.md).

The hypervisor validates the VMM and both bundled guest entries before child
start. The selected image is passed to startup explicitly, so a later input
source can use the same interface. The VMM is not reported running until it sends `VMM_READY`; guest
events are checked in protocol order. The precise child CSpace slot and message
encoding belong to the [VMM contract](vmm.md).

## Source map

| File | Responsibility |
|---|---|
| `root_bootstrap.c` | Root allocator and VSpace initialization |
| `vmm_image.c` | Pure VMM image metadata validation |
| `vmm_lifecycle.c` | Pure lifecycle state transitions |
| `vmm_resource.c` | Pure delegated-untyped selection policy |
| `vmm_manager.c` | Process creation, root-owned image mapping, capability grant, protocol handshake, and faults |
| `main.c` | Bootstrap/start/supervise wiring |
