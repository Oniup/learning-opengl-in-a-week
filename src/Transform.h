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

    glm::mat4 CreateModelMatrix() const;

    void SetEulerPitch(float angle);
    void SetEulerYaw(float angle);
    void SetEulerRoll(float angle);

    void RotatePitch(float angle, bool normalize = true);
    void RotateYaw(float angle, bool normalize = true);
    void RotateRoll(float angle, bool normalize = true);
    void Rotate(float angle, glm::vec3 axis, bool normalize = true);
    void Rotate(glm::vec3 angles, bool normalize = true);
};

} // namespace lgl
