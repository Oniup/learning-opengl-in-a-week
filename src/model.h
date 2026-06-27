#pragma once

#include <string>

#include "material.h"
#include "texture.h"
#include "transform.h"
#include "vertex_buffer.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;

namespace LrnGL {

class Model;

class Mesh
{
public:
    Mesh(ShapeVertexData&& shape, Material&& material);
    Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned>&& indices, Material&& material);

    Mesh(Mesh&& other);
    Mesh& operator=(Mesh&& other);

    Mesh(const Mesh& other)            = delete;
    Mesh& operator=(const Mesh& other) = delete;

    Material&                   GetMaterial() { return m_Material; }
    const Material&             GetMaterial() const { return m_Material; }
    const std::vector<Vertex>   GetVertices() const { return m_Vertices; }
    const std::vector<unsigned> GetIndices() const { return m_Indices; }
    const VertexBuffer&         GetVertexBuffer() const { return m_VertexBuffer; }

    void Draw(const glm::mat4& projection, const glm::mat4& view, const Transform& transform) const;

private:
    Material              m_Material;
    std::vector<Vertex>   m_Vertices;
    std::vector<unsigned> m_Indices;
    VertexBuffer          m_VertexBuffer;
};

class Model
{
    struct TextureCache
    {
        std::string Path;
        Texture     Texture;
    };

public:
    Model() = default;
    Model(std::string_view path, Shader* shader, bool flip_uvs = true);
    Model(std::vector<Mesh>&& meshes);
    Model(Mesh&& mesh);

    Model(Model&& other);
    Model& operator=(Model&& other);

    Model(const Model& other)            = delete;
    Model& operator=(const Model& other) = delete;

    const std::vector<Mesh>& GetMeshes() const { return m_Meshes; }

    void Draw(float elapsed_time, const glm::mat4& projection, const glm::mat4& view,
              const Transform& transform) const;

private:
    void LoadModelFromPath(std::string_view path, Shader* shader, bool flip_uvs);
    void ProcessNode(const aiNode* node, const aiScene* scene, Shader* shader,
                     std::string_view directory);
    Mesh ProcessMesh(const aiMesh* mesh, const aiScene* scene, Shader* shader,
                     std::string_view directory);

    MaterialColorInput LoadMaterialColorInput(aiMaterial* material, const aiScene* scene, int type,
                                              const char* color_key, unsigned type_key,
                                              unsigned index_key, std::string_view directory);

    std::vector<TextureCache> m_TextureCache;
    std::vector<Mesh>         m_Meshes;
};

} // namespace LrnGL
