#include "utilities.h"

#include <SDL3/SDL_timer.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <glad/gl.h>
#include <imgui.h>

#include <string>

#include "camera.h"
#include "window.h"

namespace LrnGL {

bool        render_wireframe_mode = false;
bool        mouse_hidden          = false;
std::string asset_directory;

std::string_view GetOpenGLErrorCodeAsString(unsigned error)
{
    switch (error)
    {
    case GL_NO_ERROR:                      return "GL_NO_ERROR";
    case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:                return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:               return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default:                               return "GL_UNKNOWN_ERROR";
    }
}

void SetAssetDirectory(std::string_view directory)
{
    asset_directory = std::string(directory);
}

std::string GetAssetPath(std::string_view path)
{
    return fmt::format("{}/{}", asset_directory, path);
}

std::string_view GetAssetPath(char* buffer, size_t buffer_length, std::string_view path)
{
    auto result = fmt::format_to_n(buffer, buffer_length, "{}/{}", asset_directory, path);
    *result.out = '\0';
    return std::string_view(buffer, result.size);
}

glm::vec3 RGBToNormalized(unsigned r, unsigned g, unsigned b)
{
    return glm::vec3((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f);
}

void CommonEventHandles(const SDL_Event& event, Window& window, Camera& camera, float delta)
{
    window.HandleEvents(event);

    camera.UpdateLookDirection(mouse_hidden, event, delta);
    camera.UpdateProjectionMatrix(mouse_hidden, event, window);

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
        case SDLK_F1:     ToggleRenderWireframeMode(); break;
        case SDLK_F2:     camera.SetPosition(glm::vec3(0.0f, 0.0f, -7.0f)); break;
        case SDLK_ESCAPE: ToggleCursorHiddenMode(window); break;
        }
    }
}

bool IsMouseHidden()
{
    return mouse_hidden;
}

bool IsRenderWireframeMode()
{
    return render_wireframe_mode;
}

void ToggleCursorHiddenMode(Window& window)
{
    mouse_hidden = !mouse_hidden;
    SDL_SetWindowRelativeMouseMode(window.GetContext(), mouse_hidden);

    ImGuiIO& io = ImGui::GetIO();
    if (mouse_hidden)
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    else
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void ToggleRenderWireframeMode()
{
    render_wireframe_mode = !render_wireframe_mode;
    glPolygonMode(GL_FRONT_AND_BACK, render_wireframe_mode ? GL_LINE : GL_FILL);
}

bool IsRenderingInWireframeMode()
{
    return render_wireframe_mode;
}

void CalculateDeltaTime(float& elapsed_time, float& delta)
{
    static float last_time = 0.0f;

    uint64_t current_time = SDL_GetPerformanceCounter();
    delta         = (float)(current_time - last_time) / (float)SDL_GetPerformanceFrequency();
    last_time     = current_time;
    elapsed_time += delta;
}

} // namespace LrnGL
