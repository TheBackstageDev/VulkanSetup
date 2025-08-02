#include "core/systemactor.hpp"
#include "core/actor_registry.hpp"

using namespace vk;
using core::systemactor;

#include <iostream>

class testActor : public systemactor
{
    // runs before the first frame
    void Awake() override
    {
        std::cout << "Just woke up!" << std::endl;
    }

    // runs every frame
    void Update(float dt) override
    {
        
    }
};

REGISTER_ACTOR(testActor);