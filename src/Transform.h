#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/vector_float3.hpp>

namespace lgl {

struct Transform
{
    glm::vec3 Position = glm::vec3(0.0f);
    glm::vec3 Scale    = glm::vec3(1.0f);
    glm::quat Rotation = glm::vec3(0.0f);

    glm::mat4 GetModelMatrix() const;

    void RotatePitch(float angle);
    void RotateYaw(float angle);
    void RotateRoll(float angle);
    void Rotate(float angle, glm::vec3 axis);
    void Rotate(glm::vec3 angles);
};

} // namespace lgl
