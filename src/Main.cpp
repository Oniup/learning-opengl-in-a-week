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
    glm::vec3(0.5f, 0.5f, 0.0f),   // top right
    glm::vec3(0.5f, -0.5f, 0.0f),  // bottom right
    glm::vec3(-0.5f, -0.5f, 0.0f), // bottom left
    glm::vec3(-0.5f, 0.5f, 0.0f),  // top left
};
size_t vertices_size = sizeof(vertices) / sizeof(glm::vec3);

// clang-format off
unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};
// clang-format on

fs::path GetStoredAssetDirectory(std::string_view target)
{
    fs::path working_dir = fs::current_path();
    while (working_dir.has_parent_path())
    {
        auto enumerate_directories = fs::directory_iterator(working_dir);

        for (const fs::directory_entry& dir : enumerate_directories)
            if (dir.is_directory() && dir.path().filename() == target)
                return dir.path();

        working_dir = working_dir.parent_path();
    }

    ASSERT(false, "Failed to find working directory");
}

int main(int argc, char** argv)
{
    fs::path shader_dir = GetStoredAssetDirectory("shaders");
    LrnGL::Window window(800, 800);

    auto shader =
        LrnGL::Shader::Load(shader_dir / "basic-fragment.glsl", shader_dir / "basic-vertex.glsl");

    unsigned vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    unsigned vertex_buffer;
    unsigned element_buffer;
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &element_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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

        glBindVertexArray(vertex_array);
        glUseProgram(shader.ID);
        // glDrawArrays(GL_TRIANGLES, 0, vertices_size);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned), GL_UNSIGNED_INT, (void*)0);

        window.SwapBuffers();
    }

    shader.Shutdown();
    return 0;
}
