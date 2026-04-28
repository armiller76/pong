FetchContent_MakeAvailable(google_test)

# gtest triggers -Wcharacter-conversion under clang-cl; suppress on third-party
# targets. Note: clang-cl sets MSVC=true, so check compiler ID explicitly.
if(CMAKE_CXX_COMPILER_ID
   STREQUAL
   "Clang"
)
  foreach(
    _t
    gtest
    gtest_main
    gmock
    gmock_main
  )
    target_compile_options(
      ${_t}
      PRIVATE /WX-
              -Wno-error
              -Wno-character-conversion
    )
  endforeach()
endif()

add_executable(
  unit_tests
  auto_release_tests.cpp
  camera_tests.cpp
  engine_error_tests.cpp
  entity_tests.cpp
  error_tests.cpp
  exception_tests.cpp
  file_tests.cpp
  hash_tests.cpp
  image_texture_resource_api_tests.cpp
  pipeline_types_tests.cpp
  transform_tests.cpp
  window_tests.cpp
)

target_include_directories(unit_tests PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(
  unit_tests
  PUBLIC gtest
         gtest_main
         core
         engine
         graphics
         platform
         utils
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
