#pragma once

#include "resource_handles.h"

namespace pong
{

class Material;
class Mesh;
struct Model;
struct Shader;
class Texture2D;

template <typename T>
struct ResourceTraits;

template <>
struct ResourceTraits<Material>
{
    using handle_type = MaterialHandle;
};

template <>
struct ResourceTraits<Mesh>
{
    using handle_type = MeshHandle;
};

template <>
struct ResourceTraits<Model>
{
    using handle_type = ModelHandle;
};

template <>
struct ResourceTraits<Shader>
{
    using handle_type = ShaderHandle;
};

template <>
struct ResourceTraits<Texture2D>
{
    using handle_type = Texture2DHandle;
};

} // namespace pong
