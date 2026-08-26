<!-- markdownlint-disable-file MD013 -->
# What do i need to boot unikraft

I've chosen to replicate multiboot behaviour and kvm functionality that unikraft expects so i can run it with minimum changes to the unikraft code.
I might paravirtualize more aggresively later by modifying the unikraft code to use hypercalls i provide in my vmm

## the state cpu needs to be in

This must match the state just before the asm routine `jump_to_entry` jumps into C:

- 64‑bit long mode
- Paging enabled
- Interrupts disabled
- Valid stack (bootsrap stack, uk will create a boot stack for itself later)
- RSP 16‑byte aligned (SysV ABI)
- RIP = `multiboot_entry` or `_ukplat_entry` functions in c
  - if i can populate `bootinfo` and do the work `multiboot_entry` does i can jump straight to `_ukplat_entry`

## the state memory needs to be in

The multiboot entry marks low memory (0-1MB) as read-only and reserved by BIOS.
It then expects there is usable RAM above that threshold.
It also relocates the kernel memory descriptors in case they aren't where they should be (e.g. messed up by linker)

- Low memory needs to be identity‑mapped

## things i need to pass to uk

### lcpu

there is an lcpu variabe passed to both of my entry options but they are written to and not read from.
the lcpu type is a cpu descriptor

### cmdline

Is a string containing args passed to the kernel like `console=ttyS0 mem=256M ukvm.noirq`
i can pass it inside the multiboot info struct that is received by
`multiboot_entry` if i ended up using it
there is also a bootloader name string that i dont think matters that much so i can just pass whatever to it

### populate bootinfo

I need to populate bootinfo struct with the following information

```c
char bootloader[16];
char bootprotocol[16]; /** Null-terminated boot protocol identifier */
__u64 cmdline;   /** Address of the kernel command line. not necessarily null-terminated.*/
__u64 cmdline_len;
__u64 dtb; /** Address of the devicetree blob still dont know if i need it **optional**/
//memory regions here
```
