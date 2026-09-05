#
# Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

# Define our top level settings.  Whilst they have doc strings for readability
# here, they are hidden in the cmake-gui as they cannot be reliably changed
# after the initial configuration.  Enterprising users can still change them if
# they know what they are doing through advanced mode.
#
# Users should initialize a build directory by doing something like:
#
# mkdir build_sabre
# cd build_sabre
#
# Then
#
# ../griddle --PLATFORM=sabre --SIMULATION
# ninja
#
# Underlord project options
# Enable the deliberate post-startup fault path used to exercise VMM fault supervision.
option(VMM_FAULT_TEST "Make the Phase 1 VMM deliberately fault after startup" OFF)
# Build host-side tests and seL4 test targets instead of requiring bundled guest artifacts.
option(UNDERLORD_BUILD_TESTS "Build Underlord host and seL4 test targets" OFF)
# Select which bundled Unikraft hello guest the normal hypervisor startup launches.
set(UNDERLORD_GUEST "c-fs" CACHE STRING "Bundled guest selected for the normal hypervisor startup")
set_property(CACHE UNDERLORD_GUEST PROPERTY STRINGS c-hello cpp-hello c-fs)
# Keep the accepted log severities available to configure-time validation.
set(_underlord_log_levels TRACE DEBUG INFO WARN ERROR CACHE INTERNAL "Underlord log levels")
# Select the minimum severity emitted by Underlord's structured logging macros.
set(UNDERLORD_LOG_LEVEL "INFO" CACHE STRING "Minimum Underlord log level (TRACE, DEBUG, INFO, WARN, or ERROR)")
set_property(CACHE UNDERLORD_LOG_LEVEL PROPERTY STRINGS TRACE DEBUG INFO WARN ERROR)
# Use the externally built C hello ELF image for packaging and ELF admission checks.
set(UNDERLORD_C_GUEST_ELF "/var/Code/final-proj/catalog-arm/c-hello/workdir/build/c-hello-arm_qemu-arm64" CACHE FILEPATH "AArch64 QEMU-virt Unikraft c-hello ELF")
# Use the final Unikraft configuration that corresponds to the C hello ELF image.
set(UNDERLORD_C_GUEST_CONFIG "/var/Code/final-proj/catalog-arm/c-hello/.config" CACHE FILEPATH "Final .config used for c-hello ELF")
# Use the externally built C++ hello ELF image for packaging and ELF admission checks.
set(UNDERLORD_CPP_GUEST_ELF "/var/Code/final-proj/catalog-arm/cpp-hello/workdir/build/cpp-hello_qemu-arm64" CACHE FILEPATH "AArch64 QEMU-virt Unikraft cpp-hello ELF")
# Use the final Unikraft configuration that corresponds to the C++ hello ELF image.
set(UNDERLORD_CPP_GUEST_CONFIG "/var/Code/final-proj/catalog-arm/cpp-hello/.config" CACHE FILEPATH "Final .config used for cpp-hello ELF")
# Use the externally built C filesystem ELF image for packaging and ELF admission checks.
set(UNDERLORD_C_FS_GUEST_ELF "/var/Code/final-proj/catalog-arm/c-fs/workdir/build/c-fs_qemu-arm64" CACHE FILEPATH "AArch64 QEMU-virt Unikraft c-fs ELF")
# Use the final Unikraft configuration that embeds the c-fs CPIO root filesystem.
set(UNDERLORD_C_FS_GUEST_CONFIG "/var/Code/final-proj/catalog-arm/c-fs/.config" CACHE FILEPATH "Final .config used for c-fs ELF")

set(SIMULATION OFF CACHE BOOL "Include only simulation compatible tests")
set(RELEASE OFF CACHE BOOL "Performance optimized build")
set(VERIFICATION OFF CACHE BOOL "Only verification friendly kernel features")
set(BAMBOO OFF CACHE BOOL "Enable machine parseable output")
set(DOMAINS OFF CACHE BOOL "Test multiple domains")
set(SMP OFF CACHE BOOL "(if supported) Test SMP kernel")
set(NUM_NODES "" CACHE STRING "(if SMP) the number of nodes (default 4)")
#set(PLATFORM "x86_64" CACHE STRING "Platform to test")
#set(KernelVTX ON CACHE BOOL "enable virtualization")
set(PLATFORM "qemu-arm-virt" CACHE STRING "Platform to test")
set(ARM_HYP ON CACHE BOOL "Hyp mode for ARM platforms")
set(KernelArmHypervisorSupport ON CACHE BOOL "Hyp mode for ARM platforms")
set(KernelRootCNodeSizeBits "13" CACHE STRING "Root CNode size for bundled guest image frame caps" FORCE)
set(MCS OFF CACHE BOOL "MCS kernel")
set(KernelSel4Arch "aarch64" CACHE STRING "aarch32, aarch64, arm_hyp, ia32, x86_64, riscv32, riscv64")
set(LibSel4TestPrinterRegex ".*" CACHE STRING "A POSIX regex pattern used to filter tests")
set(LibSel4TestPrinterHaltOnTestFailure OFF CACHE BOOL "Halt on the first test failure")
set(KernelPlatform "qemu-arm-virt" CACHE PATH "Default kernel platform")
set(CROSS_COMPILER_PREFIX "aarch64-linux-gnu-" CACHE STRING "Cross compiler")
# Visible to GenerateSimulateScript() (kernel plat config sets these only in a subdirectory scope).
set(QEMU_ARCH "aarch64" CACHE STRING "QEMU binary arch suffix for qemu-arm-virt")
# The fixed Phase-2 VMM needs one 2^28 non-device boot untyped.  With the
# rootserver's physical placement, qemu-arm-virt exposes at most 2^27 at 1 GiB.
set(QEMU_MEMORY "2048" CACHE STRING "QEMU RAM size in MiB for simulate")
set(QEMU_GIC_VERSION "2" CACHE STRING "QEMU GIC version for simulate")
# Route early printf through seL4_DebugPutChar (requires KernelDebugBuild/KernelPrinting).
set(LibSel4PlatSupportUseDebugPutChar ON CACHE BOOL "" FORCE)
mark_as_advanced(CLEAR LibSel4TestPrinterRegex LibSel4TestPrinterHaltOnTestFailure)
