#include "camera_t.hpp"

#include <cassert>

namespace eng
{
    camera_t::camera_t()
    { 
    }

    camera_t::~camera_t()
    {
    }

    void camera_t::ortho(float left, float right, float top, float bottom, float near, float far)
    {
        projection = glm::mat4{1.0f};
        projection[0][0] = 2.f / (right - left);
        projection[1][1] = 2.f / (bottom - top);
        projection[2][2] = 1.f / (far - near);
        projection[3][0] = -(right + left) / (right - left);
        projection[3][1] = -(bottom + top) / (bottom - top);
        projection[3][2] = -near / (far - near);
    }

    void camera_t::perspective(float fovy, float aspect, float near, float far)
    {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(fovy / 2.f);
        projection = glm::mat4{0.0f};
        projection[0][0] = 1.f / (aspect * tanHalfFovy);
        projection[1][1] = 1.f / (tanHalfFovy);
        projection[2][2] = far / (far - near);
        projection[2][3] = 1.f;
        projection[3][2] = -(far * near) / (far - near);
    }
} // namespace eng
