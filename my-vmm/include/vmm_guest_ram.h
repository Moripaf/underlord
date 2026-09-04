#pragma once

#include <sel4vm/guest_vm.h>
#include <vka/object.h>

#include <vmm_guest_ram_state.h>

/***
 * @file vmm_guest_ram.h
 * Arena-backed, non-reclaimable 4 KiB guest RAM registration.
 */

/*** @struct vmm_guest_ram_t
 * VMM-lifetime guest RAM arena and sequential mapping state.
 * @param arena Dedicated 128 MiB untyped reservation and allocator cookie.
 * @param vka VMM CSpace allocator used for destination frame slots.
 * @param state Sequential guest-page allocation state.
 * @param registered True after the completed bank is published to libsel4vm.
 */
typedef struct vmm_guest_ram {
    vka_object_t arena;
    vka_t *vka;
    vmm_guest_ram_state_t state;
    int registered;
} vmm_guest_ram_t;

/*** @function vmm_guest_ram_init(ram, vka, arena, base, bytes)
 * Bind a dedicated untyped arena to one fixed guest RAM bank.
 * @param ram Arena state storage.
 * @param vka VMM CSpace allocator.
 * @param arena Root VKA allocation retained for the VMM lifetime.
 * @param base First guest physical address.
 * @param bytes Fixed bank size.
 * @pre arena is a 27-bit ordinary untyped and no frame has been retyped from it.
 * @return Zero on success, -1 for invalid geometry.
 * @sideeffect Copies arena ownership into ram; callers must not free it.
 */
int vmm_guest_ram_init(vmm_guest_ram_t *ram, vka_t *vka, vka_object_t arena,
                       uintptr_t base, size_t bytes);

/*** @function vmm_guest_ram_register(vm, ram)
 * Map every arena frame into the fixed guest RAM bank and publish one region.
 * @param vm Initialized libsel4vm instance.
 * @param ram Dedicated arena state.
 * @pre Deferred libsel4vm memory mapping is disabled and vm has no RAM regions.
 * @return Zero on success; a negative terminal error otherwise.
 * @sideeffect Retypes and maps 4 KiB frames. Failure retains created arena objects and never invokes generic frame teardown.
 */
int vmm_guest_ram_register(vm_t *vm, vmm_guest_ram_t *ram);
