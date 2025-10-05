FetchContent_MakeAvailable(google_test)

add_executable(unit_tests unit_tests.cpp)

target_include_directories(unit_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(unit_tests gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)
