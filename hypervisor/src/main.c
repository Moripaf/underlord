#include "root_bootstrap.h"
#include "vmm_instance.h"
#include "vmm_manager.h"

#include <sel4/sel4.h>

int main(void) {
  hypervisor_context_t context;
  vmm_instance_t vmm_instance = {.id = 0, .guest_state = VMM_GUEST_NONE,
                                 .state = VMM_CREATED};

  if (hypervisor_bootstrap(&context) != 0 ||
      vmm_instance_start(&context, &vmm_instance) != 0) {
    /* Root-task bootstrap failures are terminal but must not return through
     * an invalid runtime environment. */
    for (;;) {
      seL4_Yield();
    }
  }
  vmm_instance_supervise(&vmm_instance);
  return 0;
}
