#include <array>
#include <string_view>

#include <gtest/gtest.h>

#include "engine/resource_manager.h"
#include "utils/hash.h"

namespace pong
{

TEST(Hash, EmptyStringMatchesFnvOffsetBasis)
{
    EXPECT_EQ(hash_string(""), 14695981039346656037ULL);
}

TEST(Hash, DeterministicForSameInput)
{
    constexpr auto s = std::string_view{"pong"};
    EXPECT_EQ(hash_string(s), hash_string(s));
}

TEST(Hash, DistinguishesTypicalInputs)
{
    constexpr auto inputs = std::array<std::string_view, 6>{
        "pong",
        "Pong",
        "simple.vert",
        "simple.frag",
        "mesh_test_triangle",
        "mesh_test_rectangle",
    };

    for (std::size_t i = 0; i < inputs.size(); ++i)
    {
        for (std::size_t j = i + 1; j < inputs.size(); ++j)
        {
            EXPECT_NE(hash_string(inputs[i]), hash_string(inputs[j]));
        }
    }
}

TEST(ResourceManager, ResourceIdUsesHashString)
{
    constexpr auto key = std::string_view{"simple.vert"};
    EXPECT_EQ(ResourceManager::get_resource_id(key), hash_string(key));
}

} // namespace pong
