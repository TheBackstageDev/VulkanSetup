#include "camera_t.hpp"

namespace eng
{
    camera_t::camera_t(ecs::scene_t<>& scene)
        : _scene(scene)
    { 
        _id = scene.create();
        _scene.construct<eng::transform_t>(_id);
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

        const transform_t& transform = _scene.get<eng::transform_t>(_id);
        setViewYXZ(transform.translation, transform.rotation);
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
        
        const transform_t& transform = _scene.get<eng::transform_t>(_id);
        setViewYXZ(transform.translation, transform.rotation);
    }

    void camera_t::lookAt(glm::vec3 target, glm::vec3 up)
    {
        transform_t& transform = _scene.get<eng::transform_t>(_id);
        glm::vec3 direction = glm::normalize(target - transform.translation);
        
        transform.applyRotation(direction);

        glm::vec3 forward = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 camUp = transform.rotation * up;
        
        view = glm::lookAt(transform.translation, transform.translation + forward, camUp);
    }

    void camera_t::setViewYXZ(glm::vec3 translation, glm::quat rotation)
    {
        glm::mat4 rotMatrix = glm::mat4_cast(rotation);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -translation);

        view = rotMatrix * translationMatrix;
    }

} // namespace eng
