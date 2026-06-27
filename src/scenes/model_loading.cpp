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
    Model     Model;

    void Draw(float elapsed_time, const glm::mat4& projection, const glm::mat4& view) const
    {
        Model.Draw(elapsed_time, projection, view, Transform);
    }
};

int ModelLoadingMain(Window& window, int argc, const char** argv)
{
    Shader phong_shader(GetAssetPath("shaders/Phong.frag"), GetAssetPath("shaders/Phong.vert"));
    InitializeMaterialTextureUniforms(phong_shader);

    LightManager light_manager;
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

    Mesh mesh = Mesh(ShapeVertexData::GenerateSphere(20, 20),
                     Material{
                         .Shader    = &phong_shader,
                         .Diffuse   = Texture(GetAssetPath("textures/eyeball.png")),
                         .Specular  = glm::vec3(1.0f),
                         .Shininess = 32,
                     });

    Actor eyeball{
        .Transform =
            Transform{
                .Position = glm::vec3(-2.0f, 0.0f, 0.0f),
                .Rotation = glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
            },
        .Model = Mesh(ShapeVertexData::GenerateSphere(20, 20),
                      Material{
                          .Shader    = &phong_shader,
                          .Diffuse   = Texture(GetAssetPath("textures/eyeball.png")),
                          .Specular  = glm::vec3(1.0f),
                          .Shininess = 32,
                      }),
    };

    Actor guitar_backpack{
        .Transform =
            Transform{
                .Position = glm::vec3(2.0f, 0.0f, 0.0f),
            },
        .Model = Model(GetAssetPath("models/backpack/backpack.obj"), &phong_shader, false),
    };

    Actor bmw{
        .Transform =
            Transform{
                .Position = glm::vec3(10.0f, 0.0f, 0.0f),
                .Scale    = glm::vec3(0.01f),
            },
        .Model = Model(GetAssetPath("models/bmw/bmw.obj"), &phong_shader, false),
    };

    // Actor guitar_backpack{
    //     .Transform =
    //         Transform{
    //             .Position = glm::vec3(2.0f, 0.0f, 0.0f),
    //             .Scale    = glm::vec3(0.01f),
    //         },
    //     .Model = Model(GetAssetPath("models/survival_guitar_backpack.glb"), &phong_shader),
    // };

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

        glm::mat4 view = glm::mat4(1.0f);
        view           = camera.CreateViewMatrix();

        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view);
        light_manager.PushLightInfoToShader(phong_shader, camera.GetPosition());

        eyeball.Draw(elapsed_time, camera.GetProjectionMatrix(), view);
        guitar_backpack.Draw(elapsed_time, camera.GetProjectionMatrix(), view);
        bmw.Draw(elapsed_time, camera.GetProjectionMatrix(), view);

        window.SwapBuffers();
    }

    return 0;
}

} // namespace LrnGL
