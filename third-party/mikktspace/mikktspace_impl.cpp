#include "graphics/mesh_view.h"
#include "mikktspace/mikktspace.h"
#include "utils/error.h"

namespace
{

auto get_face_count(const SMikkTSpaceContext *ctx) -> int
{
    auto mesh_view = static_cast<pong::MeshViewMikk *>(ctx->m_pUserData);
    arm::ensure(mesh_view->indices.size() % 3 == 0, "mesh index count % 3 must be 0");
    return mesh_view->indices.size() / 3u;
}

auto get_face_vertex_count(const SMikkTSpaceContext *ctx, const int face_index) -> int
{
    (void)ctx;        // avoids clang "unused" warning
    (void)face_index; // avoids clang "unused" warning

    // TODO only triangles for now
    return 3u;
}

auto get_vertex_position(
    const SMikkTSpaceContext *ctx,
    float face_vertex_position_out[],
    const int face_index,
    const int vertex_index) -> void
{
    auto mesh_view = static_cast<pong::MeshViewMikk *>(ctx->m_pUserData);

    const auto index = mesh_view->indices.data()[face_index * 3u + vertex_index];
    const auto &vertex = mesh_view->vertices.data()[index];
    face_vertex_position_out[0] = vertex.position.x;
    face_vertex_position_out[1] = vertex.position.y;
    face_vertex_position_out[2] = vertex.position.z;
}

auto get_normal(
    const SMikkTSpaceContext *ctx,
    float face_vertex_normal_out[],
    const int face_index,
    const int vertex_index) -> void
{
    auto mesh_view = static_cast<pong::MeshViewMikk *>(ctx->m_pUserData);

    const auto index = mesh_view->indices.data()[face_index * 3u + vertex_index];
    const auto &vertex = mesh_view->vertices.data()[index];
    face_vertex_normal_out[0] = vertex.normal.x;
    face_vertex_normal_out[1] = vertex.normal.y;
    face_vertex_normal_out[2] = vertex.normal.z;
}

auto get_uv(const SMikkTSpaceContext *ctx, float face_vertex_uv_out[], const int face_index, const int vertex_index)
    -> void
{
    auto mesh_view = static_cast<pong::MeshViewMikk *>(ctx->m_pUserData);

    const auto index = mesh_view->indices.data()[face_index * 3u + vertex_index];
    const auto &vertex = mesh_view->vertices.data()[index];
    face_vertex_uv_out[0] = vertex.uv.x;
    face_vertex_uv_out[1] = vertex.uv.y;
}

auto set_tangent_space(
    const SMikkTSpaceContext *ctx,
    const float face_vertex_tangent_in[],
    const float sign,
    const int face_index,
    const int vertex_index) -> void
{
    auto mesh_view = static_cast<pong::MeshViewMikk *>(ctx->m_pUserData);

    mesh_view->tangents[face_index * 3u + vertex_index] = {
        face_vertex_tangent_in[0], face_vertex_tangent_in[1], face_vertex_tangent_in[2], sign};
}

} // anonymous namespace

namespace pong
{

auto calculate_tangents(MeshViewMikk &mesh) -> void
{
    auto mikk_interface = SMikkTSpaceInterface{
        .m_getNumFaces = &get_face_count,
        .m_getNumVerticesOfFace = &get_face_vertex_count,
        .m_getPosition = &get_vertex_position,
        .m_getNormal = &get_normal,
        .m_getTexCoord = &get_uv,
        .m_setTSpaceBasic = &set_tangent_space,
        .m_setTSpace = nullptr};
    const auto mikk_context = SMikkTSpaceContext{
        .m_pInterface = &mikk_interface,
        .m_pUserData = static_cast<void *>(&mesh),
    };

    arm::ensure(genTangSpaceDefault(&mikk_context), "MikkTSpace tangent generate failed");
}

} // namespace pong
