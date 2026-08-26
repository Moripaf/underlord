#pragma once

/***
 * @file vmm_image.h
 * Pure validation rules for the embedded Phase-1 VMM image.
 */

#include <stddef.h>

/***
 * @define VMM_IMAGE_NAME
 * Stable non-empty CPIO entry name expected by the process loader.
 */
#define VMM_IMAGE_NAME "my-vmm"

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
