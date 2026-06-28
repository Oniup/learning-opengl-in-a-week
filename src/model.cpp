#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/texture.h>
#include <assimp/types.h>
#include <fmt/format.h>

#include "error.h"
#include "material.h"
#include "vertex_buffer.h"

namespace LrnGL {

Mesh::Mesh(ShapeVertexData&& shape, Material&& material)
    : m_Material(std::move(material)),
      m_Vertices(std::move(shape.Vertices)),
      m_Indices(std::move(shape.Indices)),
      m_VertexBuffer(m_Vertices, m_Indices)
{
}

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned>&& indices, Material&& material)
    : m_Material(std::move(material)),
      m_Vertices(std::move(vertices)),
      m_Indices(std::move(indices)),
      m_VertexBuffer(m_Vertices, m_Indices)
{
}

Mesh::Mesh(Mesh&& other)
    : m_Material(std::move(other.m_Material)),
      m_Vertices(std::move(other.m_Vertices)),
      m_Indices(std::move(other.m_Indices)),
      m_VertexBuffer(std::move(other.m_VertexBuffer))
{
}

Mesh& Mesh::operator=(Mesh&& other)
{
    m_Material     = std::move(other.m_Material);
    m_Vertices     = std::move(other.m_Vertices);
    m_Indices      = std::move(other.m_Indices);
    m_VertexBuffer = std::move(other.m_VertexBuffer);
    return *this;
}

void Mesh::Draw(const glm::mat4& projection, const glm::mat4& view,
                const Transform& transform) const
{
    m_Material.PushInfoToShader();
    m_VertexBuffer.Draw(*m_Material.Shader, projection, view, transform.CreateModelMatrix());
}

Model::Model(std::string_view path, Shader* shader, unsigned flags, TextureFilter filter)
{
    LoadModelFromPath(path, shader, flags, filter);
}

Model::Model(std::vector<Mesh>&& meshes)
    : m_Meshes(std::move(meshes))
{
}

Model::Model(Mesh&& mesh)
{
    m_Meshes.push_back(std::move(mesh));
}

Model::Model(Model&& other)
    : m_Meshes(std::move(other.m_Meshes))
{
}

Model& Model::operator=(Model&& other)
{
    m_Meshes = std::move(other.m_Meshes);
    return *this;
}

void Model::SetShaderToAllMaterials(Shader* shader)
{
    for (Mesh& mesh : m_Meshes)
        mesh.GetMaterial().Shader = shader;
}

void Model::Draw(float elapsed_time, const glm::mat4& projection, const glm::mat4& view,
                 const Transform& transform) const
{
    for (const Mesh& mesh : m_Meshes)
    {
        mesh.GetMaterial().Shader->Uniform("u_Time", elapsed_time);
        mesh.Draw(projection, view, transform);
    }
}

void Model::LoadModelFromPath(std::string_view path, Shader* shader, unsigned flags,
                              TextureFilter filter)
{
    ASSERT_STRING_VIEW_NULL_TERMINATED(path);

    unsigned post_processing = aiProcess_Triangulate | aiProcess_SortByPType |
                               aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace;

    post_processing |= (flags & ModelLoading_GenSmoothNormals) == 0 ? aiProcess_GenSmoothNormals
                                                                    : aiProcess_GenNormals;
    post_processing |= (flags & ModelLoading_FlipUVs) == 0 ? aiProcess_FlipUVs : 0;
    post_processing |= (flags & ModelLoading_DisableTransformingVertices) == 0
                           ? 0
                           : aiProcess_PreTransformVertices;

    Assimp::Importer importer;
    const aiScene*   scene = importer.ReadFile(path.data(), post_processing);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        ERROR("Failed to load model from path {}: {}", path, importer.GetErrorString());
        return;
    }

    // Make sure directory ends with null terminator
    constexpr size_t dir_buffer_length = 1024;
    char             dir_buffer[dir_buffer_length];
    auto             result = fmt::format_to_n(
        dir_buffer, dir_buffer_length, "{}", path.substr(0, path.find_last_of('/')));
    *result.out = '\0';
    std::string_view directory(dir_buffer, result.size);

    // Load all children modes meshes
    m_Meshes.reserve(scene->mNumMeshes);
    m_TextureCache.reserve(20);
    ProcessNode(scene->mRootNode, scene, shader, flags, filter, directory);
}

void Model::ProcessNode(const aiNode* node, const aiScene* scene, Shader* shader, unsigned flags,
                        TextureFilter filter, std::string_view directory)
{
    for (unsigned i = 0; i < node->mNumMeshes; i++)
    {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_Meshes.push_back(ProcessMesh(mesh, scene, shader, flags, filter, directory));
    }

    for (unsigned i = 0; i < node->mNumChildren; i++)
        ProcessNode(node->mChildren[i], scene, shader, flags, filter, directory);
}

