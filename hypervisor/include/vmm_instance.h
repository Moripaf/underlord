#pragma once

/***
 * @file vmm_instance.h
 * Per-child resource record retained by the hypervisor supervisor.
 */

#include <sel4/sel4.h>
#include <sel4utils/process.h>
#include <vka/object.h>

#include "vmm_lifecycle.h"

/***
 * @struct vmm_instance_t
 * Root-owned record for one VMM and its lifecycle state.
 * @param {unsigned int} id                Stable instance identifier for logs and policy.
 * @param {sel4utils_process_t} process    Process-helper state after configuration.
 * @param {vka_object_t} control_endpoint  Root-held endpoint copied into the child.
 * @param {vka_object_t} fault_endpoint    Root-held endpoint receiving child faults.
 * @param {vmm_lifecycle_state_t} state    Current lifecycle state.
 * @sideeffect The manager populates process and endpoints during start.
 */
typedef struct {
    unsigned int id;
    sel4utils_process_t process;
    vka_object_t control_endpoint;
    vka_object_t fault_endpoint;
    vmm_lifecycle_state_t state;
} vmm_instance_t;
