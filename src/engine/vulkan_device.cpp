#include "vulkan_device.h"

#include <array>
#include <cstdint>
#include <ranges>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan_instance.h"
#include "engine/vulkan_surface.h"
#include "utils/error.h"

namespace pong
{

VulkanDevice::VulkanDevice(const VulkanInstance &instance, const VulkanSurface &surface)
    : physical_device_({})
    , device_({})
    , graphics_queue_family_index_(0)
    , present_queue_family_index_(0)
{
    constexpr std::array<const char *, 2> required_device_extensions = {
        ::vk::KHRSwapchainExtensionName, ::vk::KHRSynchronization2ExtensionName};

    auto available_devices = instance.get().enumeratePhysicalDevices();
    arm::ensure(!available_devices.empty(), "No available graphics devices");

    auto check_device = std::ranges::find_if(
        available_devices,
        [&](const ::vk::raii::PhysicalDevice &check)
        {
            auto has_extensions = true;
            auto check_device_extensions = check.enumerateDeviceExtensionProperties();
            for (const auto &required_extension : required_device_extensions)
            {
                auto this_check_extension = std::ranges::find_if(
                    check_device_extensions,
                    [required_extension](const ::vk::ExtensionProperties &check_extension)
                    { return std::string_view(required_extension) == check_extension.extensionName; });
                has_extensions = has_extensions && this_check_extension != check_device_extensions.end();
                if (!has_extensions)
                {
                    return false;
                }
            }

            auto has_graphics = false;
            auto has_present = false;
            auto graphics_index = std::uint32_t{UINT32_MAX};
            auto present_index = std::uint32_t{UINT32_MAX};
            auto check_device_qf_properties = check.getQueueFamilyProperties();
            for (std::uint32_t i = 0; i < check_device_qf_properties.size(); ++i)
            {
                has_graphics = (check_device_qf_properties[i].queueFlags & ::vk::QueueFlagBits::eGraphics) !=
                               static_cast<::vk::QueueFlags>(0);
                if (has_graphics)
                {
                    graphics_index = i;
                }

                has_present = surface.get_present_support(check, i);
                if (has_present)
                {
                    present_index = i;
                }
            }

            // also check for discrete gpu and give it a high "score"?
        });

    if (check_device == available_devices.end())
    {
        throw arm::Exception("Unable to find a suitable graphics device");
    }
}

auto VulkanDevice::get() const -> const ::vk::raii::Device &
{
}
auto VulkanDevice::graphics_queue_index() const -> std::uint32_t
{
}
auto VulkanDevice::present_queue_index() const -> std::uint32_t
{
}

}
