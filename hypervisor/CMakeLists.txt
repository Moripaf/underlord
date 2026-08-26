cmake_minimum_required(VERSION 3.16.0)

project(hypervisor C CXX ASM)

add_executable(${PROJECT_NAME}
    archive.o
    src/main.c
    src/root_bootstrap.c
    src/vmm_image.c
    src/vmm_lifecycle.c
    src/vmm_manager.c
    ${CMAKE_SOURCE_DIR}/my-vmm/src/vmm_protocol_core.c
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/my-vmm/include
        ${CMAKE_SOURCE_DIR}/underlord-utils/hypervisor-include
)

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
MakeCPIO(archive.o "$<TARGET_FILE:my-vmm>")

include(${SEL4_ROOT}/tools/seL4/cmake-tool/helpers/rootserver.cmake)
DeclareRootserver(${PROJECT_NAME})

include(${SEL4_ROOT}/tools/seL4/cmake-tool/helpers/simulation.cmake)
GenerateSimulateScript()
