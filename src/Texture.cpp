#include "Texture.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <glad/gl.h>

#include "Error.h"

namespace lgl {

Texture Texture::Invalid{.ID = 0};

Texture Texture::Load(std::string_view path)
{
    SDL_Surface* loaded_surface = IMG_Load(path.data());
    ASSERT(loaded_surface, "Failed to load image from '{}'", path);

    SDL_Surface* surface = SDL_ConvertSurface(loaded_surface, SDL_PIXELFORMAT_RGBA32);
    ASSERT(surface, "Failed to convert image to OpenGL friendly surface");

    unsigned texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 surface->w,
                 surface->h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    Texture result = {
        .ID     = texture,
        .Width  = surface->w,
        .Height = surface->h,
    };

    SDL_DestroySurface(loaded_surface);
    SDL_DestroySurface(surface);
    glBindTexture(GL_TEXTURE_2D, 0);

    return result;
}

void Texture::Shutdown()
{
    if (ID != 0)
    {
        glDeleteTextures(1, &ID);
        ID     = 0;
        Width  = 0;
        Height = 0;
    }
}

} // namespace lgl
