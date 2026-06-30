#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>

#include <vector>

#include "shader.h"
#include "vertex_buffer.h"

namespace LrnGL {

struct alignas(16) LightData
{
    enum Type : int
    {
        Point,
        Spot,
        Directional,
    };

    static constexpr float DefaultAmbientMultiplier = 0.1f;

    int   Type            = LightData::Point; // 0
    float Intensity       = 1.0f;             // 4
    float SpotCutOff      = 7.0f;             // 8
    float SpotOuterCutOff = 7.0f;             // 12

    float Constant  = 1.0f;  // 16
    float Linear    = 0.14f; // 20
    float Quadratic = 0.07f; // 24

    alignas(16) glm::vec3 Position  = glm::vec3(0.0f);              // 32
    alignas(16) glm::vec3 Direction = glm::vec3(1.0f, -1.0f, 1.0f); // 48

    alignas(16) glm::vec3 Color    = glm::vec3(1.0f); // 64
    alignas(16) glm::vec3 Ambient  = glm::vec3(0.0f); // 80
    alignas(16) glm::vec3 Specular = glm::vec3(1.0f); // 96
};

class LightManager
{
public:
    static constexpr unsigned MaxLightCount = 10;

    LightManager(bool initialize_uniform_buffer = false);
    ~LightManager();

    void       PushLight(LightData&& light, bool show_debug_visualization = true);
    void       RemoveLight(unsigned index);
    LightData* GetLight(unsigned index);

    void SetGlobalAmbientLight(glm::vec3 color);
    void ShouldRenderDebugInfo(bool render);
    void EditLightPropertiesMenu();

    void PushLightInfoToShader(Shader& shader, glm::vec3 camera_position);
    void PushLightInfoToUniformBlockLayout(Shader& shader, glm::vec3 camera_position);
    void DrawDebugInfo(const glm::mat4& projection, const glm::mat4& view);

private:
    unsigned EditLightProperties(LightData& light, unsigned index);
    unsigned EditLightAttenuationProperties(LightData& light);
    unsigned EditLightColor(LightData& light);

    bool                          m_RenderDebugInfo = true;
    Shader                        m_LightDebugShader;
    glm::vec3                     m_GlobalAmbientLight = glm::vec3(0.07f);
    std::vector<LrnGL::LightData> m_LightData;
    std::vector<bool>             m_ShowLightDebugVisual;
    VertexBuffer                  m_Buffer;
    unsigned                      m_UniformBuffer = 0;
    bool                          m_UpdateLightUniformBuffer;
};

} // namespace LrnGL
