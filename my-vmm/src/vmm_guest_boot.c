#include <string.h>

#include <libfdt.h>
#include <sel4/sel4.h>
#include <sel4vm/arch/guest_arm_context.h>
#include <sel4vm/arch/processor.h>
#include <sel4vm/boot.h>
#include <sel4vm/guest_ram.h>
#include <sel4vm/guest_vcpu_fault.h>
#include <sel4vm/guest_vm_util.h>
#include <sel4vm/sel4_arch/processor.h>
#include <sel4vmmplatsupport/arch/guest_vcpu_fault.h>
#include <sel4vmmplatsupport/arch/smc.h>
#include <sel4vmmplatsupport/guest_vcpu_util.h>

#include <underlord/vlog.h>
#include <vmm_elf.h>
#include <vmm_guest_boot.h>
#include <vmm_guest_contract.h>
#include <vmm_protocol.h>

struct copy_state {
  const unsigned char *src;
  int zero;
};

static vmm_vm_t *active_vm;
static unsigned char runtime_fdt[65536];
static unsigned int guest_vcpu_faults;

#define VMM_GUEST_EL1H_MASKED_SPSR UINT64_C(0x3c5)

static memory_fault_result_t unexpected_mem_fault(vm_t *guest, vm_vcpu_t *vcpu,
                                                  uintptr_t address,
                                                  size_t length, void *cookie) {
  (void)guest;
  (void)vcpu;
  (void)address;
  (void)length;
  (void)cookie;
  return FAULT_ERROR;
}

static int guest_vcpu_fault(vm_vcpu_t *vcpu, uint32_t hsr, void *cookie) {
  seL4_UserContext context;
  seL4_Word caller[2] = {0};
  seL4_Word parent[2] = {0};
  (void)cookie;
  if (guest_vcpu_faults++ == 0 && vm_get_thread_context(vcpu, &context) == 0) {
    underlord_vlog_info(0, "first guest vCPU exception HSR=0x%x PC=0x%lx", hsr,
                        (unsigned long)context.pc);
    if (active_vm != NULL &&
        vm_ram_touch(&active_vm->vm, context.x29, sizeof(caller),
                     vm_guest_ram_read_callback, caller) == 0)
      underlord_vlog_info(0, "guest halt caller=0x%lx",
                          (unsigned long)caller[1]);
    if (active_vm != NULL && caller[0] != 0 &&
        vm_ram_touch(&active_vm->vm, caller[0], sizeof(parent),
                     vm_guest_ram_read_callback, parent) == 0)
      underlord_vlog_info(0, "guest halt parent=0x%lx",
                          (unsigned long)parent[1]);
  }
  if (HSR_EXCEPTION_CLASS(hsr) == HSR_WFx_EXCEPTION) {
    advance_vcpu_fault(vcpu);
    return 0;
  }
  return vmm_handle_arm_vcpu_exception(vcpu, hsr, NULL);
}

static void guest_line(const char *line, void *cookie) {
  vmm_vm_t *vm = cookie;
  underlord_vlog_guest_info(0, "%s", line);
  if (vmm_guest_console_hello_seen(&vm->guest_console) && !vm->guest_hello) {
    uint32_t word;
    vm->guest_hello = 1;
    if (vmm_protocol_encode(VMM_PROTOCOL_GUEST_STARTED, &word) == 0) {
      seL4_SetMR(0, word);
      seL4_Send(VMM_CONTROL_ENDPOINT_SLOT, seL4_MessageInfo_new(0, 0, 0, 1));
    }
  }
}

