#include <assert.h>

#include <vmm_image.h>

int main(void)
{
    assert(vmm_image_metadata_valid(VMM_IMAGE_NAME, 1) == 1);
    assert(vmm_image_metadata_valid("other", 1) == 0);
    assert(vmm_image_metadata_valid(VMM_IMAGE_NAME, 0) == 0);
    return 0;
}
