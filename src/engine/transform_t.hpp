#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace eng
{
    struct transform_t
    {
        glm::vec3 translation{0.0f};
        glm::vec3 scale{0.5f};
        glm::quat rotation{0.0f, 0.0f, 0.0f, 1.0f};

        void applyRotation(glm::vec3 eulerDegreesDelta)
        {
            glm::vec3 radians = glm::radians(eulerDegreesDelta);
            glm::quat deltaRotation = glm::quat(radians);

            rotation = glm::normalize(deltaRotation * rotation);
        }

        void applyRotation(glm::quat deltaRotation)
        {
            rotation = glm::normalize(deltaRotation * rotation);
        }

        void rotateEuler(glm::vec3 angles)
        {
            rotation = glm::normalize(glm::quat(glm::radians(angles)));
        }

        glm::vec3 getRotationEuler() const { return glm::eulerAngles(rotation); }

        glm::mat4 mat4()
        {
            glm::mat4 matrix(1.0f);

            glm::mat4 scaleMatrix = glm::scale(matrix, scale);
            glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
            glm::mat4 translationMatrix = glm::translate(matrix, translation);

            return translationMatrix * rotationMatrix * scaleMatrix;
        }
    };

} // namespace eng
