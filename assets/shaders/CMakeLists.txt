find_program(
  GLSLC
  glslc
  HINTS $ENV{VULKAN_SDK}/Bin
        $ENV{VULKAN_SDK}/bin
)
if(NOT
   GLSLC
)
  message(FATAL_ERROR "glslc not found")
endif()

file(
  GLOB_RECURSE
  SHADER_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.frag"
)

set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SHADER_BINARIES "")
foreach(SHADER_SOURCE ${SHADER_SOURCES})
  get_filename_component(
    SHADER_NAME
    ${SHADER_SOURCE}
    NAME
  )
  string(
    REPLACE "."
            "_"
            SHADER_OUTPUT_NAME
            ${SHADER_NAME}
  )
  set(SHADER_OUTPUT "${SHADER_OUTPUT_DIR}/${SHADER_OUTPUT_NAME}.spv")

  add_custom_command(
    OUTPUT ${SHADER_OUTPUT}
    COMMAND ${GLSLC} ${SHADER_SOURCE} -o ${SHADER_OUTPUT}
            -DMAX_DIR_LIGHTS=${PONG_MAX_DIR_LIGHTS}
    DEPENDS ${SHADER_SOURCE}
    COMMENT "Compiling shader: ${SHADER_NAME}"
    VERBATIM
  )

  list(
    APPEND
    SHADER_BINARIES
    ${SHADER_OUTPUT}
  )
endforeach()

add_custom_target(
  shaders
  ALL
  DEPENDS ${SHADER_BINARIES}
)
