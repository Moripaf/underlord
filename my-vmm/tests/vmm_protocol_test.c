#include <assert.h>

#include <vmm_protocol_core.h>

int main(void)
{
    uint32_t word;
    vmm_protocol_message_t message;

    assert(vmm_protocol_encode(VMM_PROTOCOL_READY, &word) == 0);
    assert(vmm_protocol_decode(word, &message) == 0);
    assert(message == VMM_PROTOCOL_READY);
    assert(vmm_protocol_encode((vmm_protocol_message_t)99, &word) == -1);
    assert(vmm_protocol_encode(VMM_PROTOCOL_GUEST_FAILED, &word) == 0);
    assert(vmm_protocol_decode(word, &message) == 0);
    assert(message == VMM_PROTOCOL_GUEST_FAILED);
    assert(vmm_protocol_decode(0, &message) == -1);
    return 0;
}
