#include "camera_t.hpp"

namespace eng
{
    camera_t::camera_t(float near, float far)
        : _near(near), _far(far)
    { 
    }

    camera_t::~camera_t()
    {
    }

    glm::mat4 camera_t::orthoView()
    {
        glm::mat4 ortho = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, _near, _far);
        ortho[1][1] *= -1;

        return glm::mat4{1.0f};
    }
} // namespace eng
