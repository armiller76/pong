#version 450

layout(binding = 0) uniform UBO
{
    mat4 view;
    mat4 proj;
}
ubo_mvp;

layout(push_constant) uniform PushConstants
{
    mat4 model;
}
push_model;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

void main()
{
    gl_Position = ubo_mvp.proj * ubo_mvp.view * push_model.model * vec4(in_position, 1.0);
    frag_color = in_color;
}
