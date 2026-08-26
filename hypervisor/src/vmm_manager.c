#include <cpio/cpio.h>
#include <sel4/sel4.h>
#include <sel4utils/process.h>
#include <underlord/hlog.h>
#include <vka/object.h>

#include "vmm_manager.h"
#include <vmm_protocol_core.h>
#include <vmm_image.h>

extern char _cpio_archive[];
extern char _cpio_archive_end[];

static int verify_vmm_image(void) {
  unsigned long size = 0;
  const unsigned long archive_size = _cpio_archive_end - _cpio_archive;
  if (cpio_get_file(_cpio_archive, archive_size, VMM_IMAGE_NAME, &size) ==
          NULL ||
      !vmm_image_metadata_valid(VMM_IMAGE_NAME, (size_t)size)) {
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
  if (vmm_lifecycle_transition(&instance->state, VMM_EVENT_START) != 0) {
    return -1;
  }
  if (sel4utils_configure_process_custom(&instance->process, &context->vka,
                                         &context->vspace, config) != 0) {
    underlord_hlog_error("VMM instance %u configuration failed", instance->id);
    goto failed;
  }
  instance->fault_endpoint = instance->process.fault_endpoint;

  if (vka_alloc_endpoint(&context->vka, &instance->control_endpoint) != 0) {
    underlord_hlog_error("VMM instance %u control endpoint allocation failed",
                         instance->id);
    goto failed;
  }
  seL4_CPtr child_slot = sel4utils_copy_cap_to_process(
      &instance->process, &context->vka, instance->control_endpoint.cptr);
  if (child_slot != SEL4UTILS_FIRST_FREE) {
    underlord_hlog_error("VMM instance %u control capability install failed",
                         instance->id);
    goto failed;
  }

  underlord_hlog_info("VMM instance %u created", instance->id);
  if (sel4utils_spawn_process_v(&instance->process, &context->vka,
                                &context->vspace, 0, NULL, 1) != 0) {
    underlord_hlog_error("VMM instance %u start failed", instance->id);
    goto failed;
  }
  {
    vmm_protocol_message_t message;
    seL4_Word badge = 0;

    seL4_Wait(instance->control_endpoint.cptr, &badge);
    if (badge != 0 || vmm_protocol_decode((uint32_t)seL4_GetMR(0), &message) != 0 ||
        message != VMM_PROTOCOL_READY ||
        vmm_lifecycle_transition(&instance->state, VMM_EVENT_READY) != 0) {
      underlord_hlog_error("VMM instance %u sent an invalid startup message",
                           instance->id);
      goto failed;
    }
  }
  underlord_hlog_info("VMM instance %u started", instance->id);
  return 0;

failed:
  (void)vmm_lifecycle_transition(&instance->state, VMM_EVENT_FAILURE);
  return -1;
}

void vmm_instance_supervise(vmm_instance_t *instance) {
  seL4_Word badge = 0;
  seL4_Wait(instance->fault_endpoint.cptr, &badge);
  (void)vmm_lifecycle_transition(&instance->state, VMM_EVENT_FAULT);
  underlord_hlog_error("VMM instance %u faulted (badge=%lu, label=%lu)",
                       instance->id, (unsigned long)badge,
                       (unsigned long)seL4_GetMR(0));

  /* A Phase 1 fault is terminal. Keep the root task blocked, not exited. */
  for (;;) {
    seL4_Wait(instance->fault_endpoint.cptr, &badge);
  }
}
