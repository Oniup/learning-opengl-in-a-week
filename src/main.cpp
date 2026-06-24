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
#include <imgui.h>

#include <filesystem>
#include <numbers>
#include <random>

#include "camera.h"
#include "error.h"
#include "light.h"
#include "material.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include "vertex_buffer.h"
#include "window.h"

struct Object
{
    enum Type
    {
        Eyeball,
        Box,
        Floor,
    };

    Type             Type;
    LrnGL::Transform Transform;
};

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

void HandleCursorInput(LrnGL::Window& window, bool hide_mouse)
{
    SDL_SetWindowRelativeMouseMode(window.GetContext(), hide_mouse);

    ImGuiIO& io = ImGui::GetIO();
    if (hide_mouse)
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    else
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

int main(int argc, char** argv)
{
    std::string   asset_dir = GetAssetDirectory();
    LrnGL::Window window;
    bool          render_wireframe_mode = false;
    bool          hide_mouse            = true;

    LrnGL::Material::LoadDefaultTexture(asset_dir);

    glEnable(GL_DEPTH_TEST);
    HandleCursorInput(window, hide_mouse);

    LrnGL::Shader shader(fmt::format("{}/shaders/Phong.frag", asset_dir),
                         fmt::format("{}/shaders/Phong.vert", asset_dir));
    LrnGL::Material::InitializeMaterialTextureUniforms(shader);

    Object objects[] = {
        Object{Object::Eyeball, LrnGL::Transform{{0.0f, 0.0f, 0.0f}}},
        Object{Object::Box, LrnGL::Transform{{2.0f, 5.0f, -15.0f}}},
        Object{Object::Eyeball, LrnGL::Transform{{-1.5f, -2.2f, -2.5f}}},
        Object{Object::Box, LrnGL::Transform{{-3.8f, -2.0f, -12.3f}}},
        Object{Object::Eyeball, LrnGL::Transform{{2.4f, -0.4f, -3.5f}}},
        Object{Object::Eyeball, LrnGL::Transform{{-1.7f, 3.0f, -7.5f}}},
        Object{Object::Eyeball, LrnGL::Transform{{1.3f, -2.0f, -2.5f}}},
        Object{Object::Box, LrnGL::Transform{{1.5f, 2.0f, -2.5f}}},
        Object{Object::Box, LrnGL::Transform{{1.5f, 0.2f, -1.5f}}},
        Object{Object::Box, LrnGL::Transform{{-1.3f, 1.0f, -1.5f}}},
    };

    std::random_device                    rand_device;
    std::mt19937                          gen(rand_device());
    std::uniform_real_distribution<float> rand_rotation(0.0f, 2 * std::numbers::pi);
    std::uniform_real_distribution<float> rand_scale(0.5f, 2.0f);
    for (unsigned i = 1; i < sizeof(objects) / sizeof(Object); i++)
    {
        Object& object = objects[i];

        object.Transform.Scale = glm::vec3(rand_scale(gen), rand_scale(gen), rand_scale(gen));
        object.Transform.Rotation =
            glm::vec3(rand_rotation(gen), rand_rotation(gen), rand_rotation(gen));
    }

    LrnGL::Material materials[] = {
        // Eyeball
        LrnGL::Material{
            .Diffuse   = LrnGL::Texture(fmt::format("{}/textures/eyeball.png", asset_dir)),
            .Shininess = 32,
        },
        // Box
        LrnGL::Material{
            .Diffuse = LrnGL::Texture(fmt::format("{}/textures/container2.png", asset_dir)),
            .Specular =
                LrnGL::Texture(fmt::format("{}/textures/container2_specular.png", asset_dir)),
            .Shininess = 128,
        },
    };

    LrnGL::VertexBuffer vertex_buffers[] = {
        LrnGL::VertexBuffer(LrnGL::ShapeMesh::GenerateSphere(20, 20)),
        LrnGL::VertexBuffer((LrnGL::ShapeMesh::GetCube())),
    };

    LrnGL::LightManager light_manager(asset_dir);

    float move_speed              = 1.0f;
    float spin_speed              = glm::radians(90.0f);
    objects[0].Transform.Rotation = glm::vec3(0.0f, std::numbers::pi / 4, 0.0f);

    LrnGL::Camera camera(glm::vec3(7.0f, -3.0f, 5.0f), 5.0f, 0.01f);
    camera.InitializeProjection(window);

    SDL_Event event;
    uint64_t  last_time    = SDL_GetPerformanceCounter();
    float     elapsed_time = 0.0f;
    while (window.IsRunning())
    {
        uint64_t current_time = SDL_GetPerformanceCounter();
        float    delta = (float)(current_time - last_time) / (float)SDL_GetPerformanceFrequency();
        last_time      = current_time;
        elapsed_time += delta;

        objects[0].Transform.RotateYaw(delta);
        objects[0].Transform.RotatePitch(delta);

        while (SDL_PollEvent(&event))
        {
            window.HandleEvents(event);

            camera.UpdateLookDirection(hide_mouse, event, delta);
            camera.UpdateProjectionMatrix(hide_mouse, event, window);

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                switch (event.key.key)
                {
                case SDLK_F1:
                    render_wireframe_mode = !render_wireframe_mode;
                    glPolygonMode(GL_FRONT_AND_BACK, render_wireframe_mode ? GL_LINE : GL_FILL);
                    break;

                case SDLK_ESCAPE:
                    hide_mouse = !hide_mouse;
                    HandleCursorInput(window, hide_mouse);
                    break;
                }
            }
        }
        camera.UpdatePosition(delta);
        light_manager.UpdateMenu();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view_matrix = glm::mat4(1.0f);
        view_matrix           = camera.CreateViewMatrix();

        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view_matrix);
        light_manager.PushLightInfoToShader(shader, camera.GetPosition());

        for (const Object& object : objects)
        {
            glm::mat4            model_matrix = object.Transform.CreateModelMatrix();
            LrnGL::Material&     material     = materials[object.Type];
            LrnGL::VertexBuffer& buffer       = vertex_buffers[object.Type];

            material.PushInfoToShader(shader);
            buffer.Draw(shader, camera.GetProjectionMatrix(), view_matrix, model_matrix);
        }

        window.SwapBuffers();
    }

    LrnGL::Material::UnloadDefaultTexture();
    return 0;
}
