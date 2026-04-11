target_sources(
  engine
  PRIVATE vulkan_depth_buffer.cpp
          vulkan_gpu_buffer.cpp
          vulkan_command_context.cpp
          vulkan_descriptor_pool.cpp
          vulkan_device.cpp
          vulkan_instance.cpp
          vulkan_pipeline_factory.cpp
          vulkan_renderer.cpp
          vulkan_surface.cpp
          vulkan_swapchain.cpp
)
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
