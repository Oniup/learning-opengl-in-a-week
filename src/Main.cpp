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
#include "VertexBuffer.h"
#include "Window.h"

namespace fs = std::filesystem;

lgl::Vertex vertices[] = {
    // top right
    lgl::Vertex{glm::vec3(0.5f,  0.5f,  0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
    // bottom right
    lgl::Vertex{glm::vec3(0.5f,  -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
    // bottom left
    lgl::Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
    // top left
    lgl::Vertex{glm::vec3(-0.5f, 0.5f,  0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}
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

int main(int argc, char** argv)
{
    std::string asset_dir = GetAssetDirectory();
    lgl::Window window(800, 800);

    lgl::Shader shader(fmt::format("{}/shaders/Fragment.glsl", asset_dir),
                       fmt::format("{}/shaders/Vertex.glsl", asset_dir));
    shader.InitializeTextureIDs(2);

    lgl::Texture bg_texture(fmt::format("{}/textures/wall.jpg", asset_dir));
    lgl::Texture fg_texture(fmt::format("{}/textures/awesomeface.png", asset_dir));

    lgl::VertexBuffer buffer(true);
    buffer.PushAttribute(0, 3, offsetof(lgl::Vertex, Position));
    buffer.PushAttribute(1, 3, offsetof(lgl::Vertex, Color));
    buffer.PushAttribute(2, 2, offsetof(lgl::Vertex, UV));
    buffer.PushData(lgl::VertexBuffer_Vertex, sizeof(vertices), vertices);
    buffer.PushData(lgl::VertexBuffer_Element, sizeof(indices), indices);

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

        shader.BindTexture(bg_texture, 0);
        shader.BindTexture(fg_texture, 1);
        buffer.Draw(shader);

        window.SwapBuffers();
    }

    return 0;
}
