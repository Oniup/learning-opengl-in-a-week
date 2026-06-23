#include "Light.h"

#include <fmt/base.h>
#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "Transform.h"
#include "VertexBuffer.h"

namespace LrnGL {

namespace Internal {

    constexpr std::array<const char*, 3> LightTypeNames = {"Point", "Spot", "Directional"};

} // namespace Internal

LightManager::LightManager(const std::string& asset_dir)
    : m_LightDebugShader(fmt::format("{}/shaders/LightF.glsl", asset_dir),
                         fmt::format("{}/shaders/LightV.glsl", asset_dir)),
      m_LightData({
          GPULightData{.Position = glm::vec3(3.0f, 3.0f, 3.0f)},
      }),
      m_Buffer(ShapeMesh::GenerateSphere(10, 10))
{
}

void LightManager::UpdateMenu()
{
    ImGui::Begin("Light Control");
    ImGui::Checkbox("Render Light Debug Info", &m_RenderDebugInfo);

    if (m_RenderDebugInfo)
    {
        for (unsigned i = 0; i < m_LightData.size(); i++)
        {
            ImGui::PushID(i);
            GPULightData& light = m_LightData[i];

            const char* type = Internal::LightTypeNames[light.Type];
            if (ImGui::CollapsingHeader(type, ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Combo("Type",
                             &light.Type,
                             Internal::LightTypeNames.data(),
                             Internal::LightTypeNames.size());

                ImGui::DragFloat3("Position", glm::value_ptr(light.Position), 0.1f);
                ImGui::ColorEdit3("Color", glm::value_ptr(light.Color));
            }

            ImGui::PopID();
        }

        if (m_LightData.size() < 10)
        {
            if (ImGui::Button("Add Light"))
                m_LightData.push_back(GPULightData{});
        }
    }

    ImGui::End();
}

void LightManager::PushLightInfoToShader(Shader& obj_shader)
{
    obj_shader.Uniform("u_LightCount", static_cast<int>(m_LightData.size()));
    for (unsigned i = 0; i < m_LightData.size(); i++)
    {
        const GPULightData& light = m_LightData[i];

        obj_shader.Uniform(fmt::format("u_Lights[{}].Position", i), light.Position);
        obj_shader.Uniform(fmt::format("u_Lights[{}].Color", i), light.Color);
    }
}

void LightManager::DrawDebugInfo(const glm::mat4& projection, const glm::mat4& view)
{
    if (!m_RenderDebugInfo)
        return;

    for (const GPULightData& light : m_LightData)
    {
        Transform transform{
            .Position = light.Position,
            .Scale    = glm::vec3(0.1f),
            .Rotation = glm::vec3(0.0f),
        };

        m_LightDebugShader.Uniform("u_Color", light.Color);
        m_Buffer.Draw(m_LightDebugShader, projection, view, transform.CreateModelMatrix());
    }
}

} // namespace LrnGL
