#include "Light.h"

#include "Transform.h"
#include "VertexBuffer.h"

namespace lgl {

LightRenderer::LightRenderer()
    : m_Buffer(ShapeMesh::GenerateSphere(10, 10))
{
}

void LightRenderer::Draw(const GPULightData& light, Shader& shader, const glm::mat4& projection,
                         const glm::mat4& view)
{
    Transform transform{
        .Position = light.Position,
        .Scale    = glm::vec3(0.1f),
        .Rotation = glm::vec3(0.0f),
    };

    shader.Uniform("u_Color", light.Color);
    m_Buffer.Draw(shader, projection, view, transform.CreateModelMatrix());
}

} // namespace lgl
