cmake_minimum_required(VERSION 3.16.0)


project(my-vmm C CXX ASM)

# Build is only used for user-mode apps.
enable_language(C)
enable_language(ASM)



add_executable(${PROJECT_NAME}
    ../${PROJECT_NAME}/src/main.c
    ../${PROJECT_NAME}/src/init.c
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
        sel4allocman
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
