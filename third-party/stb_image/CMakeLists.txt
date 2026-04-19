add_library(
  stb_image
  STATIC
)

target_sources(stb_image PRIVATE stb_image_impl.cpp)

target_include_directories(
  stb_image
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${CMAKE_CURRENT_SOURCE_DIR}
)
