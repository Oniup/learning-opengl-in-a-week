#include "vertex_buffer.h"

#include <glad/gl.h>
#include <glm/geometric.hpp>

#include <cstdint>
#include <numbers>

namespace LrnGL {

ShapeMesh ShapeMesh::GetPlane(glm::vec3 color)
{
    glm::vec3 color1 = color == glm::vec3(0.0f) ? glm::vec3(1.0f, 0.0f, 0.0f) : color;
    glm::vec3 color2 = color == glm::vec3(0.0f) ? glm::vec3(0.0f, 1.0f, 0.0f) : color;
    glm::vec3 color3 = color == glm::vec3(0.0f) ? glm::vec3(0.0f, 0.0f, 1.0f) : color;
    glm::vec3 color4 = color == glm::vec3(0.0f) ? glm::vec3(1.0f, 1.0f, 0.0f) : color;

    std::vector<Vertex> vertices = {
        Vertex{{0.5f, 0.5f, 0.0f},   {0.0f, 0.0f, 1.0f}, color1, {1.0f, 1.0f}}, // top right
        Vertex{{0.5f, -0.5f, 0.0f},  {0.0f, 0.0f, 1.0f}, color2, {1.0f, 0.0f}}, // bottom right
        Vertex{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, color3, {0.0f, 0.0f}}, // bottom left
        Vertex{{-0.5f, 0.5f, 0.0f},  {0.0f, 0.0f, 1.0f}, color4, {0.0f, 1.0f}}, // top left
    };
    std::vector<unsigned> indices = {
        // clang-format off
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        // clang-format on
    };
    return ShapeMesh{
        .Vertices = std::move(vertices),
        .Indices  = std::move(indices),
    };
}

ShapeMesh ShapeMesh::GetCube(glm::vec3 color)
{
    glm::vec3 color1 = color == glm::vec3(0.0f) ? glm::vec3(1.0f, 0.0f, 0.0f) : color;
    glm::vec3 color2 = color == glm::vec3(0.0f) ? glm::vec3(0.0f, 1.0f, 0.0f) : color;
    glm::vec3 color3 = color == glm::vec3(0.0f) ? glm::vec3(0.0f, 0.0f, 1.0f) : color;
    glm::vec3 color4 = color == glm::vec3(0.0f) ? glm::vec3(1.0f, 1.0f, 0.0f) : color;

    std::vector<Vertex> vertices = {
        // Back face
        Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, color1, {0.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, -0.5f},  {0.0f, 0.0f, -1.0f}, color2, {1.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, -0.5f},   {0.0f, 0.0f, -1.0f}, color3, {1.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, -0.5f},  {0.0f, 0.0f, -1.0f}, color4, {0.0f, 1.0f}},

        // Front face
        Vertex{{-0.5f, -0.5f, 0.5f},  {0.0f, 0.0f, 1.0f},  color1, {0.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, 0.5f},   {0.0f, 0.0f, 1.0f},  color2, {1.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, 0.5f},    {0.0f, 0.0f, 1.0f},  color3, {1.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, 0.5f},   {0.0f, 0.0f, 1.0f},  color4, {0.0f, 1.0f}},

        // Left face
        Vertex{{-0.5f, -0.5f, 0.5f},  {-1.0f, 0.0f, 0.0f}, color1, {0.0f, 0.0f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, color2, {0.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, -0.5f},  {-1.0f, 0.0f, 0.0f}, color3, {1.0f, 1.0f}},
        Vertex{{-0.5f, 0.5f, 0.5f},   {-1.0f, 0.0f, 0.0f}, color4, {1.0f, 0.0f}},

        // Right face
        Vertex{{0.5f, -0.5f, -0.5f},  {1.0f, 0.0f, 0.0f},  color1, {0.0f, 1.0f}},
        Vertex{{0.5f, -0.5f, 0.5f},   {1.0f, 0.0f, 0.0f},  color2, {0.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, 0.5f},    {1.0f, 0.0f, 0.0f},  color3, {1.0f, 0.0f}},
        Vertex{{0.5f, 0.5f, -0.5f},   {1.0f, 0.0f, 0.0f},  color4, {1.0f, 1.0f}},

        // Bottom face
        Vertex{{-0.5f, -0.5f, 0.5f},  {0.0f, -1.0f, 0.0f}, color1, {0.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, 0.5f},   {0.0f, -1.0f, 0.0f}, color2, {1.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, -0.5f},  {0.0f, -1.0f, 0.0f}, color3, {1.0f, 1.0f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, color4, {0.0f, 1.0f}},

        // Top face
        Vertex{{-0.5f, 0.5f, -0.5f},  {0.0f, 1.0f, 0.0f},  color1, {0.0f, 1.0f}},
        Vertex{{0.5f, 0.5f, -0.5f},   {0.0f, 1.0f, 0.0f},  color2, {1.0f, 1.0f}},
        Vertex{{0.5f, 0.5f, 0.5f},    {0.0f, 1.0f, 0.0f},  color3, {1.0f, 0.0f}},
        Vertex{{-0.5f, 0.5f, 0.5f},   {0.0f, 1.0f, 0.0f},  color4, {0.0f, 0.0f}}
    };

    // clang-format off
        std::vector<unsigned> indices = {
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

    return ShapeMesh{
        .Vertices = std::move(vertices),
        .Indices  = std::move(indices),
    };
}

ShapeMesh ShapeMesh::GenerateSphere(unsigned subdivisions, unsigned height_subdivisions,
                                    glm::vec3 color)
{
    ShapeMesh mesh;
    mesh.Vertices.reserve((height_subdivisions + 1) * (subdivisions + 1));
    mesh.Indices.reserve(height_subdivisions * subdivisions * 6);

    // Calculate vertices
    for (unsigned i = 0; i <= height_subdivisions; i++)
    {
        float vertical_progress = static_cast<float>(i) / height_subdivisions;
        float theta             = vertical_progress * std::numbers::pi;

        for (unsigned j = 0; j <= subdivisions; j++)
        {
            float horizontal_progress = static_cast<float>(j) / subdivisions;
            float phi                 = horizontal_progress * 2.0f * std::numbers::pi;

            Vertex vertex;

            vertex.Position.x = std::sin(theta) * std::cos(phi);
            vertex.Position.y = std::cos(theta);
            vertex.Position.z = std::sin(theta) * std::sin(phi);

            vertex.Normal = glm::normalize(vertex.Position);

            vertex.TexCoords = glm::vec2(horizontal_progress, vertical_progress);
            vertex.Color     = color;

            mesh.Vertices.push_back(vertex);
        }
    }

    // // Calculate indices
    for (unsigned i = 0; i < height_subdivisions; i++)
    {
        for (unsigned j = 0; j < subdivisions; j++)
        {
            unsigned current_vertex = i * (subdivisions + 1) + j;
            unsigned below_vertex   = current_vertex + subdivisions + 1;

            // triangle 1
            mesh.Indices.push_back(current_vertex);     // top left
            mesh.Indices.push_back(below_vertex);       // bot left
            mesh.Indices.push_back(current_vertex + 1); // top right

            // triangle 2
            mesh.Indices.push_back(current_vertex + 1); // top right
            mesh.Indices.push_back(below_vertex);       // bot left
            mesh.Indices.push_back(below_vertex + 1);   // bot right
        }
    }

    return mesh;
}

bool ShapeMesh::UsesIndices() const
{
    return !Indices.empty();
}

VertexBuffer::VertexBuffer(bool use_elements, bool dynamic)
    : m_IsDynamic(dynamic)
{
    glGenVertexArrays(1, &m_VertexArray);
    glBindVertexArray(m_VertexArray);

    glGenBuffers(1, &m_Buffers[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[0]);
    if (use_elements)
    {
        glGenBuffers(1, &m_Buffers[1]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[1]);
    }
}

VertexBuffer::VertexBuffer(const ShapeMesh& mesh, bool dynamic)
    : VertexBuffer(mesh.UsesIndices(), dynamic)
{
    PushAttribute(0, 3, offsetof(Vertex, Position));
    PushAttribute(1, 3, offsetof(Vertex, Normal));
    PushAttribute(2, 3, offsetof(Vertex, Color));
    PushAttribute(3, 2, offsetof(Vertex, TexCoords));

    PushData(VertexBuffer_Vertex, mesh.Vertices.size() * sizeof(Vertex), mesh.Vertices.data());
    PushData(VertexBuffer_Element, mesh.Indices.size() * sizeof(unsigned), mesh.Indices.data());
}

VertexBuffer::~VertexBuffer()
{
    Destroy();
}

VertexBuffer::VertexBuffer(VertexBuffer&& other)
    : m_VertexArray(other.m_VertexArray),
      m_Buffers(std::move(other.m_Buffers)),
      m_BufferSize(other.m_BufferSize),
      m_IsDynamic(other.m_IsDynamic)
{
    other.m_VertexArray = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other)
{
    Destroy();

    m_VertexArray = other.m_VertexArray;
    m_Buffers     = std::move(other.m_Buffers);
    m_BufferSize  = other.m_BufferSize;
    m_IsDynamic   = other.m_IsDynamic;

    other.m_VertexArray = 0;
    return *this;
}

void VertexBuffer::PushData(VertexBufferType type, size_t size, const void* data)
{
    unsigned target = type == VertexBuffer_Vertex ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    glBindBuffer(target, m_Buffers[type]);
    glBufferData(target, size, data, m_IsDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    if (type == VertexBuffer_Element || !ElementBufferEnabled())
        m_BufferSize = size;
}

void VertexBuffer::PushAttribute(unsigned id, unsigned element_count, unsigned offset_of_field)
{
    glBindVertexArray(m_VertexArray);
    glVertexAttribPointer(id,
                          element_count,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          reinterpret_cast<const void*>(static_cast<uintptr_t>(offset_of_field)));
    glEnableVertexAttribArray(id);
}

bool VertexBuffer::ElementBufferEnabled() const
{
    return m_Buffers.back() != 0;
}

void VertexBuffer::Bind()
{
    glBindVertexArray(m_VertexArray);
}

void VertexBuffer::Draw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
                        const glm::mat4& model) const
{
    shader.Bind();
    glBindVertexArray(m_VertexArray);

    shader.Uniform("u_Projection", projection);
    shader.Uniform("u_View", view);
    shader.Uniform("u_Model", model);

    if (ElementBufferEnabled())
        glDrawElements(GL_TRIANGLES, m_BufferSize, GL_UNSIGNED_INT, (void*)0);
    else
        glDrawArrays(GL_TRIANGLES, 0, m_BufferSize);
}

void VertexBuffer::Destroy()
{
    if (m_VertexArray != 0)
    {
        glDeleteVertexArrays(1, &m_VertexArray);
        glDeleteBuffers(ElementBufferEnabled() ? 2 : 1, m_Buffers.data());
        m_VertexArray = 0;
    }
}

} // namespace LrnGL
