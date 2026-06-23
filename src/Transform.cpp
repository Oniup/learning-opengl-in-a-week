#include "transform.h"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>

namespace LrnGL {

glm::mat4 Transform::CreateModelMatrix() const
{
    glm::mat4 model    = glm::mat4(1.0f);
    glm::mat4 rotation = glm::mat4_cast(Rotation);

    model = glm::translate(model, Position);
    model = glm::scale(model * rotation, Scale);
    return model;
}

void Transform::SetEulerPitch(float angle)
{
    Rotation = glm::vec3(angle, Rotation.y, Rotation.z);
}

void Transform::SetEulerYaw(float angle)
{
    Rotation = glm::vec3(Rotation.z, angle, Rotation.z);
}

void Transform::SetEulerRoll(float angle)
{
    Rotation = glm::vec3(Rotation.z, Rotation.y, angle);
}

void Transform::RotatePitch(float angle, bool normalize)
{
    if (angle == 0.0f)
        return;
    Rotate(angle, glm::vec3(1.0f, 0.0f, 0.0f), normalize);
}

void Transform::RotateYaw(float angle, bool normalize)
{
    if (angle == 0.0f)
        return;
    Rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f), normalize);
}

void Transform::RotateRoll(float angle, bool normalize)
{
    if (angle == 0.0f)
        return;
    Rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f), normalize);
}

void Transform::Rotate(float angle, glm::vec3 axis, bool normalize)
{
    glm::quat rotation = glm::angleAxis(angle, axis);
    Rotation           = Rotation * rotation;
    if (normalize)
        Rotation = glm::normalize(Rotation);
}

void Transform::Rotate(glm::vec3 angles, bool normalize)
{
    RotatePitch(angles.x, false);
    RotateYaw(angles.y, false);
    RotateRoll(angles.z, false);

    if (normalize)
        Rotation = glm::normalize(Rotation);
}

} // namespace LrnGL
