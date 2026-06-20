#include "vulkan_device.h"

#include <array>
#include <cstdint>
#include <limits>
#include <set>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_instance.h"
#include "engine/vulkan/vulkan_surface.h"
#include "engine/vulkan/vulkan_utils.h"
#include "graphics/types.h"
#include "utils/error.h"
#include "utils/exception.h"

namespace
{

constexpr auto DISCRETE_GPU_BONUS = 500u;
constexpr auto INTEGRATED_GPU_BONUS = 250u;
constexpr auto COMBINED_QUEUE_BONUS = 50u;
constexpr auto SEPARATE_QUEUE_BONUS = 25u;

constexpr auto uint32_max = std::numeric_limits<std::uint32_t>::max();

constexpr std::array<const char *, 1> required_device_extensions_if_13 = {
    ::vk::KHRSwapchainExtensionName,
};

constexpr std::array<const char *, 3> required_device_extensions_no_13 = {
    ::vk::KHRSwapchainExtensionName,
    ::vk::KHRSynchronization2ExtensionName,
    ::vk::KHRDynamicRenderingExtensionName,
};

const auto preferred_depth_formats = std::vector<::vk::Format>{
    ::vk::Format::eD32SfloatS8Uint,
    ::vk::Format::eD24UnormS8Uint,
    ::vk::Format::eD32Sfloat,
};

} // anonymous namespace

