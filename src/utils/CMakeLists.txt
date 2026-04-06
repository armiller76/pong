add_library(
  utils
  STATIC
)

target_sources(utils PRIVATE exception.cpp)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
