What helper to add
- Add an immediate submit function that:
1. Allocates or reuses a dedicated command buffer from a transfer/immediate command pool.
2. Begins it with one-time-submit flag.
3. Runs a caller-provided recording callback.
4. Ends and submits to graphics queue.
5. Waits for completion via fence (or queue wait as a first version).

Suggested shape (conceptually)
- Method on command context:
  immediate_submit(record_fn)
- record_fn takes a command buffer and records transfer/barrier commands.

Where to place it
- Best first step: add this to vulkan_command_context.h and vulkan_command_context.cpp, since that class already owns pools/buffers/fences.
- It will need access to queue and queue family already exposed by vulkan_device.h.

How this enables textures
- Texture upload flow becomes:
1. Create staging buffer (host visible, transfer src) using vulkan_gpu_buffer.h.
2. Copy pixel data with upload.
3. Create target GpuImage.
4. immediate_submit records:
- barrier: Undefined -> TransferDstOptimal
- copyBufferToImage
- barrier: TransferDstOptimal -> ShaderReadOnlyOptimal
5. Update descriptor image info using combined image sampler binding already defined in vulkan_pipeline_factory.cpp and vulkan_pipeline_factory.cpp.

Important detail
- Add texture-specific transition info entries (for transfer dst and shader read). Right now transition helpers are renderer-focused in vulkan_layout_transition.h, so extending that struct with texture transitions will keep barriers consistent.

If you want, I can draft the exact header and cpp changes for the immediate_submit API in your current style (RAII Vulkan + arm::ensure checks), plus the first texture upload call site skeleton.