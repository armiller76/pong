#pragma once

#include <cstdint>
#include <limits>
#include <string_view>

#include "utils/exception.h"

namespace pong
{

static constexpr auto INVALID_RESOURCE_ID = (std::numeric_limits<std::uint64_t>::max)() - 1;

constexpr auto fnv1a_mix(std::uint64_t hash, unsigned char byte) -> std::uint64_t
{
    hash ^= static_cast<std::uint64_t>(byte);
    hash *= 1099511628211ULL;
    return hash;
}

constexpr auto hash_string(std::string_view str) -> std::uint64_t
{
    // FNV-1a hash
    std::uint64_t hash = 14695981039346656037ULL;
    for (char byte : str)
    {
        hash = fnv1a_mix(hash, static_cast<unsigned char>(byte));
    }
    if (hash == INVALID_RESOURCE_ID)
    {
        hash = 14695981039346656037ULL;
        hash = fnv1a_mix(hash, static_cast<unsigned char>('0'));
        for (char byte : str)
        {
            hash = fnv1a_mix(hash, static_cast<unsigned char>(byte));
        }
    }
    if (hash == INVALID_RESOURCE_ID)
    {
        throw arm::Exception("literally how?");
    }
    return hash;
}

} // namespace pong
