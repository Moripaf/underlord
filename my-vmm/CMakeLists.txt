cmake_minimum_required(VERSION 3.16.0)


project(my-vmm C CXX ASM)

# Build is only used for user-mode apps.
enable_language(C)
enable_language(ASM)



add_executable(${PROJECT_NAME}
    src/main.c
    src/vmm_protocol_core.c
    src/vmm_guest_contract.c
    src/vmm_elf.c
    src/vmm_resources.c
    src/vmm_guest_ram_state.c
    src/vmm_guest_console.c
    src/vmm_guest_boot.c
    src/vmm_guest_ram.c
    src/vmm_vm.c
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

if(VMM_FAULT_TEST)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VMM_FAULT_TEST=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE VMM_FAULT_TEST=0)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE UNDERLORD_C_FS_9P_MODE="${UNDERLORD_C_FS_9P_MODE}")

# Keep this list intentional for what your app actually uses.
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        sel4runtime
        sel4
        muslc
        sel4muslcsys
        sel4utils
        sel4platsupport
        sel4debug
        sel4allocman
        sel4vm
        sel4vmmplatsupport
        underlord-utils
        # optional user libs if needed:
        # sel4debug
        # sel4sync
)

# If you enabled projects_libs, link in chosen targets here explicitly.
# e.g.
# target_link_libraries(${PROJECT_NAME} PRIVATE sel4vmmplatsupport sel4keyboard sel4vchan)
