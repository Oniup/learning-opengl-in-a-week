#include "VertexBuffer.h"

#include <glad/gl.h>

#include <cstdint>

namespace lgl {

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

VertexBuffer::~VertexBuffer()
{
    if (m_VertexArray != 0)
    {
        glDeleteVertexArrays(1, &m_VertexArray);
        glDeleteBuffers(ElementBufferEnabled() ? 2 : 1, m_Buffers.data());
        m_VertexArray = 0;
    }
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
                        const glm::mat4& model)
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

} // namespace lgl
