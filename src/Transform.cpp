#include "Transform.h"

#include <glm/ext.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace lgl {

glm::mat4 Transform::GetModelMatrix() const
{
    glm::mat4 model    = glm::mat4(1.0f);
    glm::mat4 rotation = glm::mat4_cast(Rotation);

    model = glm::translate(model, Position);
    model = glm::scale(model * rotation, Scale);
    return model;
}

} // namespace lgl
