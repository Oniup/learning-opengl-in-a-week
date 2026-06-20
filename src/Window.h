#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

struct SDL_Window;
typedef struct SDL_GLContextState* SDL_GLContext;

namespace lgl {

class Window
{
public:
    Window(int width, int height);
    ~Window();

    void Shutdown();

    int GetWidth() const;
    int GetHeight() const;
    void GetSize(int& width, int& height) const;
    SDL_Window* GetContext() const { return m_Window; }

    bool IsRunning();
    void HandleEvents(const SDL_Event& event);
    void SwapBuffers();

private:
    void UpdateViewport();

    bool m_Running          = true;
    SDL_Window* m_Window    = nullptr;
    SDL_GLContext m_Context = nullptr;
};

} // namespace lgl
