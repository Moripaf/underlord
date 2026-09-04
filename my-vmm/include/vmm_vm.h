#pragma once

#include <sel4vm/guest_vm.h>
#include <vka/object.h>

#include <vmm_resources.h>
#include <vmm_guest_ram.h>

/***
 * @file vmm_vm.h
 * Fixed Phase-2 libsel4vm construction interface.
 */

/***
 * @struct vmm_vm_t
 * VM construction state retained until the guest stops.
 * @param vm libsel4vm guest object.
 * @param host_endpoint VMM-local endpoint used by libsel4vm fault handling.
 * @param guest_ram Dedicated stage-2 RAM arena retained for VM lifetime.
 */
typedef struct vmm_vm {
    vm_t vm;
    vka_object_t host_endpoint;
    vmm_guest_ram_t guest_ram;
} vmm_vm_t;

/***
 * @function vmm_vm_bootstrap(vm, resources)
 * Construct a libsel4vm instance from the manifest-backed VMM services.
 * @param vm Receives a usable guest VM object.
 * @param resources VMM allocator, VKA, and host VSpace services.
 * @pre The VMM capability manifest and local allocator bootstrap succeeded.
 * @return Zero on success; a negative implementation-defined error otherwise.
 * @sideeffect Allocates a VM CSpace, stage-2 VSpace root, and local endpoint.
 */
int vmm_vm_bootstrap(vmm_vm_t *vm, vmm_resources_t *resources);
