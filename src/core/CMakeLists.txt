add_library(
  core
  STATIC
)

target_sources(
  core
  PRIVATE application.cpp
          entity.cpp
          scene.cpp
)

target_compile_definitions(core PUBLIC MAX_DIR_LIGHTS=${PONG_MAX_DIR_LIGHTS})

target_link_libraries(core PUBLIC imgui)

target_include_directories(
  core
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${PROJECT_SOURCE_DIR}/third-party
          ${Vulkan_INCLUDE_DIRS}
          ${imgui_SOURCE_DIR}
          ${imgui_SOURCE_DIR}/backends
)
