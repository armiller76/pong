#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include <spirv-tools/libspirv.h>

#include "utils/log.h"

namespace pong
{

inline auto spirv_validate(std::span<const std::byte> bytes) -> bool
{
    if (bytes.size_bytes() % sizeof(std::uint32_t) != 0 || bytes.size_bytes() < 20)
        return false; // automatically not valid;
    auto words = std::span<const std::uint32_t>(
        reinterpret_cast<const std::uint32_t *>(bytes.data()), bytes.size_bytes() / sizeof(std::uint32_t));

    const auto context = spvContextCreate(spv_target_env::SPV_ENV_VULKAN_1_3);
    auto diagnostic = spv_diagnostic{};
    auto binary = spv_const_binary_t{.code = words.data(), .wordCount = words.size()};
    auto validate_result = spvValidate(context, &binary, &diagnostic);
    if (validate_result)
    {
        if (diagnostic)
        {
            arm::log::info("message from spirv-tools spvValidate: {}", diagnostic->error);
            spvDiagnosticDestroy(diagnostic);
        }
    }
    spvContextDestroy(context);
    return validate_result == SPV_SUCCESS;
}

}
