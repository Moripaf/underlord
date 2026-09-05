#pragma once

/***
 * @file vmm_protocol.h
 * seL4 binding for the Phase-2 VMM capability manifest.
 */

#include <sel4/sel4.h>
#include <sel4utils/process.h>

#include <vmm_protocol_core.h>

/***
 * @define VMM_CONTROL_ENDPOINT_SLOT
 * Child supervisor-event endpoint slot.
 * sel4utils owns slots 1-7: CNode, fault endpoint, VSpace root, ASID pool,
 * TCB, and (where applicable) scheduling/reply objects. The hypervisor then
 * installs the badged supervisor-event endpoint at slot 8, its bounded
 * construction untyped at slot 9, and the GICv2 virtual CPU interface frame
 * at slot 10. The physical PL011 frame is retained by root; guest UART
 * accesses are trapped and emulated, and VMM-local allocation starts at 11.
 * @pre The child process was configured through sel4utils.
 * @sideeffect Defines the fixed slot used by VMM control IPC.
 * @error Using another slot breaks the capability-manifest contract.
 */
#define VMM_CONTROL_ENDPOINT_SLOT ((seL4_CPtr)SEL4UTILS_FIRST_FREE)
#define VMM_DELEGATED_UNTYPED_SLOT ((seL4_CPtr)(SEL4UTILS_FIRST_FREE + 1))
#define VMM_GIC_VCPU_INTERFACE_SLOT ((seL4_CPtr)(SEL4UTILS_FIRST_FREE + 2))
#define VMM_FIRST_LOCAL_SLOT ((seL4_CPtr)(SEL4UTILS_FIRST_FREE + 3))
#define VMM_SUPERVISOR_EVENT_BADGE ((seL4_Word)0x554c0001U)
