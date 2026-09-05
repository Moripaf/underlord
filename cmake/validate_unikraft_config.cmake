function(underlord_validate_unikraft_config config)
    file(READ "${config}" contents)
    foreach(required
            "CONFIG_ARCH_ARM_64=y"
            "CONFIG_PLAT_KVM=y"
            "CONFIG_KVM_VMM_QEMU=y"
            "CONFIG_KVM_BOOT_PROTO_QEMU_VIRT=y"
            "CONFIG_LIBUKINTCTLR_GICV2=y"
            "CONFIG_LIBPL011=y"
            "CONFIG_LIBPL011_EARLY_CONSOLE=y")
        string(FIND "${contents}" "${required}" found)
        if(found EQUAL -1)
            message(FATAL_ERROR "Unikraft config ${config} must contain ${required}")
        endif()
    endforeach()
    string(REGEX MATCH "(^|\\n)CONFIG_SMP=y" smp_enabled "${contents}")
    if(smp_enabled)
        message(FATAL_ERROR "Unikraft config ${config} must disable SMP")
    endif()
endfunction()

function(underlord_validate_unikraft_cpp_config config)
    underlord_validate_unikraft_config("${config}")
    file(READ "${config}" contents)
    foreach(required
            "CONFIG_LIBCOMPILER_RT=y"
            "CONFIG_LIBCXX=y"
            "CONFIG_LIBCXXABI=y"
            "CONFIG_LIBUNWIND=y"
            "CONFIG_LIBMUSL=y"
            "CONFIG_FPSIMD=y")
        string(FIND "${contents}" "${required}" found)
        if(found EQUAL -1)
            message(FATAL_ERROR "Unikraft C++ config ${config} must contain ${required}")
        endif()
    endforeach()
endfunction()
