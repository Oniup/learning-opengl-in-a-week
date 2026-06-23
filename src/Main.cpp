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
#include "Light.h"
#include "Shader.h"
#include "Texture.h"
#include "Transform.h"
#include "VertexBuffer.h"
#include "Window.h"

struct Object
{
    enum Type
    {
        Eyeball,
        Box,
    };

    Type Type;
    lgl::Transform Transform;
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

int main(int argc, char** argv)
{
    std::string asset_dir = GetAssetDirectory();
    lgl::Window window(800, 800);
    bool render_wireframe_mode = false;

    glEnable(GL_DEPTH_TEST);
    SDL_SetWindowRelativeMouseMode(window.GetContext(), true);

    lgl::Shader shader(fmt::format("{}/shaders/BasicF.glsl", asset_dir),
                       fmt::format("{}/shaders/BasicV.glsl", asset_dir));
    lgl::Shader light_shader(fmt::format("{}/shaders/LightF.glsl", asset_dir),
                             fmt::format("{}/shaders/LightV.glsl", asset_dir));
    shader.InitializeTextureIDs(2);

    lgl::Texture bg_texture(fmt::format("{}/textures/wall.jpg", asset_dir));
    lgl::Texture fg_texture(fmt::format("{}/textures/awesomeface.png", asset_dir));
    lgl::Texture eye_texture(fmt::format("{}/textures/eyeball.png", asset_dir));

    lgl::VertexBuffer eyeball_buffer(lgl::ShapeMesh::GenerateSphere(20, 20));
    lgl::VertexBuffer box_buffer(lgl::ShapeMesh::GetCube());

    Object objects[] = {
        Object{Object::Eyeball, lgl::Transform{{0.0f, 0.0f, 0.0f}}    },
        Object{Object::Box,     lgl::Transform{{2.0f, 5.0f, -15.0f}}  },
        Object{Object::Eyeball, lgl::Transform{{-1.5f, -2.2f, -2.5f}} },
        Object{Object::Box,     lgl::Transform{{-3.8f, -2.0f, -12.3f}}},
        Object{Object::Eyeball, lgl::Transform{{2.4f, -0.4f, -3.5f}}  },
        Object{Object::Eyeball, lgl::Transform{{-1.7f, 3.0f, -7.5f}}  },
        Object{Object::Eyeball, lgl::Transform{{1.3f, -2.0f, -2.5f}}  },
        Object{Object::Box,     lgl::Transform{{1.5f, 2.0f, -2.5f}}   },
        Object{Object::Box,     lgl::Transform{{1.5f, 0.2f, -1.5f}}   },
        Object{Object::Box,     lgl::Transform{{-1.3f, 1.0f, -1.5f}}  },
    };

    lgl::LightRenderer light_renderer;
    std::array<lgl::GPULightData, 1> light_data = {
        lgl::GPULightData{.Position = glm::vec3(3.0f, 3.0f, 3.0f)},
    };

    float move_speed              = 1.0f;
    float spin_speed              = glm::radians(90.0f);
    objects[0].Transform.Rotation = glm::vec3(0.0f, std::numbers::pi / 4, 0.0f);

    lgl::Camera camera(glm::vec3(7.0f, -3.0f, 5.0f), 5.0f, 0.01f, light_data[0].Position);
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

        objects[0].Transform.RotateYaw(delta);
        objects[0].Transform.RotatePitch(delta);

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

        for (const lgl::GPULightData& light : light_data)
            light_renderer.Draw(light, light_shader, camera.GetProjectionMatrix(), view);

        for (const Object& object : objects)
        {
            glm::mat4 model = object.Transform.CreateModelMatrix();

            if (object.Type == Object::Eyeball)
            {
                shader.Uniform("u_TextureCount", 1);
                shader.BindTexture(eye_texture, 0);
                eyeball_buffer.Draw(shader, camera.GetProjectionMatrix(), view, model);
            }
            else
            {
                shader.Uniform("u_TextureCount", 2);
                shader.BindTexture(bg_texture, 0);
                shader.BindTexture(fg_texture, 1);
                box_buffer.Draw(shader, camera.GetProjectionMatrix(), view, model);
            }
        }

        window.SwapBuffers();
    }

    return 0;
}
