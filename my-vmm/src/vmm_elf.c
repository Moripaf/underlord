#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <vmm_elf.h>
#include <vmm_guest_contract.h>

#define ELF_HEADER_SIZE 64U
#define ELF_PROGRAM_HEADER_SIZE 56U
#define ELF_SECTION_HEADER_SIZE 64U
#define ELF_PT_LOAD 1U
#define ELF_ET_EXEC 2U
#define ELF_EM_AARCH64 183U
#define ELF_PF_X 1U

static uint16_t le16(const unsigned char *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static uint32_t le32(const unsigned char *p) { return (uint32_t)le16(p) | ((uint32_t)le16(p + 2) << 16); }
static uint64_t le64(const unsigned char *p) { return (uint64_t)le32(p) | ((uint64_t)le32(p + 4) << 32); }

static bool range_valid(uint64_t offset, uint64_t size, size_t total)
{
    return offset <= total && size <= (uint64_t)total - offset;
}

static bool section_name_is(const unsigned char *names, uint64_t names_size,
                            uint32_t offset, const char *expected)
{
    size_t expected_length = strlen(expected);
    return offset < names_size && expected_length < names_size - offset &&
           memcmp(names + offset, expected, expected_length + 1) == 0;
}

int vmm_elf_validate(const void *image, size_t image_size, vmm_elf_plan_t *plan)
{
    const unsigned char *bytes = image;
    uint64_t phoff, entry;
    uint64_t shoff;
    uint16_t phentsize, phnum, shentsize, shnum, shstrndx;
    size_t loads = 0;
    bool entry_found = false;
    vmm_elf_plan_t result = {0};

    if (bytes == NULL || plan == NULL || image_size < ELF_HEADER_SIZE || bytes[0] != 0x7f ||
        memcmp(bytes + 1, "ELF", 3) != 0 || bytes[4] != 2 || bytes[5] != 1 || bytes[6] != 1 ||
        le16(bytes + 16) != ELF_ET_EXEC || le16(bytes + 18) != ELF_EM_AARCH64) return -1;
    entry = le64(bytes + 24);
    phoff = le64(bytes + 32);
    shoff = le64(bytes + 40);
    phentsize = le16(bytes + 54);
    phnum = le16(bytes + 56);
    shentsize = le16(bytes + 58);
    shnum = le16(bytes + 60);
    shstrndx = le16(bytes + 62);
    if (phentsize != ELF_PROGRAM_HEADER_SIZE || phnum == 0 ||
        phnum > SIZE_MAX / phentsize || !range_valid(phoff, (uint64_t)phnum * phentsize, image_size)) return -1;
    for (uint16_t i = 0; i < phnum; i++) {
        const unsigned char *ph = bytes + phoff + (size_t)i * phentsize;
        uint64_t offset = le64(ph + 8), vaddr = le64(ph + 16), paddr = le64(ph + 24);
        uint64_t filesz = le64(ph + 32), memsz = le64(ph + 40), end;
        if (le32(ph) != ELF_PT_LOAD) continue;
        if (loads == VMM_ELF_MAX_LOAD_SEGMENTS) return -1;
        if (filesz > memsz || !range_valid(offset, filesz, image_size) || paddr != vaddr ||
            vaddr < VMM_GUEST_RAM_BASE || memsz > VMM_GUEST_RAM_BASE + VMM_GUEST_RAM_SIZE - vaddr ||
            vaddr < VMM_GUEST_RAM_BASE + VMM_GUEST_DTB_SIZE) return -1;
        end = vaddr + memsz;
        if (end < vaddr) return -1;
        for (uint16_t j = 0; j < i; j++) {
            const unsigned char *other = bytes + phoff + (size_t)j * phentsize;
            uint64_t other_addr, other_size;
            if (le32(other) != ELF_PT_LOAD) continue;
            other_addr = le64(other + 24); other_size = le64(other + 40);
            if (other_size != 0 && vaddr < other_addr + other_size && other_addr < end) return -1;
        }
        if ((le32(ph + 4) & ELF_PF_X) != 0 && entry >= vaddr && entry < end) entry_found = true;
        if (memsz != 0) {
            result.loads[loads] = (vmm_elf_segment_t){
                .guest_address = vaddr,
                .file_offset = offset,
                .file_size = filesz,
                .memory_size = memsz,
            };
            loads++;
        }
    }
    if (loads == 0 || !entry_found || shentsize != ELF_SECTION_HEADER_SIZE || shnum == 0 ||
        shstrndx >= shnum || shnum > SIZE_MAX / shentsize ||
        !range_valid(shoff, (uint64_t)shnum * shentsize, image_size)) return -1;
    {
        const unsigned char *names = bytes + shoff + (size_t)shstrndx * shentsize;
        uint64_t names_offset = le64(names + 24), names_size = le64(names + 32);
        bool bootinfo_found = false;
        if (!range_valid(names_offset, names_size, image_size)) return -1;
        for (uint16_t i = 0; i < shnum; i++) {
            const unsigned char *section = bytes + shoff + (size_t)i * shentsize;
            uint32_t name_offset = le32(section);
            uint64_t section_offset = le64(section + 24), section_size = le64(section + 32);
            if (name_offset >= names_size || !range_valid(section_offset, section_size, image_size)) return -1;
            if (section_name_is(bytes + names_offset, names_size, name_offset, ".uk_bootinfo") &&
                section_size != 0) bootinfo_found = true;
        }
        if (!bootinfo_found) return -1;
    }
    result.entry = entry;
    result.load_count = loads;
    *plan = result;
    return 0;
}
