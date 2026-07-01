#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

#include <array>
#include <string_view>

#include "shader.h"

struct SDL_Surface;

namespace LrnGL {

class SkyBox
{
public:
    SkyBox(const std::array<std::string_view, 6>& paths, bool srgb_correction = false,
           bool flip_vertically = true);
    ~SkyBox();

    glm::vec3 GetAverageColor() const { return m_AverageColor; }

    void Draw(const glm::mat4& projection, const glm::mat4& view);

private:
    void LoadTextures(const std::array<SDL_Surface*, 6>& surfaces, bool srgb_correction);
    void CalculateAverageColor(SDL_Surface* surface);
    void InitializeVertexArray();

    unsigned  m_Texture;
    unsigned  m_VertexArray;
    unsigned  m_VertexBuffer;
    glm::vec3 m_AverageColor;
    Shader    m_Shader;
};

} // namespace LrnGL
