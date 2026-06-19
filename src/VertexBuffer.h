#pragma once

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
    void Draw(Shader& shader);

private:
    VertexBuffer() = default;

    unsigned m_VertexArray            = 0;
    std::array<unsigned, 2> m_Buffers = {0, 0};
    size_t m_BufferSize               = 0;
    bool m_IsDynamic                  = false;
};

} // namespace lgl
