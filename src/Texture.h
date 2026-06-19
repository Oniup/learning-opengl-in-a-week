#pragma once

#include <SDL3/SDL_pixels.h>

#include <string_view>

namespace lgl {

struct Texture
{
    static Texture Invalid;

    unsigned ID = 0;
    int Width   = 0;
    int Height  = 0;

    static Texture Load(std::string_view path);
    void Shutdown();
};

} // namespace lgl
