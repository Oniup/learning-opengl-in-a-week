#include <SDL3/SDL_events.h>
#include <fmt/format.h>
#include <glad/gl.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

#include <cstddef>
#include <filesystem>

#include "Error.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

namespace fs = std::filesystem;

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec2 UV;
};

Vertex vertices[] = {
    // top right
    Vertex{glm::vec3(0.5f,  0.5f,  0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
    // bottom right
    Vertex{glm::vec3(0.5f,  -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
    // bottom left
    Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
    // top left
    Vertex{glm::vec3(-0.5f, 0.5f,  0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}
};

// clang-format off
unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};
// clang-format on

std::string GetAssetDirectory(std::string_view target = "assets")
{
    fs::path working_dir = fs::current_path();
    while (working_dir.has_parent_path())
    {
        auto enumerate_directories = fs::directory_iterator(working_dir);

        for (const fs::directory_entry& dir : enumerate_directories)
            if (dir.is_directory() && dir.path().filename() == target)
                return dir.path().string();

        working_dir = working_dir.parent_path();
    }

    ASSERT(false, "Failed to find working directory");
}

void SetVertexAttribPointer(unsigned index, unsigned size, float stride, const void* offset)
{
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(index);
}

int main(int argc, char** argv)
{
    std::string asset_dir = GetAssetDirectory();
    lgl::Window window(800, 800);

    auto shader = lgl::Shader::Load(fmt::format("{}/shaders/Fragment.glsl", asset_dir),
                                    fmt::format("{}/shaders/Vertex.glsl", asset_dir));

    glUseProgram(shader.ID);
    glUniform1i(glGetUniformLocation(shader.ID, "Texture0"), 0);
    glUniform1i(glGetUniformLocation(shader.ID, "Texture1"), 1);

    auto bg_texture = lgl::Texture::Load(fmt::format("{}/textures/wall.jpg", asset_dir));
    auto fg_texture = lgl::Texture::Load(fmt::format("{}/textures/awesomeface.png", asset_dir));

    unsigned vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    unsigned vertex_buffer;
    unsigned element_buffer;
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &element_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    SetVertexAttribPointer(0, 3, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    SetVertexAttribPointer(1, 3, sizeof(Vertex), (void*)offsetof(Vertex, Color));
    SetVertexAttribPointer(2, 2, sizeof(Vertex), (void*)offsetof(Vertex, UV));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    size_t indices_size = sizeof(indices) / sizeof(unsigned);

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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bg_texture.ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fg_texture.ID);

        glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, (void*)0);

        window.SwapBuffers();
    }

    shader.Shutdown();
    return 0;
}
