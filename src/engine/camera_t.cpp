#include "camera_t.hpp"

namespace eng
{
    camera_t::camera_t(float near, float far)
        : _near(near), _far(far)
    { 
    }

    glm::mat4 camera_t::orthoView()
    {
        return glm::mat4{1.0f};
    }
} // namespace eng
