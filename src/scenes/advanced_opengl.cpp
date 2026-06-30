#include <SDL3/SDL_events.h>
#include <glad/gl.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>

#include <map>

#include "camera.h"
#include "error.h"
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
    using Array = std::array<Actor, 6>;
    static Array CreateActors(Shader& shader);

    Transform transform;
    Model     model;
    bool      draw_outline   = false;
    bool      cull_faces     = true;
    int       cull_face_type = GL_BACK;

    void Draw(float elapsed_time, const glm::mat4& projection, const glm::mat4& view) const
    {
        model.Draw(elapsed_time, projection, view, transform);
    }
};

// clang-format off
float framebuffer_quad_vertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};
// clang-format on

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

int AdvancedOpenGLMain(Window& window, int argc, const char** argv)
{
    // Phong Shader with Fog
    bool   enable_fog = true;
    Shader phong_shader(GetAssetPath("shaders/advanced_phong.frag"),
                        GetAssetPath("shaders/phong.vert"));
    Shader outline_shader(GetAssetPath("shaders/flat_color.frag"),
                          GetAssetPath("shaders/basic.vert"));
    Shader framebuffer_shader(GetAssetPath("shaders/framebuffer.frag"),
                              GetAssetPath("shaders/framebuffer.vert"));

    framebuffer_shader.Uniform("u_ColorAttachment", 0);

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

    Actor::Array          actors = Actor::CreateActors(phong_shader);
    AdvancedOpenGLOptions opts;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    // Create framebuffer
    unsigned framebuffer;
    unsigned framebuffer_color;
    unsigned framebuffer_depth_stencil;
    unsigned framebuffer_vao;
    unsigned framebuffer_vbo;
    glGenFramebuffers(1, &framebuffer);
    glGenTextures(1, &framebuffer_color);
    glGenRenderbuffers(1, &framebuffer_depth_stencil);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_2D, framebuffer_color);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer_depth_stencil);

    // Color texture attachment
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 window.GetWidth(),
                 window.GetHeight(),
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_color, 0);

    // Depth and stencil render buffer object attachment
    glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window.GetWidth(), window.GetHeight());
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer_depth_stencil);

    // framebuffer creation complete
    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Reset back to default framebuffer

    // framebuffer quad data
    glGenVertexArrays(1, &framebuffer_vao);
    glGenBuffers(1, &framebuffer_vbo);
    glBindVertexArray(framebuffer_vao);
    glBindBuffer(GL_ARRAY_BUFFER, framebuffer_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(framebuffer_quad_vertices),
                 framebuffer_quad_vertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    SDL_Event event;
    float     elapsed_time = 0.0f;
    while (window.IsRunning())
    {
        float delta;
        CalculateDeltaTime(elapsed_time, delta);

        while (SDL_PollEvent(&event))
        {
            CommonEventHandles(event, window, camera, delta);
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
            }
        }
        camera.UpdatePosition(delta);

        glm::mat4 view = camera.CreateViewMatrix();

        // =========================================================================================
        // First render pass (Rendering scene)
        // =========================================================================================

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

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
            glm::vec3 direction         = actors[i].transform.Position - camera.GetPosition();
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

            if (actor->cull_faces)
            {
                glEnable(GL_CULL_FACE);
                glCullFace(actor->cull_face_type);
            }
            else
                glDisable(GL_CULL_FACE);

            if (actor->draw_outline)
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
                if (!actor->draw_outline)
                    continue;

                actor->model.SetShaderToAllMaterials(&outline_shader);
                actor->transform.Scale += opts.Stencil.ScaleFactor;
                actor->Draw(elapsed_time, camera.GetProjectionMatrix(), view);
                actor->transform.Scale -= opts.Stencil.ScaleFactor;
                actor->model.SetShaderToAllMaterials(&phong_shader);
            }
            glEnable(GL_DEPTH_TEST);
        }

        // =========================================================================================
        // Second render pass (Framebuffer texture to quad)
        // =========================================================================================

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        // Make sure that when rendering this quad its not in wireframe mode regardless
        if (IsRenderingInWireframeMode())
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(framebuffer_shader.GetID());
        glBindVertexArray(framebuffer_vao);

        // Attach framebuffer texture as u_ColorAttachment
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer_color);

        glDrawArrays(GL_TRIANGLES, 0, sizeof(framebuffer_quad_vertices) / sizeof(float));

        if (IsRenderingInWireframeMode())
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        window.SwapBuffers();
    }

    glDeleteTextures(1, &framebuffer_color);
    glDeleteRenderbuffers(1, &framebuffer_depth_stencil);
    glDeleteFramebuffers(1, &framebuffer);
    return 0;
}

