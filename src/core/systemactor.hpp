#pragma once

#include <volk/volk.h>

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

        uint32_t id;
    };
} // namespace core
