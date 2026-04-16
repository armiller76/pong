target_sources(fastgltf PRIVATE fastgltf_wrapper.cpp)

target_include_directories(
  fastgltf
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${glm_SOURCE_DIR}
)
