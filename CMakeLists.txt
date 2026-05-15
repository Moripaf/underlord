cmake_minimum_required(VERSION 3.16)
# Project may live outside the seL4 checkout.
# Pass -DSEL4_ROOT=/abs/path/to/this/repo when configuring.
#set(SEL4_ROOT "/home/moripaf/Code/sel4/tutorials" CACHE PATH "Path to seL4 checkout")
if(NOT DEFINED SEL4_ROOT)
    message(FATAL_ERROR "Set SEL4_ROOT to the path of your seL4 checkout")
endif()

project(Underlord)

# Make Find*.cmake modules resolvable from a project outside the root checkout.
list(
    APPEND
        CMAKE_MODULE_PATH
        ${SEL4_ROOT}/kernel
        ${SEL4_ROOT}/tools/seL4/cmake-tool/helpers
        ${SEL4_ROOT}/tools/seL4/elfloader-tool
        ${SEL4_ROOT}/projects/musllibc
        ${SEL4_ROOT}/projects/sel4runtime
        ${SEL4_ROOT}/projects/util_libs
        ${SEL4_ROOT}/projects/seL4_libs
        ${SEL4_ROOT}/projects/projects_libs
        ${SEL4_ROOT}/projects/seL4_projects_libs
)

# Pull core build system pieces
find_package(seL4 REQUIRED)
find_package(elfloader-tool REQUIRED)
find_package(musllibc REQUIRED)
find_package(util_libs REQUIRED)
find_package(seL4_libs REQUIRED)
find_package(projects_libs REQUIRED)
find_package(sel4_projects_libs REQUIRED)
#find_package(seL4_projects_libs REQUIRED)

# Import build graph
sel4_import_kernel()
elfloader_import_project()

# Runtime/ABI setup for musl + libc + compiler/linker flags
musllibc_setup_build_environment_with_sel4runtime()
sel4_import_libsel4()

util_libs_import_libraries()
seL4_libs_import_libraries()
message(STATUS "projects_libs_FOUND=${projects_libs_FOUND}")
if(projects_libs_FOUND)
    projects_libs_import_libraries()
endif()
message(STATUS "sel4_projects_libs_FOUND=${sel4_projects_libs_FOUND}")
if(sel4_projects_libs_FOUND)
    sel4_projects_libs_import_libraries()
endif()

add_subdirectory(my-vmm)
