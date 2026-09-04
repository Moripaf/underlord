#include <assert.h>
#include <string.h>

#include <vmm_guest_contract.h>

int main(void)
{
    vmm_shared_image_descriptor_t descriptor;
    vmm_guest_state_t state = VMM_GUEST_NONE;

    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.magic = VMM_SHARED_IMAGE_MAGIC;
    descriptor.version = VMM_SHARED_IMAGE_VERSION;
    descriptor.header_size = sizeof(descriptor);
    descriptor.image_offset = VMM_SHARED_IMAGE_OFFSET;
    descriptor.image_length = 64;
    descriptor.mapped_length = VMM_SHARED_IMAGE_OFFSET * 2U;
    descriptor.delegated_untyped_paddr = UINT64_C(0x80000000);
    descriptor.delegated_untyped_size_bits = 28;
    descriptor.vmm_stack_guard_base = UINT64_C(0x10001000);
    descriptor.vmm_stack_reserved_length = UINT64_C(0x11000);
    assert(vmm_shared_image_descriptor_valid(&descriptor, descriptor.mapped_length) == 0);
    descriptor.image_length = 0;
    assert(vmm_shared_image_descriptor_valid(&descriptor, descriptor.mapped_length) == -1);
    descriptor.image_length = 64;
    descriptor.delegated_untyped_size_bits = 27;
    assert(vmm_shared_image_descriptor_valid(&descriptor, descriptor.mapped_length) == -1);
    assert(vmm_guest_transition(&state, VMM_GUEST_EVENT_LOAD) == 0);
    assert(vmm_guest_transition(&state, VMM_GUEST_EVENT_BOOT) == 0);
    assert(vmm_guest_transition(&state, VMM_GUEST_EVENT_START) == 0);
    assert(vmm_guest_transition(&state, VMM_GUEST_EVENT_STOP) == 0);
    assert(state == VMM_GUEST_STOPPED);
    return 0;
}
