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
