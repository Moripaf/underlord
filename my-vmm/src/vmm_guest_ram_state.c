#include <vmm_guest_ram_state.h>

#define VMM_GUEST_RAM_PAGE_BYTES ((size_t)4096)

int vmm_guest_ram_state_init(vmm_guest_ram_state_t *state, uintptr_t base, size_t bytes)
{
    if (state == NULL || (base & (VMM_GUEST_RAM_PAGE_BYTES - 1U)) != 0 ||
        bytes == 0 || (bytes & (VMM_GUEST_RAM_PAGE_BYTES - 1U)) != 0) return -1;
    state->base = base;
    state->pages = bytes / VMM_GUEST_RAM_PAGE_BYTES;
    state->next_page = 0;
    state->terminal_error = 0;
    return 0;
}

int vmm_guest_ram_state_next(const vmm_guest_ram_state_t *state, uintptr_t gpa)
{
    if (state == NULL || state->terminal_error != 0 || state->next_page >= state->pages ||
        (gpa & (VMM_GUEST_RAM_PAGE_BYTES - 1U)) != 0) return -1;
    return gpa == state->base + state->next_page * VMM_GUEST_RAM_PAGE_BYTES ? 0 : -1;
}

int vmm_guest_ram_state_commit(vmm_guest_ram_state_t *state)
{
    if (state == NULL || state->terminal_error != 0 || state->next_page >= state->pages) return -1;
    state->next_page++;
    return 0;
}

int vmm_guest_ram_state_fail(vmm_guest_ram_state_t *state, int error)
{
    if (state == NULL) return error == 0 ? -1 : error;
    if (state->terminal_error == 0) state->terminal_error = error == 0 ? -1 : error;
    return state->terminal_error;
}
