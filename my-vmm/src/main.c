#include <sel4/sel4.h>

#include <underlord/vlog.h>

#include "vmm_protocol.h"

int main(void)
{
    underlord_vlog(0, UNDERLORD_LOG_INFO, "started");

#if VMM_FAULT_TEST
    underlord_vlog(0, UNDERLORD_LOG_WARN, "fault test requested");
    *(volatile seL4_Word *)0 = 0;
#endif

    /* The hypervisor supplies this endpoint in a fixed manifest slot. */
    seL4_Wait(VMM_CONTROL_ENDPOINT_SLOT, NULL);
    return 0;
}
