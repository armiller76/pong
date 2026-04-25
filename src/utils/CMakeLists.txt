add_library(
  utils
  STATIC
)

target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_sources(utils PRIVATE exception.cpp)
