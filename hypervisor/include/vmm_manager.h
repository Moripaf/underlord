#pragma once

/***
 * @file vmm_manager.h
 * seL4-facing VMM construction and fault-supervision interface.
 */

#include "root_bootstrap.h"
#include "vmm_instance.h"

/***
 * @function vmm_instance_start(context, instance)
 * Configure, grant, spawn, and wait for READY from a created VMM instance.
 * @param {hypervisor_context_t *} context Successfully bootstrapped root context.
 * @param {vmm_instance_t *} instance Created instance record to populate.
 * @pre context is initialized and instance->state is VMM_CREATED.
 * @return 0 after a validated READY message; -1 on image, allocation, cap, spawn, or protocol failure.
 * @sideeffect Allocates root resources, grants the control cap, and advances lifecycle state.
 * @error A failure marks a starting instance faulted; callers must not supervise it.
 */
int vmm_instance_start(hypervisor_context_t *context, vmm_instance_t *instance);
/***
 * @function vmm_instance_supervise(instance)
 * Wait indefinitely for a configured VMM's fault endpoint.
 * @param {vmm_instance_t *} instance Running instance with a valid fault endpoint.
 * @pre instance->state is VMM_RUNNING.
 * @return Does not return during normal Phase-1 operation.
 * @sideeffect Marks the instance faulted and emits a diagnostic on the first fault.
 * @error Invalid input or endpoint configuration is caller error and is not recovered.
 */
void vmm_instance_supervise(vmm_instance_t *instance);
