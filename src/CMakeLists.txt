add_executable(Pong pong.cpp)

target_compile_definitions(
  Pong
  PRIVATE NOMINMAX
          WIN32_LEAN_AND_MEAN
          APP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          APP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          APP_VERSION_PATCH=${PROJECT_VERSION_PATCH})

target_link_libraries(Pong PRIVATE Vulkan::Vulkan)
target_include_directories(Pong PRIVATE ${Vulkan_INCLUDE_DIRS}
                                        "$ENV{VULKAN_SDK}/Include")
