#include "Shader.h"

#include <fmt/format.h>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <string>

#include "error.h"

namespace LrnGL {

static constexpr size_t info_log_size    = 1024;
static constexpr size_t name_buffer_size = 100;

Shader Shader::invalid_shader;

Shader::Shader(std::string_view fragment, std::string_view vertex)
{
    constexpr size_t                           shader_count = 2;
    std::array<std::string_view, shader_count> source_paths = {fragment, vertex};
    std::array<unsigned, 2>                    shaderIDs    = {};

    for (size_t i = 0; i < shader_count; i++)
    {
        std::string_view path = source_paths[i];
        char             path_buffer[info_log_size];
        auto             result = fmt::format_to_n(path_buffer, info_log_size - 1, "{}", path);
        *result.out             = '\0';

        std::string source;
        size_t      source_size = std::filesystem::file_size(path);
        source.resize(source_size);

        std::ifstream stream(path_buffer);
        if (!stream.is_open())
        {
            ERROR("Failed to create shader, could not find {} path", path);
            return;
        }

        stream.read(source.data(), source_size);

        unsigned    shader  = glCreateShader(GL_FRAGMENT_SHADER + i);
        const char* cSource = source.c_str();
        glShaderSource(shader, 1, &cSource, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char info_log[info_log_size];
            glGetShaderInfoLog(shader, info_log_size, nullptr, info_log);
            ERROR("Failed to compile shader for path {}: '{}'", path, info_log);

            if (i > 0)
                glDeleteShader(shaderIDs[i]);
            m_ID = 0;
            return;
        }

        shaderIDs[i] = shader;
    }

    m_ID = glCreateProgram();
    for (unsigned shader : shaderIDs)
        glAttachShader(m_ID, shader);
    glLinkProgram(m_ID);

    for (unsigned shader : shaderIDs)
        glDeleteShader(shader);

    int success;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[info_log_size];
        glGetProgramInfoLog(m_ID, info_log_size, nullptr, info_log);
        ERROR("Failed to link shader for paths vertex {} and fragment {}: {}",
              vertex,
              fragment,
              info_log);

        m_ID = 0;
        return;
    }
}

Shader::~Shader()
{
    Destroy();
}

void Shader::InitializeTextureIDs(unsigned number_of_ids)
{
    glUseProgram(m_ID);

    for (unsigned i = 0; i < number_of_ids; i++)
    {
        char name_buffer[name_buffer_size];
        auto result = fmt::format_to_n(name_buffer, name_buffer_size, "u_Texture{}", i);
        *result.out = '\0';
        fmt::print("{}\n", name_buffer);
        glUniform1i(GetUniformLocation(name_buffer), i);
    }
}

void Shader::Destroy()
{
    if (m_ID != 0)
    {
        glDeleteProgram(m_ID);
        m_ID = 0;
    }
}

void Shader::Uniform(std::string_view name, float val)
{
    glUseProgram(m_ID);
    glUniform1f(GetUniformLocation(name), val);
}

void Shader::Uniform(std::string_view name, bool val)
{
    glUseProgram(m_ID);
    glUniform1i(GetUniformLocation(name), val);
}

void Shader::Uniform(std::string_view name, int val)
{
    glUseProgram(m_ID);
    glUniform1i(GetUniformLocation(name), val);
}

void Shader::Uniform(std::string_view name, unsigned val)
{
    glUseProgram(m_ID);
    glUniform1i(GetUniformLocation(name), val);
}

void Shader::Uniform(std::string_view name, glm::vec4 val)
{
    glUseProgram(m_ID);
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(val));
}

void Shader::Uniform(std::string_view name, glm::vec3 val)
{
    glUseProgram(m_ID);
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(val));
}

void Shader::Uniform(std::string_view name, glm::vec2 val)
{
    glUseProgram(m_ID);
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(val));
}

void Shader::Uniform(std::string_view name, const glm::mat4& val)
{
    glUseProgram(m_ID);
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::Uniform(std::string_view name, const glm::mat3& val)
{
    glUseProgram(m_ID);
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::Bind()
{
    glUseProgram(m_ID);
}

void Shader::BindTexture(const Texture& texture, unsigned id)
{
    glUseProgram(m_ID);
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, texture.GetID());
}

int Shader::GetUniformLocation(std::string_view name)
{
    ASSERT(name.data()[name.size()] == '\0', "name parameter '{}' must be null terminated", name);
    return glGetUniformLocation(m_ID, name.data());
}

} // namespace LrnGL
