#pragma once

#include <glm/ext/vector_float3.hpp>

#include <string_view>

#include "camera.h"
#include "light.h"
#include "shader.h"
#include "texture.h"
#include "vertex_buffer.h"

namespace LrnGL {

struct Material
{
    static Texture DefaultTexture;

    glm::vec3 Ambient   = glm::vec3(0.1f);
    glm::vec3 Tint      = glm::vec3(0.0f);
    Texture   Diffuse   = DefaultTexture;
    Texture   Specular  = DefaultTexture;
    int       Shininess = 32;

    static void LoadDefaultTexture(std::string_view asset_dir);
    static void UnloadDefaultTexture();

    static void InitializeMaterialTextureUniforms(Shader& shader);
};

class Model
{
public:
    Model(Shader* shader, Material* material, ShapeMesh& mesh);

    Shader*             GetShader() { return m_Shader; }
    Material*           GetMaterial() { return m_Material; }
    const Shader*       GetShader() const { return m_Shader; }
    const Material*     GetMaterial() const { return m_Material; }
    const VertexBuffer& GetVertexBuffer() const { return m_VertexBuffer; }

    void Draw(LightManager& light_manager, const Camera& camera);

private:
    void PushMaterialInfoToShader();

    Shader*      m_Shader   = nullptr;
    Material*    m_Material = nullptr;
    VertexBuffer m_VertexBuffer;
};

} // namespace LrnGL
