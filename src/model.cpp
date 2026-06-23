#include "model.h"

#include <fmt/format.h>
#include <glad/gl.h>

#include "Error.h"

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
}

Model::Model(Shader* shader, Material* material, ShapeMesh& mesh)
    : m_Shader(shader),
      m_Material(material),
      m_VertexBuffer(mesh)
{
}

void Model::Draw(LightManager& light_manager, const Camera& camera)
{
    ASSERT(m_Material, "Must have Material assigned to model");
    ASSERT(m_Shader, "Must have shader assigned to model");

    PushMaterialInfoToShader();
    light_manager.PushLightInfoToShader(*m_Shader, camera.GetPosition());
}

void Model::PushMaterialInfoToShader()
{
    m_Shader->BindTexture(m_Material->Diffuse, 0);
    m_Shader->BindTexture(m_Material->Specular, 1);
}

} // namespace LrnGL
