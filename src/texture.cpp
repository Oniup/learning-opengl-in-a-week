#include "texture.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <glad/gl.h>

#include "error.h"

namespace LrnGL {

Texture Texture::Invalid;

Texture::Texture(std::string_view path, TextureFilter filter)
{
    SDL_Surface* loaded_surface = IMG_Load(path.data());
    ASSERT(loaded_surface, "Failed to load image from '{}'", path);
    SDL_FlipSurface(loaded_surface, SDL_FLIP_VERTICAL);

    SDL_Surface* surface = SDL_ConvertSurface(loaded_surface, SDL_PIXELFORMAT_RGBA32);
    ASSERT(surface, "Failed to convert image to OpenGL friendly surface");

    m_Width  = surface->w;
    m_Height = surface->h;

    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    int min_filter;
    int mag_filter;
    switch (filter)
    {
    case TextureFilter::Linear:
        min_filter = GL_LINEAR_MIPMAP_LINEAR;
        mag_filter = GL_LINEAR;
        break;
    case TextureFilter::Nearest:
        min_filter = GL_NEAREST_MIPMAP_NEAREST;
        mag_filter = GL_NEAREST;
        break;
    case TextureFilter::LinearNoMipmap:
        min_filter = GL_LINEAR;
        mag_filter = GL_LINEAR;
        break;
    case TextureFilter::NearestNoMipmap:
        min_filter = GL_NEAREST;
        mag_filter = GL_NEAREST;
        break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 m_Width,
                 m_Height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 surface->pixels);

    if (filter <= TextureFilter::Nearest)
        glGenerateMipmap(GL_TEXTURE_2D);

    SDL_DestroySurface(loaded_surface);
    SDL_DestroySurface(surface);
}

Texture::~Texture()
{
    Destroy();
}

Texture::Texture(const Texture& texture)
    : m_ID(texture.m_ID),
      m_Width(texture.m_Width),
      m_Height(texture.m_Height),
      m_Owns(false)
{
}

Texture::Texture(Texture&& texture)
    : m_ID(texture.m_ID),
      m_Width(texture.m_Width),
      m_Height(texture.m_Height),
      m_Owns(texture.m_Owns)
{
    texture.m_ID = 0;
}

Texture& Texture::operator=(const Texture& texture)
{
    Destroy();
    m_ID     = texture.m_ID;
    m_Width  = texture.m_Width;
    m_Height = texture.m_Height;
    m_Owns   = false;

    return *this;
}

Texture& Texture::operator=(Texture&& texture)
{
    Destroy();
    m_ID     = texture.m_ID;
    m_Width  = texture.m_Width;
    m_Height = texture.m_Height;
    m_Owns   = texture.m_Owns;

    texture.m_ID = 0;
    return *this;
}

void Texture::Destroy()
{
    if (m_ID != 0 && m_Owns)
    {
        glDeleteTextures(1, &m_ID);
        m_ID     = 0;
        m_Width  = 0;
        m_Height = 0;
    }
}

} // namespace LrnGL
