add_library(
  graphics
  STATIC
)

target_sources(
  graphics
  PRIVATE camera.cpp
          mesh.cpp
          texture2d.cpp
)

target_compile_definitions(
  graphics PUBLIC MAX_DIR_LIGHTS=${PONG_MAX_DIR_LIGHTS}
)

target_link_libraries(graphics PUBLIC glm::glm)

target_include_directories(
  graphics
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${Vulkan_INCLUDE_DIRS}
)