static memory_fault_result_t uart_fault(vm_t *guest, vm_vcpu_t *vcpu,
                                        uintptr_t address, size_t length,
                                        void *cookie) {
  vmm_vm_t *vm = cookie;
  uintptr_t offset;
  seL4_Word value = 0;
  if (guest == NULL || vcpu == NULL || vm == NULL || length != 2 ||
      address < (uintptr_t)0x09000000U || address >= (uintptr_t)0x09001000U)
    return FAULT_ERROR;
  offset = address - (uintptr_t)0x09000000U;
  underlord_vlog_debug(0, "guest PL011 %s offset=0x%lx",
                       is_vcpu_read_fault(vcpu) ? "read" : "write",
                       (unsigned long)offset);
  if (!is_vcpu_read_fault(vcpu)) {
    if (offset == 0x00) {
      char byte = (char)(get_vcpu_fault_data(vcpu) & 0xffU);
      if (vmm_guest_console_feed(&vm->guest_console, &byte, 1, guest_line,
                                 vm) != 0)
        return FAULT_ERROR;
    } else if (offset != 0x24 && offset != 0x2c && offset != 0x30 &&
               offset != 0x38 && offset != 0x44)
      return FAULT_ERROR;
  } else {
    if (offset == 0x18)
      value = 0x90; /* TXFE and RXFE, never TXFF */
    else if (offset == 0x00 || offset == 0x24 || offset == 0x2c ||
             offset == 0x30 || offset == 0x38 || offset == 0x44)
      value = 0;
    else
      return FAULT_ERROR;
  }
  if (set_vcpu_fault_data(vcpu, value) != 0)
    return FAULT_ERROR;
  advance_vcpu_fault(vcpu);
  return FAULT_HANDLED;
}

static int guest_smc(vm_vcpu_t *vcpu, seL4_UserContext *regs) {
  seL4_Word id = smc_get_function_id(regs);
  if (((id >> 24) & 0x3fU) == 4U) {
    if ((id & 0xffffU) == 8U) {
      uint32_t word;
      if (active_vm == NULL || !active_vm->guest_hello)
        return -1;
      if (vmm_protocol_encode(VMM_PROTOCOL_GUEST_STOPPED, &word) != 0)
        return -1;
      seL4_SetMR(0, word);
      seL4_Send(VMM_CONTROL_ENDPOINT_SLOT, seL4_MessageInfo_new(0, 0, 0, 1));
      return seL4_TCB_Suspend(vm_get_vcpu_tcb(vcpu)) == seL4_NoError ? 0 : -1;
    }
  }
  return vm_smc_handle_default(vcpu, regs);
}

static int copy_page(vm_t *vm, uintptr_t guest, void *vaddr, size_t size,
                     size_t offset, void *cookie) {
  struct copy_state *state = cookie;
  (void)vm;
  (void)guest;
  if (state->zero)
    memset(vaddr, 0, size);
  else
    memcpy(vaddr, state->src + offset, size);
  return 0;
}

static int put_reg64(void *fdt, int node, const char *name, uint64_t base,
                     uint64_t size) {
  fdt32_t value[4] = {
      cpu_to_fdt32((uint32_t)(base >> 32)), cpu_to_fdt32((uint32_t)base),
      cpu_to_fdt32((uint32_t)(size >> 32)), cpu_to_fdt32((uint32_t)size)};
  return fdt_setprop(fdt, node, name, value, sizeof(value));
}

static int put_gic_reg(void *fdt, int node) {
  fdt32_t value[8] = {cpu_to_fdt32(0), cpu_to_fdt32(0x08000000),
                      cpu_to_fdt32(0), cpu_to_fdt32(0x10000),
                      cpu_to_fdt32(0), cpu_to_fdt32(0x08010000),
                      cpu_to_fdt32(0), cpu_to_fdt32(0x2000)};
  return fdt_setprop(fdt, node, "reg", value, sizeof(value));
}

