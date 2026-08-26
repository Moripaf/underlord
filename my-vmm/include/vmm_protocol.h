#pragma once

/***
 * @file vmm_protocol.h
 * seL4 binding for the Phase-1 VMM control-capability manifest.
 */

#include <sel4/sel4.h>
#include <sel4utils/process.h>

#include <vmm_protocol_core.h>

/***
 * @define VMM_CONTROL_ENDPOINT_SLOT
 * Phase-1 child capability manifest slot.
 * sel4utils owns slots 1-7: CNode, fault endpoint, VSpace root, ASID pool,
 * TCB, and (where applicable) scheduling/reply objects. The hypervisor then
 * installs exactly one additional capability at slot 8: this control endpoint.
 * No untyped, IRQ, device, I/O, scheduling, VM, or vCPU capabilities are
 * delegated to the VMM in Phase 1.
 * @pre The child process was configured through sel4utils.
 * @sideeffect Defines the fixed slot used by VMM control IPC.
 * @error Using another slot breaks the capability-manifest contract.
 */
#define VMM_CONTROL_ENDPOINT_SLOT ((seL4_CPtr)SEL4UTILS_FIRST_FREE)
