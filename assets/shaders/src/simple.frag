#version 450

// layout binding = 0 is used in the vertex shader view/proj matrix.
// 0 could be reused here, but to avoid confusing myself, keeping all UBO bindings unique for now

layout(binding = 1) uniform sampler2D tex_sampler;

layout(std140, binding = 2) uniform Material
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
    out_color = vec4(ubo_material.base_color_factor.xyz, 1.0);
}
