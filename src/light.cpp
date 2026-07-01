#include "light.h"

#include <fmt/base.h>
#include <fmt/format.h>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <algorithm>
#include <tuple>

#include "error.h"
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

LightManager::LightManager(bool initialize_uniform_buffer)
    : m_LightDebugShader(GetAssetPath("shaders/Light.frag"), GetAssetPath("shaders/Light.vert")),
      m_Buffer(ShapeVertexData::GenerateSphere(10, 10))
{
    if (initialize_uniform_buffer)
    {
        glGenBuffers(1, &m_UniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBuffer);
        glBufferData(
            GL_UNIFORM_BUFFER, sizeof(LightData) * MaxLightCount, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

LightManager::~LightManager()
{
    if (m_UniformBuffer != 0)
    {
        glDeleteBuffers(1, &m_UniformBuffer);
        m_UniformBuffer = 0;
    }
}

void LightManager::SetGlobalAmbientLight(glm::vec3 color)
{
    m_GlobalAmbientLight = color;
}

void LightManager::ShouldRenderDebugInfo(bool render)
{
    m_RenderDebugInfo = render;
}

void LightManager::PushLight(LightData&& light, bool show_debug_visualization)
{
    m_LightData.push_back(std::move(light));
    m_ShowLightDebugVisual.push_back(show_debug_visualization);
    m_UpdateLightUniformBuffer = true;
}

void LightManager::RemoveLight(unsigned index)
{
    ASSERT(index < m_LightData.size(), "index out of range");
    m_LightData.erase(m_LightData.begin() + index);
    m_ShowLightDebugVisual.erase(m_ShowLightDebugVisual.begin() + index);
    m_UpdateLightUniformBuffer = true;
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
    ImGui::Checkbox("Render Light debug info", &m_RenderDebugInfo);
    ImGui::ColorEdit3("Global ambient light", glm::value_ptr(m_GlobalAmbientLight));

    unsigned changes = 0;
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
                    RemoveLight(i);
                else
                {
                    changes += EditLightProperties(light, i);
                    changes += EditLightColor(light);
                    changes += EditLightAttenuationProperties(light);
                }
            }

            ImGui::PopID();
        }

        if (m_LightData.size() < MaxLightCount)
        {
            ImGui::NewLine();

            if (ImGui::Button("Add Light"))
                PushLight(LightData{});
        }
    }

    if (!m_UpdateLightUniformBuffer)
        m_UpdateLightUniformBuffer = changes > 0;
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

void LightManager::PushLightInfoToUniformBlockLayout(Shader& shader, glm::vec3 camera_position)
{
    if (m_UpdateLightUniformBuffer)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBuffer);
        glBufferSubData(
            GL_UNIFORM_BUFFER, 0, sizeof(LightData) * m_LightData.size(), m_LightData.data());
        m_UpdateLightUniformBuffer = false;
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_UniformBuffer);
    shader.Uniform("u_LightCount", static_cast<int>(m_LightData.size()));
    shader.Uniform("u_CameraPosition", camera_position);
    shader.Uniform("u_GlobalAmbientLight", m_GlobalAmbientLight);
}

void LightManager::DrawDebugInfo(const glm::mat4& projection, const glm::mat4& view)
{
    if (!m_RenderDebugInfo)
        return;

    for (unsigned i = 0; i < m_LightData.size(); i++)
    {
        const LightData& light = m_LightData[i];
        if (light.Type == LightData::Directional || !m_ShowLightDebugVisual[i])
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

unsigned LightManager::EditLightProperties(LightData& light, unsigned index)
{
    unsigned c = 0;

    c += ImGui::Combo(
        "Type", &light.Type, Internal::LightTypeNames.data(), Internal::LightTypeNames.size());

    ImGui::SeparatorText("Properties");
    bool show                      = m_ShowLightDebugVisual[index];
    c                             += ImGui::Checkbox("Show debug visual", &show);
    m_ShowLightDebugVisual[index]  = show;

    c += ImGui::DragFloat("Intensity", &light.Intensity, 0.1f);

    switch (light.Type)
    {
    case LightData::Point:
        c += ImGui::DragFloat3("Position", glm::value_ptr(light.Position), 0.1f);
        break;
    case LightData::Spot:
        c += ImGui::DragFloat3("Position", glm::value_ptr(light.Position), 0.1f);
        c += ImGui::SliderFloat("Cut Off", &light.SpotCutOff, 0.0f, 90.0f);
        c += ImGui::SliderFloat("Outer Cut Off", &light.SpotOuterCutOff, 0.0f, 90.0f);
        c += ImGui::DragFloat3("Direction", glm::value_ptr(light.Direction), 0.1f, -1.0f, 1.0f);
        break;
    case LightData::Directional:
        c += ImGui::DragFloat3("Direction", glm::value_ptr(light.Direction), 0.1f, -1.0f, 1.0f);
        break;
    }

    light.Intensity = std::max(light.Intensity, 0.0f);
    return c;
}

unsigned LightManager::EditLightAttenuationProperties(LightData& light)
{
    unsigned c = 0;
    if (ImGui::CollapsingHeader("Attenuation"))
    {
        if (ImGui::BeginCombo("Select default distance", "select a distance..."))
        {
            for (const auto& [distance, constant, linear, quadratic] : Internal::LightDistances)
            {
                if (ImGui::Selectable(distance))
                {
                    c               += 1;
                    light.Constant   = constant;
                    light.Linear     = linear;
                    light.Quadratic  = quadratic;
                }
            }
            ImGui::EndCombo();
        }

        c += ImGui::SliderFloat("Constant", &light.Constant, 0.0f, 1.0f);
        c += ImGui::SliderFloat("Linear", &light.Linear, 0.0f, 1.0f, "%0.4f");
        c += ImGui::SliderFloat("Quadratic", &light.Quadratic, 0.0f, 0.1f, "%0.6f");

        static float preview_distance = 50.0f;
        c += ImGui::SliderFloat("Preview Distance", &preview_distance, 10.0f, 200.0f);

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
    return c;
}

unsigned LightManager::EditLightColor(LightData& light)
{
    unsigned c = 0;
    ImGui::SeparatorText("Light Color");
    c += ImGui::ColorEdit3("Color", glm::value_ptr(light.Color));
    c += ImGui::ColorEdit3("Ambient", glm::value_ptr(light.Ambient));
    c += ImGui::ColorEdit3("Specular", glm::value_ptr(light.Specular));
    return c;
}

} // namespace LrnGL
