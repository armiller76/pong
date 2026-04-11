#include "vulkan_device.h"

#include <array>
#include <cstdint>
#include <ranges>
#include <set>
#include <string_view>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "utils/error.h"
#include "utils/exception.h"
#include "vulkan_instance.h"
#include "vulkan_surface.h"

namespace
{

static constexpr uint32_t DISCRETE_GPU_BONUS = 500;
static constexpr uint32_t INTEGRATED_GPU_BONUS = 250;
static constexpr uint32_t COMBINED_QUEUE_BONUS = 50;
static constexpr uint32_t SEPARATE_QUEUE_BONUS = 25;

} // anonymous namespace

namespace pong
{

VulkanDevice::VulkanDevice(const VulkanInstance &instance, const VulkanSurface &surface)
    : physical_device_({})
    , device_({})
    , graphics_queue_family_index_(0)
    , present_queue_family_index_(0)
    , supports_api13_(false)
    , supports_dynamic_rendering_(false)
    , supports_sync2_(false)
{
    arm::log::debug("VulkanDevice constructor");
    arm::log::debug("Starting device selection and creation...");

    static constexpr std::array<const char *, 1> required_device_extensions_if_13 = {::vk::KHRSwapchainExtensionName};
    static constexpr std::array<const char *, 3> required_device_extensions_no_13 = {
        ::vk::KHRSwapchainExtensionName,
        ::vk::KHRSynchronization2ExtensionName,
        ::vk::KHRDynamicRenderingExtensionName};

    const auto available_devices = instance.get().enumeratePhysicalDevices();
    arm::ensure(!available_devices.empty(), "No available graphics devices");

    auto device_infos = std::vector<VulkanDeviceInfo>();
    for (const auto &physical_device : available_devices)
    {
        auto vulkan_device_info = VulkanDeviceInfo{*physical_device, 0, UINT32_MAX, UINT32_MAX, false, false, false};

        const auto device_properties = physical_device.getProperties();
        arm::log::debug(
            "Enumerated device: {} (vendorID: {:#x}, deviceID: {:#x}, type: {})",
            std::string_view(device_properties.deviceName),
            device_properties.vendorID,
            device_properties.deviceID,
            ::vk::to_string(device_properties.deviceType));

        if (device_properties.apiVersion >= VK_API_VERSION_1_3)
        {
            vulkan_device_info.supports_api13 = true;
            auto features =
                physical_device.getFeatures2<::vk::PhysicalDeviceFeatures2, ::vk::PhysicalDeviceVulkan13Features>();
            const auto &features13 = features.get<::vk::PhysicalDeviceVulkan13Features>();
            vulkan_device_info.supports_dynamic_rendering = features13.dynamicRendering == VK_TRUE;
            vulkan_device_info.supports_sync2 = features13.synchronization2 == VK_TRUE;
        }
        else // API < 1.3
        {
            vulkan_device_info.supports_api13 = false;
            auto features = physical_device.getFeatures2<
                ::vk::PhysicalDeviceFeatures2,
                ::vk::PhysicalDeviceSynchronization2Features,
                ::vk::PhysicalDeviceDynamicRenderingFeatures>();
            const auto feature_dynamic_rendering = features.get<::vk::PhysicalDeviceDynamicRenderingFeatures>();
            const auto feature_sync2 = features.get<::vk::PhysicalDeviceSynchronization2Features>();

            vulkan_device_info.supports_dynamic_rendering = feature_dynamic_rendering.dynamicRendering == VK_TRUE;
            vulkan_device_info.supports_sync2 = feature_sync2.synchronization2 == VK_TRUE;
        }

        auto has_extensions = true;
        const auto check_device_extensions = physical_device.enumerateDeviceExtensionProperties();
        if (vulkan_device_info.supports_api13)
        {
            for (const auto &required_extension : required_device_extensions_if_13)
            {
                const auto this_check_extension = std::ranges::find_if(
                    check_device_extensions,
                    [required_extension](const ::vk::ExtensionProperties &check_extension)
                    { return std::string_view(required_extension) == check_extension.extensionName; });
                has_extensions = has_extensions && this_check_extension != check_device_extensions.end();
            }
        }
        else
        {
            for (const auto &required_extension : required_device_extensions_no_13)
            {
                const auto this_check_extension = std::ranges::find_if(
                    check_device_extensions,
                    [required_extension](const ::vk::ExtensionProperties &check_extension)
                    { return std::string_view(required_extension) == check_extension.extensionName; });
                has_extensions = has_extensions && this_check_extension != check_device_extensions.end();
            }
        }

        if (has_extensions)
        {
            if (score_device_(vulkan_device_info, surface, physical_device))
            {
                // score_device_ returns true if the device has graphics and present support
                device_infos.push_back(vulkan_device_info);
            }
            arm::log::debug(
                "Scored physical device: {} (Score: {})",
                std::string_view(device_properties.deviceName),
                vulkan_device_info.score);
        }
    }

    arm::ensure(!device_infos.empty(), "Unable to find a suitable graphics device");

    std::ranges::sort(device_infos, std::greater{}, &VulkanDeviceInfo::score);

    physical_device_ = ::vk::raii::PhysicalDevice(instance.get(), device_infos[0].physical_device);
    graphics_queue_family_index_ = device_infos[0].graphics_index;
    present_queue_family_index_ = device_infos[0].present_index;
    arm::log::debug(
        "Selected physical device: {} (graphics queue index {}, present queue index {})",
        std::string_view(physical_device_.getProperties().deviceName),
        graphics_queue_family_index_,
        present_queue_family_index_);

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

    auto device_create_info = vk::DeviceCreateInfo{};
    [[maybe_unused]] auto feature_api13 = ::vk::PhysicalDeviceVulkan13Features{};
    [[maybe_unused]] auto feature_sync2 = ::vk::PhysicalDeviceSynchronization2Features{};
    [[maybe_unused]] auto feature_dynamic_rendering = ::vk::PhysicalDeviceDynamicRenderingFeatures{};

    if (device_infos[0].supports_api13)
    {
        feature_api13.synchronization2 = device_infos[0].supports_sync2 ? VK_TRUE : VK_FALSE;
        feature_api13.dynamicRendering = device_infos[0].supports_dynamic_rendering ? VK_TRUE : VK_FALSE;

        device_create_info = {{}, queue_create_infos, {}, required_device_extensions_if_13, nullptr};
        device_create_info.pNext = &feature_api13;
    }
    else
    {
        feature_dynamic_rendering.dynamicRendering = device_infos[0].supports_dynamic_rendering ? VK_TRUE : VK_FALSE;
        feature_sync2.synchronization2 = device_infos[0].supports_sync2 ? VK_TRUE : VK_FALSE;
        feature_sync2.pNext = &feature_dynamic_rendering;

        device_create_info = {{}, queue_create_infos, {}, required_device_extensions_no_13, nullptr};
        device_create_info.pNext = &feature_sync2;
    }

    device_ = vk::raii::Device(physical_device_, device_create_info);
    graphics_queue_ = device_.getQueue(graphics_queue_family_index_, 0);
    present_queue_ = device_.getQueue(present_queue_family_index_, 0);
    supports_api13_ = device_infos[0].supports_api13;
    supports_dynamic_rendering_ = device_infos[0].supports_dynamic_rendering;
    supports_sync2_ = device_infos[0].supports_sync2;

    arm::log::debug(
        "Device creation complete...\n\tAPI Version: {}\n\tDynamic Rendering: {}\n\tSynchronization2: {}",
        supports_api13_ ? ">= 1.3" : "< 1.3",
        supports_dynamic_rendering_ ? "Supported" : "Not Supported",
        supports_sync2_ ? "Supported" : "Not Supported");
}

auto VulkanDevice::native_handle() const -> const ::vk::raii::Device &
{
    return device_;
}

auto VulkanDevice::physical_device() const -> const ::vk::raii::PhysicalDevice &
{
    return physical_device_;
}

auto VulkanDevice::graphics_queue() const -> ::vk::Queue
{
    return graphics_queue_;
}

auto VulkanDevice::graphics_queue_family_index() const -> std::uint32_t
{
    return graphics_queue_family_index_;
}

auto VulkanDevice::present_queue() const -> ::vk::Queue
{
    return present_queue_;
}

auto VulkanDevice::present_queue_family_index() const -> std::uint32_t
{
    return present_queue_family_index_;
}

auto VulkanDevice::supports_dynamic_rendering() const -> bool
{
    return supports_dynamic_rendering_;
}

auto VulkanDevice::find_memory_type_index(
    const ::vk::MemoryRequirements requirements,
    const ::vk::MemoryPropertyFlags flags) const -> std::uint32_t
{
    const auto device_memory_info = physical_device_.getMemoryProperties();
    for (std::uint32_t i = 0; i < device_memory_info.memoryTypeCount; ++i)
    {
        const auto type_supported = bool(requirements.memoryTypeBits & (1u << i));
        const auto properties_match = (device_memory_info.memoryTypes[i].propertyFlags & flags) == flags;

        if (type_supported && properties_match)
        {
            return i;
        }
    }

    throw arm::Exception("unable to find usable gpu memory");
}

auto VulkanDevice::choose_depth_format() const -> ::vk::Format
{
    // TODO Consider parameterizing preferred depth formats
    auto preferred = std::vector{
        ::vk::Format::eD32SfloatS8Uint,
        ::vk::Format::eD24UnormS8Uint,
        ::vk::Format::eD32Sfloat,
    };

    for (const auto &entry : preferred)
    {
        auto format_properties = physical_device_.getFormatProperties(entry);
        if (format_properties.optimalTilingFeatures & ::vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            return entry;
        }
    }
    throw arm::Exception("Unable to find suitable format for depth buffer");
}

auto VulkanDevice::allocate_image(::vk::ImageCreateInfo &info, ::vk::MemoryPropertyFlags flags) const
    -> std::pair<::vk::raii::Image, ::vk::raii::DeviceMemory>
{
    auto image = ::vk::raii::Image(device_, info);

    auto memory_index = find_memory_type_index(image.getMemoryRequirements(), flags);
    auto memory_allocate_info = ::vk::MemoryAllocateInfo{};
    memory_allocate_info.sType = ::vk::StructureType::eMemoryAllocateInfo;
    memory_allocate_info.pNext = nullptr;
    memory_allocate_info.allocationSize = image.getMemoryRequirements().size;
    memory_allocate_info.memoryTypeIndex = memory_index;
    auto image_memory = ::vk::raii::DeviceMemory(device_, memory_allocate_info);

    image.bindMemory(image_memory, 0);

    return {std::move(image), std::move(image_memory)};
}

auto VulkanDevice::score_device_(
    VulkanDeviceInfo &info,
    const VulkanSurface &surface,
    const ::vk::raii::PhysicalDevice &device) -> bool
{
    auto result = false;

    const auto queue_family_properties = device.getQueueFamilyProperties();

    auto graphics_index = UINT32_MAX;
    auto present_index = UINT32_MAX;
    auto combined_index = UINT32_MAX;

    for (std::uint32_t i = 0; i < queue_family_properties.size(); ++i)
    {
        const auto has_graphics = (queue_family_properties[i].queueFlags & ::vk::QueueFlagBits::eGraphics)
                                  != static_cast<::vk::QueueFlags>(0);
        const auto has_present = surface.get_present_support(device, i);

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
        result = true;
    }
    else if (graphics_index != UINT32_MAX && present_index != UINT32_MAX)
    {
        info.graphics_index = graphics_index;
        info.present_index = present_index;
        info.score += SEPARATE_QUEUE_BONUS;
        result = true;
    }

    const auto device_properties = device.getProperties();
    if (device_properties.deviceType == ::vk::PhysicalDeviceType::eDiscreteGpu)
    {
        info.score += DISCRETE_GPU_BONUS;
    }
    else if (device_properties.deviceType == ::vk::PhysicalDeviceType::eIntegratedGpu)
    {
        info.score += INTEGRATED_GPU_BONUS;
    }

    const auto device_memory_properties = device.getMemoryProperties();
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
    return result;
}

} // namespace pong
