#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vmm_vm.h>

/*** @file vmm_guest_boot.h
 * ELF/FDT loading and initial vCPU configuration for the fixed Phase-2 guest.
 */

/*** @function vmm_guest_boot_load(vm, image, image_size, entry)
 * Validate and load the supplied ELF and runtime FDT into guest RAM.
 * @param vm Initialized VM with registered RAM.
 * @param image Immutable ELF mapping.
 * @param image_size ELF byte length.
 * @param entry Writable entry result.
 * @return 0 on success, -1 on malformed input or RAM/cache failure.
 * @sideeffect Writes guest RAM and configures vm->vm.entry.
 */
int vmm_guest_boot_load(vmm_vm_t *vm, const void *image, size_t image_size,
                        uint64_t *entry);

/*** @function vmm_guest_boot_start(vm, entry)
 * Create, configure, and start the single Phase-2 vCPU.
 * @param vm Initialized VM.
 * @param entry Valid executable guest entry.
 * @return 0 on success, -1 on construction/configuration failure.
 */
int vmm_guest_boot_start(vmm_vm_t *vm, uint64_t entry);
