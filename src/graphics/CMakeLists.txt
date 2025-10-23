add_library(graphics STATIC)

target_sources(graphics PRIVATE glm_wrapper.cpp mesh.cpp)

target_link_libraries(graphics PUBLIC glm::glm)
