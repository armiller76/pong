add_library(engine STATIC)

add_subdirectory(vulkan)

target_sources(engine PRIVATE file.cpp)
