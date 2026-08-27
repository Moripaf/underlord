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
    if (message == VMM_PROTOCOL_GUEST_LOADING) *word = VMM_PROTOCOL_GUEST_LOADING_WORD;
    else if (message == VMM_PROTOCOL_GUEST_BOOTING) *word = VMM_PROTOCOL_GUEST_BOOTING_WORD;
    else if (message == VMM_PROTOCOL_GUEST_STARTED) *word = VMM_PROTOCOL_GUEST_STARTED_WORD;
    else if (message == VMM_PROTOCOL_GUEST_STOPPED) *word = VMM_PROTOCOL_GUEST_STOPPED_WORD;
    else if (message == VMM_PROTOCOL_GUEST_FAILED) *word = VMM_PROTOCOL_GUEST_FAILED_WORD;
    else return -1;
    return 0;
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
    if (word == VMM_PROTOCOL_GUEST_LOADING_WORD) *message = VMM_PROTOCOL_GUEST_LOADING;
    else if (word == VMM_PROTOCOL_GUEST_BOOTING_WORD) *message = VMM_PROTOCOL_GUEST_BOOTING;
    else if (word == VMM_PROTOCOL_GUEST_STARTED_WORD) *message = VMM_PROTOCOL_GUEST_STARTED;
    else if (word == VMM_PROTOCOL_GUEST_STOPPED_WORD) *message = VMM_PROTOCOL_GUEST_STOPPED;
    else if (word == VMM_PROTOCOL_GUEST_FAILED_WORD) *message = VMM_PROTOCOL_GUEST_FAILED;
    else return -1;
    return 0;
}
