#pragma once

#include <glm/ext/vector_float3.hpp>

#include "shader.h"
#include "texture.h"

namespace LrnGL {

void           LoadMaterialDefaults();
void           UnloadMaterialDefaults();
const Texture& GetMaterialDefaultWhiteTexture();

void InitializeMaterialTextureUniforms(Shader& shader);

struct MaterialColorInput
{
    glm::vec3 Color = glm::vec3(1.0f);
    Texture   Image = GetMaterialDefaultWhiteTexture();

    MaterialColorInput() = default;
    MaterialColorInput(glm::vec3 color);
    MaterialColorInput(Texture&& image);
    MaterialColorInput(const Texture& image);
    MaterialColorInput(glm::vec3 color, Texture&& image);
};

struct Material
{
    Shader*            Shader       = nullptr;
    MaterialColorInput Diffuse      = glm::vec3(1.0f);
    MaterialColorInput Specular     = glm::vec3(0.0f);
    MaterialColorInput Emission     = glm::vec3(0.0f);
    glm::vec2          TilingFactor = glm::vec2(1.0f);

    int   Shininess        = 32;
    float ReflectionFactor = 0.3f;

    void PushInfoToShader() const;
};

} // namespace LrnGL
