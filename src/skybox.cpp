#include "skybox.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <glad/gl.h>

#include <array>

#include "error.h"
#include "utilities.h"

namespace LrnGL {

// clang-format off
static constexpr float vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
// clang-format on

static constexpr unsigned vertices_count = sizeof(vertices) / sizeof(float);

SkyBox::SkyBox(const std::array<std::string_view, 6>& paths, bool srgb_correction,
               bool flip_vertically)
    : m_Shader(GetAssetPath("shaders/skybox.frag"), GetAssetPath("shaders/skybox.vert"))
{
    std::array<SDL_Surface*, 6> surfaces;
    for (unsigned i = 0; i < paths.size(); i++)
    {
        std::string_view path = paths[i];
        ASSERT_STRING_VIEW_NULL_TERMINATED(path);

        SDL_Surface* surface = IMG_Load(path.data());
        ASSERT(surface, "Failed to load surface from path {}", path);
        if (flip_vertically)
            SDL_FlipSurface(surface, SDL_FLIP_VERTICAL);

        surfaces[i] = surface;
    }

    LoadTextures(surfaces, srgb_correction);

    for (SDL_Surface* surface : surfaces)
        SDL_DestroySurface(surface);

    InitializeVertexArray();
}

SkyBox::~SkyBox()
{
    if (m_Texture != 0)
    {
        glDeleteTextures(1, &m_Texture);
        m_Texture = 0;
    }
}

void SkyBox::Draw(const glm::mat4& projection, const glm::mat4& view)
{
    glDepthFunc(GL_LEQUAL);

    m_Shader.Uniform("u_Projection", projection);
    m_Shader.Uniform("u_View", glm::mat4(glm::mat3(view))); // Remove camera translation

    glUseProgram(m_Shader.GetID());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);

    glBindVertexArray(m_VertexArray);
    glDrawArrays(GL_TRIANGLES, 0, vertices_count);

    glDepthFunc(GL_LESS);
}

void SkyBox::PushInfoToShader(Shader& shader, unsigned texture_index)
{
    glUseProgram(shader.GetID());
    glActiveTexture(GL_TEXTURE0 + texture_index);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);
}

void SkyBox::InitializeTextureUniform(Shader& shader, unsigned texture_index)
{
    shader.Uniform("u_SkyBox", texture_index);
}

void SkyBox::LoadTextures(const std::array<SDL_Surface*, 6>& surfaces, bool srgb_correction)
{
    glGenTextures(1, &m_Texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture);

    for (unsigned i = 0; i < surfaces.size(); i++)
    {
        SDL_Surface* surface = SDL_ConvertSurface(surfaces[i], SDL_PIXELFORMAT_RGBA32);
        ASSERT(surface, "Failed to convert surface to RGBA32");

        if (i == 0)
            CalculateAverageColor(surface);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     srgb_correction ? GL_SRGB_ALPHA : GL_RGBA,
                     surface->w,
                     surface->h,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     surface->pixels);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        unsigned error = glGetError();
        ASSERT(error == GL_NO_ERROR,
               "Failed to create texture: error code {} '{}'",
               error,
               GetOpenGLErrorCodeAsString(error));

        SDL_DestroySurface(surface);
    }
}

void SkyBox::CalculateAverageColor(SDL_Surface* surface)
{
    uint64_t total_r     = 0;
    uint64_t total_g     = 0;
    uint64_t total_b     = 0;
    unsigned pixel_count = surface->w * surface->h;

    const uint32_t*               pixels         = static_cast<uint32_t*>(surface->pixels);
    const SDL_PixelFormatDetails* format_details = SDL_GetPixelFormatDetails(surface->format);
    for (unsigned i = 0; i < pixel_count; i++)
    {
        uint8_t r, g, b, a;
        SDL_GetRGBA(pixels[i], format_details, nullptr, &r, &g, &b, &a);

        total_r += r;
        total_g += g;
        total_b += b;
    }

    m_AverageColor = glm::vec3{
        total_r / pixel_count,
        total_g / pixel_count,
        total_b / pixel_count,
    };
    m_AverageColor /= 255;
}

void SkyBox::InitializeVertexArray()
{
    glGenVertexArrays(1, &m_VertexArray);
    glGenBuffers(1, &m_VertexBuffer);

    glBindVertexArray(m_VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

} // namespace LrnGL
