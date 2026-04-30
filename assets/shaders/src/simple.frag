#version 450

#ifndef MAX_DIR_LIGHTS
#define MAX_DIR_LIGHTS 8
#endif

layout(set = 1, binding = 0) uniform sampler2D base_color_factor_sampler;
layout(set = 1, binding = 1) uniform sampler2D metallic_factor_sampler;
layout(set = 1, binding = 2) uniform sampler2D normal_sampler;

struct Directional
{
    vec4 direction_intensity; // .xyz = direction, .w = intensity
    vec4 color;               // .xyz = color, .w = unused/pad
};

layout(std140, set = 0, binding = 1) uniform Light
{
    vec4 ambient_color_strength; // ambient
    uvec4 light_counts;          // .x = directional, .y = point, .z = spot, .w = unused/pad
    Directional directionals[8]; // TODO magic number // directional
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
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;

layout(location = 0) out vec4 out_color;

void main()
{
    vec3 t = normalize(in_tangent);
    vec3 b = normalize(in_bitangent);
    vec3 n = normalize(in_normal);
    mat3 tbn = mat3(t, b, n);
    vec4 sampled_base_color = texture(base_color_factor_sampler, in_uv) * ubo_material.base_color_factor;

    vec3 sampled_normal = texture(normal_sampler, in_uv).rgb;
    sampled_normal = normalize(sampled_normal * 2.0 - 1.0);
    vec3 world_normal = normalize(tbn * sampled_normal);

    // ambient term
    vec3 ambient = ubo_light.ambient_color_strength.xyz * ubo_light.ambient_color_strength.w;

    // directional lights
    vec3 diffuse_sum = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < min(ubo_light.light_counts.x, MAX_DIR_LIGHTS); i++)
    {
        vec3 light_dir = normalize(-ubo_light.directionals[i].direction_intensity.xyz);
        float pre_lambert = max(dot(world_normal, light_dir), 0.0);
        vec3 diffuse =
            pre_lambert * ubo_light.directionals[i].color.xyz * ubo_light.directionals[i].direction_intensity.w;
        diffuse_sum += diffuse;
    }

    out_color = vec4(sampled_base_color.rgb * (ambient + diffuse_sum), sampled_base_color.a);
}
