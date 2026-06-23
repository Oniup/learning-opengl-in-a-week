#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

#include <array>

#include "shader.h"

namespace LrnGL {

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Color;
    glm::vec2 TexCoords;
};

enum VertexBufferType : unsigned
{
    VertexBuffer_Vertex,
    VertexBuffer_Element,
};

struct ShapeMesh
{
    static ShapeMesh GetPlane(glm::vec3 color = glm::vec3(1.0f));
    static ShapeMesh GetCube(glm::vec3 color = glm::vec3(1.0f));
    static ShapeMesh GenerateSphere(unsigned subdivisions, unsigned height_subdivisions,
                                    glm::vec3 color = glm::vec3(1.0f));

    std::vector<Vertex>   Vertices;
    std::vector<unsigned> Indices;

    bool UsesIndices() const;
};

class VertexBuffer
{
public:
    static VertexBuffer Invalid;

    VertexBuffer(bool use_elements = false, bool dynamic = false);
    VertexBuffer(const ShapeMesh& mesh, bool dynamic = false);
    ~VertexBuffer();

    VertexBuffer(VertexBuffer&& other);
    VertexBuffer& operator=(VertexBuffer&& other);

    VertexBuffer(const VertexBuffer& other)            = delete;
    VertexBuffer& operator=(const VertexBuffer& other) = delete;

    void PushData(VertexBufferType type, size_t size, const void* data);
    void PushAttribute(unsigned id, unsigned element_count, unsigned offset_of_field);
    bool ElementBufferEnabled() const;

    void Bind();
    void Draw(Shader& shader, const glm::mat4& projection, const glm::mat4& view,
              const glm::mat4& model) const;
    void Destroy();

private:
    VertexBuffer() = default;

    unsigned                m_VertexArray = 0;
    std::array<unsigned, 2> m_Buffers     = {0, 0};
    size_t                  m_BufferSize  = 0;
    bool                    m_IsDynamic   = false;
};

} // namespace LrnGL
