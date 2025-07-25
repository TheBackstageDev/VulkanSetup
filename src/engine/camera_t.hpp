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
        camera_t();
        ~camera_t();

        void ortho(float left, float right, float top, float bottom, float near, float far);
        void perspective(float fovy, float aspect, float near, float far);

        glm::mat4& getProjection() { return projection; }
        glm::mat4& getView() { return view; }
    private:
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};

        glm::vec3 target{0.0f};

        void lookAt(glm::vec3 pos);
    };
} // namespace eng
