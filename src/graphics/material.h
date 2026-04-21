#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <vulkan/vulkan_raii.hpp>

#include "core/resource_handles.h"
#include "engine/ubo.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "graphics/glm_wrapper.h" // IWYU pragma: keep
#include "graphics/types.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/hash.h"

namespace pong
{

class Material
{
  public:
    Material(
        const VulkanDevice &device,
        std::string_view name,
        ::vk::raii::DescriptorSet descriptor_set,
        ::glm::vec4 base_color_factor,
        float metallic_factor,
        float roughness_factor,
        AlphaMode alpha_mode)
        : name_{name}
        , descriptor_set_{std::move(descriptor_set)}
        , base_color_factor_{base_color_factor}
        , metallic_factor_{metallic_factor}
        , roughness_factor_{roughness_factor}
        , factor_ubo_buffer_{device, ::vk::DeviceSize(sizeof(UBO_Material)), ::vk::BufferUsageFlagBits::eUniformBuffer, ::vk::MemoryPropertyFlagBits::eHostCoherent | ::vk::MemoryPropertyFlagBits::eHostVisible}
        , alpha_mode_{alpha_mode}
    {
        auto upload_data = UBO_Material{
            .base_color_factor = base_color_factor_,
            .metallic_factor = metallic_factor_,
            .roughness_factor = roughness_factor_,
            .pad1 = 0.0f,
            .pad2 = 0.0f};
        factor_ubo_buffer_.upload(&upload_data, sizeof(UBO_Material));
        arm::ensure(factor_ubo_buffer_.size() == sizeof(UBO_Material), "material UBO buffer upload failed");
    }

    auto name() const -> std::string_view
    {
        return name_;
    }

    auto descriptor_set() const -> const vk::raii::DescriptorSet &
    {
        return descriptor_set_;
    }

    auto base_color_factor() const -> const ::glm::vec4
    {
        return base_color_factor_;
    }

    auto metallic_factor() const -> float
    {
        return metallic_factor_;
    }

    auto roughness_factor() const -> float
    {
        return roughness_factor_;
    }

    auto alpha_mode() const -> AlphaMode
    {
        return alpha_mode_;
    }

    auto has_base_color_texture() const -> bool
    {
        return base_color_texture_handle_.has_value();
    }

    auto get_base_color_texture_handle() const -> const Texture2DHandle
    {
        if (has_base_color_texture())
        {
            return base_color_texture_handle_.value();
        }
        else
        {
            return Texture2DHandle(INVALID_RESOURCE_ID);
        }
    }

    auto set_base_color_texture_handle(Texture2DHandle texture_handle) -> void
    {
        if (!has_base_color_texture())
        {
            base_color_texture_handle_ = texture_handle;
        }
        else
        {
            throw arm::Exception("changing textures is not supported");
        }
    }

    auto has_metallic_roughness_texture() const -> bool
    {
        return metallic_roughness_texture_handle_.has_value();
    }

    auto get_metallic_roughness_texture_handle() const -> const Texture2DHandle
    {
        if (has_metallic_roughness_texture())
        {
            return metallic_roughness_texture_handle_.value();
        }
        else
        {
            return Texture2DHandle(INVALID_RESOURCE_ID);
        }
    }

    auto set_metallic_roughness_texture_handle(Texture2DHandle texture_handle) -> void
    {
        if (!has_metallic_roughness_texture())
        {
            metallic_roughness_texture_handle_ = texture_handle;
        }
        else
        {
            throw arm::Exception("changing textures is not supported");
        }
    }

    auto has_normal_texture() const -> bool
    {
        return normal_texture_handle_.has_value();
    }

    auto get_normal_texture_handle() const -> const Texture2DHandle
    {
        if (has_normal_texture())
        {
            return normal_texture_handle_.value();
        }
        else
        {
            return Texture2DHandle(INVALID_RESOURCE_ID);
        }
    }

    auto set_normal_texture_handle(Texture2DHandle texture_handle) -> void
    {
        if (!has_normal_texture())
        {
            normal_texture_handle_ = texture_handle;
        }
        else
        {
            throw arm::Exception("changing textures is not supported");
        }
    }

    auto get_buffer() const -> const VulkanGpuBuffer &
    {
        return factor_ubo_buffer_;
    }

  private:
    std::string name_;
    ::vk::raii::DescriptorSet descriptor_set_;

    ::glm::vec4 base_color_factor_;
    float metallic_factor_;
    float roughness_factor_;
    VulkanGpuBuffer factor_ubo_buffer_;

    std::optional<Texture2DHandle> base_color_texture_handle_;
    std::optional<Texture2DHandle> metallic_roughness_texture_handle_;
    std::optional<Texture2DHandle> normal_texture_handle_;

    AlphaMode alpha_mode_;

}; // struct Material

} // namespace pong
