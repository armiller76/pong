#pragma once

#include <cstdint>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "engine/ubo.h"

namespace pong
{

/*
thinking out loud...
CameraData is what will be sent to GPU (view matrix, proj matrix, position)

FreeLookCamera (mouse controls pitch/yaw)
OrbitCamera? (mouse rotates around fixed point)
FollowCamera? (follows character/object)
ScriptedCamera? (for cinematic/etc)
*/

struct CameraData
{
    ::glm::mat4 view_matrix;
    ::glm::mat4 projection_matrix;
    ::glm::vec3 position{};
};

class FreeLookCamera
{
  public:
    FreeLookCamera(
        const ::glm::vec3 position,
        const ::glm::vec3 world_up,
        float vertical_fov_radians,
        float width,
        float heigh,
        float near_clip,
        float far_clip);

    auto translate_by(const ::glm::vec3 delta_world) -> void;
    auto adjust_pitch(float radians) -> void;
    auto adjust_yaw(float radians) -> void;
    auto set_fov(float radians) -> void;
    auto set_world_up(const ::glm::vec3 up) -> void;
    auto resize(std::uint32_t width, std::uint32_t height) -> void;

    auto get_world_up() const -> const ::glm::vec3;
    auto get_forward_direction() const -> const ::glm::vec3;
    auto get_up_direction() const -> const ::glm::vec3;
    auto get_right_direction() const -> const ::glm::vec3;
    auto get_fov() const -> float;
    auto get_width() const -> float;
    auto get_height() const -> float;
    auto get_aspect() const -> float;
    auto get_near_clip() const -> float;
    auto get_far_clip() const -> float;
    auto get_position() const -> const ::glm::vec3;
    auto get_ubo() const -> UBO_Camera;

  private:
    float vertical_fov_radians_;
    float width_;
    float height_;
    float near_clip_;
    float far_clip_;
    float pitch_;
    float yaw_;

    ::glm::vec3 world_up_direction_;
    ::glm::vec3 forward_direction_;
    ::glm::vec3 up_direction_;
    ::glm::vec3 right_direction_;
    CameraData camera_data_;

  private:
    auto update_all_() -> void;
    auto recalculate_vectors_() -> void;
    auto update_view_() -> void;
};

}
