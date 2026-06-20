#include "Transform.h"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>

namespace lgl {

glm::mat4 Transform::GetModelMatrix() const
{
    glm::mat4 model    = glm::mat4(1.0f);
    glm::mat4 rotation = glm::mat4_cast(Rotation);

    model = glm::translate(model, Position);
    model = glm::scale(model * rotation, Scale);
    return model;
}

void Transform::RotatePitch(float angle)
{
    if (angle == 0.0f)
        return;
    Rotate(angle, glm::vec3(1.0f, 0.0f, 0.0f));
}

void Transform::RotateYaw(float angle)
{
    if (angle == 0.0f)
        return;
    Rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Transform::RotateRoll(float angle)
{
    if (angle == 0.0f)
        return;
    Rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
}

void Transform::Rotate(float angle, glm::vec3 axis)
{
    glm::quat rotation = glm::angleAxis(angle, axis);
    Rotation           = Rotation * rotation;
    Rotation           = glm::normalize(Rotation);
}

void Transform::Rotate(glm::vec3 angles)
{
    RotatePitch(angles.x);
    RotateYaw(angles.y);
    RotateRoll(angles.z);
}

} // namespace lgl
