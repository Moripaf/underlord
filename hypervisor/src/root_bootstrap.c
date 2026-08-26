#include <allocman/bootstrap.h>
#include <allocman/vka.h>
#include <sel4/sel4.h>
#include <sel4platsupport/bootinfo.h>
#include <simple-default/simple-default.h>
#include <underlord/hlog.h>

#include "root_bootstrap.h"

#define ALLOCATOR_STATIC_POOL_SIZE (1024 * 1024)
#define ALLOCATOR_VIRTUAL_POOL_SIZE (4 * 1024 * 1024)

static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

int hypervisor_bootstrap(hypervisor_context_t *context)
{
    seL4_BootInfo *bootinfo = platsupport_get_bootinfo();
    if (bootinfo == NULL) {
        underlord_hlog(UNDERLORD_LOG_ERROR, "bootinfo unavailable");
        return -1;
    }

    simple_default_init_bootinfo(&context->simple, bootinfo);
    context->allocman = bootstrap_use_current_simple(
        &context->simple, sizeof(allocator_mem_pool), allocator_mem_pool);
    if (context->allocman == NULL) {
        underlord_hlog(UNDERLORD_LOG_ERROR, "allocator bootstrap failed");
        return -1;
    }

    allocman_make_vka(&context->vka, context->allocman);
    if (sel4utils_bootstrap_vspace_with_bootinfo_leaky(
            &context->vspace, &context->vspace_data,
            simple_get_pd(&context->simple), &context->vka, bootinfo) != 0) {
        underlord_hlog(UNDERLORD_LOG_ERROR, "root vspace bootstrap failed");
        return -1;
    }

    void *virtual_pool = NULL;
    reservation_t reservation = vspace_reserve_range(
        &context->vspace, ALLOCATOR_VIRTUAL_POOL_SIZE, seL4_AllRights, 1,
        &virtual_pool);
    if (reservation.res == NULL) {
        underlord_hlog(UNDERLORD_LOG_ERROR,
                       "allocator virtual pool reservation failed");
        return -1;
    }
    bootstrap_configure_virtual_pool(context->allocman, virtual_pool,
                                     ALLOCATOR_VIRTUAL_POOL_SIZE,
                                     simple_get_pd(&context->simple));
    underlord_hlog(UNDERLORD_LOG_INFO, "root allocator ready");
    return 0;
}
