#pragma once

#include "graphics/glm_wrapper.h"
#include "utils/log.h"

namespace pong
{

static constexpr auto g_epsilon = 0.000001f;

// returns false if any vector component is infinite or NaN, otherwise true
inline auto all_finite(::glm::vec3 v) -> bool
{
    return !(::glm::any(isinf(v)) || ::glm::any(isnan(v)));
}

// returns false if any vector component is infinite or NaN, otherwise true
inline auto all_finite(::glm::vec4 v) -> bool
{
    return !(::glm::any(isinf(v)) || ::glm::any(isnan(v)));
}

// returns normalized quaternion
// falls back to identity quaternion if components or len^2 are NaN, inf, or near-zero
inline auto normalize_safe(::glm::quat q) -> ::glm::quat
{
    auto fallback = ::glm::quat_identity<float, ::glm::qualifier::defaultp>();

    const auto check = ::glm::vec4(q.w, q.x, q.y, q.z);
    if (!all_finite(check))
    {
        arm::log::info("normalize_safe Quaternion failed finite check {}", q);
        return fallback;
    }

    const auto length_squared = ::glm::dot(check, check);
    if (::glm::isinf(length_squared) || length_squared < 0.00000001f)
    {
        arm::log::info("normalize_safe Quaternion failed length check {}", q);
        return fallback;
    }

    return ::glm::normalize(q);
}

// returns false if any component of input vector is infinite or less than global epsilon
inline auto is_scale_safe(::glm::vec3 s) -> bool
{
    return !(!all_finite(s) || ::glm::any(::glm::lessThan(::glm::abs(s), ::glm::vec3{g_epsilon})));
}

}
