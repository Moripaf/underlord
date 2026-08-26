add_executable(underlord-sel4-tests
    runner.c
    ${CMAKE_SOURCE_DIR}/underlord-utils/src/log_format.c
    ${CMAKE_SOURCE_DIR}/my-vmm/src/vmm_protocol_core.c
    ${CMAKE_SOURCE_DIR}/hypervisor/src/vmm_lifecycle.c
    ${CMAKE_SOURCE_DIR}/hypervisor/src/vmm_image.c
)

target_include_directories(underlord-sel4-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/underlord-utils/include
    ${CMAKE_SOURCE_DIR}/my-vmm/include
    ${CMAKE_SOURCE_DIR}/hypervisor/include
)
target_link_libraries(underlord-sel4-tests PRIVATE
    sel4runtime sel4 muslc sel4muslcsys underlord-utils
)

include(${SEL4_ROOT}/tools/seL4/cmake-tool/helpers/rootserver.cmake)
DeclareRootserver(underlord-sel4-tests)
include(${SEL4_ROOT}/tools/seL4/cmake-tool/helpers/simulation.cmake)
GenerateSimulateScript()
