# Unikraft AArch64 KVM boot contract

This document records the boot contract implemented by the checked-in
Unikraft . It applies to
`PLAT_KVM`, `ARCH_ARM_64`, and the QEMU virtual-machine-monitor selection.
It is the Phase 2 source of truth. The older
[`unikraft_boot_requirements.md`](unikraft_boot_requirements.md) notes are
about the x86 Multiboot investigation and must not be used for the AArch64
VMM.

The [memory architecture](../memory_architecture.md) owns host backing,
delegated capabilities, and stage-2 RAM allocation; this document owns only
the guest ABI and boot contents.

## Decision for Phase 2

Build one fixed AArch64 QEMU-virt Unikraft image using
`CONFIG_KVM_BOOT_PROTO_QEMU_VIRT`. This is the simplest contract to reproduce
inside Underlord because the DTB is stored in the guest image rather than
passed in a register by a Linux-style loader.

The alternative `CONFIG_KVM_BOOT_PROTO_LXBOOT` is supported on AArch64, but
is a distinct contract. Its raw `.img` output includes a 64-byte Linux arm64
image header and requires the VMM to provide the DTB address in `x0`.
Do not accept both formats in Phase 2: they have different loading and entry
rules.

## Image and guest-physical layout

For `CONFIG_KVM_VMM_QEMU`, `kvm-arm64/image.h` fixes the RAM base at
`0x40000000`. The ARM linker script establishes this layout:

| Guest physical address range              | Contents                       | Requirement                                                                                  |
| ----------------------------------------- | ------------------------------ | -------------------------------------------------------------------------------------------- |
| `0x40000000` through `0x400fffff`         | DTB reservation                | Place a valid FDT at `_dtb`; reserve the full 1 MiB region from the guest allocator.         |
| `0x40100000` (`_base_addr`) onward        | Unikraft ELF loadable sections | Load at the linked virtual/physical addresses; the platform starts with an identity mapping. |
| `_end` onward, within the DTB memory bank | Free guest RAM                 | The DTB's memory range must extend beyond `_end`.                                            |

The loader must inspect the ELF rather than assume a fixed image length:

- Require an AArch64 little-endian ELF whose entry point is
  `_libkvmplat_entry`.
- Load every `PT_LOAD` segment at its linked address, zero each `p_memsz -
  p_filesz` tail, and reject segments outside the assigned guest RAM.
- Preserve the build-populated `.uk_bootinfo` section. `mkbootinfo.py` places
  a valid `ukplat_bootinfo` header and a memory descriptor for every loadable
  segment there; replacing it with zeroed data makes early boot fail.
- Copy the selected DTB to the reserved `_dtb` address. It is runtime input
  even though the reservation is part of the image layout.

The `QEMU_VIRT` output remains an ELF. Only the `LXBOOT` configuration runs
the `mklinux.py` post-processing step that prepends the Linux arm64 header and
produces `.img`.

## Initial vCPU state

Enter the guest at the ELF entry point `_libkvmplat_entry`, executing at
**AArch64 EL1**. This code immediately reads and writes EL1 system registers
(`SCTLR_EL1`, `TTBR0_EL1`, `TCR_EL1`, `MAIR_EL1`, `VBAR_EL1`, and
`TPIDR_EL1`), so EL0 is not sufficient.

For the selected `QEMU_VIRT` contract:

- The incoming general-purpose register values, including `x0`, are not
  consumed as boot inputs. The entry code constructs `x0 = &_dtb` itself
  before calling `ukplat_bootinfo_fdt_setup()`.
- The incoming `sp` is not consumed. Entry installs its own 4 KiB in-image
  bootstrap stack before calling C code.
- The entry code tolerates an initially enabled or disabled EL1 MMU. If it
  finds `SCTLR_EL1.M` set, it cleans and invalidates the image cache range,
  disables the EL1 MMU and data cache, then installs its own page tables.
- The VMM must provide executable, identity-addressable guest RAM at the
  linked addresses before first entry. The entry code accesses the image by
  those addresses while setting up its translation regime.
