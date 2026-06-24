#include "material.h"

#include <fmt/format.h>
#include <glad/gl.h>

namespace LrnGL {

Texture Material::DefaultTexture;

void Material::LoadDefaultTexture(std::string_view asset_dir)
{
    DefaultTexture = Texture(fmt::format("{}/textures/default.png", asset_dir));
}

void Material::UnloadDefaultTexture()
{
    DefaultTexture.Destroy();
}

void Material::InitializeMaterialTextureUniforms(Shader& shader)
{
    shader.Uniform("u_Material.Diffuse", 0);
    shader.Uniform("u_Material.Specular", 1);
}

void Material::PushInfoToShader(Shader& shader)
{
    shader.Uniform(Diffuse, 0);
    shader.Uniform(Specular, 1);

    shader.Uniform("u_Material.Tint", Tint);
    shader.Uniform("u_Material.Shininess", Shininess);

    shader.Uniform("u_Material_TilingFactor", TilingFactor);
}

} // namespace LrnGL
