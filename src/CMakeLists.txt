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
  PRIVATE NOMINMAX
          WIN32_LEAN_AND_MEAN
          APP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
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
         ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/core
         ${CMAKE_CURRENT_SOURCE_DIR}/engine
         ${CMAKE_CURRENT_SOURCE_DIR}/engine/vulkan
         ${CMAKE_CURRENT_SOURCE_DIR}/graphics
         ${CMAKE_CURRENT_SOURCE_DIR}/platform
         ${CMAKE_CURRENT_SOURCE_DIR}/utils
         ${PROJECT_SOURCE_DIR}/third-party
)
