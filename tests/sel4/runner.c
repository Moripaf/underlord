#include <stdio.h>
#include <string.h>

#include <sel4/sel4.h>

#include <underlord/log_format.h>
#include <vmm_image.h>
#include <vmm_lifecycle.h>
#include <vmm_protocol_core.h>

static int test_utils(uintptr_t environment)
{
    char buffer[64];

    (void)environment;
    return underlord_format_log(buffer, sizeof(buffer), "vmm[0]",
                                UNDERLORD_LOG_INFO, "ready") == 0 &&
           strcmp(buffer, "[INFO] vmm[0]: ready") == 0;
}

static int test_hypervisor(uintptr_t environment)
{
    vmm_lifecycle_state_t state = VMM_CREATED;

    (void)environment;
    return vmm_image_metadata_valid(VMM_IMAGE_NAME, 1) &&
           vmm_lifecycle_transition(&state, VMM_EVENT_START) == 0 &&
           vmm_lifecycle_transition(&state, VMM_EVENT_READY) == 0;
}

static int test_vmm(uintptr_t environment)
{
    uint32_t word;
    vmm_protocol_message_t message;

    (void)environment;
    return vmm_protocol_encode(VMM_PROTOCOL_READY, &word) == 0 &&
           vmm_protocol_decode(word, &message) == 0 &&
           message == VMM_PROTOCOL_READY;
}

int main(void)
{
    int failed = !test_utils(0) || !test_hypervisor(0) || !test_vmm(0);

    printf("UNDERLORD_TEST_RESULT: %s\n", failed ? "FAIL" : "PASS");
    seL4_DebugHalt();
    for (;;) {
        seL4_Yield();
    }
}
