add_library(utils STATIC)

target_sources(utils PUBLIC exception.cpp)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
