add_library(
  platform
  STATIC
)

target_sources(platform PRIVATE win32_window.cpp)

target_include_directories(
  platform
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${Vulkan_INCLUDE_DIRS}
          ${PROJECT_SOURCE_DIR}/build/_deps/imgui-src
)

target_link_libraries(platform PUBLIC imgui)
