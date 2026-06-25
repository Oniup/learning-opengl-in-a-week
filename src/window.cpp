#include "window.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <glad/gl.h>
#include <imgui.h>

#include <string_view>

#include "error.h"

namespace LrnGL {

static std::string_view GetSDLError()
{
    constexpr size_t buffer_length = 1024;
    static char      buffer[buffer_length];

    auto result = fmt::format_to_n(buffer, buffer_length - 1, "{}", SDL_GetError());
    *result.out = '\0';
    return std::string_view(buffer, result.size);
}

Window::Window(int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        FATAL("Failed to initialize SDL: {}", SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_DisplayID          display_id   = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* display_mode = SDL_GetCurrentDisplayMode(display_id);

    if (width == 0)
    {
        width  = display_mode->w - display_mode->w / 4;
        height = display_mode->h - display_mode->h / 4;
    }

    m_Window = SDL_CreateWindow(
        "Learning OpenGL", width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_Window)
    {
        std::string_view error = GetSDLError();
        Shutdown();
        FATAL("Failed to create SDL window: {}", error);
    }

    m_Context = SDL_GL_CreateContext(m_Window);
    if (!m_Context)
    {
        std::string_view error = GetSDLError();
        Shutdown();
        FATAL("Failed to create SDL OpenGL Context: {}", error);
    }

    bool glad_version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    if (glad_version == 0)
    {
        Shutdown();
        FATAL("Failed to initialize OpenGL using glad");
    }

    fmt::print("Loaded OpenGL {}.{}\n",
               GLAD_VERSION_MAJOR(glad_version),
               GLAD_VERSION_MINOR(glad_version));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io     = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplSDL3_InitForOpenGL(m_Window, m_Context);
    ImGui_ImplOpenGL3_Init();
}

Window::~Window()
{
    Shutdown();
}

void Window::Shutdown()
{
    if (m_Window)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        if (m_Window)
            SDL_DestroyWindow(m_Window);
        if (m_Context)
            SDL_GL_DestroyContext(m_Context);

        SDL_Quit();

        m_Window  = nullptr;
        m_Context = nullptr;
        m_Running = false;
    }
}

int Window::GetWidth() const
{
    int width;
    SDL_GetWindowSize(m_Window, &width, nullptr);
    return width;
}

int Window::GetHeight() const
{
    int height;
    SDL_GetWindowSize(m_Window, nullptr, &height);
    return height;
}

void Window::GetSize(int& width, int& height) const
{
    SDL_GetWindowSize(m_Window, &width, &height);
}

bool Window::IsRunning()
{
    if (m_Running)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        return true;
    }
    return false;
}

void Window::HandleEvents(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_QUIT:           m_Running = false;
    case SDL_EVENT_WINDOW_RESIZED: UpdateViewport();
    }

    ImGui_ImplSDL3_ProcessEvent(&event);
}

void Window::SwapBuffers()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(m_Window);
}

void Window::UpdateViewport()
{
    int width, height;
    SDL_GetWindowSize(m_Window, &width, &height);

    glViewport(0, 0, width, height);
}

} // namespace LrnGL
