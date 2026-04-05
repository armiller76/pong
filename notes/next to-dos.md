Implementation Sequence (small PR-sized steps)

~~~Align shader UBO layout with your CPU struct~~~ and use it in vertex position transform.
Add per-frame UBO upload in renderer (view/proj).
Replace entities.at(0) path with entity loop + per-entity draw.
Add push-constant range to pipeline layout for model matrix and push before each draw.
Add empty-scene safe path and basic tests/manual checks for 0, 1, and N entities.
If you want, next I can give you a concrete checklist for each file in order so you can implement this without backtracking.



Recommendation for question 1 (coordinate conventions):

Use right-handed world/view space.
Keep Vulkan depth range (z \in [0,1]), which you already configured via GLM in glm_wrapper.h:3.
Keep the Y-flip in projection handling explicit in one place (camera/projection builder), not scattered in renderer code.
Given your goals (3D camera, ~100 entities, scene culling, one pipeline), the next logical step is:

Stabilize the render data contract

Per-frame UBO: view + projection only.

Per-draw push constants: model matrix.

This maps cleanly to static+dynamic entities and avoids per-entity descriptor updates.

Convert renderer from single-entity to list-driven drawing

Replace the hardcoded first-entity fetch in vulkan_renderer.cpp:113 with iteration over a prebuilt draw list.

Keep clear+present even if list is empty.

Bind frame descriptor set once per frame (you already do this pattern in vulkan_renderer.cpp:160).

Introduce draw-item sorting now (good call for future Scene integration)

Build a lightweight DrawItem list after scene culling.

Sort key for now: pipeline id, material id, mesh handle, depth bucket.

Since you have one pipeline currently, pipeline id is constant, but keeping it in the key avoids redesign later.

Mesh-handle sorting gives immediate reduction in vertex/index rebinds.

Material id can be a placeholder field now (set to 0) and become real later.

Add minimal state-change tracking in record phase

Track last bound mesh and only rebind when mesh changes.

Keep one draw call per entity.

Push model matrix before each draw.

Wire frame updates every frame

Update current-frame UBO before recording command buffer.

Use frame index from command context to select correct buffer/set.

This is already scaffolded by your per-frame buffers in vulkan_renderer.cpp:34.

Most important immediate gap to close:

Shader/CPU uniform mismatch.
Shader currently uses a temp matrix and ignores camera/model transform in simple.vert:5 and simple.vert:16, while CPU struct expects model/view/proj in ubo.h:8.
Fixing this is prerequisite for meaningful frame-by-frame camera/entity behavior.
Suggested short milestone plan:

Milestone A: shader + UBO contract alignment, no sorting yet.
Milestone B: entity loop + empty-scene safe path.
Milestone C: DrawItem list + sort by mesh/material/pipeline key.
Milestone D: scene culling hands renderer a prefiltered draw list.