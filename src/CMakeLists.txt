add_executable(Pong pong.cpp)

add_dependencies(Pong shaders)

add_subdirectory(engine)
add_subdirectory(utils)
add_subdirectory(platform)

target_compile_definitions(
  Pong
  PRIVATE NOMINMAX
          WIN32_LEAN_AND_MEAN
          APP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          APP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          APP_VERSION_PATCH=${PROJECT_VERSION_PATCH})

add_library(ponglib STATIC)
target_link_libraries(ponglib PUBLIC engine platform utils)

target_link_libraries(Pong PRIVATE Vulkan::Vulkan ponglib)
target_include_directories(
  Pong
  PUBLIC ${Vulkan_INCLUDE_DIRS}
         ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/platform
         ${CMAKE_CURRENT_SOURCE_DIR}/utils
         ${CMAKE_CURRENT_SOURCE_DIR}/engine
         ${CMAKE_CURRENT_SOURCE_DIR}/engine/vulkan)
