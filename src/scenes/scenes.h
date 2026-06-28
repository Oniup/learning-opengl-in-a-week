#pragma once

#include <SDL3/SDL_events.h>
#include <glm/ext/vector_float3.hpp>

namespace LrnGL {

class Window;
class Camera;

// Demos
int PhongMain(Window& window, int argc, const char** argv);
int ModelLoadingMain(Window& window, int argc, const char** argv);
int AdvancedOpenGLMain(Window& window, int argc, const char** argv);

} // namespace LrnGL
