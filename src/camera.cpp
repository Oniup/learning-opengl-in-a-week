#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_scancode.h>
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include <algorithm>
#include <cmath>

#include "utilities.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "camera.h"

namespace LrnGL {

Camera::Camera(glm::vec3 position, float move_speed, float sensitivity, glm::vec3 look_target)
    : m_Position(position),
      m_Forward(glm::normalize(look_target - position)),
      m_MoveSpeed(move_speed),
      m_Sensitivity(sensitivity)
{
    float rad_pitch = std::asin(std::clamp(m_Forward.y, -1.0f, 1.0f));
    float rad_yaw   = std::atan2(m_Forward.z, m_Forward.x);

    m_Pitch = glm::degrees(rad_pitch);
    m_Yaw   = glm::degrees(rad_yaw);
}

void Camera::InitializeProjection(const Window& window)
{
    m_Projection = glm::perspective(
        glm::radians(m_FOV), (float)window.GetWidth() / (float)window.GetHeight(), 0.1f, 100.0f);
}

void Camera::UpdateProjectionMatrix(bool is_hiding_mouse, const SDL_Event& event,
                                    const Window& window)
{
    if (event.type == SDL_EVENT_WINDOW_RESIZED)
    {
        InitializeProjection(window);
    }
    else if (is_hiding_mouse && event.type == SDL_EVENT_MOUSE_WHEEL)
    {
        m_FOV -= event.wheel.y;
        InitializeProjection(window);
    }
}

void Camera::UpdatePosition(float delta)
{
    if (!IsMouseHidden())
        return;

    const bool* keyboard = SDL_GetKeyboardState(nullptr);

    glm::vec3 right = glm::cross(m_Forward, m_Up);

    glm::vec3 move_dir = glm::vec3(0.0f);
    if (keyboard[SDL_SCANCODE_W])
        move_dir += m_Forward;
    if (keyboard[SDL_SCANCODE_S])
        move_dir -= m_Forward;
    if (keyboard[SDL_SCANCODE_D])
        move_dir += right;
    if (keyboard[SDL_SCANCODE_A])
        move_dir -= right;

    if (keyboard[SDL_SCANCODE_SPACE])
        move_dir += m_Up;
    if (keyboard[SDL_SCANCODE_LCTRL])
        move_dir -= m_Up;

    if (glm::length2(move_dir) > 0.0f)
    {
        float move_speed = m_MoveSpeed;
        if (keyboard[SDL_SCANCODE_LSHIFT])
            move_speed *= 2;
        move_dir    = glm::normalize(move_dir);
        m_Position += move_dir * move_speed * delta;
    }
}

void Camera::UpdateLookDirection(bool mouse_is_hidden, const SDL_Event& event, float delta)
{
    if (!mouse_is_hidden || event.type != SDL_EVENT_MOUSE_MOTION)
        return;

    m_Yaw   += event.motion.xrel * m_Sensitivity;
    m_Pitch -= event.motion.yrel * m_Sensitivity;
    m_Pitch  = std::clamp(m_Pitch, -89.0f, 89.0f);

    float radYaw   = glm::radians(m_Yaw);
    float radPitch = glm::radians(m_Pitch);

    glm::vec3 direction;
    direction.x = std::cos(radYaw) * std::cos(radPitch);
    direction.y = std::sin(radPitch);
    direction.z = std::sin(radYaw) * std::cos(radPitch);

    m_Forward = glm::normalize(direction);
}

glm::mat4 Camera::CreateViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
}

} // namespace LrnGL
