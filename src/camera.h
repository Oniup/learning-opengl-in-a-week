#pragma once

#include <SDL3/SDL_events.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

#include "window.h"

namespace LrnGL {

class Camera
{
public:
    Camera(
        glm::vec3 position, float move_speed, float sensitivity,
        glm::vec3 look_target = glm::vec3(0.0f)
    );

    glm::vec3        GetPosition() const { return m_Position; }
    glm::vec3        GetFoward() const { return m_Forward; }
    glm::vec3        GetUp() const { return m_Up; }
    float            GetFOV() const { return m_FOV; }
    float            GetMoveSpeed() const { return m_MoveSpeed; }
    const glm::mat4& GetProjectionMatrix() const { return m_Projection; }

    void InitializeProjection(const Window& window);
    void UpdateProjectionMatrix(bool is_hiding_mouse, const SDL_Event& event, const Window& window);

    void UpdatePosition(float delta);
    void UpdateLookDirection(bool mouse_is_hidden, const SDL_Event& event, float delta);

    glm::mat4 CreateViewMatrix() const;

private:
    glm::vec3 m_Position  = glm::vec3(0.0);
    glm::vec3 m_Forward   = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_Up        = glm::vec3(0.0f, 1.0f, 0.0f);
    float     m_FOV       = 45.0f;
    float     m_MoveSpeed = 5.0f;

    float m_Sensitivity = 1.0f;
    float m_Pitch       = 0.0f;
    float m_Yaw         = 0.0f;

    glm::mat4 m_Projection;
};

} // namespace LrnGL
