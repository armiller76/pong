#pragma once

#include <cstdint>
#include <tuple>

namespace pong
{

using DrawSortKey = std::tuple<std::uint64_t, std::uint64_t, std::uint64_t, std::int32_t>;

constexpr auto make_draw_sort_key(
    std::uint64_t pipeline_id,
    std::uint64_t material_id,
    std::uint64_t mesh_handle,
    std::int32_t depth_bucket = 0) -> DrawSortKey
{
    return {pipeline_id, material_id, mesh_handle, depth_bucket};
}

} // namespace pong
