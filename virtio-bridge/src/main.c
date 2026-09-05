#include <sel4/sel4.h>
#include <sel4utils/process.h>

#include <virtio_bridge_protocol.h>

#define VIRTIO_BRIDGE_CONTROL_SLOT ((seL4_CPtr)SEL4UTILS_FIRST_FREE)

/* The transport endpoint is provisioned by the hypervisor in a later
 * integration step. Keep this process alive so capability failures are
 * observable instead of returning through the child runtime. */
int main(void)
{
    seL4_Word badge;
    for (;;) {
        seL4_MessageInfo_t info = seL4_Recv(VIRTIO_BRIDGE_CONTROL_SLOT, &badge);
        (void)badge;
        virtio_bridge_command_t command;
        uint32_t response_word = 0;
        if (seL4_MessageInfo_get_label(info) != 0 ||
            seL4_MessageInfo_get_length(info) != 1 ||
            virtio_bridge_command_decode((uint32_t)seL4_GetMR(0), &command) != 0) {
            seL4_SetMR(0, UINT32_C(0xffffffff));
            seL4_Reply(seL4_MessageInfo_new(0, 0, 0, 1));
            continue;
        }
        /* The outer device is not attached until root provisions its MMIO
         * page and IRQ. Keep GET_INFO truthful while that capability is
         * absent; all transfer requests return an explicit unavailable error. */
        virtio_bridge_request_t request = {
            .version = VIRTIO_BRIDGE_PROTOCOL_VERSION,
            .command = (uint16_t)command,
            .request_length = 0,
            .response_capacity = 0};
        virtio_bridge_response_t response;
        if (command == VIRTIO_BRIDGE_TRANSFER) {
            request.request_length = 1;
            request.response_capacity = VIRTIO_BRIDGE_BUFFER_SIZE;
        }
        if (virtio_bridge_dispatch(&request, &response, 0) != 0) {
            response_word = (uint32_t)(uint16_t)VIRTIO_BRIDGE_STATUS_INVALID;
        } else if (command == VIRTIO_BRIDGE_STOP) {
            seL4_SetMR(0, 0);
            seL4_Reply(seL4_MessageInfo_new(0, 0, 0, 1));
            for (;;) seL4_Wait(VIRTIO_BRIDGE_CONTROL_SLOT, NULL);
        } else {
            response_word = (uint32_t)(uint16_t)response.status;
        }
        seL4_SetMR(0, response_word);
        seL4_Reply(seL4_MessageInfo_new(0, 0, 0, 1));
    }
}
