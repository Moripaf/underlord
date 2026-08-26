#include <string.h>

#include <vmm_image.h>

int vmm_image_metadata_valid(const char *name, size_t size)
{
    return name != NULL && strcmp(name, VMM_IMAGE_NAME) == 0 && size != 0;
}
