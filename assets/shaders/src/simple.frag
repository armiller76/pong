#version 450

layout(set = 1, binding = 0) uniform sampler2D base_color_factor_sampler;
layout(set = 1, binding = 1) uniform sampler2D metallic_factor_sampler;
layout(set = 1, binding = 2) uniform sampler2D normal_sampler;

layout(std140, set = 0, binding = 1) uniform Light
{
    vec4 direction_xyz_intensity_w;
    vec4 color_xyz_strength_w;
}
ubo_light;

layout(std140, set = 1, binding = 3) uniform Material
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
    vec3 ambient = ubo_light.color_xyz_strength_w.xyz * ubo_light.color_xyz_strength_w.w;
    // vec3 lambert = max(dot(normalize(in_normal), normalize(ubo_light.direction_xyz_intensity_w.xyz)),0);
    vec3 diffuse = {0.0, 0.0, 0.0}; // lambert * light_color * intensity;
    out_color = texture(base_color_factor_sampler, in_uv) * ubo_material.base_color_factor;
    out_color = vec4(out_color.rgb * (ambient + diffuse), out_color.a);
}
