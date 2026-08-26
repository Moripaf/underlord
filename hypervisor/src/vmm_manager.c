#include <cpio/cpio.h>
#include <sel4/sel4.h>
#include <sel4utils/process.h>
#include <underlord/hlog.h>
#include <vka/object.h>

#include "vmm_manager.h"

#define VMM_IMAGE_NAME "my-vmm"

extern char _cpio_archive[];
extern char _cpio_archive_end[];

static int verify_vmm_image(void) {
  unsigned long size = 0;
  const unsigned long archive_size = _cpio_archive_end - _cpio_archive;
  if (cpio_get_file(_cpio_archive, archive_size, VMM_IMAGE_NAME, &size) ==
          NULL ||
      size == 0) {
    underlord_hlog_error("CPIO image '%s' missing", VMM_IMAGE_NAME);
    return -1;
  }
  return 0;
}

int vmm_instance_start(hypervisor_context_t *context,
                       vmm_instance_t *instance) {
  if (verify_vmm_image() != 0) {
    return -1;
  }

  sel4utils_process_config_t config = process_config_default_simple(
      &context->simple, VMM_IMAGE_NAME, seL4_MaxPrio);
  instance->state = VMM_STARTING;
  if (sel4utils_configure_process_custom(&instance->process, &context->vka,
                                         &context->vspace, config) != 0) {
    underlord_hlog_error("VMM instance %u configuration failed", instance->id);
    return -1;
  }
  instance->fault_endpoint = instance->process.fault_endpoint;

  if (vka_alloc_endpoint(&context->vka, &instance->control_endpoint) != 0) {
    underlord_hlog_error("VMM instance %u control endpoint allocation failed",
                         instance->id);
    return -1;
  }
  seL4_CPtr child_slot = sel4utils_copy_cap_to_process(
      &instance->process, &context->vka, instance->control_endpoint.cptr);
  if (child_slot != SEL4UTILS_FIRST_FREE) {
    underlord_hlog_error("VMM instance %u control capability install failed",
                         instance->id);
    return -1;
  }

  underlord_hlog_info("VMM instance %u created", instance->id);
  if (sel4utils_spawn_process_v(&instance->process, &context->vka,
                                &context->vspace, 0, NULL, 1) != 0) {
    underlord_hlog_error("VMM instance %u start failed", instance->id);
    return -1;
  }
  instance->state = VMM_RUNNING;
  underlord_hlog_info("VMM instance %u started", instance->id);
  return 0;
}

void vmm_instance_supervise(vmm_instance_t *instance) {
  seL4_Word badge = 0;
  seL4_Wait(instance->fault_endpoint.cptr, &badge);
  instance->state = VMM_FAULTED;
  underlord_hlog_error("VMM instance %u faulted (badge=%lu, label=%lu)",
                       instance->id, (unsigned long)badge,
                       (unsigned long)seL4_GetMR(0));

  /* A Phase 1 fault is terminal. Keep the root task blocked, not exited. */
  for (;;) {
    seL4_Wait(instance->fault_endpoint.cptr, &badge);
  }
}
