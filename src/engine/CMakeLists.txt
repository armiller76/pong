add_library(
  engine
  STATIC
)

add_subdirectory(vulkan)

target_sources(
  engine
  PRIVATE file.cpp
          render_context.cpp
          resource_loader.cpp
)

target_include_directories(
  engine
  PUBLIC ${PROJECT_SOURCE_DIR}/src
         ${PROJECT_SOURCE_DIR}/third-party
         ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(
  engine
  PUBLIC glm::glm
         imgui
  PRIVATE mikktspace
          stb_image
)

if(ENABLE_SPIRV_VALIDATION)
  target_link_libraries(engine PUBLIC spv_tools)
  target_compile_definitions(
    engine
    PUBLIC MAX_DIR_LIGHTS=${PONG_MAX_DIR_LIGHTS}
           PONG_ENABLE_SPIRV_VALIDATION=1
  )
endif()
