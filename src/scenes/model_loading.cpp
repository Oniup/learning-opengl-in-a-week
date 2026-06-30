#include <SDL3/SDL_events.h>
#include <glad/gl.h>
#include <glm/ext/vector_float3.hpp>

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
    Transform transform;
    Model     model;

    void Draw(float elapsed_time, const glm::mat4& projection, const glm::mat4& view) const
    {
        model.Draw(elapsed_time, projection, view, transform);
    }
};

int ModelLoadingMain(Window& window, int argc, const char** argv)
{
    Shader phong_shader(GetAssetPath("shaders/phong.frag"), GetAssetPath("shaders/phong.vert"));
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

    std::array<Actor, 3> actors = {
        // Eyeball
        Actor{
            .transform =
                Transform{
                    .Position = glm::vec3(-2.0f, 0.0f, 0.0f),
                    .Rotation = glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
                },
            .model = Mesh(ShapeVertexData::GenerateSphere(20, 20),
                          Material{
                              .Shader    = &phong_shader,
                              .Diffuse   = Texture(GetAssetPath("textures/eyeball.png"), false),
                              .Specular  = glm::vec3(1.0f),
                              .Shininess = 32,
                          }),
        },
        // Backpack
        Actor{
            .transform =
                Transform{
                    .Position = glm::vec3(2.0f, 0.0f, 0.0f),
                },
            .model = Model(
                GetAssetPath("models/backpack/backpack.obj"), &phong_shader, ModelLoading_FlipUVs),
        },
        // Robot
        Actor{

            .transform =
                Transform{
                    .Position = glm::vec3(6.0f, -1.0f, 0.0f),
                },
            .model = Model(GetAssetPath("models/soldier/soldier.fbx"),
                           &phong_shader,
                           ModelLoading_DisableTransformingVertices),
        },
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

        glm::mat4 view = glm::mat4(1.0f);
        view           = camera.CreateViewMatrix();

        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view);
        light_manager.PushLightInfoToShader(phong_shader, camera.GetPosition());

        for (const Actor& actor : actors)
            actor.Draw(elapsed_time, camera.GetProjectionMatrix(), view);

        window.SwapBuffers();
    }

    return 0;
}

} // namespace LrnGL
