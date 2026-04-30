#version 450

layout(std140, set = 0, binding = 0) uniform UBO
{
    mat4 view;
    mat4 proj;
}
ubo_vp;

layout(push_constant) uniform PushConstants
{
    mat4 model;
}
in_model;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec2 in_uv;
layout(location = 4) in vec4 in_tangent; // tangent = .xyz, sign = .w

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out vec3 out_tangent;
layout(location = 4) out vec3 out_bitangent;

void main()
{
    gl_Position = ubo_vp.proj * ubo_vp.view * in_model.model * vec4(in_position, 1.0);

    out_color = in_color;
    out_uv = in_uv;

    // transform normal and tangent to world space
    mat3 N = transpose(inverse(mat3(in_model.model)));
    out_normal = normalize(N * in_normal);
    out_tangent = normalize(N * in_tangent.xyz);

    // reorthogonalize tangent
    out_tangent = normalize(out_tangent - out_normal * dot(out_normal, out_tangent));

    // calculate bitangent
    out_bitangent = in_tangent.w * normalize(cross(out_normal, out_tangent.xyz));
}
