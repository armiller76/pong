#version 450

layout(std140, binding = 0) uniform UBO
{
    mat4 view;
    mat4 proj;
}
ubo_vp;

layout(push_constant) uniform PushConstants
{
    mat4 model;
}
push_model;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec2 in_uv;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;

void main()
{
    gl_Position = ubo_vp.proj * ubo_vp.view * push_model.model * vec4(in_position, 1.0);
    out_color = in_color;

    // TODO this won't work with non-uniform scaling. Use inverse transpose of model matrix ($$$)
    out_normal = vec3(push_model.model * vec4(in_normal, 0));

    out_uv = in_uv;
}
