cmake_minimum_required(VERSION 3.20)
project(Pong VERSION 0.0.1)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

option(BUILD_TESTING "Build tests" ON)

if(MSVC)
  add_compile_options(/WX /W4)
endif()

add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)

find_package(Vulkan REQUIRED)

add_subdirectory(assets/shaders)
add_subdirectory(src)

if(BUILD_TESTING)
  FetchContent_Declare(
    google_test
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.17.0)

  enable_testing()
  include(CTest)

  add_subdirectory(tests)
endif()
