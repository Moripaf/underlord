#pragma once

#include <allocman/allocman.h>
#include <sel4utils/vspace.h>
#include <vka/vka.h>

#include <vmm_guest_contract.h>

/***
 * @file vmm_resources.h
 * VMM-local allocator and virtual-address-space bootstrap over the fixed
 * Phase-2 capability manifest.
 */

/***
 * @struct vmm_resources
 * VMM-owned construction services derived exclusively from the delegated
 * non-device untyped capability.
 *
 * @param allocman Owns CSpace and untyped allocation state.
 * @param vka Allocates kernel objects using allocman.
 * @param vspace Allocates mappings in the VMM address space.
 * @param vspace_data Backing state for @ref vspace.
 * @param guest_ram_arena Dedicated 128 MiB untyped retained for guest frames.
 */
typedef struct vmm_resources {
    allocman_t *allocman;
    vka_t vka;
    vspace_t vspace;
    sel4utils_alloc_data_t vspace_data;
    vka_object_t guest_ram_arena;
} vmm_resources_t;

/***
 * @function vmm_resources_bootstrap(resources, manifest)
 * Bootstrap local allocation and VSpace services using only manifest slots.
 *
 * @param resources Receives the usable allocator, VKA, and VSpace services.
 * @param manifest Valid version-2 root manifest with delegated-untyped metadata.
 * @pre Slots 1, 3, and 9 contain the VMM CNode, VSpace root, and delegated
 * construction untyped respectively.
 * @return Zero on success; a negative implementation-defined error otherwise.
 * @sideeffect Consumes internal bookkeeping objects and reserves the VMM's
 * image and allocator virtual-pool address ranges.
 */
int vmm_resources_bootstrap(vmm_resources_t *resources,
                            const vmm_shared_image_descriptor_t *manifest);
