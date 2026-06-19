#include "Shader.h"

#include <glad/gl.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

#include "Error.h"

namespace lgl {

static constexpr size_t info_log_size = 1024;
Shader Shader::InvalidShader          = {.ID = std::numeric_limits<unsigned>::max()};

Shader Shader::Load(std::string_view fragment, std::string_view vertex)
{
    constexpr size_t shader_count                           = 2;
    std::array<std::string_view, shader_count> source_paths = {fragment, vertex};
    std::array<unsigned, 2> shaderIDs                       = {};

    for (size_t i = 0; i < shader_count; i++)
    {
        std::string_view path = source_paths[i];
        char path_buffer[info_log_size];
        auto result = fmt::format_to_n(path_buffer, info_log_size - 1, "{}", path);
        *result.out = '\0';

        std::string source;
        size_t source_size = std::filesystem::file_size(path);
        source.resize(source_size);

        std::ifstream stream(path_buffer);
        if (!stream.is_open())
        {
            ERROR("Failed to create shader, could not find {} path", path);
            return InvalidShader;
        }

        stream.read(source.data(), source_size);

        unsigned shader     = glCreateShader(GL_FRAGMENT_SHADER + i);
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
            return InvalidShader;
        }

        shaderIDs[i] = shader;
    }

    unsigned program = glCreateProgram();
    for (unsigned shader : shaderIDs)
        glAttachShader(program, shader);
    glLinkProgram(program);

    for (unsigned shader : shaderIDs)
        glDeleteShader(shader);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[info_log_size];
        glGetProgramInfoLog(program, info_log_size, nullptr, info_log);
        ERROR("Failed to link shader for paths vertex {} and fragment {}: {}",
              vertex,
              fragment,
              info_log);

        return InvalidShader;
    }

    return Shader{program};
}

void Shader::Shutdown()
{
    if (ID != 0)
    {
        glDeleteProgram(ID);
        ID = 0;
    }
}

} // namespace lgl
