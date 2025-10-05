FetchContent_MakeAvailable(google_test)

add_executable(unit_tests window_tests.cpp vulkan_surface_tests.cpp)

target_include_directories(unit_tests PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(unit_tests PUBLIC gtest gtest_main engine platform utils)

include(GoogleTest)
gtest_discover_tests(unit_tests)
