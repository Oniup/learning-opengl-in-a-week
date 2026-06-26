#include "shader.h"

#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>
#include <fmt/format.h>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <array>

#include "error.h"

namespace LrnGL {

static constexpr size_t info_log_size    = 1024;
static constexpr size_t name_buffer_size = 100;

Shader Shader::InvalidShader;

Shader::Shader(std::string_view fragment, std::string_view vertex)
{
    ASSERT_STRING_VIEW_NULL_TERMINATED(fragment);
    ASSERT_STRING_VIEW_NULL_TERMINATED(vertex);

    constexpr unsigned                         shader_count = 2;
    std::array<std::string_view, shader_count> source_paths = {fragment, vertex};
    std::array<unsigned, 2>                    shader_ids   = {};

    for (unsigned i = 0; i < shader_count; i++)
    {
        std::string_view path = source_paths[i];

        size_t source_size;
        char*  source = static_cast<char*>(SDL_LoadFile(path.data(), &source_size));
        if (!source)
        {
            ERROR("Failed to create shader, could not find {} path", path);
            if (i > 0)
                glDeleteShader(shader_ids[0]);
            m_ID = 0;
            return;
        }

        unsigned shader = glCreateShader(GL_FRAGMENT_SHADER + i);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        SDL_free(source);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char info_log[info_log_size];
            glGetShaderInfoLog(shader, info_log_size, nullptr, info_log);
            ERROR("Failed to compile shader for path {}: '{}'", path, info_log);

            if (i > 0)
                glDeleteShader(shader_ids[0]);
            m_ID = 0;
            return;
        }

        shader_ids[i] = shader;
    }

    m_ID = glCreateProgram();
    for (unsigned shader : shader_ids)
        glAttachShader(m_ID, shader);
    glLinkProgram(m_ID);

    for (unsigned shader : shader_ids)
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

void Shader::Uniform(const Texture& texture, unsigned id)
{
    glUseProgram(m_ID);
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, texture.GetID());
}

int Shader::GetUniformLocation(std::string_view name)
{
    ASSERT_STRING_VIEW_NULL_TERMINATED(name);
    return glGetUniformLocation(m_ID, name.data());
}

} // namespace LrnGL
