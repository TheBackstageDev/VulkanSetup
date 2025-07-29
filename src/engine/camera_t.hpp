#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/ecs.hpp"
#include "engine/transform_t.hpp"

namespace eng
{
    class camera_t
    {
    public:
        camera_t(ecs::scene_t<>& scene);

        void ortho(float left, float right, float top, float bottom, float near, float far);
        void perspective(float fovy = 70.f, float aspect = 1.0f, float near = 0.1f, float far = 100.f);

        void lookAt(glm::vec3 target, glm::vec3 up = {0.0f, -1.0f, 0.0f});

        glm::mat4& getProjection() { return projection; }
        glm::mat4& getView() { return view; }

        ecs::entity_id_t getId() const { return _id; }
    private:
        void setViewYXZ(glm::vec3 translation, glm::quat direction);

        float _aspect = 0.0f;

        ecs::entity_id_t _id = ecs::null_entity_id;
        ecs::scene_t<>& _scene;

        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
    };
} // namespace eng
