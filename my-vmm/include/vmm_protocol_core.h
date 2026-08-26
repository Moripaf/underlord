#pragma once

/***
 * @file vmm_protocol_core.h
 * Architecture-independent encoding for Phase-1 child control messages.
 */

#include <stdint.h>

/***
 * @define VMM_PROTOCOL_VERSION
 * Version of the fixed one-word control protocol.
 * @sideeffect None.
 */
#define VMM_PROTOCOL_VERSION 1U
#define VMM_PROTOCOL_READY_WORD 0x554C0101U
#define VMM_PROTOCOL_FAULT_WORD 0x554C0102U

/***
 * @enum vmm_protocol_message_t
 * Messages a child can send to its supervising hypervisor.
 */
typedef enum {
    VMM_PROTOCOL_READY = 1,
    VMM_PROTOCOL_FAULT = 2,
} vmm_protocol_message_t;

/***
 * @function vmm_protocol_encode(message, word)
 * Encode a supported protocol message into one IPC word.
 * @param {vmm_protocol_message_t} message READY or FAULT message to encode.
 * @param {uint32_t *} word Writable output word; must not be NULL.
 * @pre message is a defined protocol enum value.
 * @return 0 on success; -1 for NULL output or unsupported message.
 * @sideeffect Writes *word only for a supported message.
 * @error Unsupported messages leave the output unspecified.
 */
int vmm_protocol_encode(vmm_protocol_message_t message, uint32_t *word);
/***
 * @function vmm_protocol_decode(word, message)
 * Decode one received IPC word into a protocol message.
 * @param {uint32_t} word Received message register value.
 * @param {vmm_protocol_message_t *} message Writable result; must not be NULL.
 * @pre No seL4 state is required.
 * @return 0 for a known word; -1 for NULL output or an unknown word.
 * @sideeffect Writes *message only for a known word.
 * @error Unknown words preserve the caller's result storage.
 */
int vmm_protocol_decode(uint32_t word, vmm_protocol_message_t *message);
