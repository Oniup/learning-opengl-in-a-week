#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_timer.h>
#include <fmt/format.h>
#include <glad/gl.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

#include <filesystem>
#include <numbers>

#include "Camera.h"
#include "Error.h"
#include "Shader.h"
#include "Texture.h"
#include "Transform.h"
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
    bool render_wireframe_mode = false;

    glEnable(GL_DEPTH_TEST);
    SDL_SetWindowRelativeMouseMode(window.GetContext(), true);

    lgl::Shader shader(fmt::format("{}/shaders/Fragment.glsl", asset_dir),
                       fmt::format("{}/shaders/Vertex.glsl", asset_dir));
    shader.InitializeTextureIDs(2);

    lgl::Texture bg_texture(fmt::format("{}/textures/wall.jpg", asset_dir));
    lgl::Texture fg_texture(fmt::format("{}/textures/awesomeface.png", asset_dir));
    lgl::Texture eye_texture(fmt::format("{}/textures/eyeball.png", asset_dir));

    lgl::VertexBuffer buffer(lgl::ShapeMesh::GenerateSphere(12, 12));
    // lgl::VertexBuffer buffer(lgl::ShapeMesh::GetCube());

    lgl::Transform transforms[] = {
        lgl::Transform{glm::vec3(0.0f, 0.0f, 0.0f)},
        lgl::Transform{glm::vec3(2.0f, 5.0f, -15.0f)},
        lgl::Transform{glm::vec3(-1.5f, -2.2f, -2.5f)},
        lgl::Transform{glm::vec3(-3.8f, -2.0f, -12.3f)},
        lgl::Transform{glm::vec3(2.4f, -0.4f, -3.5f)},
        lgl::Transform{glm::vec3(-1.7f, 3.0f, -7.5f)},
        lgl::Transform{glm::vec3(1.3f, -2.0f, -2.5f)},
        lgl::Transform{glm::vec3(1.5f, 2.0f, -2.5f)},
        lgl::Transform{glm::vec3(1.5f, 0.2f, -1.5f)},
        lgl::Transform{glm::vec3(-1.3f, 1.0f, -1.5f)},
    };

    float move_speed       = 1.0f;
    float spin_speed       = glm::radians(90.0f);
    transforms[0].Rotation = glm::vec3(0.0f, std::numbers::pi / 4, 0.0f);

    lgl::Camera camera(glm::vec3(7.0f, -3.0f, 5.0f), 5.0f, 0.01f);
    camera.InitializeProjection(window);

    SDL_Event event;
    uint64_t last_time = SDL_GetPerformanceCounter();
    float elapsed_time = 0.0f;
    while (window.IsRunning())
    {
        uint64_t current_time = SDL_GetPerformanceCounter();
        float delta = (float)(current_time - last_time) / (float)SDL_GetPerformanceFrequency();
        last_time   = current_time;
        elapsed_time += delta;

        transforms[0].RotateYaw(delta);
        transforms[0].RotatePitch(delta);

        while (SDL_PollEvent(&event))
        {
            window.HandleEvents(event);

            camera.UpdateLookDirection(event, delta);
            camera.UpdateProjectionMatrix(event, window);

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_ESCAPE)
                {
                    render_wireframe_mode = !render_wireframe_mode;
                    glPolygonMode(GL_FRONT_AND_BACK, render_wireframe_mode ? GL_LINE : GL_FILL);
                }
            }
        }
        camera.UpdatePosition(delta);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::mat4(1.0f);
        view           = camera.CreateViewMatrix();

        shader.Uniform("u_TextureCount", 1);
        shader.BindTexture(eye_texture, 0);
        // shader.BindTexture(bg_texture, 0);
        // shader.BindTexture(fg_texture, 1);
        for (const lgl::Transform& transform : transforms)
        {
            glm::mat4 model = transform.CreateModelMatrix();
            buffer.Draw(shader, camera.GetProjectionMatrix(), view, model);
        }

        window.SwapBuffers();
    }

    return 0;
}