- Unikraft then installs its own `TTBR0_EL1`, invalidates EL1 TLB entries,
  configures `MAIR_EL1` and `TCR_EL1`, enables the EL1 MMU/cache, sets its
  exception vector, and continues to C startup. There is no requirement to
  prebuild Unikraft's stage-1 page tables in the VMM.

The source does not use the x86 concepts of long mode, `RIP`, `RSP`, or a
Multiboot information block on this path. Do not model them in the AArch64
vCPU-state API.

## DTB contract

`ukplat_bootinfo_fdt_setup()` validates the FDT and derives runtime boot
information from it. At minimum, the DTB must provide:

- a valid FDT header;
- a `memory` node, with `device_type = "memory"` and a `reg` property
  describing one RAM bank that contains the complete image range
  `[_base_addr, _end)`; the parser uses the first matching node;
- address and size cell counts accepted by libfdt; in practice the current
  KVM parser reads the first 64-bit base and size pair and explicitly supports
  only one bank; and
- nodes for every enabled platform device needed during boot.

The parser marks the DTB read-only in bootinfo, imports optional
`/chosen/bootargs`, and imports optional `linux,initrd-start` /
`linux,initrd-end`. It creates free-memory descriptors only below the image
and after `_end`; it does not make the image or DTB allocatable.

For the current QEMU configuration, Unikraft selects FDT/OFW support,
QEMU's GICv2 driver, and—when the console is enabled—the PL011 early console.
The DTB therefore needs a compatible GIC description before
`uk_intctlr_probe()` can succeed. A serial hello world additionally needs a
PL011-compatible UART and `/chosen/stdout-path`; that UART is a minimum
bootstrap platform device, not virtio.

## Linux arm64 boot alternative

If Phase 2 later selects `CONFIG_KVM_BOOT_PROTO_LXBOOT`, use the generated
`.img`, not the ELF loading contract above. `mklinux.py` writes the standard
64-byte arm64 header, sets its branch to `_libkvmplat_entry`, declares a
4 KiB-granule little-endian image, and uses a load offset based on
`_base_addr - _start_ram_addr - 64`.

On that path, entry preserves incoming `x0` and passes it as the DTB pointer
to `ukplat_bootinfo_fdt_setup()`. Its own comment requires the EL1 MMU to be
disabled and the image cache range clean; it contains recovery logic for an
enabled MMU, but Underlord should satisfy the documented clean start state
rather than rely on that recovery path.

## VMM responsibilities and non-responsibilities

| VMM responsibility                                                                               | Not required for the initial hello-world guest                               |
| ------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------- |
| Allocate and map the fixed RAM bank into the guest through stage 2.                              | A guest page allocator or a general guest memory-request protocol.           |
| Load/zero/validate the ELF segments and preserve the bootinfo section.                           | Supplying a Multiboot structure or fabricating `ukplat_bootinfo` at runtime. |
| Provide the DTB at `0x40000000`, the entry PC, EL1 access, and required initial device mappings. | Building the guest's EL1 stage-1 tables; Unikraft does this itself.          |
| Provide the GIC and PL011 only when they are enabled in the selected Unikraft configuration.     | Virtio block/network devices or SMP for the one-vCPU hello-world milestone.  |

## Source evidence

- `plat/kvm/Config.uk`: AArch64 boot-protocol and QEMU configuration choices.
- `plat/kvm/include/kvm-arm64/image.h` and `plat/kvm/arm/link64.lds.S`: RAM
  base, DTB reservation, linked image base, and ELF program headers.
- `plat/kvm/arm/entry64.S` and `plat/kvm/arm/pagetable64.S`: entry register
  usage, cache/MMU handling, bootstrap stack, and EL1 translation setup.
- `plat/kvm/arm/setup.c`: the C startup sequence and mandatory interrupt
  controller probe.
- `plat/common/bootinfo_fdt.c` and `plat/common/bootinfo.c`: FDT parsing and
  bootinfo validity requirements.
- `plat/kvm/Linker.uk`, `plat/common/Makefile.rules`,
  `support/scripts/mklinux.py`, and `support/scripts/mkbootinfo.py`: artifact
  generation and bootinfo population.
