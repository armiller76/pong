
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


*Core Idea*
Descriptor and buffer ownership should follow lifetime and update frequency, not convenience.
If you align ownership this way early, lighting and future features (shadows, more lights, post-process) stay additive instead of forcing rewrites.

*Mental Model*
    Think in 4 buckets:
        Static: created once, rarely changed.
        Per-scene or per-level: changes when scene content changes.
        Per-frame: changes every frame.
        Per-draw: changes for each object draw.
        Each bucket should have its own descriptor/buffer strategy.

*Recommended Ownership Split*
        Material/system assets own static resources.
        Renderer owns per-frame resources.
        Scene/CPU model owns logical light data.
        Upload path maps scene data into renderer-owned GPU frame buffers.
    That separation keeps gameplay/editor concerns away from Vulkan lifetime hazards.

*Descriptor Ownership Strategy*

    Material descriptor sets:
        Long-lived.
        Allocated once when material is loaded.
        Should survive swapchain recreation.
    Frame/global descriptor sets:
        Owned by renderer.
        One per frame-in-flight.
        Updated every frame with camera + lighting globals.
    Per-draw descriptor sets:
        Avoid if possible for now.
        Prefer push constants or indexing into per-frame buffers.
    Big principle: avoid descriptor allocation/writes in the inner draw loop.

*Buffer Ownership Strategy*

    Static GPU buffers:
        Material constants that almost never change.
    Per-frame dynamic buffers:
        Camera, light arrays, frame globals.
        Double/triple buffered by frame-in-flight.
    Transient upload/staging buffers:
        Short-lived transfer helpers.
        Optional ring-buffer pattern for dynamic data later.
    Big principle: write only to the current frame’s region, never to data possibly in-flight.

*How This Helps Lighting Specifically*
    For Lighting v1:
        One per-frame global lighting buffer is enough.
        Directional light data goes there with camera data.
        Material set remains material-only (textures/factors).
    For future lighting:
        Adding point/spot lights becomes “grow global light buffer,” not “redesign material descriptors.”
        Adding shadows becomes “add more global resources,” not “touch every material.”

*Swapchain Recreate Boundary*
    Define this now:
        Swapchain-dependent objects can be recreated often.
        Material descriptors and static material buffers should not be touched during resize/minimize.
        Frame-global resources may resize or rebind as needed, but ownership stays renderer-local.
    If this boundary is blurred, resize bugs and descriptor churn come back.

*Common Mistakes to Avoid*
    Per-material data in per-frame buffers without frame separation.
    Per-draw descriptor allocation/update.
    Letting window/event layer trigger low-level descriptor work directly.
    Coupling swapchain lifetime to asset lifetime.
    Manual freeing in one subsystem while RAII also owns the same object.

*Practical “Is This Correct?” Check*
When adding any new buffer/descriptor, ask:
    Who owns it logically?
    What is its lifetime bucket?
    How often is it updated?
    Is it swapchain-dependent?
    Is it safe with N frames in flight?