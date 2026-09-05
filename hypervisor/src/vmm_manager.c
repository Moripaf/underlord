#include <cpio/cpio.h>
#include <string.h>
#include <stdio.h>
#include <sel4/sel4.h>
#include <sel4utils/process.h>
#include <underlord/hlog.h>
#include <vka/object.h>

#include "vmm_manager.h"
#include <vmm_protocol_core.h>
#include <vmm_protocol.h>
#include <vmm_guest_contract.h>
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

static int verify_guest_image(void) {
  unsigned long size = 0;
  const unsigned long archive_size = _cpio_archive_end - _cpio_archive;
  if (cpio_get_file(_cpio_archive, archive_size, VMM_GUEST_IMAGE_NAME, &size) == NULL ||
      !vmm_guest_image_metadata_valid(VMM_GUEST_IMAGE_NAME, (size_t)size)) {
    underlord_hlog_error("guest image '%s' missing or invalid", VMM_GUEST_IMAGE_NAME);
    return -1;
  }
  return 0;
}

static int map_guest_image(hypervisor_context_t *context, vmm_instance_t *instance)
{
  const unsigned long archive_size = _cpio_archive_end - _cpio_archive;
  unsigned long image_length = 0;
  const void *image = cpio_get_file(_cpio_archive, archive_size, VMM_GUEST_IMAGE_NAME,
                                    &image_length);
  const size_t descriptor_page = VMM_SHARED_IMAGE_OFFSET;
  size_t mapped_length;
  size_t pages;
  size_t stack_bytes;
  uintptr_t stack_base;
  uintptr_t stack_guard_base;
  reservation_t reservation;
  vmm_shared_image_descriptor_t *descriptor;

  if (image == NULL || !vmm_guest_image_metadata_valid(VMM_GUEST_IMAGE_NAME,
                                                        (size_t)image_length) ||
      image_length > SIZE_MAX - descriptor_page) {
    underlord_hlog_error("guest image mapping input is invalid");
    return -1;
  }
  mapped_length = (size_t)image_length + descriptor_page;
  if (mapped_length > SIZE_MAX - (descriptor_page - 1U)) {
    underlord_hlog_error("guest image mapping size overflows");
    return -1;
  }
  mapped_length = (mapped_length + descriptor_page - 1U) & ~(descriptor_page - 1U);
  pages = mapped_length / descriptor_page;
  if (pages == 0 || pages > INT_MAX) {
    underlord_hlog_error("guest image mapping page count is invalid");
    return -1;
  }
  if (instance->process.thread.stack_size == 0 ||
      instance->process.thread.stack_size > SIZE_MAX >> seL4_PageBits) {
    underlord_hlog_error("VMM instance %u stack geometry is invalid", instance->id);
    return -1;
  }
  stack_bytes = instance->process.thread.stack_size << seL4_PageBits;
  if ((uintptr_t)instance->process.thread.stack_top < stack_bytes + BIT(seL4_PageBits)) {
    underlord_hlog_error("VMM instance %u stack address underflows", instance->id);
    return -1;
  }
  stack_base = (uintptr_t)instance->process.thread.stack_top - stack_bytes;
  stack_guard_base = stack_base - BIT(seL4_PageBits);

  instance->guest_image = vspace_new_pages(&context->vspace, seL4_AllRights,
                                            pages, seL4_PageBits);
  if (instance->guest_image == NULL) {
    underlord_hlog_error("guest image root mapping allocation failed");
    return -1;
  }
  memset(instance->guest_image, 0, mapped_length);
  descriptor = instance->guest_image;
  *descriptor = (vmm_shared_image_descriptor_t){
      .magic = VMM_SHARED_IMAGE_MAGIC,
      .version = VMM_SHARED_IMAGE_VERSION,
      .header_size = sizeof(*descriptor),
      .image_offset = VMM_SHARED_IMAGE_OFFSET,
      .image_length = (uint32_t)image_length,
      .mapped_length = mapped_length,
      .delegated_untyped_paddr = context->vmm_untyped_paddr,
      .delegated_untyped_size_bits = (uint8_t)context->vmm_untyped_size_bits,
      .vmm_stack_guard_base = stack_guard_base,
      .vmm_stack_reserved_length = stack_bytes + BIT(seL4_PageBits),
  };
  memcpy((char *)instance->guest_image + descriptor_page, image, (size_t)image_length);

  reservation = vspace_reserve_range_at(&instance->process.vspace,
                                        (void *)(uintptr_t)VMM_SHARED_IMAGE_ADDRESS,
                                        mapped_length,
                                        seL4_CapRights_new(false, false, true, false), 1);
  if (reservation.res == NULL ||
      vspace_share_mem_at_vaddr(&context->vspace, &instance->process.vspace,
                                instance->guest_image, (int)pages, seL4_PageBits,
                                (void *)(uintptr_t)VMM_SHARED_IMAGE_ADDRESS,
                                reservation) != 0) {
    underlord_hlog_error("guest image read-only VMM mapping failed");
    return -1;
  }
  instance->guest_image_length = mapped_length;
  return 0;
}

