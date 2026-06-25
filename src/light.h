#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

#include <string>
#include <vector>

#include "shader.h"
#include "vertex_buffer.h"

namespace LrnGL {

enum LightType : int
{
    LightType_Point,
    LightType_Spot,
    LightType_Directional,
    LightType_Invalid,
};

struct LightData
{
    int       Type      = LightType_Point;
    glm::vec3 Position  = glm::vec3(0.0f);
    glm::vec3 Direction = glm::vec3(1.0f, -1.0f, 1.0f);
    float     Intensity = 1.0f;

    glm::vec3 Color             = glm::vec3(1.0f);
    glm::vec3 Specular          = glm::vec3(1.0f);
    float     SpecularStrength  = 1.0f;
    int       SpecularShininess = 32;
};

class LightManager
{
public:
    LightManager(const std::string& asset_dir);

    void SetAmbientLight(glm::vec3 color);
    void UpdateMenu();

    void PushLightInfoToShader(Shader& obj_shader, glm::vec3 camera_position);
    void DrawDebugInfo(const glm::mat4& projection, const glm::mat4& view);

private:
    bool                          m_RenderDebugInfo = true;
    Shader                        m_LightDebugShader;
    std::vector<LrnGL::LightData> m_LightData;
    glm::vec3                     m_AmbientLight = glm::vec3(0.1f);
    VertexBuffer                  m_Buffer;
};

} // namespace LrnGL
