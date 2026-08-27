#include <allocman/bootstrap.h>
#include <allocman/vka.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <simple-default/simple-default.h>
#include <underlord/hlog.h>

#include "root_bootstrap.h"
#include "vmm_resource.h"

#define ALLOCATOR_STATIC_POOL_SIZE (1024 * 1024)
#define ALLOCATOR_VIRTUAL_POOL_SIZE (4 * 1024 * 1024)

static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

static simple_t *root_simple;
static int reserved_untyped_index = -1;

static int filtered_untyped_count(void *data)
{
    (void)data;
    return simple_get_untyped_count(root_simple) - 1;
}

static seL4_CPtr filtered_nth_untyped(void *data, int index, size_t *size_bits,
                                      uintptr_t *paddr, bool *device)
{
    (void)data;
    if (index >= reserved_untyped_index) {
        index++;
    }
    return simple_get_nth_untyped(root_simple, index, size_bits, paddr, device);
}

static int select_vmm_untyped(hypervisor_context_t *context)
{
    int count = simple_get_untyped_count(&context->simple);
    size_t selected_bits = 0;
    size_t largest_bits = 0;
    vmm_untyped_candidate_t candidates[CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS];

    if (count < 0 || count > CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS) return -1;

    for (int index = 0; index < count; index++) {
        size_t bits;
        uintptr_t paddr;
        bool device;
        seL4_CPtr cap = simple_get_nth_untyped(&context->simple, index, &bits, &paddr, &device);
        if (!device && bits > largest_bits) largest_bits = bits;
        candidates[index] = (vmm_untyped_candidate_t){.size_bits = bits, .device = device};
        if (cap == seL4_CapNull) candidates[index].device = 1;
    }
    reserved_untyped_index = vmm_select_untyped(candidates, (size_t)count, 28);
    if (reserved_untyped_index < 0) {
        underlord_hlog_error("VMM requires >= 28-bit untyped; largest non-device boot untyped is %lu bits",
                             (unsigned long)largest_bits);
        return -1;
    }
    context->vmm_untyped = simple_get_nth_untyped(&context->simple, reserved_untyped_index,
                                                   &selected_bits, NULL, NULL);
    context->vmm_untyped_size_bits = selected_bits;
    return 0;
}

int hypervisor_bootstrap(hypervisor_context_t *context)
{
    seL4_BootInfo *bootinfo = platsupport_get_bootinfo();
    if (bootinfo == NULL) {
        underlord_hlog_error("bootinfo unavailable");
        return -1;
    }

    simple_default_init_bootinfo(&context->simple, bootinfo);
    if (select_vmm_untyped(context) != 0) {
        return -1;
    }
    root_simple = &context->simple;
    simple_t filtered_simple = context->simple;
    filtered_simple.untyped_count = filtered_untyped_count;
    filtered_simple.nth_untyped = filtered_nth_untyped;
    context->allocman = bootstrap_use_current_simple(
        &filtered_simple, sizeof(allocator_mem_pool), allocator_mem_pool);
    if (context->allocman == NULL) {
        underlord_hlog_error("allocator bootstrap failed");
        return -1;
    }

    allocman_make_vka(&context->vka, context->allocman);
    if (sel4utils_bootstrap_vspace_with_bootinfo_leaky(
            &context->vspace, &context->vspace_data,
            simple_get_pd(&context->simple), &context->vka, bootinfo) != 0) {
        underlord_hlog_error("root vspace bootstrap failed");
        return -1;
    }

    void *virtual_pool = NULL;
    reservation_t reservation = vspace_reserve_range(
        &context->vspace, ALLOCATOR_VIRTUAL_POOL_SIZE, seL4_AllRights, 1,
        &virtual_pool);
    if (reservation.res == NULL) {
        underlord_hlog_error("allocator virtual pool reservation failed");
        return -1;
    }
    bootstrap_configure_virtual_pool(context->allocman, virtual_pool,
                                     ALLOCATOR_VIRTUAL_POOL_SIZE,
                                     simple_get_pd(&context->simple));
    underlord_hlog_info("root allocator ready");
    return 0;
}
