#pragma once

#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

#include <string_view>

#include "texture.h"

namespace LrnGL {

class Shader
{
public:
    static Shader InvalidShader;

    Shader() = default;
    Shader(std::string_view fragment, std::string_view vertex);
    ~Shader();

    void Destroy();

    void Uniform(std::string_view name, float val);
    void Uniform(std::string_view name, bool val);
    void Uniform(std::string_view name, int val);
    void Uniform(std::string_view name, unsigned val);

    void Uniform(std::string_view name, glm::vec4 val);
    void Uniform(std::string_view name, glm::vec3 val);
    void Uniform(std::string_view name, glm::vec2 val);
    void Uniform(std::string_view name, const glm::mat4& val);
    void Uniform(std::string_view name, const glm::mat3& val);
    void Uniform(const Texture& texture, unsigned id);

    void Bind();

    unsigned GetID() const { return m_ID; }

private:
    int GetUniformLocation(std::string_view name);

    unsigned m_ID = 0;
};

} // namespace LrnGL
