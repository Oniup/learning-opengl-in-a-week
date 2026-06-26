#include "utilities.h"

#include <SDL3/SDL_timer.h>
#include <glad/gl.h>
#include <imgui.h>

#include "camera.h"
#include "window.h"

namespace LrnGL {

bool RenderWireframeMode = false;
bool MouseHidden         = false;

glm::vec3 RGBToNormalized(unsigned r, unsigned g, unsigned b)
{
    return glm::vec3((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f);
}

void CommonEventHandles(const SDL_Event& event, Window& window, Camera& camera, float delta)
{
    window.HandleEvents(event);

    camera.UpdateLookDirection(MouseHidden, event, delta);
    camera.UpdateProjectionMatrix(MouseHidden, event, window);

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
        case SDLK_F1:     ToggleRenderWireframeMode(); break;
        case SDLK_ESCAPE: ToggleCursorHiddenMode(window); break;
        }
    }
}

bool IsMouseHidden()
{
    return MouseHidden;
}

bool IsRenderWireframeMode()
{
    return RenderWireframeMode;
}

void ToggleCursorHiddenMode(Window& window)
{
    MouseHidden = !MouseHidden;
    SDL_SetWindowRelativeMouseMode(window.GetContext(), MouseHidden);

    ImGuiIO& io = ImGui::GetIO();
    if (MouseHidden)
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    else
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void ToggleRenderWireframeMode()
{
    RenderWireframeMode = !RenderWireframeMode;
    glPolygonMode(GL_FRONT_AND_BACK, RenderWireframeMode ? GL_LINE : GL_FILL);
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
