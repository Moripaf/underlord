#pragma once

/***
 * @file vmm_resource.h
 * Pure fixed-budget boot-untyped selection policy for the Phase-2 VMM.
 */

#include <stddef.h>

/*** @struct vmm_untyped_candidate_t
 * One boot-time untyped capability's allocation properties.
 * @param size_bits Object size exponent.
 * @param device Non-zero when the capability is device memory.
 */
typedef struct {
    size_t size_bits;
    int device;
} vmm_untyped_candidate_t;

/*** @function vmm_select_untyped(candidates, count, minimum_size_bits)
 * Choose the smallest eligible non-device candidate, breaking ties by index.
 * @param candidates Candidate array.
 * @param count Candidate count.
 * @param minimum_size_bits Required size exponent.
 * @return Candidate index, or -1 when none qualifies.
 * @sideeffect None.
 */
int vmm_select_untyped(const vmm_untyped_candidate_t *candidates, size_t count,
                       size_t minimum_size_bits);
