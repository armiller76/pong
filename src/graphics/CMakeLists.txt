add_library(
  graphics
  STATIC
)

target_sources(graphics PRIVATE mesh.cpp)

target_link_libraries(graphics PUBLIC glm::glm)
target_include_directories(
  graphics
  PRIVATE ${CMAKE_SOURCE_DIR}/src
          ${Vulkan_INCLUDE_DIRS}
)
