add_library(
  imgui
  STATIC
)

target_sources(imgui PRIVATE imgui_wrapper.cpp)

target_include_directories(
  imgui
  PUBLIC ${PROJECT_SOURCE_DIR}/src
         ${PROJECT_SOURCE_DIR}/third-party
         ${imgui_SOURCE_DIR}
         ${imgui_SOURCE_DIR}/backends
         ${Vulkan_INCLUDE_DIRS}
)
target_sources(
  imgui
  PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
          ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
          ${imgui_SOURCE_DIR}/imgui.cpp
          ${imgui_SOURCE_DIR}/imgui_draw.cpp
          ${imgui_SOURCE_DIR}/imgui_tables.cpp
          ${imgui_SOURCE_DIR}/imgui_widgets.cpp
          ${imgui_SOURCE_DIR}/imgui_demo.cpp
)
