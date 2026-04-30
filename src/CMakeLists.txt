add_executable(pong pong.cpp)

add_dependencies(
  pong
  shaders
)

add_subdirectory(core)
add_subdirectory(engine)
add_subdirectory(graphics)
add_subdirectory(platform)
add_subdirectory(utils)

target_compile_definitions(
  pong
  PRIVATE APP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          APP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          APP_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

add_library(
  ponglib
  INTERFACE
)
target_link_libraries(
  ponglib
  INTERFACE core
            engine
            graphics
            imgui
            platform
            utils
)

target_link_libraries(
  pong
  PRIVATE Vulkan::Vulkan
          ponglib
          fastgltf
)
target_include_directories(
  pong
  PUBLIC ${Vulkan_INCLUDE_DIRS}
         ${PROJECT_SOURCE_DIR}/src
         ${PROJECT_SOURCE_DIR}/third-party
)
