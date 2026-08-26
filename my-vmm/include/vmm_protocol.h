#pragma once

#include <sel4/sel4.h>
#include <sel4utils/process.h>

/*
 * Phase 1 child capability manifest.
 *
 * sel4utils owns slots 1-7: CNode, fault endpoint, VSpace root, ASID pool,
 * TCB, and (where applicable) scheduling/reply objects. The hypervisor then
 * installs exactly one additional capability at slot 8: this control endpoint.
 * No untyped, IRQ, device, I/O, scheduling, VM, or vCPU capabilities are
 * delegated to the VMM in Phase 1.
 */
#define VMM_CONTROL_ENDPOINT_SLOT ((seL4_CPtr)SEL4UTILS_FIRST_FREE)
