#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

#include <vector>

#include "Shader.h"
#include "VertexBuffer.h"

namespace lgl {

enum class LightType
{
    Point,
    Spot,
    Directional,
    Invalid,
};

struct GPULightData
{
    LightType type      = LightType::Point;
    glm::vec3 Position  = glm::vec3(0.0f);
    glm::vec3 Direction = glm::vec3(1.0f, -1.0f, 1.0f);

    glm::vec3 Color    = glm::vec3(1.0f);
    glm::vec3 Ambient  = glm::vec3(0.1f);
    glm::vec3 Specular = glm::vec3(1.0f);

    static void PushToShader(Shader& shader, const std::vector<GPULightData>& light_data);
};

class LightRenderer
{
public:
    LightRenderer();

    void Draw(const GPULightData& light, Shader& shader, const glm::mat4& projection,
              const glm::mat4& view);

private:
    VertexBuffer m_Buffer;
};

} // namespace lgl
