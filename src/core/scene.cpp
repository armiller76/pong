#include "scene.h"

#include <utility>
#include <vector>

#include "core/entity.h"
#include "core/resource_handles.h"
#include "utils/log.h"

namespace pong
{

Scene::Scene(std::vector<Entity> entities, std::vector<EntityIndex> root_indices)
    : entities_{std::move(entities)}
    , root_indices_{std::move(root_indices)}
{
    arm::log::debug("Scene constructor: {} entities, {} roots", entities_.size(), root_indices_.size());
}

auto Scene::insert_root(Entity &&entity) -> EntityIndex
{
    auto result = EntityIndex(entities_.size());
    entities_.push_back(std::move(entity));
    root_indices_.push_back(result);
    return result;
}

auto Scene::insert_entity(Entity &&entity) -> EntityIndex
{
    auto result = EntityIndex(entities_.size());
    entities_.push_back(std::move(entity));
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

} // namespace pong
