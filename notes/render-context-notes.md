## Three layers:

1. Pure infrastructure
- instance
- device
- surface
- swapchain-adjacent things (incl the renderer)
- descriptor pool
- command contexts
(mostly depend on Vulkan objects and window state)

2. Runtime services
- ResourceManager
- VulkanPipelineManager
- ResourceLoader
- renderer-facing caches
(depend on infrastructure and on each other by reference)
(but should not do heavy bootstrapping in constructors)

3. App/bootstrap
- creates all of the above
- calls initialize() or bootstrap_default_assets()
- loads scene
- starts frame loop

The composition root should own:
- `VulkanDevice`
- `ResourceManager`
- `VulkanPipelineManager`
- `ResourceLoader`
- `VulkanRenderer`

But ownership is not the same as dependency direction.
- `ResourceLoader` can reference `ResourceManager` and `PipelineManager`
- `VulkanRenderer` can reference `ResourceManager` and `PipelineManager`
- `ResourceManager` should not own `ResourceLoader`
- `PipelineManager` should not need `ResourceLoader` during construction

That keeps the lower-level types from knowing about bootstrap concerns.

What usually changes in practice

Instead of:

- constructor does setup
- constructor creates defaults
- constructor loads fallback assets
- constructor creates pipelines

You move to:

1. Construct objects cheaply.
2. Call explicit setup phases.

For example, conceptually:

1. Create device-related objects.
2. Create `ResourceManager`.
3. Create `PipelineManager`.
4. Create `ResourceLoader`.
5. Create `Renderer`.
6. Call a bootstrap step:
   - create fallback texture
   - load default shaders
   - register default material data
   - warm default pipeline if desired

That means "defaults exist" becomes a post-construction invariant, not a constructor side effect.

Why this is better than stuffing it into Renderer

If `VulkanRenderer` owns and initializes everything, it becomes:
- renderer
- asset bootstrapper
- dependency coordinator
- startup state machine

That tends to rot fast.

The composition root lets `VulkanRenderer` go back to one job:
- frame preparation
- recording
- submission
- swapchain/resource recreation

It should consume already-wired services, not invent them.

What this might look like architecturally

You likely want one higher-level object with a name like:
- `RenderRuntime`
- `RenderContext`
- `EngineServices`
- `GraphicsModule`
- `AppRendererContext`

Its job is not "render".
Its job is "assemble the render subsystem".

It would hold the members in dependency order, then expose a smaller API outward.

For example, conceptually it owns:
- device/surface/swapchain infrastructure
- descriptor pool
- resource manager
- pipeline manager
- resource loader
- renderer

Then it offers methods like:
- `initialize()`
- `load_scene(...)`
- `render_frame(...)`
- `shutdown()`

That gives you one place to reason about order.

The key rule

Constructors should establish local validity.
They should not establish global engine readiness.

That means:
- `ResourceLoader` constructor should not load default shaders just because it can.
- `PipelineManager` constructor should not assume resource defaults are already registered unless explicitly passed what it needs.
- `ResourceManager` should not know how resources are loaded.

How to think about your current default pipeline problem

Ask:
- Is the default pipeline a construction requirement?
- Or just a bootstrap convenience?

Usually it is bootstrap convenience.

So:
- either create it lazily on first use
- or create it in the composition root after default shaders/material state are ready

Both are cleaner than trying to force constructor order to encode runtime policy.

A good smell test

If you need to reorder class members to make the engine "work", that often means bootstrap logic is in the wrong place.

If instead you can say:
1. create services
2. initialize defaults
3. run

then the architecture is usually in a healthier place.

Applied to your codebase, the likely clean split is:
- `ResourceManager`: owns loaded resources only
- `ResourceLoader`: loads/uploads resources into the manager
- `PipelineManager`: creates/caches pipelines from already-available state
- `VulkanRenderer`: renders using those systems
- composition root: builds and initializes all four in the right order

If you want, I can next sketch a concrete initialization sequence for your exact types without proposing code.