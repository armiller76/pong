#version 450

layout(set = 1, binding = 0) uniform sampler2D base_color_factor_sampler;
layout(set = 1, binding = 1) uniform sampler2D metallic_factor_sampler;
layout(set = 1, binding = 2) uniform sampler2D normal_sampler;

layout(std140, set = 0, binding = 1) uniform Material
{
    vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
}
ubo_material;

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

void main()
{
    // out_color = vec4(ubo_material.base_color_factor.xyz, 1.0);
    out_color = texture(base_color_factor_sampler, in_uv) * ubo_material.base_color_factor;
}
