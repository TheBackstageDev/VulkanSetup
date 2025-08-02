#pragma once

#include "vk/vk_engine.hpp"
#include <functional>
#include <vector>

namespace core
{
    using ActorConstructor = std::function<void(vk::vk_engine&)>;

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
        core::registerActor([](vk::vk_engine& engine) { \
            engine.addActor<ActorType>(); \
        }); \
        return true; \
    }()
