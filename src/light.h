#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>

#include <string>
#include <vector>

#include "shader.h"
#include "vertex_buffer.h"

namespace LrnGL {

struct LightData
{
    enum Type : int
    {
        Point,
        Spot,
        Directional,
    };

    static constexpr float DefaultAmbientMultiplier = 0.1f;

    int       Type      = LightData::Point;
    glm::vec3 Position  = glm::vec3(0.0f);
    glm::vec3 Direction = glm::vec3(1.0f, -1.0f, 1.0f);
    float     Intensity = 1.0f;

    float Constant        = 1.0f;
    float Linear          = 0.14f;
    float Quadratic       = 0.07f;
    float SpotCutOff      = 7.0f;
    float SpotOuterCutOff = 7.0f;

    glm::vec3 Color    = glm::vec3(1.0f);
    glm::vec3 Ambient  = glm::vec3(0.0f);
    glm::vec3 Specular = glm::vec3(1.0f);

    bool ShowDebugVisual = true;
};

class LightManager
{
public:
    LightManager(const std::string& asset_dir);

    void       PushLight(LightData&& light);
    LightData* GetLight(unsigned index);

    void SetGlobalAmbientLight(glm::vec3 color);
    void EditLightPropertiesMenu();

    void PushLightInfoToShader(Shader& obj_shader, glm::vec3 camera_position);
    void DrawDebugInfo(const glm::mat4& projection, const glm::mat4& view);

private:
    void EditLightProperties(LightData& light);
    void EditLightAttenuationProperties(LightData& light);
    void EditLightColor(LightData& light);

    bool                          m_RenderDebugInfo = true;
    Shader                        m_LightDebugShader;
    glm::vec3                     m_GlobalAmbientLight = glm::vec3(0.07f);
    std::vector<LrnGL::LightData> m_LightData;
    VertexBuffer                  m_Buffer;
};

} // namespace LrnGL
