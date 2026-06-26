#include "model.h"

#include "material.h"
#include "vertex_buffer.h"

namespace LrnGL {

Mesh::Mesh(ShapeVertexData&& shape, Material&& material)
    : m_Material(std::move(material)),
      m_Vertices(std::move(shape.Vertices)),
      m_Indices(std::move(shape.Indices)),
      m_VertexBuffer(m_Vertices, m_Indices)
{
}

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned>&& indices, Material&& material)
    : m_Material(std::move(material)),
      m_Vertices(std::move(vertices)),
      m_Indices(std::move(indices)),
      m_VertexBuffer(m_Vertices, m_Indices)
{
}

void Mesh::Draw(const Transform& transform, const glm::mat4& projection, const glm::mat4& view)
{
    m_Material.PushInfoToShader();
    m_VertexBuffer.Draw(*m_Material.Shader, projection, view, transform.CreateModelMatrix());
}

} // namespace LrnGL