static int make_fdt(void *fdt, size_t capacity) {
  int root, cpus, cpu, intc, psci, uart, timer, chosen, aliases;
  fdt32_t one = cpu_to_fdt32(1), zero = cpu_to_fdt32(0);
  if (fdt_create_empty_tree(fdt, capacity) < 0)
    return -1;
  root = fdt_path_offset(fdt, "/");
  if (fdt_setprop(fdt, root, "#address-cells", &(fdt32_t){cpu_to_fdt32(2)},
                  sizeof(one)) < 0 ||
      fdt_setprop(fdt, root, "#size-cells", &(fdt32_t){cpu_to_fdt32(2)},
                  sizeof(one)) < 0)
    return -1;
  {
    int memory = fdt_add_subnode(fdt, root, "memory@40000000");
    if (memory < 0 ||
        fdt_setprop_string(fdt, memory, "device_type", "memory") < 0 ||
        put_reg64(fdt, memory, "reg", VMM_GUEST_RAM_BASE, VMM_GUEST_RAM_SIZE) <
            0)
      return -1;
  }
  intc = fdt_add_subnode(fdt, root, "intc@8000000");
  if (intc < 0 ||
      fdt_setprop_string(fdt, intc, "compatible", "arm,cortex-a15-gic") < 0 ||
      fdt_setprop(fdt, intc, "interrupt-controller", NULL, 0) < 0 ||
      fdt_setprop(fdt, intc, "#interrupt-cells", &(fdt32_t){cpu_to_fdt32(3)},
                  sizeof(fdt32_t)) < 0 ||
      put_gic_reg(fdt, intc) < 0)
    return -1;
  if (fdt_setprop_u32(fdt, intc, "phandle", 1) < 0)
    return -1;
  cpus = fdt_add_subnode(fdt, root, "cpus");
  if (cpus < 0 ||
      fdt_setprop(fdt, cpus, "#address-cells", &one, sizeof(one)) < 0 ||
      fdt_setprop(fdt, cpus, "#size-cells", &zero, sizeof(zero)) < 0)
    return -1;
  cpu = fdt_add_subnode(fdt, cpus, "cpu@0");
  if (cpu < 0 || fdt_setprop_string(fdt, cpu, "device_type", "cpu") < 0 ||
      fdt_setprop_string(fdt, cpu, "compatible", "arm,cortex-a53") < 0 ||
      fdt_setprop(fdt, cpu, "reg", &zero, sizeof(zero)) < 0 ||
      fdt_setprop_string(fdt, cpu, "enable-method", "psci") < 0)
    return -1;
  psci = fdt_add_subnode(fdt, root, "psci");
  if (psci < 0 ||
      fdt_setprop_string(fdt, psci, "compatible", "arm,psci-1.0") < 0 ||
      fdt_setprop_string(fdt, psci, "method", "smc") < 0)
    return -1;
  timer = fdt_add_subnode(fdt, root, "timer");
  if (timer < 0 ||
      fdt_setprop_string(fdt, timer, "compatible", "arm,armv8-timer") < 0 ||
      fdt_setprop(fdt, timer, "interrupt-parent", &(fdt32_t){cpu_to_fdt32(1)},
                  sizeof(fdt32_t)) < 0 ||
      fdt_setprop(
          fdt, timer, "interrupts",
          (fdt32_t[]){cpu_to_fdt32(1), cpu_to_fdt32(13), cpu_to_fdt32(4),
                      cpu_to_fdt32(1), cpu_to_fdt32(14), cpu_to_fdt32(4),
                      cpu_to_fdt32(1), cpu_to_fdt32(11), cpu_to_fdt32(4),
                      cpu_to_fdt32(1), cpu_to_fdt32(10), cpu_to_fdt32(4)},
          48) < 0)
    return -1;
  uart = fdt_add_subnode(fdt, root, "pl011@9000000");
  if (uart < 0 ||
      fdt_setprop_string(fdt, uart, "compatible", "arm,pl011") < 0 ||
      put_reg64(fdt, uart, "reg", UINT64_C(0x09000000), UINT64_C(0x1000)) < 0 ||
      fdt_setprop(fdt, uart, "interrupt-parent", &(fdt32_t){cpu_to_fdt32(1)},
                  sizeof(fdt32_t)) < 0 ||
      fdt_setprop(
          fdt, uart, "interrupts",
          (fdt32_t[]){cpu_to_fdt32(0), cpu_to_fdt32(1), cpu_to_fdt32(4)},
          12) < 0)
    return -1;
  aliases = fdt_add_subnode(fdt, root, "aliases");
  if (aliases < 0 ||
      fdt_setprop_string(fdt, aliases, "serial0", "/pl011@9000000") < 0)
    return -1;
  chosen = fdt_add_subnode(fdt, root, "chosen");
  if (chosen < 0 ||
      fdt_setprop_string(fdt, chosen, "stdout-path", "serial0:115200n8") < 0)
    return -1;
  if (fdt_pack(fdt) < 0)
    return -1;
  return 0;
}

