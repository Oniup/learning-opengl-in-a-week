#include "light.h"

#include <fmt/base.h>
#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <algorithm>

#include "transform.h"

namespace LrnGL {

namespace Internal {

    constexpr std::array<const char*, 3> LightTypeNames = {"Point", "Spot", "Directional"};

} // namespace Internal

LightManager::LightManager(const std::string& asset_dir)
    : m_LightDebugShader(fmt::format("{}/shaders/Light.frag", asset_dir),
                         fmt::format("{}/shaders/Light.vert", asset_dir)),
      m_LightData({
          LightData{.Position = glm::vec3(3.0f, 3.0f, 3.0f)},
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
            LightData& light = m_LightData[i];

            const char* type = Internal::LightTypeNames[light.Type];
            if (ImGui::CollapsingHeader(type, ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Combo("Type",
                             &light.Type,
                             Internal::LightTypeNames.data(),
                             Internal::LightTypeNames.size());

                ImGui::SeparatorText("Position and Intensity");
                ImGui::DragFloat3("Position", glm::value_ptr(light.Position), 0.1f);
                ImGui::DragFloat("Intensity", &light.Intensity, 0.1f);

                ImGui::SeparatorText("Light Color");
                ImGui::ColorEdit3("Color", glm::value_ptr(light.Color));
                ImGui::ColorEdit3("Specular", glm::value_ptr(light.Specular));
                ImGui::DragFloat("Specular Strength", &light.SpecularStrength, 0.1f);
                ImGui::SliderInt("Specular Shininess", &light.SpecularShininess, 0, 256);

                light.Intensity        = std::max(light.Intensity, 0.0f);
                light.SpecularStrength = std::max(light.SpecularStrength, 0.0f);
            }

            ImGui::PopID();
        }

        if (m_LightData.size() < 10)
        {
            ImGui::NewLine();

            if (ImGui::Button("Add Light"))
                m_LightData.push_back(LightData{});
        }
    }

    ImGui::End();
}

void LightManager::PushLightInfoToShader(Shader& obj_shader, glm::vec3 camera_position)
{
    obj_shader.Uniform("u_LightCount", static_cast<int>(m_LightData.size()));
    obj_shader.Uniform("u_CameraPosition", camera_position);
    obj_shader.Uniform("u_AmbientColor", m_AmbientLight);

    size_t buffer_length = 50;
    char   name_buffer[buffer_length];

    auto get_location = [&name_buffer, buffer_length](unsigned         index,
                                                      std::string_view field) -> const char*
    {
        auto result = fmt::format_to_n(name_buffer, buffer_length, "u_Lights[{}].{}", index, field);
        *result.out = '\0';
        return name_buffer;
    };

    for (unsigned i = 0; i < m_LightData.size(); i++)
    {
        const LightData& light = m_LightData[i];

        obj_shader.Uniform(get_location(i, "Type"), light.Type);
        obj_shader.Uniform(get_location(i, "Position"), light.Position);
        obj_shader.Uniform(get_location(i, "Intensity"), light.Intensity);

        obj_shader.Uniform(get_location(i, "Color"), light.Color);
        obj_shader.Uniform(get_location(i, "Specular"), light.Specular);

        obj_shader.Uniform(get_location(i, "SpecularStrength"), light.SpecularStrength);
        obj_shader.Uniform(get_location(i, "SpecularShininess"), light.SpecularShininess);
    }
}

void LightManager::DrawDebugInfo(const glm::mat4& projection, const glm::mat4& view)
{
    if (!m_RenderDebugInfo)
        return;

    for (const LightData& light : m_LightData)
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
