# Phase-2 boot and terminal shutdown

This document owns the observed boot and terminal-shutdown sequence for the
single AArch64 QEMU-virt Unikraft guest. Capability and RAM ownership remain
defined by [memory architecture](memory_architecture.md); the protocol
encoding itself remains defined by the [VMM contract](../vmm.md).

## Boot sequence

1. QEMU starts the elfloader, seL4 kernel, and the `hypervisor` root task.
2. Root retains the initial allocation and physical-device authority. It
   validates the embedded VMM and configured `c-hello` ELF, maps the immutable
   descriptor and guest image read-only into the child, and creates the VMM.
3. Root gives the VMM only child CSpace slots 8--10: the badged supervisor
   endpoint, one ordinary untyped, and the GICv2 VCPU-interface frame. The
   physical PL011 frame remains in root CSpace. VMM-local allocation begins at
   slot 11.
4. The VMM validates the manifest, reserves inherited mappings, bootstraps its
   local allocator, and creates its host endpoint, libsel4vm VM root, GICv2,
   and 128 MiB anonymous arena-backed guest RAM at
   `0x40000000..0x47ffffff`.
5. The VMM sends `READY`, then sends `GUEST_LOADING` after accepting the
   immutable guest input. Root admits only this ordered protocol progression.
6. The VMM validates the AArch64 ELF and `.uk_bootinfo`, copies every
   `PT_LOAD` file range through `vm_ram_touch`, and zeroes each BSS tail. It
   writes the runtime FDT into the reserved first MiB of guest RAM.
7. The FDT supplies one CPU, GICv2, generic timer, PSCI 1.0 over SMC, one
   128 MiB RAM bank, and an output-only virtual `arm,pl011` at `0x09000000`.
   That GPA page is reserved, not mapped to the physical PL011 frame.
8. The VMM installs its PL011 memory-fault emulator and PSCI SMC handler,
   sends `GUEST_BOOTING`, then creates and starts one CPU-0 vCPU at the ELF
   entry in EL1h with EL1 MMU and caches initially disabled. `vm_run()` waits
   on the VMM-local host endpoint for guest exits.
9. PL011 data-register writes are line-buffered and logged as
   `[INFO] vmm[0]-guest: ...`. Once the exact stream token
   `Hello from Unikraft!` is seen, the VMM emits `GUEST_STARTED`.

## Terminal shutdown sequence

1. The guest invokes PSCI `SYSTEM_OFF` by SMC after printing the exact hello.
2. The VMM rejects `SYSTEM_OFF` before the hello as a terminal failure. After
   the hello, it suspends the vCPU, marks the terminal state, and sends
   `GUEST_STOPPED` to root.
3. The VMM then blocks on its local host endpoint. It deliberately does not
   return through libsel4vm's SMC fault-reply path, and it never returns from
   its process entry point.
4. Root accepts `GUEST_STOPPED` only after the complete ordered lifecycle
   `READY → LOADING → BOOTING → STARTED → STOPPED`. It prints exactly one
   `UNDERLORD_PHASE2_RESULT: PASS` marker and then blocks on its endpoint.

The final blocking state is intentional. The test harness recognizes the one
PASS marker and terminates QEMU; neither the VMM nor root task exits into a C
runtime continuation. Any malformed IPC, unexpected guest exit, unsupported
MMIO access, failed construction, timeout, or terminal event before hello is a
failure and must not produce PASS.
