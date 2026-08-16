#include <allocman/allocman.h>
seL4_BootInfo *CURRENT_BOOT_INFO;

/* simple_t defined in simple.h */
simple_t ROOT_SIMPLE;

/* vka_t defined in vka.h */
vka_t VKA;

/* allocman_t defined in allocman.h */
allocman_t *ROOT_ALLOCMAN;

seL4_CPtr ROOT_VSPACE;
