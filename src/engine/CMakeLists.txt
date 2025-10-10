add_library(engine STATIC)

target_sources(engine PRIVATE vulkan_instance.cpp vulkan_surface.cpp
                              vulkan_device.cpp vulkan_swapchain.cpp)
target_include_directories(engine PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
