#pragma once

#include <SDL3/SDL_events.h>
#include <glm/ext/vector_float3.hpp>

#include <string>

namespace LrnGL {

class Window;
class Camera;

// Demos
int PhongMain(const std::string& asset_dir, Window& window, int argc, const char** argv);

// Utility
glm::vec3 RGBToNormalized(unsigned r, unsigned g, unsigned b);
void      CommonEventHandles(const SDL_Event& event, Window& window, Camera& camera, float delta);

bool IsMouseHidden();
bool IsRenderWireframeMode();
void ToggleCursorHiddenMode(Window& window);
void ToggleRenderWireframeMode();

void CalculateDeltaTime(float& elapsed, float& delta);

} // namespace LrnGL