namespace pong
{

VulkanDevice::VulkanDevice(const VulkanInstance &instance, const VulkanSurface &surface)
    : surface_{surface}
    , physical_device_{nullptr}
    , device_{nullptr}
    , fallback_sampler_key_{SamplerKey{
          .mag_filter_mode = FilterMode::Linear,
          .min_filter_mode = FilterMode::Linear,
          .mip_map_mode = MipMapMode::Linear,
          .u_wrap = WrapMode::Repeat,
          .v_wrap = WrapMode::Repeat,
          .w_wrap = WrapMode::Repeat,
          .enable_anisotropy = false,
          .max_anisotropy = 0.0f,
          .enable_compare = false,
          .compare_op = CompareOp::Never,
          .min_lod = 0.0f,
          .max_lod = 0.0f,
          .mip_lod_bias = 0.0f,
          .enable_border_color = false,
          .unnormalized_coordinates = false,
      }}
{
    arm::log::debug("VulkanDevice constructor");

    choose_physical_device_(instance);
    arm::ensure(physical_device_ != nullptr, "No physical device found");
    arm::ensure(graphics_queue_family_index_ != uint32_max, "No graphics queue family found");
    arm::ensure(present_queue_family_index_ != uint32_max, "No present queue family found");
    arm::log::debug(
        "Selected physical device: {} (graphics queue index {}, present queue index {})",
        std::string_view(physical_device_.getProperties().deviceName),
        graphics_queue_family_index_,
        present_queue_family_index_);

    create_device_();
    arm::ensure(device_ != nullptr, "Device was not created");

    arm::log::debug(
        "Device creation complete...\n\tAPI Version: {}\n\tDynamic Rendering: {}\n\tSynchronization2: {}",
        chosen_device_info_.supports_api13 ? ">= 1.3" : "< 1.3",
        chosen_device_info_.supports_dynamic_rendering ? "Supported" : "Not Supported",
        chosen_device_info_.supports_sync2 ? "Supported" : "Not Supported");
}

auto VulkanDevice::get() const -> const ::vk::raii::Device &
{
    return device_;
}

auto VulkanDevice::native_handle() const -> const ::vk::Device
{
    return *device_;
}

auto VulkanDevice::get_physical_device() const -> const ::vk::raii::PhysicalDevice &
{
    return physical_device_;
}

auto VulkanDevice::physical_device_native_handle() const -> const ::vk::PhysicalDevice
{
    return *physical_device_;
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
    return chosen_device_info_.supports_dynamic_rendering;
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
    arm::ensure(physical_device_ != nullptr, "can't choose depth format without a device");
    for (const auto &entry : preferred_depth_formats)
    {
        auto format_properties = physical_device_.getFormatProperties(entry);
        if (format_properties.optimalTilingFeatures & ::vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            return entry;
        }
    }
    throw arm::Exception("Unable to find suitable format for depth buffer");
}

auto VulkanDevice::allocate_image(const ::vk::ImageCreateInfo &info, ::vk::MemoryPropertyFlags flags) const
    -> std::pair<::vk::raii::Image, ::vk::raii::DeviceMemory>
{
    arm::ensure(physical_device_ != nullptr, "can't allocate an image without a device");
    auto image_result = check_vk_expected(device_.createImage(info));
    if (!image_result.has_value())
    {
        throw arm::Exception("unable to allocate image");
    }

    auto &image = image_result.value();
    const auto requirements = image.getMemoryRequirements();
    const auto memory_index = find_memory_type_index(requirements, flags);
    const auto memory_allocate_info = ::vk::MemoryAllocateInfo{
        .sType = ::vk::StructureType::eMemoryAllocateInfo,
        .pNext = nullptr,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memory_index,
    };

    auto image_memory_result = check_vk_expected(device_.allocateMemory(memory_allocate_info));
    if (!image_memory_result.has_value())
    {
        throw arm::Exception("unable to allocate image memory");
    }

    auto &memory = image_memory_result.value();
    image.bindMemory(memory, 0);

    return {std::move(image), std::move(memory)};
}

auto VulkanDevice::get_sampler(const SamplerKey &key) -> ::vk::Sampler
{
    arm::ensure(physical_device_ != nullptr, "can't get samplers without a device");
    if (const auto result = samplers_.find(key); result != samplers_.end())
    {
        return *result->second;
    }
    else
    {
        const auto create_info = ::vk::SamplerCreateInfo{
            .sType = ::vk::StructureType::eSamplerCreateInfo,
            .pNext = nullptr,
            .flags = {},
            .magFilter = to_vk(key.mag_filter_mode),
            .minFilter = to_vk(key.min_filter_mode),
            .mipmapMode = to_vk(key.mip_map_mode),
            .addressModeU = to_vk(key.u_wrap),
            .addressModeV = to_vk(key.v_wrap),
            .addressModeW = to_vk(key.w_wrap),
            .mipLodBias = key.mip_lod_bias,
            .anisotropyEnable = key.enable_anisotropy ? ::vk::True : ::vk::False,
            .maxAnisotropy = key.max_anisotropy,
            .compareEnable = key.enable_compare ? ::vk::True : ::vk::False,
            .compareOp = to_vk(key.compare_op),
            .minLod = key.min_lod,
            .maxLod = key.max_lod,
            .borderColor = ::vk::BorderColor::eFloatTransparentBlack,
            .unnormalizedCoordinates = key.unnormalized_coordinates ? ::vk::True : ::vk::False,
        };

        auto sampler_result = check_vk_expected(device_.createSampler(create_info));
        if (!sampler_result.has_value())
        {
            throw arm::Exception("unable to create sampler");
        }
        else
        {
            auto [result, inserted] = samplers_.emplace(key, std::move(sampler_result.value()));
            return *result->second;
        }
    }
}

auto VulkanDevice::get_default_sampler_key() const -> const SamplerKey &
{
    return fallback_sampler_key_;
}

auto VulkanDevice::score_device_(VulkanDeviceInfo &info, const ::vk::raii::PhysicalDevice &device) -> bool
{
    auto result = false;

    const auto queue_family_properties = device.getQueueFamilyProperties();

    auto graphics_index = uint32_max;
    auto present_index = uint32_max;
    auto combined_index = uint32_max;

    for (std::uint32_t i = 0; i < queue_family_properties.size(); ++i)
    {
        const auto has_graphics = (queue_family_properties[i].queueFlags & ::vk::QueueFlagBits::eGraphics)
                                  != static_cast<::vk::QueueFlags>(0);
        const auto has_present = device.getSurfaceSupportKHR(i, surface_.get());

        if (has_graphics && graphics_index == uint32_max)
        {
            graphics_index = i;
        }

        if (has_present && present_index == uint32_max)
        {
            present_index = i;
        }

        if (has_graphics && has_present && combined_index == uint32_max)
        {
            combined_index = i;
            break;
        }
    }

    if (combined_index != uint32_max)
    {
        info.graphics_index = combined_index;
        info.present_index = combined_index;
        info.score += COMBINED_QUEUE_BONUS;
        result = true;
    }
    else if (graphics_index != uint32_max && present_index != uint32_max)
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
    auto largest_heap_size = ::vk::DeviceSize{};
    for (const auto &heap : device_memory_properties.memoryHeaps)
    {
        if (heap.flags & ::vk::MemoryHeapFlagBits::eDeviceLocal)
        {
            largest_heap_size = heap.size > largest_heap_size ? heap.size : largest_heap_size;
        }
    }
    info.score += static_cast<uint32_t>(largest_heap_size / (1024zu * 1024zu));

    return result;
}

auto VulkanDevice::choose_physical_device_(const VulkanInstance &instance) -> void
{
    auto available_device_result = check_vk_expected(instance.get().enumeratePhysicalDevices());
    if (!available_device_result.has_value())
    {
        throw arm::Exception("unable to enumerate physical devices");
    }

    const auto &available_devices = available_device_result.value();
    arm::ensure(!available_devices.empty(), "No available graphics devices");

    auto device_infos = std::vector<VulkanDeviceInfo>();
    for (const auto &physical_device : available_devices)
    {
        auto vulkan_device_info = VulkanDeviceInfo{
            .physical_device = *physical_device,
            .score = 0,
            .graphics_index = uint32_max,
            .present_index = uint32_max,
            .supports_api13 = false,
            .supports_dynamic_rendering = false,
            .supports_sync2 = false,
        };

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
            vulkan_device_info.supports_dynamic_rendering = features13.dynamicRendering == ::vk::True;
            vulkan_device_info.supports_sync2 = features13.synchronization2 == ::vk::True;
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

            vulkan_device_info.supports_dynamic_rendering = feature_dynamic_rendering.dynamicRendering == ::vk::True;
            vulkan_device_info.supports_sync2 = feature_sync2.synchronization2 == ::vk::True;
        }

        auto has_extensions = true;
        const auto check_device_extensions = physical_device.enumerateDeviceExtensionProperties();
        if (vulkan_device_info.supports_api13)
        {
            for (const auto &required_extension : required_device_extensions_if_13)
            {
                const auto this_check_extension = std::ranges::find_if(
                    check_device_extensions,
                    [required_extension](const ::vk::ExtensionProperties &check_extension) -> bool
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
                    [required_extension](const ::vk::ExtensionProperties &check_extension) -> bool
                    { return std::string_view(required_extension) == check_extension.extensionName; });
                has_extensions = has_extensions && this_check_extension != check_device_extensions.end();
            }
        }

        if (has_extensions)
        {
            if (score_device_(vulkan_device_info, physical_device))
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
    chosen_device_info_ = device_infos[0];

    physical_device_ = ::vk::raii::PhysicalDevice(instance.get(), chosen_device_info_.physical_device);
    graphics_queue_family_index_ = chosen_device_info_.graphics_index;
    present_queue_family_index_ = chosen_device_info_.present_index;
}

auto VulkanDevice::create_device_() -> void
{
    constexpr auto queue_priority = 1.0f;
    auto queue_create_infos = std::vector<vk::DeviceQueueCreateInfo>{};

    // putting indices into a std::set deduplicates them in cases when graphics and present are the same queue.
    const auto queue_families_set = std::set<uint32_t>{graphics_queue_family_index_, present_queue_family_index_};

    for (uint32_t queue_family : queue_families_set)
    {
        auto create_info = ::vk::DeviceQueueCreateInfo{
            .sType = ::vk::StructureType::eDeviceQueueCreateInfo,
            .pNext = nullptr,
            .flags = {},
            .queueFamilyIndex = queue_family,
            .queueCount = 1u,
            .pQueuePriorities = &queue_priority};
        queue_create_infos.emplace_back(create_info);
    }

    auto device_create_info = vk::DeviceCreateInfo{};
    device_create_info.setQueueCreateInfos(queue_create_infos);

    [[maybe_unused]] auto feature_api13 = ::vk::PhysicalDeviceVulkan13Features{};
    [[maybe_unused]] auto feature_sync2 = ::vk::PhysicalDeviceSynchronization2Features{};
    [[maybe_unused]] auto feature_dynamic_rendering = ::vk::PhysicalDeviceDynamicRenderingFeatures{};

    if (chosen_device_info_.supports_api13)
    {
        feature_api13.setDynamicRendering(chosen_device_info_.supports_dynamic_rendering ? ::vk::True : ::vk::False);
        feature_api13.setSynchronization2(chosen_device_info_.supports_sync2 ? ::vk::True : ::vk::False);

        device_create_info.setPEnabledExtensionNames(required_device_extensions_if_13);
        device_create_info.setPNext(&feature_api13);
    }
    else
    {
        feature_dynamic_rendering.setDynamicRendering(
            chosen_device_info_.supports_dynamic_rendering ? ::vk::True : ::vk::False);

        feature_sync2.setSynchronization2(chosen_device_info_.supports_sync2 ? ::vk::True : ::vk::False);
        feature_sync2.setPNext(&feature_dynamic_rendering);

        device_create_info.setPEnabledExtensionNames(required_device_extensions_no_13);
        device_create_info.setPNext(&feature_sync2);
    }

    auto device_result = check_vk_expected(physical_device_.createDevice(device_create_info));
    if (!device_result.has_value())
    {
        throw arm::Exception("unable to create Vulkan device");
    }
    device_ = std::move(device_result.value());

    auto graphics_queue_result = check_vk_expected(device_.getQueue(graphics_queue_family_index_, 0));
    if (!graphics_queue_result.has_value())
    {
        throw arm::Exception("uanble to get graphics queue from device");
    }
    graphics_queue_ = std::move(graphics_queue_result.value());

    auto present_queue_result = check_vk_expected(device_.getQueue(present_queue_family_index_, 0));
    if (!present_queue_result.has_value())
    {
        throw arm::Exception("unable to get present queue from device");
    }
    present_queue_ = std::move(present_queue_result.value());
}

} // namespace pong
