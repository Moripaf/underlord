cmake_minimum_required(VERSION 3.16.0)

project(hypervisor C CXX ASM)

add_executable(${PROJECT_NAME}
    archive.o
    src/main.c
    src/root_bootstrap.c
    src/vmm_image.c
    src/vmm_lifecycle.c
    src/vmm_resource.c
    src/vmm_manager.c
    ${CMAKE_SOURCE_DIR}/my-vmm/src/vmm_protocol_core.c
    ${CMAKE_SOURCE_DIR}/my-vmm/src/vmm_guest_contract.c
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/my-vmm/include
        ${CMAKE_SOURCE_DIR}/underlord-utils/hypervisor-include
)

target_compile_definitions(${PROJECT_NAME} PRIVATE UNDERLORD_GUEST_NAME="${UNDERLORD_GUEST}")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        sel4runtime
        sel4
        muslc
        sel4muslcsys
        sel4utils
        sel4platsupport
        sel4debug
        sel4allocman
        underlord-utils
)

include(cpio)
configure_file(${UNDERLORD_C_GUEST_ELF} ${CMAKE_CURRENT_BINARY_DIR}/c-hello COPYONLY)
configure_file(${UNDERLORD_CPP_GUEST_ELF} ${CMAKE_CURRENT_BINARY_DIR}/cpp-hello COPYONLY)
configure_file(${UNDERLORD_C_FS_GUEST_ELF} ${CMAKE_CURRENT_BINARY_DIR}/c-fs COPYONLY)
configure_file(${UNDERLORD_C_FS_9P_GUEST_ELF} ${CMAKE_CURRENT_BINARY_DIR}/c-fs-9p COPYONLY)
MakeCPIO(archive.o "$<TARGET_FILE:my-vmm>;${CMAKE_CURRENT_BINARY_DIR}/c-hello;${CMAKE_CURRENT_BINARY_DIR}/cpp-hello;${CMAKE_CURRENT_BINARY_DIR}/c-fs;${CMAKE_CURRENT_BINARY_DIR}/c-fs-9p")

include(${SEL4_ROOT}/tools/seL4/cmake-tool/helpers/rootserver.cmake)
DeclareRootserver(${PROJECT_NAME})

include(${SEL4_ROOT}/tools/seL4/cmake-tool/helpers/simulation.cmake)
GenerateSimulateScript()
