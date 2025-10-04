cmake_minimum_required(VERSION 3.20)
project(Pong VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if(MSVC)
  add_compile_options(/WX /W4)
endif()

find_package(Vulkan REQUIRED)

add_subdirectory(src)
