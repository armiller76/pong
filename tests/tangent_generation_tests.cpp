#include <gtest/gtest.h>

#include "graphics/mesh_view.h"
#include "graphics/vertex.h"
#include "mikktspace/mikktspace_impl.h"
#include "utils/exception.h"

namespace pong
{

// Helper to create a simple triangle mesh for testing
struct SimpleTriangleMesh
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    // Create a single axis-aligned triangle in XY plane
    // Position in XY, normal pointing Z+, UVs in 0..1 range
    static SimpleTriangleMesh create_simple_triangle()
    {
        return SimpleTriangleMesh{
            .vertices =
                {
                    Vertex{
                        .position = {0.0f, 0.0f, 0.0f},
                        .color = {1.0f, 1.0f, 1.0f, 1.0f},
                        .normal = {0.0f, 0.0f, 1.0f},
                        .uv = {0.0f, 0.0f},
                        .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},
                    Vertex{
                        .position = {1.0f, 0.0f, 0.0f},
                        .color = {1.0f, 1.0f, 1.0f, 1.0f},
                        .normal = {0.0f, 0.0f, 1.0f},
                        .uv = {1.0f, 0.0f},
                        .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},
                    Vertex{
                        .position = {0.0f, 1.0f, 0.0f},
                        .color = {1.0f, 1.0f, 1.0f, 1.0f},
                        .normal = {0.0f, 0.0f, 1.0f},
                        .uv = {0.0f, 1.0f},
                        .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},
                },
            .indices = {0, 1, 2}};
    }

    // Create two triangles sharing an edge with different UV gradients to force seam split
    static SimpleTriangleMesh create_seam_mesh()
    {
        return SimpleTriangleMesh{
            .vertices =
                {
                    // Triangle 1
                    Vertex{
                        .position = {0.0f, 0.0f, 0.0f},
                        .color = {1.0f, 1.0f, 1.0f, 1.0f},
                        .normal = {0.0f, 0.0f, 1.0f},
                        .uv = {0.0f, 0.0f},
                        .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},
                    Vertex{
                        .position = {1.0f, 0.0f, 0.0f},
                        .color = {1.0f, 1.0f, 1.0f, 1.0f},
                        .normal = {0.0f, 0.0f, 1.0f},
                        .uv = {1.0f, 0.0f},
                        .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},
                    Vertex{
                        .position = {0.0f, 1.0f, 0.0f},
                        .color = {1.0f, 1.0f, 1.0f, 1.0f},
                        .normal = {0.0f, 0.0f, 1.0f},
                        .uv = {0.0f, 1.0f},
                        .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},

                    // Triangle 2, shares edge (1,2) but with mirrored UV
                    Vertex{
                        .position = {1.0f, 1.0f, 0.0f},
                        .color = {1.0f, 1.0f, 1.0f, 1.0f},
                        .normal = {0.0f, 0.0f, 1.0f},
                        .uv = {1.0f, 1.0f},
                        .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},
                },
            .indices = {0, 1, 2, 1, 3, 2}}; // shares vertices 1 and 2 at seam
    }
};

TEST(TangentGeneration, SimpleTriangleGeneratesTangents)
{
    auto mesh = SimpleTriangleMesh::create_simple_triangle();
    auto mikk_view =
        MeshViewMikk{{mesh.vertices.data(), mesh.vertices.size()}, {mesh.indices.data(), mesh.indices.size()}, {}};

    mikk_view.tangents.resize(mikk_view.indices.size());

    // Should not throw
    EXPECT_NO_THROW(calculate_tangents(mikk_view));

    // Verify output size
    EXPECT_EQ(mikk_view.tangents.size(), 3);

    // Verify tangents are not all zero (they should be computed)
    for (const auto &tangent : mikk_view.tangents)
    {
        const auto length_sq = tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z;
        EXPECT_GT(length_sq, 0.001f) << "Tangent should be non-zero";
    }
}

TEST(TangentGeneration, OutputSizeMatchesIndexCount)
{
    auto mesh = SimpleTriangleMesh::create_simple_triangle();
    auto mikk_view =
        MeshViewMikk{{mesh.vertices.data(), mesh.vertices.size()}, {mesh.indices.data(), mesh.indices.size()}, {}};

    const auto expected_tangent_count = mikk_view.indices.size();
    mikk_view.tangents.resize(mikk_view.indices.size());
    calculate_tangents(mikk_view);

    EXPECT_EQ(mikk_view.tangents.size(), expected_tangent_count);
}

