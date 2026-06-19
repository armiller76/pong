add_library(
  vulkan
  STATIC
)

target_sources(
  vulkan
  PRIVATE vulkan_depth_buffer.cpp
          vulkan_descriptor_pool.cpp
          vulkan_device.cpp
          vulkan_frame_command_context.cpp
          vulkan_gpu_buffer.cpp
          vulkan_gpu_image.cpp
          vulkan_immediate_command_context.cpp
          vulkan_instance.cpp
          vulkan_pipeline_manager.cpp
          vulkan_renderer.cpp
          vulkan_surface.cpp
          vulkan_swapchain.cpp
)

target_compile_definitions(
  vulkan
  PRIVATE VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
          MAX_DIR_LIGHTS=${PONG_MAX_DIR_LIGHTS}
)

target_include_directories(
  vulkan
  PUBLIC ${PROJECT_SOURCE_DIR}/src
         ${PROJECT_SOURCE_DIR}/third-party
         ${imgui_SOURCE_DIR}
         ${imgui_SOURCE_DIR}/backends
         ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(vulkan PUBLIC Vulkan::Vulkan)
