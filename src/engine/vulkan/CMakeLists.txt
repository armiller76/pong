target_sources(
  engine
  PRIVATE gpu_buffer.cpp
          vulkan_instance.cpp
          vulkan_surface.cpp
          vulkan_device.cpp
          vulkan_swapchain.cpp
          vulkan_command_context.cpp
          vulkan_pipeline_factory.cpp
)
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
