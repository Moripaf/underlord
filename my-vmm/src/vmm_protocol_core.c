#include <stddef.h>

#include <vmm_protocol_core.h>

int vmm_protocol_encode(vmm_protocol_message_t message, uint32_t *word)
{
    if (word == NULL) {
        return -1;
    }
    if (message == VMM_PROTOCOL_READY) {
        *word = VMM_PROTOCOL_READY_WORD;
        return 0;
    }
    if (message == VMM_PROTOCOL_FAULT) {
        *word = VMM_PROTOCOL_FAULT_WORD;
        return 0;
    }
    return -1;
}

int vmm_protocol_decode(uint32_t word, vmm_protocol_message_t *message)
{
    if (message == NULL) {
        return -1;
    }
    if (word == VMM_PROTOCOL_READY_WORD) {
        *message = VMM_PROTOCOL_READY;
        return 0;
    }
    if (word == VMM_PROTOCOL_FAULT_WORD) {
        *message = VMM_PROTOCOL_FAULT;
        return 0;
    }
    return -1;
}
