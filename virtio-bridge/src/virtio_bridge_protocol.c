#include <stddef.h>
#include <virtio_bridge_protocol.h>

int virtio_bridge_request_valid(const virtio_bridge_request_t *request)
{
    if (request == NULL || request->version != VIRTIO_BRIDGE_PROTOCOL_VERSION)
        return -1;
    if (request->command < VIRTIO_BRIDGE_GET_INFO ||
        request->command > VIRTIO_BRIDGE_STOP)
        return -1;
    if (request->request_length > VIRTIO_BRIDGE_BUFFER_SIZE ||
        request->response_capacity > VIRTIO_BRIDGE_BUFFER_SIZE)
        return -1;
    if (request->command != VIRTIO_BRIDGE_TRANSFER &&
        (request->request_length != 0 || request->response_capacity != 0))
        return -1;
    return 0;
}

int virtio_bridge_transfer_valid(uint32_t request_length, uint32_t response_capacity)
{
    return request_length <= VIRTIO_BRIDGE_BUFFER_SIZE &&
           response_capacity <= VIRTIO_BRIDGE_BUFFER_SIZE ? 0 : -1;
}

int virtio_bridge_response_valid(const virtio_bridge_response_t *response)
{
    if (response == NULL || response->version != VIRTIO_BRIDGE_PROTOCOL_VERSION)
        return -1;
    if (response->response_length > VIRTIO_BRIDGE_BUFFER_SIZE)
        return -1;
    return 0;
}

int virtio_bridge_command_encode(virtio_bridge_command_t command, uint32_t *word)
{
    if (word == 0 || command < VIRTIO_BRIDGE_GET_INFO || command > VIRTIO_BRIDGE_STOP)
        return -1;
    *word = (VIRTIO_BRIDGE_PROTOCOL_VERSION << 16) | (uint32_t)command;
    return 0;
}

int virtio_bridge_command_decode(uint32_t word, virtio_bridge_command_t *command)
{
    uint32_t version = word >> 16;
    uint32_t value = word & UINT32_C(0xffff);
    if (command == 0 || version != VIRTIO_BRIDGE_PROTOCOL_VERSION ||
        value < VIRTIO_BRIDGE_GET_INFO || value > VIRTIO_BRIDGE_STOP)
        return -1;
    *command = (virtio_bridge_command_t)value;
    return 0;
}

int virtio_bridge_dispatch(const virtio_bridge_request_t *request,
                           virtio_bridge_response_t *response, uint32_t device_id)
{
    if (virtio_bridge_request_valid(request) != 0 || response == 0) return -1;
    *response = (virtio_bridge_response_t){
        .version = VIRTIO_BRIDGE_PROTOCOL_VERSION,
        .status = 0,
        .response_length = 0,
        .device_id = device_id};
    if (request->command == VIRTIO_BRIDGE_TRANSFER && device_id == 0)
        response->status = VIRTIO_BRIDGE_STATUS_UNAVAILABLE;
    return 0;
}
