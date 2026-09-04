#pragma once

#include <stddef.h>
#include <stdint.h>

/***
 * @file vmm_guest_ram_state.h
 * Pure sequential-address admission rules for the fixed guest-RAM arena.
 */

/*** @struct vmm_guest_ram_state_t
 * Fixed-bank allocation progress and terminal-error state.
 * @param base First guest physical address in the bank.
 * @param pages Total 4 KiB frames available from the arena.
 * @param next_page Next frame index to allocate.
 * @param terminal_error First permanent construction error, or zero.
 */
typedef struct vmm_guest_ram_state {
    uintptr_t base;
    size_t pages;
    size_t next_page;
    int terminal_error;
} vmm_guest_ram_state_t;

/*** @function vmm_guest_ram_state_init(state, base, bytes)
 * Initialize one page-aligned fixed guest-RAM bank.
 * @param state State storage to initialize.
 * @param base First guest physical address.
 * @param bytes Bank size in bytes, divisible by 4 KiB.
 * @pre state is writable and the bank is fixed for its lifetime.
 * @return Zero on success, -1 for invalid geometry.
 * @sideeffect Clears allocation progress and terminal error.
 */
int vmm_guest_ram_state_init(vmm_guest_ram_state_t *state, uintptr_t base, size_t bytes);

/*** @function vmm_guest_ram_state_next(state, gpa)
 * Admit exactly the next expected page request without advancing it.
 * @param state Arena allocation state.
 * @param gpa Requested guest physical page address.
 * @pre state was successfully initialized.
 * @return Zero only for the next aligned page; -1 otherwise.
 * @sideeffect None.
 */
int vmm_guest_ram_state_next(const vmm_guest_ram_state_t *state, uintptr_t gpa);

/*** @function vmm_guest_ram_state_commit(state)
 * Commit the previously admitted frame allocation.
 * @param state Arena allocation state.
 * @pre The caller has successfully created the next frame.
 * @return Zero on success, -1 when the bank is exhausted or terminal.
 * @sideeffect Advances next_page by one.
 */
int vmm_guest_ram_state_commit(vmm_guest_ram_state_t *state);

/*** @function vmm_guest_ram_state_fail(state, error)
 * Preserve the first terminal construction error.
 * @param state Arena allocation state.
 * @param error Nonzero failure code.
 * @pre state is initialized.
 * @return The preserved terminal error.
 * @sideeffect Sets terminal_error once.
 */
int vmm_guest_ram_state_fail(vmm_guest_ram_state_t *state, int error);
