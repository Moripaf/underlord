cmake_minimum_required(VERSION 3.16.0)

project(underlord-utils C)

add_library(underlord-utils STATIC
    src/log.c
    src/log_format.c
)

target_include_directories(underlord-utils
    PUBLIC include
    PRIVATE hypervisor-include
)
target_compile_definitions(underlord-utils PRIVATE
    UNDERLORD_LOG_LEVEL=UNDERLORD_LOG_${UNDERLORD_LOG_LEVEL}
)
target_link_libraries(underlord-utils PRIVATE muslc)
