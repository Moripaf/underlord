#pragma once

#include <sel4/sel4.h>
#include <sel4utils/process.h>
#include <vka/object.h>

typedef enum {
    VMM_CREATED,
    VMM_STARTING,
    VMM_RUNNING,
    VMM_FAULTED,
} vmm_lifecycle_state_t;

typedef struct {
    unsigned int id;
    sel4utils_process_t process;
    vka_object_t control_endpoint;
    vka_object_t fault_endpoint;
    vmm_lifecycle_state_t state;
} vmm_instance_t;
