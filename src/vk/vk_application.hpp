#pragma once

#include "vk_engine.hpp"
#include <memory>

namespace vk
{
    class vk_application
    {
    public:
        static vk_application& getInstance() {
            static vk_application instance;
            return instance;
        }

        void run() { _engine.runMainLoop(); }

        vk_engine& getEngine() { return _engine; }

    private:
        vk_application() = default;
        vk_application(const vk_application&) = delete;
        vk_application& operator=(const vk_application&) = delete;

        vk_engine _engine;
    };
}
