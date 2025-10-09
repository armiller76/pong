#include "vulkan_device.h"

#include <array>
#include <cstdint>
#include <ranges>
#include <set>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan_instance.h"
#include "engine/vulkan_surface.h"
#include "utils/error.h"

namespace pong
{

static constexpr uint32_t DISCRETE_GPU_BONUS = 500;
static constexpr uint32_t INTEGRATED_GPU_BONUS = 250;
static constexpr uint32_t COMBINED_QUEUE_BONUS = 50;
static constexpr uint32_t SEPARATE_QUEUE_BONUS = 25;

VulkanDevice::VulkanDevice(const VulkanInstance &instance, const VulkanSurface &surface)
    : physical_device_({})
    , device_({})
    , graphics_queue_family_index_(0)
    , present_queue_family_index_(0)
{
    static constexpr std::array<const char *, 2> required_device_extensions = {
        ::vk::KHRSwapchainExtensionName, ::vk::KHRSynchronization2ExtensionName};

    const auto available_devices = instance.get().enumeratePhysicalDevices();
    arm::ensure(!available_devices.empty(), "No available graphics devices");

    auto device_infos = std::vector<VulkanDeviceInfo>();
    for (const auto &physical_device : available_devices)
    {
        auto vulkan_device_info = VulkanDeviceInfo{&physical_device, 0, UINT32_MAX, UINT32_MAX};

        auto has_extensions = true;
        const auto check_device_extensions = physical_device.enumerateDeviceExtensionProperties();
        for (const auto &required_extension : required_device_extensions)
        {
            const auto this_check_extension = std::ranges::find_if(
                check_device_extensions,
                [required_extension](const ::vk::ExtensionProperties &check_extension)
                { return std::string_view(required_extension) == check_extension.extensionName; });
            has_extensions = has_extensions && this_check_extension != check_device_extensions.end();
        }
        if (has_extensions)
        {
            score_device(vulkan_device_info, surface);
            device_infos.push_back(vulkan_device_info);
        }
    }

    std::ranges::sort(device_infos, std::greater{}, &VulkanDeviceInfo::score);

    arm::ensure(!device_infos.empty(), "Unable to find a suitable graphics device");

    physical_device_ = ::vk::raii::PhysicalDevice(*device_infos[0].physical_device);
    graphics_queue_family_index_ = device_infos[0].graphics_index;
    present_queue_family_index_ = device_infos[0].present_index;

    arm::ensure(graphics_queue_family_index_ != UINT32_MAX, "No graphics queue family found");
    arm::ensure(present_queue_family_index_ != UINT32_MAX, "No present queue family found");

    auto queue_priority = float{1.0f};
    auto queue_create_infos = std::vector<vk::DeviceQueueCreateInfo>{};
    std::set<uint32_t> queue_families_set = {graphics_queue_family_index_, present_queue_family_index_};
    for (uint32_t queue_family : queue_families_set)
    {
        queue_create_infos.emplace_back(
            vk::DeviceQueueCreateFlags{},
            queue_family,
            1, // queue count
            &queue_priority);
    }

    vk::PhysicalDeviceFeatures device_features{};
    vk::DeviceCreateInfo device_create_info{{}, queue_create_infos, {}, required_device_extensions, &device_features};
    device_ = vk::raii::Device(physical_device_, device_create_info);
    graphics_queue_ = device_.getQueue(graphics_queue_family_index_, 0);
    present_queue_ = device_.getQueue(present_queue_family_index_, 0);
}

auto VulkanDevice::get() const -> const ::vk::raii::Device &
{
    return device_;
}

auto VulkanDevice::graphics_queue() const -> ::vk::Queue
{
    return graphics_queue_;
}

auto VulkanDevice::graphics_queue_index() const -> std::uint32_t
{
    return graphics_queue_family_index_;
}

auto VulkanDevice::present_queue() const -> ::vk::Queue
{
    return present_queue_;
}

auto VulkanDevice::present_queue_index() const -> std::uint32_t
{
    return present_queue_family_index_;
}

auto VulkanDevice::score_device(VulkanDeviceInfo &info, const VulkanSurface &surface) -> void
{
    const auto queue_family_properties = info.physical_device->getQueueFamilyProperties();

    auto graphics_index = UINT32_MAX;
    auto present_index = UINT32_MAX;
    auto combined_index = UINT32_MAX;

    for (std::uint32_t i = 0; i < queue_family_properties.size(); ++i)
    {
        const auto has_graphics = (queue_family_properties[i].queueFlags & ::vk::QueueFlagBits::eGraphics) !=
                                  static_cast<::vk::QueueFlags>(0);
        const auto has_present = surface.get_present_support(*info.physical_device, i);

        if (has_graphics && graphics_index == UINT32_MAX)
            graphics_index = i;
        if (has_present && present_index == UINT32_MAX)
            present_index = i;
        if (has_graphics && has_present && combined_index == UINT32_MAX)
        {
            combined_index = i;
            break;
        }
    }

    if (combined_index != UINT32_MAX)
    {
        info.graphics_index = combined_index;
        info.present_index = combined_index;
        info.score += COMBINED_QUEUE_BONUS;
    }
    else if (graphics_index != UINT32_MAX && present_index != UINT32_MAX)
    {
        info.graphics_index = graphics_index;
        info.present_index = present_index;
        info.score += SEPARATE_QUEUE_BONUS;
    }

    const auto device_properties = info.physical_device->getProperties();
    if (device_properties.deviceType == ::vk::PhysicalDeviceType::eDiscreteGpu)
    {
        info.score += DISCRETE_GPU_BONUS;
    }
    else if (device_properties.deviceType == ::vk::PhysicalDeviceType::eIntegratedGpu)
    {
        info.score += INTEGRATED_GPU_BONUS;
    }

    const auto device_memory_properties = info.physical_device->getMemoryProperties();
    auto largest_heap_size = ::vk::DeviceSize{0};
    for (std::uint32_t i = 0; i < device_memory_properties.memoryHeapCount; ++i)
    {
        const auto memory_heap = device_memory_properties.memoryHeaps[i];
        if (memory_heap.flags & ::vk::MemoryHeapFlagBits::eDeviceLocal)
        {
            largest_heap_size = memory_heap.size > largest_heap_size ? memory_heap.size : largest_heap_size;
        }
    }
    info.score += static_cast<uint32_t>(largest_heap_size / (256 * 1024 * 1024));
}

} // namespace pong
