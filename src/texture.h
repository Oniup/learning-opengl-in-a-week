#pragma once

#include <string_view>

struct SDL_Surface;

namespace LrnGL {

enum class TextureFilter
{
    Linear,
    Nearest,
    NoMipmapLinear,
    NoMipmapNearest,
};

class Texture
{
public:
    static Texture Invalid;

    Texture() = default;
    Texture(std::string_view path, bool srgb_correction = false, bool flip_vertically = true,
            TextureFilter filter = TextureFilter::Linear);
    Texture(const unsigned char* buffer, unsigned buffer_length, bool srgb_correction = false,
            bool flip_vertically = true, TextureFilter filter = TextureFilter::Linear);
    ~Texture();

    Texture(const Texture& texture);
    Texture(Texture&& texture);

    Texture& operator=(const Texture& texture);
    Texture& operator=(Texture&& texture);

    void Destroy();

    unsigned GetID() const { return m_ID; }
    int      GetWidth() const { return m_Width; }
    int      GetHeight() const { return m_Height; }

private:
    void LoadFromSurface(SDL_Surface* loaded_surface, bool srgb_correction, bool flip_vertically,
                         TextureFilter filter);

    unsigned m_ID     = 0;
    int      m_Width  = 0;
    int      m_Height = 0;
    bool     m_Owns   = true;
};

} // namespace LrnGL
