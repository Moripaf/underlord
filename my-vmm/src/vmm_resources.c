#include <allocman/allocman.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>
#include <sel4/sel4.h>
#include <sel4utils/vspace.h>
#include <vka/object.h>

#include <vmm_protocol.h>
#include <vmm_resources.h>

#define VMM_CSPACE_SIZE_BITS 16
#define VMM_ALLOCATOR_POOL_SIZE (1024 * 1024)
#define VMM_ALLOCATOR_VIRTUAL_POOL_SIZE (4 * 1024 * 1024)
#define VMM_CNODE_SLOT ((seL4_CPtr)SEL4UTILS_CNODE_SLOT)

static char allocator_pool[VMM_ALLOCATOR_POOL_SIZE];

int vmm_resources_bootstrap(vmm_resources_t *resources)
{
    cspacepath_t untyped_path;
    size_t untyped_bits = 28;
    uintptr_t untyped_paddr = ALLOCMAN_NO_PADDR;
    vka_object_t endpoint;
    void *virtual_pool;
    void *existing_frames[2];
    reservation_t reservation;

    if (resources == NULL) return -1;

    int error = seL4_CNode_Copy(VMM_CNODE_SLOT, VMM_GIC_VCPU_INTERFACE_SLOT + 1, seL4_WordBits,
                                VMM_CNODE_SLOT, VMM_CONTROL_ENDPOINT_SLOT,
                                seL4_WordBits, seL4_AllRights);
    if (error != seL4_NoError) return -10 - error;
    if (seL4_CNode_Delete(VMM_CNODE_SLOT, VMM_GIC_VCPU_INTERFACE_SLOT + 1,
                          seL4_WordBits) != seL4_NoError) return -20;

    resources->allocman = bootstrap_use_current_1level(
        VMM_CNODE_SLOT, VMM_CSPACE_SIZE_BITS, VMM_GIC_VCPU_INTERFACE_SLOT + 1,
        ((seL4_CPtr)1 << VMM_CSPACE_SIZE_BITS), sizeof(allocator_pool), allocator_pool);
    if (resources->allocman == NULL) return -30;
    untyped_path = allocman_cspace_make_path(resources->allocman, VMM_DELEGATED_UNTYPED_SLOT);
    if (allocman_utspace_add_uts(resources->allocman, 1, &untyped_path, &untyped_bits,
                                 &untyped_paddr,
                                 ALLOCMAN_UT_KERNEL) != 0) return -40;

    allocman_make_vka(&resources->vka, resources->allocman);
    existing_frames[0] = (void *)((uintptr_t)seL4_GetIPCBuffer() &
                                  ~((uintptr_t)BIT(seL4_PageBits) - 1));
    existing_frames[1] = NULL;
    if (sel4utils_bootstrap_vspace_leaky(
            &resources->vspace, &resources->vspace_data, SEL4UTILS_PD_SLOT,
            &resources->vka, existing_frames) != 0) return -50;
    reservation = vspace_reserve_range(&resources->vspace,
                                       VMM_ALLOCATOR_VIRTUAL_POOL_SIZE,
                                       seL4_AllRights, 1, &virtual_pool);
    if (reservation.res == NULL) return -60;
    bootstrap_configure_virtual_pool(resources->allocman, virtual_pool,
                                     VMM_ALLOCATOR_VIRTUAL_POOL_SIZE,
                                     SEL4UTILS_PD_SLOT);

    if (vka_alloc_endpoint(&resources->vka, &endpoint) != 0) return -70;
    vka_free_object(&resources->vka, &endpoint);
    return 0;
}
