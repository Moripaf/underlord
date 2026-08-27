#include <assert.h>

#include <vmm_resource.h>

int main(void)
{
    const vmm_untyped_candidate_t constrained[] = {{27, 0}, {30, 1}, {26, 0}};
    const vmm_untyped_candidate_t eligible[] = {{29, 0}, {28, 0}, {28, 0}};

    assert(vmm_select_untyped(constrained, 3, 28) == -1);
    assert(vmm_select_untyped(eligible, 3, 28) == 1);
    assert(vmm_select_untyped(NULL, 0, 28) == -1);
    return 0;
}
