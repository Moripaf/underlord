#include <sel4/sel4.h>
#include <sel4platsupport/io.h>
#include <sel4utils/process.h>
#include <sel4vm/boot.h>
#include <sel4vm/guest_irq_controller.h>
#include <sel4vm/guest_ram.h>
#include <simple/simple.h>

#include <vmm_vm.h>
#include <vmm_guest_ram.h>
#include <vmm_protocol.h>
#include <underlord/vlog.h>

#define VMM_CNODE_SLOT ((seL4_CPtr)SEL4UTILS_CNODE_SLOT)

typedef struct vmm_vm_environment {
    simple_t simple;
    ps_io_ops_t io_ops;
} vmm_vm_environment_t;

static vmm_vm_environment_t environment;

static seL4_CPtr manifest_init_cap(void *data, seL4_CPtr cap)
{
    (void)data;
    switch (cap) {
    case seL4_CapInitThreadCNode:
        return SEL4UTILS_CNODE_SLOT;
    case seL4_CapInitThreadVSpace:
        return SEL4UTILS_PD_SLOT;
    case seL4_CapInitThreadASIDPool:
        return SEL4UTILS_ASID_POOL_SLOT;
    case seL4_CapInitThreadTCB:
        return SEL4UTILS_TCB_SLOT;
    default:
        return seL4_CapNull;
    }
}

static uint8_t manifest_cnode_size(void *data)
{
    (void)data;
    return 16;
}

static seL4_Error manifest_asid_assign(void *data, seL4_CPtr vspace)
{
    (void)data;
    return seL4_ARCH_ASIDPool_Assign(SEL4UTILS_ASID_POOL_SLOT, vspace);
}

static ssize_t manifest_extended_bootinfo_length(void *data, seL4_Word type)
{
    (void)data;
    (void)type;
    return -1;
}

static ssize_t manifest_extended_bootinfo(void *data, seL4_Word type, void *dest,
                                          ssize_t max_length)
{
    (void)data;
    (void)type;
    (void)dest;
    (void)max_length;
    return -1;
}

static int manifest_untyped_count(void *data)
{
    (void)data;
    return 1;
}

static seL4_CPtr manifest_nth_untyped(void *data, int index, size_t *size_bits,
                                      uintptr_t *paddr, bool *device)
{
    const vmm_shared_image_descriptor_t *manifest = data;

    if (manifest == NULL || index != 0) return seL4_CapNull;
    if (size_bits != NULL) *size_bits = manifest->delegated_untyped_size_bits;
    if (paddr != NULL) *paddr = manifest->delegated_untyped_paddr;
    if (device != NULL) *device = false;
    return VMM_DELEGATED_UNTYPED_SLOT;
}

static seL4_Error manifest_frame_cap(void *data, void *paddr, int size_bits,
                                     cspacepath_t *path)
{
    (void)data;
    if (path == NULL || (uintptr_t)paddr != 0x08040000 || size_bits != seL4_PageBits) {
        return seL4_InvalidArgument;
    }
    return seL4_CNode_Copy(path->root, path->capPtr, path->capDepth,
                           VMM_CNODE_SLOT, VMM_GIC_VCPU_INTERFACE_SLOT,
                           seL4_WordBits, seL4_AllRights);
}

int vmm_vm_bootstrap(vmm_vm_t *vm, vmm_resources_t *resources)
{
    if (vm == NULL || resources == NULL) return -1;
    environment.simple = (simple_t){
        .data = (void *)(uintptr_t)VMM_SHARED_IMAGE_ADDRESS,
        .frame_cap = manifest_frame_cap,
        .init_cap = manifest_init_cap,
        .cnode_size = manifest_cnode_size,
        .untyped_count = manifest_untyped_count,
        .nth_untyped = manifest_nth_untyped,
        .ASID_assign = manifest_asid_assign,
        .extended_bootinfo_len = manifest_extended_bootinfo_length,
        .extended_bootinfo = manifest_extended_bootinfo,
    };
    if (sel4platsupport_new_io_ops(&resources->vspace, &resources->vka,
                                   &environment.simple, &environment.io_ops) != 0) return -2;
    underlord_vlog_info(0, "allocating VM endpoint");
    if (vka_alloc_endpoint(&resources->vka, &vm->host_endpoint) != 0) return -3;
    underlord_vlog_info(0, "constructing VM root");
    if (vm_init(&vm->vm, &resources->vka, &environment.simple, resources->vspace,
                &environment.io_ops, vm->host_endpoint.cptr, "underlord") != 0) return -4;
    underlord_vlog_info(0, "constructing virtual GIC");
    if (vm_create_default_irq_controller(&vm->vm) != 0) return -5;
    underlord_vlog_info(0, "registering arena-backed guest RAM");
    if (vmm_guest_ram_init(&vm->guest_ram, &resources->vka, resources->guest_ram_arena,
                           VMM_GUEST_RAM_BASE, VMM_GUEST_RAM_SIZE) != 0) return -6;
    if (vmm_guest_ram_register(&vm->vm, &vm->guest_ram) != 0) return -7;
    return 0;
}
