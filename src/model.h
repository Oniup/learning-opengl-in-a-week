#pragma once

#include "material.h"
#include "transform.h"
#include "vertex_buffer.h"

namespace LrnGL {

class Model;

class Mesh
{
public:
    Mesh(ShapeVertexData&& shape, Material&& material);
    Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned>&& indices, Material&& material);

    Material&                   GetMaterial() { return m_Material; }
    const Material&             GetMaterial() const { return m_Material; }
    const std::vector<Vertex>   GetVertices() const { return m_Vertices; }
    const std::vector<unsigned> GetIndices() const { return m_Indices; }
    const VertexBuffer&         GetVertexBuffer() const { return m_VertexBuffer; }

    void Draw(const Transform& transform, const glm::mat4& projection, const glm::mat4& view);

private:
    Material              m_Material;
    std::vector<Vertex>   m_Vertices;
    std::vector<unsigned> m_Indices;
    VertexBuffer          m_VertexBuffer;
};

class Model
{
};

} // namespace LrnGL
