#include <virtio_queue_validate.h>
#include <virtio_mmio_model.h>

int virtio_queue_chain_validate(const virtio_queue_desc_t *desc, uint16_t queue_size,
                                uint16_t head, uint64_t ram_base, uint64_t ram_size,
                                uint64_t *total)
{
    uint8_t seen[VIRTIO_MMIO_MAX_QUEUE] = {0};
    uint64_t bytes = 0;
    uint16_t index = head;
    if (desc == 0 || total == 0 || queue_size == 0 || queue_size > VIRTIO_MMIO_MAX_QUEUE ||
        head >= queue_size) return -1;
    for (uint16_t count = 0; count < queue_size; count++) {
        const virtio_queue_desc_t *d;
        if (index >= queue_size || seen[index]) return -1;
        seen[index] = 1;
        d = &desc[index];
        if ((d->flags & VIRTIO_QUEUE_DESC_F_INDIRECT) != 0 || d->length == 0 ||
            virtio_mmio_queue_range_valid(d->address, d->length, ram_base, ram_size) != 0 ||
            bytes > UINT64_MAX - d->length) return -1;
        bytes += d->length;
        if ((d->flags & VIRTIO_QUEUE_DESC_F_NEXT) == 0) { *total = bytes; return 0; }
        index = d->next;
    }
    return -1;
}
