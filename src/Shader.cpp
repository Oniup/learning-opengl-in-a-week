#include "Shader.h"
#include "Error.h"

#include "glad/gl.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <string>

namespace LrnGL {

static constexpr size_t InfoLogBufferSize = 1024;

Shader Shader::Load(const std::filesystem::path& fragment, const std::filesystem::path& vertex)
{
    constexpr size_t shaderCount                                      = 2;
    std::array<const std::filesystem::path*, shaderCount> sourcePaths = {&fragment, &vertex};
    std::array<unsigned, 2> shaderIDs                                 = {};

    for (size_t i = 0; i < shaderCount; i++)
    {
        const std::filesystem::path& path = *sourcePaths[i];

        std::string source;
        size_t sourceSize = std::filesystem::file_size(path);
        source.resize(sourceSize);

        std::ifstream stream(path);
        if (!stream.is_open())
        {
            ERROR("Failed to create shader, could not find {} path", path);
            return InvalidShader;
        }

        stream.read(source.data(), sourceSize);

        unsigned shader     = glCreateShader(GL_FRAGMENT_SHADER + i);
        const char* cSource = source.c_str();
        glShaderSource(shader, 1, &cSource, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[InfoLogBufferSize];
            glGetShaderInfoLog(shader, InfoLogBufferSize, nullptr, infoLog);
            ERROR("Failed to compile shader for path {}: '{}'", path, infoLog);

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
        char infoLog[InfoLogBufferSize];
        glGetProgramInfoLog(program, InfoLogBufferSize, nullptr, infoLog);
        ERROR("Failed to link shader for paths vertex {} and fragment {}: {}",
              vertex,
              fragment,
              infoLog);

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
