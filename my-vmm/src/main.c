#include <sel4/sel4.h>

#include <underlord/vlog.h>

#include "vmm_protocol.h"

int main(void)
{
    underlord_vlog_info(0, "started");

#if VMM_FAULT_TEST
    underlord_vlog_warn(0, "fault test requested");
    *(volatile seL4_Word *)0 = 0;
#endif

    /* The hypervisor supplies this endpoint in a fixed manifest slot. */
    seL4_Wait(VMM_CONTROL_ENDPOINT_SLOT, NULL);
    return 0;
}
