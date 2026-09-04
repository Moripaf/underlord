#include <allocman/allocman.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>
#include <limits.h>
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

int vmm_resources_bootstrap(vmm_resources_t *resources,
                            const vmm_shared_image_descriptor_t *manifest)
{
    cspacepath_t untyped_path;
    size_t untyped_bits;
    uintptr_t untyped_paddr;
    vka_object_t endpoint;
    void *virtual_pool;
    void *existing_frames[2];
    reservation_t reservation;
    reservation_t inherited_reservation;

    if (resources == NULL || manifest == NULL ||
        manifest->delegated_untyped_size_bits < 28 ||
        manifest->delegated_untyped_size_bits >= sizeof(uintptr_t) * CHAR_BIT) return -1;
    untyped_bits = manifest->delegated_untyped_size_bits;
    untyped_paddr = manifest->delegated_untyped_paddr;
    if ((untyped_paddr & (((uintptr_t)1 << untyped_bits) - 1U)) != 0) return -2;

    resources->allocman = bootstrap_use_current_1level(
        VMM_CNODE_SLOT, VMM_CSPACE_SIZE_BITS, VMM_FIRST_LOCAL_SLOT,
        ((seL4_CPtr)1 << VMM_CSPACE_SIZE_BITS), sizeof(allocator_pool), allocator_pool);
    if (resources->allocman == NULL) return -30;
    untyped_path = allocman_cspace_make_path(resources->allocman, VMM_DELEGATED_UNTYPED_SLOT);
    if (allocman_utspace_add_uts(resources->allocman, 1, &untyped_path, &untyped_bits,
                                 &untyped_paddr,
                                 ALLOCMAN_UT_KERNEL) != 0) return -40;

    allocman_make_vka(&resources->vka, resources->allocman);
    if (vka_alloc_untyped(&resources->vka, 27, &resources->guest_ram_arena) != 0) return -45;
    existing_frames[0] = (void *)((uintptr_t)seL4_GetIPCBuffer() &
                                  ~((uintptr_t)BIT(seL4_PageBits) - 1));
    existing_frames[1] = NULL;
    if (sel4utils_bootstrap_vspace_leaky(
            &resources->vspace, &resources->vspace_data, SEL4UTILS_PD_SLOT,
            &resources->vka, existing_frames) != 0) return -50;
    inherited_reservation = vspace_reserve_range_at(
        &resources->vspace, (void *)(uintptr_t)manifest->vmm_stack_guard_base,
        manifest->vmm_stack_reserved_length, seL4_AllRights, 1);
    if (inherited_reservation.res == NULL) return -55;
    inherited_reservation = vspace_reserve_range_at(
        &resources->vspace, (void *)(uintptr_t)VMM_SHARED_IMAGE_ADDRESS,
        manifest->mapped_length, seL4_AllRights, 1);
    if (inherited_reservation.res == NULL) return -56;
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
