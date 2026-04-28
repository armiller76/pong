cmake_minimum_required(VERSION 3.20)
project(pong VERSION 0.0.1)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

option(
  BUILD_TESTING
  "Build tests"
  ON
)

option(
  ENABLE_SPIRV_VALIDATION
  "Enable SPIR-V validation via SPIRV-Tools"
  ON
)

if(MSVC)
  add_compile_options(
    /WX
    /W4
  )
endif()

add_compile_definitions(
  NOMINMAX
  WIN32_LEAN_AND_MEAN
  VK_USE_PLATFORM_WIN32_KHR
  VULKAN_HPP_NO_EXCEPTIONS
)

find_package(
  Vulkan
  REQUIRED
)

if(ENABLE_SPIRV_VALIDATION)
  # Prefer local SPIRV-Tools from the installed Vulkan SDK.
  find_path(
    PONG_SPIRV_TOOLS_INCLUDE_DIR
    NAMES spirv-tools/libspirv.h
    HINTS "$ENV{VULKAN_SDK}/Include"
  )

  find_library(
    PONG_SPIRV_TOOLS_LIBRARY_RELEASE
    NAMES SPIRV-Tools
    HINTS "$ENV{VULKAN_SDK}/Lib"
  )

  find_library(
    PONG_SPIRV_TOOLS_LIBRARY_DEBUG
    NAMES SPIRV-Toolsd
    HINTS "$ENV{VULKAN_SDK}/Lib"
  )

  add_library(
    spv_tools
    UNKNOWN
    IMPORTED
    GLOBAL
  )
  set_target_properties(
    spv_tools
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
               "${PONG_SPIRV_TOOLS_INCLUDE_DIR}"
               IMPORTED_LOCATION_RELEASE
               "${PONG_SPIRV_TOOLS_LIBRARY_RELEASE}"
               IMPORTED_LOCATION_RELWITHDEBINFO
               "${PONG_SPIRV_TOOLS_LIBRARY_RELEASE}"
               IMPORTED_LOCATION_MINSIZEREL
               "${PONG_SPIRV_TOOLS_LIBRARY_RELEASE}"
  )

  if(PONG_SPIRV_TOOLS_LIBRARY_DEBUG)
    set_property(
      TARGET spv_tools
      PROPERTY IMPORTED_LOCATION_DEBUG
               "${PONG_SPIRV_TOOLS_LIBRARY_DEBUG}"
    )
  else()
    set_property(
      TARGET spv_tools
      PROPERTY IMPORTED_LOCATION_DEBUG
               "${PONG_SPIRV_TOOLS_LIBRARY_RELEASE}"
    )
  endif()
endif()

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.7
)
FetchContent_MakeAvailable(imgui)
add_subdirectory(third-party/imgui)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.2
)
set(GLM_BUILD_TESTS
    OFF
    CACHE BOOL
          ""
          FORCE
)
set(BUILD_STATIC_LIBS
    OFF
    CACHE BOOL
          ""
          FORCE
)
FetchContent_MakeAvailable(glm)

if(BUILD_TESTING)
  FetchContent_Declare(
    google_test
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.17.0
  )

  enable_testing()
  include(CTest)

  add_subdirectory(tests)
endif()

FetchContent_Declare(
  fastgltf
  GIT_REPOSITORY https://github.com/spnda/fastgltf.git
  GIT_TAG v0.9.0
)
FetchContent_MakeAvailable(fastgltf)
if(MSVC)
  # fastgltf bundles simdjson which triggers C4100 (unreferenced formal
  # parameter). Suppress our project-wide /WX and /W4 for this third-party
  # target.
  target_compile_options(
    fastgltf
    PRIVATE /W0
            /WX-
  )
endif()

add_subdirectory(third-party/gltf)
add_subdirectory(third-party/stb_image)

add_subdirectory(assets/shaders)
add_subdirectory(src)
