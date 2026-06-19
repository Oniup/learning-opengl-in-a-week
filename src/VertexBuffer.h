#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

#include <array>

#include "Shader.h"

namespace lgl {

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec2 UV;
};

enum VertexBufferType : unsigned
{
    VertexBuffer_Vertex,
    VertexBuffer_Element,
};

class VertexBuffer
{
public:
    static VertexBuffer Invalid;

    VertexBuffer(bool use_elements = false, bool dynamic = false);
    ~VertexBuffer();

    void PushData(VertexBufferType type, size_t size, const void* data);
    void PushAttribute(unsigned id, unsigned element_count, unsigned offset_of_field);
    bool ElementBufferEnabled() const;

    void Bind();
    void Draw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
              const glm::mat4& model);

private:
    VertexBuffer() = default;

    unsigned m_VertexArray            = 0;
    std::array<unsigned, 2> m_Buffers = {0, 0};
    size_t m_BufferSize               = 0;
    bool m_IsDynamic                  = false;
};

struct Shapes
{
    static constexpr std::array<Vertex, 4> PlaneVertices = {

        lgl::Vertex{{0.5f, 0.5f, 0.0f},   {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // top right
        lgl::Vertex{{0.5f, -0.5f, 0.0f},  {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
        lgl::Vertex{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // bottom left
        lgl::Vertex{{-0.5f, 0.5f, 0.0f},  {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}  // top left
    };

    // clang-format off
    static constexpr std::array<unsigned, 6> PlaneIndices = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    // clang-format on

    static constexpr std::array<Vertex, 24> CubeVertices = {
        // Back face
        Vertex{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, -0.5f},   {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        // Front face
        Vertex{{-0.5f, -0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, 0.5f},   {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, 0.5f},    {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, 0.5f},   {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        // Left face
        Vertex{{-0.5f, -0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, 0.5f},   {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},

        // Right face
        Vertex{{0.5f, -0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        Vertex{{0.5f, -0.5f, 0.5f},   {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, 0.5f},    {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, -0.5f},   {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

        // Bottom face
        Vertex{{-0.5f, -0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, 0.5f},   {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        // Top face
        Vertex{{-0.5f, 0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        Vertex{{0.5f, 0.5f, -0.5f},   {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        Vertex{{0.5f, 0.5f, 0.5f},    {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        Vertex{{-0.5f, 0.5f, 0.5f},   {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
    };

    // clang-format off
    static constexpr std::array<uint32_t, 36> CubeIndices = {
        // Back face
        0, 1,
        2, 2,
        3, 0,
        // Front face
        4, 5,
        6, 6,
        7, 4,
        // Left face
        11, 10,
        9, 9,
        8, 11,
        // Right face
        14, 15,
        12, 12,
        13, 14,
        // Bottom face
        19, 18,
        17, 17,
        16, 19,
        // Top face
        20, 21,
        22, 22,
        23, 20
    };
    // clang-format on
};

} // namespace lgl
