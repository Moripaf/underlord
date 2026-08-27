#pragma once

/***
 * @file vmm_image.h
 * Pure validation rules for embedded Phase-2 VMM and guest images.
 */

#include <stddef.h>

/***
 * @define VMM_IMAGE_NAME
 * Stable non-empty CPIO entry name expected by the process loader.
 */
#define VMM_IMAGE_NAME "my-vmm"
#define VMM_GUEST_IMAGE_NAME "c-hello"
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
