#include "material.h"

#include <fmt/format.h>
#include <glad/gl.h>

#include "utilities.h"

namespace LrnGL {

namespace Internal {

    Texture WhiteTexture;

} // namespace Internal

void LoadMaterialDefaults()
{
    Internal::WhiteTexture = Texture(GetAssetPath("textures/default.png"));
}

void UnloadMaterialDefaults()
{
    Internal::WhiteTexture.Destroy();
}

const Texture& GetMaterialDefaultWhiteTexture()
{
    return Internal::WhiteTexture;
}

void InitializeMaterialTextureUniforms(Shader& shader)
{
    shader.Uniform("u_Material.Diffuse.Image", 0);
    shader.Uniform("u_Material.Specular.Image", 1);
    shader.Uniform("u_Material.Emission.Image", 2);
}

MaterialColorInput::MaterialColorInput(glm::vec3 color)
    : Color(color)
{
}

MaterialColorInput::MaterialColorInput(Texture&& image)
    : Color(glm::vec3(1.0f)),
      Image(std::move(image))
{
}

MaterialColorInput::MaterialColorInput(const Texture& image)
    : Color(glm::vec3(1.0f)),
      Image(image)
{
}

MaterialColorInput::MaterialColorInput(glm::vec3 color, Texture&& image)
    : Color(color),
      Image(std::move(image))
{
}

void Material::PushInfoToShader() const
{
    // Color
    Shader->Uniform("u_Material.Diffuse.Color", Diffuse.Color);
    Shader->Uniform(Diffuse.Image, 0);
    Shader->Uniform("u_Material.Specular.Color", Specular.Color);
    Shader->Uniform(Specular.Image, 1);
    Shader->Uniform("u_Material.Emission.Color", Emission.Color);
    Shader->Uniform(Emission.Image, 2);

    // Other
    Shader->Uniform("u_Material.Shininess", Shininess);
    Shader->Uniform("u_Material_TilingFactor", TilingFactor);
}

} // namespace LrnGL
