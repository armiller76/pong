#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "engine/vulkan/vulkan_device.h"
#include "engine/vulkan/vulkan_utils.h"
#include "utils/exception.h"

namespace pong
{

enum class CommandContextType : std::uint8_t
{
    Frame,
    Immediate,
};

inline auto create_command_pool(const VulkanDevice &device, CommandContextType type, std::string_view name)
    -> ::vk::raii::CommandPool
{
    auto pool_create_info = ::vk::CommandPoolCreateInfo{
        .sType = ::vk::StructureType::eCommandPoolCreateInfo,
        .pNext = nullptr,
    };

    switch (type)
    {
        using enum CommandContextType;
        case Frame:
        {
            pool_create_info.flags = ::vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            pool_create_info.queueFamilyIndex = device.graphics_queue_family_index();
        }
        break;
        case Immediate:
        {
            pool_create_info.flags =
                ::vk::CommandPoolCreateFlagBits::eTransient | ::vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            // queueFamilyIndex could be transfer_queue if/when one is implemented
            pool_create_info.queueFamilyIndex = device.graphics_queue_family_index();
        }
        break;
    }

    auto command_pool_result = check_vk_expected(device.get().createCommandPool(pool_create_info));
    if (!command_pool_result.has_value())
    {
        throw arm::Exception("unable to create command pool '{}'", name);
    }

#ifndef NDEBUG
    const auto debug_name_str = std::format("Command Pool: {}", name);
    const auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{
        .sType = ::vk::StructureType::eDebugUtilsObjectNameInfoEXT,
        .pNext = nullptr,
        .objectType = ::vk::ObjectType::eCommandPool,
        .objectHandle = reinterpret_cast<std::uint64_t>(static_cast<::VkCommandPool>(*command_pool_result.value())),
        .pObjectName = debug_name_str.c_str(),
    };
    device.get().setDebugUtilsObjectNameEXT(debug_name_info);
#endif

    return std::move(command_pool_result.value());
}

inline auto create_command_buffers(
    const VulkanDevice &device,
    std::string_view name,
    std::uint32_t count,
    const ::vk::CommandPool &pool) -> std::vector<::vk::raii::CommandBuffer>
{
    const auto cb_allocate_info = ::vk::CommandBufferAllocateInfo{
        .sType = ::vk::StructureType::eCommandBufferAllocateInfo,
        .pNext = nullptr,
        .commandPool = pool,
        .level = ::vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = count,
    };

    auto command_buffers_result = check_vk_expected(device.get().allocateCommandBuffers(cb_allocate_info));
    if (!command_buffers_result.has_value())
    {
        throw arm::Exception("unable to allocate command buffers");
    }

    auto &command_buffers = command_buffers_result.value();
#ifndef NDEBUG
    auto i = 0u;
    for (const auto &cb : command_buffers)
    {
        const auto debug_name_str = std::format("{} [{}]", name, i++);
        const auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{
            .sType = ::vk::StructureType::eDebugUtilsObjectNameInfoEXT,
            .pNext = nullptr,
            .objectType = ::vk::ObjectType::eCommandBuffer,
            .objectHandle = reinterpret_cast<std::uint64_t>(static_cast<::VkCommandBuffer>(*cb)),
            .pObjectName = debug_name_str.c_str(),
        };
        device.get().setDebugUtilsObjectNameEXT(debug_name_info);
    }
#endif

    return std::move(command_buffers);
}

} // namespace pong
