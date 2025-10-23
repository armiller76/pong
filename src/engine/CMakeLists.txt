add_library(engine STATIC)

add_subdirectory(vulkan)

target_sources(engine PRIVATE file.cpp)

target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR}/src
                                         ${Vulkan_INCLUDE_DIRS})
target_link_libraries(engine PUBLIC glm::glm)
