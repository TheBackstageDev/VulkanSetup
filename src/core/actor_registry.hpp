#pragma once

#include "vk/vk_engine.hpp"
#include <functional>
#include <vector>

namespace core
{
    using ActorConstructor = std::function<void(vk::vk_engine&, ecs::scene_t<>&)>;

    inline std::vector<ActorConstructor>& getActorRegistry() {
        static std::vector<ActorConstructor> registry;
        return registry;
    }

    inline void registerActor(ActorConstructor ctor) {
        getActorRegistry().push_back(std::move(ctor));
    }
} // namespace core

#define REGISTER_ACTOR(ActorType) \
    static bool _##ActorType##_registered = []() { \
        core::registerActor([](vk::vk_engine& engine, ecs::scene_t<>& scene) { \
            ActorType* actor = engine.addActor<ActorType>(); \
            actor->setScene(scene); \
        }); \
        return true; \
    }()
