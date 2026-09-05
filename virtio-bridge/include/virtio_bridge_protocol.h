#pragma once

/***
 * @file virtio_bridge_protocol.h
 * Bounded control messages shared by the hypervisor, VMM, and bridge.
 */

#include <stdint.h>

#define VIRTIO_BRIDGE_PROTOCOL_VERSION 1U
#define VIRTIO_BRIDGE_BUFFER_SIZE (512U * 1024U)
#define VIRTIO_BRIDGE_STATUS_OK 0
#define VIRTIO_BRIDGE_STATUS_INVALID -1
#define VIRTIO_BRIDGE_STATUS_UNAVAILABLE -2

/*** @enum virtio_bridge_command_t
 * Synchronous bridge commands.
 */
typedef enum {
    VIRTIO_BRIDGE_GET_INFO = 1,
    VIRTIO_BRIDGE_TRANSFER = 2,
    VIRTIO_BRIDGE_RESET = 3,
    VIRTIO_BRIDGE_STOP = 4
} virtio_bridge_command_t;

/*** @struct virtio_bridge_request_t
 * Fixed-size command header; payload remains in shared buffers.
 * @param version Protocol version.
 * @param command Command selector.
 * @param request_length Bytes available in the request buffer.
 * @param response_capacity Maximum response bytes accepted.
 */
typedef struct {
    uint16_t version;
    uint16_t command;
    uint32_t request_length;
    uint32_t response_capacity;
} virtio_bridge_request_t;

/*** @struct virtio_bridge_response_t
 * Fixed-size command result.
 * @param version Protocol version.
 * @param status Zero on success, negative on failure.
 * @param response_length Bytes written to the response buffer.
 * @param device_id Outer device identifier, when known.
 */
typedef struct {
    uint16_t version;
    int16_t status;
    uint32_t response_length;
    uint32_t device_id;
} virtio_bridge_response_t;

/*** @function virtio_bridge_request_valid(request)
 * Validate an untrusted bridge command header.
 * @param request Header to validate.
 * @return 0 when valid, -1 otherwise.
 * @sideeffect None.
 */
int virtio_bridge_request_valid(const virtio_bridge_request_t *request);

/*** @function virtio_bridge_transfer_valid(request_length, response_capacity)
 * Validate one bounded transfer without permitting arithmetic wraparound.
 * @param request_length Request bytes in the shared request buffer.
 * @param response_capacity Writable response-buffer capacity.
 * @return 0 when both lengths are within the fixed buffers, -1 otherwise.
 * @sideeffect None.
 */
int virtio_bridge_transfer_valid(uint32_t request_length, uint32_t response_capacity);

/*** @function virtio_bridge_response_valid(response)
 * Validate an untrusted bridge response header.
 * @param response Header to validate.
 * @return 0 when valid, -1 otherwise.
 * @sideeffect None.
 */
int virtio_bridge_response_valid(const virtio_bridge_response_t *response);

/*** @function virtio_bridge_command_encode(command, word)
 * Encode a command for one-word seL4 IPC.
 * @param command Command selector.
 * @param word Receives encoded word.
 * @return 0 when valid, -1 otherwise.
 * @sideeffect Writes word only on success.
 */
int virtio_bridge_command_encode(virtio_bridge_command_t command, uint32_t *word);

/*** @function virtio_bridge_command_decode(word, command)
 * Decode and validate one-word seL4 IPC.
 * @param word Encoded command word.
 * @param command Receives decoded command.
 * @return 0 when valid, -1 otherwise.
 * @sideeffect Writes command only on success.
 */
int virtio_bridge_command_decode(uint32_t word, virtio_bridge_command_t *command);

/*** @function virtio_bridge_dispatch(request, response, device_id)
 * Apply one command header without touching shared payload bytes.
 * @param request Validated command header.
 * @param response Receives the command result.
 * @param device_id Device identifier reported by the outer transport.
 * @return 0 when dispatched, -1 for invalid input.
 * @sideeffect Resets response storage; does not perform I/O.
 */
int virtio_bridge_dispatch(const virtio_bridge_request_t *request,
                           virtio_bridge_response_t *response, uint32_t device_id);
