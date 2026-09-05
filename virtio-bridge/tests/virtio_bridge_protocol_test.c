#include <assert.h>
#include <virtio_bridge_protocol.h>
#include <virtio_mmio_model.h>
#include <virtio_queue_validate.h>

int main(void)
{
    virtio_bridge_request_t request = {
        VIRTIO_BRIDGE_PROTOCOL_VERSION, VIRTIO_BRIDGE_TRANSFER, 1, 2};
    assert(virtio_bridge_request_valid(&request) == 0);
    request.request_length = VIRTIO_BRIDGE_BUFFER_SIZE + 1;
    assert(virtio_bridge_request_valid(&request) != 0);
    assert(virtio_bridge_transfer_valid(0, VIRTIO_BRIDGE_BUFFER_SIZE) == 0);
    assert(virtio_bridge_transfer_valid(VIRTIO_BRIDGE_BUFFER_SIZE + 1, 0) != 0);
    virtio_bridge_response_t response = {VIRTIO_BRIDGE_PROTOCOL_VERSION, 0, 4, 9};
    assert(virtio_bridge_response_valid(&response) == 0);
    response.response_length = VIRTIO_BRIDGE_BUFFER_SIZE + 1;
    assert(virtio_bridge_response_valid(&response) != 0);
    uint32_t word;
    virtio_bridge_command_t command;
    assert(virtio_bridge_command_encode(VIRTIO_BRIDGE_TRANSFER, &word) == 0);
    assert(virtio_bridge_command_decode(word, &command) == 0 && command == VIRTIO_BRIDGE_TRANSFER);
    assert(virtio_bridge_command_decode(UINT32_C(0x00020001), &command) != 0);
    request.request_length = 1;
    request.response_capacity = 2;
    virtio_bridge_response_t dispatched;
    assert(virtio_bridge_dispatch(&request, &dispatched, 9) == 0 && dispatched.status == 0);
    assert(virtio_bridge_dispatch(&request, &dispatched, 0) == 0 &&
           dispatched.status == VIRTIO_BRIDGE_STATUS_UNAVAILABLE);
    virtio_mmio_model_t model;
    assert(virtio_mmio_model_init(&model) == 0 && model.device_id == 9);
    assert(virtio_mmio_model_negotiate(&model,
        VIRTIO_MMIO_F_VERSION_1 | VIRTIO_MMIO_F_9P_MOUNT_TAG) == 0);
    assert(virtio_mmio_model_queue(&model, 128, 1) == 0);
    assert(virtio_mmio_model_queue(&model, 129, 1) != 0);
    uint32_t value;
    assert(virtio_mmio_model_read(&model, VIRTIO_MMIO_QUEUE_NUM_MAX, &value) == 0 && value == 128);
    assert(virtio_mmio_model_write(&model, VIRTIO_MMIO_STATUS, 0) == 0);
    assert(virtio_mmio_model_read(&model, VIRTIO_MMIO_STATUS, &value) == 0 && value == 0);
    assert(virtio_mmio_queue_range_valid(0x40000000, 4096, 0x40000000, 0x8000000) == 0);
    assert(virtio_mmio_queue_range_valid(0x40000001, 4096, 0x40000000, 0x8000000) != 0);
    virtio_queue_desc_t desc[2] = {{0x40000000, 4096, VIRTIO_QUEUE_DESC_F_NEXT, 1},
                                   {0x40001000, 4096, 0, 0}};
    uint64_t total;
    assert(virtio_queue_chain_validate(desc, 2, 0, 0x40000000, 0x8000000, &total) == 0 && total == 8192);
    desc[1].flags = VIRTIO_QUEUE_DESC_F_INDIRECT;
    assert(virtio_queue_chain_validate(desc, 2, 0, 0x40000000, 0x8000000, &total) != 0);
    return 0;
}
