#include <string.h>

#include <vmm_image.h>

#define VMM_IMAGE_PAGE_BITS 12U

int vmm_image_metadata_valid(const char *name, size_t size)
{
    return name != NULL && strcmp(name, VMM_IMAGE_NAME) == 0 && size != 0;
}

int vmm_guest_image_metadata_valid(const char *name, size_t size)
{
    return name != NULL && (strcmp(name, "c-hello") == 0 || strcmp(name, "cpp-hello") == 0) && size != 0 &&
           size <= VMM_GUEST_IMAGE_MAX_SIZE;
}

int vmm_guest_image_valid(const vmm_guest_image_t *image)
{
    return image != NULL && image->bytes != NULL && image->size != 0 &&
           image->size <= VMM_GUEST_IMAGE_MAX_SIZE;
}

size_t vmm_guest_image_arena_size_bits(size_t size)
{
    size_t bits = VMM_IMAGE_PAGE_BITS;
    size_t capacity = (size_t)1U << bits;
    if (size == 0 || size > VMM_GUEST_IMAGE_MAX_SIZE + 4096U) return 0;
    while (capacity < size) {
        if (bits + 1U >= sizeof(size_t) * 8U) return 0;
        bits++;
        capacity <<= 1U;
    }
    return bits;
}
