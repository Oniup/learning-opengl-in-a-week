#include "Error.h"
#include "Shader.h"
#include "Window.h"

#include "SDL3/SDL_events.h"
#include "glad/gl.h"
#include "glm/glm.hpp"
#include <filesystem>
#include <fmt/format.h>

namespace fs = std::filesystem;

glm::vec3 vertices[] = {
    glm::vec3(-0.5f, -0.5f, 0.0f),
    glm::vec3(0.5f, -0.5f, 0.0f),
    glm::vec3(0.0f, 0.5f, 0.0f),
};

fs::path GetStoredAssetDirectory(std::string_view target)
{
    fs::path workingDirectory = fs::current_path();
    while (workingDirectory.has_parent_path())
    {
        auto enumerateDirectories = fs::directory_iterator(workingDirectory);

        for (const fs::directory_entry& dir : enumerateDirectories)
            if (dir.is_directory() && dir.path().filename() == target)
                return dir.path();

        workingDirectory = workingDirectory.parent_path();
    }

    ASSERT(false, "Failed to find working directory");
}

int main(int argc, char** argv)
{
    fs::path shaderDir = GetStoredAssetDirectory("shaders");
    LrnGL::Window window(800, 800);

    auto shader =
        LrnGL::Shader::Load(shaderDir / "basic-fragment.glsl", shaderDir / "basic-vertex.glsl");

    unsigned vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    unsigned vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    SDL_Event event;
    while (window.IsRunning())
    {
        while (SDL_PollEvent(&event))
        {
            window.HandleEvents(event);
        }

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vertexArray);
        glUseProgram(shader.ID);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        window.SwapBuffers();
    }

    shader.Shutdown();
    return 0;
}
