#include "light.h"

#include <fmt/base.h>
#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <algorithm>
#include <tuple>

#include "transform.h"
#include "utilities.h"

namespace LrnGL {

namespace Internal {

    constexpr std::array<const char*, 3> LightTypeNames = {"Point", "Spot", "Directional"};

    constexpr std::array<std::tuple<const char*, float, float, float>, 12> LightDistances = {
        std::make_tuple("7", 1.0f, 0.7f, 1.8f),
        std::make_tuple("13", 1.0f, 0.35f, 0.44f),
        std::make_tuple("20", 1.0f, 0.22f, 0.20f),
        std::make_tuple("32", 1.0f, 0.14f, 0.07f),
        std::make_tuple("50", 1.0f, 0.09f, 0.032f),
        std::make_tuple("65", 1.0f, 0.07f, 0.017f),
        std::make_tuple("100", 1.0f, 0.045f, 0.0075f),
        std::make_tuple("160", 1.0f, 0.027f, 0.0028f),
        std::make_tuple("200", 1.0f, 0.022f, 0.0019f),
        std::make_tuple("325", 1.0f, 0.014f, 0.0007f),
        std::make_tuple("600", 1.0f, 0.007f, 0.0002f),
        std::make_tuple("3250", 1.0f, 0.0014f, 0.000007f),
    };

} // namespace Internal

LightManager::LightManager()
    : m_LightDebugShader(GetAssetPath("shaders/Light.frag"), GetAssetPath("shaders/Light.vert")),
      m_Buffer(ShapeVertexData::GenerateSphere(10, 10))
{
}

void LightManager::SetGlobalAmbientLight(glm::vec3 color)
{
    m_GlobalAmbientLight = color;
}

void LightManager::PushLight(LightData&& light)
{
    m_LightData.push_back(std::move(light));
}

LightData* LightManager::GetLight(unsigned index)
{
    if (index < m_LightData.size())
        return &m_LightData[index];
    return nullptr;
}

void LightManager::EditLightPropertiesMenu()
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
            if (ImGui::CollapsingHeader(type))
            {
                if (ImGui::Button("Remove"))
                    m_LightData.erase(m_LightData.begin() + i);
                else
                {
                    EditLightProperties(light);
                    EditLightColor(light);
                    EditLightAttenuationProperties(light);
                }
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

void LightManager::PushLightInfoToShader(Shader& shader, glm::vec3 camera_position)
{
    shader.Uniform("u_LightCount", static_cast<int>(m_LightData.size()));
    shader.Uniform("u_CameraPosition", camera_position);
    shader.Uniform("u_GlobalAmbientLight", m_GlobalAmbientLight);

    constexpr size_t buffer_length = 50;
    char             name_buffer[buffer_length];

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

        shader.Uniform(get_location(i, "Type"), light.Type);
        shader.Uniform(get_location(i, "Position"), light.Position);
        shader.Uniform(get_location(i, "Direction"), light.Direction);
        shader.Uniform(get_location(i, "Intensity"), light.Intensity);

        shader.Uniform(get_location(i, "Constant"), light.Constant);
        shader.Uniform(get_location(i, "Linear"), light.Linear);
        shader.Uniform(get_location(i, "Quadratic"), light.Quadratic);

        shader.Uniform(get_location(i, "SpotCutOff"), std::cos(glm::radians(light.SpotCutOff)));
        shader.Uniform(get_location(i, "SpotOuterCutOff"),
                       std::cos(glm::radians(light.SpotCutOff + light.SpotOuterCutOff)));

        shader.Uniform(get_location(i, "Color"), light.Color);
        shader.Uniform(get_location(i, "Ambient"),
                       light.Ambient == glm::vec3(0.0f)
                           ? light.Color * LightData::DefaultAmbientMultiplier
                           : light.Ambient);
        shader.Uniform(get_location(i, "Specular"), light.Specular);
    }
}

void LightManager::DrawDebugInfo(const glm::mat4& projection, const glm::mat4& view)
{
    if (!m_RenderDebugInfo)
        return;

    for (const LightData& light : m_LightData)
    {
        if (light.Type == LightData::Directional || !light.ShowDebugVisual)
            continue;

        Transform transform{
            .Position = light.Position,
            .Scale    = glm::vec3(0.1f),
            .Rotation = glm::vec3(0.0f),
        };

        m_LightDebugShader.Uniform("u_Color", light.Color);
        m_Buffer.Draw(m_LightDebugShader, projection, view, transform.CreateModelMatrix());
    }
}

void LightManager::EditLightProperties(LightData& light)
{
    ImGui::Combo(
        "Type", &light.Type, Internal::LightTypeNames.data(), Internal::LightTypeNames.size());

    ImGui::SeparatorText("Properties");
    ImGui::Checkbox("Show debug visual", &light.ShowDebugVisual);
    ImGui::DragFloat("Intensity", &light.Intensity, 0.1f);

    switch (light.Type)
    {
    case LightData::Point:
        ImGui::DragFloat3("Position", glm::value_ptr(light.Position), 0.1f);
        break;
    case LightData::Spot:
        ImGui::DragFloat3("Position", glm::value_ptr(light.Position), 0.1f);
        ImGui::SliderFloat("Cut Off", &light.SpotCutOff, 0.0f, 90.0f);
        ImGui::SliderFloat("Outer Cut Off", &light.SpotOuterCutOff, 0.0f, 90.0f);
        ImGui::DragFloat3("Direction", glm::value_ptr(light.Direction), 0.1f, -1.0f, 1.0f);
        break;
    case LightData::Directional:
        ImGui::DragFloat3("Direction", glm::value_ptr(light.Direction), 0.1f, -1.0f, 1.0f);
        break;
    }

    light.Intensity = std::max(light.Intensity, 0.0f);
}

void LightManager::EditLightAttenuationProperties(LightData& light)
{
    if (ImGui::CollapsingHeader("Attenuation"))
    {
        if (ImGui::BeginCombo("Select default distance", "select a distance..."))
        {
            for (const auto& [distance, constant, linear, quadratic] : Internal::LightDistances)
            {
                if (ImGui::Selectable(distance))
                {
                    light.Constant  = constant;
                    light.Linear    = linear;
                    light.Quadratic = quadratic;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SliderFloat("Constant", &light.Constant, 0.0f, 1.0f);
        ImGui::SliderFloat("Linear", &light.Linear, 0.0f, 1.0f, "%0.4f");
        ImGui::SliderFloat("Quadratic", &light.Quadratic, 0.0f, 0.1f, "%0.6f");

        static float preview_distance = 50.0f;
        ImGui::SliderFloat("Preview Distance", &preview_distance, 10.0f, 200.0f);

        // Generate curve
        const unsigned point_resolution = 100;
        float          attenuation_data[point_resolution];
        for (unsigned i = 0; i < point_resolution; i++)
        {
            float x = (float)i / (float)(point_resolution - 1) * preview_distance;

            float denominator   = (light.Quadratic * x * x) + (light.Linear * x) + light.Constant;
            attenuation_data[i] = denominator != 0.0f ? (1.0f / denominator) : 1.0f;
        }

        ImGui::PlotLines("AttenuationCurve",
                         attenuation_data,
                         point_resolution,
                         0,
                         "Light Falloff",
                         0.0f,
                         1.0f,
                         ImVec2(0.0f, 150.0f));
    }
}

void LightManager::EditLightColor(LightData& light)
{
    ImGui::SeparatorText("Light Color");
    ImGui::ColorEdit3("Color", glm::value_ptr(light.Color));
    ImGui::ColorEdit3("Ambient", glm::value_ptr(light.Ambient));
    ImGui::ColorEdit3("Specular", glm::value_ptr(light.Specular));
}

} // namespace LrnGL
