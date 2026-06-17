#include "SDL3/SDL_events.h"
#include "Window.h"
#include "glad/gl.h"

#include <fmt/format.h>

int main(int argc, char** argv)
{
    LrnGL::Window window(800, 800);

    SDL_Event event;
    while (window.IsRunning())
    {
        while (SDL_PollEvent(&event))
        {
            window.HandleEvents(event);
        }

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.SwapBuffers();
    }

    return 0;
}
