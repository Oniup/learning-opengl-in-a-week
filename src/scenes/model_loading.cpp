#include <SDL3/SDL_events.h>
#include <assimp/Importer.hpp>
#include <fmt/format.h>
#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>

#include "camera.h"
#include "light.h"
#include "material.h"
#include "model.h"
#include "scenes/scenes.h"
#include "shader.h"
#include "transform.h"
#include "utilities.h"
#include "vertex_buffer.h"
#include "window.h"

namespace LrnGL {

struct Actor
{
    Transform Transform;
    Mesh      Mesh;
};

int ModelLoadingMain(const std::string& asset_dir, Window& window, int argc, const char** argv)
{
    Shader phong_shader(fmt::format("{}/shaders/Phong.frag", asset_dir),
                        fmt::format("{}/shaders/Phong.vert", asset_dir));
    InitializeMaterialTextureUniforms(phong_shader);

    LightManager light_manager(asset_dir);
    light_manager.PushLight(LightData{
        .Type            = LightData::Directional,
        .Direction       = glm::vec3(1.0f, -1.0f, 0.0f),
        .ShowDebugVisual = true,
    });
    light_manager.PushLight(LightData{
        .Position = glm::vec3(3.0f, 3.0f, 3.0f),
        .Color    = RGBToNormalized(215, 188, 133),
    });

    Camera camera(glm::vec3(0.0f, 0.0f, 6.0f), 5.0f, 0.01f);
    camera.InitializeProjection(window);

    Actor eyeball = Actor{
        .Transform =
            Transform{
                .Position = glm::vec3(-2.0f, 0.0f, 0.0f),
                .Rotation = glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
            },
        .Mesh = Mesh(ShapeVertexData::GenerateSphere(20, 20),
                     Material{
                         .Shader    = &phong_shader,
                         .Diffuse   = Texture(fmt::format("{}/textures/eyeball.png", asset_dir)),
                         .Specular  = glm::vec3(1.0f),
                         .Shininess = 32,
                     }),
    };

    SDL_Event event;
    float     elapsed_time = 0.0f;
    while (window.IsRunning())
    {
        float delta;
        CalculateDeltaTime(elapsed_time, delta);

        while (SDL_PollEvent(&event))
        {
            CommonEventHandles(event, window, camera, delta);
        }
        camera.UpdatePosition(delta);
        light_manager.EditLightPropertiesMenu();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view_matrix = glm::mat4(1.0f);
        view_matrix           = camera.CreateViewMatrix();

        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view_matrix);
        light_manager.PushLightInfoToShader(phong_shader, camera.GetPosition());

        eyeball.Mesh.GetMaterial().Shader->Uniform("u_Time", elapsed_time);
        eyeball.Mesh.Draw(eyeball.Transform, camera.GetProjectionMatrix(), view_matrix);

        window.SwapBuffers();
    }

    return 0;
}

} // namespace LrnGL
