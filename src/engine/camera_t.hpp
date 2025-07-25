#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace eng
{
    class camera_t
    {
    public:
        camera_t(float near = 0.1f, float far = 100.f);
        ~camera_t();

        glm::mat4 orthoView();
    private:
        glm::mat4 view{1.0f};

        glm::vec3 target{0.0f};

        void lookAt(glm::vec3 pos);

        float _near, _far;
    };
} // namespace eng
