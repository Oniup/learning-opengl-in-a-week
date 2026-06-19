#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_timer.h>
#include <fmt/format.h>
#include <glad/gl.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
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

    glEnable(GL_DEPTH_TEST);

    lgl::Shader shader(fmt::format("{}/shaders/Fragment.glsl", asset_dir),
                       fmt::format("{}/shaders/Vertex.glsl", asset_dir));
    shader.InitializeTextureIDs(2);

    lgl::Texture bg_texture(fmt::format("{}/textures/wall.jpg", asset_dir));
    lgl::Texture fg_texture(fmt::format("{}/textures/awesomeface.png", asset_dir));

    lgl::VertexBuffer buffer(true);
    buffer.PushAttribute(0, 3, offsetof(lgl::Vertex, Position));
    buffer.PushAttribute(1, 3, offsetof(lgl::Vertex, Color));
    buffer.PushAttribute(2, 2, offsetof(lgl::Vertex, UV));
    buffer.PushData(lgl::VertexBuffer_Vertex,
                    lgl::Shapes::CubeVertices.size() * sizeof(lgl::Vertex),
                    lgl::Shapes::CubeVertices.data());
    buffer.PushData(lgl::VertexBuffer_Element,
                    lgl::Shapes::CubeIndices.size() * sizeof(unsigned),
                    lgl::Shapes::CubeIndices.data());

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale    = glm::vec3(1.0f);
    glm::vec3 rotation = glm::vec3(1.0f, 0.0f, 0.0f);
    float angle        = glm::radians(-55.0f);
    float speed        = 1.0f;

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.1f, 100.0f);

    glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -2.0f);

    SDL_Event event;
    uint64_t last_time = SDL_GetPerformanceCounter();
    while (window.IsRunning())
    {
        uint64_t current_time = SDL_GetPerformanceCounter();
        float delta = (float)(current_time - last_time) / (float)SDL_GetPerformanceFrequency();
        last_time   = current_time;

        while (SDL_PollEvent(&event))
        {
            window.HandleEvents(event);

            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                projection = glm::perspective(glm::radians(45.0f),
                                              (float)window.GetWidth() / (float)window.GetHeight(),
                                              0.1f,
                                              100.0f);
            }
        }

        const bool* keyboard_state = SDL_GetKeyboardState(nullptr);
        if (keyboard_state[SDL_SCANCODE_W])
            position.z -= speed * delta;
        if (keyboard_state[SDL_SCANCODE_S])
            position.z += speed * delta;
        if (keyboard_state[SDL_SCANCODE_D])
            position.x += speed * delta;
        if (keyboard_state[SDL_SCANCODE_A])
            position.x -= speed * delta;

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        model           = glm::translate(model, position);
        model           = glm::scale(model, scale);
        model           = glm::rotate(model, angle, rotation);

        glm::mat4 view = glm::mat4(1.0f);
        view           = glm::translate(view, cameraPosition);

        shader.BindTexture(bg_texture, 0);
        shader.BindTexture(fg_texture, 1);
        buffer.Draw(shader, projection, view, model);

        window.SwapBuffers();
    }

    return 0;
}
