#include <vmm_resource.h>

int vmm_select_untyped(const vmm_untyped_candidate_t *candidates, size_t count,
                       size_t minimum_size_bits)
{
    int selected = -1;

    if (candidates == NULL) return -1;
    for (size_t index = 0; index < count; index++) {
        if (candidates[index].device || candidates[index].size_bits < minimum_size_bits) continue;
        if (selected < 0 || candidates[index].size_bits < candidates[selected].size_bits) {
            selected = (int)index;
        }
    }
    return selected;
}
