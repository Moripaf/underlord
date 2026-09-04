#pragma once

/***
 * @file vmm_guest_contract.h
 * Fixed Phase-2 AArch64 guest layout, lifecycle, and image descriptor API.
 */

#include <stddef.h>
#include <stdint.h>

#define VMM_GUEST_RAM_BASE UINT64_C(0x40000000)
#define VMM_GUEST_RAM_SIZE (UINT64_C(128) * 1024 * 1024)
#define VMM_GUEST_DTB_SIZE (UINT64_C(1) * 1024 * 1024)
#define VMM_GUEST_ELF_MAX_SIZE (UINT64_C(8) * 1024 * 1024)
#define VMM_SHARED_IMAGE_ADDRESS UINT64_C(0x7000000000)
#define VMM_SHARED_IMAGE_MAGIC UINT32_C(0x554c494d)
#define VMM_SHARED_IMAGE_VERSION 2U
#define VMM_SHARED_IMAGE_OFFSET 4096U

/*** @struct vmm_shared_image_descriptor_t
 * First-page descriptor for the root-owned, read-only guest ELF mapping.
 * @param magic Contract magic.
 * @param version Descriptor version.
 * @param header_size Bytes occupied by this descriptor.
 * @param image_offset Page-aligned ELF offset.
 * @param image_length Exact ELF byte length.
 * @param mapped_length Total mapped bytes, including descriptor page.
 * @param delegated_untyped_paddr Physical base of child slot 9.
 * @param delegated_untyped_size_bits Exact size bits of child slot 9.
 * @param vmm_stack_guard_base Base of the root-created child stack guard page.
 * @param vmm_stack_reserved_length Guard and mapped stack byte length.
 */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;
    uint32_t image_offset;
    uint32_t image_length;
    uint64_t mapped_length;
    uint64_t delegated_untyped_paddr;
    uint8_t delegated_untyped_size_bits;
    uint8_t reserved[7];
    uint64_t vmm_stack_guard_base;
    uint64_t vmm_stack_reserved_length;
} vmm_shared_image_descriptor_t;

/*** @enum vmm_guest_state_t
 * Independent Phase-2 guest lifecycle state.
 */
typedef enum {
    VMM_GUEST_NONE,
    VMM_GUEST_LOADING,
    VMM_GUEST_BOOTING,
    VMM_GUEST_RUNNING,
    VMM_GUEST_STOPPED,
    VMM_GUEST_FAILED,
} vmm_guest_state_t;

/*** @enum vmm_guest_event_t
 * Events accepted by the guest lifecycle transition function.
 */
typedef enum {
    VMM_GUEST_EVENT_LOAD,
    VMM_GUEST_EVENT_BOOT,
    VMM_GUEST_EVENT_START,
    VMM_GUEST_EVENT_STOP,
    VMM_GUEST_EVENT_FAIL,
} vmm_guest_event_t;

/*** @function vmm_shared_image_descriptor_valid(descriptor, mapped_bytes)
 * Validate the untrusted shared-image descriptor and its mapping bounds.
 * @param descriptor Descriptor at the mapping base.
 * @param mapped_bytes Bytes mapped into the VMM.
 * @return 0 for a valid descriptor, -1 otherwise.
 * @sideeffect None.
 */
int vmm_shared_image_descriptor_valid(const vmm_shared_image_descriptor_t *descriptor,
                                      size_t mapped_bytes);

/*** @function vmm_guest_transition(state, event)
 * Apply one legal fixed-scope guest lifecycle transition.
 * @param state Current state storage.
 * @param event Requested lifecycle event.
 * @return 0 when applied, -1 for invalid input or transition.
 * @sideeffect Writes state only on success.
 */
int vmm_guest_transition(vmm_guest_state_t *state, vmm_guest_event_t event);
