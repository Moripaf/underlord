cmake_minimum_required(VERSION 3.16)
# Project may live outside the seL4 checkout.
# Pass -DSEL4_ROOT=/abs/path/to/this/repo when configuring.
#set(SEL4_ROOT "/home/moripaf/Code/sel4/tutorials" CACHE PATH "Path to seL4 checkout")
if(NOT DEFINED SEL4_ROOT)
    message(FATAL_ERROR "Set SEL4_ROOT to the path of your seL4 checkout")
endif()

project(Underlord)

option(VMM_FAULT_TEST "Make the Phase 1 VMM deliberately fault after startup" OFF)
option(UNDERLORD_BUILD_TESTS "Build Underlord host and seL4 test targets" OFF)
set(UNDERLORD_LOG_LEVEL "INFO" CACHE STRING "Minimum Underlord log level (TRACE, DEBUG, INFO, WARN, or ERROR)")
set_property(CACHE UNDERLORD_LOG_LEVEL PROPERTY STRINGS TRACE DEBUG INFO WARN ERROR)
set(_underlord_log_levels TRACE DEBUG INFO WARN ERROR)
if(NOT UNDERLORD_LOG_LEVEL IN_LIST _underlord_log_levels)
    message(FATAL_ERROR "UNDERLORD_LOG_LEVEL must be one of: TRACE, DEBUG, INFO, WARN, ERROR")
endif()

# Phase 2 deliberately consumes an externally built Unikraft artifact.  Do
# not build or alter the catalog application from this project.
set(UNIKRAFT_GUEST_ELF "/var/Code/final-proj/catalog-core/c-hello-arm/workdir/build/c-hello-arm_qemu-arm64" CACHE FILEPATH "AArch64 QEMU-virt Unikraft c-hello ELF")
set(UNIKRAFT_GUEST_CONFIG "/var/Code/final-proj/catalog-core/c-hello-arm/.config" CACHE FILEPATH "Final .config used for UNIKRAFT_GUEST_ELF")
if(NOT UNDERLORD_BUILD_TESTS)
    if(NOT EXISTS "${UNIKRAFT_GUEST_ELF}" OR NOT EXISTS "${UNIKRAFT_GUEST_CONFIG}")
        message(FATAL_ERROR "Phase 2 requires UNIKRAFT_GUEST_ELF and UNIKRAFT_GUEST_CONFIG files")
    endif()
    include(${CMAKE_SOURCE_DIR}/cmake/validate_unikraft_config.cmake)
    underlord_validate_unikraft_config("${UNIKRAFT_GUEST_CONFIG}")
endif()

# elfloader strips AArch64 kernel and root-task ELF files; use the matching
# cross-binutils tool instead of the host strip binary.
find_program(SEL4_CROSS_STRIP NAMES aarch64-linux-gnu-strip REQUIRED)
set(CMAKE_STRIP "${SEL4_CROSS_STRIP}" CACHE FILEPATH "Target strip tool" FORCE)

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

# Enable kernel debug printing so root-task printf can use seL4_DebugPutChar.
include(application_settings)
ApplyCommonReleaseVerificationSettings(${RELEASE} ${VERIFICATION})
set(LibSel4PlatSupportUseDebugPutChar ON CACHE BOOL "" FORCE)

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

# Shared utilities are linked by both independently running components.
add_subdirectory(underlord-utils)

# Build the child ELF before the rootserver packages it into its CPIO archive.
add_subdirectory(my-vmm)
if(UNDERLORD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
else()
    add_subdirectory(hypervisor)
endif()
