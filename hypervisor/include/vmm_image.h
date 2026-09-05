#pragma once

/***
 * @file vmm_image.h
 * Pure validation rules for embedded Phase-2 VMM and guest images.
 */

#include <stddef.h>

/*** @struct vmm_guest_image_t
 * Immutable guest-image input passed from the hypervisor to the VMM manager.
 * @param name Stable diagnostic/profile name; may be NULL for external input.
 * @param bytes Complete ELF byte stream.
 * @param size Exact number of bytes in @ref bytes.
 */
typedef struct {
    const char *name;
    const void *bytes;
    size_t size;
} vmm_guest_image_t;

/***
 * @define VMM_IMAGE_NAME
 * Stable non-empty CPIO entry name expected by the process loader.
 */
#define VMM_IMAGE_NAME "my-vmm"
#define VMM_GUEST_IMAGE_MAX_SIZE (8U * 1024U * 1024U)

/***
 * @function vmm_image_metadata_valid(name, size)
 * Validate the name and byte count returned for the configured CPIO entry.
 * @param {const char *} name Candidate archive entry name; must not be NULL.
 * @param {size_t} size Candidate image byte count.
 * @pre No seL4 state is required; this is pure validation.
 * @return Non-zero for VMM_IMAGE_NAME with a non-zero size; otherwise zero.
 * @sideeffect None.
 * @error NULL names and empty images are rejected.
 */
int vmm_image_metadata_valid(const char *name, size_t size);

/*** @function vmm_guest_image_metadata_valid(name, size)
 * Validate the CPIO guest artifact before exposing it to the VMM.
 * @param name Archive entry name.
 * @param size Guest ELF byte count.
 * @return Non-zero only for the fixed guest entry and allowed non-zero size.
 * @sideeffect None.
 */
int vmm_guest_image_metadata_valid(const char *name, size_t size);

/*** @function vmm_guest_image_valid(image)
 * Validate generic immutable guest-image input before mapping it.
 * @param image Candidate image view.
 * @return Non-zero when bytes are present and size is within the fixed limit.
 * @sideeffect None.
 */
int vmm_guest_image_valid(const vmm_guest_image_t *image);

/*** @function vmm_guest_image_arena_size_bits(size)
 * Return the smallest page-aligned power-of-two arena exponent for an image.
 * @param size Descriptor-plus-image mapping length in bytes.
 * @return Size exponent, or zero when size is invalid or exceeds the limit.
 * @sideeffect None.
 */
size_t vmm_guest_image_arena_size_bits(size_t size);