Actor::Array Actor::CreateActors(Shader& shader)
{
    return {
        // Floor
        Actor{
            .transform =
                {
                    .Position = glm::vec3(0.0f, -3.0f, 0.0f),
                    .Scale    = glm::vec3(20.0f, 20.0f, 1.0f),
                    .Rotation = glm::vec3(glm::radians(90.0f), 0.0f, 0.0f),
                },
            .model = Mesh(ShapeVertexData::GetPlane(),
                          Material{
                              .Shader   = &shader,
                              .Diffuse  = Texture(GetAssetPath("textures/wood-floor/diffuse.png")),
                              .Specular = Texture(GetAssetPath("textures/wood-floor/specular.png")),
                              .TilingFactor = glm::vec2(3.0f),
                              .Shininess    = 10,
                          }),
        },
        // Sphere
        Actor{
            .transform =
                {
                    .Position = glm::vec3(-2.1f, 0.0f, 0.0f),
                    .Rotation = glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
                },
            .model = Mesh(ShapeVertexData::GenerateSphere(20, 20),
                          Material{
                              .Shader    = &shader,
                              .Diffuse   = Texture(GetAssetPath("textures/eyeball.png"), false),
                              .Specular  = glm::vec3(1.0f),
                              .Shininess = 256,
                          }),
            .draw_outline = true,
        },
        // Backpack
        Actor{
            .transform =
                {
                    .Position = glm::vec3(2.0f, 0.0f, 0.0f),
                },
            .model =
                Model(GetAssetPath("models/backpack/backpack.obj"), &shader, ModelLoading_FlipUVs),
        },
        // Grass
        Actor{
            .transform =
                {
                    .Position = glm::vec3(0.0f, -2.5f, 2.0f),
                },
            .model      = Mesh(ShapeVertexData::GetPlane(),
                               Material{
                                   .Shader  = &shader,
                                   .Diffuse = Texture(GetAssetPath("textures/grass.png"),
                                                      false,
                                                      true,
                                                      TextureFilter::Linear,
                                                      TextureFilterWrapping::ClampToEdge),
                               }),
            .cull_faces = false,
        },
        // Windows
        Actor{
            .transform =
                {
                    .Position = glm::vec3(0.0f, 0.0f, 1.0f),
                },
            .model = Mesh(ShapeVertexData::GetPlane(),
                          Material{Material{
                              .Shader  = &shader,
                              .Diffuse = Texture(GetAssetPath("textures/transparent_window.png"),
                                                 false,
                                                 true,
                                                 TextureFilter::Linear,
                                                 TextureFilterWrapping::ClampToEdge),
                          }}),
            .cull_faces = false,
        },
        Actor{
            .transform =
                {
                    .Position = glm::vec3(0.0f, 0.0f, 2.0f),
                },
            .model = Mesh(ShapeVertexData::GetPlane(),
                          Material{Material{
                              .Shader  = &shader,
                              .Diffuse = Texture(GetAssetPath("textures/transparent_window.png"),
                                                 false,
                                                 true,
                                                 TextureFilter::Linear,
                                                 TextureFilterWrapping::ClampToEdge),
                          }}),
            .cull_faces = false,
        },
    };
}

} // namespace LrnGL
