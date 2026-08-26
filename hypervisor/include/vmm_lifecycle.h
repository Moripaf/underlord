#pragma once

/***
 * @file vmm_lifecycle.h
 * Pure lifecycle state machine for a single VMM instance.
 */

/***
 * @enum vmm_lifecycle_state_t
 * States visible to the manager; faulted is terminal in Phase 1.
 */
typedef enum {
    VMM_CREATED,
    VMM_STARTING,
    VMM_RUNNING,
    VMM_FAULTED,
} vmm_lifecycle_state_t;

/***
 * @enum vmm_lifecycle_event_t
 * Events accepted by vmm_lifecycle_transition().
 */
typedef enum {
    VMM_EVENT_START,
    VMM_EVENT_READY,
    VMM_EVENT_FAILURE,
    VMM_EVENT_FAULT,
} vmm_lifecycle_event_t;

/***
 * @function vmm_lifecycle_transition(state, event)
 * Apply one legal lifecycle event to a VMM state.
 * @param {vmm_lifecycle_state_t *} state Writable current state; must not be NULL.
 * @param {vmm_lifecycle_event_t} event Event to apply.
 * @pre state contains a defined lifecycle value.
 * @return 0 after a legal transition; -1 for NULL or an illegal transition.
 * @sideeffect Replaces *state only after a legal transition.
 * @error Illegal events preserve the previous state.
 */
int vmm_lifecycle_transition(vmm_lifecycle_state_t *state,
                             vmm_lifecycle_event_t event);
