#pragma once

/***
 * @file vmm_protocol_core.h
 * Architecture-independent encoding for Phase-2 child supervision messages.
 */

#include <stdint.h>

/***
 * @define VMM_PROTOCOL_VERSION
 * Version of the fixed supervision protocol.
 * @sideeffect None.
 */
#define VMM_PROTOCOL_VERSION 2U
#define VMM_PROTOCOL_READY_WORD 0x554C0201U
#define VMM_PROTOCOL_GUEST_LOADING_WORD 0x554C0202U
#define VMM_PROTOCOL_GUEST_BOOTING_WORD 0x554C0203U
#define VMM_PROTOCOL_GUEST_STARTED_WORD 0x554C0204U
#define VMM_PROTOCOL_GUEST_STOPPED_WORD 0x554C0205U
#define VMM_PROTOCOL_GUEST_FAILED_WORD 0x554C0206U

/***
 * @enum vmm_protocol_message_t
 * Ordered status messages a VMM sends to its supervising hypervisor.
 */
typedef enum {
    VMM_PROTOCOL_READY = 1,
    VMM_PROTOCOL_GUEST_LOADING = 2,
    VMM_PROTOCOL_GUEST_BOOTING = 3,
    VMM_PROTOCOL_GUEST_STARTED = 4,
    VMM_PROTOCOL_GUEST_STOPPED = 5,
    VMM_PROTOCOL_GUEST_FAILED = 6,
} vmm_protocol_message_t;

/*** @enum vmm_failure_stage_t
 * Typed construction or runtime failure stage sent with GUEST_FAILED.
 */
typedef enum {
    VMM_FAILURE_RESOURCE_BOOTSTRAP = 1,
    VMM_FAILURE_DESCRIPTOR,
    VMM_FAILURE_ELF,
    VMM_FAILURE_VM,
    VMM_FAILURE_RAM,
    VMM_FAILURE_FDT,
    VMM_FAILURE_INTERRUPT_CONTROLLER,
    VMM_FAILURE_PL011,
    VMM_FAILURE_VCPU,
    VMM_FAILURE_RUNTIME,
} vmm_failure_stage_t;

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
