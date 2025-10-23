#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

#include "utils/util.h"

namespace pong
{
class VulkanInstance;
class VulkanSurface;

class VulkanWindow
{
  public:
    virtual ~VulkanWindow() = default;

    virtual auto process_events() -> void = 0;
    virtual auto should_close() const -> bool = 0;

    virtual auto size_pixels() const -> Size = 0;
    virtual auto set_title(std::string_view title) -> void = 0;

    virtual auto create_vulkan_surface(const VulkanInstance &instance) const -> VulkanSurface = 0;

    virtual auto add_close_callback(std::function<void()>) -> std::uint64_t = 0;
    virtual auto remove_close_callback(std::uint64_t) -> void = 0;

    virtual auto add_resize_callback(std::function<void(std::uint32_t, std::uint32_t)>) -> std::uint64_t = 0;
    virtual auto remove_resize_callback(std::uint64_t) -> void = 0;
};

}
