# Error Handling Policy

## Goals

- Keep runtime rendering behavior explicit and predictable.
- Avoid hidden control flow in hot paths.
- Standardize when to use exceptions versus status/result values.

## Core Rule

- Use explicit status/result-based error handling in runtime engine systems.
- Reserve exceptions for narrow boundaries where fail-fast behavior is acceptable.

## Vulkan-Hpp Exception Mode

- Keep Vulkan-Hpp exceptions **off** for renderer/runtime code.
- Handle Vulkan results explicitly and map them to engine status values.

Rationale:

- Runtime Vulkan errors (for example swapchain out-of-date, suboptimal, minimize/restore transitions) are operational states that often require controlled recovery logic.
- Explicit handling is easier to reason about in frame loops and synchronization-heavy code.

## Error Policy Matrix

### 1) Engine/Renderer Initialization (startup)

Recommended:

- Explicit status/result returns by default.
- Optional exception boundary at app entrypoint if desired.

Guidance:

- Initialization can tolerate fail-fast behavior more than runtime.
- If using an exception boundary, catch once at a high level and convert to a user-facing error + clean shutdown.

### 2) Runtime Frame Loop (prepare frame, record, submit, present)

Recommended:

- No exceptions.
- Explicit result/status codes only.

Guidance:

- Branch explicitly on expected Vulkan outcomes.
- Keep recovery paths local and clear (recreate swapchain/resources, skip frame, signal resize needed, etc.).

### 3) Resource Loading (runtime path)

Recommended:

- Prefer explicit result/status in systems called by the frame loop.
- For offline or tooling-style loaders, exceptions are acceptable if they are contained.

Guidance:

- Avoid throwing across renderer boundaries.
- Convert low-level failures into structured, actionable error values.

### 4) Tools / Offline Pipelines (asset import, shader build tools)

Recommended:

- Exceptions allowed.

Guidance:

- These paths are usually not latency-critical and often benefit from fail-fast control flow.

### 5) Tests

Recommended:

- Either style is acceptable; favor readability and direct failure signaling.
- Match production path behavior in integration-style tests.

## Practical Conventions

- Keep expected operational outcomes out of "error" exception flow.
- Use error categories to distinguish:
  - Recoverable runtime state transitions
  - Fatal device/runtime failures
  - Programmer/configuration errors
- Log at boundaries where decisions are made (for example recreate, skip, shutdown), not at every low-level call.

## Summary Decision

- Renderer/runtime: explicit status/result handling, Vulkan exceptions off.
- Tooling/offline utilities: exceptions allowed where they improve clarity.

## Core Categories
Ok
Unknown
InvalidArgument
InvalidState
NotInitialized
AlreadyInitialized
Unsupported
Timeout

## Platform / Window
WindowCreationFailed
SurfaceCreationFailed
WindowMinimized (optional recoverable state code, if you model states as errors)

## Vulkan Instance / Device
InstanceCreationFailed
PhysicalDeviceNotFound
DeviceCreationFailed
RequiredExtensionMissing
RequiredFeatureMissing
QueueFamilyNotFound
DeviceLost (fatal)

## Swapchain / Presentation
SwapchainCreationFailed
SwapchainOutOfDate (recoverable)
SwapchainSuboptimal (recoverable)
AcquireImageFailed
PresentFailed
SurfaceLost (usually fatal/reinit)

## Pipeline / Shader
ShaderNotFound
ShaderLoadFailed
ShaderModuleCreationFailed
PipelineLayoutCreationFailed
DescriptorSetLayoutCreationFailed
PipelineCreationFailed
PipelineCacheError (optional)

## Descriptors / Resources
DescriptorPoolCreationFailed
DescriptorAllocationFailed
DescriptorUpdateFailed
BufferCreationFailed
ImageCreationFailed
MemoryAllocationFailed
MemoryMapFailed
ResourceNotFound

## Commands / Sync
CommandPoolCreationFailed
CommandBufferAllocationFailed
FenceCreationFailed
SemaphoreCreationFailed
FenceWaitFailed
QueueSubmitFailed (often fatal)

## Assets / IO
FileNotFound
FileReadFailed
ParseFailed
InvalidAssetData
UnsupportedAssetFormat