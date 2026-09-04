#include <platsupport/io.h>
#include <sel4/sel4.h>
#include <sel4vm/guest_memory.h>

#include <vmm_guest_ram.h>

static memory_fault_result_t arena_ram_fault(vm_t *vm, vm_vcpu_t *vcpu,
                                             uintptr_t address, size_t length, void *cookie)
{
    (void)vm; (void)vcpu; (void)address; (void)length; (void)cookie;
    return FAULT_ERROR;
}

static vm_frame_t arena_frame(uintptr_t gpa, void *cookie)
{
    vmm_guest_ram_t *ram = cookie;
    cspacepath_t path;
    vm_frame_t frame = { seL4_CapNull, seL4_NoRights, 0, 0 };
    int error;

    if (ram == NULL || vmm_guest_ram_state_next(&ram->state, gpa) != 0) return frame;
    error = vka_cspace_alloc_path(ram->vka, &path);
    if (error != 0) {
        vmm_guest_ram_state_fail(&ram->state, error);
        return frame;
    }
    error = seL4_Untyped_Retype(ram->arena.cptr, seL4_ARCH_4KPage, 0,
                                 path.root, path.dest, path.destDepth, path.offset, 1);
    if (error != seL4_NoError) {
        vka_cspace_free_path(ram->vka, path);
        vmm_guest_ram_state_fail(&ram->state, error);
        return frame;
    }
    if (vmm_guest_ram_state_commit(&ram->state) != 0) {
        vmm_guest_ram_state_fail(&ram->state, -1);
        return frame;
    }
    frame.cptr = path.capPtr;
    frame.rights = seL4_AllRights;
    frame.vaddr = gpa;
    frame.size_bits = seL4_PageBits;
    return frame;
}

int vmm_guest_ram_init(vmm_guest_ram_t *ram, vka_t *vka, vka_object_t arena,
                       uintptr_t base, size_t bytes)
{
    if (ram == NULL || vka == NULL || arena.cptr == seL4_CapNull ||
        vmm_guest_ram_state_init(&ram->state, base, bytes) != 0) return -1;
    ram->arena = arena;
    ram->vka = vka;
    ram->registered = 0;
    return 0;
}

int vmm_guest_ram_register(vm_t *vm, vmm_guest_ram_t *ram)
{
    vm_memory_reservation_t *reservation;
    vm_ram_region_t *region;
    int error;

    if (vm == NULL || ram == NULL || ram->registered || vm->mem.num_ram_regions != 0) return -1;
    error = ps_calloc(&vm->io_ops->malloc_ops, 1, sizeof(*region), (void **)&region);
    if (error != 0) return vmm_guest_ram_state_fail(&ram->state, error);
    reservation = vm_reserve_memory_at(vm, ram->state.base,
                                       ram->state.pages << seL4_PageBits,
                                       arena_ram_fault, NULL);
    if (reservation == NULL) return vmm_guest_ram_state_fail(&ram->state, -2);
    error = vm_map_reservation(vm, reservation, arena_frame, ram);
    if (error != 0) return vmm_guest_ram_state_fail(&ram->state, -3);
    if (ram->state.next_page != ram->state.pages) return vmm_guest_ram_state_fail(&ram->state, -4);
    region->start = ram->state.base;
    region->size = ram->state.pages << seL4_PageBits;
    region->allocated = 0;
    vm->mem.ram_regions = region;
    vm->mem.num_ram_regions = 1;
    ram->registered = 1;
    return 0;
}
