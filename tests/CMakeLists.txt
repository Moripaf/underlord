find_program(UNDERLORD_HOST_CC NAMES cc gcc REQUIRED)

add_test(NAME underlord-utils-host
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run-host-test.sh utils ${UNDERLORD_HOST_CC} ${CMAKE_SOURCE_DIR})
add_test(NAME hypervisor-host
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run-host-test.sh hypervisor ${UNDERLORD_HOST_CC} ${CMAKE_SOURCE_DIR})
add_test(NAME my-vmm-host
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run-host-test.sh vmm ${UNDERLORD_HOST_CC} ${CMAKE_SOURCE_DIR})

add_subdirectory(sel4)
