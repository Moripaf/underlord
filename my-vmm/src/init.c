#include "../include/init.h"
#include "platsupport/io.h"
#include "sel4/simple_types.h"
#include "simple/simple.h"
#include "vspace/vspace.h"
#include <sel4platsupport/bootinfo.h>
#include <sel4vm/boot.h>
#include <sel4vm/guest_vm.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

seL4_BootInfo *CURRENT_BOOT_INFO;
simple_t ROOT_SIMPLE;
vka_t VKA;
allocman_t *ROOT_ALLOCMAN;
seL4_CPtr ROOT_VSPACE;

int load_guest_vmm(uintptr_t load_address, guest_kernel_image_t *result) {
  vm_t *loading_vm;

  int error = vm_load_guest_kernel(loading_vm, GUEST_KERNEL_NAME, load_address,
                                   IMAGE_LOAD_ALIGNMENT, result);
  return error;
}
int initialize_vm(const char *name) {
  vm_t *loading_vm;
  int error = 0;
  ps_io_ops_t ops;
  seL4_CPtr endpoint;
  vspace_t vspace;
  //  error = vm_init(loading_vm, &VKA, &ROOT_SIMPLE, ROOT_VSPACE, &ops,
  //  endpoint, name);
  return error;
}

int initialize_allocators() {
  int error = 0;
  CURRENT_BOOT_INFO = platsupport_get_bootinfo();
  simple_default_init_bootinfo(&ROOT_SIMPLE, CURRENT_BOOT_INFO);
  // simple_print(&ROOT_SIMPLE);
  ROOT_ALLOCMAN = bootstrap_use_current_simple(
      &ROOT_SIMPLE, ALLOCATOR_STATIC_POOL_SIZE, allocator_mem_pool);
  printf("making vka\n");
  allocman_make_vka(&VKA, ROOT_ALLOCMAN);
  printf("vka made\n");
  ROOT_VSPACE = simple_get_pd(&ROOT_SIMPLE);
  printf("program descriptor retrieved\n");
  if (CURRENT_BOOT_INFO == NULL || ROOT_ALLOCMAN == NULL) {
    error = -1;
  }
  return error;
}
