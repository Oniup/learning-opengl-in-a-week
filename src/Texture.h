#pragma once

#include <SDL3/SDL_pixels.h>

#include <string_view>

namespace lgl {

enum class TextureFilter
{
    Linear,
    Nearest,
    LinearNoMipmap,
    NearestNoMipmap,
};

class Texture
{
public:
    static Texture Invalid;

    Texture() = default;
    Texture(std::string_view path, TextureFilter filter = TextureFilter::Linear);
    ~Texture();

    void Shutdown();

    unsigned GetID() const { return m_ID; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

private:
    unsigned m_ID = 0;
    int m_Width   = 0;
    int m_Height  = 0;
};

} // namespace lgl
