#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

#include <string>
#include <vector>

#include "Shader.h"
#include "VertexBuffer.h"

namespace LrnGL {

enum LightType : int
{
    LightType_Point,
    LightType_Spot,
    LightType_Directional,
    LightType_Invalid,
};

struct GPULightData
{
    int Type            = LightType_Point;
    glm::vec3 Position  = glm::vec3(0.0f);
    glm::vec3 Direction = glm::vec3(1.0f, -1.0f, 1.0f);

    glm::vec3 Color    = glm::vec3(1.0f);
    glm::vec3 Ambient  = glm::vec3(0.1f);
    glm::vec3 Specular = glm::vec3(1.0f);

    static void PushToShader(Shader& shader, const std::vector<GPULightData>& light_data);
};

class LightManager
{
public:
    LightManager(const std::string& asset_dir);

    void UpdateMenu();
    void PushLightInfoToShader(Shader& obj_shader);
    void DrawDebugInfo(const glm::mat4& projection, const glm::mat4& view);

private:
    bool m_RenderDebugInfo = true;
    Shader m_LightDebugShader;
    std::vector<LrnGL::GPULightData> m_LightData;
    VertexBuffer m_Buffer;
};

} // namespace LrnGL
