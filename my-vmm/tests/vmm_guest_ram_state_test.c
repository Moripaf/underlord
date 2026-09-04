#include <assert.h>

#include <vmm_guest_ram_state.h>

int main(void)
{
    vmm_guest_ram_state_t state;
    size_t index;

    assert(vmm_guest_ram_state_init(&state, UINT64_C(0x40000000),
                                    UINT64_C(128) * 1024 * 1024) == 0);
    assert(state.pages == 32768);
    assert(vmm_guest_ram_state_next(&state, UINT64_C(0x40000001)) == -1);
    assert(vmm_guest_ram_state_next(&state, UINT64_C(0x40001000)) == -1);
    for (index = 0; index < state.pages; index++) {
        assert(vmm_guest_ram_state_next(&state, state.base + index * 4096) == 0);
        assert(vmm_guest_ram_state_commit(&state) == 0);
    }
    assert(vmm_guest_ram_state_next(&state, state.base) == -1);
    assert(vmm_guest_ram_state_commit(&state) == -1);
    assert(vmm_guest_ram_state_fail(&state, -7) == -7);
    assert(vmm_guest_ram_state_fail(&state, -8) == -7);
    return 0;
}
