#include <algorithm>
#include <array>
#include <vector>

#include <gtest/gtest.h>

#include "engine/vulkan/vulkan_render_sort_key.h"

namespace pong
{

TEST(RenderSortKey, LexicographicPriorityMatchesFieldOrder)
{
    auto keys = std::vector<DrawSortKey>{
        make_draw_sort_key(1, 1, 3, 0),
        make_draw_sort_key(0, 2, 1, 0),
        make_draw_sort_key(0, 1, 5, 0),
        make_draw_sort_key(0, 1, 2, 0),
    };

    std::ranges::sort(keys);

    const auto expected = std::array<DrawSortKey, 4>{
        make_draw_sort_key(0, 1, 2, 0),
        make_draw_sort_key(0, 1, 5, 0),
        make_draw_sort_key(0, 2, 1, 0),
        make_draw_sort_key(1, 1, 3, 0),
    };

    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(keys[i], expected[i]);
    }
}

} // namespace pong
