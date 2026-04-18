#pragma once

#include <format>
#include <string>
#include <vector>

#include "graphics/glm_wrapper.h"
#include "math/utils.h"

namespace pong
{

class Transform
{
  public:
    constexpr Transform()
        : position{}
        , rotation{::glm::quat_identity<float, ::glm::qualifier::defaultp>()}
        , scale{1.0f}
    {
    }

    Transform(::glm::vec3 position, ::glm::vec3 scale, ::glm::quat rotation)
        : position{position}
        , rotation{normalize_safe(rotation)}
        , scale{scale}
    {
    }

    Transform(const ::glm::mat4x4 &model)
    {
        auto dummy1 = ::glm::vec3();
        auto dummy2 = ::glm::vec4();
        ::glm::decompose(model, scale, rotation, position, dummy1, dummy2);
    }

    operator ::glm::mat4() const
    {
        auto result = ::glm::mat4(::glm::translate(::glm::mat4(1.0f), position));
        result *= ::glm::mat4_cast(rotation);
        return ::glm::scale(result, scale);
    }

    ::glm::vec3 position;
    ::glm::quat rotation;
    ::glm::vec3 scale;
};

inline auto to_string(const Transform &transform) -> std::string
{
    return std::format(
        "transform pos:{} rot:{} scale:{}",
        ::glm::to_string(transform.position),
        ::glm::to_string(transform.rotation),
        ::glm::to_string(transform.scale));
}

}
