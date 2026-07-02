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
#include "skybox.h"
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
constexpr float framebuffer_quad_vertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};
// clang-format on
constexpr unsigned framebuffer_quad_vertices_count = 6;

constexpr const char* FramebufferPostProcessingModeNames[] = {
    "Default",
    "Invert",
    "Averaged Gray Scale",
    "Weighted Gray Scale",
    "Kernel",
};

constexpr const char* FramebufferPostProcessingKernelModeNames[] = {
    "Sharpen",
    "Blur",
    "Edge Detection",
};

struct AdvancedOpenGL
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
        bool      Disable     = true;
        bool      DrawOutline = true;
        bool      IgnoreDepth = false;
        float     ScaleFactor = 0.05f;
        glm::vec4 Color       = glm::vec4(0.667f, 0.267f, 0.122f, 1.000f);
    } Stencil;

    struct
    {
        unsigned ID;
        unsigned ColorTextureId;
        unsigned DepthStencilRenderBuffer;
        int      Mode        = 0;
        int      KernelMode  = 0;
        int      SampleCount = 4;

        struct
        {
            unsigned ID;
            unsigned ColorTextureId;
            unsigned VertexArray;
            unsigned VertexBuffer;
        } Intermediate;
    } Framebuffer;

    bool RenderSkyBoxFirst = true;

    ~AdvancedOpenGL()
    {
        glDeleteFramebuffers(1, &Framebuffer.ID);
        glDeleteTextures(1, &Framebuffer.ColorTextureId);
        glDeleteRenderbuffers(1, &Framebuffer.DepthStencilRenderBuffer);

        glDeleteFramebuffers(1, &Framebuffer.Intermediate.ID);
        glDeleteTextures(1, &Framebuffer.Intermediate.ColorTextureId);
        glDeleteVertexArrays(1, &Framebuffer.Intermediate.VertexArray);
        glDeleteBuffers(1, &Framebuffer.Intermediate.VertexBuffer);
    }

    void PushInfoToShader(Shader& shader, float near, float far)
    {
        shader.Uniform("u_Near", near);
        shader.Uniform("u_Far", far);
        shader.Uniform("u_FogDensity", Fog.Density);
        shader.Uniform("u_FogColor", Fog.Color);
        shader.Uniform("u_ViewDepthBuffer", Fog.ViewDepthBuffer);
        shader.Uniform("u_SquareFog", Fog.SquareFog);
    }

    void Edit(const Window& window)
    {
        ImGui::Begin("Fog and SkyBox");
        ImGui::Checkbox("View depth buffer", &Fog.ViewDepthBuffer);
        ImGui::Checkbox("Square density", &Fog.SquareFog);
        ImGui::Checkbox("Render sky box first", &RenderSkyBoxFirst);
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

        ImGui::Begin("Framebuffer");
        ImGui::Combo("Post processing",
                     &Framebuffer.Mode,
                     FramebufferPostProcessingModeNames,
                     IM_ARRAYSIZE(FramebufferPostProcessingModeNames));

        int max_samples;
        glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
        int sample_count = Framebuffer.SampleCount;
        ImGui::SliderInt("Sample count", &sample_count, 1, max_samples);
        if (sample_count != Framebuffer.SampleCount)
            UpdateMultiSampleCount(sample_count, window);

        if (Framebuffer.Mode == 4) // Kernel mode
        {
            ImGui::Combo("Kernel Mode",
                         &Framebuffer.KernelMode,
                         FramebufferPostProcessingKernelModeNames,
                         IM_ARRAYSIZE(FramebufferPostProcessingKernelModeNames));
        }
        ImGui::End();
    }

    void CreateFramebuffer(const Window& window)
    {
        // Create framebuffer
        // -----------------------------------------------------------------------------------------
        glGenFramebuffers(1, &Framebuffer.ID);
        glGenTextures(1, &Framebuffer.ColorTextureId);
        glGenRenderbuffers(1, &Framebuffer.DepthStencilRenderBuffer);

        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer.ID);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Framebuffer.ColorTextureId);
        glBindRenderbuffer(GL_RENDERBUFFER, Framebuffer.DepthStencilRenderBuffer);

        // Color texture attachment
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                                Framebuffer.SampleCount,
                                GL_RGB,
                                window.GetWidth(),
                                window.GetHeight(),
                                GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D_MULTISAMPLE,
                               Framebuffer.ColorTextureId,
                               0);

        // Depth and stencil render buffer object attachment
        glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                         Framebuffer.SampleCount,
                                         GL_DEPTH24_STENCIL8,
                                         window.GetWidth(),
                                         window.GetHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER,
                                  Framebuffer.DepthStencilRenderBuffer);

        // framebuffer creation complete
        ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
               "Failed to create MSAA enabled framebuffer");

        // Create Intermediate framebuffer (Only need color attachment)
        // -----------------------------------------------------------------------------------------
        glGenFramebuffers(1, &Framebuffer.Intermediate.ID);
        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer.Intermediate.ID);

        glGenTextures(1, &Framebuffer.Intermediate.ColorTextureId);
        glBindTexture(GL_TEXTURE_2D, Framebuffer.Intermediate.ColorTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               Framebuffer.Intermediate.ColorTextureId,
                               0);

        ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
               "Failed to create intermediate framebuffer");

        // framebuffer quad data
        glGenVertexArrays(1, &Framebuffer.Intermediate.VertexArray);
        glGenBuffers(1, &Framebuffer.Intermediate.VertexBuffer);
        glBindVertexArray(Framebuffer.Intermediate.VertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, Framebuffer.Intermediate.VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(framebuffer_quad_vertices),
                     framebuffer_quad_vertices,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }

    void UpdateMultiSampleCount(int count, const Window& window)
    {
        Framebuffer.SampleCount = count;

        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer.ID);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Framebuffer.ColorTextureId);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                                Framebuffer.SampleCount,
                                GL_RGB,
                                window.GetWidth(),
                                window.GetHeight(),
                                GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D_MULTISAMPLE,
                               Framebuffer.ColorTextureId,
                               0);

        glBindRenderbuffer(GL_RENDERBUFFER, Framebuffer.DepthStencilRenderBuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                         Framebuffer.SampleCount,
                                         GL_DEPTH24_STENCIL8,
                                         window.GetWidth(),
                                         window.GetHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER,
                                  Framebuffer.DepthStencilRenderBuffer);
    }

    void RenderToMainFramebuffer(Shader& shader, Window& window)
    {
        // glBindFramebuffer(GL_READ_FRAMEBUFFER, Framebuffer.ID);
        // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // glBlitFramebuffer(0,
        //                   0,
        //                   window.GetWidth(),
        //                   window.GetHeight(),
        //                   0,
        //                   0,
        //                   window.GetWidth(),
        //                   window.GetHeight(),
        //                   GL_COLOR_BUFFER_BIT,
        //                   GL_NEAREST);

        // Blitz framebuffer color data to intermediate framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, Framebuffer.ID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Framebuffer.Intermediate.ID);
        glBlitFramebuffer(0,
                          0,
                          window.GetWidth(),
                          window.GetHeight(),
                          0,
                          0,
                          window.GetWidth(),
                          window.GetHeight(),
                          GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        unsigned error = glGetError();
        ASSERT(error == GL_NO_ERROR,
               "Failed to blitz framebuffer with error code {} '{}'",
               error,
               GetOpenGLErrorCodeAsString(error));

        // Draw blitz framebuffer color attachment to quad and render to main framebuffer
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Post processing effect options
        shader.Uniform("u_PostProcessingMode", Framebuffer.Mode);
        shader.Uniform("u_PostProcessingKernelMode", Framebuffer.KernelMode);

        // Make sure that when rendering this quad its not in wireframe mode regardless
        if (IsRenderingInWireframeMode())
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glUseProgram(shader.GetID());
        glBindVertexArray(Framebuffer.Intermediate.VertexArray);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Framebuffer.Intermediate.ColorTextureId);

        glDrawArrays(GL_TRIANGLES, 0, framebuffer_quad_vertices_count);

        if (IsRenderingInWireframeMode())
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
    }

    void ResizeFramebuffer(Window& window)
    {
        // MSAA framebuffer enabled
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Framebuffer.ColorTextureId);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                                Framebuffer.SampleCount,
                                GL_RGB,
                                window.GetWidth(),
                                window.GetHeight(),
                                GL_TRUE);

        glBindRenderbuffer(GL_RENDERBUFFER, Framebuffer.DepthStencilRenderBuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                         Framebuffer.SampleCount,
                                         GL_DEPTH24_STENCIL8,
                                         window.GetWidth(),
                                         window.GetHeight());

        // Intermediate
        glBindTexture(GL_TEXTURE_2D, Framebuffer.Intermediate.ColorTextureId);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     window.GetWidth(),
                     window.GetHeight(),
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     nullptr);
    }
};

