#include <assert.h>

#include <vmm_lifecycle.h>

int main(void)
{
    vmm_lifecycle_state_t state = VMM_CREATED;

    assert(vmm_lifecycle_transition(&state, VMM_EVENT_START) == 0);
    assert(state == VMM_STARTING);
    assert(vmm_lifecycle_transition(&state, VMM_EVENT_READY) == 0);
    assert(state == VMM_RUNNING);
    assert(vmm_lifecycle_transition(&state, VMM_EVENT_FAULT) == 0);
    assert(state == VMM_FAULTED);
    assert(vmm_lifecycle_transition(&state, VMM_EVENT_START) == -1);
    return 0;
}
