add_library(
  mikktspace
  STATIC
)

target_sources(
  mikktspace
  PRIVATE mikktspace_impl.cpp
          mikktspace.c
)

set_source_files_properties(
  mikktspace.c
  PROPERTIES COMPILE_OPTIONS
             "-Wno-unused-but-set-variable"
)

target_include_directories(
  mikktspace
  PRIVATE ${PROJECT_SOURCE_DIR}/src
          ${PROJECT_SOURCE_DIR}/third-party
          ${glm_SOURCE_DIR}
)
