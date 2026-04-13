#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include <spirv-tools/libspirv.h>

#include "utils/log.h"

namespace pong
{

inline auto spirv_validate(std::span<const std::uint32_t> words) -> bool
{
    if (words.size() < 5)
        return false; // automatically not valid;

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
