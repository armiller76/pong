add_library(
  graphics
  STATIC
)

target_sources(
  graphics
  PRIVATE camera.cpp
          mesh.cpp
          texture2d.cpp
          texture2d_impl_vulkan.cpp
)

target_link_libraries(graphics PUBLIC glm::glm)
target_include_directories(
  graphics
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${Vulkan_INCLUDE_DIRS}
)
