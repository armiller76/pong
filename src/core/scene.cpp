#include "scene.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "core/entity.h"
#include "core/resource_handles.h"
#include "engine/ubo.h"
#include "graphics/free_look_camera.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace pong
{

Scene::Scene(
    std::vector<Entity> entities,
    std::vector<EntityIndex> root_indices,
    std::uint32_t view_width,
    std::uint32_t view_height)
    : entities_{std::move(entities)}
    , root_indices_{std::move(root_indices)}
    , frame_camera_{
          {0.0f, 0.0f, 10.0f},
          {0.0f, 1.0f, 0.0f},
          ::glm::radians(33.0f),
          static_cast<float>(view_width),
          static_cast<float>(view_height),
          0.1f,
          100.f}
{
    arm::log::debug("Scene constructor: {} entities, {} roots", entities_.size(), root_indices_.size());
}

auto Scene::insert_entity(Entity &&entity, bool is_root) -> EntityIndex
{
    auto result = EntityIndex(entities_.size());
    entities_.push_back(std::move(entity));
    if (is_root)
    {
        root_indices_.push_back(result);
    }
    return result;
}

auto Scene::entities() -> std::vector<Entity> &
{
    return entities_;
}

auto Scene::entities() const -> const std::vector<Entity> &
{
    return entities_;
}

auto Scene::root_indices() const -> const std::vector<EntityIndex> &
{
    return root_indices_;
}

auto Scene::get_ambient_color() const -> Color
{
    return ambient_color_;
}

auto Scene::get_ambient_strength() const -> float
{
    return ambient_strength_;
}

auto Scene::set_ambient_color(Color color) -> void
{
    ambient_color_ = color;
}

auto Scene::set_ambient_strength(float strength) -> void
{
    ambient_strength_ = std::max(strength, 0.0f);
}

auto Scene::get_directional_light(const LightHandle handle) -> DirectionalLightData &
{
    if (directional_versions_[handle.value] != handle.version)
    {
        // TODO what do we do with a stale handle?
        throw arm::Exception("stale directional light handle");
    }

    auto light_result = directional_lights_.find(handle.value);
    if (light_result == directional_lights_.end())
    {
        // TODO what do we do if a handle we expect exists, doesn't?
        throw arm::Exception("can't find directional light handle");
    }
    else
    {
        return light_result->second;
    }
}

auto Scene::add_directional_light(DirectionalLightData light) -> LightHandle
{
    auto index = std::uint32_t{};
    auto version = std::uint32_t{};

    if (directional_free_list_.empty())
    {
        index = static_cast<std::uint32_t>(directional_versions_.size());
        version = 0u;
        directional_versions_.push_back(0u);
    }
    else
    {
        index = directional_free_list_.back();
        directional_free_list_.pop_back();
        version = directional_versions_[index];
    }
    auto [handle_result, inserted] = directional_lights_.emplace(index, light);
    arm::ensure(inserted, "directional light map insertion error");
    return {handle_result->first, version};
}

auto Scene::remove_directional_light(const LightHandle handle) -> void
{
    if (directional_versions_[handle.value] != handle.version)
    {
        // TODO what do we do with a stale handle?
        throw arm::Exception("stale directional light handle");
    }
    else
    {
        auto light_result = directional_lights_.find(handle.value);
        if (light_result == directional_lights_.end())
        {
            // TODO what do we do if an expected handle is missing?
            throw arm::Exception("directional light handle not in map");
        }
        else
        {
            directional_lights_.erase(handle.value);
            directional_versions_[handle.value]++;
            directional_free_list_.push_back(handle.value);
        }
    }
}

auto Scene::frame_camera() -> FreeLookCamera &
{
    return frame_camera_;
}

auto Scene::light_ubo() const -> UBO_Lighting
{
    auto result = UBO_Lighting{
        .ambient_color_strength = {ambient_color_.r, ambient_color_.g, ambient_color_.b, ambient_strength_},
        .light_counts = {},
        .directional = {},
    };

    auto directional_count = 0u;
    for (const auto &[key, value] : directional_lights_)
    {
        if (directional_count == MAX_DIR_LIGHTS)
        {
            break;
        }

        result.directional[directional_count] = value;
        directional_count++;
    }
    result.light_counts.x = directional_count;

    return result;
}

auto Scene::frame_camera_ubo() const -> UBO_Camera
{
    return frame_camera_.get_ubo();
}

} // namespace pong
