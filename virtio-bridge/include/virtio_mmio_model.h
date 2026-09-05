#pragma once

/***
 * @file virtio_mmio_model.h
 * Pure modern virtio-MMIO state and queue admission rules.
 */

#include <stdint.h>

#define VIRTIO_MMIO_DEVICE_ID_9P 9U
#define VIRTIO_MMIO_MAX_QUEUE 128U
#define VIRTIO_MMIO_F_VERSION_1 (UINT64_C(1) << 32)
#define VIRTIO_MMIO_F_9P_MOUNT_TAG (UINT64_C(1) << 0)
#define VIRTIO_MMIO_STATUS 0x070U
#define VIRTIO_MMIO_DEVICE_FEATURES 0x010U
#define VIRTIO_MMIO_DRIVER_FEATURES 0x020U
#define VIRTIO_MMIO_QUEUE_NUM_MAX 0x034U
#define VIRTIO_MMIO_QUEUE_NUM 0x038U
#define VIRTIO_MMIO_QUEUE_READY 0x044U

/*** @struct virtio_mmio_model_t
 * Bounded state needed by one modern virtio-MMIO device.
 * @param status Device status byte.
 * @param features Negotiated feature bits.
 * @param queue_size Configured queue size.
 * @param queue_ready Whether queue zero has been enabled.
 * @param device_id Device identifier exposed to the guest.
 */
typedef struct {
    uint8_t status;
    uint64_t features;
    uint16_t queue_size;
    uint8_t queue_ready;
    uint32_t device_id;
} virtio_mmio_model_t;

/*** @function virtio_mmio_model_init(model)
 * Initialize one 9P modern-MMIO device model.
 * @param model State to initialize.
 * @return 0 on success, -1 for null input.
 * @sideeffect Resets all model state.
 */
int virtio_mmio_model_init(virtio_mmio_model_t *model);

/*** @function virtio_mmio_model_negotiate(model, features)
 * Admit only the bounded feature subset supported by the bridge.
 * @param model Device state.
 * @param features Guest-provided feature selection.
 * @return 0 on success, -1 for invalid or incomplete selection.
 * @sideeffect Stores the negotiated feature bits.
 */
int virtio_mmio_model_negotiate(virtio_mmio_model_t *model, uint64_t features);

/*** @function virtio_mmio_model_queue(model, size, ready)
 * Configure queue zero with a bounded descriptor count.
 * @param model Device state.
 * @param size Requested descriptor count.
 * @param ready Whether the guest enables the queue.
 * @return 0 on success, -1 for invalid state or size.
 * @sideeffect Updates queue state.
 */
int virtio_mmio_model_queue(virtio_mmio_model_t *model, uint16_t size, int ready);

/*** @function virtio_mmio_model_read(model, offset, value)
 * Read a supported 32-bit MMIO register.
 * @param model Device state.
 * @param offset Register byte offset.
 * @param value Receives register value.
 * @return 0 when supported, -1 otherwise.
 * @sideeffect None.
 */
int virtio_mmio_model_read(const virtio_mmio_model_t *model, uint32_t offset,
                           uint32_t *value);

/*** @function virtio_mmio_model_write(model, offset, value)
 * Write a supported 32-bit MMIO register.
 * @param model Device state.
 * @param offset Register byte offset.
 * @param value Register value.
 * @return 0 when accepted, -1 otherwise.
 * @sideeffect Updates status, features, or queue state.
 */
int virtio_mmio_model_write(virtio_mmio_model_t *model, uint32_t offset,
                            uint32_t value);

/*** @function virtio_mmio_queue_range_valid(address, bytes, ram_base, ram_size)
 * Validate a queue or descriptor range against guest RAM.
 * @param address Guest physical start address.
 * @param bytes Range length.
 * @param ram_base Guest RAM base.
 * @param ram_size Guest RAM size.
 * @return 0 when aligned and contained, -1 otherwise.
 * @sideeffect None.
 */
int virtio_mmio_queue_range_valid(uint64_t address, uint64_t bytes,
                                  uint64_t ram_base, uint64_t ram_size);
