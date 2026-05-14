#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "core/entity.h"
#include "core/resource_handles.h"
#include "engine/ubo.h"
#include "graphics/color.h"
#include "graphics/free_look_camera.h"
#include "resource_handles.h"

namespace pong
{

class ResourceLoader;

class Scene
{
  public:
    Scene(
        std::vector<Entity> entities,
        std::vector<EntityIndex> root_indices,
        std::uint32_t view_width,
        std::uint32_t view_height);

    auto insert_root(Entity &&entity) -> EntityIndex;
    auto insert_entity(Entity &&entity) -> EntityIndex;

    auto entities() -> std::vector<Entity> &; // TODO get rid of this when not needed for debug
    auto entities() const -> const std::vector<Entity> &;
    auto root_indices() const -> const std::vector<EntityIndex> &;

    auto get_ambient_color() const -> Color;
    auto get_ambient_strength() const -> float;

    auto set_ambient_color(Color color) -> void;
    auto set_ambient_strength(float strength) -> void;

    auto get_directional_light(LightHandle handle) -> DirectionalLightData &;
    auto add_directional_light(DirectionalLightData light) -> LightHandle;
    auto remove_directional_light(LightHandle handle) -> void;

    auto frame_camera() -> FreeLookCamera &;

    auto light_ubo() const -> UBO_Lighting;
    auto frame_camera_ubo() const -> UBO_Camera;

  private:
    // entities
    std::vector<Entity> entities_;
    std::vector<EntityIndex> root_indices_;

    // lights
    Color ambient_color_;
    float ambient_strength_;

    std::unordered_map<std::uint32_t, DirectionalLightData> directional_lights_;
    std::vector<std::uint32_t> directional_free_list_;
    std::vector<std::uint32_t> directional_versions_;

    FreeLookCamera frame_camera_;
}; // class Scene

} // namespace pong
