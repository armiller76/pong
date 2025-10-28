#pragma once

#include <cstdint>
#include <string_view>

namespace pong
{

constexpr auto hash_string(std::string_view str) -> std::uint64_t
{
    // FNV-1a hash
    std::uint64_t hash = 14695981039346656037ULL;
    for (char c : str)
    {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

}
