#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <utility>

#include <vulkan/vulkan_raii.hpp>

#include "utils/error.h"
#include "vulkan_device.h"

namespace pong
{

enum class command_context_type
{
    Frame,
    Immediate,
};

inline auto create_command_pool(const VulkanDevice &device, command_context_type type, std::string_view name)
    -> ::vk::raii::CommandPool
{
#ifndef NDEBUG
    auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{};
    debug_name_info.sType = ::vk::StructureType::eDebugUtilsObjectNameInfoEXT;
    auto debug_name_str = std::string{};
#endif

    auto pool_create_info = ::vk::CommandPoolCreateInfo{};
    pool_create_info.sType = ::vk::StructureType::eCommandPoolCreateInfo;
    using enum command_context_type;
    switch (type)
    {
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
    auto command_pool = ::vk::raii::CommandPool{device.native_handle(), pool_create_info};
#ifndef NDEBUG
    debug_name_str = std::format("Command Pool {}", name);
    debug_name_info.pObjectName = debug_name_str.c_str();
    debug_name_info.objectType = ::vk::ObjectType::eCommandPool;
    debug_name_info.objectHandle = reinterpret_cast<std::uint64_t>(static_cast<::VkCommandPool>(*command_pool));
    device.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
#endif

    return command_pool;
}

inline auto create_command_buffers(
    const VulkanDevice &device,
    std::string_view name,
    std::uint32_t count,
    const ::vk::CommandPool &pool) -> ::vk::raii::CommandBuffers
{
    auto cb_allocate_info = ::vk::CommandBufferAllocateInfo{};
    cb_allocate_info.sType = ::vk::StructureType::eCommandBufferAllocateInfo;
    cb_allocate_info.commandPool = pool;
    cb_allocate_info.level = ::vk::CommandBufferLevel::ePrimary;
    cb_allocate_info.commandBufferCount = count;
    auto command_buffers = ::vk::raii::CommandBuffers{device.native_handle(), cb_allocate_info};

#ifndef NDEBUG
    auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{};
    debug_name_info.sType = ::vk::StructureType::eDebugUtilsObjectNameInfoEXT;
    auto debug_name_str = std::string{};
    for (std::size_t i = 0; i < command_buffers.size(); ++i)
    {
        debug_name_str = std::format("{} [{}]", name, i);
        debug_name_info.pObjectName = debug_name_str.c_str();
        debug_name_info.objectType = ::vk::ObjectType::eCommandBuffer;
        debug_name_info.objectHandle =
            reinterpret_cast<std::uint64_t>(static_cast<::VkCommandBuffer>(*command_buffers.at(i)));
        device.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
    }
#endif

    return command_buffers;
}

} // namespace pong
