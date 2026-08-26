#include "root_bootstrap.h"
#include "vmm_instance.h"
#include "vmm_manager.h"

int main(void) {
  hypervisor_context_t context;
  vmm_instance_t vmm_instance = {.id = 0, .state = VMM_CREATED};

  if (hypervisor_bootstrap(&context) != 0 ||
      vmm_instance_start(&context, &vmm_instance) != 0) {
    return -1;
  }
  vmm_instance_supervise(&vmm_instance);
  return 0;
}
