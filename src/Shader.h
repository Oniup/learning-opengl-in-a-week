#pragma once

#include <string_view>

namespace lgl {

struct Shader
{
    static Shader InvalidShader;

    unsigned ID = 0;

    static Shader Load(std::string_view fragment, std::string_view vertex);
    void Shutdown();
};

} // namespace lgl
