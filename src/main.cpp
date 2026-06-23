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

#include "camera.h"
#include "error.h"
#include "light.h"
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

    glEnable(GL_DEPTH_TEST);
    HandleCursorInput(window, hide_mouse);

    LrnGL::Shader obj_shader(fmt::format("{}/shaders/Phong.frag", asset_dir),
                             fmt::format("{}/shaders/Phong.vert", asset_dir));
    obj_shader.InitializeTextureIDs(2);

    LrnGL::Texture bg_texture(fmt::format("{}/textures/wall.jpg", asset_dir));
    LrnGL::Texture fg_texture(fmt::format("{}/textures/awesomeface.png", asset_dir));
    LrnGL::Texture eye_texture(fmt::format("{}/textures/eyeball.png", asset_dir));

    LrnGL::VertexBuffer eyeball_buffer(LrnGL::ShapeMesh::GenerateSphere(20, 20));
    LrnGL::VertexBuffer box_buffer(LrnGL::ShapeMesh::GetCube());

    Object objects[] = {
        Object{Object::Eyeball, LrnGL::Transform{{0.0f, 0.0f, 0.0f}}    },
        Object{Object::Box,     LrnGL::Transform{{2.0f, 5.0f, -15.0f}}  },
        Object{Object::Eyeball, LrnGL::Transform{{-1.5f, -2.2f, -2.5f}} },
        Object{Object::Box,     LrnGL::Transform{{-3.8f, -2.0f, -12.3f}}},
        Object{Object::Eyeball, LrnGL::Transform{{2.4f, -0.4f, -3.5f}}  },
        Object{Object::Eyeball, LrnGL::Transform{{-1.7f, 3.0f, -7.5f}}  },
        Object{Object::Eyeball, LrnGL::Transform{{1.3f, -2.0f, -2.5f}}  },
        Object{Object::Box,     LrnGL::Transform{{1.5f, 2.0f, -2.5f}}   },
        Object{Object::Box,     LrnGL::Transform{{1.5f, 0.2f, -1.5f}}   },
        Object{Object::Box,     LrnGL::Transform{{-1.3f, 1.0f, -1.5f}}  },
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

        glm::mat4 view = glm::mat4(1.0f);
        view           = camera.CreateViewMatrix();

        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view);

        for (const Object& object : objects)
        {
            light_manager.PushLightInfoToShader(obj_shader, camera.GetPosition());
            glm::mat4 model = object.Transform.CreateModelMatrix();

            switch (object.Type)
            {
            case Object::Eyeball:
                obj_shader.Uniform("u_TextureCount", 1);
                obj_shader.Uniform("u_TilingFactor", glm::vec2(1.0f));
                obj_shader.BindTexture(eye_texture, 0);
                eyeball_buffer.Draw(obj_shader, camera.GetProjectionMatrix(), view, model);
                break;
            case Object::Box:
                obj_shader.Uniform("u_TextureCount", 2);
                obj_shader.Uniform("u_TilingFactor", glm::vec2(1.0f));
                obj_shader.BindTexture(bg_texture, 0);
                obj_shader.BindTexture(fg_texture, 1);
                box_buffer.Draw(obj_shader, camera.GetProjectionMatrix(), view, model);
                break;
            case Object::Floor: FATAL("Not implemented yet");
            }
        }

        window.SwapBuffers();
    }

    return 0;
}
