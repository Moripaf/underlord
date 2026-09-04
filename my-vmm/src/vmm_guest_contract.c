#include <limits.h>

#include <vmm_guest_contract.h>

int vmm_shared_image_descriptor_valid(const vmm_shared_image_descriptor_t *descriptor,
                                      size_t mapped_bytes)
{
    uint64_t image_end;

    if (descriptor == NULL || descriptor->magic != VMM_SHARED_IMAGE_MAGIC ||
        descriptor->version != VMM_SHARED_IMAGE_VERSION ||
        descriptor->header_size != sizeof(*descriptor) ||
        descriptor->image_offset != VMM_SHARED_IMAGE_OFFSET ||
        descriptor->image_length == 0 || descriptor->image_length > VMM_GUEST_ELF_MAX_SIZE ||
        descriptor->delegated_untyped_size_bits < 28 ||
        descriptor->delegated_untyped_size_bits >= sizeof(uintptr_t) * CHAR_BIT ||
        descriptor->vmm_stack_guard_base == 0 ||
        descriptor->vmm_stack_reserved_length < (2U * (uint64_t)VMM_SHARED_IMAGE_OFFSET) ||
        (descriptor->vmm_stack_guard_base & (VMM_SHARED_IMAGE_OFFSET - 1U)) != 0 ||
        (descriptor->vmm_stack_reserved_length & (VMM_SHARED_IMAGE_OFFSET - 1U)) != 0 ||
        descriptor->mapped_length > mapped_bytes || descriptor->mapped_length > SIZE_MAX) {
        return -1;
    }
    if ((descriptor->delegated_untyped_paddr &
         (((uintptr_t)1 << descriptor->delegated_untyped_size_bits) - 1U)) != 0) return -1;
    if (descriptor->vmm_stack_guard_base > UINT64_MAX - descriptor->vmm_stack_reserved_length) return -1;
    image_end = (uint64_t)descriptor->image_offset + descriptor->image_length;
    if (image_end < descriptor->image_offset || image_end > descriptor->mapped_length ||
        descriptor->mapped_length < VMM_SHARED_IMAGE_OFFSET ||
        (descriptor->mapped_length & (VMM_SHARED_IMAGE_OFFSET - 1U)) != 0) {
        return -1;
    }
    return 0;
}

int vmm_guest_transition(vmm_guest_state_t *state, vmm_guest_event_t event)
{
    vmm_guest_state_t next;

    if (state == NULL) {
        return -1;
    }
    next = *state;
    if (*state == VMM_GUEST_NONE && event == VMM_GUEST_EVENT_LOAD) next = VMM_GUEST_LOADING;
    if (*state == VMM_GUEST_LOADING && event == VMM_GUEST_EVENT_BOOT) next = VMM_GUEST_BOOTING;
    if (*state == VMM_GUEST_BOOTING && event == VMM_GUEST_EVENT_START) next = VMM_GUEST_RUNNING;
    if (*state == VMM_GUEST_RUNNING && event == VMM_GUEST_EVENT_STOP) next = VMM_GUEST_STOPPED;
    if ((*state == VMM_GUEST_LOADING || *state == VMM_GUEST_BOOTING || *state == VMM_GUEST_RUNNING) &&
        event == VMM_GUEST_EVENT_FAIL) next = VMM_GUEST_FAILED;
    if (next == *state) {
        return -1;
    }
    *state = next;
    return 0;
}
