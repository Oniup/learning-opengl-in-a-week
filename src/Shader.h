#pragma once

#include <filesystem>
#include <limits>

namespace LrnGL {

struct Shader
{
    unsigned ID = 0;

    static Shader Load(const std::filesystem::path& fragment, const std::filesystem::path& vertex);
    void Shutdown();
};

static constexpr Shader InvalidShader{.ID = std::numeric_limits<unsigned>::max()};

} // namespace LrnGL
