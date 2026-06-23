#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "engine/input_state.h"
#include "engine/render_context.h"
#include "engine/vulkan/vulkan_instance.h"
#include "platform/win32_window.h"

namespace pong
{

class ImguiWrapper;

struct Version
{
    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
};

struct ApplicationInfo
{
    std::string application_name;
    std::string engine_name;
    std::filesystem::path application_root_dir;
    Version version;
};

class Application
{
  public:
    Application(ApplicationInfo info);
    ~Application();

    auto run() -> void;

  private:
    ApplicationInfo app_info_;
    RenderContextInfo render_info_;

    InputState input_state_;
    Win32Window win32_window_;
    VulkanInstance vulkan_instance_;
    RenderContext render_context_;

    std::unique_ptr<ImguiWrapper> debug_renderer_;
};

}
