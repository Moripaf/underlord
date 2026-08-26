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
target_link_libraries(underlord-utils PRIVATE muslc)
