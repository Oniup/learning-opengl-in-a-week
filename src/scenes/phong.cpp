#include <SDL3/SDL_events.h>
#include <fmt/format.h>
#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>

#include <numbers>
#include <random>

#include "camera.h"
#include "light.h"
#include "material.h"
#include "scenes/scenes.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include "utilities.h"
#include "vertex_buffer.h"
#include "window.h"

namespace LrnGL {

struct Object
{
    enum Type
    {
        Eyeball,
        Box,
    };

    Type      Type;
    Transform Transform;
};

int PhongMain(const std::string& asset_dir, Window& window, int argc, const char** argv)
{
    Shader phong_shader(fmt::format("{}/shaders/Phong.frag", asset_dir),
                        fmt::format("{}/shaders/Phong.vert", asset_dir));
    InitializeMaterialTextureUniforms(phong_shader);

    Object objects[] = {
        Object{Object::Eyeball, Transform{{0.0f, 0.0f, 0.0f}}},
        Object{Object::Box, Transform{{2.0f, 5.0f, -15.0f}}},
        Object{Object::Eyeball, Transform{{-1.5f, -2.2f, -2.5f}}},
        Object{Object::Box, Transform{{-3.8f, -2.0f, -12.3f}}},
        Object{Object::Eyeball, Transform{{2.4f, -0.4f, -3.5f}}},
        Object{Object::Eyeball, Transform{{-1.7f, 3.0f, -7.5f}}},
        Object{Object::Eyeball, Transform{{1.3f, -2.0f, -2.5f}}},
        Object{Object::Box, Transform{{1.5f, 2.0f, -2.5f}}},
        Object{Object::Box, Transform{{1.5f, 0.2f, -1.5f}}},
        Object{Object::Box, Transform{{-1.3f, 1.0f, -1.5f}}},
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

    Material materials[] = {
        // Eyeball
        Material{
            .Shader    = &phong_shader,
            .Diffuse   = Texture(fmt::format("{}/textures/eyeball.png", asset_dir)),
            .Specular  = glm::vec3(1.0f),
            .Shininess = 32,
        },
        // Box
        Material{
            .Shader    = &phong_shader,
            .Diffuse   = Texture(fmt::format("{}/textures/container2.png", asset_dir)),
            .Specular  = Texture(fmt::format("{}/textures/container2_specular2.png", asset_dir)),
            .Emission  = Texture(fmt::format("{}/textures/matrix.jpg", asset_dir)),
            .Shininess = 128,
        },
    };

    VertexBuffer vertex_buffers[] = {
        VertexBuffer(ShapeVertexData::GenerateSphere(20, 20)),
        VertexBuffer((ShapeVertexData::GetCube())),
    };

    LightManager light_manager(asset_dir);
    // Camera light
    light_manager.PushLight(LightData{
        .Type            = LightData::Spot,
        .Position        = glm::vec3(3.0f, 3.0f, 3.0f),
        .Linear          = 0.07f,
        .Quadratic       = 0.017f,
        .ShowDebugVisual = true,
    });
    light_manager.PushLight(LightData{
        .Position = glm::vec3(3.0f, 3.0f, 3.0f),
        .Color    = RGBToNormalized(215, 188, 133),
    });

    float move_speed              = 1.0f;
    float spin_speed              = glm::radians(90.0f);
    objects[0].Transform.Rotation = glm::vec3(0.0f, std::numbers::pi / 4, 0.0f);

    Camera camera(glm::vec3(7.0f, -3.0f, 5.0f), 5.0f, 0.01f);
    camera.InitializeProjection(window);

    SDL_Event event;
    float     elapsed_time = 0.0f;
    while (window.IsRunning())
    {
        float delta;
        CalculateDeltaTime(elapsed_time, delta);

        objects[0].Transform.RotatePitch(delta * 1.5f);
        objects[0].Transform.RotateYaw(delta * 2.0f);
        objects[0].Transform.RotateRoll(delta * 0.5);

        while (SDL_PollEvent(&event))
        {
            CommonEventHandles(event, window, camera, delta);
        }
        camera.UpdatePosition(delta);
        light_manager.EditLightPropertiesMenu();

        LightData* light = light_manager.GetLight(0);
        if (light)
        {
            light->Position  = camera.GetPosition();
            light->Direction = camera.GetForward();
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view_matrix = glm::mat4(1.0f);
        view_matrix           = camera.CreateViewMatrix();

        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view_matrix);
        light_manager.PushLightInfoToShader(phong_shader, camera.GetPosition());

        for (const Object& object : objects)
        {
            glm::mat4     model_matrix = object.Transform.CreateModelMatrix();
            Material&     material     = materials[object.Type];
            VertexBuffer& buffer       = vertex_buffers[object.Type];

            material.PushInfoToShader();
            material.Shader->Uniform("u_Time", elapsed_time);

            buffer.Draw(*material.Shader, camera.GetProjectionMatrix(), view_matrix, model_matrix);
        }

        window.SwapBuffers();
    }

    return 0;
}

} // namespace LrnGL
