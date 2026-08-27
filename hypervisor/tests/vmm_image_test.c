#include <assert.h>

#include <vmm_image.h>

int main(void)
{
    assert(vmm_image_metadata_valid(VMM_IMAGE_NAME, 1) == 1);
    assert(vmm_image_metadata_valid("other", 1) == 0);
    assert(vmm_image_metadata_valid(VMM_IMAGE_NAME, 0) == 0);
    assert(vmm_guest_image_metadata_valid(VMM_GUEST_IMAGE_NAME, 1) == 1);
    assert(vmm_guest_image_metadata_valid(VMM_GUEST_IMAGE_NAME, VMM_GUEST_IMAGE_MAX_SIZE + 1U) == 0);
    return 0;
}
