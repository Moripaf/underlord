#include <sel4/sel4.h>

#include <underlord/vlog.h>

#include "vmm_protocol.h"

int main(void)
{
    uint32_t ready_word;

    underlord_vlog_info(0, "started");
    if (vmm_protocol_encode(VMM_PROTOCOL_READY, &ready_word) != 0) {
        return -1;
    }
    seL4_SetMR(0, ready_word);
    seL4_Send(VMM_CONTROL_ENDPOINT_SLOT, seL4_MessageInfo_new(0, 0, 0, 1));

#if VMM_FAULT_TEST
    underlord_vlog_warn(0, "fault test requested");
    *(volatile seL4_Word *)0 = 0;
#endif

    /* The hypervisor supplies this endpoint in a fixed manifest slot. */
    seL4_Wait(VMM_CONTROL_ENDPOINT_SLOT, NULL);
    return 0;
}
