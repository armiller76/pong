#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


#include <vulkan/vulkan_raii.hpp>

#include "engine/engine_error.h"
#include "engine/vulkan/vulkan_device.h"
#include "utils/exception.h"

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
    auto command_pool_result = check_vk_expected(device.native_handle().createCommandPool(pool_create_info));
    if (!command_pool_result)
    {
        throw arm::Exception("unable to create command pool '{}'", name);
    }

#ifndef NDEBUG
    debug_name_str = std::format("Command Pool {}", name);
    debug_name_info.pObjectName = debug_name_str.c_str();
    debug_name_info.objectType = ::vk::ObjectType::eCommandPool;
    debug_name_info.objectHandle =
        reinterpret_cast<std::uint64_t>(static_cast<::VkCommandPool>(*command_pool_result.value()));
    device.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
#endif

    return std::move(command_pool_result.value());
}

inline auto create_command_buffers(
    const VulkanDevice &device,
    std::string_view name,
    std::uint32_t count,
    const ::vk::CommandPool &pool) -> std::vector<::vk::raii::CommandBuffer>
{
    auto cb_allocate_info = ::vk::CommandBufferAllocateInfo{};
    cb_allocate_info.sType = ::vk::StructureType::eCommandBufferAllocateInfo;
    cb_allocate_info.commandPool = pool;
    cb_allocate_info.level = ::vk::CommandBufferLevel::ePrimary;
    cb_allocate_info.commandBufferCount = count;

    auto command_buffers_result = check_vk_expected(device.native_handle().allocateCommandBuffers(cb_allocate_info));
    if (!command_buffers_result)
    {
        throw arm::Exception("unable to allocate command buffers");
    }

#ifndef NDEBUG
    auto debug_name_info = ::vk::DebugUtilsObjectNameInfoEXT{};
    debug_name_info.sType = ::vk::StructureType::eDebugUtilsObjectNameInfoEXT;
    auto debug_name_str = std::string{};
    for (std::size_t i = 0; i < command_buffers_result.value().size(); ++i)
    {
        debug_name_str = std::format("{} [{}]", name, i);
        debug_name_info.pObjectName = debug_name_str.c_str();
        debug_name_info.objectType = ::vk::ObjectType::eCommandBuffer;
        debug_name_info.objectHandle =
            reinterpret_cast<std::uint64_t>(static_cast<::VkCommandBuffer>(*command_buffers_result.value().at(i)));
        device.native_handle().setDebugUtilsObjectNameEXT(debug_name_info);
    }
#endif

    return std::move(command_buffers_result.value());
}

} // namespace pong
