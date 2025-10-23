target_sources(
  engine PRIVATE vulkan_instance.cpp vulkan_surface.cpp vulkan_device.cpp
                 vulkan_swapchain.cpp vulkan_command_context.cpp)
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR}/src)
