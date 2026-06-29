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
    bool      DrawOutline = false;

    void Draw(float elapsed_time, const glm::mat4& projection, const glm::mat4& view) const
    {
        Model.Draw(elapsed_time, projection, view, Transform);
    }
};

struct AdvancedOpenGLOptions
{
    struct
    {
        glm::vec3 Color           = glm::vec3(0.1f);
        float     Density         = 0.03f;
        bool      ViewDepthBuffer = false;
        bool      SquareFog       = true;
    } Fog;

    struct
    {
        bool      Disable     = false;
        bool      DrawOutline = true;
        bool      IgnoreDepth = false;
        float     ScaleFactor = 0.05f;
        glm::vec3 Color       = RGBToNormalized(170, 68, 31);
    } Stencil;

    void PushInfoToShader(Shader& shader, float near, float far)
    {
        shader.Uniform("u_Near", near);
        shader.Uniform("u_Far", far);
        shader.Uniform("u_FogDensity", Fog.Density);
        shader.Uniform("u_FogColor", Fog.Color);
        shader.Uniform("u_ViewDepthBuffer", Fog.ViewDepthBuffer);
        shader.Uniform("u_SquareFog", Fog.SquareFog);
    }

    void Edit()
    {
        ImGui::Begin("Edit Fog");
        ImGui::Checkbox("View depth buffer", &Fog.ViewDepthBuffer);
        ImGui::Checkbox("Square density", &Fog.SquareFog);
        ImGui::ColorEdit3("Color", glm::value_ptr(Fog.Color));
        ImGui::SliderFloat("Density", &Fog.Density, 0.0f, 1.0f, "%.4f");
        ImGui::End();

        ImGui::Begin("Stencil");
        ImGui::Checkbox("Disable", &Stencil.Disable);
        if (!Stencil.Disable)
        {
            ImGui::Checkbox("Draw outline", &Stencil.DrawOutline);
            ImGui::Checkbox("Ignore depth", &Stencil.IgnoreDepth);
            ImGui::DragFloat("Scale factor", &Stencil.ScaleFactor, 0.01f, 0.0f, 1.0f);
            ImGui::ColorEdit3("Stencil flat color", glm::value_ptr(Stencil.Color));
        }
        ImGui::End();
    }
};

int AdvancedOpenGLMain(Window& window, int argc, const char** argv)
{
    // Base Phong Shader
    // bool   enable_fog = false;
    // Shader shader(GetAssetPath("shaders/phong.frag"), GetAssetPath("shaders/phong.vert"));

    // Phong Shader with Fog
    bool   enable_fog = true;
    Shader phong_shader(GetAssetPath("shaders/phong_with_fog.frag"),
                        GetAssetPath("shaders/phong.vert"));
    Shader outline_shader(GetAssetPath("shaders/flat_color.frag"),
                          GetAssetPath("shaders/basic.vert"));

    InitializeMaterialTextureUniforms(phong_shader);

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
            .Model       = Mesh(ShapeVertexData::GenerateSphere(20, 20),
                                Material{
                                    .Shader    = &phong_shader,
                                    .Diffuse   = Texture(GetAssetPath("textures/eyeball.png"), false),
                                    .Specular  = glm::vec3(1.0f),
                                    .Shininess = 256,
                                }),
            .DrawOutline = true,
        },
        // Backpack
        Actor{
            .Transform =
                {
                    .Position = glm::vec3(2.0f, 0.0f, 0.0f),
                },
            .Model = Model(
                GetAssetPath("models/backpack/backpack.obj"), &phong_shader, ModelLoading_FlipUVs),
        },
        // Floor
        Actor{
            .Transform =
                {
                    .Position = glm::vec3(0.0f, -3.0f, 0.0f),
                    .Scale    = glm::vec3(20.0f, 20.0f, 1.0f),
                    .Rotation = glm::vec3(glm::radians(90.0f), 0.0f, 0.0f),
                },
            .Model = Mesh(ShapeVertexData::GetPlane(),
                          Material{
                              .Shader   = &phong_shader,
                              .Diffuse  = Texture(GetAssetPath("textures/wood-floor/diffuse.png")),
                              .Specular = Texture(GetAssetPath("textures/wood-floor/specular.png")),
                              .TilingFactor = glm::vec2(3.0f),
                              .Shininess    = 10,
                          }),
        },
    };

    AdvancedOpenGLOptions opts;

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

        glm::mat4 view = glm::mat4(1.0f);
        view           = camera.CreateViewMatrix();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        glStencilMask(0xFF);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilMask(0x00);

        light_manager.EditLightPropertiesMenu();
        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view);
        light_manager.PushLightInfoToShader(phong_shader, camera.GetPosition());

        if (enable_fog)
        {
            opts.Edit();
            opts.PushInfoToShader(phong_shader, camera.GetNearPlane(), camera.GetFarPlane());
        }

        // Draw body
        glStencilOp(GL_KEEP,               // Stencil fails
                    GL_KEEP,               // Stencil passes, Depth fails
                    GL_REPLACE);           // Both stencil and depth passes
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // All fragments should pass the stencil test
        for (const Actor& actor : actors)
        {
            if (actor.DrawOutline)
                glStencilMask(0xFF); // Enable writing to the stencil buffer
            actor.Draw(elapsed_time, camera.GetProjectionMatrix(), view);
            glStencilMask(0x00); // Disable writing to stencil buffer
            // glDisable(GL_STENCIL_TEST);
        }

        // Draw outline
        if (!opts.Stencil.Disable)
        {
            outline_shader.Uniform("u_FlatColor", opts.Stencil.Color);
            int type = opts.Stencil.DrawOutline ? GL_NOTEQUAL : GL_ALWAYS;

            if (opts.Stencil.IgnoreDepth)
                glDisable(GL_DEPTH_TEST); // Don't want depth to affect the outline
            glStencilFunc(type, 1, 0xFF); // Check if ref is not equal to the mask
            glStencilMask(0x00);          // Disable writing to the stencil buffer

            for (Actor& actor : actors)
            {
                if (!actor.DrawOutline)
                    continue;

                actor.Model.SetShaderToAllMaterials(&outline_shader);
                actor.Transform.Scale += opts.Stencil.ScaleFactor;
                actor.Draw(elapsed_time, camera.GetProjectionMatrix(), view);
                actor.Transform.Scale -= opts.Stencil.ScaleFactor;
                actor.Model.SetShaderToAllMaterials(&phong_shader);
            }
            glEnable(GL_DEPTH_TEST);
        }

        window.SwapBuffers();
    }

    return 0;
}

} // namespace LrnGL
