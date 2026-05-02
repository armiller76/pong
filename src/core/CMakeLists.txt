add_library(
  core
  STATIC
)

target_sources(
  core
  PRIVATE entity.cpp
          scene.cpp
)

target_compile_definitions(core PUBLIC MAX_DIR_LIGHTS=${PONG_MAX_DIR_LIGHTS})

# target_link_libraries(core PUBLIC )
target_include_directories(
  core
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${Vulkan_INCLUDE_DIRS}
)
