#!/bin/sh
set -eu

test_name=$1
host_cc=$2
source_root=$3
work_dir=$(mktemp -d)
trap 'rm -rf "$work_dir"' EXIT

case "$test_name" in
utils)
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/underlord-utils/include" \
        "$source_root/underlord-utils/tests/log_format_test.c" \
        "$source_root/underlord-utils/src/log_format.c" -o "$work_dir/test"
    ;;
hypervisor)
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/hypervisor/include" \
        "$source_root/hypervisor/tests/vmm_lifecycle_test.c" \
        "$source_root/hypervisor/src/vmm_lifecycle.c" -o "$work_dir/lifecycle"
    "$work_dir/lifecycle"
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/hypervisor/include" \
        "$source_root/hypervisor/tests/vmm_image_test.c" \
        "$source_root/hypervisor/src/vmm_image.c" -o "$work_dir/image"
    "$work_dir/image"
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/hypervisor/include" \
        "$source_root/hypervisor/tests/vmm_resource_test.c" \
        "$source_root/hypervisor/src/vmm_resource.c" -o "$work_dir/resource"
    "$work_dir/resource"
    exit 0
    ;;
vmm)
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/my-vmm/include" \
        "$source_root/my-vmm/tests/vmm_protocol_test.c" \
        "$source_root/my-vmm/src/vmm_protocol_core.c" -o "$work_dir/test"
    "$work_dir/test"
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/my-vmm/include" \
        "$source_root/my-vmm/tests/vmm_guest_contract_test.c" \
        "$source_root/my-vmm/src/vmm_guest_contract.c" -o "$work_dir/contract"
    "$work_dir/contract"
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/my-vmm/include" \
        "$source_root/my-vmm/tests/vmm_elf_test.c" \
        "$source_root/my-vmm/src/vmm_elf.c" -o "$work_dir/elf"
    "$work_dir/elf"
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/my-vmm/include" \
        "$source_root/my-vmm/tests/vmm_guest_ram_state_test.c" \
        "$source_root/my-vmm/src/vmm_guest_ram_state.c" -o "$work_dir/ram"
    "$work_dir/ram"
    "$host_cc" -std=c11 -Wall -Wextra -Werror \
        -I"$source_root/my-vmm/include" \
        "$source_root/my-vmm/tests/vmm_guest_console_test.c" \
        "$source_root/my-vmm/src/vmm_guest_console.c" -o "$work_dir/console"
    "$work_dir/console"
    exit 0
    ;;
*)
    exit 2
    ;;
esac
"$work_dir/test"
