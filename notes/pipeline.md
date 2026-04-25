“Refactor pipeline handling just enough to support multiple pipeline variants cleanly, then implement Lighting v1 as the first additional pipeline-driven feature.”

A pipeline is not just shaders. It is a full render-state package:
    shader stages
    descriptor set layouts
    push constant ranges
    vertex input layout
    raster state
    blend state
    depth/stencil state
    render target compatibility

Examples of pipeline keys:
    opaque vs alpha blended
    double-sided vs back-face culled
    skinned vs static mesh
    lit vs unlit
    shadow pass vs main pass
    wireframe/debug variants
    normal mapping on/off
    vertex format differences

The usual architecture becomes:
    Pipeline description/key
    Pipeline cache/factory
    Pipeline resources/layout metadata
    Render pass or pass-type abstraction
    Draw-item sort key that includes pipeline identity

A mature but still sane pipeline system often looks like this conceptually:
    PipelineKey
        Small value type describing the features/state that matter for pipeline creation.
        Example categories: pass type, blend mode, cull mode, shader feature bits, vertex layout id.
        
    PipelineLayout / ResourceLayout definition
        Separate from actual pipeline creation.
        Defines descriptor set contracts and push constants.
        Lets you reuse the same layout across multiple pipelines.

    PipelineManager or PipelineLibrary
        Given a PipelineKey, returns an existing pipeline or creates one lazily.
        Owns the cache and creation policy.
        Renderer stops constructing pipelines directly.
        
    Material-to-pipeline compatibility layer
        Material says what features it needs.
        Renderer derives a PipelineKey from material + pass + mesh features.
        PipelineManager resolves the actual Vulkan pipeline.

    Pass-oriented rendering
        Main color pass
        Shadow pass
        Debug pass
        UI pass
    Each pass may use different pipelines even for the same mesh/material.

Do not make pipeline keys depend on every material parameter. Only include things that actually change pipeline state or shader compilation.
Do not let descriptor set layout drift independently across “similar” pipelines unless there is a real reason.

Make sure the renderer no longer assumes exactly one graphics pipeline.
Introduce the concept of a pipeline key or pipeline id.
Move pipeline lookup/creation behind one abstraction.
Keep the descriptor set contract stable if possible:
    set 0 = frame/global
    set 1 = material
This will help lighting a lot.

Concretely, the readiness refactor should achieve these goals:
    Renderer can bind pipelines by id/key rather than assuming one global pipeline.
    Draw sorting already accounts for pipeline id in a real way.
    Pipeline creation is centralized.
    Pipeline layout compatibility is explicit.

If you have those, Lighting v1 becomes a good forcing function instead of a hack. You can add:
    unlit pipeline
    lit pipeline
    maybe debug normal pipeline later
That gives you a natural test of the new design without overbuilding it.
