#include <sel4/sel4.h>

#include <underlord/vlog.h>

#include "vmm_protocol.h"
#include "vmm_guest_contract.h"
#include "vmm_resources.h"
#include "vmm_vm.h"
#include "vmm_guest_boot.h"

int main(void)
{
    uint32_t ready_word;
    vmm_guest_state_t guest_state = VMM_GUEST_NONE;
    vmm_resources_t resources;
    vmm_vm_t vm;

    underlord_vlog_info(0, "started");
    const vmm_shared_image_descriptor_t *manifest =
        (const vmm_shared_image_descriptor_t *)(uintptr_t)VMM_SHARED_IMAGE_ADDRESS;

    if (vmm_shared_image_descriptor_valid(manifest,
                                          VMM_SHARED_IMAGE_OFFSET + VMM_GUEST_ELF_MAX_SIZE) != 0) {
        underlord_vlog_error(0, "shared guest-image manifest is invalid");
        return -1;
    }
    int resource_error = vmm_resources_bootstrap(&resources, manifest);
    if (resource_error != 0) {
        underlord_vlog_error(0, "delegated allocator bootstrap failed (%d)", resource_error);
        return -1;
    }
    if (vmm_vm_bootstrap(&vm, &resources) != 0) {
        underlord_vlog_error(0, "VM construction failed");
        return -1;
    }
    if (vmm_protocol_encode(VMM_PROTOCOL_READY, &ready_word) != 0) {
        return -1;
    }
    seL4_SetMR(0, ready_word);
    seL4_Send(VMM_CONTROL_ENDPOINT_SLOT, seL4_MessageInfo_new(0, 0, 0, 1));

    if (vmm_guest_transition(&guest_state, VMM_GUEST_EVENT_LOAD) != 0 ||
        vmm_protocol_encode(VMM_PROTOCOL_GUEST_LOADING, &ready_word) != 0) return -1;
    seL4_SetMR(0, ready_word);
    seL4_Send(VMM_CONTROL_ENDPOINT_SLOT, seL4_MessageInfo_new(0, 0, 0, 1));

    const void *guest_image = (const void *)(uintptr_t)(VMM_SHARED_IMAGE_ADDRESS + manifest->image_offset);
    uint64_t entry;
    if (vmm_guest_boot_load(&vm, guest_image, manifest->image_length, &entry) != 0 ||
        vmm_guest_transition(&guest_state, VMM_GUEST_EVENT_BOOT) != 0) {
        underlord_vlog_error(0, "guest loading failed");
        return -1;
    }
    if (vmm_protocol_encode(VMM_PROTOCOL_GUEST_BOOTING, &ready_word) != 0) return -1;
    seL4_SetMR(0, ready_word);
    seL4_Send(VMM_CONTROL_ENDPOINT_SLOT, seL4_MessageInfo_new(0, 0, 0, 1));
    if (vmm_guest_boot_start(&vm, entry) != 0) {
        underlord_vlog_error(0, "guest vCPU startup failed");
        return -1;
    }
#if VMM_FAULT_TEST
    underlord_vlog_warn(0, "fault test requested");
    *(volatile seL4_Word *)0 = 0;
#endif

    /* vm_run owns the VMM-local endpoint and handles every guest exit. */
    if (vm_run(&vm.vm) != 0 || !vm.guest_stopped) {
        underlord_vlog_error(0, "guest runtime ended unexpectedly");
        return -1;
    }

    /* The guest vCPU is suspended after its accepted PSCI SYSTEM_OFF.  Keep
     * the VMM alive in its terminal state instead of returning to the C
     * runtime, which has no valid continuation for this child task. */
    for (;;) {
        seL4_Wait(vm.host_endpoint.cptr, NULL);
    }
}
