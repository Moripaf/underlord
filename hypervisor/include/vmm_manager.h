#pragma once

#include "root_bootstrap.h"
#include "vmm_instance.h"

int vmm_instance_start(hypervisor_context_t *context, vmm_instance_t *instance);
void vmm_instance_supervise(vmm_instance_t *instance);
