#version 450

#ifndef MAX_DIR_LIGHTS
#define MAX_DIR_LIGHTS 8
#endif

#define PI 3.14159265358979323846

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
    Directional directionals[MAX_DIR_LIGHTS];
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
layout(location = 5) in vec3 in_camera_pos;
layout(location = 6) in vec3 in_world_pos;

layout(location = 0) out vec4 out_color;

float smith(vec3 x, vec3 n, float k)
{
    return max(dot(n, x), 0.0) / (max(dot(n, x), 0.0) * (1 - k) + k);
}

vec3 kill_diffuse(vec3 f, float m)
{
    return (1.0 - f) * (1 - m);
}

vec3 brdf(
    vec3 srgb_base_color,
    vec3 frag_dir,
    float frag_intensity,
    vec3 frag_color,
    vec3 world_normal,
    vec3 v,
    vec3 f0,
    float alpha,
    float metallic)
{
    vec3 light_dir = normalize(-frag_dir);
    float dot_wn_l = max(dot(world_normal, light_dir), 0.0);

    vec3 h = normalize(v + light_dir);
    vec3 f = f0 + (1.0 - f0) * pow(1.0 - max(dot(h, v), 0.0), 5.0);
    float dot_wn_h = max(dot(world_normal, h), 0.0);
    float ndf = pow(alpha, 2.0) / (PI * (pow((pow(dot_wn_h, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0), 2.0)));
    float g = smith(v, world_normal, alpha / 2.0) * smith(light_dir, world_normal, alpha / 2.0);

    vec3 specular = (ndf * f * g) / max(4 * max(dot(world_normal, v), 0.0) * dot_wn_l, 0.0001);
    return (kill_diffuse(f, metallic) * srgb_base_color / PI + specular) * frag_color * frag_intensity * dot_wn_l;
}

void main()
{
    vec3 final_light = vec3(0.0, 0.0, 0.0);
    vec3 t = normalize(in_tangent);
    vec3 b = normalize(in_bitangent);
    vec3 n = normalize(in_normal);
    vec3 v = normalize(in_camera_pos - in_world_pos);
    mat3 tbn = mat3(t, b, n);

    // normals
    vec3 sampled_normal = texture(normal_sampler, in_uv).rgb;
    sampled_normal = normalize(sampled_normal * 2.0 - 1.0);
    vec3 world_normal = normalize(tbn * sampled_normal);
    float dot_wn_v = max(dot(world_normal, v), 0.0);

    // base color
    vec4 sampled_base_color = texture(base_color_factor_sampler, in_uv) * ubo_material.base_color_factor;
    vec3 srgb_base_color = pow(sampled_base_color.rgb, vec3(2.2));

    // metallic / roughness->alpha
    vec4 metal_rough = texture(metallic_factor_sampler, in_uv);
    float metallic = metal_rough.b * ubo_material.metallic_factor;
    vec3 f0 = mix(vec3(0.04), srgb_base_color, metallic);
    float roughness = max(metal_rough.g * ubo_material.roughness_factor, 0.05);
    float alpha = pow(roughness, 2.0);

    // ambient term
    vec3 ambient = ubo_light.ambient_color_strength.xyz * ubo_light.ambient_color_strength.w;

    // directional lights
    for (int i = 0; i < min(ubo_light.light_counts.x, MAX_DIR_LIGHTS); i++)
    {
        Directional light = ubo_light.directionals[i];
        final_light += brdf(
            srgb_base_color,
            light.direction_intensity.xyz,
            light.direction_intensity.w,
            light.color.xyz,
            world_normal,
            v,
            f0,
            alpha,
            metallic);
    }

    vec3 result = srgb_base_color * ambient + final_light;
    out_color = vec4(pow(result, vec3(1.0 / 2.2)), sampled_base_color.a);
}
