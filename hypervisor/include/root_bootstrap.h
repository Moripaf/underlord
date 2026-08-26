#pragma once

#include <allocman/allocman.h>
#include <sel4utils/vspace.h>
#include <simple/simple.h>
#include <vka/vka.h>
#include <vspace/vspace.h>

typedef struct {
    simple_t simple;
    allocman_t *allocman;
    vka_t vka;
    vspace_t vspace;
    sel4utils_alloc_data_t vspace_data;
} hypervisor_context_t;

int hypervisor_bootstrap(hypervisor_context_t *context);
