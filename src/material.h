#pragma once

#include <glm/ext/vector_float3.hpp>

#include <string_view>

#include "shader.h"
#include "texture.h"

namespace LrnGL {

struct Material
{
    static Texture DefaultTexture;

    static void LoadDefaultTexture(std::string_view asset_dir);
    static void UnloadDefaultTexture();

    static void InitializeMaterialTextureUniforms(Shader& shader);

    glm::vec3 Tint         = glm::vec3(1.0f);
    glm::vec2 TilingFactor = glm::vec2(1.0f);
    Texture   Diffuse      = DefaultTexture;
    Texture   Specular     = DefaultTexture;
    int       Shininess    = 32;

    void PushInfoToShader(Shader& shader);
};

} // namespace LrnGL
