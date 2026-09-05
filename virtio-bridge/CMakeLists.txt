cmake_minimum_required(VERSION 3.16.0)
project(virtio_bridge C)

add_executable(${PROJECT_NAME} src/main.c)
target_include_directories(${PROJECT_NAME} PRIVATE include)

add_library(virtio_bridge_protocol STATIC src/virtio_bridge_protocol.c src/virtio_mmio_model.c src/virtio_queue_validate.c)
target_include_directories(virtio_bridge_protocol PUBLIC include)
target_link_libraries(${PROJECT_NAME} PRIVATE virtio_bridge_protocol sel4runtime sel4 muslc sel4muslcsys sel4utils)
