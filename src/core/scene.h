#pragma once

#include <vector>

#include "core/entity.h"
#include "core/resource_handles.h"

namespace pong
{

class ResourceLoader;

class Scene
{
  public:
    Scene(std::vector<Entity> entities, std::vector<EntityIndex> root_indices);

    auto insert_root(Entity &&entity) -> EntityIndex;
    auto insert_entity(Entity &&entity) -> EntityIndex;

    auto entities() const -> const std::vector<Entity> &;
    auto root_indices() const -> const std::vector<EntityIndex> &;

  private:
    std::vector<Entity> entities_;
    std::vector<EntityIndex> root_indices_;

}; // class Scene

} // namespace pong