int vmm_instance_start(hypervisor_context_t *context,
                       vmm_instance_t *instance) {
  if (verify_vmm_image() != 0 || verify_guest_image() != 0) {
    return -1;
  }

  sel4utils_process_config_t config = process_config_default_simple(
      &context->simple, VMM_IMAGE_NAME, seL4_MaxPrio);
  config = process_config_create_cnode(config, 16);
  if (vka_alloc_endpoint(&context->vka, &instance->control_endpoint) != 0) {
    underlord_hlog_error("VMM instance %u supervisor endpoint allocation failed", instance->id);
    return -1;
  }
  config = process_config_fault_endpoint(config, instance->control_endpoint);
  if (vmm_lifecycle_transition(&instance->state, VMM_EVENT_START) != 0) {
    return -1;
  }
  if (sel4utils_configure_process_custom(&instance->process, &context->vka,
                                         &context->vspace, config) != 0) {
    underlord_hlog_error("VMM instance %u configuration failed", instance->id);
    goto failed;
  }
  instance->fault_endpoint = instance->control_endpoint;
  cspacepath_t supervisor_path;
  vka_cspace_make_path(&context->vka, instance->control_endpoint.cptr, &supervisor_path);
  seL4_CPtr child_slot = sel4utils_mint_cap_to_process(
      &instance->process, supervisor_path, seL4_AllRights, VMM_SUPERVISOR_EVENT_BADGE);
  if (child_slot != SEL4UTILS_FIRST_FREE) {
    underlord_hlog_error("VMM instance %u control capability install failed",
                         instance->id);
    goto failed;
  }
  child_slot = sel4utils_copy_cap_to_process(&instance->process, &context->vka,
                                             context->vmm_untyped);
  if (child_slot != VMM_DELEGATED_UNTYPED_SLOT) {
    underlord_hlog_error("VMM instance %u untyped capability install failed", instance->id);
    goto failed;
  }
  if (vka_alloc_frame_at(&context->vka, seL4_PageBits, 0x08040000,
                         &instance->gic_vcpu_interface) != 0) {
    underlord_hlog_error("VMM instance %u GICv2 VCPU interface frame allocation failed", instance->id);
    goto failed;
  }
  child_slot = sel4utils_copy_cap_to_process(&instance->process, &context->vka,
                                             instance->gic_vcpu_interface.cptr);
  if (child_slot != VMM_GIC_VCPU_INTERFACE_SLOT) {
    underlord_hlog_error("VMM instance %u GICv2 VCPU interface capability install failed", instance->id);
    goto failed;
  }
  if (map_guest_image(context, instance) != 0) {
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

    seL4_MessageInfo_t info = seL4_Recv(instance->control_endpoint.cptr, &badge);
    if (badge != VMM_SUPERVISOR_EVENT_BADGE || seL4_MessageInfo_get_label(info) != 0 ||
        seL4_MessageInfo_get_length(info) != 1 ||
        vmm_protocol_decode((uint32_t)seL4_GetMR(0), &message) != 0 ||
        message != VMM_PROTOCOL_READY ||
        vmm_lifecycle_transition(&instance->state, VMM_EVENT_READY) != 0) {
      underlord_hlog_error("VMM instance %u sent an invalid startup message (badge=%lu label=%lu length=%lu mr0=%lu)",
                           instance->id, (unsigned long)badge,
                           (unsigned long)seL4_MessageInfo_get_label(info),
                           (unsigned long)seL4_MessageInfo_get_length(info),
                           (unsigned long)seL4_GetMR(0));
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
  for (;;) {
    seL4_MessageInfo_t info = seL4_Recv(instance->fault_endpoint.cptr, &badge);
    if (badge == VMM_SUPERVISOR_EVENT_BADGE) {
      vmm_protocol_message_t message;
      if (seL4_MessageInfo_get_label(info) != 0 || seL4_MessageInfo_get_length(info) != 1 ||
          vmm_protocol_decode((uint32_t)seL4_GetMR(0), &message) != 0) goto invalid_event;
      if (message == VMM_PROTOCOL_GUEST_LOADING &&
          vmm_guest_transition(&instance->guest_state, VMM_GUEST_EVENT_LOAD) == 0) {
        underlord_hlog_info("VMM instance %u accepted guest image", instance->id);
        continue;
      }
      if (message == VMM_PROTOCOL_GUEST_BOOTING &&
          vmm_guest_transition(&instance->guest_state, VMM_GUEST_EVENT_BOOT) == 0) continue;
      if (message == VMM_PROTOCOL_GUEST_STARTED &&
          vmm_guest_transition(&instance->guest_state, VMM_GUEST_EVENT_START) == 0) continue;
      if (message == VMM_PROTOCOL_GUEST_STOPPED &&
          vmm_guest_transition(&instance->guest_state, VMM_GUEST_EVENT_STOP) == 0 &&
          vmm_lifecycle_transition(&instance->state, VMM_EVENT_STOP) == 0) {
        printf("UNDERLORD_PHASE2_RESULT: PASS\n");
        return;
      }
invalid_event:
      (void)vmm_lifecycle_transition(&instance->state, VMM_EVENT_FAILURE);
      underlord_hlog_error("VMM instance %u sent an invalid guest event", instance->id);
      continue;
    }
    (void)vmm_lifecycle_transition(&instance->state, VMM_EVENT_FAULT);
    underlord_hlog_error("VMM instance %u faulted (badge=%lu, label=%lu)", instance->id,
                         (unsigned long)badge, (unsigned long)seL4_GetMR(0));
  }
}
