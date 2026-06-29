#include <SDL3/SDL_events.h>
#include <glad/gl.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>

#include <array>
#include <map>

#include "camera.h"
#include "light.h"
#include "material.h"
#include "model.h"
#include "scenes/scenes.h"
#include "texture.h"
#include "transform.h"
#include "utilities.h"
#include "vertex_buffer.h"
#include "window.h"

namespace LrnGL {

struct Actor
{
    Transform Transform;
    Model     Model;
    bool      DrawOutline  = false;
    bool      CullFaces    = true;
    int       CullFaceType = GL_BACK;

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
        glm::vec4 Color       = glm::vec4(0.667f, 0.267f, 0.122f, 1.000f);
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
            ImGui::ColorEdit4("Stencil flat color", glm::value_ptr(Stencil.Color));
        }
        ImGui::End();
    }
};

void InitializeGrass(std::vector<Actor>& actors)
{
}

int AdvancedOpenGLMain(Window& window, int argc, const char** argv)
{
    // Base Phong Shader
    // bool   enable_fog = false;
    // Shader shader(GetAssetPath("shaders/phong.frag"), GetAssetPath("shaders/phong.vert"));

    // Phong Shader with Fog
    bool   enable_fog = true;
    Shader phong_shader(GetAssetPath("shaders/advanced_phong.frag"),
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

    Camera camera(glm::vec3(0.0f, 0.0f, 7.0f), 5.0f, 0.01f);
    camera.InitializeProjection(window);

    std::array<Actor, 6> actors = {
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
        // Sphere
        Actor{
            .Transform =
                {
                    .Position = glm::vec3(-2.1f, 0.0f, 0.0f),
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
        // Grass
        Actor{
            .Transform =
                {
                    .Position = glm::vec3(0.0f, -2.5f, 2.0f),
                },
            .Model     = Mesh(ShapeVertexData::GetPlane(),
                              Material{
                                  .Shader  = &phong_shader,
                                  .Diffuse = Texture(GetAssetPath("textures/grass.png"),
                                                     false,
                                                     true,
                                                     TextureFilter::Linear,
                                                     TextureFilterWrapping::ClampToEdge),
                              }),
            .CullFaces = false,
        },
        // Windows
        Actor{
            .Transform =
                {
                    .Position = glm::vec3(0.0f, 0.0f, 1.0f),
                },
            .Model = Mesh(ShapeVertexData::GetPlane(),
                          Material{Material{
                              .Shader  = &phong_shader,
                              .Diffuse = Texture(GetAssetPath("textures/transparent_window.png"),
                                                 false,
                                                 true,
                                                 TextureFilter::Linear,
                                                 TextureFilterWrapping::ClampToEdge),
                          }}),
            .CullFaces = false,
        },
        Actor{
            .Transform =
                {
                    .Position = glm::vec3(0.0f, 0.0f, 2.0f),
                },
            .Model = Mesh(ShapeVertexData::GetPlane(),
                          Material{Material{
                              .Shader  = &phong_shader,
                              .Diffuse = Texture(GetAssetPath("textures/transparent_window.png"),
                                                 false,
                                                 true,
                                                 TextureFilter::Linear,
                                                 TextureFilterWrapping::ClampToEdge),
                          }}),
            .CullFaces = false,
        },
    };

    AdvancedOpenGLOptions opts;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

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

        glm::mat4 view = camera.CreateViewMatrix();

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

        std::map<float, Actor*> sorted_render_order;
        for (unsigned i = 1; i < actors.size(); i++)
        {
            glm::vec3 direction         = actors[i].Transform.Position - camera.GetPosition();
            float     length_sqr        = glm::dot(direction, direction);
            float     length            = glm::length(direction);
            sorted_render_order[length] = &actors[i];
        }
        sorted_render_order[10000.0f] = &actors[0]; // Set floor to be first last

        // Draw body
        glStencilOp(GL_KEEP,               // Stencil fails
                    GL_KEEP,               // Stencil passes, Depth fails
                    GL_REPLACE);           // Both stencil and depth passes
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // All fragments should pass the stencil test
        for (auto iter = sorted_render_order.rbegin(); iter != sorted_render_order.rend(); iter++)
        {
            Actor* actor = iter->second;

            if (actor->CullFaces)
            {
                glEnable(GL_CULL_FACE);
                glCullFace(actor->CullFaceType);
            }
            else
                glDisable(GL_CULL_FACE);

            if (actor->DrawOutline)
                glStencilMask(0xFF); // Enable writing to the stencil buffer
            actor->Draw(elapsed_time, camera.GetProjectionMatrix(), view);
            glStencilMask(0x00); // Disable writing to stencil buffer
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

            for (auto iter = sorted_render_order.rbegin(); iter != sorted_render_order.rend();
                 iter++)
            {
                Actor* actor = iter->second;
                if (!actor->DrawOutline)
                    continue;

                actor->Model.SetShaderToAllMaterials(&outline_shader);
                actor->Transform.Scale += opts.Stencil.ScaleFactor;
                actor->Draw(elapsed_time, camera.GetProjectionMatrix(), view);
                actor->Transform.Scale -= opts.Stencil.ScaleFactor;
                actor->Model.SetShaderToAllMaterials(&phong_shader);
            }
            glEnable(GL_DEPTH_TEST);
        }

        window.SwapBuffers();
    }

    return 0;
}

} // namespace LrnGL
