#pragma once

#include <SDL3/SDL_events.h>
#include <glm/ext/vector_float3.hpp>

#include <string>
#include <string_view>

namespace LrnGL {

class Window;
class Camera;

std::string_view GetOpenGLErrorCodeAsString(unsigned error);

void             SetAssetDirectory(std::string_view directory);
std::string      GetAssetPath(std::string_view path);
std::string_view GetAssetPath(char* buffer, size_t buffer_length, std::string_view path);

glm::vec3 RGBToNormalized(unsigned r, unsigned g, unsigned b);
void      CommonEventHandles(const SDL_Event& event, Window& window, Camera& camera, float delta);

bool IsMouseHidden();
bool IsRenderWireframeMode();
void ToggleCursorHiddenMode(Window& window);
void ToggleRenderWireframeMode();
bool IsRenderingInWireframeMode();

void CalculateDeltaTime(float& elapsed_time, float& delta);

} // namespace LrnGL
