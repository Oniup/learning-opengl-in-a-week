#include <SDL3/SDL_events.h>
#include <glad/gl.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>

#include <array>

#include "camera.h"
#include "light.h"
#include "material.h"
#include "model.h"
#include "scenes/scenes.h"
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

struct Fog
{
    glm::vec3 Color           = glm::vec3(0.1f);
    float     Density         = 0.03f;
    bool      ViewDepthBuffer = false;
    bool      SquareFog       = true;

    void PushInfoToShader(Shader& shader, float near, float far)
    {
        shader.Uniform("u_Near", near);
        shader.Uniform("u_Far", far);
        shader.Uniform("u_FogDensity", Density);
        shader.Uniform("u_FogColor", Color);
        shader.Uniform("u_ViewDepthBuffer", ViewDepthBuffer);
        shader.Uniform("u_SquareFog", SquareFog);
    }

    void Edit()
    {
        ImGui::Begin("Edit Fog");
        ImGui::Checkbox("View depth buffer", &ViewDepthBuffer);
        ImGui::Checkbox("Square density", &SquareFog);
        ImGui::ColorEdit3("Color", glm::value_ptr(Color));
        ImGui::SliderFloat("Density", &Density, 0.0f, 1.0f, "%.4f");
        ImGui::End();
    }
};

int AdvancedOpenGLMain(Window& window, int argc, const char** argv)
{
    glEnable(GL_DEPTH_TEST);

    // Base Phong Shader
    // bool   enable_fog = false;
    // Shader shader(GetAssetPath("shaders/phong.frag"), GetAssetPath("shaders/phong.vert"));

    // Phong Shader with Fog
    bool   enable_fog = true;
    Shader shader(GetAssetPath("shaders/phong_with_fog.frag"), GetAssetPath("shaders/phong.vert"));

    InitializeMaterialTextureUniforms(shader);

    LightManager light_manager;
    light_manager.PushLight(LightData{
        .Type            = LightData::Directional,
        .Direction       = glm::vec3(1.0f, -1.0f, -0.4f),
        .ShowDebugVisual = true,
    });
    light_manager.PushLight(LightData{
        .Position = glm::vec3(3.0f, 3.0f, 3.0f),
        .Color    = RGBToNormalized(215, 188, 133),
    });

    Camera camera(glm::vec3(0.0f, 0.0f, 6.0f), 5.0f, 0.01f);
    camera.InitializeProjection(window);

    std::array<Actor, 3> actors = {
        // Sphere
        Actor{
            .Transform =
                {
                    .Position = glm::vec3(-2.0f, 0.0f, 0.0f),
                    .Rotation = glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
                },
            .Model = Mesh(ShapeVertexData::GenerateSphere(20, 20),
                          Material{
                              .Shader    = &shader,
                              .Diffuse   = Texture(GetAssetPath("textures/eyeball.png"), false),
                              .Specular  = glm::vec3(1.0f),
                              .Shininess = 256,
                          }),
        },
        // Backpack
        Actor{
            .Transform =
                {
                    .Position = glm::vec3(2.0f, 0.0f, 0.0f),
                },
            .Model =
                Model(GetAssetPath("models/backpack/backpack.obj"), &shader, ModelLoading_FlipUVs),
        },
        // Floor
        Actor{.Transform =
                  {
                      .Position = glm::vec3(0.0f, -3.0f, 0.0f),
                      .Scale    = glm::vec3(20.0f, 20.0f, 1.0f),
                      .Rotation = glm::vec3(glm::radians(90.0f), 0.0f, 0.0f),
                  },
              .Model =
                  Mesh(ShapeVertexData::GetPlane(),
                       Material{
                           .Shader   = &shader,
                           .Diffuse  = Texture(GetAssetPath("textures/wood-floor/diffuse.png")),
                           .Specular = Texture(GetAssetPath("textures/wood-floor/specular.png")),
                           .TilingFactor = glm::vec2(3.0f),
                           .Shininess    = 10,
                       })},
    };

    Fog fog;

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

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::mat4(1.0f);
        view           = camera.CreateViewMatrix();

        light_manager.EditLightPropertiesMenu();
        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view);
        light_manager.PushLightInfoToShader(shader, camera.GetPosition());

        if (enable_fog)
        {
            fog.Edit();
            fog.PushInfoToShader(shader, camera.GetNearPlane(), camera.GetFarPlane());
        }

        for (const Actor& actor : actors)
            actor.Draw(elapsed_time, camera.GetProjectionMatrix(), view);

        window.SwapBuffers();
    }

    return 0;
}

} // namespace LrnGL