static int write_fdt(vm_t *vm, const void *fdt, size_t size) {
  struct copy_state state = {fdt, 0};
  return vm_ram_touch(vm, VMM_GUEST_RAM_BASE, size, copy_page, &state);
}

int vmm_guest_boot_load(vmm_vm_t *vm, const void *image, size_t image_size,
                        uint64_t *entry) {
  vmm_elf_plan_t plan;
  if (vm == NULL || image == NULL || entry == NULL ||
      vmm_elf_validate(image, image_size, &plan) != 0)
    return -1;
  underlord_vlog_info(0, "guest ELF validated (%u segments)",
                      (unsigned)plan.load_count);
  active_vm = vm;
  vmm_guest_console_init(&vm->guest_console);
  vm->guest_hello = 0;
  vm->vm.mem.clean_cache = 1;
  if (vm_register_unhandled_mem_fault_callback(&vm->vm, unexpected_mem_fault,
                                               vm) != 0 ||
      vm_reserve_memory_at(&vm->vm, (uintptr_t)0x09000000U, 0x1000, uart_fault,
                           vm) == NULL ||
      vm_register_smc_handler_callback(&vm->vm, guest_smc) != 0)
    return -1;
  for (size_t i = 0; i < plan.load_count; i++) {
    const vmm_elf_segment_t *segment = &plan.loads[i];
    struct copy_state copy = {
        (const unsigned char *)image + segment->file_offset, 0};
    if (segment->file_size &&
        vm_ram_touch(&vm->vm, segment->guest_address, segment->file_size,
                     copy_page, &copy) != 0) {
      underlord_vlog_error(0, "guest ELF file copy failed at 0x%lx",
                           (unsigned long)segment->guest_address);
      return -1;
    }
    if (segment->memory_size > segment->file_size) {
      struct copy_state zero = {NULL, 1};
      if (vm_ram_touch(&vm->vm, segment->guest_address + segment->file_size,
                       segment->memory_size - segment->file_size, copy_page,
                       &zero) != 0) {
        underlord_vlog_error(0, "guest ELF zero fill failed at 0x%lx",
                             (unsigned long)segment->guest_address);
        return -1;
      }
    }
    underlord_vlog_info(0, "guest ELF segment %u loaded", (unsigned)i);
  }
  if (make_fdt(runtime_fdt, sizeof(runtime_fdt)) != 0) {
    underlord_vlog_error(0, "guest FDT construction failed");
    return -1;
  }
  if (write_fdt(&vm->vm, runtime_fdt, fdt_totalsize(runtime_fdt)) != 0) {
    underlord_vlog_error(0, "guest FDT copy failed");
    return -1;
  }
  underlord_vlog_info(0, "guest runtime FDT loaded");
  vm->vm.entry = plan.entry;
  *entry = plan.entry;
  return 0;
}

int vmm_guest_boot_start(vmm_vm_t *vm, uint64_t entry) {
  vm_vcpu_t *vcpu;
  seL4_UserContext context = {0};
  if (vm == NULL)
    return -1;
  vcpu = create_vmm_plat_vcpu(&vm->vm, 0);
  if (vcpu == NULL || vm_assign_vcpu_target(vcpu, 0) != 0)
    return -1;
  if (vm_register_unhandled_vcpu_fault_callback(vcpu, guest_vcpu_fault, NULL) !=
      0)
    return -1;
  context.pc = entry;
  context.spsr = VMM_GUEST_EL1H_MASKED_SPSR;
  if (vm_set_thread_context(vcpu, context) != 0 ||
      vm_set_arm_vcpu_reg(vcpu, seL4_VCPUReg_SCTLR, 0) != 0 ||
      vcpu_start(vcpu) != 0)
    return -1;
  underlord_vlog_info(0, "guest vCPU started at 0x%lx", (unsigned long)entry);
  return 0;
}
