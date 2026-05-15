cmake_minimum_required(VERSION 3.16.0)

# Project may live outside the seL4 checkout.
# Pass -DSEL4_ROOT=/abs/path/to/this/repo when configuring.
#set(SEL4_ROOT "/home/moripaf/Code/final-proj/vmm-on-sel4" CACHE PATH "Path to seL4 checkout")
set(SEL4_ROOT "/home/moripaf/Code/sel4/tutorials" CACHE PATH "Path to seL4 checkout")
if(NOT DEFINED SEL4_ROOT)
    message(FATAL_ERROR "Set SEL4_ROOT to the path of your seL4 checkout")
endif()

project(my-vmm C CXX ASM)

# Build is only used for user-mode apps.
enable_language(C)
enable_language(ASM)

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
find_package(sel4_projects_libs REQUIRED)
#find_package(projects_libs REQUIRED)
#find_package(seL4_projects_libs REQUIRED)

# Import build graph
sel4_import_kernel()
elfloader_import_project()

# Runtime/ABI setup for musl + libc + compiler/linker flags
musllibc_setup_build_environment_with_sel4runtime()
sel4_import_libsel4()

util_libs_import_libraries()
seL4_libs_import_libraries()
if(projects_libs_FOUND)
    projects_libs_import_libraries()
endif()
message(STATUS "sel4_projects_libs_FOUND=${sel4_projects_libs_FOUND}")
if(sel4_projects_libs_FOUND)
    sel4_projects_libs_import_libraries()
endif()

add_executable(${PROJECT_NAME}
    ../${PROJECT_NAME}/src/main.c
)

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
        sel4vmmplatsupport
        # optional user libs if needed:
        # sel4debug
        # sel4sync
)

# If you enabled projects_libs, link in chosen targets here explicitly.
# e.g.
# target_link_libraries(${PROJECT_NAME} PRIVATE sel4vmmplatsupport sel4keyboard sel4vchan)

# Root image generation (kernel + root task image under build/images)
include(${SEL4_ROOT}/tools/seL4/cmake-tool/helpers/rootserver.cmake)
DeclareRootserver(${PROJECT_NAME})

# Optional: generate ./build/simulate and ./build/launch_gdb scripts
include(${SEL4_ROOT}/tools/seL4/cmake-tool/helpers/simulation.cmake)
GenerateSimulateScript()
