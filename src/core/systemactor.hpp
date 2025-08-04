#pragma once

#include <volk/volk.h>
#include "core/ecs.hpp"
#include "core/ecs_defines.hpp"

namespace core
{
    class systemactor {
    public:
        virtual void Awake() {}
        virtual void Start() {}
        virtual void Update(float dt) {}
        virtual void LateUpdate(float dt) {}
        virtual void OnRender(VkCommandBuffer cmd) {}
        virtual bool ShouldDestroy() { return false; }
        virtual ~systemactor() = default;

        void setId(uint32_t id) { _id = id; } // to change for UUID later;
        uint32_t getId() { return _id; }

        void setScene(ecs::scene_t<>& scene) { _scene = &scene; }
    private:
        uint32_t _id;

    protected:
        ecs::scene_t<>* _scene;
    };
} // namespace core
