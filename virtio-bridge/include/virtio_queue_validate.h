#pragma once

/***
 * @file virtio_queue_validate.h
 * Bounded split-virtqueue descriptor-chain validation.
 */

#include <stdint.h>

#define VIRTIO_QUEUE_DESC_F_NEXT 1U
#define VIRTIO_QUEUE_DESC_F_WRITE 2U
#define VIRTIO_QUEUE_DESC_F_INDIRECT 4U

/*** @struct virtio_queue_desc_t
 * Guest-provided split-queue descriptor.
 * @param address Guest physical buffer address.
 * @param length Buffer length.
 * @param flags Descriptor flags.
 * @param next Next descriptor index.
 */
typedef struct { uint64_t address; uint32_t length; uint16_t flags; uint16_t next; }
    virtio_queue_desc_t;

/*** @function virtio_queue_chain_validate(desc, queue_size, head, ram_base, ram_size, total)
 * Validate one non-indirect chain and its guest RAM bounds.
 * @param desc Descriptor table.
 * @param queue_size Number of descriptors in the table.
 * @param head Head descriptor index.
 * @param ram_base Guest RAM base.
 * @param ram_size Guest RAM size.
 * @param total Receives aggregate byte length.
 * @return 0 when valid, -1 otherwise.
 * @sideeffect Writes total only on success.
 */
int virtio_queue_chain_validate(const virtio_queue_desc_t *desc, uint16_t queue_size,
                                uint16_t head, uint64_t ram_base, uint64_t ram_size,
                                uint64_t *total);
