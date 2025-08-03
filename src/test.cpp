#include "core/systemactor.hpp"
#include "core/actor_registry.hpp"

using namespace vk;
using core::systemactor;

#include <iostream>

class testActor : public systemactor
{
public:

    // runs before the first frame
    void Awake() override
    {
        std::cout << "Just woke up!" << std::endl;
           
        eng::model_t model;
        eng::modelloader_t::loadModel("src/resource/suzane.obj", &model);

        modelId = _scene->create();
        auto& transform = _scene->construct<eng::transform_t>(modelId);
        transform.translation = {0.f, 0.f, 2.0f};
        transform.applyRotation(glm::vec3(0.f, 180.0f, 0.f));

        _scene->construct<eng::model_t>(modelId) = model;
    }

    // runs every frame
    void Update(float dt) override
    {
        std::cout << "Updated me!" << std::endl;
        _scene->get<eng::transform_t>(modelId).applyRotation(glm::vec3(0.f, 15.0f, 0.f) * dt);
    }

private:
    ecs::entity_id_t modelId;
};

REGISTER_ACTOR(testActor);