#pragma once

/***
 * @file root_bootstrap.h
 * Root-task bootstrap interface and the authority it owns after setup.
 */

#include <allocman/allocman.h>
#include <sel4utils/vspace.h>
#include <simple/simple.h>
#include <vka/vka.h>
#include <vspace/vspace.h>

/***
 * @struct hypervisor_context_t
 * Root-task services used to create VMM processes. The context owns its
 * allocator and VSpace state for the lifetime of the hypervisor.
 * @param {simple_t} simple               Initialized boot-info capability view.
 * @param {allocman_t *} allocman         Root-owned allocator, or NULL before bootstrap.
 * @param {vka_t} vka                     Object allocator backed by allocman.
 * @param {vspace_t} vspace               Root virtual-memory allocator.
 * @param {sel4utils_alloc_data_t} vspace_data Backing data for vspace.
 * @param {seL4_CPtr} vmm_untyped Delegated normal-RAM untyped retained by root.
 * @param {size_t} vmm_untyped_size_bits Exact delegated untyped size.
 * @param {uintptr_t} vmm_untyped_paddr Exact delegated untyped physical base.
 */
typedef struct {
    simple_t simple;
    allocman_t *allocman;
    vka_t vka;
    vspace_t vspace;
    sel4utils_alloc_data_t vspace_data;
    seL4_CPtr vmm_untyped;
    size_t vmm_untyped_size_bits;
    uintptr_t vmm_untyped_paddr;
} hypervisor_context_t;

/***
 * @function hypervisor_bootstrap(context)
 * Initialize root-task allocation and virtual-memory services.
 * @param {hypervisor_context_t *} context Writable, uninitialized context storage.
 * @pre Runs once in the root task before creating VMM instances.
 * @return 0 on success; -1 when boot info, allocator, VSpace, or pool setup fails.
 * @sideeffect Initializes context and reserves the root allocator virtual pool.
 * @error Leaves later context members unusable after a failed initialization.
 */
int hypervisor_bootstrap(hypervisor_context_t *context);
