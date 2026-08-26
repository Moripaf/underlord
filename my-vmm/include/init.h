#include <sel4vmmplatsupport/guest_image.h>
#include <stdint.h>

#include <simple-default/simple-default.h>
#include <simple/simple.h>

#include <vka/object.h>

#include "globals.h"
#include <allocman/bootstrap.h>
#include <allocman/vka.h>
#define GUEST_KERNEL_NAME "Unikraft"
#define IMAGE_LOAD_ALIGNMENT 16
#define ALLOCATOR_STATIC_POOL_SIZE (BIT(seL4_PageBits) * 10)
UNUSED static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];
/***
 * @function vm_load_guest_kernel(load_address, guest_kernel_image) Load guest
 * kernel image
 * @param {guest_kernel_image_t *} result           Handle to
 * information regarding the resulted loading of the guest kernel image
 * @return                                                      0 on success,
 * otherwise -1 on error
 */
int load_guest_vmm(uintptr_t load_address, guest_kernel_image_t *result);
/***
 * @function initialize_allocators() initializes base kernel memory and object
 * allocators to default values
 */
int initialize_allocators();
