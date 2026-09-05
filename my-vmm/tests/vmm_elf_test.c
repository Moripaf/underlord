#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <vmm_elf.h>
#include <vmm_guest_contract.h>

static void put16(unsigned char *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (unsigned char)value;
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static void put32(unsigned char *bytes, size_t offset, uint32_t value)
{
    put16(bytes, offset, (uint16_t)value);
    put16(bytes, offset + 2, (uint16_t)(value >> 16));
}

static void put64(unsigned char *bytes, size_t offset, uint64_t value)
{
    put32(bytes, offset, (uint32_t)value);
    put32(bytes, offset + 4, (uint32_t)(value >> 32));
}

int main(void)
{
    unsigned char image[1024] = {0};
    vmm_elf_plan_t plan = {0};

    memcpy(image, "\177ELF", 4);
    image[4] = 2;
    image[5] = 1;
    image[6] = 1;
    put16(image, 16, 2);
    put16(image, 18, 183);
    put64(image, 24, VMM_GUEST_RAM_BASE + VMM_GUEST_DTB_SIZE);
    put64(image, 32, 64);
    put64(image, 40, 512);
    put16(image, 54, 56);
    put16(image, 56, 1);
    put16(image, 58, 64);
    put16(image, 60, 3);
    put16(image, 62, 1);
    put32(image, 64, 1);
    put32(image, 68, 5);
    put64(image, 72, 256);
    put64(image, 80, VMM_GUEST_RAM_BASE + VMM_GUEST_DTB_SIZE);
    put64(image, 88, VMM_GUEST_RAM_BASE + VMM_GUEST_DTB_SIZE);
    put64(image, 96, 4);
    put64(image, 104, 8);
    put32(image, 576, 1);
    put64(image, 600, 768);
    put64(image, 608, 14);
    put32(image, 900, 0);
    memcpy(image + 901, ".uk_bootinfo", 12);
    put64(image, 576 + 24, 900);
    put64(image, 576 + 32, 14);
    put32(image, 768, 0xb007b0b0U);
    image[772] = 1;
    put32(image, 844, 1);
    put32(image, 640, 1);
    put64(image, 640 + 24, 768);
    put64(image, 640 + 32, 80);

    assert(vmm_elf_validate(image, sizeof(image), &plan) == 0);
    assert(plan.entry == VMM_GUEST_RAM_BASE + VMM_GUEST_DTB_SIZE);
    assert(plan.load_count == 1);
    assert(plan.loads[0].file_offset == 256);
    assert(plan.loads[0].file_size == 4);
    assert(plan.loads[0].memory_size == 8);
    /* NOBITS has no file range and must still be admissible. */
    put32(image, 512 + 4, 8);
    put64(image, 512 + 32, 16);
    assert(vmm_elf_validate(image, sizeof(image), &plan) == 0);
    /* A segment above RAM must be rejected without unsigned underflow. */
    put64(image, 80, UINT64_C(0x500000000));
    assert(vmm_elf_validate(image, sizeof(image), &plan) == -1);
    put64(image, 80, VMM_GUEST_RAM_BASE + VMM_GUEST_DTB_SIZE);
    image[18] = 0;
    assert(vmm_elf_validate(image, sizeof(image), &plan) == -1);
    return 0;
}
