
Lighting v1
    One directional light.
    Light color, intensity, direction.
    Shading uses normal plus material factors.
    Goal is visual correctness and controllable results.

Lighting v2
    Add point and spot lights.
    Add a bounded light list per frame.
    Keep one global lighting block, not per-object light copies.

Lighting v3
    Add shadows for the directional light first.
    Then optional point/spot shadows.
    Separate shadow settings from core light data.

Lighting v4
    Scale to many lights with culling strategy.
    Move from simple forward to tiled/clustered forward only when needed.


Keep two representations
    Engine light model: clean, expressive, easy to edit.
    GPU light model: tightly packed, alignment-safe, frame-upload structure.
    Do not collapse these into one struct too early.

Stay in linear color space internally
    Light colors and intensities in linear.
    Do conversion only at presentation/output boundaries.

Use physically sensible units where possible
    Intensity semantics should be explicit and consistent.
    This avoids re-tuning everything later when adding exposure or HDR.

Avoid hardcoding single-light assumptions in APIs
    Even if you only use one light now, make interfaces plural-ready.
    Example mindset: active directional light now, extensible light collection later.

Keep material and light concerns separate
    Materials describe surface response.
    Lights describe incoming illumination.
    Mixing these creates painful coupling later.

Plan descriptor and buffer ownership early
    Per-frame light buffer.
    Clear update point each frame.
    This prevents descriptor churn when light count grows.

---

Normal Mapping Implementation Checklist (step #1 for v1 quality pass)

Why first:
    Biggest visual improvement with moderate complexity.
    Unlocks proper surface detail without adding geometry.
    Paves way for PBR later (specular, roughness require normal).

CPU Load-Time (Mesh data)
    [X] Add tangent field to Vertex struct in src/graphics/vertex.h
        - vec3 tangent + float handedness in vec4 format
        - Update vertex input binding/attribute descriptions
        - Vertex layout now: position, color, normal, uv, tangent
    
    [X] Compute tangents in mesh loader (resource_loader or glTF parse path)
        - For each triangle, compute per-triangle T and B from position + UV edges
        - Accumulate per-vertex T/B (smooth across faces)
        - Orthonormalize: T = normalize(T - dot(N, T) * N)
        - Compute handedness: tangent.w = sign(dot(cross(N, T), B))
        - Store final vec4 tangent in vertex buffer
    
    [X] Verify tangent generation does not crash on degenerate UVs
        - Check determinant > epsilon before division
        - Fall back to reasonable default if needed (e.g., (1,0,0,1))

GPU Vertex Shader (src/assets/shaders/src/simple.vert)
    [X] Add tangent input layout(location = N) in vec4 in_tangent
    
    [X] Transform normal to world space (using normal matrix if needed)
        - world_normal = normalize(normal_matrix * in_normal)
    
    [X] Transform tangent to world space
        - world_tangent = normalize(normal_matrix * in_tangent.xyz)
    
    [X] Compute bitangent in shader
        - world_bitangent = cross(world_normal, world_tangent) * in_tangent.w
    
    [X] Output to fragment stage
        - Pass world_normal, world_tangent, world_bitangent to fragment
        - Or pass as 3 separate layout(location = N) out vec3

GPU Fragment Shader (src/assets/shaders/src/simple.frag)
    [X] Add normal map sampler input
        - layout(set = 1, binding = 2) uniform sampler2D normal_sampler; (already exists)
    
    [X] Sample and decode normal map
        - vec3 sampled_normal = texture(normal_sampler, in_uv).rgb
        - sampled_normal = normalize(sampled_normal * 2.0 - 1.0)  // [0,1] -> [-1,1]
    
    [X] Assemble TBN matrix
        - mat3 TBN = mat3(in_tangent, in_bitangent, in_normal)
    
    [X] Transform sampled normal to world space
        - vec3 world_normal = normalize(TBN * sampled_normal)
    
    [X] Use world_normal for lighting instead of in_normal
        - Replace existing in_normal use with world_normal in any lighting calc

Validation
    [ ] Shaders compile without errors
    [ ] App runs with at least one mesh loaded
    [ ] Rotate mesh; confirm surface highlights move correctly
    [ ] No black/flat surface (indicates TBN sign flip or misalignment)
    [ ] Specular or bump detail visible (indicates normal map is being read)
    [ ] No seams/discontinuities that weren't in the original model (TBN orthogonalization might need tuning)

Debugging tips
    [ ] Visualize world_normal as RGB to inspect TBN correctness
        - out_color = vec4(normalize(world_normal) * 0.5 + 0.5, 1.0)
        - Should show smooth gradient, not noisy or flipped
    
    [ ] Check tangent.w sign in vertex output
        - out_color = vec4(vec3(in_tangent.w), 1.0)  // should be 1 or -1 (appear white or black)
    
    [ ] Disable normal map temporarily, use flat normal
        - vec3 world_normal = normalize(in_normal)
        - If this looks correct, bug is in normal map path
    
    [ ] Inspect mesh tangents in debugger
        - Verify tangent is not (0,0,0)
        - Verify tangent is reasonably orthogonal to normal

Next steps after validation
    1. Add Blinn-Phong specular using the corrected normal
    2. Tune material roughness/metallic impact on specular
    3. Add hemisphere ambient or improve current ambient model
    4. Consider tone mapping and gamma correction
