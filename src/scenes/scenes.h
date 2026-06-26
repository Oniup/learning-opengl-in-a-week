#pragma once

#include <SDL3/SDL_events.h>
#include <glm/ext/vector_float3.hpp>

#include <string>

namespace LrnGL {

class Window;
class Camera;

// Demos
int PhongMain(const std::string& asset_dir, Window& window, int argc, const char** argv);
int ModelLoadingMain(const std::string& asset_dir, Window& window, int argc, const char** argv);

} // namespace LrnGL
