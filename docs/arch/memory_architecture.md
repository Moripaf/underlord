# Phase-2 memory architecture

This document owns the Phase-2 guest-memory and capability design.  The
system, hypervisor, VMM, boot-contract, phase-plan, and testing documents link
here instead of repeating these rules.

## Decision and research record

The initial isolated-VMM experiment fixed manifest metadata and inherited-VSpace
reservations, then reached RAM registration before failing at GPA `0x44b27000`.
This was not physical RAM exhaustion: the 4 MiB allocman virtual pool was
exhausted with about 120 bytes left, roughly 4,200 CSpace slots remained, and a
free 128 MiB host block existed at `0x78000000`. Per-page untyped splitting
retained roughly two intermediate caps per frame, approaching 98,000 slots for
32,768 guest frames. The selected arena removes that bookkeeping growth.

The CAmkES VM reference has two distinct modes.  Its physical one-to-one mode
uses CapDL-provisioned, address-specific device/untyped memory and registers
that backing one-to-one.  Its ordinary dynamic RAM path calls
`vm_ram_register_at(..., false)`, allocating ordinary host frames through the
component allocator.  Underlord adopts the latter.  Copying the former would
need a different authority and CapDL memory model, and would make the VMM own
physical placement it does not need.

The root remains the authority owner.  It selects one ordinary boot untyped,
records its exact size bits and physical base in a read-only manifest, copies
the cap to the VMM, and retains its original cap.  The VMM uses that metadata
to bootstrap its allocator and allocates anonymous backing frames for guest
RAM.  Guest RAM is never a host-physical one-to-one mapping.

## Address and capability map

| Item | Address or slot | Owner and access |
| --- | --- | --- |
| Read-only manifest and ELF | VMM VA `0x7000000000` | Root owns the source frames and maps them read-only into the VMM. |
| VMM stack and guard | Root-selected VMM VA range | Root maps the child stack; manifest v2 records it so child VSpace bookkeeping reserves it before allocator-pool growth. |
| Guest RAM | GPA `0x40000000` through `0x47ffffff` (128 MiB) | One 27-bit VKA untyped arena is reserved before VMM objects; the local adapter directly retypes sequential 4 KiB frames. |
| Guest EL1 mapping | Identity over the loaded image and runtime FDT | The guest constructs its EL1 tables; stage 2 supplies the identity-addressable GPA backing. |
| Host backing for guest RAM | Allocated dynamically from slot 9's untyped | VMM owns the derived frame caps while running; root retains the original untyped cap. |
| Runtime FDT reservation | GPA `0x40000000` through `0x400fffff` | VMM writes the fixed FDT before vCPU start; guest treats it as boot input. |
| Guest ELF load range | GPA `0x40100000` onward | VMM validates, copies, and zeroes each PT_LOAD segment through `vm_ram_touch`. |
| Supervisor endpoint | Child CSpace slot 8 | Root-created, badged endpoint for lifecycle events only. |
| Delegated normal-RAM untyped | Child CSpace slot 9 | One non-device untyped with manifest size bits and physical base. |
| GICv2 virtual-CPU interface | Child CSpace slot 10 | Root-created frame capability used only by the virtual GIC. |
| PL011 UART frame | Root CSpace only | Root retains the physical device frame; guest GPA `0x09000000..0x09000fff` is reserved for a trapped virtual UART. |

Slots 1--7 remain sel4utils process resources; slot 11 is the first VMM-local
allocation slot.  The child CSpace remains 16-bit.  No other root, IRQ,
device, or I/O authority is delegated.

## Budget and bootstrap order

The VMM accepts only a manifest for an ordinary untyped of at least 28 bits.
That one budget must cover 128 MiB of anonymous guest frames plus the VMM's
VM objects, page tables, CSpace objects, IPC buffer, and local allocator
metadata.  The fixed QEMU machine is configured with 2 GiB because its
rootserver placement otherwise does not expose a contiguous qualifying boot
untyped.  This is a host allocation constraint, not a request for a 2 GiB
guest.

Boot order is deliberately fixed:

1. Root selects the untyped, records the child stack-and-guard range in
   manifest version 2, and installs slots 8--10 before the child starts.
2. VMM validates the manifest and reserves inherited IPC, stack, and shared
   image mappings before reserving its virtual allocator pool.
3. VMM initializes the truthful one-untyped `simple_t`, allocman, and VKA,
   then immediately reserves one 27-bit guest arena before ordinary VM objects.
4. VMM creates libsel4vm and its virtual GIC, maps the arena-backed RAM bank,
   and reserves the single trapped PL011 GPA page without mapping a device.
5. VMM validates and loads the ELF, zeroes BSS tails, writes the runtime FDT,
   and creates one EL1h vCPU on CPU 0.
6. VMM reports lifecycle progress and starts the vCPU.  PSCI `SYSTEM_OFF`
   stops it and sends the terminal stop event.

The complete ordered startup and terminal-shutdown behavior is in the
[boot sequence](boot.md).

## Failure and reporting rules

Arena registration uses public `vm_reserve_memory_at` and `vm_map_reservation`
with deferred mapping disabled. It publishes its sole `vm_ram_region_t` only
after all 32,768 mappings succeed. On failure it becomes terminal, retains all
created arena objects, and never routes them through generic VKA teardown.

## Recorded allocator evidence

The arena is one 27-bit (128 MiB) ordinary untyped, selected by the VKA before
VSpace/VM kernel-object allocation. Its 32,768 4 KiB frames consume one child
CSpace slot each; it creates no per-page untyped split nodes. The preceding
splitter failure was observed with approximately 4,200 free slots and only 120
bytes remaining in the 4 MiB allocman virtual pool, so that pool—not physical
RAM—was the limiting resource.

On 2026-09-04, `CCACHE_DISABLE=1 ninja -C build` followed by
`cd build && timeout 60s ./simulate` with `QEMU_MEMORY=2048` logged
`registering arena-backed guest RAM`, `VMM instance 0 started`, and
`VMM instance 0 accepted guest image`. Those latter two lines are the existing
`READY` and `GUEST_LOADING` acceptance path. The normal image now proceeds
through the trapped UART and PSCI terminal path; its exact run evidence belongs
to the testing strategy.

The pure host test exercises all 32,768 sequential admissions and rejects
misaligned, repeated, out-of-order, and exhausted requests. Separate AArch64
component tests for CSpace-slot accounting, arena ancestry, `vm_ram_touch`, and
forced construction failures remain Phase-2 test work; no production test hook
was added for them.

Every startup or runtime failure emits `GUEST_FAILED` with a failure stage and
a signed error in message registers, then remains terminal.  The root accepts
only `READY -> LOADING -> BOOTING -> STARTED -> STOPPED`; it emits exactly one
`UNDERLORD_PHASE2_RESULT: PASS` only after the clean terminal stop.  A fault,
invalid event, or `GUEST_FAILED` produces no PASS result.

On 2026-09-05, the configured target completed the real lifecycle: it loaded
the ELF/FDT into the arena-backed RAM, printed the captured hello through the
virtual UART, and invoked the accepted PSCI terminal path. The VMM and root
then remain intentionally blocked after the single PASS marker.

## Explicit exclusions

This design does not add physical one-to-one normal RAM, virtio, SMP, extra
guests, restart or reclamation, arbitrary device passthrough, a guest memory
request protocol, or a management API. The physical PL011 frame remains
root-owned and is not delegated; GPA 0x09000000--0x09000fff is reserved for a
trapped, emulated PL011.
