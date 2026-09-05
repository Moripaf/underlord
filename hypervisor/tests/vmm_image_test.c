#include <assert.h>

#include <vmm_image.h>

int main(void)
{
    assert(vmm_image_metadata_valid(VMM_IMAGE_NAME, 1) == 1);
    assert(vmm_image_metadata_valid("other", 1) == 0);
    assert(vmm_image_metadata_valid(VMM_IMAGE_NAME, 0) == 0);
    assert(vmm_guest_image_metadata_valid("c-hello", 1) == 1);
    assert(vmm_guest_image_metadata_valid("cpp-hello", 1) == 1);
    assert(vmm_guest_image_metadata_valid("c-fs", 1) == 1);
    assert(vmm_guest_image_metadata_valid("cpp-hello", VMM_GUEST_IMAGE_MAX_SIZE + 1U) == 0);
    assert(vmm_guest_image_metadata_valid("other", 1) == 0);
    assert(vmm_guest_image_valid(&(vmm_guest_image_t){"cpp-hello", (void *)1, 1}) == 1);
    assert(vmm_guest_image_valid(&(vmm_guest_image_t){"cpp-hello", NULL, 1}) == 0);
    assert(vmm_guest_image_valid(&(vmm_guest_image_t){"cpp-hello", (void *)1, VMM_GUEST_IMAGE_MAX_SIZE}) == 1);
    assert(vmm_guest_image_valid(&(vmm_guest_image_t){"cpp-hello", (void *)1, VMM_GUEST_IMAGE_MAX_SIZE + 1U}) == 0);
    assert(vmm_guest_image_arena_size_bits(0) == 0);
    assert(vmm_guest_image_arena_size_bits(1) == 12);
    assert(vmm_guest_image_arena_size_bits(4097) == 13);
    assert(vmm_guest_image_arena_size_bits(VMM_GUEST_IMAGE_MAX_SIZE + 4096U) == 24);
    assert(vmm_guest_image_arena_size_bits(VMM_GUEST_IMAGE_MAX_SIZE + 4097U) == 0);
    return 0;
}
