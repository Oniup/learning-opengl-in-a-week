#include "Shader.h"
#include "Error.h"

#include "glad/gl.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <string>

namespace LrnGL {

static constexpr size_t InfoLogSize = 1024;

Shader Shader::Load(const std::filesystem::path& fragment, const std::filesystem::path& vertex)
{
    constexpr size_t shader_count                                       = 2;
    std::array<const std::filesystem::path*, shader_count> source_paths = {&fragment, &vertex};
    std::array<unsigned, 2> shaderIDs                                   = {};

    for (size_t i = 0; i < shader_count; i++)
    {
        const std::filesystem::path& path = *source_paths[i];

        std::string source;
        size_t source_size = std::filesystem::file_size(path);
        source.resize(source_size);

        std::ifstream stream(path);
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
            char info_log[InfoLogSize];
            glGetShaderInfoLog(shader, InfoLogSize, nullptr, info_log);
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
        char info_log[InfoLogSize];
        glGetProgramInfoLog(program, InfoLogSize, nullptr, info_log);
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

} // namespace LrnGL
