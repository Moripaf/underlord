# Phase 2: Initial AArch64 VMM and Unikraft Hello World

Phase 2 fixes the machine to one AArch64 QEMU-virt guest: one vCPU on CPU 0,
128 MiB at `0x40000000`, an externally built compatible C or C++ QEMU-virt ELF, and a
runtime FDT. The VMM reports `VMM_READY`, `GUEST_LOADING`, `GUEST_BOOTING`,
`GUEST_STARTED`, then `GUEST_STOPPED`; failure is terminal and reports a typed
stage and signed error.

The root task remains the authority owner. The [Memory architecture](memory_architecture.md)
owns its version-2 manifest, anonymous-RAM allocation, slot 8--11 capability
contract, and address map. Both hello ELFs are packed under stable names and the
selected guest is validated to
be no more than 8 MiB, copied to root-owned frames, and exposed read-only at
VMM VA `0x7000000000`; the descriptor's ELF begins at offset 4096.

The VMM rejects every ELF except ELF64 little-endian `ET_EXEC` `EM_AARCH64`
images whose loadable segments are identity-addressed, non-overlapping, inside
the RAM bank but outside the first 1 MiB DTB reservation, and whose executable
segment contains the entry. A valid `.uk_bootinfo` is mandatory. The platform
is fixed to GICv2, generic timer, PSCI 1.0 SMC `SYSTEM_OFF`, and output-only
PL011 at `0x09000000`; no virtio, SMP, extra guests, restart, or reclamation is
introduced.

The build accepts the two profile ELF/config pairs only when
the sidecar enables AArch64, KVM/QEMU-virt, GICv2, and PL011 early console, and
disables SMP. The [AArch64 boot contract](thinking/unikraft_aarch64_kvm_boot_contract.md)
owns the precise EL1, entry, DTB, and load semantics.