TEST(TangentGeneration, FailsOnMisalignedIndexCount)
{
    std::vector<Vertex> vertices = {
        Vertex{
            .position = {0.0f, 0.0f, 0.0f},
            .color = {1.0f, 1.0f, 1.0f, 1.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .uv = {0.0f, 0.0f},
            .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{
            .position = {1.0f, 0.0f, 0.0f},
            .color = {1.0f, 1.0f, 1.0f, 1.0f},
            .normal = {0.0f, 0.0f, 1.0f},
            .uv = {1.0f, 0.0f},
            .tangent = {0.0f, 0.0f, 0.0f, 0.0f}},
    };
    std::vector<std::uint32_t> bad_indices = {0, 1}; // Only 2 indices, not divisible by 3

    auto mikk_view = MeshViewMikk{{vertices.data(), vertices.size()}, {bad_indices.data(), bad_indices.size()}, {}};
    mikk_view.tangents.resize(mikk_view.indices.size());

    EXPECT_THROW(calculate_tangents(mikk_view), arm::Exception);
}

TEST(VertexDeduplication, NoSeamProducesNoSplit)
{
    auto mesh = SimpleTriangleMesh::create_simple_triangle();
    auto mikk_view =
        MeshViewMikk{{mesh.vertices.data(), mesh.vertices.size()}, {mesh.indices.data(), mesh.indices.size()}, {}};

    mikk_view.tangents.resize(mikk_view.indices.size());
    calculate_tangents(mikk_view);

    // Manually rebuild as done in resource_loader
    auto new_vertices = std::vector<Vertex>();
    auto new_indices = std::vector<std::uint32_t>();
    auto map = std::unordered_map<Vertex, std::uint32_t>();

    for (std::size_t i = 0; i < mikk_view.indices.size(); ++i)
    {
        auto vertex_index = mikk_view.indices[i];
        auto new_vertex = Vertex{
            mikk_view.vertices[vertex_index].position,
            mikk_view.vertices[vertex_index].color,
            mikk_view.vertices[vertex_index].normal,
            mikk_view.vertices[vertex_index].uv,
            mikk_view.tangents[i]};

        auto found_in_map = map.find(new_vertex);
        if (found_in_map != map.end())
        {
            new_indices.push_back(found_in_map->second);
        }
        else
        {
            auto new_index = new_vertices.size();
            new_vertices.push_back(new_vertex);
            map.emplace(new_vertex, new_index);
            new_indices.push_back(new_index);
        }
    }

    // Single triangle with no shared vertices, no split expected
    EXPECT_EQ(new_vertices.size(), 3);
    EXPECT_EQ(new_indices.size(), 3);
}

TEST(VertexDeduplication, SeamMayProduceSplit)
{
    auto mesh = SimpleTriangleMesh::create_seam_mesh();
    auto mikk_view =
        MeshViewMikk{{mesh.vertices.data(), mesh.vertices.size()}, {mesh.indices.data(), mesh.indices.size()}, {}};

    mikk_view.tangents.resize(mikk_view.indices.size());
    calculate_tangents(mikk_view);

    // Manually rebuild
    auto new_vertices = std::vector<Vertex>();
    auto new_indices = std::vector<std::uint32_t>();
    auto map = std::unordered_map<Vertex, std::uint32_t>();

    for (std::size_t i = 0; i < mikk_view.indices.size(); ++i)
    {
        auto vertex_index = mikk_view.indices[i];
        auto new_vertex = Vertex{
            mikk_view.vertices[vertex_index].position,
            mikk_view.vertices[vertex_index].color,
            mikk_view.vertices[vertex_index].normal,
            mikk_view.vertices[vertex_index].uv,
            mikk_view.tangents[i]};

        auto found_in_map = map.find(new_vertex);
        if (found_in_map != map.end())
        {
            new_indices.push_back(found_in_map->second);
        }
        else
        {
            auto new_index = new_vertices.size();
            new_vertices.push_back(new_vertex);
            map.emplace(new_vertex, new_index);
            new_indices.push_back(new_index);
        }
    }

    // Two triangles sharing an edge with different UV gradients may cause tangent split
    EXPECT_GE(new_vertices.size(), 4); // At least 4 verts (some may be split)
    EXPECT_EQ(new_indices.size(), 6);  // Still 6 index buffer entries (2 triangles)
}

TEST(VertexDeduplication, RebuildIndexBufferIsValid)
{
    auto mesh = SimpleTriangleMesh::create_simple_triangle();
    auto mikk_view =
        MeshViewMikk{{mesh.vertices.data(), mesh.vertices.size()}, {mesh.indices.data(), mesh.indices.size()}, {}};

    mikk_view.tangents.resize(mikk_view.indices.size());
    calculate_tangents(mikk_view);

    // Rebuild
    auto new_vertices = std::vector<Vertex>();
    auto new_indices = std::vector<std::uint32_t>();
    auto map = std::unordered_map<Vertex, std::uint32_t>();

    for (std::size_t i = 0; i < mikk_view.indices.size(); ++i)
    {
        auto vertex_index = mikk_view.indices[i];
        auto new_vertex = Vertex{
            mikk_view.vertices[vertex_index].position,
            mikk_view.vertices[vertex_index].color,
            mikk_view.vertices[vertex_index].normal,
            mikk_view.vertices[vertex_index].uv,
            mikk_view.tangents[i]};

        auto found_in_map = map.find(new_vertex);
        if (found_in_map != map.end())
        {
            new_indices.push_back(found_in_map->second);
        }
        else
        {
            auto new_index = new_vertices.size();
            new_vertices.push_back(new_vertex);
            map.emplace(new_vertex, new_index);
            new_indices.push_back(new_index);
        }
    }

    // All indices must point to valid vertices
    for (const auto idx : new_indices)
    {
        EXPECT_LT(idx, new_vertices.size()) << "Index out of bounds";
    }
}

} // namespace pong
