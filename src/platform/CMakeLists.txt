add_library(
  platform
  STATIC
)

target_sources(platform PRIVATE win32_window.cpp)
target_include_directories(
  platform
  PRIVATE ${CMAKE_SOURCE_DIR}/src
          ${Vulkan_INCLUDE_DIRS}
)