int AdvancedOpenGLMain(Window& window, int argc, const char** argv)
{
    // Phong Shader with Fog
    Shader phong_shader(GetAssetPath("shaders/advanced_phong.frag"),
                        GetAssetPath("shaders/phong.vert"));
    Shader outline_shader(GetAssetPath("shaders/flat_color.frag"),
                          GetAssetPath("shaders/basic.vert"));

    Shader framebuffer_shader(GetAssetPath("shaders/framebuffer.frag"),
                              GetAssetPath("shaders/framebuffer.vert"));
    framebuffer_shader.Uniform("u_ColorAttachment", 0);

    InitializeMaterialTextureUniforms(phong_shader);

    LightManager light_manager(true);
    light_manager.PushLight(
        LightData{
            .Type      = LightData::Directional,
            .Direction = glm::vec3(0.8f, -0.6f, 1.0f),
        },
        false);
    light_manager.PushLight(LightData{
        .Position = glm::vec3(3.0f, 3.0f, 3.0f),
        .Color    = RGBToNormalized(215, 188, 133),
    });

    Camera camera(glm::vec3(-7.0f, 0.0f, 7.0f), 5.0f, 0.01f);
    camera.InitializeProjection(window);

    Actor::Array   actors = Actor::CreateActors(phong_shader);
    AdvancedOpenGL adv;
    adv.CreateFramebuffer(window);

    SkyBox skybox(
        {
            GetAssetPath("textures/skybox/right.jpg"),
            GetAssetPath("textures/skybox/left.jpg"),
            GetAssetPath("textures/skybox/top.jpg"),
            GetAssetPath("textures/skybox/bottom.jpg"),
            GetAssetPath("textures/skybox/front.jpg"),
            GetAssetPath("textures/skybox/back.jpg"),
        },
        false,
        false);
    adv.Fog.Color = skybox.GetAverageColor();
    light_manager.SetGlobalAmbientLight(skybox.GetAverageColor());
    unsigned skybox_texture_index = 3;
    skybox.InitializeTextureUniform(phong_shader, skybox_texture_index);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
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
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
                adv.ResizeFramebuffer(window);
        }
        camera.UpdatePosition(delta);

        glm::mat4 view = camera.CreateViewMatrix();

        // First render pass (Rendering scene)
        glBindFramebuffer(GL_FRAMEBUFFER, adv.Framebuffer.ID);

        glStencilMask(0xFF);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilMask(0x00);

        if (adv.RenderSkyBoxFirst)
            skybox.Draw(camera.GetProjectionMatrix(), view);
        skybox.PushInfoToShader(phong_shader, skybox_texture_index);

        light_manager.EditLightPropertiesMenu();
        light_manager.DrawDebugInfo(camera.GetProjectionMatrix(), view);
        light_manager.PushLightInfoToUniformBlockLayout(phong_shader, camera.GetPosition());

        adv.Edit(window);
        adv.PushInfoToShader(phong_shader, camera.GetNearPlane(), camera.GetFarPlane());

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

        // Draw sky box last so that we can optimise the number of fragments being drawn
        if (!adv.RenderSkyBoxFirst)
            skybox.Draw(camera.GetProjectionMatrix(), view);

        // Draw outline
        if (!adv.Stencil.Disable)
        {
            outline_shader.Uniform("u_FlatColor", adv.Stencil.Color);
            int type = adv.Stencil.DrawOutline ? GL_NOTEQUAL : GL_ALWAYS;

            if (adv.Stencil.IgnoreDepth)
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
                actor->transform.Scale += adv.Stencil.ScaleFactor;
                actor->Draw(elapsed_time, camera.GetProjectionMatrix(), view);
                actor->transform.Scale -= adv.Stencil.ScaleFactor;
                actor->model.SetShaderToAllMaterials(&phong_shader);
            }
            glEnable(GL_DEPTH_TEST);
        }

        // Second render pass to framebuffer
        adv.RenderToMainFramebuffer(framebuffer_shader, window);
        window.SwapBuffers();
    }
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
                              .TilingFactor     = glm::vec2(3.0f),
                              .Shininess        = 10,
                              .ReflectionFactor = 0.0f,
                          }),
            // .cull_faces = false,
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
                                   .Shader           = &shader,
                                   .Diffuse          = Texture(GetAssetPath("textures/grass.png"),
                                                               false,
                                                               true,
                                                               TextureFilter::Linear,
                                                               TextureFilterWrapping::ClampToEdge),
                                   .ReflectionFactor = 0.0f,
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
