#include <stddef.h>

#include <vmm_lifecycle.h>

int vmm_lifecycle_transition(vmm_lifecycle_state_t *state,
                             vmm_lifecycle_event_t event)
{
    if (state == NULL) {
        return -1;
    }
    if (*state == VMM_CREATED && event == VMM_EVENT_START) {
        *state = VMM_STARTING;
        return 0;
    }
    if (*state == VMM_STARTING && event == VMM_EVENT_READY) {
        *state = VMM_RUNNING;
        return 0;
    }
    if (*state == VMM_RUNNING && event == VMM_EVENT_STOP) {
        *state = VMM_STOPPED;
        return 0;
    }
    if ((*state == VMM_STARTING || *state == VMM_RUNNING) &&
        (event == VMM_EVENT_FAILURE || event == VMM_EVENT_FAULT)) {
        *state = VMM_FAULTED;
        return 0;
    }
    return -1;
}
