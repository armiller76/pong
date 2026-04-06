FetchContent_MakeAvailable(google_test)

add_executable(
  unit_tests
  entity_tests.cpp
  error_tests.cpp
  exception_tests.cpp
  file_tests.cpp
  hash_tests.cpp
  render_sort_key_tests.cpp
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
         platform
         utils
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
