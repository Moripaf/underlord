#pragma once

/***
 * @file vmm_elf.h
 * Strict, target-independent validation for the Phase-2 AArch64 guest ELF.
 */

#include <stddef.h>
#include <stdint.h>

/*** @define VMM_ELF_MAX_LOAD_SEGMENTS
 * Maximum loadable ELF program headers accepted by the fixed Phase-2 loader.
 */
#define VMM_ELF_MAX_LOAD_SEGMENTS 8U

/*** @struct vmm_elf_segment_t
 * One validated PT_LOAD copy and zero-fill operation.
 * @param guest_address Identity-mapped guest physical and virtual destination.
 * @param file_offset Source byte offset in the immutable ELF mapping.
 * @param file_size Bytes copied from the ELF.
 * @param memory_size Total bytes initialized in guest RAM, including BSS.
 */
typedef struct {
    uint64_t guest_address;
    uint64_t file_offset;
    uint64_t file_size;
    uint64_t memory_size;
} vmm_elf_segment_t;

/*** @struct vmm_elf_plan_t
 * Validated ELF loading facts needed by the seL4-facing loader.
 * @param entry AArch64 guest entry point.
 * @param load_count Number of loadable segments.
 * @param loads Bounded copy/zero-fill operations in guest-address order.
 */
typedef struct {
    uint64_t entry;
    size_t load_count;
    vmm_elf_segment_t loads[VMM_ELF_MAX_LOAD_SEGMENTS];
} vmm_elf_plan_t;

/*** @function vmm_elf_validate(image, image_size, plan)
 * Validate an ELF64 little-endian AArch64 ET_EXEC guest image.
 * @param image Complete untrusted ELF byte stream.
 * @param image_size Number of available bytes.
 * @param plan Writable validation result.
 * @return 0 for a loadable Phase-2 image, -1 otherwise.
 * @sideeffect Writes plan only after successful validation.
 */
int vmm_elf_validate(const void *image, size_t image_size, vmm_elf_plan_t *plan);