Mesh Model::ProcessMesh(const aiMesh* mesh, const aiScene* scene, Shader* shader, unsigned flags,
                        TextureFilter filter, std::string_view directory)
{
    std::vector<Vertex>   vertices;
    std::vector<unsigned> indices;
    Material              material{.Shader = shader};

    vertices.reserve(mesh->mNumVertices);
    indices.reserve(mesh->mNumFaces * 3);

    for (unsigned i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        vertex.Position =
            glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasNormals())
            vertex.Normal =
                glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        else
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

        vertex.Color =
            mesh->mColors[0]
                ? glm::vec3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b)
                : glm::vec3(1.0f);

        vertex.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x,
                                                               mesh->mTextureCoords[0][i].y)
                                                   : glm::vec2(0.0f);
        vertices.push_back(vertex);
    }

    for (unsigned i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* mesh_material = scene->mMaterials[mesh->mMaterialIndex];

        // Texture maps
        material.Diffuse  = LoadMaterialColorInput(mesh_material,
                                                   scene,
                                                   aiTextureType_DIFFUSE,
                                                   AI_MATKEY_COLOR_DIFFUSE,
                                                   flags,
                                                   filter,
                                                   directory);
        material.Specular = LoadMaterialColorInput(mesh_material,
                                                   scene,
                                                   aiTextureType_SPECULAR,
                                                   AI_MATKEY_COLOR_SPECULAR,
                                                   flags,
                                                   filter,
                                                   directory);
        material.Emission = LoadMaterialColorInput(mesh_material,
                                                   scene,
                                                   aiTextureType_EMISSIVE,
                                                   AI_MATKEY_COLOR_EMISSIVE,
                                                   flags,
                                                   filter,
                                                   directory);
        // Shininess
        float shininess;
        if (mesh_material->Get(AI_MATKEY_SHININESS, shininess) == aiReturn_SUCCESS)
        {
            material.Shininess = shininess > 1.0f ? static_cast<int>(shininess) : 32;
        }

        // Tiling factor
        aiUVTransform uv_transform;
        if (mesh_material->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_DIFFUSE, 0), uv_transform))
        {
            material.TilingFactor = glm::vec2(uv_transform.mScaling.x, uv_transform.mScaling.y);
        }
    }

    return Mesh(std::move(vertices), std::move(indices), std::move(material));
}

MaterialColorInput Model::LoadMaterialColorInput(aiMaterial* material, const aiScene* scene,
                                                 int type, const char* color_key, unsigned type_key,
                                                 unsigned index_key, unsigned flags,
                                                 TextureFilter filter, std::string_view directory)
{
    if (material->GetTextureCount((aiTextureType)type) > 0)
    {
        aiString path;
        material->GetTexture((aiTextureType)type, 0, &path);

        for (const TextureCache& cache : m_TextureCache)
        {
            if (cache.Path == std::string_view(path.C_Str(), path.length))
                return cache.Texture;
        }

        const char* texture_loading_type;

        const aiTexture* embedded_texture = scene->GetEmbeddedTexture(path.C_Str());
        Texture          texture;
        if (embedded_texture)
        {
            texture_loading_type = "embedded";
            if (embedded_texture->mHeight == 0)
            {
                texture = Texture(reinterpret_cast<unsigned char*>(embedded_texture->pcData),
                                  embedded_texture->mWidth,
                                  flags & ModelLoading_ApplyGammaCorrectionSRGB,
                                  false,
                                  filter);
            }
            else
                FATAL("Uncompressed embedded textures are not supported");
        }
        else
        {
            texture_loading_type = "external";
            texture = Texture(fmt::format("{}/{:.{}}", directory, path.C_Str(), path.length),
                              flags & ModelLoading_ApplyGammaCorrectionSRGB,
                              false,
                              filter);
        }

        fmt::print("Loading {} texture at path {}\n", texture_loading_type, path.C_Str());

        m_TextureCache.push_back(TextureCache{
            .Path    = path.C_Str(),
            .Texture = std::move(texture),
        });

        return m_TextureCache.back().Texture;
    }

    aiColor3D color(1.0f, 1.0f, 1.0f);
    if (material->Get(color_key, type_key, index_key, color) == aiReturn_SUCCESS)
        return MaterialColorInput(glm::vec3(color.r, color.g, color.b));

    return MaterialColorInput(glm::vec3(type == aiTextureType_DIFFUSE ? 1.0f : 0.0f));
}

} // namespace LrnGL
