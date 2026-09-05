#include <virtio_mmio_model.h>
#include <stdint.h>

int virtio_mmio_model_init(virtio_mmio_model_t *model)
{
    if (model == 0) return -1;
    *model = (virtio_mmio_model_t){.device_id = VIRTIO_MMIO_DEVICE_ID_9P};
    return 0;
}

int virtio_mmio_model_negotiate(virtio_mmio_model_t *model, uint64_t features)
{
    const uint64_t supported = VIRTIO_MMIO_F_VERSION_1 | VIRTIO_MMIO_F_9P_MOUNT_TAG;
    if (model == 0 || (features & supported) != supported || (features & ~supported) != 0)
        return -1;
    model->features = features;
    return 0;
}

int virtio_mmio_model_queue(virtio_mmio_model_t *model, uint16_t size, int ready)
{
    if (model == 0 || model->features !=
            (VIRTIO_MMIO_F_VERSION_1 | VIRTIO_MMIO_F_9P_MOUNT_TAG) ||
        size == 0 || size > VIRTIO_MMIO_MAX_QUEUE || (ready != 0 && ready != 1))
        return -1;
    model->queue_size = size;
    model->queue_ready = (uint8_t)ready;
    return 0;
}

int virtio_mmio_model_read(const virtio_mmio_model_t *model, uint32_t offset,
                           uint32_t *value)
{
    if (model == 0 || value == 0) return -1;
    switch (offset) {
    case VIRTIO_MMIO_STATUS: *value = model->status; return 0;
    case VIRTIO_MMIO_DEVICE_FEATURES: *value = (uint32_t)(VIRTIO_MMIO_F_VERSION_1 |
        VIRTIO_MMIO_F_9P_MOUNT_TAG); return 0;
    case VIRTIO_MMIO_QUEUE_NUM_MAX: *value = VIRTIO_MMIO_MAX_QUEUE; return 0;
    case VIRTIO_MMIO_QUEUE_NUM: *value = model->queue_size; return 0;
    case VIRTIO_MMIO_QUEUE_READY: *value = model->queue_ready; return 0;
    default: return -1;
    }
}

int virtio_mmio_model_write(virtio_mmio_model_t *model, uint32_t offset,
                            uint32_t value)
{
    if (model == 0) return -1;
    switch (offset) {
    case VIRTIO_MMIO_STATUS:
        if ((value & 0xffU) == 0) return virtio_mmio_model_init(model);
        model->status = (uint8_t)value;
        return 0;
    case VIRTIO_MMIO_DRIVER_FEATURES:
        return virtio_mmio_model_negotiate(model, value);
    case VIRTIO_MMIO_QUEUE_NUM:
        if (value > UINT16_MAX) return -1;
        model->queue_size = (uint16_t)value;
        return model->queue_size <= VIRTIO_MMIO_MAX_QUEUE && model->queue_size != 0 ? 0 : -1;
    case VIRTIO_MMIO_QUEUE_READY:
        if (value > 1 || model->queue_size == 0) return -1;
        model->queue_ready = (uint8_t)value;
        return 0;
    default: return -1;
    }
}

int virtio_mmio_queue_range_valid(uint64_t address, uint64_t bytes,
                                  uint64_t ram_base, uint64_t ram_size)
{
    uint64_t ram_end;
    uint64_t end;
    if (bytes == 0 || (address & 0xfffU) != 0 || ram_base > UINT64_MAX - ram_size ||
        address < ram_base || address > UINT64_MAX - bytes)
        return -1;
    ram_end = ram_base + ram_size;
    end = address + bytes;
    return end <= ram_end ? 0 : -1;
}
