#pragma once

#include <SDL3/SDL_events.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

namespace lgl {

class Camera
{
public:
    Camera(glm::vec3 position, float move_speed, float sensitivity,
           glm::vec3 look_target = glm::vec3(0.0f));

    void UpdatePosition(float delta);
    void UpdateLookDirection(const SDL_Event& event, float delta);

    glm::mat4 GetViewMatrix() const;

private:
    glm::vec3 m_Position = glm::vec3(0.0);
    glm::vec3 m_Forward  = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_Up       = glm::vec3(0.0f, 1.0f, 0.0f);
    float m_MoveSpeed    = 5.0f;

    float m_Sensitivity = 1.0f;
    float m_Pitch       = 0.0f;
    float m_Yaw         = 0.0f;
};

} // namespace lgl
